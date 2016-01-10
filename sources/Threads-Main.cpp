#include <vector>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include "FindFactorsTask.h"
#include "Number.h"
#include "Lock.h"

#define NUM_WORKERS 4
#define MAX_PENDING_TASKS 900000
#define MAX_NUMBERS_PER_TASK 10000

void* worker_routine(void*);
int main(int,char**);

// create all data structures needed to store results, tasks, and execution statistics
typedef struct
{
    std::vector<FindFactorsTask*>* tasksPtr;
    std::vector<Number*>* resultsPtr;
    bool* allTasksProducedPtr;
    sem_t* taskAccessPtr;
    sem_t* resultAccessPtr;
}
WorkerThreadParams;

int main(int argc,char** argv)
{
    // parse command line arguments
    Number prime;
    mpz_set_str(prime.value,argv[1],10);

    // create all data structures needed to store results, tasks, and execution
    // statistics
    std::vector<FindFactorsTask*> tasks;
    std::vector<Number*> results;
    bool allTasksProduced = false;

    sem_t taskAccess = {0};
    sem_t resultAccess = {0};

    sem_init(&taskAccess, 0, 1);
    sem_init(&resultAccess, 0, 1);
    // todo: create a thing used to store execution statistics

    // prepare worker thread parameters
    WorkerThreadParams params;
    params.tasksPtr = &tasks;
    params.resultsPtr = &results;
    params.allTasksProducedPtr = &allTasksProduced;
    params.taskAccessPtr = &taskAccess;
    params.resultAccessPtr = &resultAccess;

    // create the worker threads
    pthread_t workers[NUM_WORKERS];
    for(register unsigned int i = 0; i < NUM_WORKERS; ++i)
    {
        pthread_create(workers+i,0,worker_routine,&params);
    }

    // create tasks and place them into the tasks vector
    {
        Number loBound;
        for(mpz_set_ui(loBound.value,1);
            mpz_cmp(loBound.value,prime.value) <= 0;
            mpz_add_ui(loBound.value,loBound.value,MAX_NUMBERS_PER_TASK))
        {
            // calculate the hiBound for a task
            Number hiBound;
            mpz_add_ui(hiBound.value,loBound.value,MAX_NUMBERS_PER_TASK-1);
            if(mpz_cmp(hiBound.value,prime.value) > 0)
            {
                mpz_set(hiBound.value,prime.value);
            }

            // create the task
            FindFactorsTask* newTask = new FindFactorsTask(prime.value,hiBound.value,loBound.value);

            // insert the task into the task queue once there is room
            while (true)
            {
                Lock scopelock(&taskAccess);

                if(tasks.size() > MAX_PENDING_TASKS)
                {
                    pthread_yield();
                    continue;
                }

                tasks.push_back(newTask);
                break;
            }
        }

        allTasksProduced = true;
    }

    // join all worker threads
    for(register unsigned int i = 0; i < NUM_WORKERS; ++i)
    {
        void* unused;
        pthread_join(workers[i],&unused);
    }

    return 0;
}

void* worker_routine(void* ptr)
{
    WorkerThreadParams* params = (WorkerThreadParams*) ptr;

    while(true)
    {
        FindFactorsTask* taskPtr;

        // get the next task that needs processing
        {
            Lock scopelock(params->taskAccessPtr);

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
            Lock scopelock(params->resultAccessPtr);

            std::vector<mpz_t*>* results = taskPtr->get_results();
            for(register unsigned int i = 0; i < results->size(); ++i)
            {
                Number* numPtr = new Number();
                mpz_set(numPtr->value,*results->at(i));
                params->resultsPtr->push_back(numPtr);
                gmp_printf("result: %Zd\n",(void*) numPtr->value);   // todo: remove debug output
            }
        }

        delete taskPtr;
    }

    pthread_exit(0);
}
