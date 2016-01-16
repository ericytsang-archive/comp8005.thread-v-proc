/**
 * the process version of the program.
 *
 * usage: ./Processes-Main [integer] [log file]
 *
 * finds all the factors of the passed integer.
 *
 * anything that is printed to stdout is also printed to the specified file.
 *
 * @sourceFile Processes-Main.cpp
 *
 * @program    Processes-Main.out
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
o o()
#include <poll.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <algorithm>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/types.h>
#include "Lock.h"
#include "Number.h"
#include "Semaphore.h"
#include "FindFactorsTask.h"

#define NUM_WORKERS 4
#define MAX_NUMBERS_PER_TASK 10000

int main(int,char**);
long current_timestamp();
int worker_process();
void read_feedback_pipe(int sigNum);

/**
 * number to find all the factors of.
 */
Number prime;

/**
 * vector of calculation results read from the feedback pipe.
 */
std::vector<Number*> results;

/**
 * pipe. contains tasks from parent, consumed by children.
 */
int tasks[2];

/**
 * pipe. contains calculation results from children, read by parent.
 */
int feedback[2];

/**
 * pointer to a sem_t sized shared memory where a semaphore will be allocated
 *   onto. used by children to ensure mutual access when reading from the task
 *   pipe.
 */
sem_t* tasksLock = (sem_t*) mmap(0,sizeof(sem_t),PROT_READ|PROT_WRITE,MAP_SHARED|MAP_ANONYMOUS,-1,0);

/**
 * pointer to a sem_t sized shared memory where a semaphore will be allocated
 *   onto. used by processes to ensure that there can only be
 *   MAX_NUMBERS_PER_TASK serialized tasks in the task pipe at a time.
 */
sem_t* tasksNotFullSem = (sem_t*) mmap(0,sizeof(sem_t),PROT_READ|PROT_WRITE,MAP_SHARED|MAP_ANONYMOUS,-1,0);

/**
 * pointer to a sem_t sized shared memory where a semaphore will be allocated
 *   onto. used by children to ensure mutual access when writing into the
 *   feedback pipe.
 */
sem_t* feedbackLock = (sem_t*) mmap(0,sizeof(sem_t),PROT_READ|PROT_WRITE,MAP_SHARED|MAP_ANONYMOUS,-1,0);

/**
 * file descriptor for reading from the task pipe.
 */
FILE* taskOut = {0};

/**
 * file descriptor for writing into the feedback pipe.
 */
FILE* feedbackIn = {0};

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
 * sets up IPC, spawns children, generates tasks for children, received results
 *   from children, waits for children to terminate, writes results to a file,
 *   and stdout.
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
    if (argc != 2)
    {
        fprintf(stderr,"usage: %s [integer]\n",argv[0]);
        return 1;
    }
    mpz_set_str(prime.value,argv[1],10);

    // create all synchronization primitives, data structures needed to store
    // results, tasks, and execution statistics
    if (pipe(tasks) < 0 ||
        pipe(feedback) < 0)
    {
        perror("pipe");
        return 1;
    }

    if (tasksLock == MAP_FAILED ||
        tasksNotFullSem == MAP_FAILED ||
        feedbackLock == MAP_FAILED)
    {
        perror("mmap");
        return 1;
    }

    if (sem_init(tasksLock,1,1) < 0 ||
        sem_init(tasksNotFullSem,1,NUM_WORKERS*10) < 0 ||
        sem_init(feedbackLock,1,1) < 0)
    {
        perror("sem_init");
        return 1;
    }

    // get start time
    long startTime = current_timestamp();

    // create the worker processes
    for(register unsigned int i = 0; i < NUM_WORKERS; ++i)
    {
        if (!fork())
        {
            // child process
            return worker_process();
        }
    }

    // parent process

    // close unused pipe descriptors
    close(tasks[0]);
    close(feedback[1]);

    // setup signal handlers
    signal(SIGUSR1,read_feedback_pipe);

    // get stream references to file descriptors
    taskOut = fdopen(tasks[1],"w");
    feedbackIn = fdopen(feedback[0],"r");

    if (taskOut == 0 || feedbackIn == 0)
    {
        perror("failed on fdopen");
        return 1;
    }

    // create tasks and place them into the tasks pipe
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
            if (mpz_cmp(prevPercentageComplete.value,percentageComplete.value) != 0)
            {
                gmp_fprintf(stderr,"%Zd%\n",percentageComplete.value);
            }

            // write the loBound into the task pipe
            sem_wait(tasksNotFullSem);
            if (!mpz_out_raw(taskOut,loBound.value))
            {
                perror("failed to write to pipe");
                return 1;
            }
            fflush(taskOut);
        }

        close(tasks[1]);
    }

    // join all child processes
    for(register unsigned int i = 0; i < NUM_WORKERS; ++i)
    {
        wait(0);
    }

    // get end time
    long endTime = current_timestamp();

    // read in any remaining results
    read_feedback_pipe(SIGUSR1);

    // clean up remaining system resources
    sem_destroy(tasksLock);
    sem_destroy(tasksNotFullSem);
    sem_destroy(feedbackLock);

    munmap(tasksLock,sizeof(sem_t));
    munmap(tasksNotFullSem,sizeof(sem_t));
    munmap(feedbackLock,sizeof(sem_t));

    close(feedback[0]);

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

