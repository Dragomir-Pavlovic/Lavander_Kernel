#include <stdio.h>
#include "config.h"

void Emanuel(void) {
    setColor();
    printf(" █████████████████████████████████████████████████████████████████████████████████████████████████████████████\n");
    printf(" █▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒█▒\n");
    printf(" █▒                                                                                                          █▒\n");
    printf(" █▒ THIS IS A LAVANDER KERNEL & SANDBOX CREATED AS A TUTORIAL AND INTRODUCTION ON HOW KERNEL WORKS           █▒\n");
    printf(" █▒                                                                                                          █▒\n");
    printf(" █▒ This is a short tutorial which will explain to you basics of every commands and how any command function █▒\n");
    printf(" █▒ Tutorial is created simply to introduce you in basics of UNIX command and to teach you how to run it     █▒\n");
    printf(" █▒ In a few days you will be able to run any of this command's from your head. Enjoy using it and have fun  █▒\n");
    printf(" █▒                                                                                                          █▒\n"); 
    printf(" █████████████████████████████████████████████████████████████████████████████████████████████████████████████▒\n");
    printf(" █▒                                                                                                          █▒\n");
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
    printf(" █▒ 11. Command 'exit' leaves lavander kernel sandbox                                                        █▒\n");
    printf(" █▒                                                                                                          █▒\n");
    printf(" █████████████████████████████████████████████████████████████████████████████████████████████████████████████▒\n");
    printf(" █▒                                                                                                          █▒\n");
    printf(" █▒ ADDITIONAL COMMANDS FOR LAVANDER KERNEL & SANDBOX                                                        █▒\n");
    printf(" █▒                                                                                                          █▒\n");
    printf(" █▒ Command 'tetris' opens a falling block puzzle game                                                       █▒\n");
    printf(" █▒ Command 'snake' opens a game where snake chase and eats one block and grow up for +1                     █▒\n");
    printf(" █▒ Command 'info' opens an information screen which show information of person who created project          █▒\n");
    printf(" █▒%-106s█▒\n", "FILE SYSTEM COMMANDS (work on Linux and Windows the same way)");
    printf(" █▒%-106s█▒\n", "1. command 'mkdir NAME' creates a directory, nested paths like a/b/c work too");
    printf(" █▒%-106s█▒\n", "2. command 'rmdir NAME' removes an empty directory, 'rm -r NAME' removes it with everything inside");
    printf(" █▒%-106s█▒\n", "3. command 'rm NAME' deletes a single file");
    printf(" █▒%-106s█▒\n", "4. command 'ls' (same as 'dir') lists what is inside the current directory");
    printf(" █▒%-106s█▒\n", "5. command 'cd NAME' enters a directory, 'cd ..' goes one level up, 'cd' alone returns to /");
    printf(" █▒%-106s█▒\n", "6. command 'pwd' prints the directory you are currently in");
    printf(" █▒%-106s█▒\n", "7. command 'touch NAME' creates an empty text file");
    printf(" █▒%-106s█▒\n", "8. command 'cat NAME' prints the contents of a file on the screen");
    printf(" █▒%-106s█▒\n", "9. command 'write NAME TEXT' creates or replaces a file with one line of text");
    printf(" █▒%-106s█▒\n", "10. command 'edit NAME' opens the line editor: a/i/r/d/p/w/q, press h inside for help");
    printf(" █▒%-106s█▒\n", "11. command 'mv A B' moves or renames a file or a directory (for example mv notes.txt docs)");
    printf(" █▒%-106s█▒\n", "Everything you create lives inside the default directory shown by pwd ('/' = lavander_home)");
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
