#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "config.h"

void BSP(void) {
    setColor();
    printf("\n!!! PANIC !!!\n");
    printf("[ERROR] something happened with system.\n");
    printf("Possible error on user side\n");
    resetColor();
    exit(1);
}

void bootScreen(void) {
    clearScreen();
    shell_interface();
}

void loadingScreen(void) {
    clearScreen();
    setColor();
    printf("\n\n");
    printf(" █████████████████████████████████████\n");
    printf(" █▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒█▒\n");
    printf(" █▒                                  █▒\n");
    printf(" █▒     LITTLE PENGUIN KERNEL        █▒\n");
    printf(" █▒                                  █▒\n");
    printf(" █████████████████████████████████████▒\n");
    printf("  ▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\n\n");
    printf("     Loading");
    fflush(stdout);

    for (int i = 0; i < 5; i++) {
        usleep(400000);
        printf(".");
        fflush(stdout);
    }

    printf("\n");
    usleep(300000);
    resetColor();
}
