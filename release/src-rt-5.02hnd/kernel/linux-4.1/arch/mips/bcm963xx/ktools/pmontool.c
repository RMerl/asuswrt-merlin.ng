#if defined(CONFIG_BCM_KF_PMON)
/*
<:copyright-BRCM:2007:DUAL/GPL:standard

   Copyright (c) 2007 Broadcom 
   All Rights Reserved

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License, version 2, as published by
the Free Software Foundation (the "GPL").

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.


A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.

:>
*/
/*
 *------------------------------------------------------------------------------
 *
 * Filename: pmontool.c
 *
 * Implements a rudimentary API to the MIPS performance counters.
 * Statically built into the kernel for usage in kernel profiling.
 *
 *  pmon_bgn: Sets up a baseline counter reference used in subsequent pmon_log
 *  pmon_log(eventid): This API must be called with sequential eventid's.
 *  pmon_end: Termintaes one iteration, computes logged counter deltas and
 *            accumulates them. After a sample number of iterations, the
 *            accumulated counters are averaged and the next performance
 *            counter type is applied.
 *  pmon_clr: This function may be invoked to skip the current iteration from
 *            the test average.
 *
 *  pmon_reg: Register a string description with an even point, for us in report
 *  pmon_enable: May be invoked by C code or by the control utility to start
 *            the monitoring.
 *
 *  - Measurement configuration options:
 *    skip: Delayed enabling after a number of iterations
 *    iter: Average over number of iteration per metric
 *    metric: Compute all=1 or only cyclecount=0 metrics
 *
 * Implements a character driver for configuration from pmon control utility
 *
 * Uncomment PMON_UT to enable the unit test mode of PMON tool. In this mode,
 * upon enabling PMON via the control utility, the pmon_ut() will be invoked.
 * pmon_ut() consists of a sample loop that has been instrumented with 
 * microsecond delays (caliberated kernel function) in between instrumentation.
 * The report function will be invoked on completion of the specified test.
 *
 *------------------------------------------------------------------------------
 */

#if defined( CONFIG_PMON )

#include <asm/pmonapi.h>
#include <linux/delay.h>
#include <linux/version.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <bcm_cpu.h>

// #define PMON_UT     /* PMON Unit Testing */

#ifdef PMON_COLOR
#define _H_                     "\e[0;36;44m"
#define _N_                     "\e[0m"
#define _R_                     "\e[0;31m"
#define _G_                     "\e[0;32m"
#define _B_                     "\e[0;34m"
#define _NL_                    _N_ "\n"
#else
#define _H_
#define _N_
#define _R_
#define _G_
#define _B_
#define _NL_                    "\n"
#endif

typedef struct PMetricConfig {
    uint32_t perfGbl;
    uint32_t perfCtl;
} PMetricConfig_t;

/* Default values correspond to CyclesCount */
#define DEF_GBL_VAL             0x80000000u
#define DEF_CTL_VAL             0x00008048u

#undef  PMON_DECL
#define PMON_DECL(x)    x,

typedef enum PMetric {
    PMON_DECL(DISABLED)
#if defined(PMON_RAC_METRIC)
    PMON_DECL(RACLookup)
    PMON_DECL(RACHits)
    PMON_DECL(Prefetch)
#endif
    PMON_DECL(DCacheHits)
    PMON_DECL(DCacheMiss)
    PMON_DECL(ICacheHits)
    PMON_DECL(ICacheMiss)
    PMON_DECL(InstrRate)
    PMON_DECL(CyclesCount)     /* CyclesCount must be the last */
    PMON_MAX_METRIC
} PMetric_t;

#undef  PMON_DECL
#define PMON_DECL(x)    #x

const char * PMetric_Str[] = {
    PMON_DECL(DISABLED)
#if defined(PMON_RAC_METRIC)
    PMON_DECL(RACLookup)
    PMON_DECL(RACHits)
    PMON_DECL(Prefetch)
#endif
    PMON_DECL(DCacheHits)
    PMON_DECL(DCacheMiss)
    PMON_DECL(ICacheHits)
    PMON_DECL(ICacheMiss)
    PMON_DECL(InstrRate)
    PMON_DECL(CyclesCount)
    "INVALID"
};

