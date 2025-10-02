#include "calculator.h"

#include <stdlib.h>

struct Calculator {
    int x;
};

Calculator* calculator_create()
{
    Calculator* c = calloc(1, sizeof(Calculator));
    c->x = 0;
    return c;
}

void calculator_destroy(Calculator* c)
{
    free(c);
}

int calculator_sum(Calculator* c, int a, int b)
{
    (void) c;
    return a + b;
}
