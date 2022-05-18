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
/**
 * This compile unit implements online tests for haveged through the public functions tests*.
 * Online tests are run directly against the contents of the collection buffer immediately after
 * a buffer fill. Because collection buffer size does not have any direct relationship with
 * the data requirements of the individual tests, all tests implement a state machine to
 * handle segmented input.
 *
 * Note code directly related to the havege interface has been moved to a conditional
 * in that unit for easier maintainability.
 */
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "havegetest.h"

#ifdef ONLINE_TESTS_ENABLE
/**
 * Final value for aisSeq() when no transition found. Originally, Initially this used
 * INFINITY from <math.h> but definition is undefined some gcc versions - foo!
 */
#define  NO_TRANSITION  999999
/**
 * This structure is used only to pack the test structures into a single memory allocation.
 * This is necessary because some architectures have stringent alignment requirements that
 * cannot be met unless (compiler generated) padding is included. On mips in particular
 * double must be dword aligned or bus errors result.
 */
typedef struct {
   onlineTests   olt;
   procA         pa;
   procB         pb;
} testsMemory;
/**
 * The tests and supporting methods
 */
static H_UINT aisProcedureA(H_COLLECT *h_ctxt, procShared *tps,
                  procA *context, H_UINT *buffer, H_UINT sz, H_UINT offs, H_UINT prod);
static H_UINT aisProcedureB(H_COLLECT *h_ctxt, procShared *tps,
                  procB *context, H_UINT *buffer, H_UINT sz, H_UINT offs, H_UINT prod);
static H_UINT aisSeq(procB *p, H_UINT offs, H_UINT id);
static H_UINT aisTest(H_COLLECT * h_ctxt, H_UINT prod, H_UINT *buffer, H_UINT sz);
static H_UINT copyBits(procA *p, H_UINT ct,H_UINT sz);
static H_UINT fips140(procShared *tps, procA *p, H_UINT offs, H_UINT id);
static H_UINT test0(procA *p, H_UINT offs, H_UINT id);
static int    test0cmp(const void *aa, const void *bb);
static H_UINT test5(procA *p, H_UINT offs, H_UINT id);
static H_UINT test5XOR(H_UINT8 *src, H_UINT shift);
static H_UINT test6a(procB *p, H_UINT offs, H_UINT id);
static H_UINT test8(procShared *tps, procB *p, H_UINT offs, H_UINT id);
static int    testsDiscard(H_COLLECT *rdr);
static void   testsMute(H_COLLECT * h_ctxt, H_UINT action, H_UINT prod, H_UINT state, H_UINT ct);
static int    testsRun(H_COLLECT *rdr, H_UINT prod);

/**
 * The following suite of macros encapsulate the major bit operations of the test suite.
 * The intention is to write simple rather than clever code and let the optimizer strut
 * it's sutff. Note bit index starts with MSB for direct comparison with the test suit'e
 * Java reference implementation.
 */
#define  BITSTREAM_BIT()      ((*bitstream_src)&bitstream_in)==0? 0 : 1
#define  BITSTREAM_NEXT()     {if (bitstream_in==1) {\
                                 bitstream_src+=1;\
                                 bitstream_in=0x80;\
                                 }\
                              else bitstream_in>>=1;}
#define  BITSTREAM_OPEN(a,b)  H_UINT8 *bitstream_src=(H_UINT8 *)(a);\
                              H_UINT   bitstream_in=0x80>>((b)%8);\
                              bitstream_src+=(b)/8
/**
 * Setup shared resources for online tests by sorting the test options into "tot"
 * and production groupings and allocating any resources used by the tests.
 * Caller is responsible for initializing the procShared structure with the
 * report, testsUsed, totTests[], runTests[], totText, and prodText fields.
 */
int   havege_test(         /* RETURN: nz on failure   */
  procShared *tps,         /* IN-OUT: test anchor     */
  H_PARAMS *params)        /* IN: app parameters      */
{
   H_UINT i;

   tps->discard   = testsDiscard;
   if (0==tps->report)
      tps->report = testsMute;
   tps->run       = testsRun;
   tps->options   = params->options;

   if (0!=(tps->testsUsed & A_RUN)) {
      H_UINT low[6]  = {FIPS_RUNS_LOW};   
      H_UINT high[6] = {FIPS_RUNS_HIGH};

      tps->procReps  = 1 + (5 * AIS_A_REPS);
      for (i=0;i<6;i++) {
         tps->fips_low[i]  = low[i];
         tps->fips_high[i] = high[i];
         }
      }
   if (0!=(tps->testsUsed & B_RUN)) {
      tps->G = (double *) malloc((Q+K+1)*sizeof(double));
      if (0 == tps->G)
         return 1;
      tps->G[1] = 0.0;
      for(i=1; i<=(K+Q-1); i++)
         tps->G[i+1]=tps->G[i]+1.0/i;
      for(i=1; i<=(K+Q); i++)
         tps->G[i] /= LN2;
      }
   return 0;
}
/**
 * Check if the current buffer should be released if continuous testing is
 * being performed. The buffer must be discarded if it contains an
 * uncompleted retry or an uncompleted procedure with a failed test
 * or a failed procedure.
 */
