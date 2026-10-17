/*
 * platform.c - Linux / Windows portability layer for Lavander Kernel.
 *
 * Linux  : uses /proc, termios and ANSI escape codes.
 * Windows: uses the Win32 API (kernel32 / advapi32) for the same things.
 */

/* Feature test macros must come before any system header is included. */
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include "platform.h"

#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <conio.h>
#include <direct.h> /* _mkdir / _rmdir */
#else
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <sys/utsname.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <time.h>
#endif

/* ------------------------------------------------------------------ */
/* Console setup                                                       */
/* ------------------------------------------------------------------ */

void platform_init(void)
{
#ifdef _WIN32
    /* The shell prints box drawing and block characters, so make sure the
       console understands UTF-8 instead of the legacy OEM code page. */
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    /* Windows 10+ terminals understand ANSI escapes; enabling them keeps the
       behaviour identical to Linux. Failure is fine on older systems. */
    HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
    if (out != INVALID_HANDLE_VALUE) {
        DWORD mode = 0;
        if (GetConsoleMode(out, &mode))
            SetConsoleMode(out, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    }
#endif
}

/* ------------------------------------------------------------------ */
/* Colours and screen clearing                                         */
/* ------------------------------------------------------------------ */

#ifdef _WIN32
static HANDLE platform_stdout_handle(void)
{
    HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
    return (handle == INVALID_HANDLE_VALUE) ? NULL : handle;
}
#endif

void platform_set_color(void)
{
#ifdef _WIN32
    HANDLE handle = platform_stdout_handle();
    if (handle != NULL)
        SetConsoleTextAttribute(handle, FOREGROUND_RED | FOREGROUND_BLUE |
                                         FOREGROUND_INTENSITY);
#else
    printf("\x1b[35m"); /* magenta, same as the Linux build */
    fflush(stdout);
#endif
}

void platform_reset_color(void)
{
#ifdef _WIN32
    HANDLE handle = platform_stdout_handle();
    if (handle != NULL)
        SetConsoleTextAttribute(handle, FOREGROUND_RED | FOREGROUND_GREEN |
                                         FOREGROUND_BLUE);
#else
    printf("\x1b[0m");
    fflush(stdout);
#endif
}

void platform_clear_screen(void)
{
#ifdef _WIN32
    HANDLE handle = platform_stdout_handle();
    CONSOLE_SCREEN_BUFFER_INFO info;
    DWORD cells, written;
    COORD origin = { 0, 0 };

    if (handle == NULL)
        return;

    if (GetConsoleScreenBufferInfo(handle, &info)) {
        cells = (DWORD)info.dwSize.X * (DWORD)info.dwSize.Y;
        FillConsoleOutputCharacterA(handle, ' ', cells, origin, &written);
        FillConsoleOutputAttribute(handle, info.wAttributes, cells, origin,
                                   &written);
        SetConsoleCursorPosition(handle, origin);
    }
#else
    /* ANSI clear that also works on dumb terminals. */
    printf("\x1b[2J\x1b[H");
    fflush(stdout);
#endif
}

/* ------------------------------------------------------------------ */
/* Time                                                                */
/* ------------------------------------------------------------------ */

void platform_sleep_ms(unsigned int ms)
{
#ifdef _WIN32
    Sleep(ms);
#else
    usleep((unsigned int)(ms * 1000u));
#endif
}

unsigned long long platform_uptime_seconds(void)
{
#ifdef _WIN32
    return (unsigned long long)(GetTickCount64() / 1000ULL);
#else
    FILE *f = fopen("/proc/uptime", "r");
    double uptime = 0.0;

    if (f) {
        if (fscanf(f, "%lf", &uptime) != 1)
            uptime = 0.0;
        fclose(f);
    }

    return (unsigned long long)uptime;
#endif
}

/* ------------------------------------------------------------------ */
/* Keyboard                                                            */
/* ------------------------------------------------------------------ */

#ifdef _WIN32

int platform_kbhit(void)
{
    return _kbhit();
}

int platform_getch(void)
{
    return _getch();
}

#else

int platform_kbhit(void)
{
    struct termios oldt, newt;
    int ch, oldf;

    if (tcgetattr(STDIN_FILENO, &oldt) != 0)
        return 0;

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

int platform_getch(void)
{
    struct termios oldt, newt;
    int ch;

    if (tcgetattr(STDIN_FILENO, &oldt) != 0)
        return getchar();

    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

    return ch;
}

#endif

/* ------------------------------------------------------------------ */
/* System information                                                  */
/* ------------------------------------------------------------------ */

#ifdef _WIN32

/* Reads a REG_SZ value from HKLM without pulling in extra libraries. */
static int win_read_reg_string(const char *subkey, const char *value_name,
                               char *buffer, size_t size)
{
    HKEY key = NULL;
    DWORD type = 0;
    DWORD bytes = (DWORD)size;
    LONG result;

    if (buffer == NULL || size == 0)
        return -1;

    buffer[0] = '\0';

    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, subkey, 0, KEY_READ, &key) !=
        ERROR_SUCCESS)
        return -1;

    result = RegQueryValueExA(key, value_name, NULL, &type, (LPBYTE)buffer,
                              &bytes);
    RegCloseKey(key);

    if (result != ERROR_SUCCESS || (type != REG_SZ && type != REG_EXPAND_SZ))
        return -1;

    buffer[size - 1] = '\0';
    return 0;
}