/**
 * SIGUSR1 handler. reads all the results from the feedback pipe, and places
 *   them into the results vector.
 *
 * @method     read_feedback_pipe
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
 * @signature  void read_feedback_pipe(int)
 *
 * @param      int unused
 */
void read_feedback_pipe(int)
{
    // read all results from feedback pipe, and put into results vector
    pollfd pollParams;
    pollParams.fd = feedback[0];
    pollParams.events = POLLIN;

    while(poll(&pollParams,1,0) == 1)
    {
        Number* result = new Number();
        if (!mpz_inp_raw(result->value,feedbackIn))
        {
            if (errno) perror("failed on read");
            break;
        }

        results.push_back(result);
    }
}

/**
 * function that is executed on the child process.
 *
 * @function   worker_process
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
 * continuously reads tasks from the task pipe, executes them, and writes the
 *   results to the feedback pipe for the parent to receive.
 *
 * once task pipe is emptied, and the child gets an EOF exception when reading
 *   from it, the child will release all of its system resources, and terminate.
 *
 * @signature  int worker_process()
 *
 * @return     status code.
 */
int worker_process()
{
    // close unused pipe descriptors
    close(tasks[1]);
    close(feedback[0]);

    // get stream references to file descriptors
    FILE* taskIn = fdopen(tasks[0],"r");
    FILE* feedbackOut = fdopen(feedback[1],"w");

    if (taskIn == 0 || feedbackOut == 0)
    {
        perror("failed on fdopen");
        return 1;
    }

    // do what worker processes do
    while(true)
    {
        FindFactorsTask* taskPtr;

        // get the next task that needs processing
        {
            // read data from the task pipe needed to create a task
            Number loBound;
            {
                Lock scopelock(tasksLock);

                if (!mpz_inp_raw(loBound.value,taskIn))
                {
                    sem_post(tasksNotFullSem);
                    return 0;
                }
            }
            sem_post(tasksNotFullSem);

            // calculate the hiBound from the loBound for the task
            Number hiBound;
            mpz_add_ui(hiBound.value,loBound.value,MAX_NUMBERS_PER_TASK-1);
            if (mpz_cmp(hiBound.value,prime.value) > 0)
            {
                mpz_set(hiBound.value,prime.value);
            }

            // create the task
            taskPtr = new FindFactorsTask(prime.value,hiBound.value,loBound.value);
        }

        // do the processing
        taskPtr->execute();

        // post results of the tasks
        {
            Lock scopelock(feedbackLock);

            std::vector<mpz_t*>* results = taskPtr->get_results();
            for(register unsigned int i = 0; i < results->size(); ++i)
            {
                if (!mpz_out_raw(feedbackOut,*results->at(i)))
                {
                    perror("failed to write to pipe");
                    return 1;
                }
            }
            fflush(feedbackOut);
            kill(getppid(),SIGUSR1);
        }

        delete taskPtr;
    }

    close(tasks[0]);
    close(feedback[1]);

    return 0;
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
    gettimeofday(&te,0);
    return te.tv_sec*1000L + te.tv_usec/1000;
}