static int testsDiscard(   /* RETURN: non-zero to discard   */
   H_COLLECT * h_ctxt)     /* IN-OUT: collector context     */
{
   onlineTests *context = (onlineTests *) h_ctxt->havege_tests;
   procShared  *tps = TESTS_SHARED(h_ctxt);
   procInst    *p;
   H_UINT      i;

   if (0==tps->testsUsed)
      return 0;
   if (context->result!=0)
      return -1;
   p = tps->runTests + context->runIdx;
   switch(p->action) {
      case A_RUN:
         if (0 != context->pA->procRetry)
            return 1;
         for (i = 0;i<context->pA->testRun;i++)
            if (0 !=(context->pA->results[i].testResult & 1))
               return 1;
         break;
      case B_RUN:
         if (0 != context->pB->procRetry)
            return 1;
         for (i=0;i<context->pB->testNbr;i++)
            if (0!=(context->pB->results[i].testResult & 0xff))
               return 1;
         break;
      }
   return 0;
}
/**
 * Place holder for when report is not configured
 */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
static void testsMute(
   H_COLLECT * h_ctxt,     /* IN-OUT: collector context     */
   H_UINT action,          /* IN: A_RUN or B_RUN            */
   H_UINT prod,            /* IN: 0==tot, else continuous   */
   H_UINT state,           /* IN: state variable            */
   H_UINT ct)              /* IN: bytes consumed            */
{
   ;
}
#pragma GCC diagnostic pop
/**
 * The public wrapper that runs the tests. On the first call, the necessary machinery is built.
 * The calls to aisTest() actually run the tests. The test shared structure is read only in this
 * case, since testsRun could be called in a multi-threaded environment where an onlineTests
 * structure is associated with each collection thread.
 */
static int testsRun(       /* RETURN: nz if input needed    */
   H_COLLECT * h_ctxt,     /* IN-OUT: collector context     */
   H_UINT prod)            /* IN: nz if production else tot */
{
   procShared  *tps = TESTS_SHARED(h_ctxt);
   onlineTests *context;
   testsMemory *mem;
   procB       *pb;

   if (0 ==(tps->testsUsed))
      return 0;
   if (0 == h_ctxt->havege_tests) {
      H_UINT sz = sizeof(testsMemory);

      if (0==(tps->testsUsed & A_RUN))
         sz -= sizeof(procA);
      if (0==(tps->testsUsed & B_RUN))
         sz -= sizeof(procB);
      mem = (testsMemory *) malloc(sz);
      if (NULL==mem) {
         h_ctxt->havege_err = H_NOTESTMEM;
         return 1;
         }
      context = (onlineTests *) mem;
      memset(context, 0, sizeof(onlineTests));
      if (0!=(tps->testsUsed & A_RUN)) {
         context->pA = &mem->pa;
         context->pA->procState = TEST_INIT;
         pb = &mem->pb;
         }
      else pb = (procB *)((void *) &mem->pa);
      if (0!=(tps->testsUsed & B_RUN)) {
         context->pB = pb;
         context->pB->procState = TEST_INIT;
         }
      h_ctxt->havege_tests = context;
      if (0 != (h_ctxt->havege_raw & H_DEBUG_TEST_IN))
         return 0;
      }
   return aisTest(h_ctxt, prod, (H_UINT *)h_ctxt->havege_bigarray, h_ctxt->havege_szFill);
}
/**
 * AIS-31 test procedure A. The test is initiated by setting procState to TEST_INIT and
 * the test is performed by calling the procedure adding input until completion is indicated
 * by a proc state of TEST_DONE. The first test requires 3145728 bits (393,216 bytes) and
 * the remaining 5 tests are repeated on sucessive 2500 byte samples for 257 times.
 *
 * Exit states TEST_DONE, TEST_IGNORE, TEST_INPUT, TEST_RETRY
 * 
 * An ideal RNG will pass this test with a probablilty of 0.9987. If there is a single failed
 * test, the test will be repeated. An ideal RNG should almost never fail the retry. The goal
 * of this procedure is to verify RNG output is statisically inconspicuous.
 */
