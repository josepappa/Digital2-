/*
 * Botones_A0_A5_UART.c
 *
 * ATmega328P - Arduino Nano
 * Interrupciones por cambio de pin en A0-A5
 * Muestra por UART:
 * Arriba, Abajo, Izquierda, Derecha, A, B
 */

#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>

// =====================================================
// PROTOTIPOS DE TU LIBRERIA UART
// AJUSTA ESTOS NOMBRES A TU LIBRERIA REAL
// =====================================================

// Si tu función de inicialización se llama diferente, cámbiala aquí
void UART_RECEIVER(uint16_t ubrr);

// Si tu función para mandar strings se llama cadena(), déjala así
void cadena(char *texto);

// =====================================================
// VARIABLES GLOBALES
// =====================================================

volatile uint8_t estado_anterior = 0x3F;   // PC0-PC5 en 1 al inicio por pull-up
volatile uint8_t evento_pendiente = 0;     // 0 = nada, 1..6 = botón detectado

// =====================================================
// SETUP
// =====================================================

void setup(void) {
    cli();

    // A0-A5 como entradas
    DDRC &= ~((1 << DDC0) | (1 << DDC1) | (1 << DDC2) |
              (1 << DDC3) | (1 << DDC4) | (1 << DDC5));

    // Pull-up en A0-A5
    PORTC |= ((1 << PORTC0) | (1 << PORTC1) | (1 << PORTC2) |
              (1 << PORTC3) | (1 << PORTC4) | (1 << PORTC5));

    // Estado inicial de los botones
    estado_anterior = PINC & 0x3F;

    // Habilitar interrupciones por cambio de pin del grupo PORTC
    PCICR |= (1 << PCIE1);

    // Habilitar PCINT8 a PCINT13 (A0 a A5)
    PCMSK1 |= (1 << PCINT8)  | (1 << PCINT9)  | (1 << PCINT10) |
              (1 << PCINT11) | (1 << PCINT12) | (1 << PCINT13);

    // Inicializar UART
    UART_RECEIVER(103);   // 9600 baud para 16 MHz, si así funciona tu librería

    sei();
}

// =====================================================
// ISR DE CAMBIO DE PIN EN PORTC
// =====================================================

ISR(PCINT1_vect) {
    uint8_t estado_actual = PINC & 0x3F;

    // Detectar flanco de bajada: pasó de 1 a 0 = botón presionado
    uint8_t flancos_bajada = estado_anterior & (~estado_actual);

    // Guardar el último estado
    estado_anterior = estado_actual;

    // Solo guardar evento si no hay uno pendiente
    // prioridad de A0 a A5
    if (evento_pendiente == 0) {
        if (flancos_bajada & (1 << PC0)) {
            evento_pendiente = 1;   // Arriba
        }
        else if (flancos_bajada & (1 << PC1)) {
            evento_pendiente = 2;   // Abajo
        }
        else if (flancos_bajada & (1 << PC2)){ 
            evento_pendiente = 3;   // Izquierda
        }
        else if (flancos_bajada & (1 << PC3)) {
            evento_pendiente = 4;   // Derecha
        }
        else if (flancos_bajada & (1 << PC4)) {
            evento_pendiente = 5;   // A
        }
        else if (flancos_bajada & (1 << PC5)) {
            evento_pendiente = 6;   // B
        }
    }
}

// =====================================================
// MAIN
// =====================================================

int main(void) {
    setup();

    //cadena("Sistema iniciado\r\n");

    while (1) {
        if (evento_pendiente != 0) {
            switch (evento_pendiente) {
                case 1:
                    cadena("1");
                    break;

                case 2:
                    cadena("2");
                    break;

                case 3:
                    cadena("3");
                    break;

                case 4:
                    cadena("4");
                    break;

                case 5:
                    cadena("5");
                    break;

                case 6:
                    cadena("6");
                    break;
            }

            evento_pendiente = 0;
        }
    }
}