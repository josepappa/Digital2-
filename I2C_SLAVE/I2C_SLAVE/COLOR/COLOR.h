/*
 * COLOR.h
 *
 * Created: 2/17/2026 6:42:23 PM
 *  Author: jargu
 */ 


#ifndef COLOR_H_
#define COLOR_H_

#include <stdint.h>

// Inicializa pines del TCS230
void TCS230_Init(void);

// Devuelve frecuencia de cada color (Hz)
uint32_t TCS230_ReadRed(uint16_t window_ms);
uint32_t TCS230_ReadGreen(uint16_t window_ms);
uint32_t TCS230_ReadBlue(uint16_t window_ms);

// Devuelve color básico dominante: 'R', 'G' o 'B'
uint8_t TCS230_ReadBasicColor(uint16_t window_ms);

#endif /* COLOR_H_ */