static H_UINT aisProcedureA(  /* RETURN: bits used             */
   H_COLLECT *h_ctxt,         /* IN-OUT: collection instance   */
   procShared  *tps,          /* IN-OUT: shared data           */
   procA *p,                  /* IN: the context               */
   H_UINT *buffer,            /* IN: the input                 */
   H_UINT sz,                 /* IN: the input range           */
   H_UINT ct,                 /* IN: initial bit offset        */
   H_UINT prod)               /* IN: production if nz          */
{
   onlineTests *context = TESTS_CONTEXT(h_ctxt);
   H_UINT i, r;

   switch(p->procState) {
      case TEST_INIT:
         p->bytesUsed = 0;
         p->procRetry = 0;
         /* fallthrough */
      case TEST_RETRY:
         p->procState = TEST_INPUT;
         p->testState = TEST_INIT;
         p->testId = p->testRun   = 0;
         /* fallthrough */
      case TEST_INPUT:
         p->data  = (H_UINT8 *)buffer;
         p->range = sz * sizeof(H_UINT) <<3;
         while(p->testRun < tps->procReps) {
            p->testId  = p->testRun<6? p->testRun : (1+(p->testRun-6) % 5);
            switch(p->testId) {
               case 0:
                  ct = test0(p, ct, p->testRun);
                  break;
               case 1:  case 2:  case 3: case 4:
                  ct = fips140(tps, p, ct, p->testRun);
                  break;
               case 5:
                  ct = test5(p, ct, p->testRun);
                  break;
               }
            context->szCarry = ct;
            if (p->testState == TEST_DONE)
               p->testState = TEST_INPUT;
            else if (p->testState == TEST_INPUT)
               return 0;
            }
            /* fallthrough */
      case TEST_EVAL:
         p->procState = TEST_DONE;
         for (r = i = 0;i<p->testRun;i++)
            r += p->results[i].testResult & 1;
         if (0!=r) {
            tps->meters[prod? H_OLT_PROD_A_F : H_OLT_TOT_A_F] += 1;
            if (1==r && 0==p->procRetry) {
               p->procRetry = 1;
               p->procState = TEST_RETRY;
               }
            else if (0!=(p->options & A_WARN))
               p->procState = TEST_IGNORE;
            else {
               context->result = A_RUN;
               h_ctxt->havege_err = prod? H_NOTESTRUN : H_NOTESTTOT;
               }
            break;
            }
         else tps->meters[prod? H_OLT_PROD_A_P : H_OLT_TOT_A_P] += 1;
         if (0!=(tps->options & (H_DEBUG_OLT|H_DEBUG_OLT))|| TEST_DONE != p->procState)
            tps->report(h_ctxt, A_RUN, prod, p->procState, p->bytesUsed);
         break;
      }
   return p->bytesUsed<<3;
}
/**
 * AIS-31 test procedure B. The test is initiated by setting procState to TEST_INIT and
 * the test is performed by calling the procedure adding input until completion is indicated
 * by a proc state of TEST_DONE. Unlike procedure A, the number of input bits is not fixed
 * but depends on the input.
 *
 * Exit states TEST_DONE, TEST_IGNORE, TEST_INPUT, TEST_RETRY
 *
 * The probability that an ideal RNG will pass this test is 0.9998. If a single test fails,
 * the test is repeated. An ideal RNG should almost never fail the retry. The goal of this
 * procedure is to ensure the entropy of the output is sufficiently large.
 */
static H_UINT aisProcedureB(  /* RETURN: bits used             */
   H_COLLECT  *h_ctxt,        /* IN-OUT: collection instance   */
   procShared *tps,           /* IN-OUT: shared data           */
   procB *p,                  /* IN: the context               */
   H_UINT *buffer,            /* IN: the input                 */
   H_UINT sz,                 /* IN: the input range           */
   H_UINT ct,                 /* IN: initial bit offset        */
   H_UINT prod)               /* IN: production if nz          */
{
   onlineTests *context = TESTS_CONTEXT(h_ctxt);
   H_UINT i, r;
   
   switch(p->procState) {
      case TEST_INIT:
         p->bitsUsed = 0;
         p->procRetry = 0;
         /* fallthrough */
      case TEST_RETRY:
         p->testId = p->testNbr = 0;
         p->procState = TEST_INPUT;
         p->testState = TEST_INIT;
         /* fallthrough */
      case TEST_INPUT:
         p->noise = buffer;
         p->range = sz * BITS_PER_H_UINT;
         i = p->testId;
         while(p->testState != TEST_DONE && i < 5) {
            p->seq   = 1<<i;
            switch(i) {
               case 0:  ct = test6a(p, ct, i);     break;
               case 4:  ct = test8(tps,p,ct,i);    break;
               default: ct = aisSeq(p,ct,i);       break;
               }
            if (p->testState == TEST_INPUT)
               break;
            p->testId = ++i;
            p->testState = TEST_INIT;
            }
         context->szCarry = ct;
         if (p->testState == TEST_INPUT)
            return 0;
         /* fallthrough */
      case TEST_EVAL:
         p->procState = TEST_DONE;
         for (i=r=0;i<p->testNbr;i++)
            r += p->results[i].testResult & 1;
         if (0!=r) {
            tps->meters[prod? H_OLT_PROD_B_F : H_OLT_TOT_B_F] += 1;
            if (1==r && 0==p->procRetry) {
               p->procRetry = 1;
               p->procState = TEST_RETRY;
               }
            else if (0!=(p->options & B_WARN))
               p->procState = TEST_IGNORE;
            else {
               context->result = B_RUN;
               h_ctxt->havege_err = prod? H_NOTESTRUN : H_NOTESTTOT;
               }
            }
         else tps->meters[prod? H_OLT_PROD_B_P : H_OLT_TOT_B_P] += 1;
         if (0!=(tps->options & H_DEBUG_OLT)|| TEST_DONE != p->procState)
            tps->report(h_ctxt, B_RUN, prod, p->procState, p->bitsUsed/8);
         break;
      }
   return p->bitsUsed;
}
/**
 * Driver for disjoint sequence tests - steps 2,3,4 of AIS-31 procedure B (aka test6b, test7a, and test7b).
 * Input tid is the width of the transition to be analyzed: tid=1 { 0x, 1x }, tid=2 {00x, 01x, 10x, 11x},
 * tid=3 {000x, 001x, 010x, 011x, 100x, 101x, 110x, 111x}. The seq menber of procB gives # categories.
 * For a tuple of width n, transition probabilities are calculated for log2(n) transitions for the first
 * 100000 sequences. The deadman counter prevents total runaways with pathalogical input by counting
 * interations that fail to update any counter. If the deadman value exceeds the limit, evaluation of
 * the result forced. The probability of a forced evaluation is 10e-15.
 *
 * The macros below use fields in the procB structure to save/restore context when the input is
 * segmented.
 */
