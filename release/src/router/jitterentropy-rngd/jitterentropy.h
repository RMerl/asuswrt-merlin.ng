/*
 * Non-physical true random number generator based on timing jitter.
 *
 * Copyright Stephan Mueller <smueller@chronox.de>, 2014 - 2021
 *
 * License
 * =======
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, and the entire permission notice in its entirety,
 *    including the disclaimer of warranties.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 *
 * ALTERNATIVELY, this product may be distributed under the terms of
 * the GNU General Public License, in which case the provisions of the GPL are
 * required INSTEAD OF the above restrictions.  (This clause is
 * necessary due to a potential bad interaction between the GPL and
 * the restrictions contained in a BSD-style copyright.)
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, ALL OF
 * WHICH ARE HEREBY DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF NOT ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 */

#ifndef _JITTERENTROPY_H
#define _JITTERENTROPY_H

/***************************************************************************
 * Jitter RNG Configuration Section
 *
 * You may alter the following options
 ***************************************************************************/

/*
 * Enable timer-less timer support
 *
 * In case the hardware is identified to not provide a high-resolution time
 * stamp, this option enables a built-in high-resolution time stamp mechanism.
 *
 * The timer-less noise source is based on threads. This noise source requires
 * the linking with the POSIX threads library. I.e. the executing environment
 * must offer POSIX threads. If this option is disabled, no linking
 * with the POSIX threads library is needed.
 */
#define JENT_CONF_ENABLE_INTERNAL_TIMER

/*
 * Disable the loop shuffle operation
 *
 * The shuffle operation enlarges the timing of the conditioning function
 * by a variable length defined by the LSB of a time stamp. Some mathematicians
 * are concerned that this pseudo-random selection of the loop iteration count
 * may create some form of dependency between the different loop counts
 * and the associated time duration of the conditioning function. It
 * also complicates entropy assessment because it effectively combines a bunch
 * of shifted/scaled copies the same distribution and masks failures from the
 * health testing.
 *
 * By enabling this flag, the loop shuffle operation is disabled and
 * the entropy collection operates in a way that honor the concerns.
 *
 * By enabling this flag, the time of collecting entropy may be enlarged.
 */
#define JENT_CONF_DISABLE_LOOP_SHUFFLE

/***************************************************************************
 * Jitter RNG State Definition Section
 ***************************************************************************/

#include "jitterentropy-base-user.h"

#define SHA3_256_SIZE_DIGEST_BITS	256
#define SHA3_256_SIZE_DIGEST		(SHA3_256_SIZE_DIGEST_BITS >> 3)

/* The entropy pool */
struct rand_data
{
	/* all data values that are vital to maintain the security
	 * of the RNG are marked as SENSITIVE. A user must not
	 * access that information while the RNG executes its loops to
	 * calculate the next random value. */
	uint8_t data[SHA3_256_SIZE_DIGEST]; /* SENSITIVE Actual random number */
	uint64_t prev_time;		/* SENSITIVE Previous time stamp */
#define DATA_SIZE_BITS (SHA3_256_SIZE_DIGEST_BITS)
	uint64_t last_delta;		/* SENSITIVE stuck test */
	uint64_t last_delta2;		/* SENSITIVE stuck test */
	unsigned int osr;		/* Oversampling rate */
#define JENT_MEMORY_BLOCKS 64
#define JENT_MEMORY_BLOCKSIZE 32
#define JENT_MEMORY_ACCESSLOOPS 128
#define JENT_MEMORY_SIZE (JENT_MEMORY_BLOCKS*JENT_MEMORY_BLOCKSIZE)
	unsigned char *mem;		/* Memory access location with size of
					 * memblocks * memblocksize */
	unsigned int memlocation; 	/* Pointer to byte in *mem */
	unsigned int memblocks;		/* Number of memory blocks in *mem */
	unsigned int memblocksize; 	/* Size of one memory block in bytes */
	unsigned int memaccessloops;	/* Number of memory accesses per random
					 * bit generation */

	/* Repetition Count Test */
	int rct_count;			/* Number of stuck values */

	/* Adaptive Proportion Test for a significance level of 2^-30 */
#define JENT_APT_CUTOFF		325	/* Taken from SP800-90B sec 4.4.2 */
#define JENT_APT_WINDOW_SIZE	512	/* Data window size */
	/* LSB of time stamp to process */
#define JENT_APT_LSB		16
#define JENT_APT_WORD_MASK	(JENT_APT_LSB - 1)
	unsigned int apt_observations;	/* Number of collected observations */
	unsigned int apt_count;		/* APT counter */
	uint64_t apt_base;		/* APT base reference */
	unsigned int apt_base_set:1;	/* APT base reference set? */

