#include <stdio.h>
#include "Number.h"

int main()
{
    // declare & initialize numbers
    Number hi;
    Number* hello = new Number();

    // do test stuff...
    mpz_set_ui(hi.value,600);
    mpz_set_ui(hello->value,700);

    // print the results
    gmp_printf("%Zd == 600\n",hi.value);
    gmp_printf("%Zd == 700\n",hello->value);

    // deallocate numbers
    delete hello;

    return 0;
}
