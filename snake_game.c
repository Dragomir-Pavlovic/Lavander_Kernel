/*
 * snake.game - classic snake for the terminal (Linux + Windows).
 *
 * The sources live in games/ so that a plain "gcc *.c" build of the kernel
 * never picks up this second main().
 *
 * Builds as a standalone program next to the kernel:
 *     lavander_games/snake.game        (Linux)
 *     lavander_games\snake.game.exe    (Windows)
 *
 * Controls: W/A/S/D or the arrow keys, Q (or Esc) to quit.
 * After a round press R to play again, or Enter to leave.
 *
 * The game only depends on the portability layer (platform.c), so the
 * same source runs on both operating systems.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "platform.h"

#define FIELD_W   15
#define FIELD_H   10
#define MAX_LEN   (FIELD_W * FIELD_H)
#define FRAME_CAP 4096

#define START_SPEED_MS 129
#define MIN_SPEED_MS    90
#define SPEED_STEP_MS    6

/* Smallest console the picture fits into: every line is at most
   MIN_COLUMNS-1 characters wide and the picture is MIN_ROWS lines tall,
   so nothing can wrap or scroll. */
#define MIN_COLUMNS 30
#define MIN_ROWS    40

#define WALL "\xE2\x96\x88" /* U+2588 full block, UTF-8 */
#define HEAD "@"
#define BODY "o"
#define FOOD "*"

typedef struct {
    int sx[MAX_LEN];
    int sy[MAX_LEN];
    int len;
    int dir_x;
    int dir_y;
    int food_x;
    int food_y;
    int score;
    int speed_ms;
    int running;
    int quit;
    int won;
} SnakeGame;

enum {
    KEY_NONE = 0,
    KEY_QUIT,
    KEY_UP,
    KEY_DOWN,
    KEY_LEFT,
    KEY_RIGHT
};

/* ------------------------------------------------------------------ */
/* Frame buffer helpers (no unbounded formatting, no warnings)         */
/* ------------------------------------------------------------------ */

static void put_str(char *buffer, size_t capacity, size_t *length,
                    const char *text)
{
    size_t available;
    size_t count;

    if (buffer == NULL || length == NULL || *length + 1 >= capacity)
        return;

    available = capacity - *length - 1;
    count = strlen(text);

    if (count > available)
        count = available;

    if (count > 0) {
        memcpy(buffer + *length, text, count);
        *length += count;
        buffer[*length] = '\0';
    }
}

/* ------------------------------------------------------------------ */
/* Input                                                               */
/* ------------------------------------------------------------------ */

static int map_char(int c)
{
    switch (c) {
    case 'w': case 'W': return KEY_UP;
    case 's': case 'S': return KEY_DOWN;
    case 'a': case 'A': return KEY_LEFT;
    case 'd': case 'D': return KEY_RIGHT;
    default:            return KEY_NONE;
    }
}

/*
 * Returns KEY_NONE when nothing is waiting, KEY_QUIT for Q / Esc and a
 * direction for movement keys (WASD on both systems, arrows as well).
 */
static int read_key(void)
{
    int c;

    if (!platform_kbhit())
        return KEY_NONE;

    c = platform_getch();

    if (c == 'q' || c == 'Q')
        return KEY_QUIT;

#ifdef _WIN32
    /* Arrows come as a 0/224 prefix followed by 72/75/77/80. */
    if (c == 0 || c == 224) {
        int code = platform_getch();

        switch (code) {
        case 72: return KEY_UP;
        case 80: return KEY_DOWN;
        case 75: return KEY_LEFT;
        case 77: return KEY_RIGHT;
        default: return KEY_NONE;
        }
    }

    if (c == 27)
        return KEY_QUIT;
#else
    /* Arrows come as ESC [ A/B/C/D. */
    if (c == 27) {
        int first;
        int second;

        if (!platform_kbhit())
            return KEY_QUIT;

        first = platform_getch();
        if (first != '[' || !platform_kbhit())
            return KEY_QUIT;

        second = platform_getch();
        switch (second) {
        case 'A': return KEY_UP;
        case 'B': return KEY_DOWN;
        case 'C': return KEY_RIGHT;
        case 'D': return KEY_LEFT;
        default:  return KEY_NONE;
        }
    }
#endif

    return map_char(c);
}


/* Game logic */

