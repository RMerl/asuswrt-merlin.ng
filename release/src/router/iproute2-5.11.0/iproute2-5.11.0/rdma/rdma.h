/* SPDX-License-Identifier: GPL-2.0 OR Linux-OpenIB */
/*
 * rdma.c	RDMA tool
 * Authors:     Leon Romanovsky <leonro@mellanox.com>
 */
#ifndef _RDMA_TOOL_H_
#define _RDMA_TOOL_H_

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <getopt.h>
#include <netinet/in.h>
#include <libmnl/libmnl.h>
#include <rdma/rdma_netlink.h>
#include <rdma/rdma_user_cm.h>
#include <time.h>
#include <net/if_arp.h>

#include "list.h"
#include "utils.h"
#include "mnl_utils.h"
#include "json_print.h"

#define pr_err(args...) fprintf(stderr, ##args)
#define pr_out(args...) fprintf(stdout, ##args)

#define RDMA_BITMAP_ENUM(name, bit_no) RDMA_BITMAP_##name = BIT(bit_no),
#define RDMA_BITMAP_NAMES(name, bit_no) [bit_no] = #name,

#define MAX_NUMBER_OF_FILTERS 64
struct filters {
	const char *name;
	uint8_t is_number:1;
	uint8_t is_doit:1;
};

struct filter_entry {
	struct list_head list;
	char *key;
	char *value;
	/*
	 * This field means that we can try to issue .doit calback
	 * on value above. This value can be converted to integer
	 * with simple atoi(). Otherwise "is_doit" will be false.
	 */
	uint8_t is_doit:1;
};

struct dev_map {
	struct list_head list;
	char *dev_name;
	uint32_t num_ports;
	uint32_t idx;
};

struct rd {
	int argc;
	char **argv;
	char *filename;
	uint8_t show_details:1;
	uint8_t show_driver_details:1;
	uint8_t show_raw:1;
	struct list_head dev_map_list;
	uint32_t dev_idx;
	uint32_t port_idx;
	struct mnl_socket *nl;
	struct nlmsghdr *nlh;
	char *buff;
	json_writer_t *jw;
	int json_output;
	int pretty_output;
	bool suppress_errors;
	struct list_head filter_list;
	char *link_name;
	char *link_type;
};

struct rd_cmd {
	const char *cmd;
	int (*func)(struct rd *rd);
};

/*
 * Parser interface
 */
bool rd_no_arg(struct rd *rd);
bool rd_is_multiarg(struct rd *rd);
void rd_arg_inc(struct rd *rd);

char *rd_argv(struct rd *rd);

/*
 * Commands interface
 */
int cmd_dev(struct rd *rd);
int cmd_link(struct rd *rd);
int cmd_res(struct rd *rd);
int cmd_sys(struct rd *rd);
int cmd_stat(struct rd *rd);
int rd_exec_cmd(struct rd *rd, const struct rd_cmd *c, const char *str);
int rd_exec_dev(struct rd *rd, int (*cb)(struct rd *rd));
int rd_exec_require_dev(struct rd *rd, int (*cb)(struct rd *rd));
int rd_exec_link(struct rd *rd, int (*cb)(struct rd *rd), bool strict_port);
void rd_free(struct rd *rd);
int rd_set_arg_to_devname(struct rd *rd);
int rd_argc(struct rd *rd);

int strcmpx(const char *str1, const char *str2);

/*
 * Device manipulation
 */
struct dev_map *dev_map_lookup(struct rd *rd, bool allow_port_index);

/*
 * Filter manipulation
 */
bool rd_doit_index(struct rd *rd, uint32_t *idx);
int rd_build_filter(struct rd *rd, const struct filters valid_filters[]);
bool rd_is_filtered_attr(struct rd *rd, const char *key, uint32_t val,
			 struct nlattr *attr);
bool rd_is_string_filtered_attr(struct rd *rd, const char *key, const char *val,
				struct nlattr *attr);
/*
 * Netlink
 */
int rd_send_msg(struct rd *rd);
int rd_recv_msg(struct rd *rd, mnl_cb_t callback, void *data, uint32_t seq);
int rd_sendrecv_msg(struct rd *rd, unsigned int seq);
void rd_prepare_msg(struct rd *rd, uint32_t cmd, uint32_t *seq, uint16_t flags);
int rd_dev_init_cb(const struct nlmsghdr *nlh, void *data);
int rd_attr_cb(const struct nlattr *attr, void *data);
int rd_attr_check(const struct nlattr *attr, int *typep);

/*
 * Print helpers
 */
void print_driver_table(struct rd *rd, struct nlattr *tb);
void print_raw_data(struct rd *rd, struct nlattr **nla_line);
void newline(struct rd *rd);
void newline_indent(struct rd *rd);
void print_raw_data(struct rd *rd, struct nlattr **nla_line);
#define MAX_LINE_LENGTH 80

#endif /* _RDMA_TOOL_H_ */
