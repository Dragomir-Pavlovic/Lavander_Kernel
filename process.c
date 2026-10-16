#include <stdio.h>
#include <string.h>
#include "config.h"

void GameSelectionScreenWithDefault(const char *default_input)
{
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
            printf("█▒              -  GAME SELECTION MENU  -                    █▒\n");
            printf("█▒                                                           █▒\n");
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
            printf("█▒  TO LEAVE GAME PRESS 'q'                                  █▒\n");
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
            strncpy(input, default_input, MAX_INPUT - 1);
            input[MAX_INPUT - 1] = '\0';
            fast_forward = 0;
        }

        if (strcmp(input, "q") == 0 || strcmp(input, "Q") == 0) {
            clearScreen();
            return;
        }

        if (strcmp(input, "1") == 0 || strcmp(input, "tetris") == 0) {
            printf("\n[GAME] Tetris nije implementiran.\n");
            printf("[GAME] Press 'q' to return.\n");
            while (1) {
                if (fgets(q, sizeof(q), stdin) == NULL)
                    return;
                q[strcspn(q, "\n")] = 0;
                if (strcmp(q, "q") == 0 || strcmp(q, "Q") == 0)
                    break;
            }
        }
        else if (strcmp(input, "2") == 0 ||
                 strcmp(input, "snake") == 0 ||
                 strcmp(input, "pake") == 0) {
            pake();
        }
        else if (strcmp(input, "3") == 0 ||
                 strcmp(input, "wildbound") == 0 ||
                 strcmp(input, "longiobiobor") == 0 ||
                 strcmp(input, "longiobor") == 0) {
            printf("\n[GAME] Wildbound nije implementiran.\n");
            printf("[GAME] Press 'q' to return.\n");
            while (1) {
                if (fgets(q, sizeof(q), stdin) == NULL)
                    return;
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

void GameSelectionScreen(void)
{
    GameSelectionScreenWithDefault(NULL);
}