PMetricConfig_t pMetricConfig[] =
{
    { DEF_GBL_VAL, DEF_CTL_VAL },   /* EventType     MOD SET       EventId,00 */

#if defined(PMON_RAC_METRIC)
    { 0x8000002Cu, 0x00008108u },   /* RAC Lookup 2C=1011 00   108=1000010,00 */
    { 0x8000002Eu, 0x00008114u },   /* RAC Hits   2E=1011 10   114=1000101,00 */
    { 0x8000002Du, 0x0000812Cu },   /* Prefetch   2D=1011 01   12C=1001011,00 */
#endif
     
    { 0x80000011u, 0x00008028u },   /* DCacheHit  11=0100 01   028=0001010,00 */
    { 0x80000011u, 0x00008024u },   /* DCacheMiss 11=0100 01   024=0001001,00 */

    { 0x80000018u, 0x00008018u },   /* ICacheHit  18=0110 00   018=0000110,00 */
    { 0x80000018u, 0x00008014u },   /* ICacheMiss 18=0110 00   014=0000101,00 */

    { 0x80000000u, 0x00008044u },   /* Instructn  00=xxxx xx   044=0010001,00 */
    { 0x80000000u, 0x00008048u },   /* CycleCount 00=xxxx xx   048=0010010,00 */

    { DEF_GBL_VAL, DEF_CTL_VAL }
};

uint32_t    iteration   = 0;
uint32_t    sample_size = 0;
PMetric_t   metric      = DISABLED;
uint32_t    skip_enable = 0;
uint32_t    running     = 0;
uint32_t    report_all  = 0;
uint32_t    cycles_per_usec = 400;

uint32_t pfCtr[ PMON_OVFLW_EVENTS ]; /* Current sample values */
uint32_t pfSum[ PMON_MAX_EVENTS ];   /* Accumulate sum of sample iterations */
uint32_t pfTmp[ PMON_MAX_EVENTS ];   /* MIPS workaround for CP0 counters not
                                        getting updated correctly. */

uint32_t pfMon[ PMON_MAX_METRIC + 1 ][ PMON_MAX_EVENTS ];
uint32_t pfTot[ PMON_MAX_METRIC ];

typedef char PmonEventName_t[128];
PmonEventName_t pfEvt[ PMON_MAX_EVENTS ];

void pmon_reg(uint32_t event, char * eventName)
{
    if ( event < PMON_MAX_EVENTS )
    {
        strncpy( pfEvt[event], eventName, 127 );

        if ( strcmp( pfEvt[event], PMON_DEF_UNREGEVT ) )
            printk( _G_ "PMON Registering event %u : %s" _NL_,
                    event, pfEvt[event] );
    }
}

#if defined( PMON_UT )

int pmon_loops = 0;

void pmon_ut(void)
{
    int iter;

        /* Each iteration from bgn to end is a sample */
    for (iter=0; iter<pmon_loops; iter++)
    {
        pmon_bgn();
        udelay(1); pmon_log(1);
        udelay(2); pmon_log(2);
        udelay(3); pmon_log(3);
        udelay(4); pmon_log(4);
        if ( ( iter % 10 ) == 0 )
            continue;                   /* fake an incomplete iteration */
        udelay(5); pmon_log(5);
        udelay(6); pmon_log(6);
        if ( ( iter % 100 ) == 0 )      /* fake a skipped complete iteration */
        {
            pmon_clr();
            goto skip;
        }
        udelay(7); pmon_log(7);
        udelay(8); pmon_log(8);
skip:
        udelay(9); pmon_log(9);
        pmon_end(9);                    /* iteration complete */

        PMONDBG( printk("pmon_ut next iter<%u>\n", iter ); );
    }

    printk( _G_ "Done %u loops in pmon_ut()" _NL_, pmon_loops );

    __write_pfgblctl(DEF_GBL_VAL);  /* Enable global PCE */
    __write_pfctl_0 (DEF_CTL_VAL);  /* Enable cycle count in CTR0 */

}
#endif  /* defined( PMON_UT ) */


/*
 *-----------------------------------------------------------------------------
 * Function: pmon_report
 * Report all collected metrics measurements.
 *-----------------------------------------------------------------------------
 */
