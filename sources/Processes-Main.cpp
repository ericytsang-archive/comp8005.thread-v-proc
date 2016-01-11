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
void sigusr1_handler(int sigNum);

Number prime;

std::vector<Number*> results;

int tasks[2];
int feedback[2];

sem_t* tasksLock = 0;
sem_t* tasksNotFullSem = 0;
sem_t* feedbackLock = 0;

FILE* taskOut = {0};
FILE* feedbackIn = {0};

int main(int argc,char** argv)
{
    // parse command line arguments
    mpz_set_str(prime.value,argv[1],10);

    // create all synchronization primitives, data structures needed to store
    // results, tasks, and execution statistics
    if (pipe(tasks) < 0 || pipe(feedback) < 0)
    {
        perror("failed to create pipe");
        return 1;
    }

    tasksLock = (sem_t*) mmap(0,sizeof(sem_t),PROT_READ|PROT_WRITE,MAP_SHARED|MAP_ANONYMOUS,-1,0);
    tasksNotFullSem = (sem_t*) mmap(0,sizeof(sem_t),PROT_READ|PROT_WRITE,MAP_SHARED|MAP_ANONYMOUS,-1,0);
    feedbackLock = (sem_t*) mmap(0,sizeof(sem_t),PROT_READ|PROT_WRITE,MAP_SHARED|MAP_ANONYMOUS,-1,0);

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
            return worker_process();
        }
    }

    // close unused pipe descriptors
    close(tasks[0]);
    close(feedback[1]);

    // setup signal handlers
    signal(SIGUSR1,sigusr1_handler);

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
            {
                if (!mpz_out_raw(taskOut,loBound.value))
                {
                    perror("failed to write to pipe");
                    return 1;
                }
                fflush(taskOut);
            }
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

    // clean up system resources
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

void sigusr1_handler(int sigNum)
{
    // read all results from feedback pipe, and put into results vector
    pollfd pollParams = {0};
    pollParams.fd = feedback[0];
    pollParams.events = POLLIN;

    while(poll(&pollParams,1,0) == 1)
    {
        Number* result = new Number();
        if (!mpz_inp_raw(result->value,feedbackIn))
        {
            perror("failed on read");
        }

        results.push_back(result);
    }
}

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
            // gmp_printf("prime.value: %Zd, hiBound.value: %Zd, loBound.value: %Zd\n",prime.value,hiBound.value,loBound.value);
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

long current_timestamp()
{
    struct timeval te;
    gettimeofday(&te, NULL); // get current time
    return te.tv_sec*1000L + te.tv_usec/1000; // caculate milliseconds
}
