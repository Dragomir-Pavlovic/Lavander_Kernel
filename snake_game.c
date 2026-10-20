/*
 * Snake game for Lavander Kernel (Linux + Windows).
 * Controls: W/A/S/D or arrow keys, Q/Esc to quit, R to replay.
 */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "platform.h"
#define WIDTH 15
#define HEIGHT 10
#define MAX_LEN (WIDTH * HEIGHT)
#define START_SPEED 120
#define MIN_SPEED 60
#define KEY_NONE  0
#define KEY_QUIT -1
#define KEY_UP    1
#define KEY_DOWN  2
#define KEY_LEFT  3
#define KEY_RIGHT 4
typedef struct {
    int x[MAX_LEN];
    int y[MAX_LEN];
    int length;
    int dx, dy;
    int food_x, food_y;
    int score;
    int speed;
    int running;
    int won;
} Snake;
static int direction_from_char(int c)
{
    switch (c) {
        case 'w': case 'W': return KEY_UP;
        case 's': case 'S': return KEY_DOWN;
        case 'a': case 'A': return KEY_LEFT;
        case 'd': case 'D': return KEY_RIGHT;
        case 'q': case 'Q': case 27: return KEY_QUIT;
        default: return KEY_NONE;
    }
}
static int read_key(void)
{
    int c;
    if (!platform_kbhit())
        return KEY_NONE;
    c = platform_getch();
#ifdef _WIN32
    if (c == 0 || c == 224) {
        switch (platform_getch()) {
            case 72: return KEY_UP;
            case 80: return KEY_DOWN;
            case 75: return KEY_LEFT;
            case 77: return KEY_RIGHT;
            default: return KEY_NONE;
        }
    }
#else
    if (c == 27) {
        int a, b;
        if (!platform_kbhit()) return KEY_QUIT;
        a = platform_getch();
        if (a != '[' || !platform_kbhit()) return KEY_QUIT;
        b = platform_getch();
        switch (b) {
            case 'A': return KEY_UP;
            case 'B': return KEY_DOWN;
            case 'D': return KEY_LEFT;
            case 'C': return KEY_RIGHT;
            default: return KEY_NONE;
        }
    }
#endif
    return direction_from_char(c);
}
static int snake_has(int x, int y, int length, const Snake *s)
{
    int i;
    for (i = 0; i < length; i++)
        if (s->x[i] == x && s->y[i] == y)
            return 1;
    return 0;
}
static void place_food(Snake *s)
{
    int x, y;
    if (s->length >= MAX_LEN) {
        s->won = 1;
        s->running = 0;
        return;
    }
    do {
        x = rand() % WIDTH;
        y = rand() % HEIGHT;
    } while (snake_has(x, y, s->length, s));
    s->food_x = x;
    s->food_y = y;
}
static void reset_game(Snake *s)
{
    int i;
    s->length = 3;
    s->dx = 1;
    s->dy = 0;
    s->score = 0;
    s->speed = START_SPEED;
    s->running = 1;
    s->won = 0;
    for (i = 0; i < s->length; i++) {
        s->x[i] = WIDTH / 2 - i;
        s->y[i] = HEIGHT / 2;
    }
    place_food(s);
}
static void change_direction(Snake *s, int key)
{
    if (key == KEY_UP && s->dy != 1) {
        s->dx = 0; s->dy = -1;
    } else if (key == KEY_DOWN && s->dy != -1) {
        s->dx = 0; s->dy = 1;
    } else if (key == KEY_LEFT && s->dx != 1) {
        s->dx = -1; s->dy = 0;
    } else if (key == KEY_RIGHT && s->dx != -1) {
        s->dx = 1; s->dy = 0;
    }
}
static void update_game(Snake *s)
{
    int nx = s->x[0] + s->dx;
    int ny = s->y[0] + s->dy;
    int eating = (nx == s->food_x && ny == s->food_y);
    int i;
    if (nx < 0 || nx >= WIDTH || ny < 0 || ny >= HEIGHT) {
        s->running = 0;
        return;
    }
    if (snake_has(nx, ny, eating ? s->length : s->length - 1, s)) {
        s->running = 0;
        return;
    }
    for (i = s->length - 1; i > 0; i--) {
        s->x[i] = s->x[i - 1];
        s->y[i] = s->y[i - 1];
    }
    s->x[0] = nx;
    s->y[0] = ny;
    if (eating) {
        if (s->length < MAX_LEN) {
            s->x[s->length] = s->x[s->length - 1];
            s->y[s->length] = s->y[s->length - 1];
            s->length++;
        }
        s->score += 10;
        if (s->speed > MIN_SPEED)
            s->speed -= 3;
        place_food(s);
    }
}
static void draw_game(const Snake *s)
{
    int x, y, i;
    platform_clear_screen();
    platform_set_color();
    printf("  SNAKE   Score: %d   Len: %d\n", s->score, s->length);
    printf(" █████████████████\n");
    for (y = 0; y < HEIGHT; y++) {
        printf("█");
        for (x = 0; x < WIDTH; x++) {
            char cell = ' ';
            if (x == s->food_x && y == s->food_y)
                cell = '*';
            for (i = 0; i < s->length; i++) {
                if (s->x[i] == x && s->y[i] == y) {
                    cell = (i == 0) ? '@' : 'o';
                    break;
                }
            }
            printf("%c", cell);
        }
        printf("█\n");
    }
    printf(" █████████████████\n");
    printf(" W/A/S/D move   R restart   Q quit\n");
    fflush(stdout);
}
int main(void)
{
    Snake snake;
    int key;
    platform_init();
    platform_set_color();
    srand((unsigned int)time(NULL));
    for (;;) {
        reset_game(&snake);
        draw_game(&snake);
        while (snake.running) {
            while ((key = read_key()) != KEY_NONE) {
                if (key == KEY_QUIT) {
                    platform_reset_color();
                    platform_clear_screen();
                    return 0;
                }
                change_direction(&snake, key);
            }
            update_game(&snake);
            draw_game(&snake);
            if (snake.running)
                platform_sleep_ms((unsigned int)snake.speed);
        }
        printf("\n%s\n", snake.won ? "YOU WIN!" : "GAME OVER");
        printf("Score: %d\n", snake.score);
        printf("Press R to play again, or Enter to exit: ");
        fflush(stdout);
        {
            char answer[16];
            if (!fgets(answer, sizeof(answer), stdin) ||
                (answer[0] != 'r' && answer[0] != 'R'))
                break;
        }
    }
    platform_reset_color();
    return 0;
}
