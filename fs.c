/*
 * fs.c - cross platform filesystem commands for Lavander Kernel.
 *
 * Everything works relative to a default directory (root) created at
 * startup, so the same commands behave identically on Linux and Windows.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "config.h"
#include "fs.h"

/* ------------------------------------------------------------------ */
/* State                                                               */
/* ------------------------------------------------------------------ */

static char fs_root[FS_MAX_PATH];      /* native path of the root dir */
static char fs_cwd_rel[FS_MAX_PATH];   /* "" = root, else "docs/sub"  */

/* ------------------------------------------------------------------ */
/* Small helpers                                                       */
/* ------------------------------------------------------------------ */

static const char *skip_spaces(const char *s)
{
    while (s != NULL && (*s == ' ' || *s == '\t'))
        s++;
    return s;
}

/*
 * Reads one token (optionally double quoted) into out.
 * Returns a pointer just after the token.
 */
static const char *next_token(const char *s, char *out, size_t out_size)
{
    size_t n = 0;

    if (out_size == 0)
        return s;

    s = skip_spaces(s);

    if (*s == '"') {
        s++;
        while (*s != '\0' && *s != '"') {
            if (n + 1 < out_size)
                out[n++] = *s;
            s++;
        }
        if (*s == '"')
            s++;
    } else {
        while (*s != '\0' && *s != ' ' && *s != '\t') {
            if (n + 1 < out_size)
                out[n++] = *s;
            s++;
        }
    }

    out[n] = '\0';
    return s;
}

static int ci_strcmp(const char *a, const char *b)
{
    for (;; a++, b++) {
        int ca = tolower((unsigned char)*a);
        int cb = tolower((unsigned char)*b);

        if (ca != cb || ca == 0)
            return ca - cb;
    }
}

static void format_path(const char *rel, char *out, size_t out_size)
{
    if (rel == NULL || rel[0] == '\0')
        snprintf(out, out_size, "/");
    else
        snprintf(out, out_size, "/%s", rel);
}

/*
 * Joins two path pieces with a '/' - never uses snprintf with overlapping
 * buffers and never truncates silently.
 * Returns 0 on success, -1 when the result would not fit.
 */
static int join_path(char *out, size_t out_size, const char *first,
                     const char *second)
{
    size_t first_len = strlen(first);
    size_t second_len = strlen(second);
    int separator = (first_len > 0) ? 1 : 0;

    if (first_len + (size_t)separator + second_len + 1 > out_size)
        return -1;

    memcpy(out, first, first_len);
    if (separator)
        out[first_len] = '/';
    memcpy(out + first_len + (size_t)separator, second, second_len + 1);

    return 0;
}

/*
 * Characters that are not allowed in a name. The list is the same on both
 * platforms so a directory tree stays portable between Linux and Windows.
 */
static int name_valid(const char *name)
{
    size_t i;

    if (name == NULL || name[0] == '\0')
        return 0;

    if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0)
        return 0;

    if (strchr(name, '/') != NULL || strchr(name, '\\') != NULL)
        return 0;

    for (i = 0; name[i] != '\0'; i++) {
        unsigned char c = (unsigned char)name[i];

        if (c < 32 || strchr("<>:\"|?*", (int)c) != NULL)
            return 0;
    }

#ifdef _WIN32
    /* Windows specific rules: trailing space/dot and device names. */
    {
        static const char *reserved[] = {
            "CON", "PRN", "AUX", "NUL",
            "COM1", "COM2", "COM3", "COM4", "COM5", "COM6", "COM7", "COM8",
            "COM9", "LPT1", "LPT2", "LPT3", "LPT4", "LPT5", "LPT6", "LPT7",
            "LPT8", "LPT9", NULL
        };
        size_t len = strlen(name);

        if (name[len - 1] == ' ' || name[len - 1] == '.')
            return 0;

        for (i = 0; reserved[i] != NULL; i++) {
            if (ci_strcmp(name, reserved[i]) == 0)
                return 0;
        }
    }
#endif

    return 1;
}

/* Validates every single component of a relative path. */
static int components_valid(const char *rel)
{
    const char *p = rel;

    while (*p != '\0') {
        const char *start = p;
        char component[PLATFORM_MAX_NAME];
        size_t n;

        while (*p != '\0' && *p != '/')
            p++;

        n = (size_t)(p - start);
        if (n == 0) {
            if (*p == '/')
                p++;
            continue;
        }

        if (n >= sizeof(component))
            return 0;

        memcpy(component, start, n);
        component[n] = '\0';

        if (!name_valid(component))
            return 0;

        if (*p == '/')
            p++;
    }

    return 1;
}

/* ------------------------------------------------------------------ */
/* Path handling                                                       */
/* ------------------------------------------------------------------ */