int pmon_report(void)
{
    uint32_t evt, cycles;
    PMetric_t metric;

    if ( running )
        printk( _R_ "WARNING: PMON is still running" _NL_ );

    printk( " Poor Man's Performance Monitor\n"
            "\trunning: %s\n"
            "\tcaliberation cycles_per_usec: %u\n",
            running ? "TRUE" : "FALSE",
            cycles_per_usec );

    memset( pfTot, 0, sizeof(pfTot) );

    if ( report_all )
        printk( _H_ "Evt:  Cycles-Count Nano-secs Instruction "
                    "I-C_Hits I-C_Miss D-C_Hits D-C_Miss"
#if defined(PMON_RAC_METRIC)
                    " Prefetch RAC_Hits RAC_Look"
#endif
                    " : EventName" _NL_ );
    else
        printk( _H_ "Evt:  Cycles-Count Nano-secs : EventName" _NL_ );

    for ( evt=1; evt<PMON_MAX_EVENTS; evt++ )
    {
        if ( pfMon[CyclesCount][evt] == 0 ) break;

        cycles = pfMon[CyclesCount][evt]/sample_size;

        pfTot[CyclesCount] += cycles;

        if ( report_all )
        {
            for (metric=DISABLED+1; metric < CyclesCount; metric++)
                pfTot[metric] += pfMon[metric][evt]/sample_size;

            printk( "%3u:  %12u %9u %11u %8u %8u %8u %8u"
#if defined(PMON_RAC_METRIC)
                    " %8u %8u %8u"
#endif
                    " : %s\n",
                    evt, cycles,
                    (cycles * 1000) / cycles_per_usec,
                    pfMon[InstrRate][evt]/sample_size,
                    pfMon[ICacheHits][evt]/sample_size,
                    pfMon[ICacheMiss][evt]/sample_size,
                    pfMon[DCacheHits][evt]/sample_size,
                    pfMon[DCacheMiss][evt]/sample_size,
#if defined(PMON_RAC_METRIC)
                    pfMon[Prefetch][evt]/sample_size,
                    pfMon[RACHits][evt]/sample_size,
                    pfMon[RACLookup][evt]/sample_size,
#endif
                    pfEvt[evt] );
        }
        else
            printk( "%3u:  %12u %9u : %s\n",
                    evt, cycles, (cycles * 1000) / cycles_per_usec, pfEvt[evt]);
    }

    if ( report_all )
        printk( _B_ "\nTot:  %12u %9u %11u %8u %8u %8u %8u"
#if defined(PMON_RAC_METRIC)
                " %8u %8u %8u"
#endif
                "\n" _NL_,
                pfTot[CyclesCount],
                (pfTot[CyclesCount] * 1000) / cycles_per_usec,
                pfTot[InstrRate],
                pfTot[ICacheHits],
                pfTot[ICacheMiss],
                pfTot[DCacheHits],
                pfTot[DCacheMiss]
#if defined(PMON_RAC_METRIC)
                , pfTot[Prefetch],
                pfTot[RACHits],
                pfTot[RACLookup]
#endif
                );

    else
        printk( _B_ "\nTot:  %12u %9u\n" _NL_,
                pfTot[CyclesCount],
                (pfTot[CyclesCount] * 1000) / cycles_per_usec );

    for ( evt=PMON_MAX_EVENTS; evt<PMON_OVFLW_EVENTS; evt++ )
        if ( pfCtr[evt] != 0 )
            printk( _R_ "WARNING: %u event not reported" _NL_, evt );

    return 0;
}

/*
 *-----------------------------------------------------------------------------
 * Function: pmon_bind
 * Bind the Performance Monitoring hardware module for counting in Counter 0.
 *-----------------------------------------------------------------------------
 */
void pmon_bind(PMetric_t metric)
{
    PMetricConfig_t * pmon_cfg_p;

    PMONDBG( printk("bind() metric<%u:%s>\n", metric, PMetric_Str[metric]); );

    /* Apply GBL and CNTL configuration for metric */
    pmon_cfg_p = &pMetricConfig[metric];

    /* PS: For DISABLED metric, default is Cycle count */
    __write_pfgblctl( pmon_cfg_p->perfGbl );/* Configure Global MOD SET */
    __write_pfctl_0(  pmon_cfg_p->perfCtl );/* Configure Control Select CTR0|1*/
}

