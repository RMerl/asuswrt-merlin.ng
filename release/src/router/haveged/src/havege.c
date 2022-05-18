/**
 ** Simple entropy harvester based upon the havege RNG
 **
 ** Copyright 2018-2022 Jirka Hladky hladky DOT jiri AT gmail DOT com
 ** Copyright 2009-2014 Gary Wuertz gary@issiweb.com
 ** Copyright 2011-2012 BenEleventh Consulting manolson@beneleventh.com
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
 **
 */
/**
 * This compile unit implements the havege algorithm as an inteface to
 * either a single collector in the calling process or an interface to
 * multiple collector processes (experimental).
 */
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include "havegetest.h"
#include "havegetune.h"
/**
 * The library version interface results in a pair of version definitions
 * which must agree yet must also be string literals. No foolproof build
 * mechanism could be devised to ensure this, so a run-time check was added
 * instead - if the two definitions do not agree, the interface is diabled.
 */
#define  INTERFACE_DISABLED() strcmp(PACKAGE_VERSION,HAVEGE_PREP_VERSION)

#if  NUMBER_CORES>1
#include <errno.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <sched.h>
/**
 * Collection thread directory
 */
typedef struct {
   pid_t    main;       /* the main thread         */
   H_UINT   exit_ct;    /* shutdown counter        */
   H_UINT   count;      /* associated count        */
   H_UINT   last;       /* last output             */
   H_UINT   *out;       /* buffer pointer          */
   H_UINT   fatal;      /* fatal error in last     */
   sem_t    flags[1];   /* thread signals          */
} H_THREAD;
/**
 * Local prototypes
 */
static int  havege_exit(H_PTR h_ptr);
static void havege_ipc(H_PTR h_ptr, H_UINT n, H_UINT sz);
static int  havege_rngChild(H_PTR h_ptr, H_UINT cNumber);
static void havege_unipc(H_PTR h_ptr);
#endif
/**
 * Main allocation
 */
#ifdef ONLINE_TESTS_ENABLE
typedef struct {
   struct h_anchor   info;       /* Application anchor      */
   HOST_CFG          cfg;        /* Runtime environment     */
   procShared        std;        /* Shared test data        */
} H_SETUP;

static int    testsConfigure(H_UINT *tot, H_UINT *run, char *options);
static void   testsStatus(procShared *tps, char *tot, char *prod);

static void   testReport(H_COLLECT * h_ctxt, H_UINT action, H_UINT prod, H_UINT state, H_UINT ct);
static void   testReportA(H_PTR h, procA *context);
static void   testReportB(H_PTR h, procB *context);

#else
typedef struct {
   struct h_anchor   info;       /* Application anchor      */
   HOST_CFG          cfg;        /* Runtime environment     */
} H_SETUP;

#endif
/**
 * Local prototypes
 */
static void havege_mute(const char *format, ...);
/**
 * Initialize the environment based upon the tuning survey. This includes,
 * allocation the output buffer (in shared memory if mult-threaded) and
 * fitting the collection code to the tuning inputs.
 */
