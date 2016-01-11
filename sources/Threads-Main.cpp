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
#define MAX_PENDING_TASKS 1000
#define MAX_NUMBERS_PER_TASK 10000

long current_timestamp();
void produce_tasks(Number*,std::vector<FindFactorsTask*>*,Semaphore*);
void* worker_routine(void*);
int main(int,char**);

// create all data structures needed to store results, tasks, and execution statistics
typedef struct
{
    std::vector<FindFactorsTask*>* tasksPtr;
    std::vector<Number*>* resultsPtr;
    bool* allTasksProducedPtr;
    Semaphore* taskAccessPtr;
    Semaphore* resultAccessPtr;
}
WorkerThreadParams;

int main(int argc,char** argv)
{
    // parse command line arguments
    Number prime;
    mpz_set_str(prime.value,argv[1],10);

    // create all synchronization primitives, and data structures needed to
    // store results, tasks, and execution statistics
    std::vector<FindFactorsTask*> tasks;
    std::vector<Number*> results;
    bool allTasksProduced = false;

    Semaphore taskAccess(false,1);
    Semaphore resultAccess(false,1);

    // prepare worker thread parameters
    WorkerThreadParams params;
    params.tasksPtr = &tasks;
    params.resultsPtr = &results;
    params.allTasksProducedPtr = &allTasksProduced;
    params.taskAccessPtr = &taskAccess;
    params.resultAccessPtr = &resultAccess;

    // get start time
    long startTime = current_timestamp();

    // create the worker threads
    pthread_t workers[NUM_WORKERS];
    for(register unsigned int i = 0; i < NUM_WORKERS; ++i)
    {
        pthread_create(workers+i,0,worker_routine,&params);
    }

    // create tasks and place them into the tasks vector
    produce_tasks(&prime,&tasks,&taskAccess);
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

void produce_tasks(Number* prime,std::vector<FindFactorsTask*>* tasks,Semaphore* taskAccess)
{
    Number prevPercentageComplete;
    Number percentageComplete;
    Number tempLoBound;
    Number loBound;

    for(mpz_set_ui(loBound.value,1);
        mpz_cmp(loBound.value,prime->value) <= 0;
        mpz_add_ui(loBound.value,loBound.value,MAX_NUMBERS_PER_TASK))
    {
        // calculate and print percentage complete
        mpz_set(prevPercentageComplete.value,percentageComplete.value);
        mpz_mul_ui(tempLoBound.value,loBound.value,100);
        mpz_div(percentageComplete.value,tempLoBound.value,prime->value);
        if(mpz_cmp(prevPercentageComplete.value,percentageComplete.value) != 0)
        {
            gmp_printf("%Zd%\n",percentageComplete.value);
        }

        // calculate the hiBound for a task
        Number hiBound;
        mpz_add_ui(hiBound.value,loBound.value,MAX_NUMBERS_PER_TASK-1);
        if(mpz_cmp(hiBound.value,prime->value) > 0)
        {
            mpz_set(hiBound.value,prime->value);
        }

        // create the task
        FindFactorsTask* newTask = new FindFactorsTask(prime->value,hiBound.value,loBound.value);

        // insert the task into the task queue once there is room
        while (true)
        {
            Lock scopelock(&taskAccess->sem);

            if(tasks->size() > MAX_PENDING_TASKS)
            {
                pthread_yield();
                continue;
            }

            tasks->push_back(newTask);
            break;
        }
    }
}

void* worker_routine(void* ptr)
{
    WorkerThreadParams* params = (WorkerThreadParams*) ptr;

    while(true)
    {
        FindFactorsTask* taskPtr;

        // get the next task that needs processing
        {
            Lock scopelock(&params->taskAccessPtr->sem);

            // if there are tasks available to do, do them
            if(!params->tasksPtr->empty())
            {
                taskPtr = params->tasksPtr->back();
                params->tasksPtr->pop_back();
            }

            // otherwise, if there are tasks available in the future, wait for
            // them to be available
            else if(!*params->allTasksProducedPtr)
            {
                pthread_yield();
                continue;
            }

            // otherwise, there are no more tasks, end the loop
            else
            {
                break;
            }
        }

        // do the processing
        taskPtr->execute();

        // post results of the tasks
        {
            Lock scopelock(&params->resultAccessPtr->sem);

            std::vector<mpz_t*>* results = taskPtr->get_results();
            for(register unsigned int i = 0; i < results->size(); ++i)
            {
                Number* numPtr = new Number();
                mpz_set(numPtr->value,*results->at(i));
                params->resultsPtr->push_back(numPtr);
            }
        }

        delete taskPtr;
    }

    pthread_exit(0);
}

long current_timestamp()
{
    struct timeval te;
    gettimeofday(&te, NULL); // get current time
    return te.tv_sec*1000L + te.tv_usec/1000; // caculate milliseconds
}
