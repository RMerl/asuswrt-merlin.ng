/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _TC_QEVENT_H_
#define _TC_QEVENT_H_

#include <stdbool.h>
#include <linux/types.h>
#include <libnetlink.h>

struct qevent_base {
	__u32 block_idx;
};

struct qevent_util {
	const char *id;
	int (*parse_qevent)(struct qevent_util *qu, int *argc, char ***argv);
	int (*read_qevent)(struct qevent_util *qu, struct rtattr **tb);
	void (*print_qevent)(struct qevent_util *qu, FILE *f);
	int (*dump_qevent)(struct qevent_util *qu, struct nlmsghdr *n);
	size_t data_size;
	void *data;
	int attr;
};

#define QEVENT(_name, _form, _data, _attr)				\
	{								\
		.id = _name,						\
		.parse_qevent = qevent_parse_##_form,			\
		.read_qevent = qevent_read_##_form,			\
		.print_qevent = qevent_print_##_form,			\
		.dump_qevent = qevent_dump_##_form,			\
		.data_size = sizeof(struct qevent_##_form),		\
		.data = _data,						\
		.attr = _attr,						\
	}

void qevents_init(struct qevent_util *qevents);
int qevent_parse(struct qevent_util *qevents, int *p_argc, char ***p_argv);
int qevents_read(struct qevent_util *qevents, struct rtattr **tb);
int qevents_dump(struct qevent_util *qevents, struct nlmsghdr *n);
void qevents_print(struct qevent_util *qevents, FILE *f);
bool qevents_have_block(struct qevent_util *qevents, __u32 block_idx);

struct qevent_plain {
	struct qevent_base base;
};
int qevent_parse_plain(struct qevent_util *qu, int *p_argc, char ***p_argv);
int qevent_read_plain(struct qevent_util *qu, struct rtattr **tb);
void qevent_print_plain(struct qevent_util *qu, FILE *f);
int qevent_dump_plain(struct qevent_util *qu, struct nlmsghdr *n);

#endif
