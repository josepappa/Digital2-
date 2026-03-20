#define F_CPU 16000000UL

#include "lcd.h"
#include <avr/io.h>
#include <util/delay.h>

// ======================================================
// CONFIGURACIÓN DE PINES
// Datos LCD:
//   D0..D1 -> PB0..PB1
//   D2..D7 -> PD2..PD7
//
// Control LCD:
//   RS -> PC1
//   RW -> PC2
//   E  -> PC3
// ======================================================

// ---- Datos ----
#define LCD_DATA_DDR_B    DDRB
#define LCD_DATA_PORT_B   PORTB
#define LCD_DATA_DDR_D    DDRD
#define LCD_DATA_PORT_D   PORTD

// ---- Control ----
#define LCD_CTRL_DDR      DDRC
#define LCD_CTRL_PORT     PORTC

#define LCD_RS   PC1
#define LCD_RW   PC2
#define LCD_E    PC3

// ======================================================
// FUNCIONES INTERNAS
// ======================================================

static void LCD_enable_pulse(void)
{
    LCD_CTRL_PORT |= (1 << LCD_E);
    _delay_us(1);
    LCD_CTRL_PORT &= ~(1 << LCD_E);
    _delay_us(100);
}

// Escribe un byte completo en el bus del LCD
static void LCD_write_bus(uint8_t v)
{
    // PB0..PB1  <- bits 0..1
    LCD_DATA_PORT_B = (LCD_DATA_PORT_B & 0xFC) | (v & 0x03);

    // PD2..PD7  <- bits 2..7 (NO tocar PD0/PD1)
    LCD_DATA_PORT_D = (LCD_DATA_PORT_D & 0x03) | (v & 0xFC);
}

// ======================================================
// FUNCIONES PÚBLICAS
// ======================================================

void LCD_command(uint8_t cmd)
{
    LCD_CTRL_PORT &= ~(1 << LCD_RS); // RS = 0 -> comando
    LCD_CTRL_PORT &= ~(1 << LCD_RW); // RW = 0 -> escribir

    LCD_write_bus(cmd);
    LCD_enable_pulse();

    // Comandos lentos
    if (cmd == 0x01 || cmd == 0x02) _delay_ms(2);
}

void LCD_data(uint8_t data)
{
    LCD_CTRL_PORT |= (1 << LCD_RS);  // RS = 1 -> dato
    LCD_CTRL_PORT &= ~(1 << LCD_RW); // RW = 0 -> escribir

    LCD_write_bus(data);
    LCD_enable_pulse();
}

void LCD_init(void)
{
    // ---- Configurar pines de datos como salida ----
    LCD_DATA_DDR_B |= 0x03;  // PB0..PB1
    LCD_DATA_DDR_D |= 0xFC;  // PD2..PD7

    LCD_DATA_PORT_B &= 0xFC;
    LCD_DATA_PORT_D &= 0x03;

    // ---- Pines de control como salida ----
    LCD_CTRL_DDR |= (1 << LCD_RS) | (1 << LCD_RW) | (1 << LCD_E);
    LCD_CTRL_PORT &= ~((1 << LCD_RS) | (1 << LCD_RW) | (1 << LCD_E));

    // ---- Secuencia de inicialización robusta (datasheet) ----
    _delay_ms(20);          // >15 ms after power-on

    LCD_write_bus(0x30);    // Function set (8-bit)
    LCD_enable_pulse();
    _delay_ms(5);           // >4.1 ms

    LCD_write_bus(0x30);    // Repetición
    LCD_enable_pulse();
    _delay_us(150);         // >100 us

    LCD_write_bus(0x30);    // Tercera vez
    LCD_enable_pulse();
    _delay_us(150);

    // ---- Ahora el LCD ya entiende comandos ----
    LCD_command(0x38);      // 8-bit, 2 líneas, 5x8
    LCD_command(0x0C);      // display ON, cursor OFF
    LCD_command(0x06);      // entry mode
    LCD_command(0x01);      // clear
    _delay_ms(2);
}


void LCD_clear(void)
{
    LCD_command(0x01);
    _delay_ms(2);
}

void LCD_home(void)
{
    LCD_command(0x02);
    _delay_ms(2);
}

void LCD_gotoxy(uint8_t col, uint8_t row)
{
    uint8_t addr = (row == 0) ? 0x00 : 0x40;
    addr += col;
    LCD_command(0x80 | addr);
}

void LCD_putc(char c)
{
    LCD_data((uint8_t)c);
}

void LCD_puts(const char *s)
{
    while (*s) LCD_putc(*s++);
}
