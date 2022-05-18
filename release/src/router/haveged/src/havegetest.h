/**
 ** Simple entropy harvester based upon the havege RNG
 **
 ** Copyright 2018-2022 Jirka Hladky hladky DOT jiri AT gmail DOT com
 ** Copyright 2012-2014 Gary Wuertz gary@issiweb.com
 ** Copyright 2012 BenEleventh Consulting manolson@beneleventh.com
 **
 ** This program is free software: you can redistribute it and/or modify
 ** it under the terms of the GNU General Public License as published by
 ** the Free Software Foundation, either version 3 of the License, or
 ** (at your option) any later version.
 **
 ** This program is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 ** GNU General Public License for more details.
 **
 ** You should have received a copy of the GNU General Public License
 ** along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef HAVEGETEST_H
#define HAVEGETEST_H
/**
 * The haveged test suite is built from the 8 tests specified in AIS-31
 * organized into test procedure A and test procedure B and structured
 * as state machines capable of processing segmented input streams.
 */
#include "havegecollect.h"
/**
 * All individual tests and the test procedures use the following
 * simple state machine to manage input.
 */
typedef enum {
  TEST_INIT,               /* initialize test (internal)    */
  TEST_INPUT,              /* test input needed             */
  TEST_EVAL,               /* evaluating results (internal) */
  TEST_DONE,               /* test complete                 */
  TEST_RETRY,              /* retry the test                */
  TEST_IGNORE,             /* ignore failure and continue   */
  TEST_FAIL                /* Test has failed               */
  } TEST_STATE;
/**
 * AIS-31 procedure A uses the FIPS140-1 as test1 thru test4. A disjointedness test is
 * used as test0 and a autocorrelation test is used as test5. test0 is executed only
 * once, the other tests are repeated in sequence 257 times.
 */
#define  AIS_A_REPS        257   /* reps for test1 through test 5       */
/**
 * Constants for the fips tests. Note AIS-31 v1 uses the unamended FIPS test limits
 */
#define  FIPS_USED         20000
#ifndef  USE_AMENDED_FIPS
#define  FIPS_MAX_RUN      34
#define  FIPS_ONES_LOW     9654
#define  FIPS_ONES_HIGH    10346
#define  FIPS_POKER_LOW    1562822  /* 1.03  */
#define  FIPS_POKER_HIGH   1580438  /* 57.4  */
#define  FIPS_RUNS_LOW     2267,1079,502,223,90,90
#define  FIPS_RUNS_HIGH    2733,1421,748,402,223,223
#else
#define  FIPS_MAX_RUN      25
#define  FIPS_ONES_LOW     9725
#define  FIPS_ONES_HIGH    10275
#define  FIPS_POKER_LOW    1576928  /* 2.16  */
#define  FIPS_POKER_HIGH   1576928  /* 46.17 */
#define  FIPS_RUNS_LOW     2315,1114,525,240,103,103
#define  FIPS_RUNS_HIGH    2685,1386,723,384,209,209
#endif
/**
 * test 0 consumes 64k * 48 bits
 */
#define  TEST0_LENGTH      65536
#define  TEST0_USED        (TEST0_LENGTH * 48)
#define  TEST5_LENGTH      5000
/**
 * Fixed size input for procedure A
 */
#define  AIS_A_SIZE  (TEST0_USED+(2500*257))
/**
 * AIS-31 procedure A results
 */
typedef struct {
   H_UINT   testResult;          /* id 8 bits, pass/fail 8bits */
   H_UINT   finalValue;          /* end result                 */
} resultA;
/**
 * AIS-31 procedure A context. Options are defined in haveged.h
 * This puppy weighs in at ~3 MB.
 */
typedef struct {
   H_UINT8  *data;               /* input for test             */         
   H_UINT   range;               /* number of bits of input    */
   H_UINT   procState;           /* procedure state            */
   H_UINT   procRetry;           /* retry indication           */
   H_UINT   testId;              /* test selector 0-5          */
   H_UINT   testRun;             /* test index 1 - 1285        */
   H_UINT   testState;           /* FSM state of current test  */
   H_UINT   bridge;              /* index for data bridge      */
   H_UINT   bytesUsed;           /* number of bytes used       */
   H_UINT   options;             /* duty cycle for test5       */
   H_UINT8  aux[TEST0_USED];     /* extra work space           */
   resultA  results[1286];       /* test results               */
   } procA;
/**
 * AIS-31 procedure B is a set of multinomial distribution tests
 * and an entropy estimate (Coron' test). The distribution tests,
 * test6 and test7, require at least AIS_LENGTH sequences of 1, 2
 * 4, and 8 bits.
 */
/*
 * Bit range of AIS-31 procedure B distribution tests
 */
