#ifndef NUMBER_H
#define NUMBER_H

#include <gmp.h>

class Number
{
public:

    Number()
    {
        mpz_init(value);
    }

    ~Number()
    {
        mpz_clear(value);
    }

    mpz_t value;
};

#endif
