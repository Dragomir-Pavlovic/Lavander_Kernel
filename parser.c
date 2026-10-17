#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"
#include "fs.h"

void process_command(const char *input, int *running) {
    /* Filesystem commands are handled by their own module. */
    if (fs_handle_command(input))
        return;

    if (strcmp(input, "help") == 0) {
        clearScreen();
        showHelp();
    }
    else if (strcmp(input, "clear") == 0) {
        clearScreen();
    }
    else if (strcmp(input, "man") == 0) {
        clearScreen();
        Emanuel();
    }
    else if (strcmp(input, "calc") == 0) {
        calculator();
    }
    else if (strcmp(input, "date") == 0) {
        date_and_time_command(1, 0);
    }
    else if (strcmp(input, "time") == 0) {
        date_and_time_command(0, 1);
    }
    else if (strcmp(input, "hist") == 0) {
        history_of_last_five_minute_commands();
    }
    else if (strcmp(input, "ktop") == 0) {
        showcase_of_working_proces_in_background();
    }
    else if (strcmp(input, "free") == 0) {
        free_memory_in_well_memory_ofc();
    }
    else if (strcmp(input, "bbtbs") == 0) {
        bbtbs();
    }
    else if (strcmp(input, "pake") == 0) {
	    pake();
	    bootScreen();
    }
    else if (strncmp(input, "echo ", 5) == 0) {
        printf("%s\n", input + 5);
    }
    else if (strcmp(input, "games") == 0) {
        GameSelectionScreenWithDefault(NULL);
        bootScreen();
    }
    else if (strcmp(input, "tetris") == 0 || strcmp(input, "snake") == 0 ||
             strcmp(input, "pake") == 0 || strcmp(input, "wildbound") == 0) {
        GameSelectionScreenWithDefault(input);
        bootScreen();
    }
    else if (strcmp(input, "info") == 0) {
        char q_info[8];

        setColor();
        printf("███████████████████████████████████████████████████████\n");
        printf("█▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒█▒\n");
        printf("█▒ Project made by: Dragomir Pavlovic                 █▒\n");
        printf("█▒ To contact creator: dragomirpavlovic20@gmail.com   █▒\n");
        printf("█▒ Project: TINY KERNEL AND FIELD COMPUTER            █▒\n");
        printf("█▒ To exit press 'q'                                  █▒\n");
        printf("███████████████████████████████████████████████████████▒\n");
        printf(" ▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\n");

        while (1) {
            if (fgets(q_info, sizeof(q_info), stdin) == NULL)
                break;
            q_info[strcspn(q_info, "\n")] = 0;
            if (strcmp(q_info, "q") == 0 || strcmp(q_info, "Q") == 0)
                break;
        }

        clearScreen();
        shell_interface();
    }
    else if (strcmp(input, "panic") == 0) {
        clearScreen();
        BSP();
    }
    else if (strcmp(input, "exit") == 0 || strcmp(input, "leave") == 0) {
        clearScreen();
        setColor();
        printf("Exiting Kernel\n");
        printf("Or stopping it for safety reasons\n");
        *running = 0;
    }
    else {
        setColor();
        printf("INVALID COMMAND. Type 'help'.\n");
        resetColor();
    }
}
