#include <stdio.h>
#include <stdlib.h>
#include "config.h"
#include "platform.h"

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
    int i;

    clearScreen();
    setColor();
    printf("\n\n");
    printf(" █████████████████████████████████████\n");
    printf(" █▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒█▒\n");
    printf(" █▒                                  █▒\n");
    printf(" █▒   ## -- LAVANDER KERNEL -- ##    █▒\n");
    printf(" █▒                                  █▒\n");
    printf(" █████████████████████████████████████▒\n");
    printf("  ▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\n\n");
    printf("     Loading");
    fflush(stdout);

    for (i = 0; i < 5; i++) {
        platform_sleep_ms(400);
        printf(".");
        fflush(stdout);
    }

    printf("\n");
    platform_sleep_ms(300);
    resetColor();
}
