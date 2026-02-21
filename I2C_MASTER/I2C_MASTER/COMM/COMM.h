/*
 * COMM.h
 *
 * Created: 2/15/2026 2:14:41 PM
 *  Author: jargu
 */ 


// BOARD_COM/board_com.h
#ifndef BOARD_COM_H
#define BOARD_COM_H

#include <stdint.h>

//-------- I2C ADDRESSES --------
#define SLAVE1_ADDR   0x10
#define SLAVE2_ADDR   0x20

// (7-bit addr << 1) + R/W
#define SLAVE2_R      ((SLAVE2_ADDR << 1) | 0x01)
#define SLAVE2_W      ((SLAVE2_ADDR << 1) & 0xFE)

#define SLAVE1_R      ((SLAVE1_ADDR << 1) | 0x01)
#define SLAVE1_W      ((SLAVE1_ADDR << 1) & 0xFE)


//-------- API: COMMANDOS A SLAVE2 --------

// Pide 3 bytes al SLAVE2 usando comando 'W'.
// Retorna 1 si OK, 0 si falló I2C.
uint8_t COM_RequestWeight_Slave2(uint8_t *b0, uint8_t *b1, uint8_t *b2);

// Envía comando para mover servo al SLAVE2.
// Por defecto normalmente usarás 'S'.
// Retorna 1 si OK, 0 si falló I2C.
uint8_t COM_SendServo_Slave2(uint8_t servo_cmd);

// Envía un comando de 1 byte al SLAVE2 (genérica).
// Útil para futuros comandos: 'O', 'K', etc.
// Retorna 1 si OK, 0 si falló I2C.
uint8_t COM_SendCommand_Slave2(uint8_t cmd);

#endif // BOARD_COM_H
