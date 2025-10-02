#include "error.h"

#include <string.h>

const char* n_error(int errn)
{
    switch(errn) {
        default:
            return strerror(errn);
    }
}
