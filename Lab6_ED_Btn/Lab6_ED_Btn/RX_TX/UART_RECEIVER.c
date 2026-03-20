/*
 * UART_RECEIVER.c
 *
 * Created: 26/01/2026 08:11:21
 *  Author: rodro
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <stdlib.h>

#include "UART_RECEIVER.h"

// ------------------------- RX LINE BUFFER (ESP -> ATmega) -------------------------
volatile char UART_rxbuf[16];
volatile uint8_t UART_rxidx = 0;
volatile uint8_t UART_line_ready = 0;

// Recibe chars hasta '\n' y marca línea lista
ISR(USART_RX_vect)
{
    char c = UDR0;

    if (c == '\n') {
        UART_rxbuf[UART_rxidx] = '\0';
        UART_rxidx = 0;
        UART_line_ready = 1;
    } else {
        if (UART_rxidx < (sizeof(UART_rxbuf) - 1)) {
            UART_rxbuf[UART_rxidx++] = c;
        } else {
            UART_rxidx = 0; // overflow -> reset
        }
    }
}

void UART_RECEIVER(uint8_t baudrate){
    // Ponemos como salida tx y entrada rx
    DDRD |= (1<<DDD1);    // TX
    DDRD &= ~(1<<DDD0);   // RX

    UCSR0A = 0;
    UCSR0B = (1 << RXCIE0) | (1 << RXEN0) | (1 << TXEN0);
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
    UBRR0 = baudrate; // 103 para 9600 a 16MHz
}

// Rutina para transmitir un caracter
void writechar(char caracter)
{
    while((UCSR0A & (1 << UDRE0)) == 0);
    UDR0 = caracter;
}

// rutina para transmitir una cadena
void cadena(char* frase){
    for(uint8_t i = 0; *(frase+i) != '\0'; i++){
        writechar(*(frase+i));
    }
}