	unsigned int fips_enabled:1;
	unsigned int health_failure:1;	/* Permanent health failure */
	unsigned int enable_notime:1;	/* Use internal high-res timer */

#ifdef JENT_CONF_ENABLE_INTERNAL_TIMER
	volatile uint8_t notime_interrupt;	/* indicator to interrupt ctr */
	volatile uint64_t notime_timer;		/* high-res timer mock-up */
	uint64_t notime_prev_timer;		/* previous timer value */
	pthread_attr_t notime_pthread_attr;	/* pthreads library */
	pthread_t notime_thread_id;		/* pthreads thread ID */
#endif /* JENT_CONF_ENABLE_INTERNAL_TIMER */
};

/* Flags that can be used to initialize the RNG */
#define JENT_DISABLE_STIR (1<<0) 	/* UNUSED */
#define JENT_DISABLE_UNBIAS (1<<1) 	/* UNUSED */
#define JENT_DISABLE_MEMORY_ACCESS (1<<2) /* Disable memory access for more
					     entropy, saves MEMORY_SIZE RAM for
					     entropy collector */
#define JENT_FORCE_INTERNAL_TIMER (1<<3)  /* Force the use of the internal
					     timer */
#define JENT_DISABLE_INTERNAL_TIMER (1<<4)  /* Disable the potential use of
					       the internal timer. */
#define JENT_FORCE_FIPS (1<<5)		  /* Force FIPS compliant mode
					     including full SP800-90B
					     compliance. */

#ifdef JENT_CONF_DISABLE_LOOP_SHUFFLE
# define JENT_MIN_OSR	3
#else
# define JENT_MIN_OSR	1
#endif

/* -- BEGIN Main interface functions -- */

#ifndef JENT_STUCK_INIT_THRES
/*
 * Per default, not more than 90% of all measurements during initialization
 * are allowed to be stuck.
 *
 * It is allowed to change this value as required for the intended environment.
 */
#define JENT_STUCK_INIT_THRES(x) ((x*9) / 10)
#endif

#ifdef JENT_PRIVATE_COMPILE
# define JENT_PRIVATE_STATIC static
#else /* JENT_PRIVATE_COMPILE */
# define JENT_PRIVATE_STATIC
#endif

/* Number of low bits of the time value that we want to consider */
/* get raw entropy */
JENT_PRIVATE_STATIC
ssize_t jent_read_entropy(struct rand_data *ec, char *data, size_t len);
/* initialize an instance of the entropy collector */
JENT_PRIVATE_STATIC
struct rand_data *jent_entropy_collector_alloc(unsigned int osr,
	       				       unsigned int flags);
/* clearing of entropy collector */
JENT_PRIVATE_STATIC
void jent_entropy_collector_free(struct rand_data *entropy_collector);

/* initialization of entropy collector */
JENT_PRIVATE_STATIC
int jent_entropy_init(void);

/* return version number of core library */
JENT_PRIVATE_STATIC
unsigned int jent_version(void);

/* -- END of Main interface functions -- */

/* -- BEGIN error codes for init function -- */
#define ENOTIME  	1 /* Timer service not available */
#define ECOARSETIME	2 /* Timer too coarse for RNG */
#define ENOMONOTONIC	3 /* Timer is not monotonic increasing */
#define EMINVARIATION	4 /* Timer variations too small for RNG */
#define EVARVAR		5 /* Timer does not produce variations of variations
			     (2nd derivation of time is zero) */
#define EMINVARVAR	6 /* Timer variations of variations is too small */
#define EPROGERR	7 /* Programming error */
#define ESTUCK		8 /* Too many stuck results during init. */
#define EHEALTH		9 /* Health test failed during initialization */
#define ERCT		10 /* RCT failed during initialization */
#define EHASH		11 /* Hash self test failed */

/* -- BEGIN statistical test functions only complied with CONFIG_CRYPTO_CPU_JITTERENTROPY_STAT -- */

#ifdef CONFIG_CRYPTO_CPU_JITTERENTROPY_STAT
JENT_PRIVATE_STATIC
uint64_t jent_lfsr_var_stat(struct rand_data *ec, unsigned int min);
#endif /* CONFIG_CRYPTO_CPU_JITTERENTROPY_STAT */

/* -- END of statistical test function -- */

#endif /* _JITTERENTROPY_H */