#define  RESTORE(a,b,c) a=p->bridge;b=p->lastpos[0];c=p->lastpos[1]
#define  SAVE(a,b,c)    p->bridge=a;p->lastpos[0]=b;p->lastpos[1]=c

static H_UINT aisSeq(      /* RETURN: last bit index  */
   procB *p,               /* IN-OUT: the context     */
   H_UINT offs,            /* IN: starting bit offset */
   H_UINT tid)             /* IN: test id == #bits    */
{
   static const H_UINT seq_dead[5]  = {0, 50, 120, 258, 0};     /* dead man limit */
   static const H_UINT seq_mask[5]  = {0, 3, 15, 255, 0};       /* full mask      */
   H_UINT   i=0, c, deadman, r, s, j, hilf;
   
   switch(p->testState) {
      case TEST_INIT:
         for(j=0;j<p->seq;j++)
            p->counter[j] = p->einsen[j] = 0;
         p->full = 0;
         p->testState = TEST_INPUT;
         SAVE(0,0,0);
         /* fallthrough */
      case TEST_INPUT:
         RESTORE(j, hilf, deadman);
         offs %= p->range;
         r = p->range - offs;
         s = r % (tid+1);
         r -= s;
         {
            BITSTREAM_OPEN(p->noise,offs);
            while(i<r) {
               for(;j<tid;i++,j++) {
                  hilf += hilf+(BITSTREAM_BIT());BITSTREAM_NEXT();
                  }
               c = BITSTREAM_BIT();BITSTREAM_NEXT();
               i += 1;
               if (0!=(p->full & (1<<hilf))) {
                  if ((deadman+=1)> seq_dead[tid]) {
                     p->testState = TEST_DONE;
                     break;
                     }
                  }
               else {
                  deadman = 0;
                  p->einsen[hilf] += c;
                  if ((p->counter[hilf]+=1)==AIS_LENGTH) {
                     p->full |= (1<<hilf);
                     if (p->full == seq_mask[tid]) {
                        p->testState = TEST_EVAL;
                        break;
                        }
                     }
                  }
               j = hilf = 0;
               }
            if (p->testState==TEST_INPUT) {
               for(j=hilf=0;j<s;j++,i++) {
                  hilf += hilf+(BITSTREAM_BIT());BITSTREAM_NEXT();
                  }
               SAVE(j,hilf,deadman);
               }
         }
         p->bitsUsed += i;
         if (p->testState == TEST_INPUT)
            break;
         /* fallthrough */
      case TEST_EVAL:
         if (tid==1) {
            double q[2];

            if (p->testState == TEST_EVAL) {
               for(j=0;j<2;j++)
                  q[j] = (double)(p->einsen[j]) / (double) AIS_LENGTH;
               p->results[p->testNbr].finalValue = q[0] - q[1];
               }
            else p->results[p->testNbr].finalValue = NO_TRANSITION;
            hilf = tid << 8;
            if (p->results[p->testNbr].finalValue <= -0.02 || p->results[p->testNbr].finalValue >= 0.02)
               hilf |= 1;
            p->results[p->testNbr++].testResult = hilf;
            }
         else {
            /**
             * The spec is very confusing but the reference implementation is correct. The test
             * operates on observed transions, i.e. the difference between pairs of successive
             * einsen and nullen (AIS_LENGTH - einsen)
             */
            for(j=0; j<p->seq; j+=2) {
               if (p->testState == TEST_EVAL) {
                  double qn = (double)((int)p->einsen[j] - (int)p->einsen[j+1]);
                  double qd = (double)(p->einsen[j] + p->einsen[j+1]);
                  double pd = AIS_LENGTH * 2.0 - qd;
                  p->results[p->testNbr].finalValue = ((qn * qn) / pd) + ((qn * qn) / qd);
                  }
               else p->results[p->testNbr].finalValue = NO_TRANSITION;
               hilf = tid << 8;
               if (p->results[p->testNbr].finalValue > 15.13)
                  hilf |= 1;
               p->results[p->testNbr++].testResult = hilf;
               }
            }
         p->testState = TEST_DONE;
         break;
      }
   return i+offs;
}
/**
 * Run the configured test procedures. This function cycles the procedure calls
 * setup by the configuration using tail recursion to sequence multiple tests.
 */
