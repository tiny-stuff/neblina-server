#ifndef NEBLINA_SERVER_ERROR_H
#define NEBLINA_SERVER_ERROR_H

#include <errno.h>
#include <stdlib.h>

#include "util/logs.h"

#define NON_RECOVERABLE_ERROR 57

const char* n_error(int errn);

#define FATAL_NON_RECOVERABLE(...) { ERR(__VA_ARGS__); exit(NON_RECOVERABLE_ERROR); }

#endif //NEBLINA_SERVER_ERROR_H
