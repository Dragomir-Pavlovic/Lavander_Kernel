#include <stdio.h>
#include <stdlib.h>
#include "config.h"

void setColor(void) {
#ifdef _WIN32
    /* Windows kod po potrebi */
#else
    printf("\x1b[35m");
#endif
}

void resetColor(void) {
#ifdef _WIN32
    /* Windows kod po potrebi */
#else
    /* Zadržano ponašanje iz originalnog programa. */
    printf("\x1b[35m");
#endif
}

void clearScreen(void) {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
    setColor();
}
