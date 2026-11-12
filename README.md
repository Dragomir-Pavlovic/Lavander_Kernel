# Lavander kernel simulation for Linux and Windows based mashines

** Lavander kernel script is build for Linux and Windows based mashines to simulate kernel based on Unix.
** Simulation run's like early Unix version 1 to 4 (Research Unix System build for local mashines inside MIT Bell lab)

## Supported platforms

The kernel runs on **Linux** and **Windows** (7 / 10 / 11, x86, x86_64 and ARM64)
from the same source code. All operating system specific code lives in one file,
`platform.c` / `platform.h`, so everything else in the project is portable:

| Feature                | Linux                         | Windows                              |
| ---------------------- | ----------------------------- | ------------------------------------ |
| Colors                 | ANSI escape codes             | Win32 `SetConsoleTextAttribute`      |
| Clear screen           | ANSI                          | Win32 console buffer                 |
| Sleep / timing         | `usleep`                      | `Sleep`                              |
| Keyboard (games)       | `termios`                     | `_kbhit` / `_getch`                  |
| Kernel name            | `/etc/os-release`             | Registry (`ProductName`)             |
| Kernel version         | `uname`                       | `RtlGetVersion`                      |
| Architecture           | `uname -m`                    | `GetNativeSystemInfo`                |
| CPU model              | `/proc/cpuinfo`               | Registry (`ProcessorNameString`)     |
| Memory                 | `/proc/meminfo`               | `GlobalMemoryStatusEx`               |
| Uptime                 | `/proc/uptime`                | `GetTickCount64`                     |

On Windows the console is switched to UTF-8 at startup, so the box drawing and
block characters of the menus and of the bundled games render correctly.

For now kernel shows specifications of already installed system that runs outside Lavander kernel sandbox.

## Building the kernel

The build produces **few programs**: the kernel itself with system information with help and manual as one
and the bundled games, snake and tetris.
Games, which lands in a `lavander_games` directory:

| Program                | Linux                          | Windows                          |
| ---------------------- | ------------------------------ | -------------------------------- |
| the kernel             | `./lavander`                   | `lavander.exe`                   |
| the bundled snake      | `lavander_games/snake.game`    | `lavander_games\snake.game.exe`  |
| the bundled tetris     | `lavander_games/tetris.game`   | `lavander_games\tetris.game.exe` |

### Linux running process from terminal

```sh
make            # builds ./lavander and both games in lavander_games/
make kernel     # kernel only
make games      # games only
make run        # build and start the kernel
make clean
```

Or by hand: `gcc *.c -o lavander`

### Windows

With MinGW-w64 (MSYS2, WinLibs, TDM-GCC...) or MSVC:

```bat
build.bat       :: picks gcc automatically, falls back to cl.exe
```

Or by hand from an MSYS2 / MinGW shell:

```sh
make            # builds lavander.exe and both games in lavander_games\
```

Or manually:

```bat
mkdir lavander_games
gcc -std=c11 -Wall -Wextra -O2 *.c -o lavander.exe -ladvapi32
gcc -std=c11 -Wall -Wextra -O2 -I. games\snake_game.c  platform.c -o lavander_games\snake.game.exe  -ladvapi32
gcc -std=c11 -Wall -Wextra -O2 -I. games\tetris_game.c platform.c -o lavander_games\tetris.game.exe -ladvapi32
```

The game sources live in `games/`, so a plain `gcc *.c` build only ever
produces the kernel - every game has its own `main()` and is always built as a
separate program.

Note: `shell.c` is the old single file version of the project, kept only as a
reference. It is disabled by default (its `#if 0` block) so it can never clash
with the modular sources; enable it only to build it standalone.
Not recommended if you want fresh out of the box experience with Lavander kernel sandbox.

## File system commands

The kernel has its own file manager. Everything happens inside a **default
directory** (`lavander_home`, created next to where you start the program, and
overridable with the `LAVANDER_HOME` environment variable). `pwd` shows `/`,
which is that directory - you can go as deep as you want (`dir` inside `dir`
inside `dir`) but you can never escape the sandbox, so the same commands behave
identically on Linux and Windows.

Note: Be careful, some of the commands, if not used properly can make mistake and brake system.
read whole this Readme.txt file and read manual used as 'man' command and help used as 'help'
command properly so you wont have any mistakes and have full understanding of program and how to run it.

| Command             | What it does                                              |
| ------------------- | --------------------------------------------------------- |
| `pwd`               | prints the directory you are in (`/` at the root)         |
| `ls` / `dir [PATH]` | lists a directory (`[DIR]` / `[FILE]`, sorted, with sizes)|
| `mkdir PATH`        | creates a directory, nested paths allowed (`a/b/c`)       |
| `rmdir PATH`        | removes an **empty** directory                            |
| `rm PATH`           | deletes a file                                            |
| `rm -r PATH`        | deletes a directory with everything inside it             |
| `cd [PATH]`         | changes directory, `..` goes up, `cd` alone = home        |
| `touch FILE`        | creates an empty text file                                |
| `cat FILE`          | prints the contents of a file                             |
| `write FILE TEXT`   | creates / overwrites a file with one line of text         |
| `edit FILE`         | opens the built-in line editor                            |
| `mv SRC DST`        | moves or renames a file or directory                      |

Paths may be absolute from the root (`/docs/notes`) or relative to where you
are (`notes/2026`, `../..`, `./file.txt`). Names may be quoted when they
contain spaces: `cat "my notes.txt"`.

