#include <stdio.h>
#include <stdlib.h>
#include "config.h"
#include "platform.h"

void setColor(void) {
    platform_set_color();
}

void resetColor(void) {
    platform_reset_color();
}

void clearScreen(void) {
    platform_clear_screen();
    setColor();
}
