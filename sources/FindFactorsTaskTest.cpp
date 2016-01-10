#include <gmp.h>
#include <stdio.h>
#include <vector>
#include "FindFactorsTask.h"
#include "Number.h"
#include <stdlib.h>

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
