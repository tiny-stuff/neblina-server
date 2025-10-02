#include "os/fs.h"

#include <sys/stat.h>

#include "common.h"

bool fs_file_exists(const char* path)
{
    struct stat st;
    return stat(path, &st) == 0;
}

static int mkdir_maybe_existent(const char* path)
{
    int ret = mkdir(path, 0755);
    if (ret != 0 && errno == EEXIST)
        ret = 0;
    return ret;
}

bool fs_mkdir_recursive(const char* path)
{
    int ret = 0;

    char* path_buf = strdup(path);
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
}
