#include "mcu.h"

#if defined (__AVR__) || defined (__AVR_ARCH__)

#else

#include <iostream>
#include <string>

char     pgm_read_byte(const char * _address_short)     { return *_address_short; }
uint8_t  pgm_read_byte(const uint8_t * _address_short)  { return *_address_short; }
uint16_t pgm_read_word(const uint16_t * _address_short) { return *_address_short; }
uint32_t pgm_read_dword(const uint32_t * _address_short){ return *_address_short; }

void _test_eeprom_reset() {
    std::cout << __FILE__ << __LINE__ << __FUNCTION__ << '\n';
}

void eeprom_write_block( const void * src, void* dst, size_t n ) {
    (void)src;
    (void)dst;
    (void)n;
    std::cout << __FILE__ << __LINE__ << __FUNCTION__ << '\n';
}

void eeprom_read_block(  void * dest, const void * source, size_t n ) {
    (void)source;
    (void)dest;
    (void)n;
    std::cout << __FILE__ << __LINE__ << __FUNCTION__ << '\n';
}

void wdt_forceRestart() {
    std::cout << __FILE__ << __LINE__ << __FUNCTION__ << '\n';
}

#endif
