#include <stdio.h>
#include <time.h>
#include "config.h"

void showHelp(void) {
    setColor();
    printf("██████████████████████████████████████████████████████████████████████████████\n");
    printf("█▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒█▒\n");
    printf("█▒COMMAND LIST:                                                              █▒\n");
    printf("█▒help              - Show command list                                      █▒\n");
    printf("█▒clear             - Clear screen                                           █▒\n");
    printf("█▒echo TEXT         - Print text                                             █▒\n");
    printf("█▒date / time       - show date or time                                      █▒\n");
    printf("█▒calc              - calculator                                             █▒\n");
    printf("█▒hist              - show commands entered in last 5 minutes                █▒\n");
    printf("█▒man               - manual for detail explanation of commands              █▒\n");
    printf("█▒ktop              - shows what kernel it is and which version              █▒\n");
    printf("█▒free              - showcase of free memory in partition                   █▒\n");
    printf("█▒panic             - Turns off bash script if user or system makes an error █▒\n");
    printf("█▒bbtbs             - Bringing back user to beginning screen of bash script  █▒\n");
    printf("█▒exit              - Shutdown                                               █▒\n");
    printf("██████████████████████████████████████████████████████████████████████████████▒\n");
    printf(" ▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\n");
    resetColor();
}

void date_and_time_command(int show_date, int show_time) {
    time_t current_time = time(NULL);
    struct tm *local = localtime(&current_time);

    if (local == NULL) {
        printf("ERROR: Cannot read system time.\n");
        return;
    }

    if (show_date)
        printf("Date: %04d-%02d-%02d\n", local->tm_year + 1900,
               local->tm_mon + 1, local->tm_mday);

    if (show_time)
        printf("Time: %02d:%02d:%02d\n", local->tm_hour, local->tm_min,
               local->tm_sec);
}
