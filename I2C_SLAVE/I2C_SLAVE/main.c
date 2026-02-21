/*
 * SLAVE2.c
 *
 * Created: 12/02/2026 19:20:50
 * Author : rodro
 */

#define F_CPU 16000000UL

#include <avr/io.h>
#include <stdint.h>
#include <avr/interrupt.h>
#include <util/delay.h>


#define Slaveadress 0x10

#include "I2C/I2C.h"
#include "TIMER1_FREQ/TIMER1_FREQ.h"   // aquí ya tienes TIMER1_Freq_* y TCS230_*

void setup(void);

/****************************************/
// Globals
volatile uint8_t buffer = 0;   // último byte recibido del master
volatile uint8_t DATOS  = 0;   // byte que el master leerá
uint8_t estado;
/****************************************/

int main(void)
{
    // LED debug en PB5 (Arduino Nano: D13)
    DDRB |= (1<<DDB5);

    
    DDRD |= (1<<DDD0) | (1<<DDD2) | (1<<DDD3) | (1<<DDD4);
    PORTD &= ~((1<<PORTD0) | (1<<PORTD2) | (1<<PORTD3) | (1<<PORTD4));

    // Setup interrupciones / pinchange
    setup();

    // I2C slave
    I2C_Slave_Init(Slaveadress);

    // Init Timer1 contador externo + sensor color
    TIMER1_Freq_Init();
    TCS230_Init();

    // (Opcional) valor inicial
    DATOS = 0x00;

    while (1)
    {
        if(buffer == 'M'){
            PORTD |= (1<<PORTD0);
            PORTD &= ~(1<<PORTD2);
        }
        else if(buffer == 'L'){
            PORTD &= ~(1<<PORTD0);
            PORTD |= (1<<PORTD2);
        }
        else if(buffer == 'C')
        {
            const uint16_t win_ms = 100;

            DATOS = 0x00;                 // BUSY/NO LISTO (para que el master sepa esperar)
            DATOS = TCS230_ReadBasicColor(win_ms);   // 'R','G','B'
            buffer = 0;
        }
        
        // Si querés, aquí puedes dejar otras tareas no bloqueantes
    }
}

/****************************************/
// NON INTERRUPTION SUBRUTINE/
void setup(void)
{
    cli();

    // PD1 como entrada con pull-up
    DDRD &= ~(1<<DDD1);
    PORTD |= (1<<PORTD1);

    // Pin change para port D
    PCICR |= (1<<PCIE2);

    // PD1 = PCINT17
    PCMSK2 |= (1<<PCINT17);

    sei();
}

/****************************************/
// ISR: TWI (I2C) SLAVE
ISR(TWI_vect)
{
    estado = (TWSR & 0xF8);
    switch(estado)
    {
        // --- MASTER -> SLAVE (SLA+W) ---
        case 0x60:
        case 0x70:
            TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWEA)|(1<<TWIE);
            break;

        case 0x80:
        case 0x90:
            buffer = TWDR;
            TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWEA)|(1<<TWIE);
            break;

        // --- MASTER <- SLAVE (SLA+R) ---
        case 0xA8:
        case 0xB8:
            TWDR = DATOS;  // 1 byte listo 
            TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWEA)|(1<<TWIE);
            break;

        case 0xC0:
        case 0xC8:
            TWCR = 0;
            TWCR = (1<<TWEN)|(1<<TWEA)|(1<<TWIE);
            break;

        case 0xA0:
            TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWEA)|(1<<TWIE);
            break;

        default:
            TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWEA)|(1<<TWIE);
            break;
    }
}

/****************************************/
// ISR Pin change
ISR(PCINT2_vect)
{
    if(buffer == 'D'){
        if(!(PIND & (1<<PIND1))){
            DATOS = 'D';
        
        }
            }
}