/*
 * Collapses ".", ".." and duplicate separators.
 * Input may be absolute ("/x/y") or relative ("x/y").
 * Output is relative to the root, "" means the root itself.
 */
static int normalize_path(const char *input, char *out, size_t out_size)
{
    char buffer[FS_MAX_PATH];
    char *parts[128];
    char *p;
    int count = 0;
    size_t i;
    size_t pos = 0;

    snprintf(buffer, sizeof(buffer), "%s", input != NULL ? input : "");

#ifdef _WIN32
    for (i = 0; buffer[i] != '\0'; i++) {
        if (buffer[i] == '\\')
            buffer[i] = '/';
    }
#endif

    p = buffer;
    while (*p != '\0') {
        char *start = p;

        while (*p != '\0' && *p != '/')
            p++;

        if (*p == '/') {
            *p = '\0';
            p++;
        }

        if (start[0] == '\0' || strcmp(start, ".") == 0)
            continue;

        if (strcmp(start, "..") == 0) {
            if (count > 0)
                count--;
            continue;
        }

        if (count == (int)(sizeof(parts) / sizeof(parts[0])))
            return -1;

        parts[count++] = start;
    }

    if (count == 0) {
        if (out_size > 0)
            out[0] = '\0';
        return 0;
    }

    for (i = 0; i < (size_t)count; i++) {
        int written = snprintf(out + pos, out_size - pos, "%s%s",
                               i > 0 ? "/" : "", parts[i]);

        if (written < 0 || (size_t)written >= out_size - pos)
            return -1;

        pos += (size_t)written;
    }

    return 0;
}

/*
 * Turns user input into a path relative to the root.
 * Paths starting with "/" (or "~") are taken from the root, everything
 * else is resolved against the current working directory.
 */
static int resolve_path(const char *input, char *out, size_t out_size)
{
    char combined[FS_MAX_PATH];

    if (input == NULL || input[0] == '\0') {
        if (fs_cwd_rel[0] == '\0') {
            if (out_size > 0)
                out[0] = '\0';
        } else {
            snprintf(out, out_size, "%s", fs_cwd_rel);
        }
        return 0;
    }

    if (input[0] == '/' || input[0] == '\\' || input[0] == '~')
        return normalize_path(input, out, out_size);

    if (join_path(combined, sizeof(combined),
                  fs_cwd_rel[0] == '\0' ? "" : fs_cwd_rel, input) != 0)
        return -1;

    return normalize_path(combined, out, out_size);
}

/* Builds the native path (root + relative path). */
static int build_native(const char *rel, char *out, size_t out_size)
{
    int written;

    if (rel == NULL || rel[0] == '\0')
        written = snprintf(out, out_size, "%s", fs_root);
    else
        written = snprintf(out, out_size, "%s/%s", fs_root, rel);

    if (written < 0 || (size_t)written >= out_size)
        return -1;

    return 0;
}

/* Returns the last component of a relative path. */
static const char *base_name(const char *rel)
{
    const char *slash = strrchr(rel, '/');

    return (slash != NULL) ? slash + 1 : rel;
}

/* 0 when the parent directory of rel exists, -1 when it does not. */
static int parent_exists(const char *rel)
{
    char parent[FS_MAX_PATH];
    char native[FS_MAX_PATH];
    const char *slash;
    int is_dir = 0;

    slash = strrchr(rel, '/');
    if (slash == NULL)
        return 0; /* the root always exists */

    if ((size_t)(slash - rel) >= sizeof(parent))
        return -1;

    memcpy(parent, rel, (size_t)(slash - rel));
    parent[slash - rel] = '\0';

    if (build_native(parent, native, sizeof(native)) != 0)
        return -1;

    if (platform_path_info(native, &is_dir, NULL) != 1 || !is_dir)
        return -1;

    return 0;
}

/* Creates a directory and every missing parent above it. */
static int make_dirs(const char *native_path)
{
    char buffer[FS_MAX_PATH];
    size_t i;
    size_t len;

    snprintf(buffer, sizeof(buffer), "%s", native_path);
    len = strlen(buffer);

    for (i = 1; i < len; i++) {
        if (buffer[i] == '/' || buffer[i] == '\\') {
            char saved = buffer[i];

            buffer[i] = '\0';
            if (!platform_path_exists(buffer))
                platform_mkdir(buffer);
            buffer[i] = saved;
        }
    }

    if (platform_path_exists(buffer))
        return 0;

    return platform_mkdir(buffer);
}

/* Keeps the working directory valid after something below it was removed. */
static void fix_cwd(const char *removed_rel)
{
    size_t n = strlen(removed_rel);

    if (fs_cwd_rel[0] == '\0' || n == 0)
        return;

    if (strcmp(fs_cwd_rel, removed_rel) == 0 ||
        (strncmp(fs_cwd_rel, removed_rel, n) == 0 && fs_cwd_rel[n] == '/')) {
        fs_cwd_rel[0] = '\0';
        printf("[FS] Working directory was removed - switched back to /\n");
    }
}