/* Windows version numbers are exposed through ntdll at runtime. */
typedef struct {
    ULONG dwOSVersionInfoSize;
    ULONG dwMajorVersion;
    ULONG dwMinorVersion;
    ULONG dwBuildNumber;
    ULONG dwPlatformId;
    WCHAR szCSDVersion[128];
} LAVANDER_RTL_OSVERSIONINFOW;

typedef LONG (WINAPI *RtlGetVersionFn)(LAVANDER_RTL_OSVERSIONINFOW *);

void platform_os_name(char *buffer, size_t size)
{
    char product[128] = "";
    char display[64] = "";

    if (buffer == NULL || size == 0)
        return;

    win_read_reg_string("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion",
                        "ProductName", product, sizeof(product));
    win_read_reg_string("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion",
                        "DisplayVersion", display, sizeof(display));

    if (product[0] != '\0') {
        if (display[0] != '\0')
            snprintf(buffer, size, "%s %s", product, display);
        else
            snprintf(buffer, size, "%s", product);
    } else {
        snprintf(buffer, size, "Windows");
    }
}

void platform_kernel_version(char *buffer, size_t size)
{
    HMODULE ntdll;
    RtlGetVersionFn fn = NULL;
    LAVANDER_RTL_OSVERSIONINFOW info;
    union {
        FARPROC proc;
        RtlGetVersionFn target;
    } caster;

    if (buffer == NULL || size == 0)
        return;

    ntdll = GetModuleHandleA("ntdll.dll");
    if (ntdll != NULL) {
        /* Transfer the address through a union: a direct cast between
           incompatible function pointer types triggers -Wcast-function-type. */
        caster.proc = GetProcAddress(ntdll, "RtlGetVersion");
        fn = caster.target;
    }

    if (fn != NULL) {
        memset(&info, 0, sizeof(info));
        info.dwOSVersionInfoSize = sizeof(info);
        if (fn(&info) == 0) {
            snprintf(buffer, size, "Windows NT %lu.%lu (build %lu)",
                     info.dwMajorVersion, info.dwMinorVersion,
                     info.dwBuildNumber);
            return;
        }
    }

    snprintf(buffer, size, "Windows NT");
}

void platform_architecture(char *buffer, size_t size)
{
    SYSTEM_INFO info;

    if (buffer == NULL || size == 0)
        return;

    GetNativeSystemInfo(&info);

    switch (info.wProcessorArchitecture) {
    case PROCESSOR_ARCHITECTURE_AMD64:
        snprintf(buffer, size, "x86_64");
        break;
    case PROCESSOR_ARCHITECTURE_ARM64:
        snprintf(buffer, size, "aarch64");
        break;
    case PROCESSOR_ARCHITECTURE_INTEL:
        snprintf(buffer, size, "x86");
        break;
    case PROCESSOR_ARCHITECTURE_ARM:
        snprintf(buffer, size, "arm");
        break;
    default:
        snprintf(buffer, size, "Unknown");
        break;
    }
}

void platform_cpu_model(char *buffer, size_t size)
{
    size_t len;

    if (buffer == NULL || size == 0)
        return;

    if (win_read_reg_string(
            "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0",
            "ProcessorNameString", buffer, size) != 0) {
        snprintf(buffer, size, "Unknown");
        return;
    }

    /* The registry value is padded with spaces on most systems. */
    len = strlen(buffer);
    while (len > 0 && (buffer[len - 1] == ' ' || buffer[len - 1] == '\t'))
        buffer[--len] = '\0';
}

