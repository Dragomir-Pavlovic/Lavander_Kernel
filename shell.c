#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include <sys/stat.h>

#define MAX_INPUT 255
#define MAX_HISTORY 50

void bootScreen(void);
void loadingScreen(void);
void GameSelectionScreen(void);
void GameSelectionScreenWithDefault(const char *default_input);

void setColor(void) {
#ifdef _WIN32
    /* Windows kod po potrebi */
#else
    printf("\x1b[35m"); // ljubicasta boja
#endif
}

void resetColor(void) {
#ifdef _WIN32
    /* Windows kod po potrebi */
#else
    printf("\x1b[35m"); // ISPRAVLJENO: Standardni reset boje na podrazumevanu
#endif
}

int shell_interface(void) {
    setColor();
    printf("█████████████████████████████████████████████████████████████████████\n");
    printf("█▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒█▒\n");
    printf("█▒ - LITTLE PENGUIN KERNEL FOR A FIELD USE (OFFLINE) -              █▒\n");
    printf("█▒  To continue to shell type 'help' for help or 'man' for tutorial █▒\n");
    printf("█▒  To open a games type 'games' to leave game press 'q'            █▒\n");
    printf("█▒  To open a penguin game type 'pake' to leave game press 'q'      █▒\n");
    printf("█▒  To open information on program type 'info' to leave type 'q'    █▒\n");
    printf("█▒  To leave shell type 'leave'                                     █▒\n");
    printf("█████████████████████████████████████████████████████████████████████▒\n");
    printf(" ▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\n");
    resetColor();
    return 0;
}

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

static char command_history[MAX_HISTORY][MAX_INPUT];
static size_t command_history_count = 0;

static void add_history(const char *command) {
    if (command == NULL || command[0] == '\0')
        return;
    if (command_history_count < MAX_HISTORY) {
        snprintf(command_history[command_history_count], MAX_INPUT, "%s", command);
        command_history_count++;
        return;
    }
    memmove(command_history, command_history + 1, (MAX_HISTORY - 1) * sizeof(command_history[0]));
    snprintf(command_history[MAX_HISTORY - 1], MAX_INPUT, "%s", command);
}

typedef struct {
    const char *current;
    int error;
    int division_by_zero;
} ExpressionParser;

static double parse_expression(ExpressionParser *parser);

static double parse_factor(ExpressionParser *parser) {
    char *end;
    double value;
    while (isspace((unsigned char)*parser->current))
        parser->current++;
    if (*parser->current == '+' || *parser->current == '-') {
        int negative = *parser->current == '-';
        parser->current++;
        value = parse_factor(parser);
        return negative ? -value : value;
    }
    if (*parser->current == '(') {
        parser->current++;
        value = parse_expression(parser);
        while (isspace((unsigned char)*parser->current))
           pointer_space: parser->current++;
        if (*parser->current != ')')
            parser->error = 1;
        else
            parser->current++;
        return value;
    }
    value = strtod(parser->current, &end);
    if (end == parser->current) {
        parser->error = 1;
        return 0.0;
    }
    parser->current = end;
    return value;
}

static double parse_term(ExpressionParser *parser) {
    double value = parse_factor(parser);
    while (!parser->error) {
        char operation;
        double next_value;
        while (isspace((unsigned char)*parser->current))
            parser->current++;
        operation = *parser->current;
        if (operation != '*' && operation != '/')
            break;
        parser->current++;
        next_value = parse_factor(parser);
        if (operation == '*') {
            value *= next_value;
        } else if (next_value == 0.0) {
            parser->division_by_zero = 1;
            parser->error = 1;
        } else {
            value /= next_value;
        }
    }
    return value;
}

static double parse_expression(ExpressionParser *parser) {
    double value = parse_term(parser);
    while (!parser->error) {
        char operation;
        double next_value;
        while (isspace((unsigned char)*parser->current))
            parser->current++;
        operation = *parser->current;
        if (operation != '+' && operation != '-')
            break;
        parser->current++;
        next_value = parse_term(parser);
        if (operation == '+')
            value += next_value;
        else
            value -= next_value;
    }
    return value;
}

static int evaluate_expression(const char *input, double *result, int *division_by_zero) {
    ExpressionParser parser = { input, 0, 0 };
    *result = parse_expression(&parser);
    while (isspace((unsigned char)*parser.current))
        parser.current++;
    *division_by_zero = parser.division_by_zero;
    return !parser.error && *parser.current == '\0';
}

void clearScreen(void) {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
    setColor();
}

