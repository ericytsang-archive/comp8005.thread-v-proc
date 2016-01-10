#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include <semaphore.h>

class Semaphore
{
public:

    Semaphore(bool betweenProcesses,int permits)
    {
        sem_init(&semaphore,betweenProcesses?1:0,permits);
    }

    ~Semaphore()
    {
        sem_destroy(&semaphore);
    }

    void post()
    {
        sem_post(&semaphore);
    }

    void wait()
    {
        sem_wait(&semaphore);
    }

private:

    sem_t semaphore;
};

#endif
