/*
 * lcd.h
 *
 * Created: 1/22/2026 8:12:36 PM
 *  Author: jargu
 */ 


#ifndef LCD_H_
#define LCD_H_

#include <stdint.h>

void LCD_init(void);

void LCD_command(uint8_t cmd);
void LCD_data(uint8_t data);

void LCD_clear(void);
void LCD_home(void);
void LCD_gotoxy(uint8_t col, uint8_t row);

void LCD_putc(char c);
void LCD_puts(const char *s);

#endif /* LCD_H_ */
