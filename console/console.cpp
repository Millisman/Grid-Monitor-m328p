#include "mcu/mcu.h"
#include "console.h"


#include <stdio.h>
#include "mcu/timer.h"
#include <math.h>
#include "avr/uart.h"
#include <stdlib.h>
#include <util/delay.h>


namespace app {
    
void Console::init() { }

#define CONSOLE_BUFFER 64

void cmd_cd(char* args) {
    (void) args;
    printf_P(PSTR("CD\n"));
}
void cmd_uptime(char* args) {
    (void) args;
    printf_P(PSTR("UPTIME\n"));
}

void cmd_reboot(char* args) {
    (void) args;
    uart0_flush();
    wdt_disable();
    cli();
    _delay_ms(200);
//     redraw
//     wdt_enable(WDTO_15MS);
//     while (1) { }
    typedef void (*do_reboot_t)(void);
    const do_reboot_t do_reboot = (do_reboot_t)((FLASHEND - 511) >> 1);    
    TCCR0A = TCCR1A = TCCR2A = 0; // make sure interrupts are off and timers are reset.
    MCUSR = 0;
    do_reboot();
}

typedef struct {
    const char *cmd;
    const char *help;
    void (*func)(char *);
} stringfunc;

const char pstr_reboot[]    PROGMEM = "reboot";
const char pstr_reboot_h[]  PROGMEM = "help string for reboot";
const char pstr_cd[]        PROGMEM = "cd";
const char pstr_cd_h[]      PROGMEM = "help string for cd";
const char pstr_uptime[]    PROGMEM = "uptime";
const char pstr_uptime_h[]  PROGMEM = "help string for uptime";
const char pstr_help[]      PROGMEM = "help";
const char pstr_help_h[]    PROGMEM = "help string for help";
const char pstr_minmax[]    PROGMEM = "minmax";
const char pstr_minmax_h[]  PROGMEM = "help string for minmax 1/0";


static bool print_min_max = false;

void cmd_minmax(char *args) {
    if (args) {
        print_min_max = (bool)atoi(args);
    }
}




stringfunc procs[] = {
    {pstr_minmax,   pstr_minmax_h,  cmd_minmax},
    {pstr_reboot,   pstr_reboot_h,  cmd_reboot},
    {pstr_cd,       pstr_cd,        cmd_cd},
    {pstr_uptime,   pstr_uptime,    cmd_uptime},
    {pstr_help,     pstr_help,      cmd_help},
    {0, 0, 0}
};

void cmd_help (char *args) {
    (void) args;
    for (stringfunc* f = procs; f->cmd; f++) {
        uart0_puts_p(f->cmd);
        uart0_puts_p(PSTR("\t\t"));
        uart0_puts_p(f->help);
        uart0_puts_p(PSTR("\r\n"));
    }
}

uint8_t compareCmds(const char* s1, const char* s2) {
    while (*s1 && (*s1 != ' ') && (*s1 == pgm_read_byte(s2))) { s1++, s2++; }
    if (*s1 == ' ') { return pgm_read_byte(s2); }
    return *(const uint8_t*)s1 - pgm_read_byte(s2);
}

//returns a pointer to the first cmd argument.
static char* getFirstArg(char* cmd) {
    while(*cmd && *cmd != ' ') {cmd++;}
    while(*cmd && *cmd == ' ') {cmd++;}
    return cmd;
}

static char buffer[CONSOLE_BUFFER];
static bool echo = true;
static uint8_t len = 0;

void Console::update() {
    uint16_t c = uart0_getc();
    if (c & UART_NO_DATA) {
        
    } else {
        if (c & UART_FRAME_ERROR)       { uart0_puts_p(PSTR("UART Frame Error!\n")); }
        if (c & UART_OVERRUN_ERROR)     { uart0_puts_p(PSTR("UART Overrun Error!\n")); }
        if (c & UART_BUFFER_OVERFLOW)   { uart0_puts_p(PSTR("Buffer overflow error!\n")); }

        if (c <= 255) {
            if (len == 0 && (uint8_t)c == '$') { echo = false; }
            
            if (echo) {
                uart0_putc((uint8_t)c);
                if ((uint8_t)c == '\r') uart0_putc('\n');
            }
            
            if ((uint8_t)c == 0x08 || (uint8_t)c == 0x7F) {
                if (len) buffer[--len] = 0;
            } else {
                if ((uint8_t)c != '\n') buffer[len++] = (uint8_t)c;
            }
            
            if (len == sizeof(buffer) || (uint8_t)c == '\r') {
                buffer[--len] = 0;
                len = 0;
                echo = true;
                for(stringfunc* f = procs; f->cmd; f++) {
                    if (!compareCmds(buffer, f->cmd)) {
                        f->func(getFirstArg(buffer));
                        return;
                    }
                }
                if (*buffer) { printf_P(PSTR("unknown command %s\n"), buffer); }
            }
        }
    }
}

void Console::update_100ms() {

}

void Console::update_1S() {
    if (print_min_max) {
        printf_P(PSTR("ADC 3 min %0000u max %0000u\n"), AdcMega_get_AD_min(3), AdcMega_get_AD_max(3));
        printf_P(PSTR("ADC 4 min %0000u max %0000u\n"), AdcMega_get_AD_min(4), AdcMega_get_AD_max(4));
        printf_P(PSTR("ADC 5 min %0000u max %0000u\n"), AdcMega_get_AD_min(5), AdcMega_get_AD_max(5));
        printf_P(PSTR("ADC 6 min %0000u max %0000u\n"), AdcMega_get_AD_min(6), AdcMega_get_AD_max(6));
        printf_P(PSTR("ADC 7 min %0000u max %0000u\n"), AdcMega_get_AD_min(7), AdcMega_get_AD_max(7));
    }
}



}
