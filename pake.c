#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <time.h>
#include "config.h"

static int kbhit(void)
{
    struct termios oldt, newt;
    int ch, oldf;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if (ch != EOF) {
        ungetc(ch, stdin);
        return 1;
    }
    return 0;
}

void pake(void)
{
    const int width = 40;
    const int height = 25;
    const char *head = ":D";
    const char *body = "o";
    const char *food = "🮆";
    const char *wall = "█";

    int x = width / 2;
    int y = height / 2;
    int foodX, foodY;
    int score = 0;
    int tailX[400], tailY[400];
    int tailLen = 0;
    enum { STOP = 0, LEFT, RIGHT, UP, DOWN } dir = STOP;
    int gameOver = 0;
    int i, j, k;

    srand((unsigned)time(NULL));
    foodX = rand() % width;
    foodY = rand() % height;

    while (!gameOver) {
        clearScreen();
        setColor();

        printf("Score: %d  |  Q = leave\n", score);

        for (i = 0; i < width + 2; i++)
            printf("%s", wall);
        putchar('\n');

        for (i = 0; i < height; i++) {
            printf("%s", wall);
            for (j = 0; j < width; j++) {
                if (i == y && j == x) {
                    printf("%s", head);
                } else if (i == foodY && j == foodX) {
                    printf("%s", food);
                } else {
                    int printed = 0;
                    for (k = 0; k < tailLen; k++) {
                        if (tailX[k] == j && tailY[k] == i) {
                            printf("%s", body);
                            printed = 1;
                            break;
                        }
                    }
                    if (!printed)
                        putchar(' ');
                }
            }
            printf("%s", wall);
            putchar('\n');
        }

        for (i = 0; i < width + 2; i++)
            printf("%s", wall);
        putchar('\n');
        resetColor();

        if (kbhit()) {
            int c = getchar();
            if (c == 'q' || c == 'Q')
                break;
            if (c == 'a' || c == 'A')
                dir = LEFT;
            if (c == 'd' || c == 'D')
                dir = RIGHT;
            if (c == 'w' || c == 'W')
                dir = UP;
            if (c == 's' || c == 'S')
                dir = DOWN;
        }

        if (tailLen > 0) {
            for (k = tailLen - 1; k > 0; k--) {
                tailX[k] = tailX[k - 1];
                tailY[k] = tailY[k - 1];
            }
            tailX[0] = x;
            tailY[0] = y;
        }

        if (dir == LEFT)
            x--;
        if (dir == RIGHT)
            x++;
        if (dir == UP)
            y--;
        if (dir == DOWN)
            y++;

        if (x < 0 || x >= width || y < 0 || y >= height)
            gameOver = 1;

        for (k = 0; k < tailLen; k++) {
            if (tailX[k] == x && tailY[k] == y)
                gameOver = 1;
        }

        if (x == foodX && y == foodY) {
            score += 10;
            if (tailLen < 400)
                tailLen++;
            foodX = rand() % width;
            foodY = rand() % height;
            if (score >= 450)
                break;
        }

        usleep(120000);
    }

    clearScreen();
    setColor();
    if (score >= 450)
        printf("PAKE: You win! Score: %d\n", score);
    else
        printf("PAKE: Game over. Score: %d\n", score);
    printf("Press Enter...\n");
    resetColor();
    getchar();
}
