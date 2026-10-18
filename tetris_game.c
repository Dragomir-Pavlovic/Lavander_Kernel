/*
 * tetris.game - classic tetris for the terminal (Linux + Windows).
 *
 * Sources live in games/, so a plain "gcc *.c" build of the kernel never
 * picks up this second main().
 *
 * Builds as a standalone program next to the kernel:
 *     lavander_games/tetris.game        (Linux)
 *     lavander_games\tetris.game.exe    (Windows)
 *
 * Controls: A/D or arrows to move, W / up to rotate, S / down for a soft
 * drop, Space for a hard drop, Q (or Esc) to quit.
 * After a round press R to play again, or Enter to leave.
 *
 * Rules: 10x20 field, 7 random pieces in a shuffled bag, wall kicks while
 * rotating, a short lock delay, line clear flash, scoring and levels like
 * the classic game. Only the portability layer (platform.c) is used, so the
 * same source runs on both operating systems.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "platform.h"

#define FIELD_W 10
#define FIELD_H 20
#define FRAME_CAP 4096

/* Smallest console the picture fits into. Every drawn line is at most
   MIN_COLUMNS-1 characters wide and the picture is MIN_ROWS lines tall,
   so nothing ever wraps or scrolls. */
#define MIN_COLUMNS 36
#define MIN_ROWS    23

#define TICK_MS        20
#define LOCK_DELAY_MS  500
#define MAX_LOCK_RESETS 15
#define FLASH_MS       260

#define PIECES 7

#define PIECE_I 0
#define PIECE_O 1
#define PIECE_T 2
#define PIECE_S 3
#define PIECE_Z 4
#define PIECE_J 5
#define PIECE_L 6

/*
 * One cell of the board is two terminal columns wide: an empty cell is
 * two spaces, a filled one is two block characters.  Mixing widths here
 * is what used to make the walls zigzag once blocks appeared.
 */
#define WALL   "\xE2\x96\x88"             /* U+2588 full block, 1 column */
#define BLOCK  "\xE2\x96\x88\xE2\x96\x88" /* whole cell, 2 columns      */
#define FALL   "\xE2\x96\x93"             /* U+2593 dark shade, 1 column */
#define FALLC  "\xE2\x96\x93\xE2\x96\x93" /* falling cell, 2 columns    */
#define BLANK  "  "                       /* empty cell, 2 columns      */

/* Score for clearing 1, 2, 3 or 4 lines (multiplied by the level). */
static const int LINE_SCORES[5] = { 0, 100, 300, 500, 800 };

/* Spawn orientation of the seven pieces, in their own box. */
static const unsigned char PIECE_BASE[PIECES][4][4] = {
    { { 0, 0, 0, 0 }, { 1, 1, 1, 1 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 } }, /* I */
    { { 1, 1 },       { 1, 1 },       { 0, 0 },       { 0, 0 }       }, /* O */
    { { 0, 1, 0 },    { 1, 1, 1 },    { 0, 0, 0 },    { 0, 0, 0 }    }, /* T */
    { { 0, 1, 1 },    { 1, 1, 0 },    { 0, 0, 0 },    { 0, 0, 0 }    }, /* S */
    { { 1, 1, 0 },    { 0, 1, 1 },    { 0, 0, 0 },    { 0, 0, 0 }    }, /* Z */
    { { 1, 0, 0 },    { 1, 1, 1 },    { 0, 0, 0 },    { 0, 0, 0 }    }, /* J */
    { { 0, 0, 1 },    { 1, 1, 1 },    { 0, 0, 0 },    { 0, 0, 0 }    }  /* L */
};

/* [piece][rotation][row][column], always stored inside a 4x4 box. */
static unsigned char shapes[PIECES][4][4][4];

typedef struct {
    unsigned char board[FIELD_H][FIELD_W]; /* 0 = empty, 1..7 = piece     */
    int type;
    int rot;
    int x;
    int y;
    int next;
    int bag[PIECES];
    int bag_pos;
    int score;
    int lines;
    int level;
    int running;
    int quit;
    int game_over;
    int lock_pending;
    int lock_timer;
    int lock_resets;
    int clearing[FIELD_H]; /* rows that are being cleared                */
    int clear_count;
    int flash_timer;
    int flashing;
    int dirty;
} TetrisGame;

enum {
    KEY_NONE = 0,
    KEY_QUIT,
    KEY_LEFT,
    KEY_RIGHT,
    KEY_ROTATE,
    KEY_SOFT,
    KEY_HARD
};