/* ------------------------------------------------------------------ */
/* Commands                                                            */
/* ------------------------------------------------------------------ */

static int cmp_entry(const void *a, const void *b)
{
    const platform_dirent *x = (const platform_dirent *)a;
    const platform_dirent *y = (const platform_dirent *)b;

    if (x->is_dir != y->is_dir)
        return y->is_dir - x->is_dir;

    return ci_strcmp(x->name, y->name);
}

static int cmd_ls(const char *arg)
{
    char rel[FS_MAX_PATH];
    char native[FS_MAX_PATH];
    char display[FS_MAX_PATH + 2];
    platform_dirent *entries;
    int count = 0;
    int dirs = 0;
    int files = 0;
    int i;

    if (resolve_path(arg, rel, sizeof(rel)) != 0) {
        printf("[FS] Path is too long.\n");
        return 1;
    }

    if (build_native(rel, native, sizeof(native)) != 0) {
        printf("[FS] Path is too long.\n");
        return 1;
    }

    entries = (platform_dirent *)malloc(FS_MAX_ENTRIES * sizeof(*entries));
    if (entries == NULL) {
        printf("[FS] Out of memory.\n");
        return 1;
    }

    if (platform_dir_list(native, entries, FS_MAX_ENTRIES, &count) != 0) {
        int is_dir = 0;

        if (platform_path_info(native, &is_dir, NULL) == 1 && !is_dir)
            printf("[FS] Not a directory: %s\n", arg);
        else
            printf("[FS] Cannot read directory: %s\n", arg);

        free(entries);
        return 1;
    }

    qsort(entries, (size_t)count, sizeof(*entries), cmp_entry);

    format_path(rel, display, sizeof(display));
    printf(" Directory: %s\n", display);
    printf(" ----------------------------------------------------------\n");
    if (rel[0] != '\0')
        printf(" [DIR]  ..\n");

    for (i = 0; i < count; i++) {
        if (entries[i].is_dir) {
            printf(" [DIR]  %s\n", entries[i].name);
            dirs++;
        } else {
            printf(" [FILE] %-28s %10llu B\n", entries[i].name,
                   entries[i].size);
            files++;
        }
    }

    printf(" ----------------------------------------------------------\n");
    printf(" %d entr%s (%d dir%s, %d file%s)\n", count, count == 1 ? "y" : "ies",
           dirs, dirs == 1 ? "" : "s", files, files == 1 ? "" : "s");

    free(entries);
    return 1;
}

static int cmd_cd(const char *arg)
{
    char rel[FS_MAX_PATH];
    char native[FS_MAX_PATH];
    char display[FS_MAX_PATH + 2];
    int is_dir = 0;

    /* "cd" or "cd ~" goes back to the default directory. */
    if (arg == NULL || arg[0] == '\0' || arg[0] == '~') {
        fs_cwd_rel[0] = '\0';
        printf(" Working directory: /\n");
        return 1;
    }

    if (resolve_path(arg, rel, sizeof(rel)) != 0) {
        printf("[FS] Path is too long.\n");
        return 1;
    }

    if (rel[0] == '\0') {
        fs_cwd_rel[0] = '\0';
        printf(" Working directory: /\n");
        return 1;
    }

    if (build_native(rel, native, sizeof(native)) != 0) {
        printf("[FS] Path is too long.\n");
        return 1;
    }

    if (platform_path_info(native, &is_dir, NULL) != 1) {
        printf("[FS] No such directory: %s\n", arg);
        return 1;
    }

    if (!is_dir) {
        printf("[FS] Not a directory: %s\n", arg);
        return 1;
    }

    snprintf(fs_cwd_rel, sizeof(fs_cwd_rel), "%s", rel);
    format_path(fs_cwd_rel, display, sizeof(display));
    printf(" Working directory: %s\n", display);
    return 1;
}

static int cmd_pwd(void)
{
    char display[FS_MAX_PATH + 2];

    format_path(fs_cwd_rel, display, sizeof(display));
    printf("%s\n", display);
    return 1;
}

static int cmd_mkdir(const char *arg)
{
    char rel[FS_MAX_PATH];
    char native[FS_MAX_PATH];
    char display[FS_MAX_PATH + 2];

    if (arg == NULL || arg[0] == '\0') {
        printf(" Usage: mkdir DIRECTORY  (several levels are allowed)\n");
        return 1;
    }

    if (resolve_path(arg, rel, sizeof(rel)) != 0) {
        printf("[FS] Path is too long.\n");
        return 1;
    }

    if (rel[0] == '\0') {
        printf("[FS] The default directory already exists.\n");
        return 1;
    }

    if (!components_valid(rel)) {
        printf("[FS] Invalid name: %s\n", arg);
        return 1;
    }

    if (build_native(rel, native, sizeof(native)) != 0) {
        printf("[FS] Path is too long.\n");
        return 1;
    }

    if (platform_path_exists(native)) {
        printf("[FS] Already exists: %s\n", arg);
        return 1;
    }

    if (make_dirs(native) != 0) {
        printf("[FS] Cannot create directory: %s\n", arg);
        return 1;
    }

    format_path(rel, display, sizeof(display));
    printf("[FS] Created directory: %s\n", display);
    return 1;
}

