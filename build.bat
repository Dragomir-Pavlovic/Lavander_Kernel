@echo off
REM ============================================================
REM  Lavander Kernel - Windows build script
REM  Tries MinGW gcc first, then MSVC (cl.exe).
REM ============================================================

setlocal enabledelayedexpansion

set "SRCS=main.c config.c commands.c system.c monitor.c history.c parser.c process.c calculator.c notes.c todo.c firesystem.c pake.c sound_system_settings.c platform.c fs.c"
set "TARGET=lavander.exe"

where gcc >NUL 2>&1
if %errorlevel%==0 (
    echo [BUILD] Using MinGW GCC...
    gcc -std=c11 -Wall -Wextra -O2 %SRCS% -o %TARGET% -ladvapi32
    if errorlevel 1 (
        echo [BUILD] GCC build failed.
        exit /b 1
    )
    echo [BUILD] OK: %TARGET%
    exit /b 0
)

where cl >NUL 2>&1
if %errorlevel%==0 (
    echo [BUILD] Using MSVC...
    call cl /nologo /W3 /O2 %SRCS% /Fe:%TARGET% advapi32.lib
    if errorlevel 1 (
        echo [BUILD] MSVC build failed.
        exit /b 1
    )
    del /Q *.obj >NUL 2>&1
    echo [BUILD] OK: %TARGET%
    exit /b 0
)

echo [BUILD] Nema compiler-a na putanji.
echo         Instaliraj MinGW-w64 (https://www.mingw-w64.org) ili MSYS2,
echo         ili pokreni "x64 Native Tools Command Prompt for VS" za MSVC.
exit /b 1
