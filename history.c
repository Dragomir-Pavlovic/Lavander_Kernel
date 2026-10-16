#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "config.h"

static char command_history[MAX_HISTORY][MAX_INPUT];
static size_t command_history_count = 0;

void add_history(const char *command) {
    if (command == NULL || command[0] == '\0')
        return;

    if (command_history_count < MAX_HISTORY) {
        snprintf(command_history[command_history_count], MAX_INPUT, "%s", command);
        command_history_count++;
        return;
    }

    memmove(command_history, command_history + 1,
            (MAX_HISTORY - 1) * sizeof(command_history[0]));
    snprintf(command_history[MAX_HISTORY - 1], MAX_INPUT, "%s", command);
}

void history_of_last_five_minute_commands(void) {
    size_t index;

    if (command_history_count == 0) {
        printf("Command history is empty.\n");
        return;
    }

    for (index = 0; index < command_history_count; index++)
        printf("%zu  %s\n", index + 1, command_history[index]);
}
