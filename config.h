#ifndef LITTLE_PENGUIN_CONFIG_H
#define LITTLE_PENGUIN_CONFIG_H

#include <stddef.h>

#define MAX_INPUT 255
#define MAX_HISTORY 50

void pake(void);
void setColor(void);
void resetColor(void);
void clearScreen(void);

void shell_interface(void);
void showHelp(void);
void date_and_time_command(int show_date, int show_time);
void calculator(void);
void history_of_last_five_minute_commands(void);
void showcase_of_working_proces_in_background(void);
void free_memory_in_well_memory_ofc(void);
void bbtbs(void);
void BSP(void);
void bootScreen(void);
void loadingScreen(void);
void Emanuel(void);

void GameSelectionScreen(void);
void GameSelectionScreenWithDefault(const char *default_input);

void add_history(const char *command);

int evaluate_expression(const char *input, double *result, int *division_by_zero);

void process_command(const char *input, int *running);

#endif
