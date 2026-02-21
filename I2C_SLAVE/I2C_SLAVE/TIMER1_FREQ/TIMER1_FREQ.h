/*
 * TIMER1_FREQ.h
 *
 * Created: 2/17/2026 2:12:31 AM
 *  Author: jargu
 */ 


#ifndef TIMER1_FREQ_H_
#define TIMER1_FREQ_H_

#include <stdint.h>

// Inicializa Timer1 como contador externo en pin T1 (PD5)
void TIMER1_Freq_Init(void);

// Reinicia el contador
void TIMER1_Freq_Reset(void);

// Lee el valor actual del contador
uint16_t TIMER1_Freq_Read(void);

// Mide frecuencia durante ventana_ms milisegundos
uint32_t TIMER1_Freq_Measure(uint16_t ventana_ms);

//-----------------------------------------------------
// ------------------- COLOR --------------------------

// Inicializa pines del TCS230
void TCS230_Init(void);

// Devuelve frecuencia de cada color (Hz)
uint32_t TCS230_ReadRed(uint16_t window_ms);
uint32_t TCS230_ReadGreen(uint16_t window_ms);
uint32_t TCS230_ReadBlue(uint16_t window_ms);

// Devuelve color básico dominante: 'R', 'G' o 'B'
uint8_t TCS230_ReadBasicColor(uint16_t window_ms);

#endif /* TIMER1_FREQ_H_ */
