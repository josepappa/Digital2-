/*
 * display.c
 *
 * Created: 1/22/2026 12:59:58 AM
 *  Author: jargu
 */ 

#include "display.h"

static const uint8_t display_lut[16] = {
    0b11000000, // 0
    0b11111001, // 1
    0b10100100, // 2
    0b10110000, // 3
    0b10011001, // 4
    0b10010010, // 5
    0b10000010, // 6
    0b11111000, // 7
    0b10000000, // 8
    0b10010000, // 9
    0b10001000, // A
    0b10000011, // b
    0b11000110, // C
    0b10100001, // d
    0b10000110, // E
    0b10001110  // F
};

uint8_t display_get(uint8_t hex)
{
    return display_lut[hex & 0x0F];
}