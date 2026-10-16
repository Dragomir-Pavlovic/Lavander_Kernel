#include <stdio.h>
#include <string.h>
#include "config.h"

int main(void) {
    char input[MAX_INPUT];
    int running = 1;

    setColor();
    loadingScreen();
    bootScreen();

    while (running) {
        setColor();
        printf("[CORE-16] >> ");
        fflush(stdout);

        if (fgets(input, MAX_INPUT, stdin) == NULL)
            break;

        input[strcspn(input, "\n")] = 0;
        add_history(input);
        process_command(input, &running);
    }

    resetColor();
    return 0;
}
