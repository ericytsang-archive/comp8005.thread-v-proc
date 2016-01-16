/**
 * the threaded version of the program.
 *
 * usage: ./Threads-Main [integer] [log file]
 *
 * finds all the factors of the passed integer.
 *
 * anything that is printed to stdout is also printed to the specified file.
 *
 * @sourceFile Threads-Main.cpp
 *
 * @program    Threads-Main.out
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
#include <vector>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <algorithm>
#include <sys/stat.h>
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

/**
 * number to find all the factors of.
 */
Number prime;

/**
 * vector used to store all the serialized tasks produced by the main thread,
 *   and consumed by worker threads.
 */
std::vector<Number*> tasks;

/**
 * vector used to store all the results tasks produced by the worker threads,
 *   and consumed by main thread.
 */
std::vector<Number*> results;

/**
 * set to true once all tasks have been produced by the main thread, so worker
 *   threads know that there are no more tasks to consume once the tasks vector
 *   is empty, and this flag is set to true.
 */
bool allTasksProduced = false;

/**
 * mutex used to ensure mutual access to the tasks vector.
 */
Semaphore taskAccess(false,1);

/**
 * mutex used to ensure that there is only a maximum of MAX_PENDING_TASKS tasks
 *   in the tasks vector at any time.
 */
Semaphore tasksNotFullSem(false,MAX_PENDING_TASKS);

/**
 * mutex used to ensure mutual access to the results vector.
 */
Semaphore resultAccess(false,1);

/**
 * entry point of the program.
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
 * @note
 *
 * sets up synchronization primitives, spawns worker threads, generates tasks
 *   for workers, receives results from workers, waits for workers to terminate,
 *   writes results to a file, and stdout.
 *
 * @signature  int main(int argc,char** argv)
 *
 * @param      argc number of command line arguments
 * @param      argv array of c strings of command line arguments
 *
 * @return     status code.
 */
int main(int argc,char** argv)
{
    // parse command line arguments
    if (argc != 3)
    {
        fprintf(stderr,"usage: %s [integer] [path to log file]\n",argv[0]);
        return 1;
    }
    if(mpz_set_str(prime.value,argv[1],10) == -1)
    {
        fprintf(stderr,"usage: %s [integer] [path to log file]\n",argv[0]);
        return 1;
    }
    int logfile = open(argv[2],O_CREAT|O_WRONLY|O_APPEND);
    FILE* logFileOut = fdopen(logfile,"w");
    if(logfile == -1 || errno)
    {
        fprintf(stderr,"usage: %s [integer] [path to log file]\nerror occurred: ",argv[0]);
        perror(0);
        return 1;
    }

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
                gmp_fprintf(stdout,"%Zd%\n",percentageComplete.value);
                gmp_fprintf(logFileOut,"%Zd%\n",percentageComplete.value);
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
    fprintf(stdout,"factors: ");
    fprintf(logFileOut,"factors: ");
    for(register unsigned int i = 0; i < results.size(); ++i)
    {
        gmp_fprintf(stdout,"%s%Zd",i?", ":"",results[i]);
        gmp_fprintf(logFileOut,"%s%Zd",i?", ":"",results[i]);
        delete results[i];
    }
    fprintf(stdout,"\n");
    fprintf(logFileOut,"\n");

    // print out execution results
    fprintf(stdout,"total runtime: %lums\n",endTime-startTime);
    fprintf(logFileOut,"total runtime: %lums\n",endTime-startTime);

    // release system resources
    close(logfile);

    return 0;
}

/**
 * routine executed by worker threads.
 *
 * @function   worker_routine
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
 * continuously reads tasks from the tasks vector, executes them, and writes the
 *   results to the results pipe for the parent to receive.
 *
 * once there are no more tasks to execute, the thread terminates.
 *
 * @signature  void* worker_routine(void*)
 */
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

/**
 * returns the current system time in milliseconds.
 *
 * @function   current_timestamp
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
 * @signature  long current_timestamp()
 *
 * @return     current system time in milliseconds.
 */
long current_timestamp()
{
    struct timeval te;
    gettimeofday(&te, NULL); // get current time
    return te.tv_sec*1000L + te.tv_usec/1000; // caculate milliseconds
}
