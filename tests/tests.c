#include <assert.h>
#include <stdio.h>

#include "calculator.h"

int main()
{
    Calculator* calculator = calculator_create();
    assert(calculator_sum(calculator, 2, 2) == 4);
    calculator_destroy(calculator);
    printf("\x1b[0;32mTests successful!\x1b[0m\n");
}
