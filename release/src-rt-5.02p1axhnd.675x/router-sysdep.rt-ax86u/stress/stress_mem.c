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
#include <pthread.h>
#include <stdint.h>
#include <inttypes.h>


typedef struct thread_info {
    int           thread_num;
    pthread_t     tid;
    unsigned int  num_allocs;
    unsigned int  num_frees;
    int           max_alloc_size;
} thread_info_t;

typedef struct mem_info {
    int in_use;
    unsigned int pattern;
    int num_words;
    unsigned int *buf;
} mem_info_t;


#define LOCKED_INC(m, v) do {                           \
                               pthread_mutex_lock(m);   \
                               v++;                     \
                               pthread_mutex_unlock(m); \
                            } while (0);

/*
 * Forward declarations
 */
static void *mem_thread(void *arg);
static void fill_slot(mem_info_t *slot);
static int verify_slot(const mem_info_t *slot);
static void free_slot(mem_info_t *slot);
static void *cond_lock_calloc(pthread_mutex_t *m, int s, int n);
static void *cond_lock_malloc(pthread_mutex_t *m, int s);
static void cond_lock_free(pthread_mutex_t *m, void *p);


int debug=0;
int num_threads=1;
int num_slots=200;
int num_loops=5000;
int bind_to_cpu=-1;
int do_mutex=0;
int do_help=0;
unsigned int g_total_mallocs;
unsigned int g_total_frees;
pthread_mutex_t mem_mutex;
pthread_mutex_t malloc_mutex;
pthread_mutex_t free_mutex;


void parse_mem_major_mode(int argc, char **argv)
{
    int c;
    int option_index=0;

    printf("parent: entered mem major mode:\n");
    while (1)
    {
        static struct option long_options[] =
        {
            {"debug", no_argument, &debug, 1},
            {"lock_mem", no_argument, &do_mutex, 1},
            {"help", no_argument, &do_help, 1},
            {"bind_to_cpu", required_argument, NULL, 'b'},
            {"num_threads", required_argument, NULL, 't'},
            {"num_slots", required_argument, NULL, 's'},
            {"num_loops", required_argument, NULL, 'l'}
        };

        c = getopt_long(argc, argv, "dmht:s:l:", long_options, &option_index);

        if (c == -1)
            break;

        switch (c)
        {
        case 0:
            printf("option %s automatically set\n", long_options[option_index].name);
            break;

        case 'd':
            printf("setting debug\n");
            debug=1;
            break;

        case 'm':
            printf("setting do_mutex\n");
            do_mutex=1;
            break;

        case 'h':
            printf("setting do_help\n");
            do_help=1;
            break;

        case 'b':
            printf("bind_to_cpu\n");
            bind_to_cpu = atoi(optarg);
            break;

        case 't':
            printf("setting num_threads\n");
            num_threads = atoi(optarg);
            break;

        case 's':
            printf("setting num_slots\n");
            num_slots = atoi(optarg);
            break;

        case 'l':
            printf("setting num_loops\n");
            num_loops = atoi(optarg);
            break;
        }
    }

    if (do_help)
    {
        printf("   --num_threads (-t): number of threads, default %d\n", num_threads);
        printf("   --num_slots (-s)  : number of memory slots, default %d\n", num_slots);
        printf("   --num_loops (-l)  : number of loops, default %d\n", num_loops);
        printf("   --lock_mem (-m)   : use lock to protect malloc and free, should not be needed, default %d\n", do_mutex);
        printf("   --bind_to_cpu (-b): bind all threads to the specified cpu, by default -1, which means no binding\n");
        exit(0);
    }

    printf("debug=%d\n", debug);
    printf("num_threads=%d\n", num_threads);
    printf("num_slots=%d (4-16372 bytes per slot, max mem usage=%dMB)\n",
            num_slots, (num_slots * 16)/1024);
    printf("num_loops=%d\n", num_loops);
    printf("do_mutex=%d\n", do_mutex);
    printf("bind_to_cpu=%d\n", bind_to_cpu);
}


int run_mem_major_mode(void)
{
    int i, rv=0;
    thread_info_t *tinfo_array;
    unsigned int total_mallocs=0;
    unsigned int total_frees=0;
    printf("parent: Starting mem test mode\n");
    printf("parent:    RAND_MAX=%d\n", RAND_MAX);


    pthread_mutex_init(&mem_mutex, NULL);
    pthread_mutex_init(&malloc_mutex, NULL);
    pthread_mutex_init(&free_mutex, NULL);

    tinfo_array = cond_lock_calloc(&mem_mutex, sizeof(thread_info_t), num_threads);

    for (i=0; i < num_threads; i++)
    {
        tinfo_array[i].thread_num = i+1;
        printf("parent: Starting thread %d\n", tinfo_array[i].thread_num);
        pthread_create(&tinfo_array[i].tid, NULL,
                       mem_thread, (void *) &tinfo_array[i]);
    }

    printf("parent: waiting for threads to finish\n");
    for (i=0; i < num_threads; i++)
    {
        int rc;
        void *result=NULL;
        rc = pthread_join(tinfo_array[i].tid, &result);
        if (rc != 0 || result != NULL)
        {
            printf("Error on join of thread %d, rc=%d result=%p\n",
                   i, rc, result);
            rv = 1;
        }
        else
        {
            printf("parent: collected thread %d\n", tinfo_array[i].thread_num);
            total_mallocs += tinfo_array[i].num_allocs;
            total_frees += tinfo_array[i].num_frees;
        }
    }

    printf("parent: All threads collected.\n");

    if (total_mallocs != g_total_mallocs)
    {
        printf("MISMATCH: g_total_mallocs=%d total_mallocs=%d\n",
                g_total_mallocs, total_mallocs);
        rv = 1;
    }

    if (total_frees != g_total_frees)
        {
            printf("MISMATCH: g_total_frees=%d total_frees=%d\n",
                    g_total_frees, total_frees);
            rv = 1;
        }

    pthread_mutex_destroy(&mem_mutex);
    pthread_mutex_destroy(&malloc_mutex);
    pthread_mutex_destroy(&free_mutex);

    return rv;
}


