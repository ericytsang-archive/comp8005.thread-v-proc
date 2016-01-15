#include "Lock.h"

Lock::Lock(sem_t* _sem):sem(_sem)
{
    sem_wait(sem);
}

Lock::~Lock()
{
    sem_post(sem);
}
