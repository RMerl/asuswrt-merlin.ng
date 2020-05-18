

#ifndef RANDOMGEN_DEFINED
#define RANDOMGEN_DEFINED 1

#ifdef __cplusplus
extern "C" {
#endif

#include "Common.h"

#define RANDOMGEN_N 624

typedef struct
{
	unsigned long mt[RANDOMGEN_N]; // the array for the state vector
	int mti; // mti==N+1 means mt[N] is not initialized
	double y2; // guassian
	int use_last; // guassian
} RandomGen;

BASEKIT_API RandomGen *RandomGen_new(void);
BASEKIT_API void RandomGen_free(RandomGen *self);

BASEKIT_API void RandomGen_setSeed(RandomGen *self, unsigned long seed);
BASEKIT_API void RandomGen_chooseRandomSeed(RandomGen *self);

// generates a random number on between 0.0 and 1.0
BASEKIT_API double RandomGen_randomDouble(RandomGen *self);

BASEKIT_API int RandomGen_randomInt(RandomGen *self);

BASEKIT_API double RandomGen_gaussian(RandomGen *self, double mean, double standardDeviation);

#ifdef __cplusplus
}
#endif
#endif