static int cmd_rmdir(const char *arg)
{
    char rel[FS_MAX_PATH];
    char native[FS_MAX_PATH];
    char display[FS_MAX_PATH + 2];
    platform_dirent *entries;
    int is_dir = 0;
    int count = 0;

    if (arg == NULL || arg[0] == '\0') {
        printf(" Usage: rmdir DIRECTORY\n");
        return 1;
    }

    if (resolve_path(arg, rel, sizeof(rel)) != 0 || rel[0] == '\0') {
        printf("[FS] The default directory cannot be removed.\n");
        return 1;
    }

    if (build_native(rel, native, sizeof(native)) != 0) {
        printf("[FS] Path is too long.\n");
        return 1;
    }

    if (platform_path_info(native, &is_dir, NULL) != 1) {
        printf("[FS] No such directory: %s\n", arg);
        return 1;
    }

    if (!is_dir) {
        printf("[FS] Not a directory (use rm FILE): %s\n", arg);
        return 1;
    }

    entries = (platform_dirent *)malloc(64 * sizeof(*entries));
    if (entries != NULL) {
        platform_dir_list(native, entries, 64, &count);
        free(entries);
    }

    if (count > 0) {
        format_path(rel, display, sizeof(display));
        printf("[FS] Directory is not empty (%d entr%s): %s\n", count,
               count == 1 ? "y" : "ies", display);
        printf("     Use: rm -r %s\n", display);
        return 1;
    }

    if (platform_rmdir(native) != 0) {
        printf("[FS] Cannot remove directory: %s\n", arg);
        return 1;
    }

    format_path(rel, display, sizeof(display));
    printf("[FS] Removed directory: %s\n", display);
    fix_cwd(rel);
    return 1;
}

/* Recursively deletes a file or directory. */
static int remove_tree(const char *native, int depth)
{
    platform_dirent *entries;
    int count = 0;
    int i;
    int result = 0;

    if (depth > 32)
        return -1;

    entries = (platform_dirent *)malloc(FS_MAX_ENTRIES * sizeof(*entries));
    if (entries == NULL)
        return -1;

    if (platform_dir_list(native, entries, FS_MAX_ENTRIES, &count) != 0) {
        free(entries);
        return platform_unlink(native);
    }

    for (i = 0; i < count; i++) {
        char child[FS_MAX_PATH * 2];

        snprintf(child, sizeof(child), "%s/%s", native, entries[i].name);

        if (entries[i].is_dir) {
            if (remove_tree(child, depth + 1) != 0)
                result = -1;
        } else if (platform_unlink(child) != 0) {
            result = -1;
        }
    }

    free(entries);

    if (platform_rmdir(native) != 0)
        result = -1;

    return result;
}

static int cmd_rm(const char *arg, int recursive)
{
    char rel[FS_MAX_PATH];
    char native[FS_MAX_PATH];
    char display[FS_MAX_PATH + 2];
    int is_dir = 0;

    if (arg == NULL || arg[0] == '\0') {
        printf(" Usage: rm FILE      - delete a file\n");
        printf("        rm -r DIR    - delete a directory and its contents\n");
        return 1;
    }

    if (resolve_path(arg, rel, sizeof(rel)) != 0) {
        printf("[FS] Path is too long.\n");
        return 1;
    }

    if (rel[0] == '\0') {
        printf("[FS] The default directory cannot be removed.\n");
        return 1;
    }

    if (build_native(rel, native, sizeof(native)) != 0) {
        printf("[FS] Path is too long.\n");
        return 1;
    }

    if (platform_path_info(native, &is_dir, NULL) != 1) {
        printf("[FS] No such file or directory: %s\n", arg);
        return 1;
    }

    format_path(rel, display, sizeof(display));

    if (is_dir && !recursive) {
        printf("[FS] Is a directory (use rm -r): %s\n", display);
        return 1;
    }

    if (remove_tree(native, 0) != 0) {
        printf("[FS] Cannot delete: %s\n", display);
        return 1;
    }

    printf("[FS] Deleted: %s\n", display);
    fix_cwd(rel);
    return 1;
}

