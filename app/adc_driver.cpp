#include "mcu/mcu.h"
#include "adc_driver.h"

#include <stdio.h>
#include <avr/wdt.h>
#include "avr/adc_mega.h"
#include "mcu/timer.h"
#include <math.h>
#include "LiquidCrystalS.h"

namespace app {

static AdcData_t AdcData[PHASES];
static uint8_t cphase;
#define ADC_COUNTS  1024
#define ADC_REF_MV  5000
#define V_RATIO_DEF (double)((ADC_REF_MV/1000.0) / (ADC_COUNTS))

static int16_t adc_raw_current;
static int16_t adc_raw_started;
static double filtered_V_last;
static double filtered_V;
static bool cross_V_last;
static bool cross_VC_check;


// static double sqV;
static double sqrt_V_sum;
static double offset_V;

void AdcDriver::calc_Volts(const uint8_t zcc, const uint8_t tot) {
    uint32_t start = millis();
    while (true) {
        if (AdcMega_get_Done(AdcData[cphase].Channel)) {
            adc_raw_started = AdcMega_get_ADC(AdcData[cphase].Channel);
            if (AdcMega_get_ZC(AdcData[cphase].Channel)) break;
        }
        if (( millis() - start) > tot) break;
    }

    uint8_t zc_counter = 0;        // Used to measure number of times threshold is crossed.
    uint16_t numberOfSamples = 0;   // This is now incremented
    start = millis();
    while ((zc_counter < zcc) && (( millis() - start) < tot)) {
        if (AdcMega_get_Done(AdcData[cphase].Channel)) {
            adc_raw_current = AdcMega_get_ADC(AdcData[cphase].Channel);
            ++numberOfSamples;
            offset_V = offset_V + (( adc_raw_current - offset_V)/ADC_COUNTS);
            filtered_V_last = filtered_V;
            filtered_V = adc_raw_current - offset_V;
            sqrt_V_sum += (filtered_V * filtered_V);
            cross_V_last = cross_VC_check;
            if ( adc_raw_current > adc_raw_started) { cross_VC_check = true; }
            else { cross_VC_check = false; }
            if (numberOfSamples == 1) { cross_V_last = cross_VC_check; }
            if (cross_V_last != cross_VC_check) { zc_counter++; }
        }
    }
    double V_RATIO = AdcData[cphase].Calib * V_RATIO_DEF;
    AdcData[cphase].RMS_Voltage = V_RATIO * sqrt(sqrt_V_sum / numberOfSamples);

    sqrt_V_sum = 0;
}

#define PIN_RELAY   B, 0, H

void AdcDriver::init() {
    DRIVER(PIN_RELAY, OUT);
    OFF(PIN_RELAY);
    cphase = 0;
    AdcData[0] = {3, 253.2, 0.0};
    AdcData[1] = {4, 253.2, 0.0};
    AdcData[2] = {5, 253.2, 0.0};
    AdcData[3] = {6, 253.2, 0.0};
    AdcMega_init();
}

static uint8_t DownCounter;


bool Check_Fault(double cv) {
    if ((cv > 254.0) || (cv < 190.0)) {
        DownCounter = 10;
        OFF(PIN_RELAY);
        return true;
    }
    return false;
}

bool Check_Fault_N(double cv) {
    if (cv > 40.0) {
        DownCounter = 90;
        OFF(PIN_RELAY);
        return true;
    }
    return false;
}




void AdcDriver::update() {
    wdt_reset();
    calc_Volts(10, 200); // 10zc 200ms
    double  Current_Voltage = AdcData[cphase].RMS_Voltage;
    bool fault_phase;
    
    switch (cphase) {
        case 0:
            fault_phase = Check_Fault(Current_Voltage);
            app::LiquidCrystal::setCursor(0 ,0);
        break;
        case 1:
            fault_phase = Check_Fault(Current_Voltage);
            app::LiquidCrystal::setCursor(4 ,0);
        break;
        case 2:
            fault_phase = Check_Fault(Current_Voltage);
            app::LiquidCrystal::setCursor(8 ,0);
        break;
        default:
            fault_phase = Check_Fault_N(Current_Voltage);
            app::LiquidCrystal::setCursor(12 ,0);
            fprintf(stderr, "N");
    }
    
    if (Current_Voltage > 9.0f) {
        fprintf_P(stderr, PSTR("%03.0f"), Current_Voltage);
    } else {
        fprintf_P(stderr, PSTR("%01.1f"), Current_Voltage);
    }
    
    switch (cphase) {
        case 0: app::LiquidCrystal::setCursor(0 ,1); break;
        case 1: app::LiquidCrystal::setCursor(2 ,1); break;
        case 2: app::LiquidCrystal::setCursor(4 ,1); break;
        case 3: app::LiquidCrystal::setCursor(6 ,1); break;
    }
    
    if (fault_phase) fprintf(stderr, "E"); else fprintf(stderr, "+");
    
    ++cphase;
    if (cphase >= PHASES) { cphase = 0; }
}




// resistance at 25 degrees C
#define THERMISTORNOMINAL 10000      
// temp. for nominal resistance (almost always 25 C)
#define TEMPERATURENOMINAL 25   
// the value of the 'other' resistor
#define SERIESRESISTOR 10000    
// The beta coefficient of the thermistor (usually 3000-4000)
#define BCOEFFICIENT 3950


float CalcSteinhart(uint16_t adc) {
    float average = adc;
    average = 1023 / average - 1;
    average = SERIESRESISTOR / average;
    float steinhart;
    steinhart = average / THERMISTORNOMINAL;            // (R/Ro)
    steinhart = log(steinhart);                         // ln(R/Ro)
    steinhart /= BCOEFFICIENT;                          // 1/B * ln(R/Ro)
    steinhart += 1.0 / (TEMPERATURENOMINAL + 273.15);   // + (1/To)
    steinhart = 1.0 / steinhart;                        // Invert
    steinhart -= 273.15;                                // convert to C
    return isnan(steinhart) ? -200.0 : steinhart;
}


static uint8_t redraw = 120;

void AdcDriver::update_1S() {
    
    if (redraw) {
        --redraw;
        if (redraw == 0) {
            app::LiquidCrystal::clear();
            redraw = 120;
        }
    }
    
    if (AdcMega_get_Done(7)) {
        app::LiquidCrystal::setCursor(12 ,1);
        float tc = CalcSteinhart(AdcMega_get_ADC(7));
        if (tc > 70.0f) {
            DownCounter = 99;
            OFF(PIN_RELAY);
        }
        fprintf_P(stderr, PSTR("%02.1f"), tc);    
    }
    
    if (DownCounter > 0) {
        OFF(PIN_RELAY);
        --DownCounter;
    } else {
        ON(PIN_RELAY);
    }
    
    
    app::LiquidCrystal::setCursor(8 ,1);
    fprintf(stderr, "%02u", DownCounter);
}






}
