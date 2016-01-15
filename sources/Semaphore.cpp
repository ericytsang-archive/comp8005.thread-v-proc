#include "Semaphore.h"

Semaphore::Semaphore(bool betweenProcesses,int permits)
{
    sem_init(&sem,betweenProcesses?1:0,permits);
}

Semaphore::~Semaphore()
{
    sem_destroy(&sem);
}

void Semaphore::post()
{
    sem_post(&sem);
}

void Semaphore::wait()
{
    sem_wait(&sem);
}