/* ------------------------------------------------------------------ */
/* Shapes                                                              */
/* ------------------------------------------------------------------ */

static int piece_size(int type)
{
    if (type == PIECE_I)
        return 4;
    if (type == PIECE_O)
        return 2;
    return 3;
}

/* Rotates a size x size matrix 90 degrees clockwise. */
static void rotate_matrix(const unsigned char *source, unsigned char *dest,
                          int size)
{
    int row;
    int col;

    memset(dest, 0, 16);

    for (row = 0; row < size; row++) {
        for (col = 0; col < size; col++)
            dest[col * 4 + (size - 1 - row)] = source[row * 4 + col];
    }
}

static void build_shapes(void)
{
    int type;
    int rot;
    unsigned char buffer[16];

    memset(shapes, 0, sizeof(shapes));

    for (type = 0; type < PIECES; type++) {
        int size = piece_size(type);

        memcpy(buffer, PIECE_BASE[type], sizeof(buffer));
        memcpy(shapes[type][0], buffer, sizeof(buffer));

        for (rot = 1; rot < 4; rot++) {
            rotate_matrix(shapes[type][rot - 1][0], buffer, size);
            memcpy(shapes[type][rot], buffer, sizeof(buffer));
        }
    }
}

/* ------------------------------------------------------------------ */
/* Game state                                                          */
/* ------------------------------------------------------------------ */

static int collides(const TetrisGame *game, int type, int rot, int x, int y)
{
    int size = piece_size(type);
    int row;
    int col;

    for (row = 0; row < size; row++) {
        for (col = 0; col < size; col++) {
            int world_x;
            int world_y;

            if (shapes[type][rot][row][col] == 0)
                continue;

            world_x = x + col;
            world_y = y + row;

            if (world_x < 0 || world_x >= FIELD_W || world_y >= FIELD_H)
                return 1;

            /* Cells above the ceiling are empty by definition - reading
               board[-1] would run off the array. */
            if (world_y >= 0 && game->board[world_y][world_x] != 0)
                return 1;
        }
    }

    return 0;
}

/* Returns 1 when the cell (row, col) of the field is covered by the
   piece that is currently falling. */
static int piece_covers(const TetrisGame *game, int row, int col)
{
    int size = piece_size(game->type);
    int local_row = row - game->y;
    int local_col = col - game->x;

    if (local_row < 0 || local_col < 0 || local_row >= size || local_col >= size)
        return 0;

    return shapes[game->type][game->rot][local_row][local_col] != 0;
}

static int row_is_clearing(const TetrisGame *game, int row)
{
    int i;

    for (i = 0; i < game->clear_count; i++) {
        if (game->clearing[i] == row)
            return 1;
    }

    return 0;
}

static void refill_bag(TetrisGame *game)
{
    int i;

    for (i = 0; i < PIECES; i++)
        game->bag[i] = i;

    for (i = PIECES - 1; i > 0; i--) {
        int other = rand() % (i + 1);
        int swap = game->bag[i];

        game->bag[i] = game->bag[other];
        game->bag[other] = swap;
    }

    game->bag_pos = 0;
}

static int take_from_bag(TetrisGame *game)
{
    if (game->bag_pos >= PIECES)
        refill_bag(game);

    return game->bag[game->bag_pos++];
}

static void spawn_piece(TetrisGame *game)
{
    game->type = game->next;
    game->next = take_from_bag(game);
    game->rot = 0;
    game->x = (FIELD_W - piece_size(game->type)) / 2;
    game->y = 0;
    game->lock_pending = 0;
    game->lock_timer = 0;
    game->lock_resets = 0;
    game->dirty = 1;

    if (collides(game, game->type, 0, game->x, game->y)) {
        game->game_over = 1;
        game->running = 0;
    }
}

static void finish_line_clear(TetrisGame *game);

