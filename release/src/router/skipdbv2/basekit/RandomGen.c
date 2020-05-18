
/*
 A C-program for MT19937, with improved initialization 2002/1/26.

 This is an optimized version that amortizes the shift/reload cost,
 by Eric Landry 2004-03-15.

 Before using, initialize the state by using RandomGen_setSeed(seed) or
 init_by_array(init_key, key_length).

 Copyright (C) 1997--2004, Makoto Matsumoto, Takuji Nishimura, and
 Eric Landry; All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions
 are met:

 1. Redistributions of source code must retain the above copyright
 notice, this list of conditions and the following disclaimer.

 2. Redistributions in binary form must reproduce the above copyright
 notice, this list of conditions and the following disclaimer
 in the documentation and/or other materials provided with the
 distribution.

 3. The names of its contributors may not be used to endorse or
 promote products derived from this software without specific
 prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT
 OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 Any feedback is very welcome.
 http://www.math.sci.hiroshima-u.ac.jp/~m-mat/MT/emt.html
 email: m-mat @ math.sci.hiroshima-u.ac.jp (remove space)

 Reference: M. Matsumoto and T. Nishimura, "Mersenne Twister:
 A 623-Dimensionally Equidistributed Uniform Pseudo-RandomGen Number
 Generator", ACM Transactions on Modeling and Computer Simulation,
 Vol. 8, No. 1, January 1998, pp 3--30.
 */

#include "Base.h"
#include "RandomGen.h"

/* Period parameters */
#define N RANDOMGEN_N
#define M 397
#define MATRIX_A 0x9908b0dfUL   /* constant vector a */
#define UPPER_MASK 0x80000000UL /* most significant w-r bits */
#define LOWER_MASK 0x7fffffffUL /* least significant r bits */

/*static unsigned long self->mt[N];*/ /* the array for the state vector  */
/*static int self->mti=N+1;*/ /* self->mti==N+1 means self->mt[N] is not initialized */

/* initializes self->mt[N] with a seed */

static void init_genrand(RandomGen *self, unsigned long s)
{
	self->mt[0]= s & 0xffffffffUL;
	for (self->mti=1; self->mti<N; self->mti ++)
	{
		self->mt[self->mti] =
		(1812433253UL * (self->mt[self->mti-1] ^ (self->mt[self->mti-1] >> 30)) + self->mti);
		/* See Knuth TAOCP Vol2. 3rd Ed. P.106 for multiplier. */
		/* In the previous versions, MSBs of the seed affect   */
		/* only MSBs of the array self->mt[].                        */
		/* 2002/01/09 modified by Makoto Matsumoto             */
		self->mt[self->mti] &= 0xffffffffUL;
		/* for >32 bit machines */
	}
}

void RandomGen_setSeed(RandomGen *self, unsigned long seed)
{
	init_genrand(self, seed);
}

#include <time.h>

void RandomGen_chooseRandomSeed(RandomGen *self)
{
	unsigned long seed = 0;

	seed ^= (unsigned long)clock(); // processor time since program start
	seed ^= (unsigned long)time(NULL); // seconds since 1970

	RandomGen_setSeed(self, seed);
}

/* initialize by an array with array-length */
/* init_key is the array for initializing keys */
/* key_length is its length */
/* slight change for C++, 2004/2/26 */
#ifdef EXTRAS
static void init_by_array(RandomGen *self, unsigned long init_key[],int key_length)
{
	int i, j, k;

	init_genrand(self, 19650218UL);
	i=1; j=0;
	k = (N>key_length ? N : key_length);

	for (; k; k--) {
		self->mt[i] = (self->mt[i] ^ ((self->mt[i-1] ^ (self->mt[i-1] >> 30)) * 1664525UL))
		+ init_key[j] + j; /* non linear */
		self->mt[i] &= 0xffffffffUL; /* for WORDSIZE > 32 machines */
		i ++; j++;
		if (i>=N) { self->mt[0] = self->mt[N-1]; i=1; }
		if (j>=key_length) j=0;
	}

	for (k=N-1; k; k--) {
		self->mt[i] = (self->mt[i] ^ ((self->mt[i-1] ^ (self->mt[i-1] >> 30)) * 1566083941UL))
		- i; /* non linear */
		self->mt[i] &= 0xffffffffUL; /* for WORDSIZE > 32 machines */
		i ++;
		if (i>=N) { self->mt[0] = self->mt[N-1]; i=1; }
	}

	self->mt[0] = 0x80000000UL; /* MSB is 1; assuring non-zero initial array */
}
#endif

