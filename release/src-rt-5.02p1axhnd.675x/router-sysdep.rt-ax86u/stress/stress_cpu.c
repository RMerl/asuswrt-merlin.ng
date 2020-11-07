/***********************************************************************
 *
 *  Copyright (c) 2010  Broadcom Corporation
 *  All Rights Reserved
 *
<:label-BRCM:2010:proprietary:standard

 This program is the proprietary software of Broadcom and/or its
 licensors, and may only be used, duplicated, modified or distributed pursuant
 to the terms and conditions of a separate, written license agreement executed
 between you and Broadcom (an "Authorized License").  Except as set forth in
 an Authorized License, Broadcom grants no license (express or implied), right
 to use, or waiver of any kind with respect to the Software, and Broadcom
 expressly reserves all rights in and to the Software and all intellectual
 property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
 NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
 BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.

 Except as expressly set forth in the Authorized License,

 1. This program, including its structure, sequence and organization,
    constitutes the valuable trade secrets of Broadcom, and you shall use
    all reasonable efforts to protect the confidentiality thereof, and to
    use this information only in connection with your use of Broadcom
    integrated circuit products.

 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
    RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
    ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
    FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
    COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
    TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
    PERFORMANCE OF THE SOFTWARE.

 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
    ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
    INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
    WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
    IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
    OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
    SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
    SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
    LIMITED REMEDY.
:>
 *
 ************************************************************************/

#define _GNU_SOURCE
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <time.h>
#include <sys/time.h>
#include <sys/times.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>

struct worker_data {
    char name[64];
    int pid;
    pthread_t pthread_id;
    unsigned long work_units_done;
    unsigned long cpu_used_ms;
};

void init_ticks();
unsigned long get_cpu_used();
void *worker(void *d);
void dump_worker_stats(const struct worker_data *wd);
void do_work_unit();
void do_sleep_ms(unsigned long sleep_ms, int cycle_count);
unsigned long delta_in_ms(const struct timeval *end, const struct timeval *start);
void set_sched_prio();
void set_cpu_binding();


static int bind_to_cpu=-1;
static int sched_algo=0;
static int prio=0;
static int duty_cycle=1000;  // period which the percentage is measured, in ms
static int pcpu=100;         // percentage of CPU to try to consume
static unsigned long target_work_in_cycle;
static unsigned long target_sleep_in_cycle;
static int run_time=60;      // run time in seconds
static int do_help=0;
static int debug=0;
static int ms_per_tick=0;
static int num_forks=0;
static int num_pthreads=0;
static int num_workers=1;