#define  AIS_LENGTH     100000
/**
 * AIS-31 test8 constants (Coron's test)
 */
#define  Q  2560
#define  K  256000
#define LN2 0.69314718055994530941
/**
 * AIS-31 procedure B results
 */
typedef struct {
   H_UINT testResult;            /* id 8 bits, pass/fail 8bits */
   double finalValue;            /* final value                */
   } resultB;
/**
 * AIS-31 procedure B context, a svelt 1.25 KB
 */
typedef struct {
   H_UINT   *noise;              /* input for test             */         
   H_UINT   range;               /* number of bits of input    */
   H_UINT   procState;           /* procedure state            */
   H_UINT   procRetry;           /* retry indication           */
   H_UINT   testId;              /* test selector 6-8          */ 
   H_UINT   testNbr;             /* current test number        */
   H_UINT   testState;           /* FSM state of current test  */
   H_UINT   seq;                 /* aisSeq() sequence needed   */
   H_UINT   bitsUsed;            /* bits used by procedure     */
   H_UINT   bridge;              /* data bridge test6,7        */
   H_UINT   counter[8];          /* sequence lengths           */
   H_UINT   einsen[8];           /* sequence counts (ones)     */
   H_UINT   full;                /* sequence flags             */
   H_UINT   lastpos[256];        /* counters for test 8        */
   H_UINT   options;             /* RFU                        */
   resultB  results[9];          /* test results               */
   } procB;
/**
 * Testing options
 */
#define  A_CYCLE    0x000001ff   /* test5 duty cycle           */
#define  A_WARN     0x00000200   /* Only warn of A fails       */
#define  A_RUN      0x00000400   /* Run procedure A            */
#define  A_OPTIONS  0x000003ff
#define  B_WARN     0x00001000   /* Only warn of B fails       */
#define  B_RUN      0x00002000   /* Run proceure B             */
#define  B_OPTIONS  0x00001000
#define  X_OPTIONS  0x000f0000   /* isolated test index        */
#define  X_RUN      0x00100000   /* diagnostic isolated test   */
/**
 * A test procedure run consists of an indicator and options
 */
typedef struct {
   H_UINT   action;              /* action code A_RUN, B_RUN  */
   H_UINT   options;             /* WARN and other options    */
  } procInst;

/**
 * Services provided
 */
typedef int   (*ptrDiscard)(H_COLLECT *rdr);
typedef void  (*ptrReport)(H_COLLECT * h_ctxt, H_UINT action, H_UINT prod, H_UINT state, H_UINT ct);
typedef int   (*ptrRun)(H_COLLECT *rdr, H_UINT prod);

/**
 * A test procedure is associated with a collection buffer. Some
 * resources are shared by all collection buffers. This includes
 * roll-your-own vtable used to avoid polluting the RNG name space
 * This structure ends up in hptr->testData
 */
typedef struct {
   ptrDiscard     discard;                   /* release test resources     */
   ptrRun         run;                       /* run test suite             */
   ptrReport      report;                    /* report test results        */
   H_UINT         options;                   /* verbosity, etc.            */
   H_UINT         testsUsed;                 /* tests used                 */
   procInst       totTests[2];               /* tot tests to run           */
   procInst       runTests[2];               /* production tests to run    */
   H_UINT         procReps;                  /* Number of  A repetitions   */
   H_UINT         fips_low[6];               /* low runs thresholds        */
   H_UINT         fips_high[6];              /* high runs thresholds       */
   char           totText[8];                /* tot test text rep          */
   char           prodText[8];               /* production test text rep   */
   H_UINT         meters[H_OLT_PROD_B_P+1];  /* test counters              */
   double         lastCoron;                 /* last test8 result          */
   double         *G;                        /* test8 lookup table         */
} procShared;
/**
 * How to get test context and shared data from H_COLLECT
 */
#define TESTS_CONTEXT(c) (onlineTests *)(c->havege_tests)
#define TESTS_SHARED(c)  (procShared *)(((H_PTR)(c->havege_app))->testData)
/**
 * Online testing context - one per collector. Note szTotal is for diagnostic
 * use only, no effort is made to account for overflow.
 */
typedef struct {
   H_UINT      result;           /* nz if failed               */
   H_UINT      totIdx;           /* tot test idx               */
   H_UINT      runIdx;           /* run test idx               */
   H_UINT      szCarry;          /* bits carried in next proc  */
   H_UINT      szTotal;          /* total bits processed       */
   procA       *pA;              /* procedure A instance       */
   procB       *pB;              /* procedure B instance       */
} onlineTests;
/**
 * Default options are to run the tot tests.
 */
#define DEFAULT_TEST_OPTIONS "ta8b"
/**
 * Public interface
 */
int havege_test(procShared *tps, H_PARAMS *params);

#endif