static H_UINT aisTest(     /* RETURN: nz if input needed    */
   H_COLLECT * h_ctxt,     /* IN-OUT: collector context     */
   H_UINT prod,            /* IN: production indicator      */
   H_UINT *buffer,         /* IN: test data, H_UINT aligned */
   H_UINT sz)              /* IN: size of data in H_UINT    */
{
   procShared  *tps = TESTS_SHARED(h_ctxt);
   onlineTests *context = (onlineTests *) h_ctxt->havege_tests;
   procInst   *p;
   H_UINT     offs,state=TEST_DONE, tot=0;

   if (context->result!=0)
      return 0;
   if (prod==0)
      p = tps->totTests + context->totIdx;
   else p = tps->runTests + context->runIdx;

   switch(p->action) {
      case A_RUN:
         if (context->pA->procState==TEST_INIT)
            context->pA->options = p->options;
         tot  = aisProcedureA(h_ctxt, tps, context->pA,
               buffer, sz, context->szCarry, prod);
         state = context->pA->procState;
         break;
      case B_RUN:
         if (context->pB->procState==TEST_INIT)
            context->pB->options = p->options;
         tot = aisProcedureB(h_ctxt, tps, context->pB,
               buffer, sz, context->szCarry, prod);
         state = context->pB->procState;
         break;
      }
   if (state==TEST_INPUT) {
      context->szCarry = 0;
      return 1;
      }
   context->szTotal += tot;
   if (prod==0) {
      if (context->totIdx>=1)                      /* check for end of tot          */
         return 0;
      context->totIdx += 1;
      p = tps->totTests + context->totIdx;
      }
   else {
      if (0==tps->runTests[0].action)              /* check for no cont tests       */
         return 0;
      else if (0!=tps->runTests[1].action)         /* check for only 1 cont test    */
         context->runIdx = context->runIdx? 0 : 1;
      p = tps->runTests + context->runIdx;
      }
   switch(p->action) {
      case A_RUN:
         context->pA->procState=TEST_INIT;
         break;
      case B_RUN:
         context->pB->procState=TEST_INIT;
         break;
      }
   offs = context->szCarry/BITS_PER_H_UINT;
   if (offs<sz) {
      context->szCarry -= offs * BITS_PER_H_UINT;
      return aisTest(h_ctxt, prod, buffer+offs, sz - offs);
      }
   return 1;
}
/**
 * Procedure A input is obtained by copying bits from p->data to p->aux using
 * p->bridge as position. This realigns the input to a byte boundary and
 * resolves segmentation issues. Originally implemented in BITSTREAM macros
 * performance was bad enough to justify serious tuning. Returns the updated
 * bit offset.
 */
/**
 * The BITSTREAM macros were totally inadequate for the proecedure A needs. These
 * helpers are used to implement a high performance copyBit().
 */
#define COPY_BYTE()           {c = (*src<<bit_diff_ls)|(*(src+1)>>bit_diff_rs);src++;}
#define COPY_FIRST()          if ( (int) xfr >= (8 - dst_bits)) {\
                                 *dst &= rm[dst_bits];\
                                 xfr -= 8 - dst_bits;\
                                 }\
                              else {\
                                 *dst &= rm[dst_bits] | rm_xor[dst_bits + xfr + 1];\
                                 c &= rm[dst_bits + xfr];\
                                 xfr = 0;\
                                 }\
                              xfr_bytes = xfr>>3;\
                              xfr_bits = xfr&7;\
                              *dst++ |= c;
/**
 * Each procedure A repetition moves TEST0_USED + 257*FIPS_USED bits
 * to the auxilary work space - a little more than 1MB
 */
