#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include <semaphore.h>

class Semaphore
{
public:

    Semaphore(bool betweenProcesses,int permits)
    {
        sem_init(&sem,betweenProcesses?1:0,permits);
    }

    ~Semaphore()
    {
        sem_destroy(&sem);
    }

    void post()
    {
        sem_post(&sem);
    }

    void wait()
    {
        sem_wait(&sem);
    }

    sem_t sem;
};

#endif
