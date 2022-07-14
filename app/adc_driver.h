#pragma once

#include <stdint.h>

namespace app {

#define   PHASES 4

typedef struct {
    uint8_t Channel;
    double  Calib;
    double  RMS_Voltage;
} AdcData_t;

class AdcDriver {
    static void calc_Volts(const uint8_t zcc, const uint8_t tot);
public:
    static void init();
    static void update();
    static void update_1S();
    // static void update_100ms();
};

}
