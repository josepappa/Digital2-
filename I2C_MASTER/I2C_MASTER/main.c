
#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>
#include <stdio.h>
#include "I2C/I2C.h"
#include "TIMER1/TIMER1.h"
#include "UART/UART_RECEIVER.h"
#include "HX711/HX711.h"
#include "LCD/DISPLAYLCD.h"


#include <stdlib.h>
#include <avr/interrupt.h>

#define SLAVE1      0x10
#define SLAVE2      0x20

#define SLAVE2_R   (0x20 << 1) | 0x01
#define SLAVE2_W   (0x20 << 1) & 0b11111110

#define SLAVE1_R   (0x10 << 1) | 0x01
#define SLAVE1_W   (0x10 << 1) & 0b11111110

volatile uint8_t vl53_stop_variable = 0;


uint8_t rxdato1;
volatile uint8_t step = 0;
volatile uint8_t flagstep = 0;
volatile uint8_t motorflag = 0;
volatile uint8_t motor_apagado = 0;
uint16_t dist_corr;

uint8_t rawdata1 = 0;
uint8_t rawdata2 = 0;
uint8_t rawdata3 = 0;

uint8_t ROJO = 0;
uint8_t VERDE = 0;
uint8_t AZUL = 0;

static uint8_t motor_off_sent = 0;

uint8_t bandaflag;

int32_t datosprocesados = 0;
HX711_Cal cal = {.offset = 0, .scale = 420.0f};
float masa = 0;
uint8_t tare_flag = 0;
char buffer[20];
char out[40];
char masa_str[16];

uint8_t listo = 0; // indica que se leyó la báscula

// -----------------------------------------------------------------------------
// 1) "MILLIS" con TIMER0 (no bloqueante)
// -----------------------------------------------------------------------------

static volatile uint32_t g_ms = 0;

static void timer0_millis_init(void)
{
    // Timer0 CTC -> interrupción cada 1 ms
    TCCR0A = (1<<WGM01);                 // CTC
    TCCR0B = (1<<CS01) | (1<<CS00);      // prescaler 64
    OCR0A  = 249;                        // 16MHz/64 = 250kHz -> 250 ticks = 1ms -> 249
    TIMSK0 = (1<<OCIE0A);                // habilita interrupción compare A
}

ISR(TIMER0_COMPA_vect)
{
    g_ms++;
}

static uint32_t millis(void)
{
    uint32_t t;
    cli();
    t = g_ms;
    sei();
    return t;
}

// Devuelve 1 si ya pasaron "period_ms" desde la última vez, si no devuelve 0.
// Actualiza automáticamente *last_ms cuando se cumple.
static uint8_t timer_elapsed_ms(uint32_t *last_ms, uint32_t period_ms)
{
    uint32_t now = millis();
    if ((now - *last_ms) >= period_ms)
    {
        *last_ms = now;
        return 1;
    }
    return 0;
}


static uint8_t request_color_from_slave(uint8_t *color_out)
{
    if(!color_out) return 0;

    // 1) Enviar comando 'C'
    if(!I2C_Master_Start()) return 0;
    if(!I2C_Master_Write(SLAVE1_W)) { I2C_Master_Stop(); return 0; }
    I2C_Master_Write('C');
    I2C_Master_Stop();

    // 2) Polling: leer hasta recibir R/G/B (timeout ~600ms)
    uint32_t t0 = millis();
    uint8_t c = 0x00;

    while((millis() - t0) < 600)
    {
        _delay_ms(20);

        if(!I2C_Master_Start()) return 0;
        if(!I2C_Master_Write(SLAVE1_R)) { I2C_Master_Stop(); return 0; }
        I2C_Master_Read(&c, 0);
        I2C_Master_Stop();

        if(c == 'R' || c == 'G' || c == 'B')
        {
            *color_out = c;
            return 1;
        }
        // si c==0x00 -> todavía midiendo, seguimos esperando
    }

    return 0; // timeout
}

