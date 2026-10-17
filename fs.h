#ifndef LAVANDER_FS_H
#define LAVANDER_FS_H

#include <stddef.h>
#include "platform.h"

/*
 * Filesystem module of the kernel (Linux + Windows).
 *
 * The shell works inside a default directory ("lavander_home" next to the
 * program, overridable with the LAVANDER_HOME environment variable).
 * All paths are kept relative to that root, so the user can create
 * directories several levels deep without ever leaving the sandbox.
 */

#define FS_MAX_PATH      512
#define FS_MAX_ENTRIES   1024
#define FS_MAX_EDIT_LINES 1000
#define FS_MAX_EDIT_LINE  241  /* 240 chars + NUL */

/* Creates the default directory. Called once at startup. */
void fs_init(void);

/* Current working directory displayed to the user ("/" at the root). */
const char *fs_cwd(void);

/*
 * Runs a filesystem command (mkdir, rmdir, rm, ls, dir, cd, pwd, touch,
 * cat, write, edit, mv).
 * Returns 1 when the input was handled by this module, 0 when it is not
 * a filesystem command and must be processed elsewhere.
 */
int fs_handle_command(const char *input);

#endif /* LAVANDER_FS_H */
