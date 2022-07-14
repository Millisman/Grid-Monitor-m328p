#include "timer.h"

#if defined (__AVR__) || defined (__AVR_ARCH__)

#else

#include <iostream>
#include <string>
#include <sys/time.h>
#include <unistd.h>

static uint64_t app_start_ms = 0;
static uint64_t last_onesec = 0;

uint64_t _internal_current_ms_stamp() {
    struct timeval te;
    gettimeofday(&te, NULL); // get current time
    return te.tv_sec*1000LL + te.tv_usec/1000; // calculate milliseconds;
}

void millis_begin() {
    std::cout << __FILE__ ":" << __LINE__ << ":" << __FUNCTION__ << "()\n";
    app_start_ms = _internal_current_ms_stamp();
    last_onesec = app_start_ms;
}

uint32_t millis() {
    return _internal_current_ms_stamp() - app_start_ms;
}

uint32_t uptime_sec() {
    return (_internal_current_ms_stamp() - app_start_ms) / 1000;
}

bool elapsed_one_sec() {
    if (_internal_current_ms_stamp() - last_onesec > 1000) {
        last_onesec = _internal_current_ms_stamp();
        return true;
    }
    return false;
}

#endif
