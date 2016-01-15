#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include <semaphore.h>

class Semaphore
{
public:

    Semaphore(bool betweenProcesses,int permits);
    ~Semaphore();
    void post();
    void wait();
    sem_t sem;
};

#endif
