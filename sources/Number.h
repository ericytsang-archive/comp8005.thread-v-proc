#ifndef NUMBER_H
#define NUMBER_H

#include <gmp.h>

class Number
{
public:
    Number();
    ~Number();
    mpz_t value;
};

#endif
