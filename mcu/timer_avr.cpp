#include "timer.h"

#if defined (__AVR__) || defined (__AVR_ARCH__)

#include <util/atomic.h>

static volatile bool elapsed_100ms = false;
static volatile bool elapsed_1_sec = false;
static volatile uint8_t  counter_10int = 0;
static volatile uint8_t  counter_100int = 0;
static volatile uint8_t  counter_10int_sec = 0;
static volatile uint32_t timer0_millis = 0;
static volatile uint32_t timer0_uptime_s = 0;

ISR(TIMER0_COMPA_vect) {
    counter_10int++;
    if (counter_10int >= 10) {
        counter_10int = 0;
        ++timer0_millis;    // millis
        ++counter_100int;
        if (counter_100int >= 100) {
            counter_100int = 0;
            elapsed_100ms = true;
            ++counter_10int_sec;
            if (counter_10int_sec >= 10) {
                counter_10int_sec = 0;
                ++timer0_uptime_s;  // sec
                elapsed_1_sec = true;
            }
        }
    }
}

void millis_begin() {
    cli();
    // http://www.8bit-era.cz/arduino-timer-interrupts-calculator.html
    // TIMER 0 for interrupt frequency 10 000 Hz:
    TCCR0A = 0; // set entire TCCR0A register to 0
    TCCR0B = 0; // same for TCCR0B
    TCNT0  = 0; // initialize counter value to 0
    // set compare match register for 10000 Hz increments
    OCR0A = 199; // = 16000000 / (8 * 10000) - 1 (must be <256)
    // turn on CTC mode
    TCCR0B |= (1 << WGM01);
    // Set CS02, CS01 and CS00 bits for 8 prescaler
    TCCR0B |= (0 << CS02) | (1 << CS01) | (0 << CS00);
    // enable timer compare interrupt
    TIMSK0 |= (1 << OCIE0A);
    sei(); // allow interrupts
}

uint32_t millis() {
    uint32_t r;
    ATOMIC_BLOCK(ATOMIC_FORCEON) { r = timer0_millis; }
    return r;
}

uint32_t uptime_sec() {
    uint32_t r;
    ATOMIC_BLOCK(ATOMIC_FORCEON) { r = timer0_uptime_s; }
    return r;
}

Elapsed_Int_t elapsed_time() {
    if (elapsed_100ms) {
        elapsed_100ms = false;
        return ELAPSED_100ms;
    }
    if (elapsed_1_sec) {
        elapsed_1_sec = false;
        return ELAPSED_1S;
    }
    return NOT_ELAPSED;
}

#endif
