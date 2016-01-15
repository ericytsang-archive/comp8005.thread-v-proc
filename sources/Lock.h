#ifndef LOCK_H
#define LOCK_H

#include "semaphore.h"

class Lock
{
public:
    Lock(sem_t* _sem);
    ~Lock();

private:
    sem_t* sem;
};

#endif
