/**
 * implementation of the FindFactorsTask class declared in FindFactorsTask.h
 *
 * @sourceFile FindFactorsTask.cpp
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
 * @note
 *
 * when this object is constructed, its results vector is empty.
 *
 * the execute method should only be called once per instance. after the execute
 *   method returns, the results of this object will be populated with factors
 *   of the passed number in the range it was to check.
 *
 * when this object is destroyed, all the objects in its results vector are also
 *   destroyed.
 */
#include "FindFactorsTask.h"
#include <stdlib.h>

/**
 * constructor for the FindFactorsTask class.
 *
 * @class      FindFactorsTask
 *
 * @method     FindFactorsTask
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
 * @signature  FindFactorsTask::FindFactorsTask(mpz_t _testSubject,mpz_t _upperBound,mpz_t _lowerBound)
 *
 * @param      _testSubject number to find factors for.
 * @param      _upperBound lower bound of the range to check for factors.
 * @param      _lowerBound upper bound of the range to check for factors.
 *
 * @return     an instance of FindFactorsTask.
 */
FindFactorsTask::FindFactorsTask(mpz_t _testSubject,mpz_t _upperBound,mpz_t _lowerBound)
{
    mpz_init_set(upperBound,_upperBound);
    mpz_init_set(lowerBound,_lowerBound);
    mpz_init_set(testSubject,_testSubject);
}

/**
 * destructor for the FindFactorsTask class.
 *
 * @class      FindFactorsTask
 *
 * @method     ~FindFactorsTask
 *
 * @date       2016-01-15
 *
 * @revision   none
 *
 * @designer   Eric Tsang
 *
 * @programmer Eric Tsang
 *
 * @note
 *
 * when this object is destroyed, all the objects in its results vector are also
 *   destroyed.
 *
 * @signature  FindFactorsTask::~FindFactorsTask()
 */
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

/**
 * computes all the factors for the number in this range, and places them into
 *   its internal results vector which may be accessed through the get_results
 *   method.
 *
 * @class      FindFactorsTask
 *
 * @method     execute
 *
 * @date       2016-01-15
 *
 * @revision   none
 *
 * @designer   Eric Tsang
 *
 * @programmer Eric Tsang
 *
 * @note
 *
 * the execute method should only be called once per instance of this class.
 *   after the execute method returns, the results of this object will be
 *   populated with factors of the passed number in the range it was to check.
 *
 * @signature  void FindFactorsTask::execute()
 */
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

/**
 * returns the results vector of this task object.
 *
 * @class      FindFactorsTask
 *
 * @method     get_results
 *
 * @date       2016-01-15
 *
 * @revision   none
 *
 * @designer   Eric Tsang
 *
 * @programmer Eric Tsang
 *
 * @note
 *
 * when this object is first created, the results vector will be empty.
 *
 * when this object is destroyed, all the objects in its results vector are also
 *   destroyed; they should not be dereferenced after the task object is
 *   destroyed.
 *
 * @signature  std::vector<mpz_t*>* FindFactorsTask::get_results()
 *
 * @return     the results vector of this task object. it contains the all
 *   factors of the specified number that is within the specified range after
 *   execute has been called.
 */
std::vector<mpz_t*>* FindFactorsTask::get_results()
{
    return &results;
}
