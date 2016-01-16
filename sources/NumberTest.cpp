/**
 * contains a main function that uses the Number class. meant to be run with
 *   debugging tools to make sure there are no memory leaks and other problems.
 *
 * @sourceFile NumberTest.cpp
 *
 * @program    NumberTest.out
 *
 * @date       2016-01-15
 *
 * @revision   none
 *
 * @designer   Eric Tsang
 *
 * @programmer Eric Tsang
 *
 * @note       none
 */
#include <stdio.h>
#include "Number.h"

/**
 * uses the Number class. this program is meant to be run with debugging tools
 *   like valgrind to verify that there are no memory leaks and other issues.
 *
 * @function   main
 *
 * @date       2016-01-15
 *
 * @revision   none
 *
 * @designer   Eric Tsang
 *
 * @programmer Eric Tsang
 *
 * @note       none
 *
 * @signature  int main()
 *
 * @return     exit status.
 */
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