void parse_cpu_major_mode(int argc, char **argv)
{
    int c;
    int option_index=0;

    printf("parent: entered cpu major mode:\n");
    while (1)
    {
        static struct option long_options[] =
        {
            {"help", no_argument, &do_help, 1},
            {"debug", no_argument, &debug, 1},
            {"bind_to_cpu", required_argument, NULL, 'b'},
            {"sched_fifo", required_argument, NULL, 'f'},
            {"sched_rr", required_argument, NULL, 'r'},
            {"sched_other", required_argument, NULL, 'o'},
            {"duty_cycle", required_argument, NULL, 'c'},
            {"pcpu", required_argument, NULL, 'p'},
            {"run_time", required_argument, NULL, 't'},
            {"forks", required_argument, NULL, 'k'},
            {"pthreads", required_argument, NULL, 'e'}
        };

        c = getopt_long(argc, argv, "hdb:f:r:o:c:p:t:k:e:", long_options, &option_index);

        if (c == -1)
            break;

        switch (c)
        {
        case 'd':
            printf("setting debug\n");
            debug=1;
            break;

        case 'h':
            printf("setting do_help\n");
            do_help=1;
            break;

        case 'b':
            bind_to_cpu = atoi(optarg);
            break;

        case 'f':
            sched_algo = SCHED_FIFO;
            prio = atoi(optarg);
            break;

        case 'r':
           sched_algo = SCHED_RR;
           prio = atoi(optarg);
           break;

        case 'o':
           sched_algo = SCHED_OTHER;
           prio = atoi(optarg);
           break;

        case 'c':
            duty_cycle = atoi(optarg);
            break;

        case 'p':
            pcpu = atoi(optarg);
            break;

        case 't':
            run_time = atoi(optarg);
            break;

        case 'k':
            num_forks = atoi(optarg);
            num_workers = num_forks;
            break;

        case 'e':
            num_pthreads = atoi(optarg);
            num_workers = num_pthreads;
            break;

        default:
            printf("option %s was set by value in longopt, val=%d\n",
                   long_options[option_index].name, c);
            break;
        }
    }

    if (do_help)
    {
        printf("   -h: print usage and exit\n");
        printf("   -d: enable debug messages\n");
        printf("   --bind_to_cpu (-b): bind thread to a particular CPU, values are 0 ~ 3, -1 means no binding\n");
        printf("   --sched_fifo (-f) : run in FIFO real-time scheduling class, at specified priority [1-99]\n");
        printf("   --sched_rr   (-r) : run in Round Robin real-time scheduling class, at specified priority [1-99]\n");
        printf("   --sched_other (-o): run in normal scheduling class, only priority 0 is allowed\n");
        printf("   --duty_cycle  (-c): number of milliseconds in measurement cycle [100-1000]\n");
        printf("   --pcpu       (-p) : percentage of the CPU to use in measurement cycle [1-100]\n");
        printf("   --run_time   (-t) : amount of (wall clock) time to run in seconds\n");
        printf("   --forks      (-k) : fork specified number of processes to do work\n");
        printf("   --threads    (-e) : spawn specified number of pthreads to do work\n");
        exit(0);
    }

    init_ticks();

    target_work_in_cycle = (unsigned long) (duty_cycle * pcpu / 100);
    target_sleep_in_cycle = ((unsigned long) duty_cycle) - target_work_in_cycle;

    printf("debug=%d\n", debug);
    printf("bind_to_cpu=%d\n", bind_to_cpu);
    if (bind_to_cpu < -1 || bind_to_cpu > 4) {
        printf("----error: bind_to_cpu must be 0 ~ 3\n");
        exit(-1);
    }
    printf("sched_algo(0=o, 1=FIFO, 2=RR)/priority: %d/%d\n", sched_algo, prio);
    printf("duty_cycle_in_ms=%d\n", duty_cycle);
    if (duty_cycle < 100 || duty_cycle > 1000)
        printf("---- recommend duty cycle between 100 to 1000ms\n");
    printf("percent cpu=%d\n", pcpu);
    if (pcpu <= 0 || pcpu > 100) {
        printf("----error: pcpu must be between 1 and 100\n");
        exit(-1);
    }

    printf("targets per duty cycle: work=%lu ms, sleep=%lu ms\n",
               target_work_in_cycle, target_sleep_in_cycle);
    printf("run_time=%d secs\n", run_time);
    printf("ms_per_clock_tick=%d\n", ms_per_tick);
    if (num_forks > 0)
        printf("Creating %d forked processes to do work\n", num_forks);
    else if (num_pthreads > 0)
        printf("Creating %d pthreads to do work\n", num_pthreads);
    else
        printf("Single threaded mode\n");
}


int run_cpu_major_mode(void)
{
    struct worker_data *worker_array;
    int i=0;

    worker_array = (struct worker_data *) malloc(num_workers * sizeof(struct worker_data));
    if (worker_array == NULL) {
        printf("Could not allocate %d worker_data's\n", num_workers);
        exit(-1);
    }

    /* CPU binding and priority are inherited by forks and pthreads. */
    /* Work around a bug(?) in the kernel by setting the cpu binding before
     * the RT priority.  Otherwise, rt_nr_migratory and nr_cpus_allowed
     * becomes inconsistent. */
    set_cpu_binding();
    set_sched_prio();

    memset(worker_array, 0, num_workers * sizeof(struct worker_data));

    /* create the workers */
    if (num_forks > 0) {
        for (i=0; i < num_forks; i++) {
            sprintf(worker_array[i].name, "fork%d", i);
            worker_array[i].pid = fork();
            if (worker_array[i].pid == 0) {
                worker(&worker_array[i]);
                dump_worker_stats(&worker_array[i]);
                exit(0);
            }
        }
    }
    else if (num_pthreads > 0) {
        for (i=0; i < num_pthreads; i++) {
            sprintf(worker_array[i].name, "pthread%d", i);
            pthread_create(&worker_array[i].pthread_id, NULL, worker,
                           (void *) &worker_array[i]);
        }
    }
    else {
        //sprintf(worker_array[0].name, "");
        worker(&worker_array[0]);
    }

    /* now collect all the workers */
    if (num_forks > 0) {
        for (i=0; i < num_forks; i++) {
            int childstat=0;
            waitpid(worker_array[i].pid, &childstat, 0);
        }
    }
    else if (num_pthreads > 0) {
        void *rv=NULL;
        for (i=0; i < num_pthreads; i++) {
            pthread_join(worker_array[i].pthread_id, &rv);
            dump_worker_stats(&worker_array[i]);
        }
    }
    else {
        dump_worker_stats(&worker_array[0]);
    }

    return 0;
}