static H_UINT copyBits(       /* RETURN: updated bit offset    */
   procA *p,                  /* IN-OUT: the context           */
   H_UINT offs,               /* IN: the input bit offset      */
   H_UINT sz)                 /* IN: number of bits to copy    */
{
   H_UINT avail = p->range;
   H_UINT need  = sz - p->bridge;
   H_UINT xfer, xfr;
   
   offs %= avail;
   xfer = (avail-offs)<need? (avail-offs) : need;
   if ((xfr = xfer)!=0) {
      static const H_UINT8 rm[]     = { 0x55, 0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe, 0xff };
      static const H_UINT8 rm_xor[] = { 0xff, 0x7f, 0x3f, 0x1f, 0x0f, 0x07, 0x03, 0x01, 0x00 };
      H_UINT8 *src = p->data+(offs>>3);
      H_UINT8 *dst = p->aux +(p->bridge>>3);
      H_UINT8 src_bits = offs&7;
      H_UINT8 dst_bits = p->bridge&7;
      H_UINT  xfr_bytes = xfr>>3;
      H_UINT  xfr_bits  = xfr&7;
      H_UINT8 c;

      if (src_bits==dst_bits) {
         if (src_bits!=0){
            c = rm_xor[dst_bits] & *src++;
            COPY_FIRST();
            }
         if (xfr_bytes!=0) {
            memcpy(dst, src, xfr_bytes);
            src += xfr_bytes;
            dst += xfr_bytes;
            }
         if (xfr_bits) {
            *dst &= rm_xor[xfr_bits];
            *dst |= rm[xfr_bits] & *src++;
            }
         }
      else {
         H_UINT bit_diff_ls, bit_diff_rs;
         if (src_bits>dst_bits) {
            bit_diff_ls = src_bits - dst_bits;
            bit_diff_rs = 8 - bit_diff_ls;
            COPY_BYTE();
            c &= rm_xor[dst_bits];
            }
         else {
            bit_diff_rs = dst_bits - src_bits;
            bit_diff_ls = 8 - bit_diff_rs;
            c = *src >> bit_diff_rs & rm_xor[dst_bits];
            }
         COPY_FIRST();
         while (xfr_bytes-- != 0) {
            COPY_BYTE();
            *dst++ = c;
            }
         if (xfr_bits!=0) {
            COPY_BYTE();
            c &= rm[xfr_bits];
            *dst &= rm_xor[xfr_bits];
            *dst |= c;
            }
         }
      }
   p->bridge += xfer;
   if (p->bridge>=sz) {
      p->bytesUsed += sz>>3;
      p->bridge = 0;
      p->testState = TEST_EVAL;
      }
   return offs + xfer;
}
/**
 * Procedure A tests 1 through 4 correspond to the fips140-1 tests. These tests
 * are conducted on the same input stream, so the calculations can be
 * done in parallel.
 */
#define  FIPS_ADD()  {\
                     if (runLength < 5)\
                        runs[runLength + (6*current)]++;\
                     else runs[5 + (6*current)]++;\
                     }

static H_UINT fips140(     /* RETURN: updated bit offset */
   procShared  *tps,       /* IN: shared data            */
   procA *p,               /* IN-OUT: the context        */
   H_UINT offs,            /* IN: starting offset        */
   H_UINT tid)             /* IN: test id                */
{
   H_UINT    poker[16];    /* counters for poker test    */
   H_UINT    ones;         /* counter for monbit test    */
   H_UINT    runs[12];     /* counters for runs tests    */
   H_UINT    runLength;    /* current run length         */
   H_UINT    maxRun;       /* largest run encountered    */
   H_UINT    current;      /* current bit                */
   H_UINT    last;         /* last bit index             */
   H_UINT    c, i, j, k;
   
   switch(p->testState) {
      case TEST_INIT:
         p->testState = TEST_INPUT;
         p->bridge = 0;
         /* fallthrough */
      case TEST_INPUT:
         offs = copyBits(p, offs, FIPS_USED);
         if (p->testState!=TEST_EVAL)
            break;
         /* fallthrough */
      case TEST_EVAL:
         maxRun = ones = runLength = 0;
         memset(poker, 0, 16*sizeof(H_UINT));
         memset(runs, 0, 12*sizeof(H_UINT));
         {
            BITSTREAM_OPEN(p->aux,0);
            last = BITSTREAM_BIT();
            for (c=i=0;i<FIPS_USED;i++) {
               current = BITSTREAM_BIT();
               if (current==last) {
                  if (++runLength>maxRun)
                     maxRun = runLength;
                  }
               else {
                  FIPS_ADD();
                  runLength = 0;
                  last = current;
                  }
               c += c + current;
               ones += current;
               if (bitstream_in==1) {
                  poker[c&15] += 1;
                  c = 0;
                  }
               else if (bitstream_in==16)
                  poker[c] += 1;
               BITSTREAM_NEXT();
               }
            FIPS_ADD();
         }
         /* 1 = monobit test  */
         k = (ones >= FIPS_ONES_HIGH || ones <= FIPS_ONES_LOW)? 1 : 0;
         p->results[tid].testResult = k | (1<<8);
         p->results[tid++].finalValue = ones;
         /* 2 = poker test    */
         for(j=k=0;j<16;j++)  k += poker[j]*poker[j];
         j = (k <= FIPS_POKER_LOW || k >= FIPS_POKER_HIGH)? 1 : 0;
         p->results[tid].testResult = j | (2<<8);
         p->results[tid++].finalValue = k;
         /* 3 = runs test     */
         for(i=j=k=0;j<12;j++)
            if (runs[j] < tps->fips_low[j%6] || runs[j] > tps->fips_high[j%6]) {
               k |= 1;
               i = runs[j];
               }
         p->results[tid].testResult = k | (3<<8);
         p->results[tid++].finalValue = i;
         /* 4 = max run length */
         k = maxRun>=FIPS_MAX_RUN? 1 : 0;
         p->results[tid].testResult = k | (4<<8);
         p->results[tid++].finalValue = maxRun;
         p->testRun = tid;
         p->testState = TEST_DONE;
      }
   return offs;
}
/**
 * Procedure A disjointness test on 48 bit strings. Rejection probability for ideal
 * RNG is 2e^-17
 */
