#ifndef LOCK_H
#define LOCK_H

#include "semaphore.h"

class Lock
{
public:
    Lock(sem_t* _sem):sem(_sem)
    {
        sem_wait(sem);
    }

    ~Lock()
    {
        sem_post(sem);
    }

private:
    sem_t* sem;
};

#endif
