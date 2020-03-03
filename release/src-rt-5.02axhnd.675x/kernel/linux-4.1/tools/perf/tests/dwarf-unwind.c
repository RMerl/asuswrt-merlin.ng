#include <linux/compiler.h>
#include <linux/types.h>
#include <unistd.h>
#include "tests.h"
#include "debug.h"
#include "machine.h"
#include "event.h"
#include "unwind.h"
#include "perf_regs.h"
#include "map.h"
#include "thread.h"
#include "callchain.h"

/* For bsearch. We try to unwind functions in shared object. */
#include <stdlib.h>

static int mmap_handler(struct perf_tool *tool __maybe_unused,
			union perf_event *event,
			struct perf_sample *sample __maybe_unused,
			struct machine *machine)
{
	return machine__process_mmap2_event(machine, event, NULL);
}

static int init_live_machine(struct machine *machine)
{
	union perf_event event;
	pid_t pid = getpid();

	return perf_event__synthesize_mmap_events(NULL, &event, pid, pid,
						  mmap_handler, machine, true);
}

#define MAX_STACK 8

static int unwind_entry(struct unwind_entry *entry, void *arg)
{
	unsigned long *cnt = (unsigned long *) arg;
	char *symbol = entry->sym ? entry->sym->name : NULL;
	static const char *funcs[MAX_STACK] = {
		"test__arch_unwind_sample",
		"unwind_thread",
		"compare",
		"bsearch",
		"krava_3",
		"krava_2",
		"krava_1",
		"test__dwarf_unwind"
	};

	if (*cnt >= MAX_STACK) {
		pr_debug("failed: crossed the max stack value %d\n", MAX_STACK);
		return -1;
	}

	if (!symbol) {
		pr_debug("failed: got unresolved address 0x%" PRIx64 "\n",
			 entry->ip);
		return -1;
	}

	pr_debug("got: %s 0x%" PRIx64 "\n", symbol, entry->ip);
	return strcmp((const char *) symbol, funcs[(*cnt)++]);
}

__attribute__ ((noinline))
static int unwind_thread(struct thread *thread)
{
	struct perf_sample sample;
	unsigned long cnt = 0;
	int err = -1;

	memset(&sample, 0, sizeof(sample));

	if (test__arch_unwind_sample(&sample, thread)) {
		pr_debug("failed to get unwind sample\n");
		goto out;
	}

	err = unwind__get_entries(unwind_entry, &cnt, thread,
				  &sample, MAX_STACK);
	if (err)
		pr_debug("unwind failed\n");
	else if (cnt != MAX_STACK) {
		pr_debug("got wrong number of stack entries %lu != %d\n",
			 cnt, MAX_STACK);
		err = -1;
	}

 out:
	free(sample.user_stack.data);
	free(sample.user_regs.regs);
	return err;
}

static int global_unwind_retval = -INT_MAX;

__attribute__ ((noinline))
static int compare(void *p1, void *p2)
{
	/* Any possible value should be 'thread' */
	struct thread *thread = *(struct thread **)p1;

	if (global_unwind_retval == -INT_MAX)
		global_unwind_retval = unwind_thread(thread);

	return p1 - p2;
}

__attribute__ ((noinline))
static int krava_3(struct thread *thread)
{
	struct thread *array[2] = {thread, thread};
	void *fp = &bsearch;
	/*
	 * make _bsearch a volatile function pointer to
	 * prevent potential optimization, which may expand
	 * bsearch and call compare directly from this function,
	 * instead of libc shared object.
	 */
	void *(*volatile _bsearch)(void *, void *, size_t,
			size_t, int (*)(void *, void *));

	_bsearch = fp;
	_bsearch(array, &thread, 2, sizeof(struct thread **), compare);
	return global_unwind_retval;
}

__attribute__ ((noinline))
static int krava_2(struct thread *thread)
{
	return krava_3(thread);
}

__attribute__ ((noinline))
static int krava_1(struct thread *thread)
{
	return krava_2(thread);
}

int test__dwarf_unwind(void)
{
	struct machines machines;
	struct machine *machine;
	struct thread *thread;
	int err = -1;

	machines__init(&machines);

	machine = machines__find(&machines, HOST_KERNEL_ID);
	if (!machine) {
		pr_err("Could not get machine\n");
		return -1;
	}

	callchain_param.record_mode = CALLCHAIN_DWARF;

	if (init_live_machine(machine)) {
		pr_err("Could not init machine\n");
		goto out;
	}

	if (verbose > 1)
		machine__fprintf(machine, stderr);

	thread = machine__find_thread(machine, getpid(), getpid());
	if (!thread) {
		pr_err("Could not get thread\n");
		goto out;
	}

	err = krava_1(thread);

 out:
	machine__delete_threads(machine);
	machine__exit(machine);
	machines__exit(&machines);
	return err;
}
