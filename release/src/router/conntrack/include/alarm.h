#ifndef _ALARM_H_
#define _ALARM_H_

#include "linux_rbtree.h"
#include "linux_list.h"

#include <sys/time.h>

struct alarm_block {
	struct rb_node		node;
	struct list_head	list;
	struct timeval		tv;
	void			*data;
	void			(*function)(struct alarm_block *a, void *data);
};

void init_alarm(struct alarm_block *t,
		void *data,
		void (*fcn)(struct alarm_block *a, void *data));

void add_alarm(struct alarm_block *alarm, unsigned long sc, unsigned long usc);

void del_alarm(struct alarm_block *alarm);

int alarm_pending(struct alarm_block *alarm);

struct timeval *
get_next_alarm_run(struct timeval *next_alarm);

struct timeval *
do_alarm_run(struct timeval *next_alarm);

#endif