H_PTR havege_create(             /* RETURN: app state    */
  H_PARAMS *params)              /* IN: input params     */
{
   H_SETUP    *anchor;
   HOST_CFG   *env;
   H_PTR      h = 0;
   H_UINT     n = params->nCores;
   H_UINT     sz = params->ioSz;

   if (INTERFACE_DISABLED())
      return NULL;
   if (0 == n)
      n = 1;
   if (0 == sz)
      sz = DEFAULT_BUFSZ;
   anchor = (H_SETUP *)calloc(sizeof(H_SETUP),1);
   if (NULL==anchor)
      return h;
   h   = &anchor->info;
   h->print_msg = params->msg_out==0? havege_mute : params->msg_out;
   h->metering  = params->metering;
   env = &anchor->cfg;
   havege_tune(env, params);
   h->error           = H_NOERR;
   h->arch            = ARCH;
   h->inject          = params->injection;
   h->n_cores         = n;
   h->havege_opts     = params->options;
   h->i_collectSz     = params->collectSize==0? NDSIZECOLLECT : params->collectSize;
   h->i_readSz        = sz;
   h->tuneData        = env;
   h->cpu             = &env->cpus[env->a_cpu];
   h->instCache       = &env->caches[env->i_tune];
   h->dataCache       = &env->caches[env->d_tune];
#ifdef ONLINE_TESTS_ENABLE
   {
      static const H_UINT tests[5]  = {B_RUN, A_RUN};

      H_UINT tot=0,run=0;
      H_UINT i, j;

      procShared  *tps = (procShared *)&anchor->std;
      if (testsConfigure(&tot, &run, params->testSpec)) {
         h->error = H_NOTESTSPEC;
         return h;
         }
      for(i=j=0;i<2;i++)
         if (0!=(tot & tests[i])) {
            tps->testsUsed |= tests[i];
            tps->totTests[j].action  = tests[i];
            tps->totTests[j++].options = tot;
            }
      for(i=j=0;i<2;i++)
         if (0!=(run & tests[i])) {
            tps->testsUsed |= tests[i];
            tps->runTests[j].action  = tests[i];
            tps->runTests[j++].options = run;
            }
      testsStatus(tps, tps->totText, tps->prodText);
      tps->report = testReport;
      h->testData = tps;
      if (havege_test(tps, params)) {
         h->error = H_NOTESTMEM;
         return h;
         }
   }
#endif
#if NUMBER_CORES>1
   havege_ipc(h, n, sz);
#else
   h->io_buf  = malloc(sz);
   h->threads = NULL;
#endif
   if (NULL==h->io_buf) {
      h->error = H_NOBUF;
      return h;
      }
   havege_ndsetup(h);
   return h;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
void havege_reparent(
  H_PTR hptr)
{
#if NUMBER_CORES>1
   H_THREAD *t = (H_THREAD *)hptr->threads;
   if (0 == t)
      return; /* single-threaded */

   t->main = getpid();
#endif
}
#pragma GCC diagnostic pop

/**
 * Destructor. In a multi-collector build, this method should be called from a signal handler
 * to avoid creating processes.
 */
void havege_destroy(       /* RETURN: none            */
  H_PTR hptr)              /* IN-OUT: app anchor      */
{
   if (NULL != hptr) {
      H_COLLECT *htemp;
      void *temp;
#if NUMBER_CORES>1
      if (!havege_exit(hptr))
         return;           /* only main thread continues  */
#endif
      if (0 != (temp=hptr->io_buf)) {
         hptr->io_buf = 0;
         free(temp);
         }
#ifdef ONLINE_TESTS_ENABLE
      if (0 != (temp=hptr->testData)) {
         double *g = ((procShared *)temp)->G;
         hptr->testData = 0;
         if (0 != g)
            free(g);
         }
#endif
      if (0 != (htemp=hptr->collector)) {
         hptr->collector = 0;
         havege_nddestroy(htemp);
         }
      free(hptr);
      }
}
/**
 * Read random words. In the single-collector case, input is read by the calling
 * the collection method directly. In the multi-collector case, the request info is
 * signaled to the collector last read and the caller waits for a completion signal.
 */
int havege_rng(            /* RETURN: number words read     */
  H_PTR h,                 /* IN-OUT: app state             */
  H_UINT *buffer,          /* OUT: read buffer              */
  H_UINT sz)               /* IN: number words to read      */
{
#if NUMBER_CORES>1
   H_THREAD    *t = (H_THREAD *) h->threads;
   
   t->count = sz;
   t->out   = buffer;
   if (0!=sem_post(&t->flags[t->last]))
      h->error = H_NORQST;
   else if (0!=sem_wait(&t->flags[h->n_cores]))
      h->error = H_NOCOMP;
   else if (H_NOERR != t->fatal)
      h->error = t->fatal;
#else
   H_UINT i;

   for(i=0;i<sz;i++)
      buffer[i] = havege_ndread((H_COLLECT *)h->collector);
   h->error = ((H_COLLECT *)h->collector)->havege_err;
#endif
   return h->error==(H_UINT)H_NOERR? (int) sz : -1;
}
/**
 * Start the entropy collector.
 */
int havege_run(            /* RETURN: NZ on failure   */
  H_PTR h)                 /* IN-OUT: app anchor      */
{
   int        i = 0;

#if NUMBER_CORES>1
   for(i = 0; i < h->n_cores;i++)
      if (0 == havege_rngChild(h, i))
         return 1;
#else
   if (NULL==(h->collector = havege_ndcreate(h, i)))
      return 1;
#endif
   return 0;
}
/**
 * Report concealed setup data
 */
void havege_status(        /* RETURN: none     */
  H_PTR h_ptr,             /* IN: app state    */
  H_STATUS h_sts)          /* OUT: app state   */
{
   if (0 != h_sts) {
      HOST_CFG   *en = (HOST_CFG *)  (h_ptr->tuneData);
      CACHE_INST *cd = (CACHE_INST *)(h_ptr->dataCache);
      CACHE_INST *ci = (CACHE_INST *)(h_ptr->instCache);
      CPU_INST   *cp = (CPU_INST *)  (h_ptr->cpu);
      procShared *ps = (procShared *)(h_ptr->testData);

      h_sts->version        = HAVEGE_PREP_VERSION;
      h_sts->buildOptions   = en->buildOpts;
      h_sts->cpuSources     = en->cpuOpts;
      h_sts->i_cacheSources = en->icacheOpts;
      h_sts->d_cacheSources = en->dcacheOpts;
      h_sts->vendor         = cp->vendor;
      h_sts->d_cache        = cd->size;
      h_sts->i_cache        = ci->size;
      h_sts->tot_tests      = (0 != ps)? ps->totText :"";
      h_sts->prod_tests     = (0 != ps)? ps->prodText :"";
      if (0 != ps) {
         memcpy(h_sts->n_tests, ps->meters, (H_OLT_PROD_B_P+1) * sizeof(H_UINT));
         h_sts->last_test8     = ps->lastCoron;
         }
      }
}
/**
 * Standard status presetations
 */
int  havege_status_dump(   /* RETURN: output length   */
  H_PTR hptr,              /* IN: app state           */
  H_SD_TOPIC topic,        /* IN: presentation topic  */
  char *buf,               /* OUT: output area        */
  size_t len)              /* IN: size of buf         */
{
   struct   h_status status;
   int      n = 0;

   if (buf != 0) {
      *buf = 0;
      len -= 1;
      havege_status(hptr, &status);
      switch(topic) {
         case H_SD_TOPIC_BUILD:
            n += snprintf(buf, len, "ver: %s; arch: %s; vend: %s; build: (%s); collect: %uK",
               status.version,
               hptr->arch,
               status.vendor,
               status.buildOptions,
               hptr->i_collectSz/1024
               );
            break;
         case H_SD_TOPIC_TUNE:
            n += snprintf(buf, len, "cpu: (%s); data: %uK (%s); inst: %uK (%s); idx: %u/%u; sz: %u/%u",
               status.cpuSources,
               status.d_cache,
               status.d_cacheSources,
               status.i_cache,
               status.i_cacheSources,
               hptr->i_maxidx - hptr->i_idx, hptr->i_maxidx,
               hptr->i_sz,  hptr->i_maxsz
               );
            break;
         case H_SD_TOPIC_TEST:
            {
               H_UINT   m;

               if (strlen(status.tot_tests)>0) {
                  n += snprintf(buf+n, len-n, "tot tests(%s): ", status.tot_tests);
                  if ((m = status.n_tests[ H_OLT_TOT_A_P] + status.n_tests[ H_OLT_TOT_A_F])>0)
                     n += snprintf(buf+n, len-n, "A:%u/%u ", status.n_tests[ H_OLT_TOT_A_P], m);
                  if ((m = status.n_tests[ H_OLT_TOT_B_P] + status.n_tests[ H_OLT_TOT_B_F])>0)
                     n += snprintf(buf+n, len, "B:%u/%u ", status.n_tests[ H_OLT_TOT_B_P], m);
                  }
               if (strlen(status.prod_tests)>0) {
                  n += snprintf(buf+n, len-n, "continuous tests(%s): ", status.prod_tests);
                  if ((m = status.n_tests[ H_OLT_PROD_A_P] + status.n_tests[ H_OLT_PROD_A_F])>0)
                     n += snprintf(buf+n, len-n, "A:%u/%u ", status.n_tests[ H_OLT_PROD_A_P], m);
                  if ((m = status.n_tests[ H_OLT_PROD_B_P] + status.n_tests[ H_OLT_PROD_B_F])>0)
                     n += snprintf(buf+n, len, "B:%u/%u ", status.n_tests[ H_OLT_PROD_B_P], m);
                  }
               if (n>0)
                  n += snprintf(buf+n, len-n, " last entropy estimate %g", status.last_test8);
            }
            break;
         case H_SD_TOPIC_SUM:
            {
               char   units[] = {'T', 'G', 'M', 'K', 0};
               double factor[2];
               factor[0] = 1024.0 * 1024.0 * 1024.0 * 1024.0;
               factor[1] = factor[0];
               double sz = ((double)hptr->n_fills * hptr->i_collectSz) * sizeof(H_UINT);
               double ent = ((double) hptr->n_entropy_bytes);
               int i[2];

               for (i[0]=0;0 != units[i[0]];i[0]++) {
                  if (sz >= factor[0])
                     break;
                  factor[0] /= 1024.0;
                  }
               for (i[1]=0;0 != units[i[1]];i[1]++) {
                  if (ent >= factor[1])
                     break;
                  factor[1] /= 1024.0;
                  }
               n = snprintf(buf, len, "fills: %u, generated: %.4g %c bytes, RNDADDENTROPY: %.4g %c bytes",
                  hptr->n_fills,
                  sz / factor[0],
                  units[i[0]],
                  ent / factor[1],
                  units[i[1]]
                  );
            }
            break;
         }
      }
   return n;
}
/**
 * Return-check library prep version. Calling havege_version() with a NULL version
 * returns the definition of HAVEGE_PREP_VERSION used to build the library. Calling
 * with HAVEGE_PREP_VERSION as the version checks if this headers definition is
 * compatible with that of the library, returning NULL if the input is incompatible
 * with the library. 
 */
const char *havege_version(const char *version)
{
   if (INTERFACE_DISABLED())
      return NULL;
   /**
    * Version check academic at the moment, but initial idea is to do a table
    * lookup on the library version to get a pattern to match against the
    * input version.
    */
   if (version) {
      H_UINT l_interface=0, l_revision=0, l_age=0;
      H_UINT p, p_interface, p_revision, p_patch;
      
#ifdef HAVEGE_LIB_VERSION
      sscanf(HAVEGE_LIB_VERSION, "%u:%u:%u", &l_interface, &l_revision, &l_age);
#endif
      (void)l_interface;(void)l_revision;(void)l_age;(void)p_patch;  /* No check for now */

      p = sscanf(version, "%u.%u.%u", &p_interface, &p_revision, &p_patch);
      if (p!=3 || p_interface != 1 || p_revision != 9)
         return NULL;
      }
   return HAVEGE_PREP_VERSION;
}

/**
 * Place holder if output display not provided
 */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
static void havege_mute(   /* RETURN: none            */
   const char *format,     /* IN: printf format       */
   ...)                    /* IN: args                */
{
   ;
}
#pragma GCC diagnostic pop

#if NUMBER_CORES > 1
/**
 * Cleanup collector(s). In a multi-collector environment, need to kill
 * children to avoid zombies.
 */
static int havege_exit(    /* RETURN: NZ if child  */
  H_PTR h_ptr)             /* IN: app state        */
{
   H_THREAD *t = (H_THREAD *)h_ptr->threads;
   pid_t  p;
   H_UINT i;
   
   if (0 == t)
      return 0;            /* Must be main thread */
   t->fatal = H_EXIT;
   for(i=0;i<t->exit_ct;i++)
      (void)sem_post(&t->flags[i]);
   if (getpid() != t->main)
      return 1;
   do {                    /* Wait for children    */
      p = wait(NULL);
      } while(p != -1 && errno != ECHILD);
   for(i=0;i<t->exit_ct;i++)
      (void)sem_destroy(&t->flags[i]);
   if (i==h_ptr->n_cores)
      (void)sem_destroy(&t->flags[i]);
   havege_unipc(h_ptr);    /* unmap memory         */
   return 0;
}

/**
 * Initialize IPC mechanism. This consists of setting up a shared memory area
 * containing the output buffer and the collection thread directory.
 */
static void havege_ipc(    /* RETURN: None            */
   H_PTR h,                /* IN: app state           */
   H_UINT n,               /* IN: number of threads   */
   H_UINT sz)              /* IN: size of buffer      */
{
   void     *m;
   H_THREAD *t;
   H_UINT   m_sz; 
   int      i;

   if (n > NUMBER_CORES) {
      h->error = H_NOCORES;
      return;
      }
   m = mmap(NULL,
            m_sz = sz + sizeof(H_THREAD) + n * sizeof(sem_t),
            PROT_READ | PROT_WRITE,
            MAP_SHARED | MAP_ANONYMOUS,
            0,
            0);
   if (m != MAP_FAILED) {
      h->io_buf  = m;
      h->m_sz = m_sz;
      t = (H_THREAD *)((char *) m + sz);
      memset(t, 0, sizeof(H_THREAD));
      t->main = getpid();
      h->threads = t;
      for(i=0;i<=n;i++)
         if(sem_init(&t->flags[i],1,0) < 0) {
            h->error = H_NOINIT;
            break;
            }
      t->exit_ct = i;
      }
}
/**
 * Child harvester task. If task fails to start H_PTR::error will be set to reason.
 * If task fails after start, H_THREAD::fatal will be set to the reason and a completion
 * will be posted to prevent the main thread from hanging waiting for a response.
 */
static int havege_rngChild(/* RETURN: none         */   
  H_PTR h_ptr,             /* IN: app state        */
  H_UINT cNumber)          /* IN: collector index  */
{
   H_COLLECT   *h_ctxt = 0;
   H_THREAD    *thds = (H_THREAD *) h_ptr->threads;
   H_UINT      cNext, i, r;
   int         pid;

   switch(pid=fork()) {
      case 0:
         h_ctxt = havege_ndcreate(h_ptr, cNumber);
         if (NULL != h_ctxt) {
            cNext = (cNumber + 1) % h_ptr->n_cores;
            while(1) {
               if (0!=sem_wait(&thds->flags[cNumber])) {
                  thds->fatal = H_NOWAIT;
                  break;
                  }
               if (H_NOERR != thds->fatal) {
                  havege_nddestroy(h_ctxt);
                  exit(0);
                  }
               thds->last = cNumber;
               r = h_ctxt->havege_szFill - h_ctxt->havege_nptr;
               if (thds->count < r)
                  r = thds->count; 
               for(i=0;i<r;i++)
                  thds->out[i] = havege_ndread(h_ctxt);
               thds->fatal = h_ctxt->havege_err;
               if (0==(thds->count -= i)) {
                  if (0!=sem_post(&thds->flags[h_ptr->n_cores])) {
                     thds->fatal = H_NODONE;
                     break;
                     }
                  continue;
                  }
               thds->out += i;
               if (0!=sem_post(&thds->flags[cNext])) {
                  thds->fatal = H_NOPOST;
                  break;
                  }
#ifdef HAVE_SCHED_YIELD
               (void)sched_yield();
#endif
               (void)havege_ndread(h_ctxt);
               h_ctxt->havege_nptr = 0;
               }
            havege_nddestroy(h_ctxt);
            }
         else thds->fatal = h_ptr->error;                /* h_ptr is a copy!!    */
         (void)sem_post(&thds->flags[h_ptr->n_cores]);   /* announce death!      */
         break;
      case -1:
         h_ptr->error = H_NOTASK;
      }
   return pid;
}
/**
 * Unmap memory
 */
static void havege_unipc(  /* RETURN: none         */
  H_PTR h_ptr)             /* IN: app state        */
{
   if (0 != h_ptr->m_sz)  {
      munmap(h_ptr->io_buf, h_ptr->m_sz);
      h_ptr->io_buf = 0;
      }

}
#endif
#ifdef ONLINE_TESTS_ENABLE
/**
 * Interpret options string as settings. The option string consists of terms
 * like "[t|c][a[1-8][w]|b[w]]". 
 */
static int testsConfigure( /* RETURN: non-zero on error  */
   H_UINT *tot,            /* OUT: tot test options      */
   H_UINT *run,            /* OUT: run test options      */
   char *options)          /* IN: option string          */
{
   H_UINT section=0;
   int   c;

   if (options==0)
      options = DEFAULT_TEST_OPTIONS;
   while(0 != (c = *options++)) {
      switch(c) {
         case 'T': case 't':        /* tot test          */
            section = 't';
            *tot = 0;
            break;
         case 'C': case 'c':        /* production test   */
            section = 'c';
            *run = 0;
            break;
         case 'A': case 'a':
            if (!section) return 1;
            c  = atoi(options);
            if (c >= 1 && c < 9) {
               c = 1<<c;
               options +=1;
               }
            else c = 0;
            c |= A_RUN;
            if (*options=='W' || *options=='w') {
               c |= A_WARN;
               options++;
               }
            if (section=='t')
               *tot |= c;
            else *run |= c;
            break;
         case 'B': case 'b':
            if (!section) return 1;
            c = B_RUN;
            if (*options=='W' || *options=='w') {
               c |= B_WARN;
               options++;
               }
            if (section=='t')
               *tot |= c;
            else *run |= c;
            break;
         default:
            return 1;
         }
      }
   return 0;
}
/**
 * Show test setup. Output strings are [A[N]][B]..
 */
static void testsStatus(    /* RETURN: test config     */
   procShared  *tps,        /* IN: shared data         */
   char *tot,               /* OUT: tot tests          */
   char *prod)              /* OUT: production tests   */
{
   procInst    *p;
   char        *dst = tot;
   H_UINT      i, j, k, m;
   
   *dst = *tot = 0;
   p = tps->totTests;
   for(i=0;i<2;i++,p = tps->runTests, dst = prod) {
      for(j=0;j<2;j++,p++) {
         switch(p->action) {
            case A_RUN:
               *dst++ = 'A';
               if (0!=(m = p->options & A_CYCLE)) {
                  for(k=0;m>>=1 != 0;k++);
                  *dst++ = '0' + k;
                  }
               if (0 != (p->options & A_WARN))
                  *dst++ = 'w';
               break;
            case B_RUN:
               *dst++ = 'B';
               if (0 != (p->options & B_WARN))
                  *dst++ = 'w';
               break;
            }
         *dst = 0;
         }
      }
}
/**
 * Reporting unit for tests
 */
static void testReport(
   H_COLLECT * h_ctxt,     /* IN-OUT: collector context     */
   H_UINT action,          /* IN: A_RUN or B_RUN            */
   H_UINT prod,            /* IN: 0==tot, else continuous   */
   H_UINT state,           /* IN: state variable            */
   H_UINT ct)              /* IN: bytes consumed            */
{
   H_PTR       h_ptr = (H_PTR)(h_ctxt->havege_app);
   onlineTests *context = (onlineTests *) h_ctxt->havege_tests;
   char        *result;
   
   switch(state) {
      case TEST_DONE:   result = "success";           break;
      case TEST_RETRY:  result = "retry";             break;
      case TEST_IGNORE: result = "warning";           break;
      default:          result = "failure";
      }
   h_ptr->print_msg("AIS-31 %s procedure %s: %s %d bytes fill %d\n",
      prod==0? "tot" : "continuous", action==A_RUN? "A":"B", result, ct, h_ptr->n_fills);
   if (0 != (h_ptr->havege_opts & (H_DEBUG_OLTR|H_DEBUG_OLT)))
      switch(action){
         case A_RUN:
            testReportA(h_ptr, context->pA);
            break;
         case B_RUN:
            testReportB(h_ptr, context->pB);
            break;
         }
}
/**
 * Reporting unit for procedure A. Results are 0-257*(1-[4 or 5])
 */
static void testReportA(       /* RETURN: nothing               */
   H_PTR h_ptr,               /* IN: application instance      */
   procA *p)                  /* IN: proc instance             */
{
   static const char * pa_tests[6] = {"test0","test1","test2","test3","test4","test5"};

   H_UINT ran[6],sum[6];
   H_UINT ct, i, j, k;
         
   for (i=0;i<6;i++)
      ran[i] = sum[i] = 0;
   for(i=0;i<p->testRun;i++){
      ct = p->results[i].testResult;
      j = ct>>8;
      ran[j] += 1;
      if (0==(ct & 0xff))
         sum[j] += 1;
      }
   h_ptr->print_msg("procedure A: %s:%d/%d, %s:%d/%d, %s:%d/%d, %s:%d/%d, %s:%d/%d, %s:%d/%d\n",
      pa_tests[0], sum[0], ran[0],
      pa_tests[1], sum[1], ran[1],
      pa_tests[2], sum[2], ran[2],
      pa_tests[3], sum[3], ran[3],
      pa_tests[4], sum[4], ran[4],
      pa_tests[5], sum[5], ran[5]
      );
   for(i=k=0;i<p->testRun;i++){
      ct = p->results[i].testResult;
      j = ct>>8;
      if (j==1)
         k+=1;
      if (0!=(ct & 0xff))
         h_ptr->print_msg("  %s[%d] failed with %d\n", pa_tests[j%6],k,p->results[i].finalValue);
      }
}
/**
 * Reporting unit for procedure B. Results are 6a-6b-7a[0]-7a[1]-7b[0]-7b[1]-7b[2]-7b[3]-8
 */
static void testReportB(       /* RETURN: nothing               */
   H_PTR h_ptr,               /* IN: application instance      */
   procB *p)                  /* IN: proc instance             */
{
   static const char * pb_tests[5] = {"test6a","test6b","test7a","test7b","test8"};

   H_UINT ct, i, j, ran[5],sum[5];

   for (i=0;i<5;i++) {
      ran[i]  = sum[i] = 0;
      }
   for(i=0;i<p->testNbr;i++){
      ct = p->results[i].testResult;
      j = ct>>8;
      ran[j] += 1;
      if (0==(ct & 0xff))
         sum[j] += 1;
      }
   h_ptr->print_msg("procedure B: %s:%d/%d, %s:%d/%d, %s:%d/%d, %s:%d/%d, %s:%d/%d\n",
      pb_tests[0], sum[0], ran[0],
      pb_tests[1], sum[1], ran[1],
      pb_tests[2], sum[2], ran[2],
      pb_tests[3], sum[3], ran[3],
      pb_tests[4], sum[4], ran[4]
      );
   for(i=0;i<5;i++)
      ran[i] = p->testNbr;
   for(i=0;i<p->testNbr;i++){
      ct = p->results[i].testResult;
      j = ct>>8;
      if (i < ran[j]) ran[j] = i;
      if (0!=(ct & 0xff))
         h_ptr->print_msg("  %s[%d] failed with %g\n", pb_tests[j],i-ran[j],p->results[i].finalValue);
      }
}
#endif

