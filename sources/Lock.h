/**
 * header file for the Lock class. implementation is in Lock.cpp
 *
 * @sourceFile Lock.h
 *
 * @program    Threads-Main.out, Processes-Main.out
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
 */
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
