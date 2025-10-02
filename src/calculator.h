#ifndef CALCULATOR_H_
#define CALCULATOR_H_

typedef struct Calculator Calculator;

Calculator* calculator_create();
void        calculator_destroy(Calculator* c);

int         calculator_sum(Calculator* c, int a, int b);

#endif