// -----------------------------------------------------------------------------
// 2) Función: pedir 3 bytes al SLAVE2 y decodificar muestra HX711
//    - Importante: aquí NO salimos de main. Si falla I2C, devolvemos 0.
// -----------------------------------------------------------------------------
static uint8_t HX711_request_sample_from_slave(void)
{
    // Inicia comunicación
    if(!I2C_Master_Start()) return 0;

    if(!I2C_Master_Write(SLAVE2_W))
    {
        I2C_Master_Stop();
        return 0;
    }

    // Comando al slave: "W" = quiero peso
    I2C_Master_Write('W');

    // Repeated start para leer
    if(!I2C_Master_RepeatedStart())
    {
        I2C_Master_Stop();
        return 0;
    }

    if(!I2C_Master_Write(SLAVE2_R))
    {
        I2C_Master_Stop();
        return 0;
    }

    // Lee 3 bytes (ACK, ACK, NACK)
    I2C_Master_Read(&rawdata1, 1);
    I2C_Master_Read(&rawdata2, 1);
    I2C_Master_Read(&rawdata3, 0);

    I2C_Master_Stop();
    return 1;
}



// -----------------------------------------------------------------------------
// VL53L0X (TOF) - Rutinas mínimas I2C para lectura en mm (SINGLE SHOT)
// -----------------------------------------------------------------------------


#define VL53_ADDR7     0x29
#define VL53_W        ((VL53_ADDR7 << 1) & 0xFE)  // 0x52
#define VL53_R        ((VL53_ADDR7 << 1) | 0x01)  // 0x53

static uint8_t VL53_write_reg(uint8_t reg, uint8_t val)
{
    if(!I2C_Master_Start()) return 0;
    if(!I2C_Master_Write(VL53_W)) { I2C_Master_Stop(); return 0; }
    I2C_Master_Write(reg);
    I2C_Master_Write(val);
    I2C_Master_Stop();
    return 1;
}

static uint8_t VL53_read_reg(uint8_t reg, uint8_t *val)
{
    if(!I2C_Master_Start()) return 0;
    if(!I2C_Master_Write(VL53_W)) { I2C_Master_Stop(); return 0; }
    I2C_Master_Write(reg);

    if(!I2C_Master_RepeatedStart()) { I2C_Master_Stop(); return 0; }
    if(!I2C_Master_Write(VL53_R)) { I2C_Master_Stop(); return 0; }

    I2C_Master_Read(val, 0); // NACK
    I2C_Master_Stop();
    return 1;
}

static uint8_t VL53_read_multi(uint8_t start_reg, uint8_t *buf, uint8_t len)
{
    if(!I2C_Master_Start()) return 0;
    if(!I2C_Master_Write(VL53_W)) { I2C_Master_Stop(); return 0; }
    I2C_Master_Write(start_reg);

    if(!I2C_Master_RepeatedStart()) { I2C_Master_Stop(); return 0; }
    if(!I2C_Master_Write(VL53_R)) { I2C_Master_Stop(); return 0; }

    for(uint8_t i = 0; i < len; i++)
    {
        // ACK hasta el penúltimo, NACK al final
        I2C_Master_Read(&buf[i], (i < (len-1)) ? 1 : 0);
    }
    I2C_Master_Stop();
    return 1;
}

