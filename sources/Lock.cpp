/**
 * implementation of the Lock class declared in Lock.h
 *
 * @sourceFile Lock.cpp
 *
 * @program    Threads-Main.out, Processes-Main.out
 *
 * @class      Lock
 *
 * @date       2016-01-15
 *
 * @revision   none
 *
 * @designer   Eric Tsang
 *
 * @programmer Eric Tsang
 *
 * @note
 *
 * used to wait on, and post to a semaphore using the RAII (Resource Allocation
 *   Is Initialization) idiom.
 *
 * it waits on the semaphore when it is constructed, and when destroyed, posts
 *   back to the semaphore.
 */
#include "Lock.h"

/**
 * waits upon the passed semaphore object.
 *
 * @class      Lock
 *
 * @date       2016-01-15
 *
 * @revision   none
 *
 * @designer   Eric Tsang
 *
 * @programmer Eric Tsang
 *
 * @note       none
 *
 * @signature  Lock::Lock(sem_t* _sem)
 *
 * @param      _sem pointer to the semaphore to wait on when constructed, and
 *   post to once destructed.
 *
 * @return     an instance of the Lock class.
 */
Lock::Lock(sem_t* _sem):sem(_sem)
{
    sem_wait(sem);
}

/**
 * posts to the semaphore object passed in from the constructor.
 *
 * @class      Lock
 *
 * @date       2016-01-15
 *
 * @revision   none
 *
 * @designer   Eric Tsang
 *
 * @programmer Eric Tsang
 *
 * @note       none
 *
 * @signature  ~Lock()
 */
Lock::~Lock()
{
    sem_post(sem);
}
