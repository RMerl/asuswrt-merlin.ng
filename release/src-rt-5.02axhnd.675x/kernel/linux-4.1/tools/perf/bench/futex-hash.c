/*
 * Copyright (C) 2013  Davidlohr Bueso <davidlohr@hp.com>
 *
 * futex-hash: Stress the hell out of the Linux kernel futex uaddr hashing.
 *
 * This program is particularly useful for measuring the kernel's futex hash
 * table/function implementation. In order for it to make sense, use with as
 * many threads and futexes as possible.
 */

#include "../perf.h"
#include "../util/util.h"
#include "../util/stat.h"
#include "../util/parse-options.h"
#include "../util/header.h"
#include "bench.h"
#include "futex.h"

#include <err.h>
#include <stdlib.h>
#include <sys/time.h>
#include <pthread.h>

static unsigned int nthreads = 0;
static unsigned int nsecs    = 10;
/* amount of futexes per thread */
static unsigned int nfutexes = 1024;
static bool fshared = false, done = false, silent = false;
static int futex_flag = 0;

struct timeval start, end, runtime;
static pthread_mutex_t thread_lock;
static unsigned int threads_starting;
static struct stats throughput_stats;
static pthread_cond_t thread_parent, thread_worker;

struct worker {
	int tid;
	u_int32_t *futex;
	pthread_t thread;
	unsigned long ops;
};

static const struct option options[] = {
	OPT_UINTEGER('t', "threads", &nthreads, "Specify amount of threads"),
	OPT_UINTEGER('r', "runtime", &nsecs,    "Specify runtime (in seconds)"),
	OPT_UINTEGER('f', "futexes", &nfutexes, "Specify amount of futexes per threads"),
	OPT_BOOLEAN( 's', "silent",  &silent,   "Silent mode: do not display data/details"),
	OPT_BOOLEAN( 'S', "shared",  &fshared,  "Use shared futexes instead of private ones"),
	OPT_END()
};

static const char * const bench_futex_hash_usage[] = {
	"perf bench futex hash <options>",
	NULL
};

static void *workerfn(void *arg)
{
	int ret;
	unsigned int i;
	struct worker *w = (struct worker *) arg;

	pthread_mutex_lock(&thread_lock);
	threads_starting--;
	if (!threads_starting)
		pthread_cond_signal(&thread_parent);
	pthread_cond_wait(&thread_worker, &thread_lock);
	pthread_mutex_unlock(&thread_lock);

	do {
		for (i = 0; i < nfutexes; i++, w->ops++) {
			/*
			 * We want the futex calls to fail in order to stress
			 * the hashing of uaddr and not measure other steps,
			 * such as internal waitqueue handling, thus enlarging
			 * the critical region protected by hb->lock.
			 */
			ret = futex_wait(&w->futex[i], 1234, NULL, futex_flag);
			if (!silent &&
			    (!ret || errno != EAGAIN || errno != EWOULDBLOCK))
				warn("Non-expected futex return call");
		}
	}  while (!done);

	return NULL;
}

static void toggle_done(int sig __maybe_unused,
			siginfo_t *info __maybe_unused,
			void *uc __maybe_unused)
{
	/* inform all threads that we're done for the day */
	done = true;
	gettimeofday(&end, NULL);
	timersub(&end, &start, &runtime);
}

static void print_summary(void)
{
	unsigned long avg = avg_stats(&throughput_stats);
	double stddev = stddev_stats(&throughput_stats);

	printf("%sAveraged %ld operations/sec (+- %.2f%%), total secs = %d\n",
	       !silent ? "\n" : "", avg, rel_stddev_stats(stddev, avg),
	       (int) runtime.tv_sec);
}

int bench_futex_hash(int argc, const char **argv,
		     const char *prefix __maybe_unused)
{
	int ret = 0;
	cpu_set_t cpu;
	struct sigaction act;
	unsigned int i, ncpus;
	pthread_attr_t thread_attr;
	struct worker *worker = NULL;

	argc = parse_options(argc, argv, options, bench_futex_hash_usage, 0);
	if (argc) {
		usage_with_options(bench_futex_hash_usage, options);
		exit(EXIT_FAILURE);
	}

	ncpus = sysconf(_SC_NPROCESSORS_ONLN);

	sigfillset(&act.sa_mask);
	act.sa_sigaction = toggle_done;
	sigaction(SIGINT, &act, NULL);

	if (!nthreads) /* default to the number of CPUs */
		nthreads = ncpus;

	worker = calloc(nthreads, sizeof(*worker));
	if (!worker)
		goto errmem;

	if (!fshared)
		futex_flag = FUTEX_PRIVATE_FLAG;

	printf("Run summary [PID %d]: %d threads, each operating on %d [%s] futexes for %d secs.\n\n",
	       getpid(), nthreads, nfutexes, fshared ? "shared":"private", nsecs);

	init_stats(&throughput_stats);
	pthread_mutex_init(&thread_lock, NULL);
	pthread_cond_init(&thread_parent, NULL);
	pthread_cond_init(&thread_worker, NULL);

	threads_starting = nthreads;
	pthread_attr_init(&thread_attr);
	gettimeofday(&start, NULL);
	for (i = 0; i < nthreads; i++) {
		worker[i].tid = i;
		worker[i].futex = calloc(nfutexes, sizeof(*worker[i].futex));
		if (!worker[i].futex)
			goto errmem;

		CPU_ZERO(&cpu);
		CPU_SET(i % ncpus, &cpu);

		ret = pthread_attr_setaffinity_np(&thread_attr, sizeof(cpu_set_t), &cpu);
		if (ret)
			err(EXIT_FAILURE, "pthread_attr_setaffinity_np");

		ret = pthread_create(&worker[i].thread, &thread_attr, workerfn,
				     (void *)(struct worker *) &worker[i]);
		if (ret)
			err(EXIT_FAILURE, "pthread_create");

	}
	pthread_attr_destroy(&thread_attr);

	pthread_mutex_lock(&thread_lock);
	while (threads_starting)
		pthread_cond_wait(&thread_parent, &thread_lock);
	pthread_cond_broadcast(&thread_worker);
	pthread_mutex_unlock(&thread_lock);

	sleep(nsecs);
	toggle_done(0, NULL, NULL);

	for (i = 0; i < nthreads; i++) {
		ret = pthread_join(worker[i].thread, NULL);
		if (ret)
			err(EXIT_FAILURE, "pthread_join");
	}

	/* cleanup & report results */
	pthread_cond_destroy(&thread_parent);
	pthread_cond_destroy(&thread_worker);
	pthread_mutex_destroy(&thread_lock);

	for (i = 0; i < nthreads; i++) {
		unsigned long t = worker[i].ops/runtime.tv_sec;
		update_stats(&throughput_stats, t);
		if (!silent) {
			if (nfutexes == 1)
				printf("[thread %2d] futex: %p [ %ld ops/sec ]\n",
				       worker[i].tid, &worker[i].futex[0], t);
			else
				printf("[thread %2d] futexes: %p ... %p [ %ld ops/sec ]\n",
				       worker[i].tid, &worker[i].futex[0],
				       &worker[i].futex[nfutexes-1], t);
		}

		free(worker[i].futex);
	}

	print_summary();

	free(worker);
	return ret;
errmem:
	err(EXIT_FAILURE, "calloc");
}
