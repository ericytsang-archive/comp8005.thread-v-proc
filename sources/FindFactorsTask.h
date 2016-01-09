//
// Created by etsang on 08/01/16.
//

#ifndef FINDFACTORSTASK_H
#define FINDFACTORSTASK_H

#include <gmp.h>
#include <vector>


class FindFactorsTask
{
public:

    FindFactorsTask(mpz_t,mpz_t,mpz_t);
    ~FindFactorsTask();
    void execute();
    std::vector<mpz_t*>* get_results();

private:

    mpz_t upperBound;
    mpz_t lowerBound;
    mpz_t testSubject;
    std::vector<mpz_t*> results;
};


#endif //COMP8005_THREAD_V_PROC_FINDFACTORSTASK_H