static int cmd_touch(const char *arg)
{
    char rel[FS_MAX_PATH];
    char native[FS_MAX_PATH];
    char display[FS_MAX_PATH + 2];
    int is_dir = 0;
    FILE *f;

    if (arg == NULL || arg[0] == '\0') {
        printf(" Usage: touch FILE\n");
        return 1;
    }

    if (resolve_path(arg, rel, sizeof(rel)) != 0 || rel[0] == '\0') {
        printf("[FS] Invalid file name.\n");
        return 1;
    }

    if (!components_valid(rel)) {
        printf("[FS] Invalid name: %s\n", arg);
        return 1;
    }

    if (build_native(rel, native, sizeof(native)) != 0) {
        printf("[FS] Path is too long.\n");
        return 1;
    }

    if (platform_path_info(native, &is_dir, NULL) == 1) {
        format_path(rel, display, sizeof(display));
        printf("[FS] Already exists: %s%s\n", display,
               is_dir ? " (directory)" : "");
        return 1;
    }

    if (parent_exists(rel) != 0) {
        printf("[FS] Parent directory does not exist: %s\n", arg);
        return 1;
    }

    f = fopen(native, "a");
    if (f == NULL) {
        printf("[FS] Cannot create file: %s\n", arg);
        return 1;
    }
    fclose(f);

    format_path(rel, display, sizeof(display));
    printf("[FS] Created file: %s\n", display);
    return 1;
}

static int cmd_cat(const char *arg)
{
    char rel[FS_MAX_PATH];
    char native[FS_MAX_PATH];
    char line[1024];
    int is_dir = 0;
    int empty = 1;
    int ends_with_newline = 1;
    FILE *f;

    if (arg == NULL || arg[0] == '\0') {
        printf(" Usage: cat FILE\n");
        return 1;
    }

    if (resolve_path(arg, rel, sizeof(rel)) != 0 || rel[0] == '\0') {
        printf("[FS] Invalid file name.\n");
        return 1;
    }

    if (build_native(rel, native, sizeof(native)) != 0) {
        printf("[FS] Path is too long.\n");
        return 1;
    }

    if (platform_path_info(native, &is_dir, NULL) != 1) {
        printf("[FS] No such file: %s\n", arg);
        return 1;
    }

    if (is_dir) {
        printf("[FS] Is a directory: %s\n", arg);
        return 1;
    }

    f = fopen(native, "r");
    if (f == NULL) {
        printf("[FS] Cannot open file: %s\n", arg);
        return 1;
    }

    while (fgets(line, sizeof(line), f) != NULL) {
        size_t len = strlen(line);

        empty = 0;
        ends_with_newline = (len > 0 && line[len - 1] == '\n');
        fputs(line, stdout);
    }

    fclose(f);

    if (empty)
        printf("(empty file)\n");
    else if (!ends_with_newline)
        putchar('\n');

    return 1;
}

static int cmd_write(const char *arg)
{
    char rel[FS_MAX_PATH];
    char native[FS_MAX_PATH];
    char display[FS_MAX_PATH + 2];
    char path_arg[FS_MAX_PATH];
    const char *text;
    int is_dir = 0;
    FILE *f;

    arg = next_token(arg, path_arg, sizeof(path_arg));
    text = skip_spaces(arg);

    if (path_arg[0] == '\0') {
        printf(" Usage: write FILE TEXT\n");
        return 1;
    }

    if (text == NULL || *text == '\0') {
        printf("[FS] Nothing to write. Usage: write FILE TEXT\n");
        return 1;
    }

    if (resolve_path(path_arg, rel, sizeof(rel)) != 0 || rel[0] == '\0' ||
        !components_valid(rel)) {
        printf("[FS] Invalid file name: %s\n", path_arg);
        return 1;
    }

    if (build_native(rel, native, sizeof(native)) != 0) {
        printf("[FS] Path is too long.\n");
        return 1;
    }

    if (platform_path_info(native, &is_dir, NULL) == 1 && is_dir) {
        printf("[FS] Is a directory: %s\n", path_arg);
        return 1;
    }

    if (parent_exists(rel) != 0) {
        printf("[FS] Parent directory does not exist: %s\n", path_arg);
        return 1;
    }

    f = fopen(native, "w");
    if (f == NULL) {
        printf("[FS] Cannot write file: %s\n", path_arg);
        return 1;
    }

    fputs(text, f);
    fputc('\n', f);
    fclose(f);

    format_path(rel, display, sizeof(display));
    printf("[FS] Wrote: %s\n", display);
    return 1;
}

