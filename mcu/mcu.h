#pragma once

#if defined (__AVR__) || defined (__AVR_ARCH__)

#ifndef F_CPU
#define F_CPU   16000000UL
#endif

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <avr/wdt.h>
#include "pin_macros.h"
#include "../avr/adc_mega.h"

#else

#define F_CPU   20000000UL

#define PSTR
#define SEI
#define PROGMEM
#define memcpy_P memcpy
#define strstr_P strstr
#define strcasestr_P strcasestr
#define EEMEM
#define strlen_P strlen
#include <bits/stdint-uintn.h>
#include <functional>
char pgm_read_byte(const char * _address_short);
uint8_t  pgm_read_byte(const uint8_t * _address_short);
uint16_t pgm_read_word(const uint16_t * _address_short);
uint32_t pgm_read_dword(const uint32_t * _address_short);

#define EEPROM_SIZE 2048

void _test_eeprom_reset();
void eeprom_write_block( const void * src, void* dst, size_t n );
void eeprom_read_block(  void * dest, const void * source, size_t n );

#endif

#include "timer.h"

void wdt_forceRestart();