int platform_memory_info(unsigned long long *total_mb,
                         unsigned long long *available_mb)
{
    MEMORYSTATUSEX status;

    if (total_mb == NULL || available_mb == NULL)
        return -1;

    memset(&status, 0, sizeof(status));
    status.dwLength = sizeof(status);

    if (GlobalMemoryStatusEx(&status) == 0)
        return -1;

    *total_mb = (unsigned long long)(status.ullTotalPhys / (1024ULL * 1024ULL));
    *available_mb =
        (unsigned long long)(status.ullAvailPhys / (1024ULL * 1024ULL));

    return 0;
}

#else /* POSIX ------------------------------------------------------------------ */

void platform_os_name(char *buffer, size_t size)
{
    FILE *f;
    char line[256];

    if (buffer == NULL || size == 0)
        return;

    snprintf(buffer, size, "Linux");

    f = fopen("/etc/os-release", "r");
    if (f == NULL)
        return;

    while (fgets(line, sizeof(line), f)) {
        if (strncmp(line, "PRETTY_NAME=", 12) == 0) {
            char *p = line + 12;
            char *end;

            if (*p == '"')
                p++;

            end = p + strlen(p);
            while (end > p && (end[-1] == '\n' || end[-1] == '\r' ||
                               end[-1] == '"'))
                end--;
            *end = '\0';

            snprintf(buffer, size, "%s", p);
            break;
        }
    }

    fclose(f);
}

void platform_kernel_version(char *buffer, size_t size)
{
    struct utsname info;

    if (buffer == NULL || size == 0)
        return;

    if (uname(&info) == 0)
        snprintf(buffer, size, "%s %s", info.release, info.version);
    else
        snprintf(buffer, size, "Unknown");
}

void platform_architecture(char *buffer, size_t size)
{
    struct utsname info;

    if (buffer == NULL || size == 0)
        return;

    if (uname(&info) == 0)
        snprintf(buffer, size, "%s", info.machine);
    else
        snprintf(buffer, size, "Unknown");
}

void platform_cpu_model(char *buffer, size_t size)
{
    FILE *f;
    char line[256];

    if (buffer == NULL || size == 0)
        return;

    snprintf(buffer, size, "Unknown");

    f = fopen("/proc/cpuinfo", "r");
    if (f == NULL)
        return;

    while (fgets(line, sizeof(line), f)) {
        if (strncmp(line, "model name", 10) == 0) {
            char *p = strchr(line, ':');

            if (p) {
                p++;
                while (*p == ' ')
                    p++;
                p[strcspn(p, "\n")] = '\0';
                snprintf(buffer, size, "%s", p);
                break;
            }
        }
    }

    fclose(f);
}

int platform_memory_info(unsigned long long *total_mb,
                         unsigned long long *available_mb)
{
    FILE *f;
    char line[256];
    unsigned long long total_kb = 0;
    unsigned long long available_kb = 0;

    if (total_mb == NULL || available_mb == NULL)
        return -1;

    f = fopen("/proc/meminfo", "r");
    if (f == NULL)
        return -1;

    while (fgets(line, sizeof(line), f)) {
        if (sscanf(line, "MemTotal: %llu kB", &total_kb) == 1)
            continue;
        if (sscanf(line, "MemAvailable: %llu kB", &available_kb) == 1)
            continue;
    }

    fclose(f);

    if (total_kb == 0)
        return -1;

    *total_mb = total_kb / 1024ULL;
    *available_mb = available_kb / 1024ULL;

    return 0;
}

#endif

/* ------------------------------------------------------------------ */
/* Filesystem                                                          */
/* ------------------------------------------------------------------ */

#ifdef _WIN32

/* The CRT accepts forward slashes, but Win32 calls are happier with '\'. */
static void win_native_path(const char *input, char *output, size_t size)
{
    size_t i = 0;

    if (output == NULL || size == 0)
        return;

    while (input != NULL && input[i] != '\0' && i + 1 < size) {
        output[i] = (input[i] == '/') ? '\\' : input[i];
        i++;
    }

    output[i] = '\0';
}

int platform_mkdir(const char *path)
{
    char native[1024];

    win_native_path(path, native, sizeof(native));
    return (_mkdir(native) == 0) ? 0 : -1;
}

int platform_rmdir(const char *path)
{
    char native[1024];

    win_native_path(path, native, sizeof(native));
    return (_rmdir(native) == 0) ? 0 : -1;
}

int platform_unlink(const char *path)
{
    char native[1024];

    win_native_path(path, native, sizeof(native));
    return (remove(native) == 0) ? 0 : -1;
}

int platform_path_exists(const char *path)
{
    char native[1024];

    win_native_path(path, native, sizeof(native));
    return (GetFileAttributesA(native) != INVALID_FILE_ATTRIBUTES) ? 1 : 0;
}

