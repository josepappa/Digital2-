/*
 * COMM.c
 *
 * Created: 2/15/2026 2:15:03 PM
 *  Author: jargu
 */ 

// BOARD_COM/board_com.c
#include "board_com.h"
#include "I2C/I2C.h"

//-------- FUNCIÓN GENÉRICA: ENVIAR 1 BYTE A SLAVE2 --------
uint8_t COM_SendCommand_Slave2(uint8_t cmd)
{
    if(!I2C_Master_Start()) return 0;

    if(!I2C_Master_Write(SLAVE2_W))
    {
        I2C_Master_Stop();
        return 0;
    }

    I2C_Master_Write(cmd);

    I2C_Master_Stop();
    return 1;
}


//-------- SERVO: ENVIAR COMANDO A SLAVE2 --------
uint8_t COM_SendServo_Slave2(uint8_t servo_cmd)
{
    // servo_cmd normalmente será 'S'
    return COM_SendCommand_Slave2(servo_cmd);
}


//-------- HX711/PESO: PEDIR 3 BYTES A SLAVE2 --------
uint8_t COM_RequestWeight_Slave2(uint8_t *b0, uint8_t *b1, uint8_t *b2)
{
    if(!b0 || !b1 || !b2) return 0;

    // START
    if(!I2C_Master_Start()) return 0;

    // SLA+W
    if(!I2C_Master_Write(SLAVE2_W))
    {
        I2C_Master_Stop();
        return 0;
    }

    // Comando al slave: 'W' = quiero peso
    I2C_Master_Write('W');

    // Repeated Start para leer
    if(!I2C_Master_RepeatedStart())
    {
        I2C_Master_Stop();
        return 0;
    }

    // SLA+R
    if(!I2C_Master_Write(SLAVE2_R))
    {
        I2C_Master_Stop();
        return 0;
    }

    // Read 3 bytes (ACK, ACK, NACK)
    I2C_Master_Read(b0, 1);
    I2C_Master_Read(b1, 1);
    I2C_Master_Read(b2, 0);

    I2C_Master_Stop();
    return 1;
}
