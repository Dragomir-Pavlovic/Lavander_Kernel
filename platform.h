#ifndef LAVANDER_PLATFORM_H
#define LAVANDER_PLATFORM_H

#include <stddef.h>

/*
 * Cross-platform (Linux + Windows) abstraction layer used by the kernel.
 * Every function here has an implementation for POSIX systems and one for
 * Win32, so the rest of the code stays free of #ifdefs.
 */

/* One-time console setup (UTF-8 output on Windows, no-op on Linux). */
void platform_init(void);

/* Console colours, screen clearing and terminal size helpers. */
void platform_set_color(void);
void platform_reset_color(void);
void platform_clear_screen(void);

/*
 * Size of the console window in characters. Returns 1 when the size is
 * known, 0 when it cannot be determined (for example when the output is
 * redirected to a file).
 */
int platform_terminal_size(int *columns, int *rows);

/*
 * Makes sure the console is at least min_columns x min_rows characters.
 * Windows: the console window is grown when needed.
 * Linux  : terminals cannot be resized from inside a program, so nothing
 *          is changed there - the caller has to ask the user to resize.
 * Returns 1 when the console is big enough, 0 when it is not.
 */
int platform_ensure_terminal_size(int min_columns, int min_rows);

/* Blocking sleep expressed in milliseconds. */
void platform_sleep_ms(unsigned int ms);

/* Non blocking keyboard polling (1 = key waiting, 0 = nothing). */
int platform_kbhit(void);

/* Reads a single character without echoing it (raw input). */
int platform_getch(void);

/*
 * System information. Buffers must be at least `size` bytes and are always
 * NUL terminated. Unknown values fall back to "Unknown".
 */
void platform_os_name(char *buffer, size_t size);
void platform_kernel_version(char *buffer, size_t size);
void platform_architecture(char *buffer, size_t size);
void platform_cpu_model(char *buffer, size_t size);

/* Memory report in megabytes. Returns 0 on success, -1 when unavailable. */
int platform_memory_info(unsigned long long *total_mb,
                         unsigned long long *available_mb);

/* Seconds since the machine was booted. Returns 0 when unavailable. */
unsigned long long platform_uptime_seconds(void);

/* ------------------------------------------------------------------ */
/* Filesystem primitives                                               */
/*                                                                     */
/* Paths are native paths (relative to the process working directory   */
/* or absolute). All of them return 0 on success and -1 on failure     */
/* unless stated otherwise.                                            */
/* ------------------------------------------------------------------ */

#define PLATFORM_MAX_NAME 260

typedef struct {
    char name[PLATFORM_MAX_NAME]; /* file or directory name, no path  */
    int is_dir;
    unsigned long long size;      /* bytes, 0 for directories          */
} platform_dirent;

/* Creates one directory (parent must already exist). */
int platform_mkdir(const char *path);

/* Removes one directory (must be empty). */
int platform_rmdir(const char *path);

/* Removes one file. */
int platform_unlink(const char *path);

/* 1 = path exists, 0 = it does not, -1 = could not be determined. */
int platform_path_exists(const char *path);

/*
 * Fills is_dir/size for an existing path.
 * Returns 1 when the path exists, 0 when it does not, -1 on error.
 */
int platform_path_info(const char *path, int *is_dir,
                       unsigned long long *size);

/*
 * Lists the entries of a directory (".", ".." excluded).
 * Returns 0 on success, -1 when the directory cannot be read.
 * *count receives the number of entries written to out.
 */
int platform_dir_list(const char *path, platform_dirent *out, int capacity,
                      int *count);

/* Renames / moves a file or a directory. 0 on success, -1 on failure. */
int platform_rename(const char *from, const char *to);

#endif /* LAVANDER_PLATFORM_H */
