# Lavander Kernel - cross platform build (Linux, macOS, Windows/MinGW)
#
#   make            build the kernel
#   make run        build and start it
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
        pake.c \
        sound_system_settings.c \
        platform.c \
        fs.c

# shell.c is the legacy single file version and is intentionally not built.

ifeq ($(OS),Windows_NT)
    TARGET  := lavander.exe
    LDLIBS  := -ladvapi32
    RM      := cmd /C del /Q
else
    TARGET  := lavander
    LDLIBS  :=
    RM      := rm -f
endif

.PHONY: all run clean

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET) $(LDFLAGS) $(LDLIBS)

run: $(TARGET)
	./$(TARGET)

clean:
	$(RM) $(TARGET)