static int occupied(const SnakeGame *game, int x, int y)
{
    int i;

    for (i = 0; i < game->len; i++) {
        if (game->sx[i] == x && game->sy[i] == y)
            return 1;
    }

    return 0;
}

static void place_food(SnakeGame *game)
{
    int tries = 0;
    int x;
    int y;

    do {
        game->food_x = rand() % FIELD_W;
        game->food_y = rand() % FIELD_H;
        tries++;
    } while (occupied(game, game->food_x, game->food_y) && tries < 1000);

    if (occupied(game, game->food_x, game->food_y)) {
        for (y = 0; y < FIELD_H; y++) {
            for (x = 0; x < FIELD_W; x++) {
                if (!occupied(game, x, y)) {
                    game->food_x = x;
                    game->food_y = y;
                    return;
                }
            }
        }
    }
}

static void init_game(SnakeGame *game)
{
    int center_x = FIELD_W / 2;
    int center_y = FIELD_H / 2;
    int i;

    memset(game, 0, sizeof(*game));

    game->len = 3;
    for (i = 0; i < game->len; i++) {
        game->sx[i] = center_x - i;
        game->sy[i] = center_y;
    }

    game->dir_x = 1;
    game->dir_y = 0;
    game->speed_ms = START_SPEED_MS;
    game->running = 1;
    game->quit = 0;
    game->won = 0;

    place_food(game);
}

static void apply_direction(SnakeGame *game, int key)
{
    /* A direction change may never be a 180 degree turn. */
    switch (key) {
    case KEY_UP:
        if (game->dir_y != 1) { game->dir_x = 0;  game->dir_y = -1; }
        break;
    case KEY_DOWN:
        if (game->dir_y != -1) { game->dir_x = 0;  game->dir_y = 1; }
        break;
    case KEY_LEFT:
        if (game->dir_x != 1) { game->dir_x = -1; game->dir_y = 0; }
        break;
    case KEY_RIGHT:
        if (game->dir_x != -1) { game->dir_x = 1;  game->dir_y = 0; }
        break;
    default:
        break;
    }
}

static void step(SnakeGame *game)
{
    int new_x = game->sx[0] + game->dir_x;
    int new_y = game->sy[0] + game->dir_y;
    int eating = (new_x == game->food_x && new_y == game->food_y);
    int tail_x;
    int tail_y;
    int limit;
    int i;

    /* Walls kill. */
    if (new_x < 0 || new_x >= FIELD_W || new_y < 0 || new_y >= FIELD_H) {
        game->running = 0;
        return;
    }

    /* Biting yourself kills - the last cell is free unless we eat now. */
    limit = eating ? game->len : game->len - 1;
    for (i = 0; i < limit; i++) {
        if (game->sx[i] == new_x && game->sy[i] == new_y) {
            game->running = 0;
            return;
        }
    }

    tail_x = game->sx[game->len - 1];
    tail_y = game->sy[game->len - 1];

    for (i = game->len - 1; i > 0; i--) {
        game->sx[i] = game->sx[i - 1];
        game->sy[i] = game->sy[i - 1];
    }

    game->sx[0] = new_x;
    game->sy[0] = new_y;

    if (eating) {
        game->len++;
        game->sx[game->len - 1] = tail_x;
        game->sy[game->len - 1] = tail_y;
        game->score += 10;

        if (game->speed_ms > MIN_SPEED_MS) {
            game->speed_ms -= SPEED_STEP_MS;
            if (game->speed_ms < MIN_SPEED_MS)
                game->speed_ms = MIN_SPEED_MS;
        }

        if (game->len >= MAX_LEN) {
            game->won = 1;
            game->running = 0;
            return;
        }

        place_food(game);
    }
}


/* Drawing */