static void lock_piece(TetrisGame *game)
{
    int size = piece_size(game->type);
    int row;
    int col;

    for (row = 0; row < size; row++) {
        for (col = 0; col < size; col++) {
            int world_x;
            int world_y;

            if (shapes[game->type][game->rot][row][col] == 0)
                continue;

            world_x = game->x + col;
            world_y = game->y + row;

            if (world_x >= 0 && world_x < FIELD_W && world_y >= 0 &&
                world_y < FIELD_H)
                game->board[world_y][world_x] =
                    (unsigned char)(game->type + 1);
        }
    }

    game->lock_pending = 0;
    game->lock_timer = 0;
    game->lock_resets = 0;
    game->dirty = 1;

    /* Which rows are now completely full? */
    game->clear_count = 0;

    for (row = 0; row < FIELD_H; row++) {
        int full = 1;

        for (col = 0; col < FIELD_W; col++) {
            if (game->board[row][col] == 0) {
                full = 0;
                break;
            }
        }

        if (full) {
            game->clearing[game->clear_count] = row;
            game->clear_count++;
        }
    }

    if (game->clear_count > 0) {
        game->flashing = 1;
        game->flash_timer = 0;
    } else {
        spawn_piece(game);
    }
}

/* Removes the full rows, counts the score and drops the next piece. */
static void finish_line_clear(TetrisGame *game)
{
    int read_row;
    int write_row = FIELD_H - 1;
    int count = game->clear_count;
    int new_level;

    for (read_row = FIELD_H - 1; read_row >= 0; read_row--) {
        if (row_is_clearing(game, read_row))
            continue;

        if (write_row != read_row)
            memcpy(game->board[write_row], game->board[read_row], FIELD_W);

        write_row--;
    }

    while (write_row >= 0) {
        memset(game->board[write_row], 0, FIELD_W);
        write_row--;
    }

    game->score += LINE_SCORES[count] * game->level;
    game->lines += count;

    new_level = 1 + game->lines / 10;
    if (new_level > game->level)
        game->level = new_level;

    game->clear_count = 0;
    game->flashing = 0;
    game->flash_timer = 0;

    spawn_piece(game);
}

/* ------------------------------------------------------------------ */
/* Moves                                                               */
/* ------------------------------------------------------------------ */

static int try_move(TetrisGame *game, int dx, int dy)
{
    if (collides(game, game->type, game->rot, game->x + dx, game->y + dy))
        return 0;

    game->x += dx;
    game->y += dy;
    game->dirty = 1;

    return 1;
}

static int try_rotate(TetrisGame *game, int direction)
{
    /* Simple wall kicks: try in place, then sideways, then one row up. */
    static const int kicks[6][2] = {
        { 0, 0 }, { -1, 0 }, { 1, 0 }, { -2, 0 }, { 2, 0 }, { 0, -1 }
    };

    int new_rot = (game->rot + direction + 4) % 4;
    int i;

    for (i = 0; i < 6; i++) {
        int new_x = game->x + kicks[i][0];
        int new_y = game->y + kicks[i][1];

        if (!collides(game, game->type, new_rot, new_x, new_y)) {
            game->rot = new_rot;
            game->x = new_x;
            game->y = new_y;
            game->dirty = 1;
            return 1;
        }
    }

    return 0;
}

static void hard_drop(TetrisGame *game)
{
    int distance = 0;

    while (!collides(game, game->type, game->rot, game->x, game->y + 1)) {
        game->y++;
        distance++;
    }

    game->score += distance * 2;
    lock_piece(game);
}

static int fall_interval(const TetrisGame *game)
{
    int interval = 800 - (game->level - 1) * 72;

    if (interval < 80)
        interval = 80;

    return interval;
}

/* ------------------------------------------------------------------ */
/* Input                                                               */
/* ------------------------------------------------------------------ */

static int map_char(int c)
{
    switch (c) {
    case 'a': case 'A': return KEY_LEFT;
    case 'd': case 'D': return KEY_RIGHT;
    case 'w': case 'W': return KEY_ROTATE;
    case 's': case 'S': return KEY_SOFT;
    case ' ':            return KEY_HARD;
    default:             return KEY_NONE;
    }
}

/*
 * Returns KEY_NONE when nothing is waiting, KEY_QUIT for Q / Esc and a
 * direction for the movement keys (WASD on both systems, arrows as well).
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
        case 72: return KEY_ROTATE;
        case 80: return KEY_SOFT;
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
        case 'A': return KEY_ROTATE;
        case 'B': return KEY_SOFT;
        case 'C': return KEY_RIGHT;
        case 'D': return KEY_LEFT;
        default:  return KEY_NONE;
        }
    }
#endif

    return map_char(c);
}

static void handle_key(TetrisGame *game, int key)
{
    int moved = 0;

    /* No input while the cleared lines are flashing. */
    if (game->flashing)
        return;

    switch (key) {
    case KEY_LEFT:
        moved = try_move(game, -1, 0);
        break;
    case KEY_RIGHT:
        moved = try_move(game, 1, 0);
        break;
    case KEY_ROTATE:
        moved = try_rotate(game, 1);
        break;
    case KEY_SOFT:
        if (try_move(game, 0, 1))
            game->score++;
        else
            game->lock_pending = 1;
        moved = 1;
        break;
    case KEY_HARD:
        hard_drop(game);
        moved = 1;
        break;
    default:
        break;
    }

    /* Moving or rotating while the piece rests can lift it again, which
       restarts the lock delay - a few times at most. */
    if (moved && game->lock_pending) {
        if (!collides(game, game->type, game->rot, game->x, game->y + 1)) {
            game->lock_pending = 0;
        } else {
            game->lock_resets++;
            if (game->lock_resets <= MAX_LOCK_RESETS)
                game->lock_timer = 0;
        }
    }
}

