/**
 * header file for the Number class. implementation is in Number.cpp
 *
 * @sourceFile Number.h
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
 * used to wrap an integer of the gmp library so that they will be properly
 *   initialized and deallocated when an instance of this class is constructed
 *   or destroyed.
 */
#ifndef NUMBER_H
#define NUMBER_H

#include <gmp.h>

class Number
{
public:
    Number()
    ~Number()
    mpz_t value;
};

#endif
