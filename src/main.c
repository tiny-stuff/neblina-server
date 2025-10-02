#include <stdio.h>

#include "calculator.h"

int main()
{
    Calculator* calculator = calculator_create();
    printf("2 + 2 = %d\n", calculator_sum(calculator, 2, 2));
    calculator_destroy(calculator);
}
