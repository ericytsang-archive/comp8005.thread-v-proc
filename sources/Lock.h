#ifndef LOCK_H
#define LOCK_H

#include "Semaphore.h"

class Lock
{
public:
    Lock(Semaphore* _sem):sem(_sem)
    {
        sem->wait();
    }

    ~Lock()
    {
        sem->post();
    }

private:
    Semaphore* sem;
};

#endif
