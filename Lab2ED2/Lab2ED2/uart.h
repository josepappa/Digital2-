/*
 * uart.h
 *
 * Created: 1/29/2026 11:13:41 AM
 *  Author: jargu
 */ 


#ifndef UART_H_
#define UART_H_

#include <stdint.h>

void UART_init(uint16_t ubrr);
void UART_tx_char(char c);
void UART_tx_str(const char *s);

uint8_t UART_available(void);
char UART_rx_char(void);          // bloqueante (espera)
uint8_t UART_rx_char_nb(char *c); // no bloqueante (si hay dato)



#endif /* UART_H_ */