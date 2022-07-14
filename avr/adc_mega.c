#include "../mcu/mcu.h"
#include "adc_mega.h"

/*  case ADC_1V1: // 1v1    ADM*UX |= ((1 << REFS1) | (1 << REFS0));
    case ADC_AREF: // ADC_AREF (default)    ADMUX &= ~((1 << REFS1) | (1 << REFS0));
    case ADC_VCC: // ADC_VCC    ADMUX &= ~(1 << REFS1); ADMUX |= (1 << REFS0);
*/

#define ADC_REFERENCE REFS0

extern uint32_t timer0_millis;

typedef struct {
    uint16_t    adc_value;
    uint16_t    adc_value_max;
    uint16_t    adc_value_min;
    uint16_t    adc_peak_p;
    uint16_t    adc_peak_n;
    bool        adc_peak_tripped;
    bool        adc_adone;
    bool        adc_in_zero;
} AdcBody;


static volatile uint8_t  _ad_ch;
static volatile uint16_t _ad_val;

// L3 current  0
// L2 current  1
// L1 current  2
// L1 voltage  3
// L2 voltage  4
// L3 voltage  5
// PEN voltage 6
// TEMP        7

#define ADC_L3_I    0
#define ADC_L2_I    1
#define ADC_L1_I    2
#define ADC_L1_V    3
#define ADC_L2_V    4
#define ADC_L3_V    5
#define ADC_PENV    6
#define ADC_TEMP    7

// #define ADC_CHANNELS        8
#define ADC_COUNTS          1024
#define ADC_ZEROCROSS       511
#define ADC_ZEROCROSS_P     (ADC_ZEROCROSS + 30)
#define ADC_ZEROCROSS_N     (ADC_ZEROCROSS - 30)

#define ADC_COUNTS_PEAK_P   (ADC_COUNTS - 10)
#define ADC_COUNTS_PEAK_N   (10)

#define ADC_MAX     7
#define ADC_START   3

static volatile AdcBody AdChannel[ADC_MAX + 1];

ISR(ADC_vect) {
    _ad_ch      = ADMUX & 0x07;
    _ad_val     = ADCW;
    AdChannel[_ad_ch].adc_value = _ad_val;
    if (AdChannel[_ad_ch].adc_value_max < _ad_val) { AdChannel[_ad_ch].adc_value_max = _ad_val; }
    if (AdChannel[_ad_ch].adc_value_min > _ad_val) { AdChannel[_ad_ch].adc_value_min = _ad_val; }
    
    AdChannel[_ad_ch].adc_adone = 1;

    if ((_ad_val > AdChannel[_ad_ch].adc_peak_p) || (_ad_val < AdChannel[_ad_ch].adc_peak_n)) {
        AdChannel[_ad_ch].adc_peak_tripped = 1;
    }

    AdChannel[_ad_ch].adc_in_zero = ((_ad_val > ADC_ZEROCROSS_N) && (_ad_val < ADC_ZEROCROSS_P));

    ++_ad_ch;
    if (_ad_ch > ADC_MAX) _ad_ch = ADC_START;
    ADMUX = (1 << ADC_REFERENCE) | _ad_ch;
    ADCSRA |= (1 << ADSC); // Start new conversion

}

void AdcMega_StructReset(const uint8_t j) {
    AdChannel[j].adc_value   = ADC_ZEROCROSS;
    AdChannel[j].adc_value_max = 0;
    AdChannel[j].adc_value_min = ADC_COUNTS;
    AdChannel[j].adc_in_zero = 1;
    AdChannel[j].adc_adone   = false;
    AdChannel[j].adc_peak_tripped = false;
    AdChannel[j].adc_peak_n = ADC_COUNTS_PEAK_N;
    AdChannel[j].adc_peak_p = ADC_COUNTS_PEAK_P;

}


// ADCSRA = ADEN ADSC ADATE ADIF ADIE ADPS2 ADPS1 ADPS0
// ADPS2  ADPS1   ADPS0 Division Factor
//  0       0       0       2
//  0       0       1       2
//  0       1       0       4
//  0       1       1       8
//  1       0       0       16
//  1       0       1       32
//  1       1       0       64
//  1       1       1       128

void AdcMega_init() {
    for (uint8_t j = ADC_START; j < ADC_MAX + 1; ++j) {
        AdcMega_StructReset(j);
    }
    ADCSRA = (1 << ADEN) | (1 << ADIE) | (1 << ADPS2) | (1 << ADPS1);                // speed (clk/64), interrupt enabled
    // ADCSRA = (1 << ADEN) | (1 << ADIE) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // slow speed (clk/128), interrupt enabled
    ADMUX  = (1 << ADC_REFERENCE) | ADC_START;
    ADCSRA |= (1 << ADSC); // Start conversion
}

uint8_t AdcMega_get_Done(const uint8_t channel) {
    uint8_t old_SREG = SREG;
    cli();
    uint8_t ret = AdChannel[channel].adc_adone;
    SREG = old_SREG;
    return ret;
}

uint16_t AdcMega_get_ADC(const uint8_t channel) {
    uint8_t old_SREG = SREG;
    cli();
    uint16_t ret = AdChannel[channel].adc_value;
    AdChannel[channel].adc_adone = 0;
    SREG = old_SREG;
    return ret;
}

uint8_t AdcMega_get_ZC(const uint8_t channel) {
    uint8_t old_SREG = SREG;
    cli();
    uint8_t ret = AdChannel[channel].adc_in_zero;
    SREG = old_SREG;
    return ret;
}

bool AdcMega_get_peak_trip(const uint8_t channel) {
    uint8_t old_SREG = SREG;
    cli();
    bool ret = AdChannel[channel].adc_peak_tripped;
    AdChannel[channel].adc_peak_tripped = false;
    SREG = old_SREG;
    return ret;
}



uint16_t AdcMega_get_AD_max(const uint8_t channel) {
    uint8_t old_SREG = SREG;
    cli();
    uint16_t ret = AdChannel[channel].adc_value_max;
    AdChannel[channel].adc_value_max = 0;
    SREG = old_SREG;
    return ret;
}

uint16_t AdcMega_get_AD_min(const uint8_t channel) {
    uint8_t old_SREG = SREG;
    cli();
    uint16_t ret = AdChannel[channel].adc_value_min;
    AdChannel[channel].adc_value_min = ADC_COUNTS;
    SREG = old_SREG;
    return ret;
}
