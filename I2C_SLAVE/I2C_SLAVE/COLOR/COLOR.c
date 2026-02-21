/*
 * COLOR.c
 *
 * Created: 2/17/2026 6:41:31 PM
 *  Author: jargu
 */ 

#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include "COLOR.h"
#include "TIMER1_FREQ/TIMER1_FREQ.h"  

/* ========================
   CONFIGURACIÓN DE PINES
   ======================== */

#define TCS_PORT    PORTB
#define TCS_DDR     DDRB

#define TCS_S0      PB0   // D8
#define TCS_S1      PB1   // D9
#define TCS_S2      PB2   // D10
#define TCS_S3      PB3   // D11

#define TCS_OUT_DDR  DDRD
#define TCS_OUT_PIN  PD5   // D5 = T1 (Timer1 external counter)

/* ========================
   FUNCIONES INTERNAS
   ======================== */

static void TCS230_SelectRed(void)
{
    TCS_PORT &= ~(1<<TCS_S2);
    TCS_PORT &= ~(1<<TCS_S3);
}

static void TCS230_SelectBlue(void)
{
    TCS_PORT &= ~(1<<TCS_S2);
    TCS_PORT |=  (1<<TCS_S3);
}

static void TCS230_SelectGreen(void)
{
    TCS_PORT |=  (1<<TCS_S2);
    TCS_PORT |=  (1<<TCS_S3);
}

/* ========================
   API PÚBLICA
   ======================== */

void TCS230_Init(void)
{
    // S0..S3 como salida
    TCS_DDR |= (1<<TCS_S0) | (1<<TCS_S1) | (1<<TCS_S2) | (1<<TCS_S3);

    // Escala 20%: S0=1, S1=0
    TCS_PORT |=  (1<<TCS_S0);
    TCS_PORT &= ~(1<<TCS_S1);

    // OUT como entrada
    TCS_OUT_DDR &= ~(1<<TCS_OUT_PIN);

    // Filtro inicial rojo
    TCS230_SelectRed();
}

uint32_t TCS230_ReadRed(uint16_t window_ms)
{
    TCS230_SelectRed();
    _delay_ms(3);
    return TIMER1_Freq_Measure(window_ms);
}

uint32_t TCS230_ReadGreen(uint16_t window_ms)
{
    TCS230_SelectGreen();
    _delay_ms(3);
    return TIMER1_Freq_Measure(window_ms);
}

uint32_t TCS230_ReadBlue(uint16_t window_ms)
{
    TCS230_SelectBlue();
    _delay_ms(3);
    return TIMER1_Freq_Measure(window_ms);
}

uint8_t TCS230_ReadBasicColor(uint16_t window_ms)
{
    uint32_t fR, fG, fB;

    fR = TCS230_ReadRed(window_ms);
    fG = TCS230_ReadGreen(window_ms);
    fB = TCS230_ReadBlue(window_ms);

    // Clasificación simple por dominante
    if (fR >= fG && fR >= fB) return 'R';
    if (fG >= fR && fG >= fB) return 'G';
    return 'B';
}
