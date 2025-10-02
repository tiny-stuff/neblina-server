#include "os/fs.h"

#define _POSIX_C_SOURCE 1
#include <windows.h>

#include "common.h"

bool fs_file_exists(const char* path)
{
    DWORD attrs = GetFileAttributesA(path);
    return (attrs != INVALID_FILE_ATTRIBUTES);
}

static int mkdir_maybe_existent(const char* path)
{
    if (!CreateDirectoryA(path, NULL)) {
        DWORD err = GetLastError();
        if (err == ERROR_ALREADY_EXISTS)
            return 0;
        return err;
    }
    return 0;
}

bool fs_mkdir_recursive(const char* path)
{
    int ret = 0;

    char* path_buf = strdup(path);
    for (char* c = path_buf; *c; ++c)
        if (*c == '\\')
            *c = '/';

    char* slash = path_buf;

    for (;;) {
        slash = strchr(slash + 1, '/');
        if (!slash) {
            ret = mkdir_maybe_existent(path_buf);
            break;
        }
        *slash = '\0';
        ret = mkdir_maybe_existent(path_buf);
        if (ret != 0)
            break;
        *slash = '/';
    }

    free(path_buf);

    return ret == 0;
    // TODO
}
