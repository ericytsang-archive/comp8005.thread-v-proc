#include "Number.h"

Number::Number()
{
    mpz_init(value);
}

Number::~Number()
{
    mpz_clear(value);
}