/* ------------------------------------------------------------------ */
/* Drawing                                                             */
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

/*
 * Everything next to the board: the preview piece, the score and the
 * controls. Keeping them here (instead of under the board) keeps every
 * line short, so the game also works in a narrow console.
 */
static void draw_panel_row(char *frame, size_t capacity, size_t *length,
                           const TetrisGame *game, int row)
{
    char text[32];

    put_str(frame, capacity, length, "  ");

    /* The preview piece occupies the first four rows. */
    if (row < 4) {
        int size = piece_size(game->next);
        int col;

        for (col = 0; col < size; col++) {
            int filled =
                (row < size) && (shapes[game->next][0][row][col] != 0);

            /* The preview cells have the same 2-column width. */
            put_str(frame, capacity, length, filled ? BLOCK : BLANK);
        }

        return;
    }

    switch (row) {
    case 5:
        put_str(frame, capacity, length, "Score");
        break;
    case 6:
        snprintf(text, sizeof(text), "%d", game->score);
        put_str(frame, capacity, length, text);
        break;
    case 7:
        put_str(frame, capacity, length, "Lines");
        break;
    case 8:
        snprintf(text, sizeof(text), "%d", game->lines);
        put_str(frame, capacity, length, text);
        break;
    case 9:
        put_str(frame, capacity, length, "Level");
        break;
    case 10:
        snprintf(text, sizeof(text), "%d", game->level);
        put_str(frame, capacity, length, text);
        break;
    case 12:
        put_str(frame, capacity, length, "A/D move");
        break;
    case 13:
        put_str(frame, capacity, length, "W rotate");
        break;
    case 14:
        put_str(frame, capacity, length, "S soft drop");
        break;
    case 15:
        put_str(frame, capacity, length, "Space drop");
        break;
    case 16:
        put_str(frame, capacity, length, "Q quit");
        break;
    default:
        break;
    }
}

static void draw_field(const TetrisGame *game, char *frame, size_t capacity)
{
    size_t length = 0;
    int blink = (game->flash_timer / 80) & 1;
    int row;
    int col;
    int i;

    frame[0] = '\0';

    put_str(frame, capacity, &length, " TETRIS\n");

    /* Top border with the NEXT label. It is as wide as a field row:
       one space, then the ten 2-column cells plus the right wall. */
    put_str(frame, capacity, &length, " ");
    for (i = 0; i < FIELD_W * 2 + 1; i++)
        put_str(frame, capacity, &length, WALL);
    put_str(frame, capacity, &length, "  NEXT\n");

    for (row = 0; row < FIELD_H; row++) {
        int clearing = row_is_clearing(game, row) && blink;

        put_str(frame, capacity, &length, WALL);

        for (col = 0; col < FIELD_W; col++) {
            /* Every cell is exactly two columns wide. */
            if (clearing) {
                put_str(frame, capacity, &length, BLANK);
            } else if (game->board[row][col] != 0) {
                put_str(frame, capacity, &length, BLOCK);
            } else if (piece_covers(game, row, col)) {
                put_str(frame, capacity, &length, FALLC);
            } else {
                put_str(frame, capacity, &length, BLANK);
            }
        }

        put_str(frame, capacity, &length, WALL);
        draw_panel_row(frame, capacity, &length, game, row);
        put_str(frame, capacity, &length, "\n");
    }

    /* Bottom border. The last line deliberately has no newline: writing
       one on the very last row of the console would scroll the whole
       picture up by one line. */
    put_str(frame, capacity, &length, " ");
    for (i = 0; i < FIELD_W * 2 + 1; i++)
        put_str(frame, capacity, &length, WALL);
}

