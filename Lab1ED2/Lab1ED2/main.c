/*
 * Lab1ED2.c
 *
 * Created: 1/15/2026
 * Author : jargu
 */

#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include "display.h"




volatile uint8_t mux = 0;       // alterna entre transistores 0,1,2
volatile uint16_t sec = 0;      // acumula 200 overflows para obtener 1s
volatile uint8_t digit = 5;     // lo que se muestra en el display: 5..0
volatile uint8_t start = 0;		// da inicio al conteo regresivo
volatile uint8_t play = 0;      // habilita el juego
volatile uint8_t cont1 = 0;     // lleva la cuenta de botonazos del jugador 1
volatile uint8_t cont2 = 0;     // lleva la cuenta de botonazos del jugador 2
volatile uint8_t show1 = 0;     // muestra en PORTD el valor de cont1
volatile uint8_t show2 = 0;     // muestra en PORTD el valor de cont2

// ---- antirrebote (solo PC1 y PB3) ----
#define DEBOUNCE_TICKS 4        // 4 * 5ms = ~20ms
volatile uint8_t db_pc1 = 0;
volatile uint8_t db_pb3 = 0;
volatile uint8_t last_pinc = 0xFF;
volatile uint8_t last_pinb = 0xFF;

// Funcion para configurar el timer
void timer0_overflow_5ms_init(void){
	TCCR0A = 0x00;
	TCCR0B = (1<<CS02) | (1<<CS00);  // prescaler 1024
	TCNT0  = 178;                    // preload para ~5ms
	TIMSK0 = (1<<TOIE0);             // habilita overflow interrupt
}

void pinchange_int(void){
	PCICR |= (1<<PCIE0) | (1<<PCIE1);
	PCMSK1 |= (1<<PCINT8) | (1<<PCINT9);
    PCMSK0 |= (1<<PCINT3);
	PCIFR |= (1<<PCIF0) | (1<<PCIF1);
}

// Configuración general
void setup(void){
    cli();

    // -------- PORTD: PD0..PD6 como salidas --------
    DDRD |= (1<<DDD0) | (1<<DDD1) | (1<<DDD2) | (1<<DDD3) |
            (1<<DDD4) | (1<<DDD5) | (1<<DDD6);

    // -------- PORTB: PB0..PB2 como salidas (transistores) --------
    DDRB |= (1<<DDB0) | (1<<DDB1) | (1<<DDB2);

    // Inicial: todo apagado
    PORTB &= ~((1<<PORTB0) | (1<<PORTB1) | (1<<PORTB2));
    PORTD = 0x00;

 	// --------- PORTB: PB3 como entrada --------
    DDRB &= ~(1<<DDB3);
    PORTB |= (1<<PORTB3); // pull-up

	// --------- PORTC: PC0..PC2 como entradas (botones) --------
	DDRC &= ~((1<<DDC0) | (1<<DDC1) | (1<<DDC2));
	PORTC |= (1<<PORTC0) | (1<<PORTC1) | (1<<PORTC2); // pull-up

    // snapshots para detectar flanco (1->0)
    last_pinc = PINC;
    last_pinb = PINB;

    timer0_overflow_5ms_init();
	pinchange_int();

    sei();
}

int main(void){
	setup();
	while(1){
    }
}

// --------- INTERRUPCIONES ---------

ISR(TIMER0_OVF_vect){
    TCNT0 = 178;

    // antirrebote timers
    if (db_pc1) db_pc1--;
    if (db_pb3) db_pb3--;

    if (start)
    {
        sec++;

        if (sec >= 200)
        {
            sec = 0;

            if (digit > 0)
            {
                digit--;
            }
            else if (digit == 0)
            {
                play = 1;
                start = 0;
                digit = 0;
            }
        }
    }

    // ---- Multiplex PB0, PB1, PB2 ----
    mux++;
    if(mux > 2) mux = 0;

    PORTB &= ~((1<<PORTB0) | (1<<PORTB1) | (1<<PORTB2));

    if(mux == 0){
        if (start)
        {
            PORTD = 0X00;
        }
        else if (!start)
        {
            PORTD = cont1;
            PORTB |= (1<<PORTB0);
        }
    }
    else if (mux == 1){
        if (start)
        {
            PORTD = 0X00;
        }
        else if (!start)
        {
          PORTD = cont2;
          PORTB |= (1<<PORTB1);
        }
    }
    else{
        PORTD = display_get(digit);
        PORTB |= (1<<PORTB2);
    }
}

ISR(PCINT1_vect){
    uint8_t curr = PINC;

    if (!(PINC & (1<<PORTC0)))
	{
		start = 1;
        digit = 5;
        cont1 = 0;
        cont2 = 0;
	}
    else if ( (!(curr & (1<<PORTC1))) && (last_pinc & (1<<PORTC1)) && (db_pc1 == 0) )
    {
        db_pc1 = DEBOUNCE_TICKS;

        if (!start)
        {
            if (play && (cont1 < 0x0F) && (cont2 < 0x0F))
            {
                if (cont1 == 0)
                {
                    cont1++;
                }
                else if (cont1 == 0b00000100)
                {
                    cont1 = 0x0F;
                    digit = 1;
                    cont2 = 0;
                }
                else
                {
                    cont1 <<= 1;
                }
            }
        }
        else if (start)
        {
            cont1 = 0;
        }
    }

    last_pinc = curr;
}

ISR(PCINT0_vect){
    uint8_t curr = PINB;

    if ( (!(curr & (1<<PINB3))) && (last_pinb & (1<<PINB3)) && (db_pb3 == 0) )
    {
        db_pb3 = DEBOUNCE_TICKS;

        if (!start)
        {
            if (play && (cont1 < 0x0F) && (cont2 < 0x0F))
            {
                if (cont2 == 0)
                {
                    cont2++;
                }
                else if (cont2 == 0b00000100)
                {
                    cont2 = 0x0F;
                    digit = 2;
                    cont1 = 0;
                }
                else
                {
                    cont2 <<= 1;
                }
            }
        }
        else if (start)
        {
            cont2 = 0;
        }
    }

    last_pinb = curr;
}