/*
 *-----------------------------------------------------------------------------
 * Function: pmon_enable
 * Apply skip, sample and metric configuration.
 *-----------------------------------------------------------------------------
 */
int pmon_enable(uint32_t skip, uint32_t samples, uint32_t all_metric)
{
    printk(_G_ "PMON: skip<%u> samples<%u> all_metric<%u>" _NL_,
            skip, samples, all_metric );

    memset( pfCtr, 0, sizeof(pfCtr) );
    memset( pfSum, 0, sizeof(pfSum) );
    memset( pfMon, 0, sizeof(pfMon) );

    sample_size = samples;
    iteration = 0;  /* current iteration */
    running = 0;

    report_all = all_metric;
    metric = ( report_all == 0 ) ? CyclesCount : (DISABLED+1);
    skip_enable = (skip == 0) ? 1 : skip;

#ifdef PMON_UT
    pmon_loops = (sample_size + skip) * 2; 
    if ( all_metric ) pmon_loops *= 10;
    pmon_ut();
#endif

    return 0;
}

/*
 *-----------------------------------------------------------------------------
 * Function: pmon_bgn
 * Start a new iteration. Ignore the first few iterations defined by skip.
 * If pmon_bgn is invoked before pmon_end, then current iteration overwritten
 *-----------------------------------------------------------------------------
 */
void pmon_bgn(void)
{
#ifdef PMON_UT
    PMONDBG( printk("pmon_bgn() skip_enable<%u>\n", skip_enable ); );
#endif

    if ( unlikely(skip_enable) )    /* iterations to skip */
    {
        skip_enable--;
        if ( skip_enable == 0 )
        {
            pmon_bind( metric );    /* setup pmon_fn pointer */
            running = 1;            /* tracing begins now */
            pmon_log(0);            /* record event0 : baseline value */
        }
        else
            pmon_clr();
    }
    else
        pmon_log(0);                /* record event0 : baseline value */
}

/*
 *-----------------------------------------------------------------------------
 * Function: pmon_end
 * Function to be invoked to demarcate the end of an iteration.
 *
 * Accumulates the elapsed count into an accumulator per event. Assumes that
 * events will be invoked in order of event id. Each time pmon_end() is invoked
 * an iteration completes, implying a sample is taken. Upon completion of the
 * sample size of iterations, measurements for the next metric will commence.
 *
 *-----------------------------------------------------------------------------
 */
void pmon_end(uint32_t max_event)
{
    uint32_t evt, elapsed_count, previous_count, current_count;

    if ( ! running )
        return;

    if ( pfCtr[0] == ~0U )
        return;

    PMONDBG( printk("pmon_end iteration<%u> metric<%u,%s>\n",
             iteration, metric, PMetric_Str[metric]); );

    /* Accumulate elapsed counter values per event */
#if defined(USE_CP0_CYCLES_COUNT)
    if ( metric == CyclesCount )    /* cycles count up */
    {
        for ( evt=1; evt<=max_event; evt++ )
        {
            previous_count = pfCtr[evt-1];
            current_count  = pfCtr[evt];

            if ( current_count < previous_count )   /* rollover */
                elapsed_count = current_count + (~0U - previous_count);
            else
                elapsed_count = (current_count - previous_count);

            if ( elapsed_count > 0xFFFF0000 )
                return;     /* exclude this entire iteration */
            else
                pfTmp[evt] = elapsed_count;
        }
    }
    else        /* Performance counters count down */
#else /* Performance counters : count down */
    /* WORKAROUND : */
    for ( evt=1; evt<=max_event; evt++ )
    {
        previous_count = pfCtr[evt-1];
        current_count  = pfCtr[evt];

        if ( current_count > previous_count )    /* rollover */
            elapsed_count = previous_count + (~0U - current_count);
        else
            elapsed_count = previous_count - current_count;

        if ( elapsed_count > 0xFFFF0000 )
            return;     /* exclude this entire iteration */
        else
            pfTmp[evt] = elapsed_count;
    }
#endif

    for ( evt=1; evt<=max_event; evt++ )
        pfSum[evt] += pfTmp[evt];   /* Accumulate into average */


    iteration++;    /* Completed the collection of one sample */


    /* 
     * Record accumulated into pfMon, if sample size number of iterations
     * have been collected, and switch to next metric measurement.
     */
    if ( iteration >= sample_size )
    {
        pfMon[metric][0] = ~0U;     /* Tag measurement as valid */
        for ( evt=1; evt<PMON_MAX_EVENTS; evt++ )
            pfMon[metric][evt] = pfSum[evt];

        metric++;                   /* Fetch next metric */
        iteration = 0;              /* First iteration of next metric */
        memset( pfSum, 0, sizeof(pfSum) );  /* Clear accumulator */

        if ( metric >= PMON_MAX_METRIC)     /* Completed all metrics ? */
        {
            running = 0;            
            metric = DISABLED;
            pmon_report();
        }

        pmon_bind( metric );        /* Configure next metric (or disable) */
    }
}

