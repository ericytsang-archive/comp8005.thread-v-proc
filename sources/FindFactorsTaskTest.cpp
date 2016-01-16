/**
 * contains a main function that uses the FindFactorsTask class. meant to be run
 *   with debugging tools to make sure there are no memory leaks and other
 *   problems.
 *
 * @sourceFile FindFactorsTaskTest.cpp
 *
 * @program    FindFactorsTaskTest.out
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
#include <gmp.h>
#include <stdio.h>
#include <vector>
#include "FindFactorsTask.h"
#include "Number.h"
#include <stdlib.h>

/**
 * uses the FindFactorsTask class. this program is meant to be run with
 *   debugging tools like valgrind to verify that there are no memory leaks and
 *   other issues.
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
    Number number;
    Number loMark;
    Number hiMark;

    mpz_set_ui(number.value,1000);
    mpz_set_ui(loMark.value,1);
    mpz_set_ui(hiMark.value,999);

    // do test stuff...
    FindFactorsTask task(number.value,hiMark.value,loMark.value);
    task.execute();
    std::vector<mpz_t*>* results = task.get_results();

    // print the results
    for(register unsigned int i = 0; i < results->size(); ++i)
    {
        gmp_printf("%Zd\n",results->at(i));
    }

    return 0;
}
