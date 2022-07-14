#include "mcu.h"

#if defined (__AVR__) || defined (__AVR_ARCH__)

void wdt_forceRestart() {
    wdt_enable(WDTO_15MS);
    while (1) {}
}

#endif
