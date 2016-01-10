#ifndef LOCK_H
#define LOCK_H

#include <semaphore.h>

class Lock
{
public:
    Lock(sem_t* _mutex):mutex(_mutex)
    {
        sem_wait(mutex);
    }

    ~Lock()
    {
        sem_post(mutex);
    }

private:
    sem_t* mutex;
};

#endif
