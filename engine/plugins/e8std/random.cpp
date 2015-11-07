#include "random.hpp"


RandomNumberGenerator* RandomNumberGenerator::Constructor(E8_IN Seed)
{
    if (Seed == E8::Undefined())
        return new RandomNumberGenerator();
    return new RandomNumberGenerator(Seed.to_long());
}

long RandomNumberGenerator::RandomNumber(E8_IN LO, E8_IN HI)
{
    uint32_t lo = 0, hi = RANDOM_MAX;

    if (LO != E8::Undefined())
        lo = LO.to_long();

    if (HI != E8::Undefined())
        hi = HI.to_long();

    return next(lo, hi);
}