// Init mínimo (suficiente para lectura single-shot en muchos módulos)
static uint8_t VL53_init_min(void)
{
    uint8_t v = 0;

    // Pone bit0 en 0x89 (2V8 mode recomendado en implementaciones típicas)
    if(!VL53_read_reg(0x89, &v)) return 0;
    if(!VL53_write_reg(0x89, (uint8_t)(v | 0x01))) return 0;

    // Secuencia típica para obtener "stop_variable" y preparar el sensor
    if(!VL53_write_reg(0x88, 0x00)) return 0;

    if(!VL53_write_reg(0x80, 0x01)) return 0;
    if(!VL53_write_reg(0xFF, 0x01)) return 0;
    if(!VL53_write_reg(0x00, 0x00)) return 0;

    // lee stop_variable en 0x91
    if(!VL53_read_reg(0x91, &v)) return 0;   // v = stop_variable
    vl53_stop_variable = v;


    if(!VL53_write_reg(0x00, 0x01)) return 0;
    if(!VL53_write_reg(0xFF, 0x00)) return 0;
    if(!VL53_write_reg(0x80, 0x00)) return 0;

    // Config básica: sequence + interrupción "new sample ready"
    if(!VL53_write_reg(0x01, 0xFF)) return 0;  // SYSTEM_SEQUENCE_CONFIG
    if(!VL53_write_reg(0x0A, 0x04)) return 0;  // SYSTEM_INTERRUPT_CONFIG_GPIO
    if(!VL53_write_reg(0x0B, 0x01)) return 0;  // SYSTEM_INTERRUPT_CLEAR

    return 1;
}

// Dispara medición single shot + espera listo + lee mm
static uint8_t VL53_read_single_mm(uint16_t *mm_out)
{
    if(!mm_out) return 0;

    // Asegura stop_variable
    if(vl53_stop_variable == 0)
    {
        uint8_t tmp;
        if(!VL53_read_reg(0x91, &tmp)) return 0;
        vl53_stop_variable = tmp;
    }

    // --- Prepara una medición single-shot (handshake típico) ---
    if(!VL53_write_reg(0x80, 0x01)) return 0;
    if(!VL53_write_reg(0xFF, 0x01)) return 0;
    if(!VL53_write_reg(0x00, 0x00)) return 0;
    if(!VL53_write_reg(0x91, vl53_stop_variable)) return 0;
    if(!VL53_write_reg(0x00, 0x01)) return 0;
    if(!VL53_write_reg(0xFF, 0x00)) return 0;
    if(!VL53_write_reg(0x80, 0x00)) return 0;

    // Limpia cualquier interrupción vieja
    if(!VL53_write_reg(0x0B, 0x01)) return 0;

    // Inicia medición
    if(!VL53_write_reg(0x00, 0x01)) return 0;

    // ? Espera a que el sensor "acepte" el start (bit0 vuelve a 0)
    uint8_t sr = 0;
    uint16_t waited = 0;
    while(1)
    {
        if(!VL53_read_reg(0x00, &sr)) return 0;
        if((sr & 0x01) == 0) break;

        _delay_ms(2);
        waited += 2;
        if(waited >= 200) return 0;
    }

    // ? Espera data ready en RESULT_INTERRUPT_STATUS (0x13)
    uint8_t status = 0;
    waited = 0;
    while(1)
    {
        if(!VL53_read_reg(0x13, &status)) return 0;
        if((status & 0x07) != 0) break;

        _delay_ms(5);
        waited += 5;
        if(waited >= 500) return 0;
    }

    // Lee resultado (rango en mm está en offset +10)
    uint8_t buf[12];
    if(!VL53_read_multi(0x14, buf, sizeof(buf))) return 0;

    *mm_out = ((uint16_t)buf[10] << 8) | buf[11];

    // Limpia interrupción para la próxima medición
    if(!VL53_write_reg(0x0B, 0x01)) return 0;

    return 1;
}

// -----------------------------------------------------------------------------
// 4) FSM (máquina de estados) del master
// -----------------------------------------------------------------------------

static uint8_t read_ir2_from_slave(uint8_t *val_out)
{
    if(!val_out) return 0;

    if(!I2C_Master_Start()) return 0;
    if(!I2C_Master_Write(SLAVE2_W)) { I2C_Master_Stop(); return 0; }
    I2C_Master_Write('S');                // pedir estado IR
    if(!I2C_Master_RepeatedStart()) { I2C_Master_Stop(); return 0; }
    if(!I2C_Master_Write(SLAVE2_R)) { I2C_Master_Stop(); return 0; }

    I2C_Master_Read(val_out, 0);
    I2C_Master_Stop();
    return 1;
}



