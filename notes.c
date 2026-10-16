#include <stdio.h>
#include "config.h"

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
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
            resetColor();
            return;
        }
    }

    printf("\nContinuing tutorial...\n");
    resetColor();
}