void GameSelectionScreenWithDefault(const char *default_input) {
    char input[MAX_INPUT];
    char q[8];
    int fast_forward = (default_input != NULL && default_input[0] != '\0');

    while (1) {
        if (!fast_forward) {
            clearScreen();
            setColor();

            printf("██████████████████████████████████████████████████████████████▒\n");
            printf("█▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒█▒\n");
            printf("█▒                                                           █▒\n");
            printf("█▒  ████████████  CASUAL GAME    ████████████  CASUAL GAME   █▒\n");
            printf("█▒  █          █  OF FALLING     █          █  OF SNAKE      █▒\n");
            printf("█▒  █  TETRIS  █  BLOCKS PUZZLE  █  SNAKE   █  EATING APPLE  █▒\n");
            printf("█▒  █          █                 █          █                █▒\n");
            printf("█▒  ████████████  TYPE: PUZZLE   ████████████  TYPE: PUZZLE  █▒\n");
            printf("█▒                                                           █▒\n");
            printf("█▒  ████████████  FANTASY WORLD     ╔══════════════════╗     █▒\n");
            printf("█▒  █          █  SET IN              GAME SELECTION:        █▒\n");
            printf("█▒  █ WILDBOUND█  LONGIOBIOBOR                               █▒\n");
            printf("█▒  █          █                      BY PRESSING 1, 2       █▒\n");
            printf("█▒  ████████████  TYPE: FANTASY       OR 3 USER SELECT       █▒\n");
            printf("█▒                                    A GAME OF CHOICE       █▒\n");
            printf("█▒                                  ╚══════════════════╝     █▒\n");
            printf("█▒                                                           █▒\n");
            printf("██████████████████████████████████████████████████████████████▒\n");
            printf(" ▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\n");
            printf("[CORE-16] >> ");
            fflush(stdout);

            if (fgets(input, MAX_INPUT, stdin) == NULL) {
                clearScreen();
                return;
            }
            input[strcspn(input, "\n")] = 0;
        } else {
            strncpy(input, default_input, MAX_INPUT);
            fast_forward = 0; 
        }

        if (strcmp(input, "q") == 0 || strcmp(input, "Q") == 0) {
            clearScreen();
            return;
        }

        if (strcmp(input, "1") == 0 || strcmp(input, "tetris") == 0 ||
            strcmp(input, "2") == 0 || strcmp(input, "snake") == 0 || strcmp(input, "pake") == 0 ||
            strcmp(input, "3") == 0 || strcmp(input, "wildbound") == 0 || strcmp(input, "longiobiobor") == 0 || strcmp(input, "longiobor") == 0) {
            
            clearScreen();
            setColor();

            if (strcmp(input, "1") == 0 || strcmp(input, "tetris") == 0) {
                printf("██████████████████████████▒\n");
                printf("█▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒█▒\n");
                printf("█▒                       █▒\n");
                printf("█▒  TETRIS IS LOADING    █▒\n");
                printf("█▒                       █▒\n");
                printf("█▒  PLEASE WAIT          █▒\n");
                printf("█▒                       █▒\n");
                printf("██████████████████████████▒\n");
                printf(" ▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\n");
        }
        else if (strcmp(input, "2") == 0 || strcmp(input, "snake") == 0 || strcmp(input, "pake") == 0) {
                printf("██████████████████████████▒\n");
                printf("█▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒█▒\n");
                printf("█▒                       █▒\n");
                printf("█▒  SNAKE IS LOADING     █▒\n");
                printf("█▒                       █▒\n");
                printf("█▒  PLEASE WAIT          █▒\n");
                printf("█▒                       █▒\n");
                printf("██████████████████████████▒\n");
                printf(" ▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\n");
        }
        else if (strcmp(input, "3") == 0 || strcmp(input, "wildbound") == 0 || strcmp(input, "longiobiobor") == 0 || strcmp(input, "longiobor") == 0) {
                printf("██████████████████████████▒\n");
                printf("█▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒█▒\n");
                printf("█▒                       █▒\n");
                printf("█▒  WILDBOUND IS LOADING █▒\n");
                printf("█▒                       █▒\n");
                printf("█▒  PLEASE WAIT          █▒\n");
                printf("█▒                       █▒\n");
                printf("██████████████████████████▒\n");
                printf(" ▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\n");
        }
printf("\n[GAME] Game is not implemented yet.\n");
printf("[GAME] Press 'q' to return to Game Selection.\n");
while (1) {
if (fgets(q, sizeof(q), stdin) == NULL) {
clearScreen();
return;
}
q[strcspn(q, "\n")] = 0;
if (strcmp(q, "q") == 0 || strcmp(q, "Q") == 0)
break;
}
}
else {
printf("\nInvalid selection. Please press 1, 2, 3 or q.\n");
printf("Press Enter to continue...");
getchar();
}
}
}
void GameSelectionScreen(void) {
GameSelectionScreenWithDefault(NULL);
}
void date_and_time_command(int show_date, int show_time) {
time_t current_time = time(NULL);
struct tm *local = localtime(&current_time);
if (local == NULL) {
printf("ERROR: Cannot read system time.\n");
return;
}
if (show_date) {
printf("Date: %04d-%02d-%02d\n", local->tm_year + 1900, local->tm_mon + 1, local->tm_mday);
}
if (show_time) {
printf("Time: %02d:%02d:%02d\n", local->tm_hour, local->tm_min, local->tm_sec);
}
}
static void calculator_expression(void) {
char input[MAX_INPUT];
printf("Calculator\n");
printf("Enter function (q for exit).\n");
while (1) {
double result;
int division_by_zero;
printf("calc> ");
if (fgets(input, sizeof(input), stdin) == NULL)
return;
input[strcspn(input, "\n")] = '\0';
if (strcmp(input, "q") == 0 || strcmp(input, "Q") == 0 || strcmp(input, "quit") == 0) {
return;
}
if (input[0] == '\0')
continue;
if (evaluate_expression(input, &result, &division_by_zero)) {
printf("= %.10g\n", result);
} else if (division_by_zero) {
printf("[ERROR]: dividing by 0 is not possible.\n");
} else {
printf("[ERROR]: bad input. Use numbers and operators + - * /.\n");
}
}
}
void calculator(void) {
calculator_expression();
}
void history_of_last_five_minute_commands(void) {
size_t index;
if (command_history_count == 0) {
printf("Command history is empty.\n");
return;
}
for (index = 0; index < command_history_count; index++) {
printf("%zu  %s\n", index + 1, command_history[index]);
}
}
void showcase_of_working_proces_in_background(void) {
time_t t = time(NULL);
struct tm *tm_info = localtime(&t);
setColor();
printf("\n");
printf(" █████████████████████████████████████████████████████████████████████████████████████████████████████████████\n");
printf(" █▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒█▒\n");
printf(" █▒                                                                                                         ▒█▒\n");
printf(" █▒  ██╗     ██╗████████╗████████╗██╗     ███████╗██████╗ ███████╗███╗   ██╗ ██████╗ ██╗   ██╗██╗███╗   ██╗ ▒█▒\n");
printf(" █▒  ██║     ██║╚══██╔══╝╚══██╔══╝██║     ██╔════╝██╔══██╗██╔════╝████╗  ██║██╔════╝ ██║   ██║██║████╗  ██║ ▒█▒\n");
printf(" █▒  ██║     ██║   ██║      ██║   ██║     █████╗  ██████╔╝█████╗  ██╔██╗ ██║██║  ███╗██║   ██║██║██╔██╗ ██║ ▒█▒\n");
printf(" █▒  ██║     ██║   ██║      ██║   ██║     ██╔══╝  ██╔═══╝ ██╔══╝  ██║╚██╗██║██║   ██║██║   ██║██║██║╚██╗██║ ▒█▒\n");
printf(" █▒  ███████╗██║   ██║      ██║   ███████╗███████╗██║     ███████╗██║ ╚████║╚██████╔╝╚██████╔╝██║██║ ╚████║ ▒█▒\n");
printf(" █▒  ╚══════╝╚═╝   ╚═╝      ╚═╝   ╚══════╝╚══════╝╚═╝     ╚══════╝╚═╝  ╚═══╝ ╚═════╝  ╚═════╝ ╚═╝╚═╝  ╚═══╝ ▒█▒\n");
printf(" █▒                                                                                                         ▒█▒\n");
printf(" █▒          ██████╗ ██╗      ██╗      ██╗  ██╗ ███████╗ ██████╗  ███╗   ██╗ ███████╗ ██╗                   ▒█▒\n");
printf(" █▒         ██╔════╝ ██║      ██║      ██║ ██╔╝ ██╔════╝ ██╔══██║ ████╗  ██║ ██╔════╝ ██║                   ▒█▒\n");
printf(" █▒   █████╗██║      ██║      ██║      █████╔╝  █████╗   █████╔═╝ ██╔██╗ ██║ █████╗   ██║                   ▒█▒\n");
printf(" █▒   ╚════╝██║      ██║      ██║      ██║ ██╗  ██╔══╝   ██╔╗██╗  ██║╚██╗██║ ██╔══╝   ██║                   ▒█▒\n");
printf(" █▒         ╚██████╗ ███████╗ ██║      ██║  ██╗ ███████╗ ██║╚╗██╗ ██║ ╚████║ ███████╗ ███████╗              ▒█▒\n");
printf(" █▒          ╚═════╝ ╚══════╝ ╚═╝      ╚═╝  ╚═╝ ╚══════╝ ╚═╝ ╚══╝ ╚═╝  ╚═══╝ ╚══════╝ ╚══════╝              ▒█▒\n");
printf(" █████████████████████████████████████████████████████████████████████████████████████████████████████████████▒\n");
printf("  ▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\n");
FILE *f = fopen("/etc/os-release", "r");
char line[256];
char os_name[128] = "Linux";
if (f) {
while (fgets(line, sizeof(line), f)) {
if (strncmp(line, "PRETTY_NAME=", 12) == 0) {
char *p = line + 12;
if (*p == '"') p++;
strncpy(os_name, p, sizeof(os_name) - 1);
os_name[strcspn(os_name, "\n")] = 0; // ISPRAVLJENO: uklonjeni dupli navodnici ""\n"
break;
}
}
fclose(f);
}
printf("  OS:          %s\n", os_name);
f = fopen("/proc/version", "r");
char kernel[128] = "Unknown";
if (f) {
if (fgets(line, sizeof(line), f)) {
sscanf(line, "Linux version %127s", kernel);
}
fclose(f);
}
printf("  Kernel:      %s\n", kernel);
printf("  Shell:       LAVANDER SHELL\n");
f = popen("uname -m", "r");
char arch[64] = "Unknown";
if (f) {
if (fgets(arch, sizeof(arch), f))
arch[strcspn(arch, "\n")] = 0;
pclose(f);
}
printf("  Architecture: %s\n", arch);
if (tm_info != NULL) {
printf("  Uptime:      %02d:%02d:%02d\n", tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec);
printf("  Date:        %02d-%02d-%d\n", tm_info->tm_mday, tm_info->tm_mon + 1, tm_info->tm_year + 1900);
}
f = fopen("/proc/meminfo", "r");
unsigned long long total = 0;
unsigned long long available = 0;
if (f) {
while (fgets(line, sizeof(line), f)) {
if (sscanf(line, "MemTotal: %llu kB", &total) == 1) continue;
if (sscanf(line, "MemAvailable: %llu kB", &available) == 1) continue;
}
fclose(f);
}
if (total > 0) {
printf("  Memory:      %llu MB / %llu MB\n", (total - available) / 1024, total / 1024);
} else {
printf("  Memory:      Unknown\n");
}
f = fopen("/proc/cpuinfo", "r");
char cpu[128] = "Unknown";
if (f) {
while (fgets(line, sizeof(line), f)) {
if (strncmp(line, "model name", 10) == 0) {
char *p = strchr(line, ':');
if (p) {
p++;
while (*p == ' ') p++;
strncpy(cpu, p, sizeof(cpu) - 1);
cpu[strcspn(cpu, "\n")] = 0;
break;
}
}
}
fclose(f);
}
printf("  CPU:         %s\n", cpu);
printf("  Status:      Running\n\n");
resetColor();
}
void Emanuel(void) {
setColor();
printf(" █████████████████████████████████████████████████████████████████████████████████████████████████████████████\n");
printf(" █▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒█▒\n");
printf(" █▒ THIS IS A TUTORIAL ON BASH SCRIPT THAT ALSO RUN WITH IT AN SLM MODEL                                     █▒\n");
printf(" █▒ 1. command 'help' shows the help menu with all commands that you can run                                 █▒\n");
printf(" █▒ 2. Command 'echo' with inserted text next to it repeat text in the new line                              █▒\n");
printf(" █▒ 3. Command 'date / time' shows the date or the time in that moment when you enter it                     █▒\n");
printf(" █▒ 4. Command 'calc' open small calculator that can run specific, small, mathematical calculations          █▒\n");
printf(" █▒ 5. Command 'hist' runs THE history of your previous inserted commands, in case if you lost your track    █▒\n");
printf(" █▒ 6. Command 'man' this short for manual. It opens this tutorial                                           █▒\n");
printf(" █▒ 7. Command 'ktop' opens cool terminal operation proccess (cool with k)                                   █▒\n");
printf(" █▒ 8. Command 'free' shows free memory on your system                                                       █▒\n");
printf(" █▒ 9. Command 'panic' exits program if you make mistake, it stops all functions                             █▒\n");
printf(" █▒ 10. Command 'bbtbs' is short for bring back to beginning screen and it turns you back to start screen    █▒\n");
printf(" █▒ 11. Command 'exit' leaves bash script                                                                    █▒\n");
printf(" █▒ ADDITIONAL COMMANDS FOR BASH SCRIPT                                                                      █▒\n");
printf(" █▒ Command 'tetris' opens a falling block puzzle game                                                       █▒\n");
printf(" █▒ Command 'snake' opens a game where snake chase and eats one block and grow up for +1                     █▒\n");
printf(" █▒ Command 'info' opens an information screen which show information of person who created project          █▒\n");
printf(" █████████████████████████████████████████████████████████████████████████████████████████████████████████████▒\n");
printf("  ▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\n\n");
char firstKey;
char secondKey;
printf("\n[MANUAL] Type q then x to leave: ");
if (scanf(" %c %c", &firstKey, &secondKey) == 2) {
if ((firstKey == 'q' || firstKey == 'Q') && (secondKey == 'x' || secondKey == 'X')) {
printf("\nLeaving tutorial...\n");
// Čišćenje bafera ulaza nakon skeniranja karaktera
int c;
while ((c = getchar()) != '\n' && c != EOF);
return;
}
}
printf("\nContinuing tutorial...\n");
resetColor();
}
void free_memory_in_well_memory_ofc(void) {
setColor();
#if defined(linux) || defined(linux) // ISPRAVLJENO: Bolja podrška za Linux makroee
FILE *memory_file = fopen("/proc/meminfo", "r");
char line[128];
unsigned long long total = 0;
unsigned long long available = 0;
if (memory_file != NULL) {
while (fgets(line, sizeof(line), memory_file) != NULL) {
if (sscanf(line, "MemTotal: %llu kB", &total) == 1) continue;
if (sscanf(line, "MemAvailable: %llu kB", &available) == 1) continue;
}
fclose(memory_file);
}
if (total > 0) {
printf("██████████████████████████████████████████████\n");
printf("█▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒█▒\n");
printf("█▒                                           █▒\n");
printf("█▒ Memory: %llu MB total, %llu MB available █▒\n", total / 1024, available / 1024);
printf("█▒                                           █▒\n");
printf("█▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒█▒\n");
printf("██████████████████████████████████████████████▒\n");
printf(" ▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\n");
} else {
printf("Memory information is unavailable.\n");
}
#else
printf("Memory information is unavailable on this platform.\n");
#endif
resetColor();
}
void bbtbs(void) {
setColor();
printf("User turned back to beginning of the main screen\n\n");
clearScreen();
bootScreen();
resetColor();
}
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
int main(void) {
setColor();
char input[MAX_INPUT];
loadingScreen();
bootScreen();
while (1) {
setColor();
printf("[CORE-16] >> ");
fflush(stdout);
if (fgets(input, MAX_INPUT, stdin) == NULL)
break;
input[strcspn(input, "\n")] = 0;
add_history(input);
if (strcmp(input, "help") == 0) {
#ifdef _WIN32
system("cls");
#else
system("clear");
#endif
showHelp();
}
else if (strcmp(input, "clear") == 0) {
clearScreen();
}
else if (strcmp(input, "man") == 0) {
#ifdef _WIN32
system("cls");
#else
system("clear");
#endif
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
else if (strncmp(input, "echo ", 5) == 0) {
printf("%s\n", input + 5);
}
else if (strcmp(input, "games") == 0) {
GameSelectionScreenWithDefault(NULL);
bootScreen();
}
else if (strcmp(input, "tetris") == 0 || strcmp(input, "snake") == 0 || strcmp(input, "pake") == 0 || strcmp(input, "wildbound") == 0) {
GameSelectionScreenWithDefault(input);
bootScreen();
}
else if (strcmp(input, "info") == 0) {
setColor();
printf("███████████████████████████████████████████████████████\n");
printf("█▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒█▒\n");
printf("█▒ Project made by: Dragomir Pavlovic                 █▒\n");
printf("█▒ To contact creator: dragomirpavlovic20@gmail.com   █▒\n");
printf("█▒ Project: TINY KERNEL AND FIELD COMPUTER            █▒\n");
printf("█▒ To exit press 'q'                                  █▒\n");
printf("███████████████████████████████████████████████████████▒\n");
printf(" ▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\n");
char q_info[8];
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
#ifdef _WIN32
system("cls");
#else
system("clear");
#endif
BSP();
}
else if (strcmp(input, "exit") == 0 || strcmp(input, "leave") == 0) {
#ifdef _WIN32
system("cls");
#else
system("clear");
#endif
setColor();
printf("Exiting Kernel\n");
printf("Or stopping it for safety reasons\n");
break;
}
else {
setColor();
printf("INVALID COMMAND. Type 'help'.\n");
resetColor();
}
}
resetColor();
return 0;
}

