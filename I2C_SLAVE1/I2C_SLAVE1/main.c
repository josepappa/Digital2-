#define F_CPU 16000000UL

#include <avr/io.h>
#include <stdint.h>
#include <avr/interrupt.h>

#define Slaveadress 0x20

#include "I2C/I2C.h"
#include "TIMER1/TIMER1.h"
#include "HX711/HX711.h"
#include "PWM/PWM.h"

void setup(void);

/****************************************/
// Globals
volatile uint8_t buffer = 0;
volatile uint8_t DATOS  = 0;

uint8_t estado;

volatile uint8_t step = 0;
volatile uint8_t flagstep = 0;

int32_t rawdata;

uint8_t rawdata0;
uint8_t rawdata1;
uint8_t rawdata2;

uint8_t i = 0;
uint8_t LEERIR2  = 0;
uint8_t LEERPESA = 0;
uint8_t PWM = 0; 

volatile uint8_t dataseparada[3];

uint8_t stepper[8] = {
    0b10010000, 0b00110000, 0b01100000, 0b11000000,
    0b10010000, 0b00110000, 0b01100000, 0b11000000
};
/****************************************/

int main(void)
{
    // LED debug en PB5 (Arduino Nano: D13)
    DDRB  |= (1<<DDB5);
    PORTB &= ~(1<<PORTB5);

    DDRD |= (1<<DDD5)|(1<<DDD6)|(1<<DDD7)|(1<<DDD4);
    PORTD &= ~((1<<PORTD5)|(1<<PORTD6)|(1<<PORTD7)|(1<<PORTD4));

    HX711_init();
    timer1_init(64,65106);
    setup();
    I2C_Slave_Init(Slaveadress);
    PWM2CONFIG(1,1,1024);
    
    sei();

    while (1)
    {
        // ---------- LECTURA HX711 SOLO SI TE PIDIERON PESO ----------
        if(LEERPESA == 'W')
        {
            if(HX711_isReady())
            {
                rawdata  = HX711_readRaw24_A128();
                rawdata0 = (rawdata >> 16) & 0xFF;
                rawdata1 = (rawdata >>  8) & 0xFF;
                rawdata2 = (rawdata >>  0) & 0xFF;

                // “último valor guardado” (atómico)
                cli();
                dataseparada[0] = rawdata0;
                dataseparada[1] = rawdata1;
                dataseparada[2] = rawdata2;
                sei();
            }
        }

        // ---------- CONTROL STEPPER (si lo quieres aquí) ----------
        if(flagstep == 1)
        {
            PORTD = stepper[step];
        }
        else
        {
            PORTD &= ~((1<<PORTD5)|(1<<PORTD4)|(1<<PORTD6)|(1<<PORTD7));
        }
    }
}

// NON INTERRUPTION SUBRUTINE
void setup(void)
{
    cli();
    DDRD &= ~(1<<DDD1);
    PORTD |= (1<<PORTD1);

    PCICR |= (1<<PCIE2);
    PCMSK2 |= (1<<PCINT17);

    sei();
}

/****************************************/
// ISR: TWI (I2C) SLAVE
ISR(TWI_vect)
{
    estado = (TWSR & 0xF8);
    PORTB |= (1<<PORTB5);

    switch(estado)
    {
        // --- MASTER -> SLAVE (SLA+W) ---
        case 0x60:
        case 0x70:
        TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWEA)|(1<<TWIE);
        break;

        case 0x80:
        case 0x90:
        {
            buffer   = TWDR;
            LEERIR2  = buffer;
            LEERPESA = buffer;
            

            // -------- COMANDO STEPPER --------
            if(buffer == 'O')
            {
                flagstep = 1;
            }
            else if(buffer == 'K')
            {
                flagstep = 0;
            }
            else if(buffer == 'H')
            {
                OCR2B = 37;
 
            }
            else if(buffer == 'U')
            {
                OCR2B = 7;
                
            }
            
            // -------- COMANDO SERVO --------
            
            

            
        }

        // --- SLAVE -> MASTER (Lectura) ---
        case 0xA8: // SLA+R
        {
            i = 0;

            if(buffer == 'W')
            TWDR = dataseparada[0];
            
            else if (buffer == 'S')
            {
                TWDR = DATOS;
            }
            else
            TWDR = 0x00;

            TWCR = (1<<TWINT)|(1<<TWEA)|(1<<TWEN)|(1<<TWIE);
            break;
        }

        case 0xB8: // DATA_ACK (master quiere más)
        {
            i++;

            if(buffer == 'W')
            {
                if(i == 1)      TWDR = dataseparada[1];
                else if(i == 2) TWDR = dataseparada[2];
                else            TWDR = 0x00;
            }
            else
            {
                TWDR = 0x00;
            }
            TWCR = (1<<TWINT)|(1<<TWEA)|(1<<TWEN)|(1<<TWIE);
            break;
        }

        case 0xC0: // NACK
        case 0xC8:
        
        TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWEA)|(1<<TWIE);
        break;

        case 0xA0: // STOP
        TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWEA)|(1<<TWIE);
        break;

        default:
        TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWEA)|(1<<TWIE);
        break;
    }
}

/****************************************/
// ISR Pin Change
ISR(PCINT2_vect)
{
    if(LEERIR2 == 'S')
    {
        if ((PIND & (1<<PIND1)))
        {
            DATOS = 0x00;
        }
        else
        {
            
            DATOS = 'A';
        }
    }
}

// ISR Timer1 overflow
ISR(TIMER1_OVF_vect)
{
    TCNT1 = 65106;
    step++;
    if(step > 7) step = 0;
}
