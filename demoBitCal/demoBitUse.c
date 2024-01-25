#include <stdio.h>

/* integer bit operation */
#define BITS_MASK(bit)       ((bit) < 64 ? (1LLU << (bit)) : 0LLU)
#define BITS_SET(value, bit) ((value) |= (1LLU << (bit)))
#define BITS_CLR(value, bit) ((value) &= ~(1LLU << (bit)))
#define BITS_TST(value, bit) (!!((value) & (1LLU << (bit))))

//比特位的运算