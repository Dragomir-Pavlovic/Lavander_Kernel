# Lavander_Shell_and_SLM
Bash shell script that run several things, one of them is SLM (Small learning model) that can chat with you, and speak in 16-bit voice

Bash shell is writen in C language and SLM in python. Bought can be run as a terminal command. Running pyscript for SLM opens a tiny window, where SLM model lives. It can be also embedded in raspberry pi.

Enjoy using it.

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
| OS name                | `/etc/os-release`             | Registry (`ProductName`)             |
| Kernel version         | `uname`                       | `RtlGetVersion`                      |
| Architecture           | `uname -m`                    | `GetNativeSystemInfo`                |
| CPU model              | `/proc/cpuinfo`               | Registry (`ProcessorNameString`)     |
| Memory                 | `/proc/meminfo`               | `GlobalMemoryStatusEx`               |
| Uptime                 | `/proc/uptime`                | `GetTickCount64`                     |

On Windows the console is switched to UTF-8 at startup, so the box drawing and
block characters of the menus and of the PAKE (snake) game render correctly.

## Building the kernel

### Linux

```sh
make            # builds ./lavander
make run        # build and start
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
make            # builds lavander.exe
```

Or manually:

```bat
gcc -std=c11 -Wall -Wextra -O2 *.c -o lavander.exe -ladvapi32
```

Note: `shell.c` is the old single file version of the project, kept only as a
reference. It is disabled by default (its `#if 0` block) so it can never clash
with the modular sources; enable it only to build it standalone.

## File system commands

The kernel has its own file manager. Everything happens inside a **default
directory** (`lavander_home`, created next to where you start the program, and
overridable with the `LAVANDER_HOME` environment variable). `pwd` shows `/`,
which is that directory - you can go as deep as you want (`dir` inside `dir`
inside `dir`) but you can never escape the sandbox, so the same commands behave
identically on Linux and Windows.

| Command             | What it does                                              |
| ------------------- | --------------------------------------------------------- |
| `pwd`               | prints the directory you are in (`/` at the root)         |
| `ls` / `dir [PATH]` | lists a directory (`[DIR]` / `[FILE]`, sorted, with sizes) |
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

## The SLM (Python)

```sh
python littlepenguin.py
```

Requires `tkinter` and `pillow`. It is written with portable APIs only
(`tkinter`, `PIL`, `os.path`), so it works on Linux and Windows unchanged.
