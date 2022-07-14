#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <util/atomic.h>

// L3 current  0
// L2 current  1
// L1 current  2
// L1 voltage  3
// L2 voltage  4
// L3 voltage  5
// PEN voltage 6
// TEMP        7

typedef enum: uint8_t {
      L3_Current  = 0
    , L2_Current  = 1
    , L1_Current  = 2
    , L1_Voltage  = 3
    , L2_Voltage  = 4
    , L3_Voltage  = 5
    , PEN_Voltage = 6
    , TEMP        = 7
} AdcMegaIn;

void AdcMega_init();
void AdcMega_StructReset(const uint8_t in);
uint8_t AdcMega_get_Done(const uint8_t channel);
uint16_t AdcMega_get_ADC(const uint8_t channel);
uint8_t AdcMega_get_ZC(const uint8_t channel);

bool     AdcMega_get_peak_trip(const uint8_t channel);
uint16_t AdcMega_get_AD_max(const uint8_t channel);
uint16_t AdcMega_get_AD_min(const uint8_t channel);
