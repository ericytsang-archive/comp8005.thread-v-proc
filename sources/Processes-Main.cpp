#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <algorithm>
#include <sys/wait.h>
#include <sys/time.h>
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

FILE* taskOut = {0};
FILE* feedbackIn = {0};

sem_t tasksLock = {0};
sem_t feedbackLock = {0};

int main(int argc,char** argv)
{
    // parse command line arguments
    mpz_set_str(prime.value,argv[1],10);

    // create all synchronization primitives, data structures needed to store
    // results, tasks, and execution statistics
    if(pipe(tasks) < 0 || pipe(feedback) < 0)
    {
        perror("failed to create pipe");
        return 1;
    }

    sem_init(&tasksLock,1,1);
    sem_init(&feedbackLock,1,1);

    // get start time
    long startTime = current_timestamp();

    // create the worker processes
    for(register unsigned int i = 0; i < NUM_WORKERS; ++i)
    {
        if(!fork())
        {
            return worker_process();
        }
    }

    // close unused pipe descriptors
    close(tasks[0]);
    close(feedback[1]);

    // get stream references to file descriptors
    if((taskOut = fdopen(tasks[1],"w")) == 0)
    {
        fprintf(stderr,"failed on fdopen for taskOut for fd %d: ",tasks[1]);
        perror("");
        return 1;
    }
    if((feedbackIn = fdopen(feedback[0],"r")) == 0)
    {
        fprintf(stderr,"failed on fdopen for feedbackIn for fd %d: ",feedback[0]);
        perror("");
        return 1;
    }

    // setup signal handlers
    signal(SIGUSR1,sigusr1_handler);

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
            mpz_set(prevPercentageComplete.value,percentageComplete.value);
            mpz_mul_ui(tempLoBound.value,loBound.value,100);
            mpz_div(percentageComplete.value,tempLoBound.value,prime.value);
            if(mpz_cmp(prevPercentageComplete.value,percentageComplete.value) != 0)
            {
                gmp_printf("%Zd%\n",percentageComplete.value);
            }

            // write the loBound into the task pipe
            if(!mpz_out_raw(taskOut,loBound.value))
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

    // clean up system resources
    sem_destroy(&tasksLock);
    sem_destroy(&feedbackLock);

    // read in all the results
    while (true)
    {
        // read a result from feedback pipe, and put it into the results vector
        Number* result = new Number();
        if(!mpz_inp_raw(result->value,feedbackIn))
        {
            errno = 1;
            perror("failed to read from pipe");
            break;
        }

        results.push_back(result);
    }

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
    close(feedback[0]);

    // print out execution results
    printf("total runtime: %lums\n",endTime-startTime);

    return 0;
}

void sigusr1_handler(int sigNum)
{
    // // read a result from feedback pipe, and put it into the results vector
    // Number* result = new Number();
    // if(!mpz_inp_raw(result->value,feedbackIn))
    // {
    //     errno = 1;
    //     perror("failed to read from pipe");
    // }

    // results.push_back(result);
}

int worker_process()
{
    // close unused pipe descriptors
    close(tasks[1]);
    close(feedback[0]);

    // get stream references to file descriptors
    FILE* taskIn = fdopen(tasks[0],"r");
    FILE* feedbackOut = fdopen(feedback[1],"w");
    if(taskIn == 0 || feedbackOut == 0)
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
            Lock scopelock(&tasksLock);

            // read data from the task pipe needed to create a task
            Number loBound;
            if(!mpz_inp_raw(loBound.value,taskIn))
            {
                errno = 1;
                perror("failed to read from pipe");
                return 1;
            }

            // calculate the hiBound from the loBound for the task
            Number hiBound;
            mpz_add_ui(hiBound.value,loBound.value,MAX_NUMBERS_PER_TASK-1);
            if(mpz_cmp(hiBound.value,prime.value) > 0)
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
            Lock scopelock(&feedbackLock);

            std::vector<mpz_t*>* results = taskPtr->get_results();
            for(register unsigned int i = 0; i < results->size(); ++i)
            {
                if(!mpz_out_raw(feedbackOut,*results->at(i)))
                {
                    perror("failed to write to pipe");
                    return 1;
                }
                fflush(feedbackOut);
                kill(getppid(),SIGUSR1);
            }
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
