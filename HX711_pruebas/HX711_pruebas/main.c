/*
 * HX711_pruebas.c
 *
 * Created: 2/14/2026 7:33:27 PM
 * Author : jargu
 */ 

#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>     // ltoa, dtostrf
#include "HX711.h"

// ---------------- UART ----------------
#define BAUD 9600UL
#define UBRR_VALUE ((F_CPU/16/BAUD)-1)

static void UART_init(void) {
    UBRR0H = (uint8_t)(UBRR_VALUE >> 8);
    UBRR0L = (uint8_t)(UBRR_VALUE & 0xFF);

    UCSR0B = (1<<TXEN0) | (1<<RXEN0);                 // TX y RX enable
    UCSR0C = (1<<UCSZ01) | (1<<UCSZ00);               // 8N1
}

static void UART_tx(char c) {
    while (!(UCSR0A & (1<<UDRE0)));
    UDR0 = c;
}

static void UART_print(const char *s) {
    while (*s) UART_tx(*s++);
}

static void UART_println(const char *s) {
    UART_print(s);
    UART_print("\r\n");
}

static uint8_t UART_rx_ready(void) {
    return (UCSR0A & (1<<RXC0));
}

static char UART_rx(void) {
    return UDR0;
}

// imprime int32
static void UART_print_i32(int32_t v) {
    char buf[16];
    ltoa((long)v, buf, 10);
    UART_print(buf);
}

// imprime float (dtostrf)
static void UART_print_f(float x, uint8_t width, uint8_t prec) {
    char buf[32];
    dtostrf(x, width, prec, buf);
    UART_print(buf);
}

// ---------------- MAIN ----------------
int main(void) {
    UART_init();
    HX711_init();

    UART_println("=== HX711 TEST (ATmega328P) ===");
    UART_println("HX711: SCK=PD2, DT=PD3");
    UART_println("UART: 9600 8N1");
    UART_println("Comandos: t=tare, +=scale up, -=scale down, r=reset scale");
    UART_println("--------------------------------");

    HX711_Cal cal = {0};
    cal.scale = 420.0f; // <- EJEMPLO: cuentas por gramo (ajustalo luego)

    // TARE inicial
    UART_println("Haciendo tare... no pongas nada.");
    _delay_ms(1200);
    int32_t raw0 = HX711_readAverage_A128(40);
    HX711_tare_fromRaw(&cal, raw0);

    UART_print("Offset RAW = ");
    UART_print_i32(cal.offset);
    UART_println("");

    while (1) {
        // lectura promedio
        int32_t raw = HX711_readAverage_A128(15);
        int32_t net = raw - cal.offset;

        float g = HX711_toUnits(&cal, raw);   // si scale = counts/gramo => unidades = gramos
        if (g > -1.0f && g < 1.0f) g = 0.0f;  // zona muerta 1g

        // imprimir
        UART_print("RAW=");
        UART_print_i32(raw);
        UART_print("  NET=");
        UART_print_i32(net);
        UART_print("  g=");
        UART_print_f(g, 0, 1);   // 0.1 g
        UART_print("  (scale=");
        UART_print_f(cal.scale, 0, 2);
        UART_println(")");

        // comandos simples por UART
        if (UART_rx_ready()) {
            char c = UART_rx();

            if (c == 't') {
                UART_println(">> TARE...");
                _delay_ms(400);
                int32_t r = HX711_readAverage_A128(40);
                HX711_tare_fromRaw(&cal, r);
                UART_print(">> Nuevo offset RAW = ");
                UART_print_i32(cal.offset);
                UART_println("");
            }
            else if (c == '+') {
                cal.scale *= 1.02f;  // +2%
                UART_print(">> scale = ");
                UART_print_f(cal.scale, 0, 2);
                UART_println("");
            }
            else if (c == '-') {
                cal.scale *= 0.98f;  // -2%
                UART_print(">> scale = ");
                UART_print_f(cal.scale, 0, 2);
                UART_println("");
            }
            else if (c == 'r') {
                cal.scale = 420.0f;
                UART_println(">> scale reset a 420.0");
            }
        }

        _delay_ms(200);
    }
}
