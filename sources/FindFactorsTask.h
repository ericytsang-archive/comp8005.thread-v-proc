/**
 * header file for the FindFactorsTask class. implementation is in
 *   FindFactorsTask.cpp
 *
 * @sourceFile FindFactorsTask.h
 *
 * @program    Threads-Main.out, Processes-Main.out
 *
 * @class      FindFactorsTask
 *
 * @date       2016-01-15
 *
 * @revision   none
 *
 * @designer   Eric Tsang
 *
 * @programmer Eric Tsang
 *
 * @note       this class encapsulates a long-running task, or sub-task that
 *   should be executed on a worker thread or process.
 */
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