static H_UINT test0(       /* RETURN: updated bit offset */
   procA *p,               /* IN-OUT: the context        */
   H_UINT offs,            /* IN: starting bit offset    */
   H_UINT tid)             /* IN: test id                */
{
   H_UINT i, j;

   switch(p->testState) {
      case TEST_INIT:
         p->testState = TEST_INPUT;
         p->bridge = 0;
         /* fallthrough */
      case TEST_INPUT:
         offs = copyBits(p, offs, TEST0_USED);
         if (p->testState!=TEST_EVAL)
            break;
         /* fallthrough */
      case TEST_EVAL:
         qsort(p->aux, TEST0_LENGTH, 6, test0cmp);
         for (i=6,j=0;i<TEST0_LENGTH && j==0;i+=6)
            if (!memcmp(p->aux+i-6, p->aux+i, 6)) {
               j=1;
               }
         p->results[tid].testResult = j;
         p->results[tid++].finalValue = i;
         p->testRun = tid;
         p->testState = TEST_DONE;
      }
   return offs;
}
/**
 * Comparison method for the test0 sort
 */
static int test0cmp(const void *aa, const void *bb)
{
   return memcmp(aa,bb,6);
}
/**
 * Procedure A autocorrelation test. Brutal bit twiddling. Uses same
 * data as FIPS - no update to bit offset
 */
static H_UINT test5(       /* RETURN: updated bit offset    */
   procA *p,               /* IN-OUT: the context           */
   H_UINT offs,            /* IN: starting bit offset       */
   H_UINT tid)             /* IN: test id                   */
{
   H_UINT8  *dp = (H_UINT8 *)p->aux;
   H_UINT j, k, max, tau, Z_tau;

   /**
    * Because this test is so slow it can be skipped on one or more repetitions
    */
   if (0 != (p->options & A_CYCLE)) {
      j = p->options & A_CYCLE;
      if (j==0 || ((tid-1)/5 % j)!=0) {
         p->results[tid++].testResult = 0xff00;
         p->testRun = tid;
         p->testState = TEST_DONE;
         return offs;
         }
      }
   /**
    * This test always uses the same data as test1 through test4
    */
   for (max = k = 0,tau=1;tau<=TEST5_LENGTH;tau++){
      Z_tau = abs( (int) test5XOR(dp, tau) - 2500);
      if (Z_tau > max) {
         max = Z_tau;
         k = tau - 1;
         }
      }
   dp += TEST5_LENGTH/8;
   Z_tau = test5XOR(dp, k + 1);
   j = 5<<8; 
   if (( Z_tau <= 2326) || ( Z_tau >= 2674))
      j  |= 1;
      
   p->results[tid].testResult = j;
   p->results[tid++].finalValue = Z_tau;
   p->testRun = tid;
   p->testState = TEST_DONE;
   return offs;
}
/**
 * The test5 reference implementation looks something like this:
 *
 *   for(i=0,j=shift;i<TEST5_LENGTH;i++,j++)
 *      rv += 1 & (((src[i>>3]>>(i & 7))) ^ ((src[j>>3]>>(j & 7))));
 *   return rv;
 *
 * A high performance optimization using multi-byte casts is 3x as fast as the above but blows up
 * because of alignment issues (leftovers from the test0 implementation)
 * The optimized single byte optimization is 2x as fast as the above but uses no alignment games
 */
static H_UINT test5XOR(H_UINT8 *src, H_UINT shift)
{
   H_UINT8  *src1;
   H_UINT   i,rest, rv;
   
   src1 = src + (shift>>3);
   shift &= 7;
   rest = 8 - shift;
   for(i=rv=0;i<(TEST5_LENGTH>>3);i++) {
      H_UINT8 lw   = *src++;
      H_UINT8 rw   = *src1++;
      H_UINT8 w;

      for (w = (lw & (0xff>>shift)) ^ (rw>>shift);w!=0;w>>=1)
         rv += w & 1;
      for (w = (lw>>rest) ^ (*src1 & (0xff>>rest));w!=0;w>>=1)
         rv += w & 1;
      }
   return rv;
}
/**
 * Procedure B uniform distribution test for parameter set (1,100000,0.25). Very simple, just
 * count bits. Fixed input size, deadman not needed
 */
#define  SIDEWAYS_ADD(c,i)   {H_UINT in = i;\
                              in -= ((in >> 1) & 0x55555555);\
                              in = (in & 0x33333333) + ((in >> 2) & 0x33333333);\
                              c=(((in + (in >> 4)) & 0x0F0F0F0F) * 0x01010101) >> 24;\
                              }

