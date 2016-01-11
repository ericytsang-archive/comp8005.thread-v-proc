#include <vector>
#include <stdio.h>
#include <pthread.h>
#include <algorithm>
#include <sys/time.h>
#include "Lock.h"
#include "Number.h"
#include "Semaphore.h"
#include "FindFactorsTask.h"

#define NUM_WORKERS 4
#define MAX_PENDING_TASKS 100
#define MAX_NUMBERS_PER_TASK 10000

long current_timestamp();
void* worker_routine(void*);
int main(int,char**);

Number prime;

std::vector<Number*> tasks;
std::vector<Number*> results;
bool allTasksProduced = false;

Semaphore taskAccess(false,1);
Semaphore tasksNotFullSem(false,MAX_PENDING_TASKS);
Semaphore resultAccess(false,1);

int main(int argc,char** argv)
{
    // parse command line arguments
    if (argc != 2)
    {
        fprintf(stderr,"usage: %s [integer]\n",argv[0]);
        return 1;
    }
    mpz_set_str(prime.value,argv[1],10);

    // get start time
    long startTime = current_timestamp();

    // create the worker threads
    pthread_t workers[NUM_WORKERS];
    for(register unsigned int i = 0; i < NUM_WORKERS; ++i)
    {
        pthread_create(workers+i,0,worker_routine,0);
    }

    // create tasks and place them into the tasks vector
    {
        Number prevPercentageComplete;
        Number percentageComplete;
        Number tempLoBound;
        Number loBound;

        for(mpz_set_ui(loBound.value,1);
            mpz_cmp(loBound.value,prime.value) <= 0;
            mpz_add_ui(loBound.value,loBound.value,MAX_NUMBERS_PER_TASK))
        {
            // calculate and print percentage complete
            mpz_set(prevPercentageComplete.value,percentageComplete.value);
            mpz_mul_ui(tempLoBound.value,loBound.value,100);
            mpz_div(percentageComplete.value,tempLoBound.value,prime.value);
            if(mpz_cmp(prevPercentageComplete.value,percentageComplete.value) != 0)
            {
                gmp_printf("%Zd%\n",percentageComplete.value);
            }

            // insert the task into the task queue once there is room
            Number* newNum = new Number();
            mpz_set(newNum->value,loBound.value);
            tasksNotFullSem.wait();
            Lock scopelock(&taskAccess.sem);
            tasks.push_back(newNum);
        }
    }
    allTasksProduced = true;

    // join all worker threads
    for(register unsigned int i = 0; i < NUM_WORKERS; ++i)
    {
        void* unused;
        pthread_join(workers[i],&unused);
    }

    // get end time
    long endTime = current_timestamp();

    // print out calculation results
    std::sort(results.begin(),results.end(),[](Number* i,Number* j)
    {
        return mpz_cmp(i->value,j->value) < 0;
    });
    printf("factors: ");
    for(register unsigned int i = 0; i < results.size(); ++i)
    {
        gmp_printf("%s%Zd",i?", ":"",results[i]);
        delete results[i];
    }
    printf("\n");

    // print out execution results
    printf("total runtime: %lums\n",endTime-startTime);

    return 0;
}

void* worker_routine(void*)
{
    bool yield = false;

    while(true)
    {
        if (yield)
        {
            pthread_yield();
            yield = false;
        }
        Number* loBoundPtr;

        // get the next task that needs processing
        {
            Lock scopelock(&taskAccess.sem);

            // if there are tasks available to get, get them
            if(!tasks.empty())
            {
                loBoundPtr = tasks.back();
                tasks.pop_back();
            }

            // otherwise, if there are tasks available in the future, wait for
            // them to be available
            else if(!allTasksProduced)
            {
                yield = true;
                continue;
            }

            // otherwise, there are no more tasks, end the loop
            else
            {
                break;
            }
        }
        tasksNotFullSem.post();

        // calculate the hiBound for a task
        Number hiBound;
        mpz_add_ui(hiBound.value,loBoundPtr->value,MAX_NUMBERS_PER_TASK-1);
        if(mpz_cmp(hiBound.value,prime.value) > 0)
        {
            mpz_set(hiBound.value,prime.value);
        }

        // create the task
        FindFactorsTask newTask(prime.value,hiBound.value,loBoundPtr->value);

        // do the processing
        newTask.execute();

        // post results of the tasks
        {
            Lock scopelock(&resultAccess.sem);

            std::vector<mpz_t*>* taskResults = newTask.get_results();
            for(register unsigned int i = 0; i < taskResults->size(); ++i)
            {
                Number* numPtr = new Number();
                mpz_set(numPtr->value,*taskResults->at(i));
                results.push_back(numPtr);
            }
        }

        delete loBoundPtr;
    }

    pthread_exit(0);
}

long current_timestamp()
{
    struct timeval te;
    gettimeofday(&te, NULL); // get current time
    return te.tv_sec*1000L + te.tv_usec/1000; // caculate milliseconds
}
