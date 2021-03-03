// SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause

/*
 * Helpers for handling qevents.
 */

#include <stdio.h>
#include <string.h>

#include "tc_qevent.h"
#include "utils.h"

void qevents_init(struct qevent_util *qevents)
{
	if (!qevents)
		return;

	for (; qevents->id; qevents++)
		memset(qevents->data, 0, qevents->data_size);
}

int qevent_parse(struct qevent_util *qevents, int *p_argc, char ***p_argv)
{
	char **argv = *p_argv;
	int argc = *p_argc;
	const char *name = *argv;
	int err;

	if (!qevents)
		goto out;

	for (; qevents->id; qevents++) {
		if (strcmp(name, qevents->id) == 0) {
			NEXT_ARG();
			err = qevents->parse_qevent(qevents, &argc, &argv);
			if (err)
				return err;

			*p_argc = argc;
			*p_argv = argv;
			return 0;
		}
	}

out:
	fprintf(stderr, "Unknown qevent `%s'\n", name);
	return -1;
}

int qevents_read(struct qevent_util *qevents, struct rtattr **tb)
{
	int err;

	if (!qevents)
		return 0;

	for (; qevents->id; qevents++) {
		if (tb[qevents->attr]) {
			err = qevents->read_qevent(qevents, tb);
			if (err)
				return err;
		}
	}

	return 0;
}

void qevents_print(struct qevent_util *qevents, FILE *f)
{
	int first = true;

	if (!qevents)
		return;

	for (; qevents->id; qevents++) {
		struct qevent_base *qeb = qevents->data;

		if (qeb->block_idx) {
			if (first) {
				open_json_array(PRINT_JSON, "qevents");
				first = false;
			}

			open_json_object(NULL);
			print_string(PRINT_ANY, "kind", "qevent %s", qevents->id);
			qevents->print_qevent(qevents, f);
			print_string(PRINT_FP, NULL, "%s", " ");
			close_json_object();
		}
	}

	if (!first)
		close_json_array(PRINT_ANY, "");
}

bool qevents_have_block(struct qevent_util *qevents, __u32 block_idx)
{
	if (!qevents)
		return false;

	for (; qevents->id; qevents++) {
		struct qevent_base *qeb = qevents->data;

		if (qeb->block_idx == block_idx)
			return true;
	}

	return false;
}

int qevents_dump(struct qevent_util *qevents, struct nlmsghdr *n)
{
	int err;

	if (!qevents)
		return 0;

	for (; qevents->id; qevents++) {
		struct qevent_base *qeb = qevents->data;

		if (qeb->block_idx) {
			err = qevents->dump_qevent(qevents, n);
			if (err)
				return err;
		}
	}

	return 0;
}

static int parse_block_idx(const char *arg, struct qevent_base *qeb)
{
	if (qeb->block_idx) {
		fprintf(stderr, "Qevent block index already specified\n");
		return -1;
	}

	if (get_unsigned(&qeb->block_idx, arg, 10) || !qeb->block_idx) {
		fprintf(stderr, "Illegal qevent block index\n");
		return -1;
	}

	return 0;
}

static int read_block_idx(struct rtattr *attr, struct qevent_base *qeb)
{
	if (qeb->block_idx) {
		fprintf(stderr, "Qevent block index already specified\n");
		return -1;
	}

	qeb->block_idx = rta_getattr_u32(attr);
	if (!qeb->block_idx) {
		fprintf(stderr, "Illegal qevent block index\n");
		return -1;
	}

	return 0;
}

static void print_block_idx(FILE *f, __u32 block_idx)
{
	print_uint(PRINT_ANY, "block", " block %u", block_idx);
}

int qevent_parse_plain(struct qevent_util *qu, int *p_argc, char ***p_argv)
{
	struct qevent_plain *qe = qu->data;
	char **argv = *p_argv;
	int argc = *p_argc;

	if (qe->base.block_idx) {
		fprintf(stderr, "Duplicate qevent\n");
		return -1;
	}

	while (argc > 0) {
		if (strcmp(*argv, "block") == 0) {
			NEXT_ARG();
			if (parse_block_idx(*argv, &qe->base))
				return -1;
		} else {
			break;
		}
		NEXT_ARG_FWD();
	}

	if (!qe->base.block_idx) {
		fprintf(stderr, "Unspecified qevent block index\n");
		return -1;
	}

	*p_argc = argc;
	*p_argv = argv;
	return 0;
}

int qevent_read_plain(struct qevent_util *qu, struct rtattr **tb)
{
	struct qevent_plain *qe = qu->data;

	return read_block_idx(tb[qu->attr], &qe->base);
}

void qevent_print_plain(struct qevent_util *qu, FILE *f)
{
	struct qevent_plain *qe = qu->data;

	print_block_idx(f, qe->base.block_idx);
}

int qevent_dump_plain(struct qevent_util *qu, struct nlmsghdr *n)
{
	struct qevent_plain *qe = qu->data;

	return addattr32(n, 1024, qu->attr, qe->base.block_idx);
}
