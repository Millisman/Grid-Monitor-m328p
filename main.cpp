#include "mcu/mcu.h"
#include "avr/uart.h"
#include "avr/adc_mega.h"
#include "app/adc_driver.h"
#include "console/console.h"
#include <stdio.h>
#include "app/LiquidCrystalS.h"

int putc_uart0(char ch, FILE *stream) {
    (void)stream;
    if (ch == '\n') uart0_putc('\r');
    uart0_putc(ch);
    return 0;
}

static uint8_t lcd_Block = 0;

int putc_lcd0(char ch, FILE *stream) {
    (void)stream;
    if ( ch >= ' ' && ch <= '~' && lcd_Block == 0 ) { app::LiquidCrystal::write(ch); }    
    return 0;
}

static FILE file_uart0;
static FILE file_lcd0;

int main() {
    MCUSR = 0;
    wdt_enable(WDTO_2S);
    wdt_reset();
    uart_init(UART_BAUD_SELECT(115200, F_CPU));
    millis_begin();
    fdev_setup_stream( &file_uart0, putc_uart0, NULL, _FDEV_SETUP_WRITE);
    stdout = &file_uart0;
    fdev_setup_stream( &file_lcd0, putc_lcd0,   NULL, _FDEV_SETUP_WRITE);
    stderr = &file_lcd0;
    sei();
    printf_P(PSTR("Starting...\n"));
    app::LiquidCrystal::begin(16, 2);
    app::LiquidCrystal::clear();
    fprintf_P(stderr, PSTR("GREENCAR MNE v01"));
    app::LiquidCrystal::setCursor(0 ,1);
    fprintf_P(stderr, PSTR("VOLTAGE  MONITOR"));
    lcd_Block = 3;
    app::AdcDriver::init();
    app::Console::init();

    while (1) {
        
        switch (elapsed_time()) {
            case ELAPSED_100ms:
                app::Console::update_100ms();
                break;
            case ELAPSED_1S:
                app::AdcDriver::update_1S();
                app::Console::update_1S();
                if (lcd_Block > 0) {
                    --lcd_Block;
                    if (lcd_Block == 0) { app::LiquidCrystal::clear(); }
                }
                break;
            default:
                app::AdcDriver::update();
                app::Console::update();
        }
        
       wdt_reset();
    }

}
