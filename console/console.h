#pragma once

#include <stdint.h>

namespace app {

class Console {

public:
    static void init();
    static void update();
    static void update_1S();
    static void update_100ms();

};

void cmd_cd(char* args);
void cmd_uptime(char* args);
void cmd_reboot(char* args);
void cmd_help(char* args);

}
