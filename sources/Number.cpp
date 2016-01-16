/**
 * implementation of the Number class declared in Number.h
 *
 * @sourceFile Number.cpp
 *
 * @program    Threads-Main.out, Processes-Main.out
 *
 * @class      Number
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
#include "Number.h"

/**
 * instantiates a Number instance.
 *
 * @class      Number
 *
 * @method     Number
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
 * initializes the internal gnp integer upon construction.
 *
 * @signature  Number::Number()
 *
 * @return     an instance of the Number class.
 */
Number::Number()
{
    mpz_init(value);
}

/**
 * destroys a Number instance.
 *
 * @class      Number
 *
 * @method     ~Number
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
 * clears the internal gnp integer upon destruction.
 *
 * @signature  Number::~Number()
 */
Number::~Number()
{
    mpz_clear(value);
}
