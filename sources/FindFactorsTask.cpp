#include "FindFactorsTask.h"
#include <stdlib.h>

FindFactorsTask::FindFactorsTask(mpz_t _testSubject,mpz_t _upperBound,mpz_t _lowerBound)
{
    mpz_init_set(upperBound,_upperBound);
    mpz_init_set(lowerBound,_lowerBound);
    mpz_init_set(testSubject,_testSubject);
}

FindFactorsTask::~FindFactorsTask()
{
    mpz_clear(upperBound);
    mpz_clear(lowerBound);
    mpz_clear(testSubject);

    for(register unsigned int i = 0; i < results.size(); ++i)
    {
        mpz_clear(*results[i]);
        free(results[i]);
    }
}

void FindFactorsTask::execute()
{
    // declare, allocate and initialize variables
    mpz_t factor;
    mpz_t zero;

    mpz_init_set(factor,lowerBound);
    mpz_init_set_ui(zero,0);

    // iterate through range and find all factors of testSubject within range
    // and put them into results.
    for(mpz_set(factor,lowerBound);
        mpz_cmp(factor,upperBound) <= 0;
        mpz_add_ui(factor,factor,1))
    {
        mpz_t surplus;
        mpz_init(surplus);

        mpz_mod(surplus,testSubject,factor);
        if(mpz_cmp(surplus,zero) == 0)
        {
            mpz_t* mallocedFactor = (mpz_t*) malloc(sizeof(mpz_t));
            mpz_init_set(*mallocedFactor,factor);
            results.push_back(mallocedFactor);
        }

        mpz_clear(surplus);
    }

    // delete variables
    mpz_clear(factor);
    mpz_clear(zero);
}

std::vector<mpz_t*>* FindFactorsTask::get_results()
{
    return &results;
}
