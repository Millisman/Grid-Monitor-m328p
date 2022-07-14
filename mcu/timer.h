#pragma once

#include <stdint.h>

typedef enum: uint8_t {
    NOT_ELAPSED     = 0,
    ELAPSED_100ms   = 1,
    ELAPSED_1S      = 2    
} Elapsed_Int_t;

Elapsed_Int_t elapsed_time();
void millis_begin();
uint32_t millis();
uint32_t uptime_sec();