`help` shows this list inside the shell, `man` shows the longer tutorial.

### The editor

Note: For now this is simple short text editor that function as a simulation of real editor.
Right now it is in process of making VIM like text editor.
Note: CORE-16 is from previous build that runs simulation of 16-BIT processor and it stuck still.
We wont change it, its part of our inside joke and also part of old, first ever build of this project.
```
CORE-16] >> edit docs/notes.txt
 ----------------------------------------------------------
 EDITOR: /docs/notes.txt  (3 lines)
 Type 'h' for commands
 ----------------------------------------------------------
edit:notes.txt>
```

| Key       | Action                             |
| --------- | ---------------------------------- |
| `a`       | append a line at the end           |
| `i N`     | insert a line before line `N`      |
| `r N`     | replace line `N`                   |
| `d N`     | delete line `N`                    |
| `p`       | print the file with line numbers   |
| `w`       | save                               |
| `wq`      | save and quit                      |
| `q`       | quit (asks before losing changes)  |
| `h`       | help                               |

### Examples

```
mkdir projects/alpha            # /projects/alpha
cd projects/alpha
write todo.txt buy milk         # creates /projects/alpha/todo.txt
edit todo.txt                   # open the editor
ls                              # shows todo.txt
cd ../..                        # back to /
mv projects/alpha/todo.txt ./   # move it to the root
cat todo.txt
rm -r projects                  # delete the whole tree
```

## The games (`snake.game`, `tetris.game`)

The build ships two classic terminal games as separate programs, in the
default `lavander_games` directory:
Note: Games wont run outside games file.
```
lavander_games/
    snake.game      tetris.game          Linux
    snake.game.exe  tetris.game.exe      Windows
```

They only depend on the portability layer (`platform.c`), so the same sources
(`games/snake_game.c`, `games/tetris_game.c`) build and play on both systems.

### Starting them

| Where                    | What to do                                                                     |
| ------------------------ | ------------------------------------------------------------------------------ |
| from the kernel shell    | type `snake` or `tetris` (it starts `lavander_games/...` for you)              |
| from the games menu      | type `games`, then `1` for tetris or 'tetris' and `2` for snake of 'snake'     |
| on its own, Windows      | `lavander_games\tetris.game.exe` (in PowerShell: `lavander_games\tetris.game`) |
| on its own, Linux        | `./lavander_games/snake.game`                                                  |

*** user@linuxmint: System: Wildbound removed, do not question it.

`pake` still works as a command: it now starts the snake game, because the old
pake penguin game was removed and replaced by it.

Start the kernel from the directory that contains `lavander_games`, otherwise
the game commands report that the game was not found and tell you to build it
with `make` / `build.bat`.

### Snake

```
 SNAKE   Score: 30   Len: 6
 █████████████████
█               █
█      ooooo@   █
█               █
█               █
█        *      █
█               █
█               █
█               █
█               █
█               █
█               █
█               █
█               █
█               █
█               █
█               █
 █████████████████
 W/A/S/D move   R restart   Q quit
```

- `W A S D` or the arrow keys - steer
- `Q` or `Esc` - quit immediately
- eat `*` to grow and score 10 points, every bite makes the snake a bit faster
- hitting a wall or your own body ends the round (the tail cell you are moving
  into is still free, as in the classic game)
- filling the whole field wins the game
- after a round: `R` plays again, `Enter` leaves

### Tetris

```
 TETRIS
 █████████████████████  NEXT
█                    █  ██
█                    █  ██
█                    █
█                    █
█                    █
█                    █  Score
█                    █  1240
█                    █  Lines
█                    █  8
█                    █  Level
█                    █  2
█                    █
█                    █  A/D move
█                    █  W rotate
█      ▓▓▓▓          █  S soft drop
█      ▓▓▓▓          █  Space drop
█                    █  Q quit
█            ██████  █
█  ██████████████    █
█████████████████    █
 █████████████████████
```

- `A` / `D` or `Left` / `Right` - move
- `W` or `Up` - rotate (with wall kicks)
- `S` or `Down` - soft drop (+1 point per cell)
- `Space` - hard drop (+2 points per cell)
- `Q` or `Esc` - quit immediately
- 10x20 field, the seven pieces come in shuffled bags of seven
- 1 line 100, 2 lines 300, 3 lines 500, 4 lines 800 - all multiplied by the
  level; a new level every 10 lines and the pieces fall faster each level
- the falling piece is drawn darker than the locked ones, `NEXT` shows the
  upcoming piece, cleared lines flash before they disappear
- a short lock delay lets you slide the piece after it lands
- after a round: `R` plays again, `Enter` leaves

### Window size

Both games draw a picture that never wraps and never scrolls, so they need a
console of at least **36 x 23** characters (tetris) or **36 x 20** (snake):

- on Windows the console window is enlarged automatically when it is too small
- on other terminals the game stops and asks you to resize the window and then
  press `Enter`; the size is checked again during play, so shrinking the
  window in the middle of a round pauses the game until it fits again

Everything (score, level, next piece, controls) is kept beside the board in
short columns for exactly this reason - one long header line would break the
whole picture in a small window.
```


Requires `tkinter` and `pillow`. It is written with portable APIs only
(`tkinter`, `PIL`, `os.path`), so it works on Linux and Windows unchanged.