static int cmd_mv(const char *arg)
{
    char src_arg[FS_MAX_PATH];
    char dst_arg[FS_MAX_PATH];
    char rel_src[FS_MAX_PATH];
    char rel_dst[FS_MAX_PATH];
    char native_src[FS_MAX_PATH];
    char native_dst[FS_MAX_PATH];
    char display[FS_MAX_PATH + 2];
    int is_dir = 0;
    int is_dir_dst = 0;

    arg = next_token(arg, src_arg, sizeof(src_arg));
    arg = next_token(arg, dst_arg, sizeof(dst_arg));

    if (src_arg[0] == '\0' || dst_arg[0] == '\0') {
        printf(" Usage: mv SOURCE DESTINATION\n");
        printf("        mv FILE DIR    - move a file into a directory\n");
        return 1;
    }

    if (resolve_path(src_arg, rel_src, sizeof(rel_src)) != 0 ||
        rel_src[0] == '\0') {
        printf("[FS] Invalid source: %s\n", src_arg);
        return 1;
    }

    if (build_native(rel_src, native_src, sizeof(native_src)) != 0) {
        printf("[FS] Path is too long.\n");
        return 1;
    }

    if (platform_path_info(native_src, &is_dir, NULL) != 1) {
        printf("[FS] No such file or directory: %s\n", src_arg);
        return 1;
    }

    if (resolve_path(dst_arg, rel_dst, sizeof(rel_dst)) != 0) {
        printf("[FS] Path is too long.\n");
        return 1;
    }

    if (build_native(rel_dst, native_dst, sizeof(native_dst)) != 0) {
        printf("[FS] Path is too long.\n");
        return 1;
    }

    /* Moving into an existing directory keeps the original name. */
    if (platform_path_info(native_dst, &is_dir_dst, NULL) == 1 && is_dir_dst) {
        const char *base = base_name(rel_src);
        char joined[FS_MAX_PATH];

        /* Built in a separate buffer: join_path must never overlap. */
        if (join_path(joined, sizeof(joined),
                      rel_dst[0] == '\0' ? "" : rel_dst, base) != 0) {
            printf("[FS] Path is too long.\n");
            return 1;
        }

        snprintf(rel_dst, sizeof(rel_dst), "%s", joined);

        if (build_native(rel_dst, native_dst, sizeof(native_dst)) != 0) {
            printf("[FS] Path is too long.\n");
            return 1;
        }
    }

    if (strcmp(rel_src, rel_dst) == 0) {
        printf("[FS] Source and destination are the same.\n");
        return 1;
    }

    if (is_dir) {
        size_t src_len = strlen(rel_src);

        if (strncmp(rel_dst, rel_src, src_len) == 0 &&
            (rel_dst[src_len] == '\0' || rel_dst[src_len] == '/')) {
            printf("[FS] Cannot move a directory into itself.\n");
            return 1;
        }
    }

    if (parent_exists(rel_dst) != 0) {
        printf("[FS] Parent directory does not exist: %s\n", dst_arg);
        return 1;
    }

    if (platform_path_exists(native_dst)) {
        printf("[FS] Destination already exists: %s\n", dst_arg);
        return 1;
    }

    if (platform_rename(native_src, native_dst) != 0) {
        printf("[FS] Cannot move %s to %s\n", src_arg, dst_arg);
        return 1;
    }

    format_path(rel_src, display, sizeof(display));
    printf("[FS] Moved: %s", display);
    format_path(rel_dst, display, sizeof(display));
    printf(" -> %s\n", display);

    fix_cwd(rel_src);
    return 1;
}

/* ------------------------------------------------------------------ */
/* Line based editor                                                   */
/* ------------------------------------------------------------------ */

static int read_line(const char *prompt, char *out, size_t out_size)
{
    size_t len;

    printf("%s", prompt);
    fflush(stdout);

    if (fgets(out, (int)out_size, stdin) == NULL)
        return -1;

    len = strlen(out);
    while (len > 0 && (out[len - 1] == '\n' || out[len - 1] == '\r'))
        out[--len] = '\0';

    return 0;
}

static void editor_help(void)
{
    printf(" ----------------------------------------------------------\n");
    printf("  a          add a line at the end\n");
    printf("  i N        insert a line before line N\n");
    printf("  r N        replace line N\n");
    printf("  d N        delete line N\n");
    printf("  p          print the file with line numbers\n");
    printf("  w          save\n");
    printf("  wq         save and quit\n");
    printf("  q          quit without saving\n");
    printf("  h          this help\n");
    printf(" ----------------------------------------------------------\n");
}

