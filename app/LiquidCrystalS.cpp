#include "LiquidCrystalS.h"

#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <util/delay_basic.h>
#include <util/delay.h>

namespace app {
    
static uint8_t _displayfunction;
static uint8_t _displaycontrol;
static uint8_t _displaymode;
static uint8_t _numlines;
static uint8_t _numcols;
static uint8_t _row_offsets[4];
    
void LiquidCrystal::begin(uint8_t cols, uint8_t lines, uint8_t dotsize) {
    DRIVER(LCD_RS, OUT);
    DRIVER(LCD_EN, OUT);
    DRIVER(LCD_D4, OUT);
    DRIVER(LCD_D5, OUT);
    DRIVER(LCD_D6, OUT);
    DRIVER(LCD_D7, OUT);
    _displayfunction = LCD_4BITMODE | LCD_1LINE | LCD_5x8DOTS;
    
    if (lines > 1) _displayfunction |= LCD_2LINE;
    _numcols = cols;
    _numlines = lines;

    setRowOffsets(0x00, 0x40, 0x00 + cols, 0x40 + cols);
    if ((dotsize != LCD_5x8DOTS) && (lines == 1)) _displayfunction |= LCD_5x10DOTS;
    _delay_ms(50);
    OFF(LCD_RS);
    OFF(LCD_EN);
    
    write4bits(0x03); // set 4 bit mode
    _delay_ms(5); // wait min 4.1ms
    write4bits(0x03); // set 4 bit mode
    _delay_ms(5); // wait min 4.1ms
    write4bits(0x03); // set 4 bit mode
    _delay_us(150);
    write4bits(0x02);
    command(LCD_FUNCTIONSET | _displayfunction);
    _displaycontrol = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;
    display();
    clear();
    _displaymode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;
    command(LCD_ENTRYMODESET | _displaymode);
}

void LiquidCrystal::setRowOffsets(int row0, int row1, int row2, int row3) {
    _row_offsets[0] = row0;
    _row_offsets[1] = row1;
    _row_offsets[2] = row2;
    _row_offsets[3] = row3;
}

void LiquidCrystal::clear() {
    command(LCD_CLEARDISPLAY);
    _delay_ms(2);
}

void LiquidCrystal::home() {
    command(LCD_RETURNHOME);
    _delay_ms(2);
}

void LiquidCrystal::setCursor(uint8_t col, uint8_t row) {
    const size_t max_lines = sizeof(_row_offsets) / sizeof(*_row_offsets);
    if ( row >= max_lines ) { row = max_lines - 1; }
    if ( row >= _numlines ) { row = _numlines - 1; }
    command(LCD_SETDDRAMADDR | (col + _row_offsets[row]));
}

void LiquidCrystal::noDisplay() {
    _displaycontrol &= ~LCD_DISPLAYON;
    command(LCD_DISPLAYCONTROL | _displaycontrol);
}

void LiquidCrystal::display() {
    _displaycontrol |= LCD_DISPLAYON;
    command(LCD_DISPLAYCONTROL | _displaycontrol);
}

void LiquidCrystal::noCursor() {
    _displaycontrol &= ~LCD_CURSORON;
    command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void LiquidCrystal::cursor() {
    _displaycontrol |= LCD_CURSORON;
    command(LCD_DISPLAYCONTROL | _displaycontrol);
}

void LiquidCrystal::noBlink() {
    _displaycontrol &= ~LCD_BLINKON;
    command(LCD_DISPLAYCONTROL | _displaycontrol);
}

void LiquidCrystal::blink() {
    _displaycontrol |= LCD_BLINKON;
    command(LCD_DISPLAYCONTROL | _displaycontrol);
}

void LiquidCrystal::scrollDisplayLeft(void) {
    command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT);
}

void LiquidCrystal::scrollDisplayRight(void) {
    command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT);
}

void LiquidCrystal::leftToRight(void) {
    _displaymode |= LCD_ENTRYLEFT;
    command(LCD_ENTRYMODESET | _displaymode);
}

void LiquidCrystal::rightToLeft(void) {
    _displaymode &= ~LCD_ENTRYLEFT;
    command(LCD_ENTRYMODESET | _displaymode);
}

void LiquidCrystal::autoscroll(void) {
    _displaymode |= LCD_ENTRYSHIFTINCREMENT;
    command(LCD_ENTRYMODESET | _displaymode);
}

void LiquidCrystal::noAutoscroll(void) {
    _displaymode &= ~LCD_ENTRYSHIFTINCREMENT;
    command(LCD_ENTRYMODESET | _displaymode);
}

void LiquidCrystal::createChar(uint8_t location, uint8_t charmap[]) {
    location &= 0x7; // we only have 8 locations 0-7
    command(LCD_SETCGRAMADDR | (location << 3));
    for (int i=0; i<8; i++) {
        write(charmap[i]);
    }
}

void   LiquidCrystal::command(uint8_t value) { send(value, 0); }
uint8_t LiquidCrystal::write(char value) { send(value, 1); return 1; } //!

void LiquidCrystal::send(uint8_t value, uint8_t mode) {
    if (mode) { ON(LCD_RS); } else { OFF(LCD_RS); }
    write4bits(value >> 4);
    write4bits(value);
}

void LiquidCrystal::pulseEnable(void) {
    OFF(LCD_EN);  _delay_us(0.6);
    ON(LCD_EN);   _delay_us(0.6);
    OFF(LCD_EN);  _delay_us(40);
}

void LiquidCrystal::write4bits(uint8_t value) {
    if ((value >> 0) & 0x01) { ON(LCD_D4); } else { OFF(LCD_D4); }
    if ((value >> 1) & 0x01) { ON(LCD_D5); } else { OFF(LCD_D5); }
    if ((value >> 2) & 0x01) { ON(LCD_D6); } else { OFF(LCD_D6); }
    if ((value >> 3) & 0x01) { ON(LCD_D7); } else { OFF(LCD_D7); }
    pulseEnable();
}

}