// -----------------------------------------------------------------------------
// 4) FSM (máquina de estados) del master
// -----------------------------------------------------------------------------
typedef enum {
    ST_PESANDO = 0,        // Pide muestras del HX711 y evalúa condición
    ST_COLOR,
    ST_MOVER_SERVO,        // Envía bandera/comando al slave para mover servo
    ST_SENSOR_IR,          // Continúa con el resto del sistema (ya NO lee báscula)
    STEPPER,
    SENSOR2_IR,
    LASER,                 //
    ST_CHECK_IR,
    
    


    
    
} estado_t;
int main(void)
{
    init8bits();
    UART_RECEIVER(103);
    I2C_Master_Init(100000, 1);

    DDRB |= (1<<DDB5);
    DDRC |= (1<<DDC1);
    PORTC &= ~(1<<PORTC1);

    timer0_millis_init();
    sei();

    HX711_tare_fromRaw(&cal, datosprocesados);

    const float THRESH_G = 100.0f;
    const uint8_t N_OK_CONSEC = 20;
    const uint32_t SAMPLE_PERIOD_MS = 50;

    estado_t estado = ST_PESANDO;

    uint8_t ok_count = 0;
    uint32_t t_sample = 0;

    while (1)
    {
        switch(estado)
        {

            //------------------------------------------------------------------------------
            // -------------------------- Case ST_PESANDO ----------------------------------

            case ST_PESANDO:
            {
                if(!I2C_Master_Start()) return 0;

                if(!I2C_Master_Write(SLAVE2_W))
                {
                    I2C_Master_Stop();
                    return 0;
                }

                I2C_Master_Write('U');
                I2C_Master_Stop();

                if(!timer_elapsed_ms(&t_sample, SAMPLE_PERIOD_MS))
                {
                    break;
                }

                if(!HX711_request_sample_from_slave())
                {
                    break;
                }

                datosprocesados = HX711_decode24(rawdata1, rawdata2, rawdata3);
                if(tare_flag == 0)
                {
                    HX711_tare_fromRaw(&cal, datosprocesados);
                    tare_flag = 1;
                }

                masa = HX711_toUnits(&cal, datosprocesados);

                // UART para mostrar pesa
                dtostrf(masa, 0, 2, masa_str);
                snprintf(out, sizeof(out), "Masa: %s g\r\n", masa_str);
                cadena(out);
                LCD_SET_CURSOR(1,1);
                LCD_WRITE_STRING("Masa(g)");
                LCD_SET_CURSOR(1,2);
                LCD_WRITE_STRING(masa_str);
                

                if(masa >= THRESH_G) ok_count++;
                else ok_count = 0;

                if(ok_count >= N_OK_CONSEC)
                {
                    listo = 1;
                    estado = ST_COLOR;
                }
                

                break;
            }
            
            


            //-----------------------------------------------------------------------------------
            // -------------------------- Case ST_COLOR -----------------------------------------
            
            case ST_COLOR:
            {
                if(!request_color_from_slave(&rxdato1))
                {
                    cadena("Color: TIMEOUT/FAIL\r\n");
                    estado = ST_COLOR;   
                    break;
                }

                if (rxdato1 == 'R')
                {
                    ROJO = 1; VERDE = 0; AZUL = 0;
                    cadena("Color: ROJO (R)\r\n");
                    LCD_SET_CURSOR(9,1);
                    LCD_WRITE_STRING("Color");
                    LCD_SET_CURSOR(9,2);
                    LCD_WRITE_STRING("Rojo");
                }
                else if (rxdato1 == 'G')
                {
                    VERDE = 1; ROJO = 0; AZUL = 0;
                    cadena("Color: VERDE (G)\r\n");
                    LCD_SET_CURSOR(9,1);
                    LCD_WRITE_STRING("Color");
                    LCD_SET_CURSOR(9,2);
                    LCD_WRITE_STRING("verde");
                    
                }
                else if (rxdato1 == 'B')
                {
                    AZUL = 1; ROJO = 0; VERDE = 0;
                    cadena("Color: AZUL (B)\r\n");
                    LCD_SET_CURSOR(9,1);
                    LCD_WRITE_STRING("Color");
                    LCD_SET_CURSOR(9,2);
                    LCD_WRITE_STRING("Azul");
                }
                else
                {
                    // masa es float
                    int32_t peso_g = (int32_t)(masa + 0.5f);   // redondea a entero

                    char out[24];
                    snprintf(out, sizeof(out), "P:%ld\n", peso_g);
                    cadena(out);
                }

                estado = ST_MOVER_SERVO;
                break;
            }


            
            


            //-----------------------------------------------------------------------------------
            // -------------------------- Case ST_MOVER_SERVO ----------------------------------
            
            case ST_MOVER_SERVO:
            {
                if(!I2C_Master_Start()) return 0;

                if(!I2C_Master_Write(SLAVE2_W))
                {
                    I2C_Master_Stop();
                    return 0;
                }

                I2C_Master_Write('H');
                I2C_Master_Stop();

                estado = STEPPER;
                break;
            }


            //------------------------------------------------------------------------------
            // -------------------------- Case STEPPER -------------------------------------


            case STEPPER:
            {
                while(rxdato1 != 'D')
                {
                    if(!I2C_Master_Start()) return 0;

                    if(!I2C_Master_Write(SLAVE1_W))
                    {
                        I2C_Master_Stop();
                        return 0;
                    }
                    I2C_Master_Write('D');

                    if(!I2C_Master_RepeatedStart())
                    {
                        I2C_Master_Stop();
                        return 0;
                    }

                    if(!I2C_Master_Write(SLAVE1_R))
                    {
                        I2C_Master_Stop();
                        return 0;
                    }

                    I2C_Master_Read(&rxdato1,0);
                    I2C_Master_Stop();
                }

                flagstep = 'O';
                motorflag = 'L';
                bandaflag = 1;
                
                char out[12];
                snprintf(out, sizeof(out), "B:%u\n", bandaflag);  // bandaflag = 0 o 1
                cadena(out);


                if(!I2C_Master_Start()) return 0;

                if(!I2C_Master_Write(SLAVE2_W))
                {
                    I2C_Master_Stop();
                    return 0;
                }
                I2C_Master_Write(flagstep);
                I2C_Master_Stop();

                if(!I2C_Master_Start()) return 0;

                if(!I2C_Master_Write(SLAVE1_W))
                {
                    I2C_Master_Stop();
                    return 0;
                }
                I2C_Master_Write(motorflag);
                I2C_Master_Stop();

                estado = SENSOR2_IR;
                break;
            }
            //------------------------------------------------------------------------------
            // -------------------------- Case SENSOR2_IR ----------------------------------

            case SENSOR2_IR:
            {
                while(rxdato1 != 'A')
                {
                    if(!I2C_Master_Start()) return 0;

                    if(!I2C_Master_Write(SLAVE2_W))
                    {
                        I2C_Master_Stop();
                        return 0;
                    }
                    I2C_Master_Write('S');

                    if(!I2C_Master_RepeatedStart())
                    {
                        I2C_Master_Stop();
                        return 0;
                    }

                    if(!I2C_Master_Write(SLAVE2_R))
                    {
                        I2C_Master_Stop();
                        return 0;
                    }

                    I2C_Master_Read(&rxdato1,0);
                    I2C_Master_Stop();
                }

                if(rxdato1 == 'A')
                {
                    flagstep = 'K';
                    motorflag = 'M';
                }

                if(!I2C_Master_Start()) return 0;

                if(!I2C_Master_Write(SLAVE2_W))
                {
                    I2C_Master_Stop();
                    return 0;
                }
                I2C_Master_Write(flagstep);
                I2C_Master_Stop();

                if(!I2C_Master_Start()) return 0;

                if(!I2C_Master_Write(SLAVE1_W))
                {
                    I2C_Master_Stop();
                    return 0;
                }
                I2C_Master_Write(motorflag);
                I2C_Master_Stop();
                bandaflag = 0;
                char out[12];
                snprintf(out, sizeof(out), "B:%u\n", bandaflag);  // bandaflag = 0 o 1
                cadena(out);

                
                estado = LASER;
                break;
            }
            //------------------------------------------------------------------------------
            // -------------------------- Case LASER ---------------------------------------

            
            case LASER:
            {
                static uint8_t vl53_ok = 0;
                
                static uint32_t t_laser = 0;

                if(!vl53_ok)
                {
                    _delay_ms(50);
                    if(VL53_init_min())
                    {
                        vl53_ok = 1;
                        cadena("VL53L0X OK\r\n");
                    }
                    else
                    {
                        cadena("VL53L0X FAIL\r\n");
                        _delay_ms(200);
                        break;
                    }
                }

                // leer cada 100ms (no bloqueante)
                if(!timer_elapsed_ms(&t_laser, 100))
                break;

                uint16_t dist_mm = 0;

                if(!VL53_read_single_mm(&dist_mm))
                {
                    cadena("TOF read FAIL\r\n");
                    break;
                }

                // corrección
                int16_t dc = (int16_t)dist_mm - 70;
                if(dc < 0) dc = 0;
                dist_corr = (uint16_t)dc;

                // UART debug
                char msg[40];
                snprintf(msg, sizeof(msg), "Distancia: %u mm\r\n", dist_corr);
                cadena(msg);

                // --------- Mostrar nivel en LCD mientras sube ----------
                if(dist_corr > 0 && dist_corr < 100){
                    LCD_SET_CURSOR(15,1); LCD_WRITE_STRING("lv");
                    LCD_SET_CURSOR(15,2); LCD_WRITE_STRING("1");
                }
                else if(dist_corr >= 100 && dist_corr < 300){
                    LCD_SET_CURSOR(15,1); LCD_WRITE_STRING("lv");
                    LCD_SET_CURSOR(15,2); LCD_WRITE_STRING("2");
                }
                else if(dist_corr >= 300 && dist_corr < 400){
                    LCD_SET_CURSOR(15,1); LCD_WRITE_STRING("lv");
                    LCD_SET_CURSOR(15,2); LCD_WRITE_STRING("3");
                }

                // --------- Condición de apagado según color ----------
                uint8_t hit = 0;
                if(ROJO  && dist_corr >= 200) hit = 1;
                if(VERDE && dist_corr >=  85) hit = 1;
                if(AZUL  && dist_corr >= 600) hit = 1;

                if(hit && !motor_off_sent)
                {
                    motorflag = 'L'; // apagar motor 

                    if(!I2C_Master_Start()) return 0;
                    if(!I2C_Master_Write(SLAVE1_W)) { I2C_Master_Stop(); return 0; }
                    I2C_Master_Write(motorflag);
                    I2C_Master_Stop();

                    motor_off_sent = 1;
                    motor_apagado = 1;
                    cadena("Motor OFF\r\n");

                    // ahora vamos a verificar IR con 'S'
                    estado = ST_CHECK_IR;
                }

                break;
            }
            
            case ST_CHECK_IR:
            {
                uint8_t ir = 0;

                if(!read_ir2_from_slave(&ir))
                {
                    cadena("IR2 read FAIL\r\n");
                    break;
                }

                // Debug por UART
                char m[24];
                snprintf(m, sizeof(m), "IR2: 0x%02X\r\n", ir);
                cadena(m);

                //  'X' = no presencia
                if(ir == 'X')
                {
                    motor_off_sent = 0;

                    // Reset de banderas para iniciar nuevo ciclo
                    ROJO = 0; VERDE = 0; AZUL = 0;
                    motor_apagado = 0;

                    ok_count = 0;         
                    tare_flag = 0;                       

                    cadena("Ciclo terminado -> ST_PESANDO\r\n");
                    estado = ST_PESANDO;
                }
                else
                {
                    // si todavía hay presencia, seguimos revisando hasta que sea X
                    estado = ST_CHECK_IR;
                }

                break;
            }



        }
        // <-- cierra switch
    }     // <-- cierra while
}         // <-- cierra main