void *worker(void *d)
{
    struct worker_data *wd= (struct worker_data *)d;
    struct timeval run_start_tv, run_end_tv;
    unsigned long run_start_cpu, run_end_cpu;
    unsigned long run_time_ms=0;
    unsigned long work_time_ms=0;
    int cycle_count=0;

    wd->pid = getpid();
    printf("=====[%s] (pid=%d) starting test=====\n\n", wd->name, wd->pid);
    fflush(stdout);

    gettimeofday(&run_start_tv, NULL);
    run_start_cpu = get_cpu_used();

    do {
        unsigned long work_start_cpu, work_end_cpu;
        struct timeval work_start_tv, work_end_tv;

start_work_cycle:

        cycle_count++;
        gettimeofday(&work_start_tv, NULL);
        work_start_cpu = get_cpu_used();
        do {
            do_work_unit();
            wd->work_units_done++;
            work_end_cpu = get_cpu_used();

            /*
             * On a heavily loaded system, this thread will be much delayed.
             * Check if our run time is already up at this point.
             * Also check if the cycle time is up.  In that case, I guess
             * just start next cycle.
             */
            gettimeofday(&run_end_tv, NULL);
            run_time_ms = delta_in_ms(&run_end_tv, &run_start_tv);
            if (run_time_ms >= (unsigned long) run_time*1000)
                break;

            gettimeofday(&work_end_tv, NULL);
            work_time_ms = delta_in_ms(&work_end_tv, &work_start_tv);
            if (work_time_ms > (unsigned long) duty_cycle)
                goto start_work_cycle;

        } while ((work_end_cpu - work_start_cpu) < target_work_in_cycle);

        /*
         * On heavily loaded system, by the time we worked the target amount,
         * the elapsed wall time may be greater than work time.  So sleep less.
         */
        if (debug)
            printf("[%d] work_cpu=%lu work_time_ms=%lu target_work_in_cycle=%lu\n",
                    cycle_count, (work_end_cpu - work_start_cpu),
                    work_time_ms, target_work_in_cycle);
        if (work_time_ms > target_work_in_cycle) {
            if (work_time_ms - target_work_in_cycle < target_sleep_in_cycle) {
                do_sleep_ms(target_sleep_in_cycle -
                             (work_time_ms - target_work_in_cycle),
                             cycle_count);
            }
        }
        else
            do_sleep_ms(target_sleep_in_cycle, cycle_count);

        gettimeofday(&run_end_tv, NULL);
        run_time_ms = delta_in_ms(&run_end_tv, &run_start_tv);
    } while (run_time_ms < (unsigned long) run_time*1000);

    run_end_cpu = get_cpu_used();

    wd->cpu_used_ms = run_end_cpu-run_start_cpu;

    return NULL;
}



/**************************************************************************
 *
 * Utility functions
 *
 **************************************************************************/

#ifndef USEC_IN_SEC
#define USEC_IN_SEC 1000000
#endif

#ifndef NANOSEC_IN_MSEC
#define NANOSEC_IN_MSEC 1000000
#endif

unsigned long delta_in_ms(const struct timeval *end, const struct timeval *start)
{
    unsigned long delta_in_us;

    // if (debug) {
    //    printf("start %lu.%lu\n", start->tv_sec, start->tv_usec);
    //    printf("end   %lu.%lu\n", end->tv_sec, end->tv_usec);
    //}

    if (start->tv_usec > end->tv_usec) {
        delta_in_us = (USEC_IN_SEC + end->tv_usec - start->tv_usec) +
                      ((end->tv_sec - 1 - start->tv_sec) * USEC_IN_SEC);
    }
    else {
        delta_in_us = (end->tv_usec - start->tv_usec) +
                      ((end->tv_sec - start->tv_sec) * USEC_IN_SEC);
    }

    return delta_in_us/1000;
}

