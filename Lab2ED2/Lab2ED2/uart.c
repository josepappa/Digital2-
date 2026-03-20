/*
 * uart.c
 *
 * Created: 1/29/2026 11:14:07 AM
 *  Author: jargu
 */ 

#include "uart.h"
#include <avr/io.h>

void UART_init(uint16_t ubrr)
{
    // Baud rate
    UBRR0H = (uint8_t)(ubrr >> 8);
    UBRR0L = (uint8_t)(ubrr & 0xFF);

    // 8N1: 8 bits, sin paridad, 1 stop
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);

    // Enable TX y RX
    UCSR0B = (1 << TXEN0) | (1 << RXEN0);
}

void UART_tx_char(char c)
{
    // Esperar buffer vacío
    while (!(UCSR0A & (1 << UDRE0)));
    UDR0 = c;
}

void UART_tx_str(const char *s)
{
    while (*s) UART_tx_char(*s++);
}

uint8_t UART_available(void)
{
    return (UCSR0A & (1 << RXC0)) ? 1 : 0;
}

char UART_rx_char(void)
{
    while (!UART_available());
    return UDR0;
}

uint8_t UART_rx_char_nb(char *c)
{
    if (UART_available()) {
        *c = UDR0;
        return 1;
    }
    return 0;
}
