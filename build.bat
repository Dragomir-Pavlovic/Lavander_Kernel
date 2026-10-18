@echo off
REM ============================================================
REM  Lavander Kernel + games - Windows build script
REM
REM  Produces:
REM    lavander.exe                    - the kernel
REM    lavander_games\snake.game.exe   - the bundled snake game
REM    lavander_games\tetris.game.exe  - the bundled tetris game
REM
REM  Tries MinGW gcc first, then MSVC (cl.exe).
REM ============================================================

setlocal

set "SRCS=main.c config.c commands.c system.c monitor.c history.c parser.c process.c calculator.c notes.c todo.c firesystem.c sound_system_settings.c platform.c fs.c"
set "TARGET=lavander.exe"
set "GAMEDIR=lavander_games"
set "GAMESRC=games\snake_game.c platform.c"
set "TETRISSRC=games\tetris_game.c platform.c"

if not exist "%GAMEDIR%" mkdir "%GAMEDIR%"

where gcc >NUL 2>&1
if %errorlevel%==0 goto :gcc

where cl >NUL 2>&1
if %errorlevel%==0 goto :msvc

echo [BUILD] Nema compiler-a na putanji.
echo         Instaliraj MinGW-w64 (https://www.mingw-w64.org) ili MSYS2,
echo         ili pokreni "x64 Native Tools Command Prompt for VS" za MSVC.
exit /b 1

:gcc
echo [BUILD] Using MinGW GCC...
gcc -std=c11 -Wall -Wextra -O2 %SRCS% -o %TARGET% -ladvapi32
if errorlevel 1 goto :failed
echo [BUILD] OK: %TARGET%

echo [BUILD] Building snake.game...
gcc -std=c11 -Wall -Wextra -O2 -I. %GAMESRC% -o "%GAMEDIR%\snake.game.exe" -ladvapi32
if errorlevel 1 goto :failed
echo [BUILD] OK: %GAMEDIR%\snake.game.exe

echo [BUILD] Building tetris.game...
gcc -std=c11 -Wall -Wextra -O2 -I. %TETRISSRC% -o "%GAMEDIR%\tetris.game.exe" -ladvapi32
if errorlevel 1 goto :failed
echo [BUILD] OK: %GAMEDIR%\tetris.game.exe
exit /b 0

:msvc
echo [BUILD] Using MSVC...
call cl /nologo /W3 /O2 %SRCS% /Fe:%TARGET% advapi32.lib
if errorlevel 1 goto :failed
echo [BUILD] OK: %TARGET%

echo [BUILD] Building snake.game...
call cl /nologo /W3 /O2 /I. %GAMESRC% /Fe:"%GAMEDIR%\snake.game.exe" advapi32.lib
if errorlevel 1 goto :failed
echo [BUILD] OK: %GAMEDIR%\snake.game.exe

echo [BUILD] Building tetris.game...
call cl /nologo /W3 /O2 /I. %TETRISSRC% /Fe:"%GAMEDIR%\tetris.game.exe" advapi32.lib
if errorlevel 1 goto :failed
echo [BUILD] OK: %GAMEDIR%\tetris.game.exe
del /Q *.obj >NUL 2>&1
exit /b 0

:failed
echo [BUILD] Build failed.
exit /b 1
