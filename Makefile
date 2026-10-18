# Lavander Kernel + games - cross platform build
# (Linux, macOS, Windows/MinGW)
#
#   make            build the kernel and the games
#   make kernel     build only the kernel
#   make games      build only the games (lavander_games/)
#   make run        build and start the kernel
#   make clean      remove build output
#
# On Windows run this from MSYS2 / MinGW, or just use build.bat.

# make predefines CC=cc, which MinGW does not provide - fall back to gcc
# while still letting the user override it (make CC=clang, env CC=...).
ifeq ($(origin CC),default)
    CC := gcc
endif

CFLAGS  ?= -std=c11 -Wall -Wextra -O2
LDFLAGS ?=

SRCS := main.c \
        config.c \
        commands.c \
        system.c \
        monitor.c \
        history.c \
        parser.c \
        process.c \
        calculator.c \
        notes.c \
        todo.c \
        firesystem.c \
        sound_system_settings.c \
        platform.c \
        fs.c

# The games only need the portability layer. Their sources live in games/,
# so a plain "gcc *.c" build of the kernel never picks up a second main().
GAME_SRCS   := games/snake_game.c platform.c
TETRIS_SRCS := games/tetris_game.c platform.c

# shell.c is the legacy single file version and is intentionally not built.

ifeq ($(OS),Windows_NT)
    TARGET     := lavander.exe
    GAME       := lavander_games\snake.game.exe
    TETRIS     := lavander_games\tetris.game.exe
    LDLIBS     := -ladvapi32
    MKDIR_GAMES := cmd /C if not exist lavander_games mkdir lavander_games
    RM         := cmd /C del /Q
else
    TARGET     := lavander
    GAME       := lavander_games/snake.game
    TETRIS     := lavander_games/tetris.game
    LDLIBS     :=
    MKDIR_GAMES := mkdir -p lavander_games
    RM         := rm -f
endif

.PHONY: all kernel games run clean

all: kernel games

kernel: $(TARGET)

games: $(GAME) $(TETRIS)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET) $(LDFLAGS) $(LDLIBS)

$(GAME): $(GAME_SRCS)
	$(MKDIR_GAMES)
	$(CC) $(CFLAGS) -I. $(GAME_SRCS) -o $(GAME) $(LDFLAGS) $(LDLIBS)

$(TETRIS): $(TETRIS_SRCS)
	$(MKDIR_GAMES)
	$(CC) $(CFLAGS) -I. $(TETRIS_SRCS) -o $(TETRIS) $(LDFLAGS) $(LDLIBS)

run: $(TARGET)
	./$(TARGET)

clean:
	$(RM) $(TARGET)
	$(RM) $(GAME)
	$(RM) $(TETRIS)
