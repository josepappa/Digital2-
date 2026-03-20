#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>

#include "adc.h"
#include "lcd.h"
#include "uart.h"

static void LCD_print_voltage_from_adc(uint16_t adc_value)
{
    uint32_t mv = ((uint32_t)adc_value * 5000UL) / 1023UL;
    uint16_t whole = mv / 1000;
    uint16_t frac  = (mv % 1000) / 10;

    char buf[6];
    buf[0] = (char)('0' + whole);
    buf[1] = '.';
    buf[2] = (char)('0' + (frac / 10));
    buf[3] = (char)('0' + (frac % 10));
    buf[4] = 'V';
    buf[5] = '\0';

    LCD_puts(buf);
}

static void LCD_print_u16_0_1023(uint16_t value)
{
    char buf[5];
    buf[4] = '\0';

    buf[3] = (value % 10) + '0'; value /= 10;
    buf[2] = (value % 10) + '0'; value /= 10;
    buf[1] = (value % 10) + '0'; value /= 10;
    buf[0] = (value % 10) + '0';

    if (buf[0] != '0') LCD_puts(buf);
    else if (buf[1] != '0') LCD_puts(&buf[1]);
    else if (buf[2] != '0') LCD_puts(&buf[2]);
    else LCD_puts(&buf[3]);
}

static void UART_tx_u16(uint16_t value)
{
    char buf[6];
    uint8_t i = 0;

    if (value == 0) { UART_tx_char('0'); return; }

    while (value > 0 && i < 5) {
        buf[i++] = (value % 10) + '0';
        value /= 10;
    }
    while (i > 0) UART_tx_char(buf[--i]);
}

int main(void)
{
    ADC_init();
    LCD_init();

    // 9600 baud @16MHz: UBRR = 103
    UART_init(103);

    int16_t s3 = 0;   // contador S3

    while (1)
    {
        // 1) Leer potenciómetros
        uint16_t adc_s1 = ADC_read(4); // A4
        uint16_t adc_s2 = ADC_read(5); // A5

        // 2) Recibir '+' o '-' desde PC (NO bloqueante)
        char rx;
        if (UART_rx_char_nb(&rx)) {
            if (rx == '+') s3++;
            else if (rx == '-') s3--;
        }

        // 3) Mostrar en LCD
        // Línea 1: S1 en voltaje
        LCD_gotoxy(0, 0);
        LCD_puts("S1:");
        LCD_print_voltage_from_adc(adc_s1);
        LCD_puts("   ");

        // Línea 2: S2 y S3
        LCD_gotoxy(0, 1);
        LCD_puts("S2:");
        LCD_print_u16_0_1023(adc_s2);
        LCD_puts(" ");

        LCD_puts("S3:");
        // imprimir s3 (simple, sin signos raros)
        if (s3 < 0) { LCD_putc('-'); LCD_print_u16_0_1023((uint16_t)(-s3)); }
        else        { LCD_print_u16_0_1023((uint16_t)s3); }
        LCD_puts("   ");

        // 4) Enviar por UART a la PC
        // Formato: S1(mV aprox o ADC) + S2 + S3
        UART_tx_str("S1_ADC=");
        UART_tx_u16(adc_s1);
        UART_tx_str(" S2_ADC=");
        UART_tx_u16(adc_s2);
        UART_tx_str(" S3=");
        if (s3 < 0) { UART_tx_char('-'); UART_tx_u16((uint16_t)(-s3)); }
        else        { UART_tx_u16((uint16_t)s3); }
        UART_tx_str("\r\n");

        _delay_ms(200);
    }
}
