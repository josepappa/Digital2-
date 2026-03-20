/*
 * adc.c
 *
 * Created: 1/22/2026 7:58:18 PM
 *  Author: jargu
 */ 
#include "adc.h"
#include <avr/io.h>

void ADC_init(void)
{
    
    ADMUX = (1 << REFS0);

    // Habilitar ADC y prescaler 128 -> 16MHz/128 = 125kHz (bien para ADC)
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}

uint16_t ADC_read(uint8_t channel)
{
    channel &= 0x07; // ATmega328P: ADC0-ADC7

    
    ADMUX = (ADMUX & 0xF8) | channel;

    // Iniciar conversión
    ADCSRA |= (1 << ADSC);

    // siempre está leyendo el adc 
    while (ADCSRA & (1 << ADSC));

    // Retorna el valor del adc 
    return ADC;
}