/*
 *-----------------------------------------------------------------------------
 * Rudimentary pmon ctl char device ioctl handling of a single word argument
 *-----------------------------------------------------------------------------
 */
static int pmon_open(struct inode *inode, struct file *filp) { return 0; }
static int pmon_rel(struct inode *inode, struct file *file)  { return 0; }

static DEFINE_MUTEX(ioctlMutex);

static long pmon_ioctl_unlocked(  struct file *  file, 
                                  unsigned int   cmd,
                                  unsigned long  arg)
{
    long ret = -ENOTTY;
    mutex_lock(&ioctlMutex);
    switch ( cmd )
    {
        case PMON_CPU_START_IOCTL:
        case PMON_ALL_START_IOCTL:
             ret = pmon_enable( (arg >>16), (arg & 0xFFFF), cmd );
        case PMON_REPORT_IOCTL:
             ret = pmon_report();
    }
    mutex_unlock(&ioctlMutex);
    return ret;
}

static struct file_operations pmon_fops =
{
    .unlocked_ioctl  = pmon_ioctl_unlocked,
    .open   = pmon_open,
    .release = pmon_rel,
    .owner  = THIS_MODULE
};

/*
 *-----------------------------------------------------------------------------
 * Function: pmon_init
 * Upon module loading, the characted device pmon is registered and the
 * performance counter module is enabled for counting cycles in Counter 0.
 *-----------------------------------------------------------------------------
 */
int __init pmon_init(void)
{
    uint32_t evt, elapsed_count, previous_count, current_count;

    if ( register_chrdev(PMON_DEV_MAJ, PMON_DEV_NAME, &pmon_fops) )
    {
        printk( _R_ "Unable to get major number <%d>" _NL_, PMON_DEV_MAJ);
        return -1;
    }

    for (evt=0; evt < PMON_MAX_EVENTS; evt++)
        pmon_reg(evt, PMON_DEF_UNREGEVT);

    __write_pfgblctl(DEF_GBL_VAL);  /* Enable global PCE */
    __write_pfctl_0 (DEF_CTL_VAL);  /* Enable cycle count in CTR0 */

    /* Estimate PMON cycles to microseconds */
    previous_count = __read_pfctr_0();
    udelay(1000);
    current_count = __read_pfctr_0();

    if ( current_count > previous_count )   /* rollover */
        elapsed_count = previous_count + (~0U - current_count);
    else
        elapsed_count = previous_count - current_count;
    cycles_per_usec = elapsed_count / 1000;

    printk( _H_ "PMON Char Driver Registered<%d> cycles_per_usec<%u>" _NL_,
            PMON_DEV_MAJ, cycles_per_usec );

    return PMON_DEV_MAJ;
}

module_init(pmon_init);

EXPORT_SYMBOL(pfCtr);
EXPORT_SYMBOL(pmon_log);
EXPORT_SYMBOL(pmon_bgn);
EXPORT_SYMBOL(pmon_end);
EXPORT_SYMBOL(pmon_reg);
EXPORT_SYMBOL(pmon_enable);

#endif  /* defined( CONFIG_PMON ) */
#endif
