/*
 * CPU <-> hardware queue mapping helpers
 *
 * Copyright (C) 2013-2014 Jens Axboe
 */
#include <linux/kernel.h>
#include <linux/threads.h>
#include <linux/module.h>
#include <linux/mm.h>
#include <linux/smp.h>
#include <linux/cpu.h>

#include <linux/blk-mq.h>
#include "blk.h"
#include "blk-mq.h"

static int cpu_to_queue_index(unsigned int nr_cpus, unsigned int nr_queues,
			      const int cpu)
{
	return cpu * nr_queues / nr_cpus;
}

static int get_first_sibling(unsigned int cpu)
{
	unsigned int ret;

	ret = cpumask_first(topology_thread_cpumask(cpu));
	if (ret < nr_cpu_ids)
		return ret;

	return cpu;
}

int blk_mq_update_queue_map(unsigned int *map, unsigned int nr_queues)
{
	unsigned int i, nr_cpus, nr_uniq_cpus, queue, first_sibling;
	cpumask_var_t cpus;

	if (!alloc_cpumask_var(&cpus, GFP_ATOMIC))
		return 1;

	cpumask_clear(cpus);
	nr_cpus = nr_uniq_cpus = 0;
	for_each_online_cpu(i) {
		nr_cpus++;
		first_sibling = get_first_sibling(i);
		if (!cpumask_test_cpu(first_sibling, cpus))
			nr_uniq_cpus++;
		cpumask_set_cpu(i, cpus);
	}

	queue = 0;
	for_each_possible_cpu(i) {
		if (!cpu_online(i)) {
			map[i] = 0;
			continue;
		}

		/*
		 * Easy case - we have equal or more hardware queues. Or
		 * there are no thread siblings to take into account. Do
		 * 1:1 if enough, or sequential mapping if less.
		 */
		if (nr_queues >= nr_cpus || nr_cpus == nr_uniq_cpus) {
			map[i] = cpu_to_queue_index(nr_cpus, nr_queues, queue);
			queue++;
			continue;
		}

		/*
		 * Less then nr_cpus queues, and we have some number of
		 * threads per cores. Map sibling threads to the same
		 * queue.
		 */
		first_sibling = get_first_sibling(i);
		if (first_sibling == i) {
			map[i] = cpu_to_queue_index(nr_uniq_cpus, nr_queues,
							queue);
			queue++;
		} else
			map[i] = map[first_sibling];
	}

	free_cpumask_var(cpus);
	return 0;
}

unsigned int *blk_mq_make_queue_map(struct blk_mq_tag_set *set)
{
	unsigned int *map;

	/* If cpus are offline, map them to first hctx */
	map = kzalloc_node(sizeof(*map) * nr_cpu_ids, GFP_KERNEL,
				set->numa_node);
	if (!map)
		return NULL;

	if (!blk_mq_update_queue_map(map, set->nr_hw_queues))
		return map;

	kfree(map);
	return NULL;
}

/*
 * We have no quick way of doing reverse lookups. This is only used at
 * queue init time, so runtime isn't important.
 */
int blk_mq_hw_queue_to_node(unsigned int *mq_map, unsigned int index)
{
	int i;

	for_each_possible_cpu(i) {
		if (index == mq_map[i])
			return cpu_to_node(i);
	}

	return NUMA_NO_NODE;
}
