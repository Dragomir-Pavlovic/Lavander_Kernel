#include <stdio.h>
#include "config.h"

/* Miscellaneous / TODO-style shell command kept separate for future expansion. */
void bbtbs(void) {
    setColor();
    printf("User turned back to beginning of the main screen\n\n");
    clearScreen();
    bootScreen();
    resetColor();
}