static H_UINT test6a(      /* RETURN: bit offset      */
   procB *p,               /* IN-OUT: the context     */
   H_UINT offs,            /* IN: starting bit offset */
   H_UINT tid)             /* IN: test id             */
{
   H_UINT r = p->range - offs;
   H_UINT i=0,j=p->bridge;

   switch(p->testState) {
      case TEST_INIT:
         j = p->counter[0] = 0;
         p->testState = TEST_INPUT;
         /* fallthrough */
      case TEST_INPUT:
         {
            BITSTREAM_OPEN(p->noise,offs);
            H_UINT c;
            
            /* align to a byte boundary, then shift gears to gobble bytes */
            while(i < r && j < AIS_LENGTH && bitstream_in != 0x80){
               p->counter[0] += BITSTREAM_BIT();BITSTREAM_NEXT();
               i++;j++;
               }
            /* align to a word boundary, then shift gears to gobble words */
            while((i+8) < r && (j+8) < AIS_LENGTH) {
               if (0==((char *)bitstream_src - (char *)p->noise) % sizeof(H_UINT))
                  break;
               SIDEWAYS_ADD(c, *bitstream_src++);
               p->counter[0] += c;
               i+=8;j+=8;
               }
            /* gobble all words available */
            while((i+BITS_PER_H_UINT) < r && (j+BITS_PER_H_UINT) < AIS_LENGTH) {
               SIDEWAYS_ADD(c, *((H_UINT *)bitstream_src));
               bitstream_src += sizeof(H_UINT);
               p->counter[0] += c;
               i+=BITS_PER_H_UINT;j+=BITS_PER_H_UINT;
               }
            /* shift back to bits & cleanup the leftovers */
            for(;i < r && j < AIS_LENGTH;i++,j++) {
               p->counter[0] += BITSTREAM_BIT();BITSTREAM_NEXT();
               }
            p->bitsUsed += i;
            if (j < AIS_LENGTH) {
               p->bridge = j;
               break;
               }
         }
         /* fallthrough */
      case TEST_EVAL:
         p->results[p->testNbr].finalValue = (double)(p->counter[0]) / (double) AIS_LENGTH;
         r = tid << 8;
         if (p->results[p->testNbr].finalValue <= 0.25 || p->results[p->testNbr].finalValue >= 0.75)
            r |= 1;
         p->results[p->testNbr++].testResult = r;
         p->testState = TEST_DONE;
      }
   return i+offs;
}
/**
 * Context is saved and restored using inactive members of the anchor.
 */
#define  RESTORE8(a,b,c,d) a=p->bridge;b=p->full;c=p->einsen[0];\
                           d=p->results[p->testNbr].finalValue
#define  SAVE8(a,b,c,d)    p->bridge=a;p->full=b;p->einsen[0]=c;\
                           p->results[p->testNbr].finalValue = d
/**
 * Procedure B entropy estimator (Coron). Find the distribution of the distance between
 * bytes and their predecessors. Fixed input size, no deadman needed.
 */
static H_UINT test8(       /* RETURN: bit offset      */
   procShared  *tps,       /* IN-OUT: shared data     */
   procB *p,               /* IN-OUT: the context     */
   H_UINT offs,            /* IN: starting bit offset */
   H_UINT tid)             /* IN: test id             */
{
   H_UINT   hilf, j, k, r, i=0;
   double   TG=0.0;
   
   switch(p->testState) {
      case TEST_INIT:
         memset(p->lastpos, 0, 256*sizeof(H_UINT));
         SAVE8(0,0,0,0.0);
         p->testState  = TEST_INPUT;
         /* fallthrough */
      case TEST_INPUT:
         RESTORE8(k,j,hilf,TG);
         r = p->range - offs;
         {
            H_UINT align;
            /* gobble bits up to a byte boundary */
            BITSTREAM_OPEN(p->noise,offs);
            for(;j<8 && i<r && bitstream_in!=0x80;i++,j++) {
               hilf += hilf+(BITSTREAM_BIT());BITSTREAM_NEXT();
               }
            align  = (j &= 7);
            while(i<r) {
               if (j==0 && (i+8)<r) {     /* gobble a byte     */
                  hilf = (0xff & (bitstream_src[0]<<(8-align))) | (bitstream_src[1]>>align);
                  bitstream_src++;i+=8;j=8;
                  }
               for(;j<8 && i<r;i++,j++) { /* gobble loose bits */
                  hilf += hilf+(BITSTREAM_BIT());BITSTREAM_NEXT();
                  }
               if (j!=8)
                  break;
               if (k<Q)
                  p->lastpos[hilf] = k++;
               else {
                  TG += tps->G[k - p->lastpos[hilf]];
                  p->lastpos[hilf] = k++;
                  if (k==(K+Q)) {
                     p->testState = TEST_EVAL;
                     break;
                     }  
                  }
               j = hilf = 0;
               }
            if (p->testState==TEST_INPUT) {
               SAVE8(k,j,hilf,TG);
               }
         }
         p->bitsUsed += i;
         if (p->testState == TEST_INPUT)
            break;
         /* fallthrough */
      case TEST_EVAL:
         tps->lastCoron = p->results[p->testNbr].finalValue = TG/(double)K;
         r = tid<<8;
         if (p->results[p->testNbr].finalValue <= 7.967)
            r |= 1;
         p->results[p->testNbr++].testResult = r;
         p->testState = TEST_DONE;
      }
   return i+offs;
}
#endif