static int cmd_edit(const char *arg)
{
    char rel[FS_MAX_PATH];
    char native[FS_MAX_PATH];
    char display[FS_MAX_PATH + 2];
    char (*lines)[FS_MAX_EDIT_LINE] = NULL;
    char buffer[FS_MAX_EDIT_LINE + 8];
    char text[FS_MAX_EDIT_LINE];
    char token[32];
    const char *rest;
    int count = 0;
    int dirty = 0;
    int running = 1;
    int is_dir = 0;
    int exists = 0;
    int i;
    FILE *f;

    if (arg == NULL || arg[0] == '\0') {
        printf(" Usage: edit FILE\n");
        return 1;
    }

    if (resolve_path(arg, rel, sizeof(rel)) != 0 || rel[0] == '\0' ||
        !components_valid(rel)) {
        printf("[FS] Invalid file name: %s\n", arg);
        return 1;
    }

    if (build_native(rel, native, sizeof(native)) != 0) {
        printf("[FS] Path is too long.\n");
        return 1;
    }

    if (platform_path_info(native, &is_dir, NULL) == 1) {
        if (is_dir) {
            printf("[FS] Is a directory: %s\n", arg);
            return 1;
        }
        exists = 1;
    }

    if (!exists && parent_exists(rel) != 0) {
        printf("[FS] Parent directory does not exist: %s\n", arg);
        return 1;
    }

    lines = (char (*)[FS_MAX_EDIT_LINE])malloc((size_t)FS_MAX_EDIT_LINES *
                                               FS_MAX_EDIT_LINE);
    if (lines == NULL) {
        printf("[FS] Out of memory.\n");
        return 1;
    }

    if (exists) {
        f = fopen(native, "r");
        if (f == NULL) {
            printf("[FS] Cannot open file: %s\n", arg);
            free(lines);
            return 1;
        }

        while (fgets(buffer, sizeof(buffer), f) != NULL) {
            size_t len = strlen(buffer);
            int truncated = (len > 0 && buffer[len - 1] != '\n');

            while (len > 0 && (buffer[len - 1] == '\n' || buffer[len - 1] == '\r'))
                len--;
            buffer[len] = '\0';

            if (truncated) {
                printf("[FS] Line %d is longer than %d characters.\n",
                       count + 1, FS_MAX_EDIT_LINE - 1);
                fclose(f);
                free(lines);
                return 1;
            }

            if (count >= FS_MAX_EDIT_LINES) {
                printf("[FS] File has too many lines (max %d).\n",
                       FS_MAX_EDIT_LINES);
                fclose(f);
                free(lines);
                return 1;
            }

            snprintf(lines[count], FS_MAX_EDIT_LINE, "%s", buffer);
            count++;
        }

        fclose(f);
    }

    format_path(rel, display, sizeof(display));
    printf(" ----------------------------------------------------------\n");
    printf(" EDITOR: %s  (%d line%s%s)\n", display, count, count == 1 ? "" : "s",
           exists ? "" : ", new file");
    printf(" Type 'h' for commands\n");
    printf(" ----------------------------------------------------------\n");

    while (running) {
        int number = 0;
        char *end_ptr = NULL;

        printf("edit:%s%s> ", base_name(rel), dirty ? " *" : "");
        fflush(stdout);

        if (read_line("", text, sizeof(text)) != 0)
            break;

        rest = next_token(text, token, sizeof(token));
        rest = skip_spaces(rest);

        if (token[0] == '\0')
            continue;

        if (strcmp(token, "h") == 0 || strcmp(token, "?") == 0 ||
            strcmp(token, "help") == 0) {
            editor_help();
            continue;
        }

        if (strcmp(token, "p") == 0) {
            if (count == 0) {
                printf(" (empty file)\n");
            } else {
                for (i = 0; i < count; i++)
                    printf(" %4d | %s\n", i + 1, lines[i]);
            }
            continue;
        }

        if (strcmp(token, "q") == 0) {
            if (dirty) {
                char answer[16];

                if (read_line(" Unsaved changes. Quit anyway? (y/n) ",
                              answer, sizeof(answer)) != 0)
                    break;

                if (answer[0] != 'y' && answer[0] != 'Y')
                    continue;
            }
            running = 0;
            continue;
        }

        if (strcmp(token, "w") == 0 || strcmp(token, "wq") == 0) {
            f = fopen(native, "w");
            if (f == NULL) {
                printf("[FS] Cannot write file: %s\n", arg);
                continue;
            }

            for (i = 0; i < count; i++)
                fprintf(f, "%s\n", lines[i]);

            if (fclose(f) != 0) {
                printf("[FS] Error while saving: %s\n", arg);
                continue;
            }

            dirty = 0;
            exists = 1;
            printf("[FS] Saved %d line%s to %s\n", count,
                   count == 1 ? "" : "s", display);

            if (strcmp(token, "wq") == 0)
                running = 0;

            continue;
        }

        if (strcmp(token, "a") == 0) {
            if (count >= FS_MAX_EDIT_LINES) {
                printf("[FS] Line limit reached (%d).\n", FS_MAX_EDIT_LINES);
                continue;
            }
            if (read_line(" text> ", text, sizeof(text)) != 0)
                break;
            snprintf(lines[count], FS_MAX_EDIT_LINE, "%s", text);
            count++;
            dirty = 1;
            continue;
        }

        if (strcmp(token, "i") == 0 || strcmp(token, "r") == 0 ||
            strcmp(token, "d") == 0) {
            while (*rest == ' ' || *rest == '\t')
                rest++;

            if (*rest == '\0') {
                printf(" Usage: %s N\n", token);
                continue;
            }

            number = (int)strtol(rest, &end_ptr, 10);
            if (end_ptr == rest) {
                printf("[FS] Expected a line number.\n");
                continue;
            }

            if (strcmp(token, "d") == 0) {
                if (number < 1 || number > count) {
                    printf("[FS] No line %d (file has %d lines).\n", number,
                           count);
                    continue;
                }

                for (i = number - 1; i < count - 1; i++)
                    snprintf(lines[i], FS_MAX_EDIT_LINE, "%s", lines[i + 1]);

                count--;
                dirty = 1;
                printf("[FS] Deleted line %d.\n", number);
                continue;
            }

            if (strcmp(token, "i") == 0) {
                if (count >= FS_MAX_EDIT_LINES) {
                    printf("[FS] Line limit reached (%d).\n",
                           FS_MAX_EDIT_LINES);
                    continue;
                }
                if (number < 1 || number > count + 1) {
                    printf("[FS] Insert position must be 1..%d.\n", count + 1);
                    continue;
                }
            } else {
                if (count == 0 || number < 1 || number > count) {
                    printf("[FS] No line %d (file has %d lines).\n", number,
                           count);
                    continue;
                }
            }

            if (read_line(" text> ", text, sizeof(text)) != 0)
                break;

            if (strcmp(token, "i") == 0) {
                for (i = count; i > number - 1; i--)
                    snprintf(lines[i], FS_MAX_EDIT_LINE, "%s", lines[i - 1]);
                snprintf(lines[number - 1], FS_MAX_EDIT_LINE, "%s", text);
                count++;
            } else {
                snprintf(lines[number - 1], FS_MAX_EDIT_LINE, "%s", text);
            }

            dirty = 1;
            continue;
        }

        printf("[FS] Unknown editor command. Type 'h' for help.\n");
    }

    free(lines);
    return 1;
}