void *mem_thread(void *arg)
{
    int i, idx;
    uintptr_t rv=0;
    thread_info_t *tinfo = (thread_info_t *) arg;
    mem_info_t *mem_array;

    if (bind_to_cpu == -1)
    {
        printf("\nThread %d starting.....\n", tinfo->thread_num);
    }
    else
    {
        cpu_set_t mask;
        CPU_ZERO(&mask);
        CPU_SET(bind_to_cpu, &mask);
        if (0 > sched_setaffinity(0, sizeof(cpu_set_t), &mask))
        {
            printf("\nThread %d failed to bind to cpu %d!  exit.\n", tinfo->thread_num, bind_to_cpu);
            rv = 1;
            return (void *) rv;
        }
        else
        {
            printf("\nThread %d starting (bound to cpu %d)...\n", tinfo->thread_num, bind_to_cpu);
        }
    }

    mem_array = cond_lock_calloc(&mem_mutex, sizeof(mem_info_t), num_slots);
    for (i=0; i < num_loops; i++)
    {
        idx = random() % num_slots;
        if (mem_array[idx].in_use)
        {
            /*
             * Buffer already allocated, verify the pattern is still correct.
             * Then free this slot.
             */
            if (verify_slot(&mem_array[idx]))
            {
                printf("thread %d: verify slot failed on slot %d loop %d\n",
                       tinfo->thread_num, idx, i);
                rv = 1;
            }

            free_slot(&mem_array[idx]);
            tinfo->num_frees++;
            LOCKED_INC(&free_mutex, g_total_frees);
        }
        else
        {
            /*
             * Slot is currently empty.  Allocate a buffer of random size
             * and fill it with a known random pattern.
             */
            fill_slot(&mem_array[idx]);
            if (mem_array[idx].num_words > 0)
                tinfo->num_allocs++;
            LOCKED_INC(&malloc_mutex, g_total_mallocs);
        }

        if ((i != 0) && (i % 1000 == 0))
        {
            printf("%d", tinfo->thread_num);
            fflush(stdout);
        }
    }

    /* free any buffers left over */
    for (idx=0; idx < num_slots; idx++)
    {
        if (mem_array[idx].in_use)
        {
            if (verify_slot(&mem_array[idx]))
            {
                printf("thread %d: verify slot failed on slot %d, cleanup\n",
                       tinfo->thread_num, idx);
                rv = 1;
            }
            free_slot(&mem_array[idx]);
            tinfo->num_frees++;
            LOCKED_INC(&free_mutex, g_total_frees);
        }
    }

    if (tinfo->num_allocs != tinfo->num_frees) {
        printf("thread %d: alloc/free mismatch, %d vs %d\n",
               tinfo->thread_num, tinfo->num_allocs, tinfo->num_frees);
        rv = 1;
    }

    printf("\nThread %d finished\n", tinfo->thread_num);

    free(mem_array);

    return (void *) rv;
}


/**************************************************************************
 *
 * Utility functions
 *
 **************************************************************************/

/** Fill a slot with a buffer, put a known pattern in the buffer, fill
 * out the mem_info struct with info about the slot.
 */
void fill_slot(mem_info_t *slot)
{
    while (slot->num_words == 0)
        slot->num_words = random() % 4096;

    slot->buf = cond_lock_malloc(&mem_mutex, slot->num_words * sizeof(int));
    if (NULL == slot->buf)
    {
        printf("memory allocation of %" PRIuPTR " bytes failed\n",
               slot->num_words * sizeof(int));
        slot->num_words = 0;
    }
    else
    {
        int j;

        slot->in_use = 1;
        slot->pattern = random();
        for (j=0; j < slot->num_words; j++)
        {
            slot->buf[j] = slot->pattern;
        }
    }
}


/** Verify the memory buffer in the given slot.
 *
 * @return 0 if good, non-zero on error.
 */
int verify_slot(const mem_info_t *slot)
{
    int j;
    int rc=0;

    for (j=0; j < slot->num_words; j++)
    {
        if (slot->buf[j] != slot->pattern)
        {
            printf("Pattern match failed at j=%d\n", j);
            rc = 1;
        }
    }

    return rc;
}
/** Free memory buffer in the given slot and zeroize the slot.
 *
 */
void free_slot(mem_info_t *slot)
{
    cond_lock_free(&mem_mutex, slot->buf);
    slot->buf = NULL;
    slot->num_words = 0;
    slot->pattern = 0;
    slot->in_use = 0;
}

void *cond_lock_calloc(pthread_mutex_t *m, int s, int n)
{
    void *p;

    if (do_mutex) {
        if (0 != pthread_mutex_lock(m))
            printf("could not acquire lock!\n");
    }

    p = calloc(s, n);

    if (do_mutex)
        pthread_mutex_unlock(m);

    return p;
}

void *cond_lock_malloc(pthread_mutex_t *m, int s)
{
    void *p;

    if (do_mutex) {
        if (0 != pthread_mutex_lock(m))
            printf("could not acquire lock!\n");
    }

    p = malloc(s);

    if (do_mutex)
        pthread_mutex_unlock(m);

    return p;
}


void cond_lock_free(pthread_mutex_t *m, void *p)
{
    if (do_mutex) {
        if (0 != pthread_mutex_lock(m))
            printf("could not acquire lock!\n");
    }

    free(p);

    if (do_mutex)
        pthread_mutex_unlock(m);
}

