#ifndef NEBLINA_FS_H
#define NEBLINA_FS_H

#include <stdbool.h>
#include <stddef.h>

bool fs_file_exists(const char* path);
bool fs_mkdir_recursive(const char* path);

#endif //NEBLINA_FS_H