int platform_path_info(const char *path, int *is_dir,
                       unsigned long long *size)
{
    char native[1024];
    WIN32_FILE_ATTRIBUTE_DATA data;

    win_native_path(path, native, sizeof(native));

    if (GetFileAttributesExA(native, GetFileExInfoStandard, &data) == 0)
        return 0;

    if (is_dir != NULL)
        *is_dir = (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? 1 : 0;

    if (size != NULL)
        *size = ((unsigned long long)data.nFileSizeHigh << 32) |
                (unsigned long long)data.nFileSizeLow;

    return 1;
}

int platform_dir_list(const char *path, platform_dirent *out, int capacity,
                      int *count)
{
    char native[1024];
    char pattern[1100];
    WIN32_FIND_DATAA data;
    HANDLE handle;
    int used = 0;

    if (out == NULL || capacity <= 0 || count == NULL)
        return -1;

    *count = 0;

    win_native_path(path, native, sizeof(native));
    snprintf(pattern, sizeof(pattern), "%s\\*", native);

    handle = FindFirstFileA(pattern, &data);
    if (handle == INVALID_HANDLE_VALUE)
        return -1;

    do {
        if (strcmp(data.cFileName, ".") == 0 ||
            strcmp(data.cFileName, "..") == 0)
            continue;

        if (used >= capacity)
            break;

        snprintf(out[used].name, sizeof(out[used].name), "%s", data.cFileName);
        out[used].is_dir =
            (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? 1 : 0;
        out[used].size = ((unsigned long long)data.nFileSizeHigh << 32) |
                         (unsigned long long)data.nFileSizeLow;
        used++;
    } while (FindNextFileA(handle, &data));

    FindClose(handle);
    *count = used;

    return 0;
}

int platform_rename(const char *from, const char *to)
{
    char src[1024];
    char dst[1024];

    win_native_path(from, src, sizeof(src));
    win_native_path(to, dst, sizeof(dst));

    /* MoveFileEx works for files and directories alike. */
    return MoveFileExA(src, dst, 0) ? 0 : -1;
}

#else /* POSIX ------------------------------------------------------------- */

int platform_mkdir(const char *path)
{
    return (mkdir(path, 0755) == 0) ? 0 : -1;
}

int platform_rmdir(const char *path)
{
    return (rmdir(path) == 0) ? 0 : -1;
}

int platform_unlink(const char *path)
{
    return (remove(path) == 0) ? 0 : -1;
}

int platform_path_exists(const char *path)
{
    struct stat info;

    return (stat(path, &info) == 0) ? 1 : 0;
}

int platform_path_info(const char *path, int *is_dir,
                       unsigned long long *size)
{
    struct stat info;

    if (stat(path, &info) != 0)
        return 0;

    if (is_dir != NULL)
        *is_dir = S_ISDIR(info.st_mode) ? 1 : 0;

    if (size != NULL)
        *size = (unsigned long long)info.st_size;

    return 1;
}

int platform_dir_list(const char *path, platform_dirent *out, int capacity,
                      int *count)
{
    DIR *dir;
    struct dirent *entry;
    int used = 0;

    if (out == NULL || capacity <= 0 || count == NULL)
        return -1;

    *count = 0;

    dir = opendir(path);
    if (dir == NULL)
        return -1;

    while ((entry = readdir(dir)) != NULL) {
        char full[1024];
        struct stat info;
        int is_dir = 0;
        unsigned long long size = 0;

        if (strcmp(entry->d_name, ".") == 0 ||
            strcmp(entry->d_name, "..") == 0)
            continue;

        if (used >= capacity)
            break;

#ifdef DT_DIR
        if (entry->d_type == DT_DIR)
            is_dir = 1;
        else if (entry->d_type == DT_REG)
            is_dir = 0;
        else
#endif
        {
            snprintf(full, sizeof(full), "%s/%s", path, entry->d_name);
            if (stat(full, &info) == 0) {
                is_dir = S_ISDIR(info.st_mode) ? 1 : 0;
                size = (unsigned long long)info.st_size;
            }
        }

        snprintf(out[used].name, sizeof(out[used].name), "%s", entry->d_name);
        out[used].is_dir = is_dir;
        out[used].size = is_dir ? 0 : size;
        used++;
    }

    closedir(dir);
    *count = used;

    return 0;
}

int platform_rename(const char *from, const char *to)
{
    return (rename(from, to) == 0) ? 0 : -1;
}

#endif