/* generates a random number on [0,0xffffffff]-interval */
static unsigned long genrand_int32(RandomGen *self)
{
	unsigned long y;
	static unsigned long mag01[2]={0x0UL, MATRIX_A};
	/* mag01[x] = x * MATRIX_A  for x=0,1 */

	if (self->mti >= N) { /* generate N words at one time */
		int kk;

		if (self->mti == N+1)   /* if init_genrand() has not been called, */
			init_genrand(self, 5489UL); /* a default initial seed is used */

		for (kk=0;kk<N-M;kk++) {
			y = (self->mt[kk]&UPPER_MASK)|(self->mt[kk+1]&LOWER_MASK);
			self->mt[kk] = self->mt[kk+M] ^ (y >> 1) ^ mag01[y & 0x1UL];
		}
		for (;kk<N-1;kk++) {
			y = (self->mt[kk]&UPPER_MASK)|(self->mt[kk+1]&LOWER_MASK);
			self->mt[kk] = self->mt[kk+(M-N)] ^ (y >> 1) ^ mag01[y & 0x1UL];
		}
		y = (self->mt[N-1]&UPPER_MASK)|(self->mt[0]&LOWER_MASK);
		self->mt[N-1] = self->mt[M-1] ^ (y >> 1) ^ mag01[y & 0x1UL];

		self->mti = 0;
	}

	y = self->mt[self->mti ++];

	// Tempering
	y ^= (y >> 11);
	y ^= (y << 7) & 0x9d2c5680UL;
	y ^= (y << 15) & 0xefc60000UL;
	y ^= (y >> 18);

	return y;
}

/* generates a random number on [0,0x7fffffff]-interval */
#ifdef EXTRAS
static long genrand_int31(RandomGen *self)
{
	return (long)(genrand_int32(self)>>1);
}

/* generates a random number on [0,1]-real-interval */
static double genrand_real1(RandomGen *self)
{
	return genrand_int32(self)*(1.0/4294967295.0);
	/* divided by 2^32-1 */
}
#endif

/* generates a random number on [0,1)-real-interval */
static double genrand_real2(RandomGen *self)
{
	return genrand_int32(self)*(1.0/4294967296.0);
	/* divided by 2^32 */
}

/* generates a random number on (0,1)-real-interval */
#ifdef EXTRAS
static double genrand_real3(RandomGen *self)
{
	return (((double)genrand_int32(self)) + 0.5)*(1.0/4294967296.0);
	/* divided by 2^32 */
}

/* generates a random number on [0,1) with 53-bit resolution*/
static double genrand_res53(RandomGen *self)
{
	unsigned long a=genrand_int32(self)>>5, b=genrand_int32(self)>>6;
	return(a*67108864.0+b)*(1.0/9007199254740992.0);
}
#endif

/* These real versions are due to Isaku Wada, 2002/01/09 added */

/*
int main(void)
{
	int i;
	unsigned long init[4]={0x123, 0x234, 0x345, 0x456}, length=4;
	init_by_array(init, length);
	printf("1000 outputs of genrand_int32(self)\n");
	for (i=0; i<1000; i ++) {
		printf("%10lu ", genrand_int32(self));
		if (i%5==4) printf("\n");
	}
	printf("\n1000 outputs of genrand_real2()\n");
	for (i=0; i<1000; i ++) {
		printf("%10.8f ", genrand_real2());
		if (i%5==4) printf("\n");
	}
	return 0;
}
*/

/* --------------------------------------------------------
   This stuff added by Steve Dekorte, 2004 07 04
*/

#include <time.h>
#include <stdlib.h>

RandomGen *RandomGen_new(void)
{
	RandomGen *self = (RandomGen *)io_calloc(1, sizeof(RandomGen));
	unsigned long t1 = (unsigned long)time(NULL);
	unsigned long t2 = clock();
	self->mti = RANDOMGEN_N + 1;
	init_genrand(self, t1 + t2);
	self->y2 = 0;
	return self;
}

void RandomGen_free(RandomGen *self)
{
	io_free(self);
}

double RandomGen_randomDouble(RandomGen *self)
{
	return genrand_real2(self);
}

int RandomGen_randomInt(RandomGen *self)
{
	return genrand_int32(self);
}

#include <math.h>
#ifndef M_PI_2 // some windows'es don't define this
# define M_PI_2 1.57079632679489661923
#endif

double RandomGen_gaussian(RandomGen *self, double m, double s)
{
		// http://www.taygeta.com/random/gaussian.html
	double x1, x2, w, y1, y2;

	do {
		x1 = 2.0 * RandomGen_randomDouble(self) - 1.0;
		x2 = 2.0 * RandomGen_randomDouble(self) - 1.0;
		w = x1 * x1 + x2 * x2;
	} while ( w >= 1.0 );

	w = sqrt( (-2.0 * log( w ) ) / w );
	y1 = x1 * w;
	y2 = x2 * w;

		// The following code resulted in a lot of nans being returned. The
		// following code *should* also be slower.
		/*
	double x1 = 2.0 * RandomGen_randomDouble(self) - 1.0;
	double x2 = 2.0 * RandomGen_randomDouble(self) - 1.0;
	double y1 = sqrt( - 2.0 * log(x1) ) * cos( M_PI_2 * x2 );
		*/

	return ( m + y1 * s );
}

/*
double RandomGen_gaussian(RandomGen *self, double m, double s)
{
	// mean m, standard deviation s
	double x1, x2, w, y1;

	if (self->use_last) // use value from previous call
	{
		y1 = self->y2;
		self->use_last = 0;
	}
	else
	{
		do {
			x1 = 2.0 * RandomGen_randomDouble(self) - 1.0;
			x2 = 2.0 * RandomGen_randomDouble(self) - 1.0;
			w = x1 * x1 + x2 * x2;
		} while ( w >= 1.0 );

		w = sqrt( (-2.0 * log( w ) ) / w );
		y1 = x1 * w;
		self->y2 = x2 * w;
		self->use_last = 1;
	}

	return ( m + y1 * s );
}
*/