static void draw_field(const SnakeGame *game, char *frame, size_t capacity)
{
    char header[160];
    size_t length = 0;
    int x;
    int y;
    int i;

    frame[0] = '\0';

    /* Two short lines instead of one long one: nothing ever wraps, even
       in a narrow console. */
    snprintf(header, sizeof(header), " SNAKE   Score: %d   Len: %d\n",
             game->score, game->len);
    put_str(frame, capacity, &length, header);

    /* Top border. */
    put_str(frame, capacity, &length, " ");
    for (i = 0; i < FIELD_W + 2; i++)
        put_str(frame, capacity, &length, WALL);
    put_str(frame, capacity, &length, "\n");

    /* Field. */
    for (y = 0; y < FIELD_H; y++) {
        put_str(frame, capacity, &length, WALL);

        for (x = 0; x < FIELD_W; x++) {
            if (x == game->food_x && y == game->food_y) {
                put_str(frame, capacity, &length, FOOD);
            } else {
                const char *cell = " ";

                for (i = 0; i < game->len; i++) {
                    if (game->sx[i] == x && game->sy[i] == y) {
                        cell = (i == 0) ? HEAD : BODY;
                        break;
                    }
                }

                put_str(frame, capacity, &length, cell);
            }
        }

        put_str(frame, capacity, &length, WALL);
        put_str(frame, capacity, &length, "\n");
    }

    /* Bottom border. */
    put_str(frame, capacity, &length, " ");
    for (i = 0; i < FIELD_W + 2; i++)
        put_str(frame, capacity, &length, WALL);
    put_str(frame, capacity, &length, "\n");

    /* Controls. The last line has no newline on purpose: writing one on
       the very last row of the console would scroll the picture up. */
    put_str(frame, capacity, &length, " W/A/S/D move   R restart   Q quit");
}

static void show_result(const SnakeGame *game)
{
    platform_clear_screen();
    platform_set_color();

    printf("\n");

    if (game->won) {
        printf(" ------------------------------\n");
        printf("   YOU WIN! The field is full.\n");
    } else {
        printf(" ------------------------------\n");
        printf("   GAME OVER\n");
    }

    printf("   Score:  %d\n", game->score);
    printf("   Length: %d\n", game->len);
    printf(" ------------------------------\n");
    printf("   Press R to replay, Enter to exit\n");
    printf("\n");
    fflush(stdout);
}

/* ------------------------------------------------------------------ */
/* Main                                                                */
/* ------------------------------------------------------------------ */

/*
 * The board only fits when the console is at least MIN_COLUMNS wide and
 * MIN_ROWS tall - otherwise every line would wrap around and the screen
 * would become a mess.  Windows consoles are grown automatically, other
 * terminals have to be resized by the user.  Returns 0 when the input is
 * gone (a closed pipe) and the game has to give up.
 */
static int wait_for_terminal(char *answer, size_t size)
{
    while (!platform_ensure_terminal_size(MIN_COLUMNS, MIN_ROWS)) {
        platform_clear_screen();
        platform_set_color();
        printf("\n SNAKE needs a bigger window.\n");
        printf(" At least %d x %d.\n\n", MIN_COLUMNS, MIN_ROWS);
        printf(" Resize it, then press Enter.\n\n");
        fflush(stdout);

        if (fgets(answer, (int)size, stdin) == NULL)
            return 0;
    }

    return 1;
}

int main(void)
{
    SnakeGame game;
    char frame[FRAME_CAP];
    char answer[32];

    platform_init();
    srand((unsigned int)time(NULL));

    platform_set_color();

    if (!wait_for_terminal(answer, sizeof(answer)))
        return 0;

    for (;;) {
        int size_timer = 0;

        init_game(&game);

        while (game.running) {
            int key;

            /* Drain everything that is waiting so no key is lost. */
            while ((key = read_key()) != KEY_NONE) {
                if (key == KEY_QUIT) {
                    game.running = 0;
                    game.quit = 1;
                    break;
                }

                apply_direction(&game, key);
            }

            if (!game.running)
                break;

            /* Re-check the console size from time to time: if the window
               is shrunk while playing, every line would wrap around. */
            size_timer += (int)game.speed_ms;

            if (size_timer >= 500) {
                size_timer = 0;

                if (!wait_for_terminal(answer, sizeof(answer))) {
                    game.quit = 1;
                    game.running = 0;
                    break;
                }
            }

            step(&game);

            platform_clear_screen();
            platform_set_color();
            draw_field(&game, frame, sizeof(frame));
            fputs(frame, stdout);
            fflush(stdout);

            if (game.running)
                platform_sleep_ms((unsigned int)game.speed_ms);
        }

        if (game.quit) {
            platform_clear_screen();
            platform_set_color();
            printf("\n Thanks for playing SNAKE!\n");
            printf(" Score: %d\n\n", game.score);
            fflush(stdout);
            break;
        }

        show_result(&game);

        if (fgets(answer, sizeof(answer), stdin) == NULL)
            break;

        if (answer[0] == 'r' || answer[0] == 'R')
            continue;

        break;
    }

    platform_reset_color();
    return 0;
}
