#ifndef RANDOM_HPP_INCLUDED
#define RANDOM_HPP_INCLUDED

#include "e8core/plugin/plugin.hpp"

#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include <stdint.h>
#include <time.h>

/**E8.Include random.hpp
 */

const uint32_t RANDOM_MAX = 4294967295ul; /*!< 2^32 - 1*/

/**E8.Class RandomNumberGenerator RandomNumberGenerator|ГенераторСлучайныхЧисел
 */
class RandomNumberGenerator {

    boost::random::mt19937 gen;

public:
    RandomNumberGenerator() : gen(time(0)) {}
    RandomNumberGenerator(long seed) : gen(seed) {}

    uint32_t
    next(uint32_t lo = 0, uint32_t hi = RANDOM_MAX)
    {
        boost::random::uniform_int_distribution<> dist(lo, hi);
        return dist(gen);
    }

    /**E8.Constructor [IN:Зерно]
     */
    static RandomNumberGenerator* Constructor(E8_IN Seed);

    /**E8.Method RandomNumber|СлучайноеЧисло int [IN:LO] [IN:HI]
     */
    long RandomNumber(E8_IN LO, E8_IN HI);

};
/*E8.EndClass*/

#endif // RANDOM_HPP_INCLUDED