void init_ticks()
{
    int hz;

    /* this reports "USER_HZ" which is always 100, rather than "HZ", which
     * on our system is 1000.
     */
    hz = sysconf(_SC_CLK_TCK);
    printf("USER_HZ=%d\n", hz);
    ms_per_tick = 1000/hz;

    return;
}

/** return the amount of CPU time consumed by this thread (both user and sys)
 * in ticks, which I assume is 1ms.
 */
unsigned long get_cpu_used()
{
    struct tms tms;
    int rv;

    rv = times(&tms);
    if (rv < 0) {
        printf("times system call failed\n");
        exit(-1);
    }

    return ms_per_tick * (tms.tms_utime + tms.tms_stime);
#ifdef USE_PROC
    /* /proc/self/stat still reports in resolution of USER_HZ
     * which is 10ms, even though internally, the modem uses HZ=1000.
     * But if CONFIG_SCHEDSTATS, there is a /proc/pid/schedstat which reports
     * in 1ms resolution.
     */

    char buf[1024]={0};
    char *filename="/proc/self/stat";
    char *cp;
    int fd, rv;
    char state;
    unsigned int ppid, pgid, sid;
    int tty;
    unsigned long utime, stime;

    fd = open(filename, O_RDONLY, 0);
    if (fd < 0) {
        printf("Could not open %s\n", filename);
        exit(0);
    }
    rv = read(fd, buf, sizeof(buf));
    close(fd);

    cp = strrchr(buf, ')');
    if (cp == NULL) {
        printf("Could not find char in %s\n", buf);
        exit(0);
    }

    sscanf(cp+2,
            "%c %u "
            "%u %u %d %*s"
            "%*s %*s %*s %*s %*s " /* flags, min_flt, cmin_flt, maj_flt, cmaj_flt */
            "%lu %lu "             /* utime, stime */
            ,
            &state, &ppid,
            &pgid, &sid, &tty,
            &utime, &stime
         );

    if (debug) {
        printf("buf=%s\n", buf);
        printf("utime %lu + stime %lu = %lu\n", utime, stime, utime+stime);
    }

    return utime+stime;
#endif
}


void do_work_unit()
{
    int i;
    int chunk=50;
    struct timeval tv;

    for (i=0; i < chunk; i++)
        gettimeofday(&tv, NULL);

    return;
}

void do_sleep_ms(unsigned long sleep_ms, int cycle_count)
{
    struct timespec ts;

    if (debug)
        printf("[%d] sleeping for %lu ms\n", cycle_count, sleep_ms);

    ts.tv_sec = sleep_ms / 1000;
    ts.tv_nsec = (sleep_ms % 1000) * NANOSEC_IN_MSEC;
    nanosleep(&ts, NULL);
}

void dump_worker_stats(const struct worker_data *wd)
{
    printf("===== [%s] (pid=%d) test complete=====\n", wd->name, wd->pid);
    printf("Work units done=%lu\n", wd->work_units_done);
    printf("CPU used=%lu ms out of %dms (%lu%%)\n",
            wd->cpu_used_ms, 1000*run_time,
            ((wd->cpu_used_ms)*100)/(run_time * 1000));
    printf("===================================\n\n");
}

void set_sched_prio()
{
    int rc;
    struct sched_param sched = {0};
    sched.__sched_priority = prio;
              // printf("thread %d: setting rt fifo\n", nthread);
    rc = sched_setscheduler(0, sched_algo, &sched);
    if (rc < 0) {
        printf("sched_setscheduler failed, rc=%d (algo=%d prio=%d)\n",
                rc, sched_algo, prio);
        exit(-1);
    }
}

void set_cpu_binding()
{
    int rc;
    cpu_set_t mask;

    if (bind_to_cpu == -1)
        return;

    CPU_ZERO(&mask);
    CPU_SET(bind_to_cpu, &mask);

    rc = sched_setaffinity(0, sizeof(cpu_set_t), &mask);
    if (rc < 0) {
        printf("sched_setaffinity failed, rc=%d\n", rc);
        exit(-1);
    }
}