static void show_result(const TetrisGame *game)
{
    platform_clear_screen();
    platform_set_color();

    printf("\n");
    printf(" ------------------------------\n");
    printf("   GAME OVER\n");
    printf("   Score:  %d\n", game->score);
    printf("   Lines:  %d\n", game->lines);
    printf("   Level:  %d\n", game->level);
    printf(" ------------------------------\n");
    printf("   Press R to replay, Enter to exit\n");
    printf("\n");
    fflush(stdout);
}

/* ------------------------------------------------------------------ */
/* Main                                                                */
/* ------------------------------------------------------------------ */

static void init_game(TetrisGame *game)
{
    memset(game, 0, sizeof(*game));

    refill_bag(game);

    game->level = 1;
    game->running = 1;
    game->quit = 0;
    game->game_over = 0;
    game->dirty = 1;

    /* The first piece comes from the bag, the preview from the bag too. */
    game->next = take_from_bag(game);
    spawn_piece(game);
}

/*
 * The board only fits when the console is at least MIN_COLUMNS wide and
 * MIN_ROWS tall.  When it is not, the picture would wrap around and the
 * screen would become a mess, so we stop and ask the user to resize.
 * Windows consoles are grown automatically; other terminals are not.
 * Returns 0 when the input is gone (a closed pipe) and the game has to
 * give up.
 */
static int wait_for_terminal(TetrisGame *game, char *answer, size_t size)
{
    while (!platform_ensure_terminal_size(MIN_COLUMNS, MIN_ROWS)) {
        platform_clear_screen();
        platform_set_color();
        printf("\n TETRIS needs a bigger window.\n");
        printf(" At least %d x %d.\n\n", MIN_COLUMNS, MIN_ROWS);
        printf(" Resize it, then press Enter.\n\n");
        fflush(stdout);

        if (fgets(answer, (int)size, stdin) == NULL)
            return 0;

        if (game != NULL)
            game->dirty = 1;
    }

    return 1;
}

int main(void)
{
    TetrisGame game;
    char frame[FRAME_CAP];
    char answer[32];

    platform_init();
    srand((unsigned int)time(NULL));
    build_shapes();
    platform_set_color();

    if (!wait_for_terminal(NULL, answer, sizeof(answer)))
        return 0;

    for (;;) {
        int fall_timer = 0;
        int size_timer = 0;

        init_game(&game);

        while (game.running) {
            int key;
            int burst;

            /* Read at most a few keys per tick, so a burst of input can
               never play several pieces without drawing in between. */
            for (burst = 0; burst < 8; burst++) {
                key = read_key();

                if (key == KEY_NONE)
                    break;

                if (key == KEY_QUIT) {
                    game.running = 0;
                    game.quit = 1;
                    break;
                }

                handle_key(&game, key);
                game.dirty = 1;
            }

            if (game.running) {
                if (game.flashing) {
                    game.flash_timer += TICK_MS;
                    game.dirty = 1;

                    if (game.flash_timer >= FLASH_MS)
                        finish_line_clear(&game);
                } else {
                    fall_timer += TICK_MS;

                    if (fall_timer >= fall_interval(&game)) {
                        fall_timer = 0;

                        if (!try_move(&game, 0, 1))
                            game.lock_pending = 1;

                        game.dirty = 1;
                    }

                    if (game.lock_pending) {
                        game.lock_timer += TICK_MS;

                        if (game.lock_timer >= LOCK_DELAY_MS) {
                            lock_piece(&game);
                            game.dirty = 1;
                        }
                    }
                }
            }

            /* Re-check the console size from time to time: if the user
               shrinks the window while playing, the picture would start
               wrapping around, so pause and wait until it fits again. */
            if (game.running) {
                size_timer += TICK_MS;

                if (size_timer >= 500) {
                    size_timer = 0;

                    if (!wait_for_terminal(&game, answer, sizeof(answer))) {
                        game.quit = 1;
                        game.running = 0;
                        break;
                    }
                }
            }

            /* The last frame of a round is drawn as well, right before the
               result screen takes over. */
            if (game.dirty) {
                platform_clear_screen();
                platform_set_color();
                draw_field(&game, frame, sizeof(frame));
                fputs(frame, stdout);
                fflush(stdout);
                game.dirty = 0;
            }

            if (!game.running)
                break;

            platform_sleep_ms(TICK_MS);
        }

        if (game.quit) {
            platform_clear_screen();
            platform_set_color();
            printf("\n Thanks for playing TETRIS!\n");
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