/* ------------------------------------------------------------------ */
/* Dispatcher                                                          */
/* ------------------------------------------------------------------ */

int fs_handle_command(const char *input)
{
    char command[32];
    const char *rest;
    int recursive = 0;

    if (input == NULL)
        return 0;

    rest = next_token(input, command, sizeof(command));
    rest = skip_spaces(rest);

    if (command[0] == '\0')
        return 0;

    if (strcmp(command, "ls") == 0 || strcmp(command, "dir") == 0)
        return cmd_ls(rest);

    if (strcmp(command, "cd") == 0)
        return cmd_cd(rest);

    if (strcmp(command, "pwd") == 0)
        return cmd_pwd();

    if (strcmp(command, "mkdir") == 0)
        return cmd_mkdir(rest);

    if (strcmp(command, "rmdir") == 0)
        return cmd_rmdir(rest);

    if (strcmp(command, "rm") == 0) {
        char flag[16];
        const char *after = skip_spaces(next_token(rest, flag, sizeof(flag)));

        if (strcmp(flag, "-r") == 0 || strcmp(flag, "-rf") == 0 ||
            strcmp(flag, "-fr") == 0) {
            recursive = 1;
            return cmd_rm(after, recursive);
        }

        return cmd_rm(rest, recursive);
    }

    if (strcmp(command, "touch") == 0)
        return cmd_touch(rest);

    if (strcmp(command, "cat") == 0)
        return cmd_cat(rest);

    if (strcmp(command, "write") == 0)
        return cmd_write(rest);

    if (strcmp(command, "edit") == 0)
        return cmd_edit(rest);

    if (strcmp(command, "mv") == 0)
        return cmd_mv(rest);

    return 0;
}

/* ------------------------------------------------------------------ */
/* Startup                                                             */
/* ------------------------------------------------------------------ */

void fs_init(void)
{
    const char *env = getenv("LAVANDER_HOME");

    if (env != NULL && env[0] != '\0')
        snprintf(fs_root, sizeof(fs_root), "%s", env);
    else
        snprintf(fs_root, sizeof(fs_root), "lavander_home");

    fs_cwd_rel[0] = '\0';

    if (platform_path_exists(fs_root)) {
        int is_dir = 0;

        if (platform_path_info(fs_root, &is_dir, NULL) == 1 && !is_dir) {
            printf("[FS] WARNING: '%s' exists and is not a directory.\n",
                   fs_root);
        }
        return;
    }

    if (make_dirs(fs_root) != 0)
        printf("[FS] WARNING: cannot create the default directory '%s'.\n",
               fs_root);
}

const char *fs_cwd(void)
{
    static char display[FS_MAX_PATH + 2];

    format_path(fs_cwd_rel, display, sizeof(display));
    return display;
}
