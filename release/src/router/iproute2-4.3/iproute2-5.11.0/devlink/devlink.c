/*
 * devlink.c	Devlink tool
 *
 *              This program is free software; you can redistribute it and/or
 *              modify it under the terms of the GNU General Public License
 *              as published by the Free Software Foundation; either version
 *              2 of the License, or (at your option) any later version.
 *
 * Authors:     Jiri Pirko <jiri@mellanox.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <getopt.h>
#include <limits.h>
#include <errno.h>
#include <inttypes.h>
#include <signal.h>
#include <time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/sysinfo.h>
#define _LINUX_SYSINFO_H /* avoid collision with musl header */
#include <linux/genetlink.h>
#include <linux/devlink.h>
#include <linux/netlink.h>
#include <libmnl/libmnl.h>
#include <netinet/ether.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <rt_names.h>

#include "version.h"
#include "list.h"
#include "mnlg.h"
#include "json_print.h"
#include "utils.h"
#include "namespace.h"

#define ESWITCH_MODE_LEGACY "legacy"
#define ESWITCH_MODE_SWITCHDEV "switchdev"
#define ESWITCH_INLINE_MODE_NONE "none"
#define ESWITCH_INLINE_MODE_LINK "link"
#define ESWITCH_INLINE_MODE_NETWORK "network"
#define ESWITCH_INLINE_MODE_TRANSPORT "transport"

#define ESWITCH_ENCAP_MODE_NONE "none"
#define ESWITCH_ENCAP_MODE_BASIC "basic"

#define PARAM_CMODE_RUNTIME_STR "runtime"
#define PARAM_CMODE_DRIVERINIT_STR "driverinit"
#define PARAM_CMODE_PERMANENT_STR "permanent"
#define DL_ARGS_REQUIRED_MAX_ERR_LEN 80

#define HEALTH_REPORTER_STATE_HEALTHY_STR "healthy"
#define HEALTH_REPORTER_STATE_ERROR_STR "error"
#define HEALTH_REPORTER_TIMESTAMP_FMT_LEN 80

static int g_new_line_count;
static int g_indent_level;
static bool g_indent_newline;

#define INDENT_STR_STEP 2
#define INDENT_STR_MAXLEN 32
static char g_indent_str[INDENT_STR_MAXLEN + 1] = "";

static void __attribute__((format(printf, 1, 2)))
pr_err(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
}

static void __attribute__((format(printf, 1, 2)))
pr_out(const char *fmt, ...)
{
	va_list ap;

	if (g_indent_newline) {
		printf("%s", g_indent_str);
		g_indent_newline = false;
	}
	va_start(ap, fmt);
	vprintf(fmt, ap);
	va_end(ap);
	g_new_line_count = 0;
}

static void __attribute__((format(printf, 2, 3)))
pr_out_sp(unsigned int num, const char *fmt, ...)
{
	va_list ap;
	int ret;

	va_start(ap, fmt);
	ret = vprintf(fmt, ap);
	va_end(ap);

	if (ret < num)
		printf("%*s", num - ret, "");
	g_new_line_count = 0;			\
}

static void __attribute__((format(printf, 1, 2)))
pr_out_tty(const char *fmt, ...)
{
	va_list ap;

	if (!isatty(STDOUT_FILENO))
		return;
	va_start(ap, fmt);
	vprintf(fmt, ap);
	va_end(ap);
}

static void __pr_out_indent_inc(void)
{
	if (g_indent_level + INDENT_STR_STEP > INDENT_STR_MAXLEN)
		return;
	g_indent_level += INDENT_STR_STEP;
	memset(g_indent_str, ' ', sizeof(g_indent_str));
	g_indent_str[g_indent_level] = '\0';
}

static void __pr_out_indent_dec(void)
{
	if (g_indent_level - INDENT_STR_STEP < 0)
		return;
	g_indent_level -= INDENT_STR_STEP;
	g_indent_str[g_indent_level] = '\0';
}

static void __pr_out_newline(void)
{
	if (g_new_line_count < 1) {
		pr_out("\n");
		g_indent_newline = true;
	}
	g_new_line_count++;
}

static int _mnlg_socket_recv_run(struct mnlg_socket *nlg,
				 mnl_cb_t data_cb, void *data)
{
	int err;

	err = mnlg_socket_recv_run(nlg, data_cb, data);
	if (err < 0) {
		pr_err("devlink answers: %s\n", strerror(errno));
		return -errno;
	}
	return 0;
}

static void dummy_signal_handler(int signum)
{
}

static int _mnlg_socket_recv_run_intr(struct mnlg_socket *nlg,
				      mnl_cb_t data_cb, void *data)
{
	struct sigaction act, oact;
	int err;

	act.sa_handler = dummy_signal_handler;
	sigemptyset(&act.sa_mask);
	act.sa_flags = SA_NODEFER;

	sigaction(SIGINT, &act, &oact);
	err = mnlg_socket_recv_run(nlg, data_cb, data);
	sigaction(SIGINT, &oact, NULL);
	if (err < 0 && errno != EINTR) {
		pr_err("devlink answers: %s\n", strerror(errno));
		return -errno;
	}
	return 0;
}

static int _mnlg_socket_send(struct mnlg_socket *nlg,
			     const struct nlmsghdr *nlh)
{
	int err;

	err = mnlg_socket_send(nlg, nlh);
	if (err < 0) {
		pr_err("Failed to call mnlg_socket_send\n");
		return -errno;
	}
	return 0;
}

static int _mnlg_socket_sndrcv(struct mnlg_socket *nlg,
			       const struct nlmsghdr *nlh,
			       mnl_cb_t data_cb, void *data)
{
	int err;

	err = _mnlg_socket_send(nlg, nlh);
	if (err)
		return err;
	return _mnlg_socket_recv_run(nlg, data_cb, data);
}

static int _mnlg_socket_group_add(struct mnlg_socket *nlg,
				  const char *group_name)
{
	int err;

	err = mnlg_socket_group_add(nlg, group_name);
	if (err < 0) {
		pr_err("Failed to call mnlg_socket_group_add\n");
		return -errno;
	}
	return 0;
}

struct ifname_map {
	struct list_head list;
	char *bus_name;
	char *dev_name;
	uint32_t port_index;
	char *ifname;
};

static struct ifname_map *ifname_map_alloc(const char *bus_name,
					   const char *dev_name,
					   uint32_t port_index,
					   const char *ifname)
{
	struct ifname_map *ifname_map;

	ifname_map = calloc(1, sizeof(*ifname_map));
	if (!ifname_map)
		return NULL;
	ifname_map->bus_name = strdup(bus_name);
	ifname_map->dev_name = strdup(dev_name);
	ifname_map->port_index = port_index;
	ifname_map->ifname = strdup(ifname);
	if (!ifname_map->bus_name || !ifname_map->dev_name ||
	    !ifname_map->ifname) {
		free(ifname_map->ifname);
		free(ifname_map->dev_name);
		free(ifname_map->bus_name);
		free(ifname_map);
		return NULL;
	}
	return ifname_map;
}

static void ifname_map_free(struct ifname_map *ifname_map)
{
	free(ifname_map->ifname);
	free(ifname_map->dev_name);
	free(ifname_map->bus_name);
	free(ifname_map);
}

#define DL_OPT_HANDLE		BIT(0)
#define DL_OPT_HANDLEP		BIT(1)
#define DL_OPT_PORT_TYPE	BIT(2)
#define DL_OPT_PORT_COUNT	BIT(3)
#define DL_OPT_SB		BIT(4)
#define DL_OPT_SB_POOL		BIT(5)
#define DL_OPT_SB_SIZE		BIT(6)
#define DL_OPT_SB_TYPE		BIT(7)
#define DL_OPT_SB_THTYPE	BIT(8)
#define DL_OPT_SB_TH		BIT(9)
#define DL_OPT_SB_TC		BIT(10)
#define DL_OPT_ESWITCH_MODE	BIT(11)
#define DL_OPT_ESWITCH_INLINE_MODE	BIT(12)
#define DL_OPT_DPIPE_TABLE_NAME	BIT(13)
#define DL_OPT_DPIPE_TABLE_COUNTERS	BIT(14)
#define DL_OPT_ESWITCH_ENCAP_MODE	BIT(15)
#define DL_OPT_RESOURCE_PATH	BIT(16)
#define DL_OPT_RESOURCE_SIZE	BIT(17)
#define DL_OPT_PARAM_NAME	BIT(18)
#define DL_OPT_PARAM_VALUE	BIT(19)
#define DL_OPT_PARAM_CMODE	BIT(20)
#define DL_OPT_HANDLE_REGION		BIT(21)
#define DL_OPT_REGION_SNAPSHOT_ID	BIT(22)
#define DL_OPT_REGION_ADDRESS		BIT(23)
#define DL_OPT_REGION_LENGTH		BIT(24)
#define DL_OPT_FLASH_FILE_NAME	BIT(25)
#define DL_OPT_FLASH_COMPONENT	BIT(26)
#define DL_OPT_HEALTH_REPORTER_NAME	BIT(27)
#define DL_OPT_HEALTH_REPORTER_GRACEFUL_PERIOD	BIT(28)
#define DL_OPT_HEALTH_REPORTER_AUTO_RECOVER	BIT(29)
#define DL_OPT_TRAP_NAME		BIT(30)
#define DL_OPT_TRAP_ACTION		BIT(31)
#define DL_OPT_TRAP_GROUP_NAME		BIT(32)
#define DL_OPT_NETNS	BIT(33)
#define DL_OPT_TRAP_POLICER_ID		BIT(34)
#define DL_OPT_TRAP_POLICER_RATE	BIT(35)
#define DL_OPT_TRAP_POLICER_BURST	BIT(36)
#define DL_OPT_HEALTH_REPORTER_AUTO_DUMP     BIT(37)
#define DL_OPT_PORT_FUNCTION_HW_ADDR BIT(38)
#define DL_OPT_FLASH_OVERWRITE		BIT(39)
#define DL_OPT_RELOAD_ACTION		BIT(40)
#define DL_OPT_RELOAD_LIMIT	BIT(41)
#define DL_OPT_PORT_FLAVOUR BIT(42)
#define DL_OPT_PORT_PFNUMBER BIT(43)
#define DL_OPT_PORT_SFNUMBER BIT(44)
#define DL_OPT_PORT_FUNCTION_STATE BIT(45)

struct dl_opts {
	uint64_t present; /* flags of present items */
	char *bus_name;
	char *dev_name;
	uint32_t port_index;
	enum devlink_port_type port_type;
	uint32_t port_count;
	uint32_t sb_index;
	uint16_t sb_pool_index;
	uint32_t sb_pool_size;
	enum devlink_sb_pool_type sb_pool_type;
	enum devlink_sb_threshold_type sb_pool_thtype;
	uint32_t sb_threshold;
	uint16_t sb_tc_index;
	enum devlink_eswitch_mode eswitch_mode;
	enum devlink_eswitch_inline_mode eswitch_inline_mode;
	const char *dpipe_table_name;
	bool dpipe_counters_enabled;
	enum devlink_eswitch_encap_mode eswitch_encap_mode;
	const char *resource_path;
	uint64_t resource_size;
	uint32_t resource_id;
	bool resource_id_valid;
	const char *param_name;
	const char *param_value;
	enum devlink_param_cmode cmode;
	char *region_name;
	uint32_t region_snapshot_id;
	uint64_t region_address;
	uint64_t region_length;
	const char *flash_file_name;
	const char *flash_component;
	const char *reporter_name;
	uint64_t reporter_graceful_period;
	bool reporter_auto_recover;
	bool reporter_auto_dump;
	const char *trap_name;
	const char *trap_group_name;
	enum devlink_trap_action trap_action;
	bool netns_is_pid;
	uint32_t netns;
	uint32_t trap_policer_id;
	uint64_t trap_policer_rate;
	uint64_t trap_policer_burst;
	char port_function_hw_addr[MAX_ADDR_LEN];
	uint32_t port_function_hw_addr_len;
	uint32_t overwrite_mask;
	enum devlink_reload_action reload_action;
	enum devlink_reload_limit reload_limit;
	uint32_t port_sfnumber;
	uint16_t port_flavour;
	uint16_t port_pfnumber;
	uint8_t port_fn_state;
};

struct dl {
	struct mnlg_socket *nlg;
	struct list_head ifname_map_list;
	int argc;
	char **argv;
	bool no_nice_names;
	struct dl_opts opts;
	bool json_output;
	bool pretty_output;
	bool verbose;
	bool stats;
	struct {
		bool present;
		char *bus_name;
		char *dev_name;
		uint32_t port_index;
	} arr_last;
};

static int dl_argc(struct dl *dl)
{
	return dl->argc;
}

static char *dl_argv(struct dl *dl)
{
	if (dl_argc(dl) == 0)
		return NULL;
	return *dl->argv;
}

static void dl_arg_inc(struct dl *dl)
{
	if (dl_argc(dl) == 0)
		return;
	dl->argc--;
	dl->argv++;
}

static void dl_arg_dec(struct dl *dl)
{
	dl->argc++;
	dl->argv--;
}

static char *dl_argv_next(struct dl *dl)
{
	char *ret;

	if (dl_argc(dl) == 0)
		return NULL;

	ret = *dl->argv;
	dl_arg_inc(dl);
	return ret;
}

static char *dl_argv_index(struct dl *dl, unsigned int index)
{
	if (index >= dl_argc(dl))
		return NULL;
	return dl->argv[index];
}

static int strcmpx(const char *str1, const char *str2)
{
	if (strlen(str1) > strlen(str2))
		return -1;
	return strncmp(str1, str2, strlen(str1));
}

static bool dl_argv_match(struct dl *dl, const char *pattern)
{
	if (dl_argc(dl) == 0)
		return false;
	return strcmpx(dl_argv(dl), pattern) == 0;
}

static bool dl_no_arg(struct dl *dl)
{
	return dl_argc(dl) == 0;
}

static void __pr_out_indent_newline(struct dl *dl)
{
	if (!g_indent_newline && !dl->json_output)
		pr_out(" ");
}

static bool is_binary_eol(int i)
{
	return !(i%16);
}

static void pr_out_binary_value(struct dl *dl, uint8_t *data, uint32_t len)
{
	int i = 0;

	while (i < len) {
		if (dl->json_output)
			print_int(PRINT_JSON, NULL, NULL, data[i]);
		else
			pr_out("%02x ", data[i]);
		i++;
		if (!dl->json_output && is_binary_eol(i))
			__pr_out_newline();
	}
	if (!dl->json_output && !is_binary_eol(i))
		__pr_out_newline();
}

static void pr_out_name(struct dl *dl, const char *name)
{
	__pr_out_indent_newline(dl);
	if (dl->json_output)
		print_string(PRINT_JSON, name, NULL, NULL);
	else
		pr_out("%s:", name);
}

static void pr_out_u64(struct dl *dl, const char *name, uint64_t val)
{
	__pr_out_indent_newline(dl);
	if (val == (uint64_t) -1)
		return print_string_name_value(name, "unlimited");

	if (dl->json_output)
		print_u64(PRINT_JSON, name, NULL, val);
	else
		pr_out("%s %"PRIu64, name, val);
}

static void pr_out_section_start(struct dl *dl, const char *name)
{
	if (dl->json_output) {
		open_json_object(NULL);
		open_json_object(name);
	}
}

static void pr_out_section_end(struct dl *dl)
{
	if (dl->json_output) {
		if (dl->arr_last.present)
			close_json_array(PRINT_JSON, NULL);
		close_json_object();
		close_json_object();
	}
}

static void pr_out_array_start(struct dl *dl, const char *name)
{
	if (dl->json_output) {
		open_json_array(PRINT_JSON, name);
	} else {
		__pr_out_indent_inc();
		__pr_out_newline();
		pr_out("%s:", name);
		__pr_out_indent_inc();
		__pr_out_newline();
	}
}

static void pr_out_array_end(struct dl *dl)
{
	if (dl->json_output) {
		close_json_array(PRINT_JSON, NULL);
	} else {
		__pr_out_indent_dec();
		__pr_out_indent_dec();
	}
}

static void pr_out_object_start(struct dl *dl, const char *name)
{
	if (dl->json_output) {
		open_json_object(name);
	} else {
		__pr_out_indent_inc();
		__pr_out_newline();
		pr_out("%s:", name);
		__pr_out_indent_inc();
		__pr_out_newline();
	}
}

static void pr_out_object_end(struct dl *dl)
{
	if (dl->json_output) {
		close_json_object();
	} else {
		__pr_out_indent_dec();
		__pr_out_indent_dec();
	}
}

static void pr_out_entry_start(struct dl *dl)
{
	if (dl->json_output)
		open_json_object(NULL);
}

static void pr_out_entry_end(struct dl *dl)
{
	if (dl->json_output)
		close_json_object();
	else
		__pr_out_newline();
}

static void check_indent_newline(struct dl *dl)
{
	__pr_out_indent_newline(dl);

	if (g_indent_newline && !is_json_context()) {
		printf("%s", g_indent_str);
		g_indent_newline = false;
	}
	g_new_line_count = 0;
}

static const enum mnl_attr_data_type devlink_policy[DEVLINK_ATTR_MAX + 1] = {
	[DEVLINK_ATTR_BUS_NAME] = MNL_TYPE_NUL_STRING,
	[DEVLINK_ATTR_DEV_NAME] = MNL_TYPE_NUL_STRING,
	[DEVLINK_ATTR_PORT_INDEX] = MNL_TYPE_U32,
	[DEVLINK_ATTR_PORT_TYPE] = MNL_TYPE_U16,
	[DEVLINK_ATTR_PORT_DESIRED_TYPE] = MNL_TYPE_U16,
	[DEVLINK_ATTR_PORT_NETDEV_IFINDEX] = MNL_TYPE_U32,
	[DEVLINK_ATTR_PORT_NETDEV_NAME] = MNL_TYPE_NUL_STRING,
	[DEVLINK_ATTR_PORT_IBDEV_NAME] = MNL_TYPE_NUL_STRING,
	[DEVLINK_ATTR_PORT_LANES] = MNL_TYPE_U32,
	[DEVLINK_ATTR_PORT_SPLITTABLE] = MNL_TYPE_U8,
	[DEVLINK_ATTR_SB_INDEX] = MNL_TYPE_U32,
	[DEVLINK_ATTR_SB_SIZE] = MNL_TYPE_U32,
	[DEVLINK_ATTR_SB_INGRESS_POOL_COUNT] = MNL_TYPE_U16,
	[DEVLINK_ATTR_SB_EGRESS_POOL_COUNT] = MNL_TYPE_U16,
	[DEVLINK_ATTR_SB_INGRESS_TC_COUNT] = MNL_TYPE_U16,
	[DEVLINK_ATTR_SB_EGRESS_TC_COUNT] = MNL_TYPE_U16,
	[DEVLINK_ATTR_SB_POOL_INDEX] = MNL_TYPE_U16,
	[DEVLINK_ATTR_SB_POOL_TYPE] = MNL_TYPE_U8,
	[DEVLINK_ATTR_SB_POOL_SIZE] = MNL_TYPE_U32,
	[DEVLINK_ATTR_SB_POOL_THRESHOLD_TYPE] = MNL_TYPE_U8,
	[DEVLINK_ATTR_SB_THRESHOLD] = MNL_TYPE_U32,
	[DEVLINK_ATTR_SB_TC_INDEX] = MNL_TYPE_U16,
	[DEVLINK_ATTR_SB_OCC_CUR] = MNL_TYPE_U32,
	[DEVLINK_ATTR_SB_OCC_MAX] = MNL_TYPE_U32,
	[DEVLINK_ATTR_ESWITCH_MODE] = MNL_TYPE_U16,
	[DEVLINK_ATTR_ESWITCH_INLINE_MODE] = MNL_TYPE_U8,
	[DEVLINK_ATTR_ESWITCH_ENCAP_MODE] = MNL_TYPE_U8,
	[DEVLINK_ATTR_DPIPE_TABLES] = MNL_TYPE_NESTED,
	[DEVLINK_ATTR_DPIPE_TABLE] = MNL_TYPE_NESTED,
	[DEVLINK_ATTR_DPIPE_TABLE_NAME] = MNL_TYPE_STRING,
	[DEVLINK_ATTR_DPIPE_TABLE_SIZE] = MNL_TYPE_U64,
	[DEVLINK_ATTR_DPIPE_TABLE_MATCHES] = MNL_TYPE_NESTED,
	[DEVLINK_ATTR_DPIPE_TABLE_ACTIONS] = MNL_TYPE_NESTED,
	[DEVLINK_ATTR_DPIPE_TABLE_COUNTERS_ENABLED] =  MNL_TYPE_U8,
	[DEVLINK_ATTR_DPIPE_ENTRIES] = MNL_TYPE_NESTED,
	[DEVLINK_ATTR_DPIPE_ENTRY] = MNL_TYPE_NESTED,
	[DEVLINK_ATTR_DPIPE_ENTRY_INDEX] = MNL_TYPE_U64,
	[DEVLINK_ATTR_DPIPE_ENTRY_MATCH_VALUES] = MNL_TYPE_NESTED,
	[DEVLINK_ATTR_DPIPE_ENTRY_ACTION_VALUES] = MNL_TYPE_NESTED,
	[DEVLINK_ATTR_DPIPE_ENTRY_COUNTER] = MNL_TYPE_U64,
	[DEVLINK_ATTR_DPIPE_MATCH] = MNL_TYPE_NESTED,
	[DEVLINK_ATTR_DPIPE_MATCH_VALUE] = MNL_TYPE_NESTED,
	[DEVLINK_ATTR_DPIPE_MATCH_TYPE] = MNL_TYPE_U32,
	[DEVLINK_ATTR_DPIPE_ACTION] = MNL_TYPE_NESTED,
	[DEVLINK_ATTR_DPIPE_ACTION_VALUE] = MNL_TYPE_NESTED,
	[DEVLINK_ATTR_DPIPE_ACTION_TYPE] = MNL_TYPE_U32,
	[DEVLINK_ATTR_DPIPE_VALUE_MAPPING] = MNL_TYPE_U32,
	[DEVLINK_ATTR_DPIPE_HEADERS] = MNL_TYPE_NESTED,
	[DEVLINK_ATTR_DPIPE_HEADER] = MNL_TYPE_NESTED,
	[DEVLINK_ATTR_DPIPE_HEADER_NAME] = MNL_TYPE_STRING,
	[DEVLINK_ATTR_DPIPE_HEADER_ID] = MNL_TYPE_U32,
	[DEVLINK_ATTR_DPIPE_HEADER_FIELDS] = MNL_TYPE_NESTED,
	[DEVLINK_ATTR_DPIPE_HEADER_GLOBAL] = MNL_TYPE_U8,
	[DEVLINK_ATTR_DPIPE_HEADER_INDEX] = MNL_TYPE_U32,
	[DEVLINK_ATTR_DPIPE_FIELD] = MNL_TYPE_NESTED,
	[DEVLINK_ATTR_DPIPE_FIELD_NAME] = MNL_TYPE_STRING,
	[DEVLINK_ATTR_DPIPE_FIELD_ID] = MNL_TYPE_U32,
	[DEVLINK_ATTR_DPIPE_FIELD_BITWIDTH] = MNL_TYPE_U32,
	[DEVLINK_ATTR_DPIPE_FIELD_MAPPING_TYPE] = MNL_TYPE_U32,
	[DEVLINK_ATTR_PARAM] = MNL_TYPE_NESTED,
	[DEVLINK_ATTR_PARAM_NAME] = MNL_TYPE_STRING,
	[DEVLINK_ATTR_PARAM_TYPE] = MNL_TYPE_U8,
	[DEVLINK_ATTR_PARAM_VALUES_LIST] = MNL_TYPE_NESTED,
	[DEVLINK_ATTR_PARAM_VALUE] = MNL_TYPE_NESTED,
	[DEVLINK_ATTR_PARAM_VALUE_CMODE] = MNL_TYPE_U8,
	[DEVLINK_ATTR_REGION_NAME] = MNL_TYPE_STRING,
	[DEVLINK_ATTR_REGION_SIZE] = MNL_TYPE_U64,
	[DEVLINK_ATTR_REGION_SNAPSHOTS] = MNL_TYPE_NESTED,
	[DEVLINK_ATTR_REGION_SNAPSHOT] = MNL_TYPE_NESTED,
	[DEVLINK_ATTR_REGION_SNAPSHOT_ID] = MNL_TYPE_U32,
	[DEVLINK_ATTR_REGION_CHUNKS] = MNL_TYPE_NESTED,
	[DEVLINK_ATTR_REGION_CHUNK] = MNL_TYPE_NESTED,
	[DEVLINK_ATTR_REGION_CHUNK_DATA] = MNL_TYPE_BINARY,
	[DEVLINK_ATTR_REGION_CHUNK_ADDR] = MNL_TYPE_U64,
	[DEVLINK_ATTR_REGION_CHUNK_LEN] = MNL_TYPE_U64,
	[DEVLINK_ATTR_INFO_DRIVER_NAME] = MNL_TYPE_STRING,
	[DEVLINK_ATTR_INFO_SERIAL_NUMBER] = MNL_TYPE_STRING,
	[DEVLINK_ATTR_INFO_BOARD_SERIAL_NUMBER] = MNL_TYPE_STRING,
	[DEVLINK_ATTR_INFO_VERSION_FIXED] = MNL_TYPE_NESTED,
	[DEVLINK_ATTR_INFO_VERSION_RUNNING] = MNL_TYPE_NESTED,
	[DEVLINK_ATTR_INFO_VERSION_STORED] = MNL_TYPE_NESTED,
	[DEVLINK_ATTR_INFO_VERSION_NAME] = MNL_TYPE_STRING,
	[DEVLINK_ATTR_INFO_VERSION_VALUE] = MNL_TYPE_STRING,
	[DEVLINK_ATTR_HEALTH_REPORTER] = MNL_TYPE_NESTED,
	[DEVLINK_ATTR_HEALTH_REPORTER_NAME] = MNL_TYPE_STRING,
	[DEVLINK_ATTR_HEALTH_REPORTER_STATE] = MNL_TYPE_U8,
	[DEVLINK_ATTR_HEALTH_REPORTER_ERR_COUNT] = MNL_TYPE_U64,
	[DEVLINK_ATTR_HEALTH_REPORTER_RECOVER_COUNT] = MNL_TYPE_U64,
	[DEVLINK_ATTR_HEALTH_REPORTER_DUMP_TS] = MNL_TYPE_U64,
	[DEVLINK_ATTR_HEALTH_REPORTER_GRACEFUL_PERIOD] = MNL_TYPE_U64,
	[DEVLINK_ATTR_FLASH_UPDATE_COMPONENT] = MNL_TYPE_STRING,
	[DEVLINK_ATTR_FLASH_UPDATE_STATUS_MSG] = MNL_TYPE_STRING,
	[DEVLINK_ATTR_FLASH_UPDATE_STATUS_DONE] = MNL_TYPE_U64,
	[DEVLINK_ATTR_FLASH_UPDATE_STATUS_TOTAL] = MNL_TYPE_U64,
	[DEVLINK_ATTR_STATS] = MNL_TYPE_NESTED,
	[DEVLINK_ATTR_TRAP_NAME] = MNL_TYPE_STRING,
	[DEVLINK_ATTR_TRAP_ACTION] = MNL_TYPE_U8,
	[DEVLINK_ATTR_TRAP_TYPE] = MNL_TYPE_U8,
	[DEVLINK_ATTR_TRAP_GENERIC] = MNL_TYPE_FLAG,
	[DEVLINK_ATTR_TRAP_METADATA] = MNL_TYPE_NESTED,
	[DEVLINK_ATTR_TRAP_GROUP_NAME] = MNL_TYPE_STRING,
	[DEVLINK_ATTR_RELOAD_FAILED] = MNL_TYPE_U8,
	[DEVLINK_ATTR_DEV_STATS] = MNL_TYPE_NESTED,
	[DEVLINK_ATTR_RELOAD_STATS] = MNL_TYPE_NESTED,
	[DEVLINK_ATTR_RELOAD_STATS_ENTRY] = MNL_TYPE_NESTED,
	[DEVLINK_ATTR_RELOAD_ACTION] = MNL_TYPE_U8,
	[DEVLINK_ATTR_RELOAD_STATS_LIMIT] = MNL_TYPE_U8,
	[DEVLINK_ATTR_RELOAD_STATS_VALUE] = MNL_TYPE_U32,
	[DEVLINK_ATTR_REMOTE_RELOAD_STATS] = MNL_TYPE_NESTED,
	[DEVLINK_ATTR_RELOAD_ACTION_INFO] = MNL_TYPE_NESTED,
	[DEVLINK_ATTR_RELOAD_ACTION_STATS] = MNL_TYPE_NESTED,
	[DEVLINK_ATTR_TRAP_POLICER_ID] = MNL_TYPE_U32,
	[DEVLINK_ATTR_TRAP_POLICER_RATE] = MNL_TYPE_U64,
	[DEVLINK_ATTR_TRAP_POLICER_BURST] = MNL_TYPE_U64,
};

static const enum mnl_attr_data_type
devlink_stats_policy[DEVLINK_ATTR_STATS_MAX + 1] = {
	[DEVLINK_ATTR_STATS_RX_PACKETS] = MNL_TYPE_U64,
	[DEVLINK_ATTR_STATS_RX_BYTES] = MNL_TYPE_U64,
	[DEVLINK_ATTR_STATS_RX_DROPPED] = MNL_TYPE_U64,
};

static int attr_cb(const struct nlattr *attr, void *data)
{
	const struct nlattr **tb = data;
	int type;

	if (mnl_attr_type_valid(attr, DEVLINK_ATTR_MAX) < 0)
		return MNL_CB_OK;

	type = mnl_attr_get_type(attr);
	if (mnl_attr_validate(attr, devlink_policy[type]) < 0)
		return MNL_CB_ERROR;

	tb[type] = attr;
	return MNL_CB_OK;
}

static int attr_stats_cb(const struct nlattr *attr, void *data)
{
	const struct nlattr **tb = data;
	int type;

	/* Allow the tool to work on top of newer kernels that might contain
	 * more attributes.
	 */
	if (mnl_attr_type_valid(attr, DEVLINK_ATTR_STATS_MAX) < 0)
		return MNL_CB_OK;

	type = mnl_attr_get_type(attr);
	if (mnl_attr_validate(attr, devlink_stats_policy[type]) < 0)
		return MNL_CB_ERROR;

	tb[type] = attr;
	return MNL_CB_OK;
}

static const enum mnl_attr_data_type
devlink_function_policy[DEVLINK_PORT_FUNCTION_ATTR_MAX + 1] = {
	[DEVLINK_PORT_FUNCTION_ATTR_HW_ADDR ] = MNL_TYPE_BINARY,
	[DEVLINK_PORT_FN_ATTR_STATE] = MNL_TYPE_U8,
};

static int function_attr_cb(const struct nlattr *attr, void *data)
{
	const struct nlattr **tb = data;
	int type;

	/* Allow the tool to work on top of newer kernels that might contain
	 * more attributes.
	 */
	if (mnl_attr_type_valid(attr, DEVLINK_PORT_FUNCTION_ATTR_MAX) < 0)
		return MNL_CB_OK;

	type = mnl_attr_get_type(attr);
	if (mnl_attr_validate(attr, devlink_function_policy[type]) < 0)
		return MNL_CB_ERROR;

	tb[type] = attr;
	return MNL_CB_OK;
}

static int ifname_map_cb(const struct nlmsghdr *nlh, void *data)
{
	struct nlattr *tb[DEVLINK_ATTR_MAX + 1] = {};
	struct genlmsghdr *genl = mnl_nlmsg_get_payload(nlh);
	struct dl *dl = data;
	struct ifname_map *ifname_map;
	const char *bus_name;
	const char *dev_name;
	uint32_t port_ifindex;
	const char *port_ifname;

	mnl_attr_parse(nlh, sizeof(*genl), attr_cb, tb);
	if (!tb[DEVLINK_ATTR_BUS_NAME] || !tb[DEVLINK_ATTR_DEV_NAME] ||
	    !tb[DEVLINK_ATTR_PORT_INDEX])
		return MNL_CB_ERROR;

	if (!tb[DEVLINK_ATTR_PORT_NETDEV_NAME])
		return MNL_CB_OK;

	bus_name = mnl_attr_get_str(tb[DEVLINK_ATTR_BUS_NAME]);
	dev_name = mnl_attr_get_str(tb[DEVLINK_ATTR_DEV_NAME]);
	port_ifindex = mnl_attr_get_u32(tb[DEVLINK_ATTR_PORT_INDEX]);
	port_ifname = mnl_attr_get_str(tb[DEVLINK_ATTR_PORT_NETDEV_NAME]);
	ifname_map = ifname_map_alloc(bus_name, dev_name,
				      port_ifindex, port_ifname);
	if (!ifname_map)
		return MNL_CB_ERROR;
	list_add(&ifname_map->list, &dl->ifname_map_list);

	return MNL_CB_OK;
}

static void ifname_map_fini(struct dl *dl)
{
	struct ifname_map *ifname_map, *tmp;

	list_for_each_entry_safe(ifname_map, tmp,
				 &dl->ifname_map_list, list) {
		list_del(&ifname_map->list);
		ifname_map_free(ifname_map);
	}
}

static int ifname_map_init(struct dl *dl)
{
	struct nlmsghdr *nlh;
	int err;

	INIT_LIST_HEAD(&dl->ifname_map_list);

	nlh = mnlg_msg_prepare(dl->nlg, DEVLINK_CMD_PORT_GET,
			       NLM_F_REQUEST | NLM_F_ACK | NLM_F_DUMP);

	err = _mnlg_socket_sndrcv(dl->nlg, nlh, ifname_map_cb, dl);
	if (err) {
		ifname_map_fini(dl);
		return err;
	}
	return 0;
}

static int ifname_map_lookup(struct dl *dl, const char *ifname,
			     char **p_bus_name, char **p_dev_name,
			     uint32_t *p_port_index)
{
	struct ifname_map *ifname_map;

	list_for_each_entry(ifname_map, &dl->ifname_map_list, list) {
		if (strcmp(ifname, ifname_map->ifname) == 0) {
			*p_bus_name = ifname_map->bus_name;
			*p_dev_name = ifname_map->dev_name;
			*p_port_index = ifname_map->port_index;
			return 0;
		}
	}
	return -ENOENT;
}

static int ifname_map_rev_lookup(struct dl *dl, const char *bus_name,
				 const char *dev_name, uint32_t port_index,
				 char **p_ifname)
{
	struct ifname_map *ifname_map;

	list_for_each_entry(ifname_map, &dl->ifname_map_list, list) {
		if (strcmp(bus_name, ifname_map->bus_name) == 0 &&
		    strcmp(dev_name, ifname_map->dev_name) == 0 &&
		    port_index == ifname_map->port_index) {
			*p_ifname = ifname_map->ifname;
			return 0;
		}
	}
	return -ENOENT;
}

static unsigned int strslashcount(char *str)
{
	unsigned int count = 0;
	char *pos = str;

	while ((pos = strchr(pos, '/'))) {
		count++;
		pos++;
	}
	return count;
}

static int strslashrsplit(char *str, char **before, char **after)
{
	char *slash;

	slash = strrchr(str, '/');
	if (!slash)
		return -EINVAL;
	*slash = '\0';
	*before = str;
	*after = slash + 1;
	return 0;
}

static int strtouint64_t(const char *str, uint64_t *p_val)
{
	char *endptr;
	unsigned long long int val;

	val = strtoull(str, &endptr, 10);
	if (endptr == str || *endptr != '\0')
		return -EINVAL;
	if (val > ULONG_MAX)
		return -ERANGE;
	*p_val = val;
	return 0;
}

static int strtouint32_t(const char *str, uint32_t *p_val)
{
	char *endptr;
	unsigned long int val;

	val = strtoul(str, &endptr, 10);
	if (endptr == str || *endptr != '\0')
		return -EINVAL;
	if (val > UINT_MAX)
		return -ERANGE;
	*p_val = val;
	return 0;
}

static int strtouint16_t(const char *str, uint16_t *p_val)
{
	char *endptr;
	unsigned long int val;

	val = strtoul(str, &endptr, 10);
	if (endptr == str || *endptr != '\0')
		return -EINVAL;
	if (val > USHRT_MAX)
		return -ERANGE;
	*p_val = val;
	return 0;
}

static int strtouint8_t(const char *str, uint8_t *p_val)
{
	char *endptr;
	unsigned long int val;

	val = strtoul(str, &endptr, 10);
	if (endptr == str || *endptr != '\0')
		return -EINVAL;
	if (val > UCHAR_MAX)
		return -ERANGE;
	*p_val = val;
	return 0;
}

static int strtobool(const char *str, bool *p_val)
{
	bool val;

	if (!strcmp(str, "true") || !strcmp(str, "1") ||
	    !strcmp(str, "enable"))
		val = true;
	else if (!strcmp(str, "false") || !strcmp(str, "0") ||
		 !strcmp(str, "disable"))
		val = false;
	else
		return -EINVAL;
	*p_val = val;
	return 0;
}

static int __dl_argv_handle(char *str, char **p_bus_name, char **p_dev_name)
{
	strslashrsplit(str, p_bus_name, p_dev_name);
	return 0;
}

static int dl_argv_handle(struct dl *dl, char **p_bus_name, char **p_dev_name)
{
	char *str = dl_argv_next(dl);

	if (!str) {
		pr_err("Devlink identification (\"bus_name/dev_name\") expected\n");
		return -EINVAL;
	}
	if (strslashcount(str) != 1) {
		pr_err("Wrong devlink identification string format.\n");
		pr_err("Expected \"bus_name/dev_name\".\n");
		return -EINVAL;
	}
	return __dl_argv_handle(str, p_bus_name, p_dev_name);
}

static int __dl_argv_handle_port(char *str,
				 char **p_bus_name, char **p_dev_name,
				 uint32_t *p_port_index)
{
	char *handlestr;
	char *portstr;
	int err;

	err = strslashrsplit(str, &handlestr, &portstr);
	if (err) {
		pr_err("Port identification \"%s\" is invalid\n", str);
		return err;
	}
	err = strtouint32_t(portstr, p_port_index);
	if (err) {
		pr_err("Port index \"%s\" is not a number or not within range\n",
		       portstr);
		return err;
	}
	err = strslashrsplit(handlestr, p_bus_name, p_dev_name);
	if (err) {
		pr_err("Port identification \"%s\" is invalid\n", str);
		return err;
	}
	return 0;
}

static int __dl_argv_handle_port_ifname(struct dl *dl, char *str,
					char **p_bus_name, char **p_dev_name,
					uint32_t *p_port_index)
{
	int err;

	err = ifname_map_lookup(dl, str, p_bus_name, p_dev_name,
				p_port_index);
	if (err) {
		pr_err("Netdevice \"%s\" not found\n", str);
		return err;
	}
	return 0;
}

static int dl_argv_handle_port(struct dl *dl, char **p_bus_name,
			       char **p_dev_name, uint32_t *p_port_index)
{
	char *str = dl_argv_next(dl);
	unsigned int slash_count;

	if (!str) {
		pr_err("Port identification (\"bus_name/dev_name/port_index\" or \"netdev ifname\") expected.\n");
		return -EINVAL;
	}
	slash_count = strslashcount(str);
	switch (slash_count) {
	case 0:
		return __dl_argv_handle_port_ifname(dl, str, p_bus_name,
						    p_dev_name, p_port_index);
	case 2:
		return __dl_argv_handle_port(str, p_bus_name,
					     p_dev_name, p_port_index);
	default:
		pr_err("Wrong port identification string format.\n");
		pr_err("Expected \"bus_name/dev_name/port_index\" or \"netdev_ifname\".\n");
		return -EINVAL;
	}
}

static int dl_argv_handle_both(struct dl *dl, char **p_bus_name,
			       char **p_dev_name, uint32_t *p_port_index,
			       uint64_t *p_handle_bit)
{
	char *str = dl_argv_next(dl);
	unsigned int slash_count;
	int err;

	if (!str) {
		pr_err("One of following identifications expected:\n"
		       "Devlink identification (\"bus_name/dev_name\")\n"
		       "Port identification (\"bus_name/dev_name/port_index\" or \"netdev ifname\")\n");
		return -EINVAL;
	}
	slash_count = strslashcount(str);
	if (slash_count == 1) {
		err = __dl_argv_handle(str, p_bus_name, p_dev_name);
		if (err)
			return err;
		*p_handle_bit = DL_OPT_HANDLE;
	} else if (slash_count == 2) {
		err = __dl_argv_handle_port(str, p_bus_name,
					    p_dev_name, p_port_index);
		if (err)
			return err;
		*p_handle_bit = DL_OPT_HANDLEP;
	} else if (slash_count == 0) {
		err = __dl_argv_handle_port_ifname(dl, str, p_bus_name,
						   p_dev_name, p_port_index);
		if (err)
			return err;
		*p_handle_bit = DL_OPT_HANDLEP;
	} else {
		pr_err("Wrong port identification string format.\n");
		pr_err("Expected \"bus_name/dev_name\" or \"bus_name/dev_name/port_index\" or \"netdev_ifname\".\n");
		return -EINVAL;
	}
	return 0;
}

static int __dl_argv_handle_region(char *str, char **p_bus_name,
				   char **p_dev_name, char **p_region)
{
	char *handlestr;
	int err;

	err = strslashrsplit(str, &handlestr, p_region);
	if (err) {
		pr_err("Region identification \"%s\" is invalid\n", str);
		return err;
	}
	err = strslashrsplit(handlestr, p_bus_name, p_dev_name);
	if (err) {
		pr_err("Region identification \"%s\" is invalid\n", str);
		return err;
	}
	return 0;
}

static int dl_argv_handle_region(struct dl *dl, char **p_bus_name,
					char **p_dev_name, char **p_region)
{
	char *str = dl_argv_next(dl);
	unsigned int slash_count;

	if (!str) {
		pr_err("Expected \"bus_name/dev_name/region\" identification.\n");
		return -EINVAL;
	}

	slash_count = strslashcount(str);
	if (slash_count != 2) {
		pr_err("Wrong region identification string format.\n");
		pr_err("Expected \"bus_name/dev_name/region\" identification.\n"".\n");
		return -EINVAL;
	}

	return __dl_argv_handle_region(str, p_bus_name, p_dev_name, p_region);
}

static int dl_argv_uint64_t(struct dl *dl, uint64_t *p_val)
{
	char *str = dl_argv_next(dl);
	int err;

	if (!str) {
		pr_err("Unsigned number argument expected\n");
		return -EINVAL;
	}

	err = strtouint64_t(str, p_val);
	if (err) {
		pr_err("\"%s\" is not a number or not within range\n", str);
		return err;
	}
	return 0;
}

static int dl_argv_uint32_t(struct dl *dl, uint32_t *p_val)
{
	char *str = dl_argv_next(dl);
	int err;

	if (!str) {
		pr_err("Unsigned number argument expected\n");
		return -EINVAL;
	}

	err = strtouint32_t(str, p_val);
	if (err) {
		pr_err("\"%s\" is not a number or not within range\n", str);
		return err;
	}
	return 0;
}

static int dl_argv_uint16_t(struct dl *dl, uint16_t *p_val)
{
	char *str = dl_argv_next(dl);
	int err;

	if (!str) {
		pr_err("Unsigned number argument expected\n");
		return -EINVAL;
	}

	err = strtouint16_t(str, p_val);
	if (err) {
		pr_err("\"%s\" is not a number or not within range\n", str);
		return err;
	}
	return 0;
}

static int dl_argv_bool(struct dl *dl, bool *p_val)
{
	char *str = dl_argv_next(dl);
	int err;

	if (!str) {
		pr_err("Boolean argument expected\n");
		return -EINVAL;
	}

	err = strtobool(str, p_val);
	if (err) {
		pr_err("\"%s\" is not a valid boolean value\n", str);
		return err;
	}
	return 0;
}

static int dl_argv_str(struct dl *dl, const char **p_str)
{
	const char *str = dl_argv_next(dl);

	if (!str) {
		pr_err("String parameter expected\n");
		return -EINVAL;
	}
	*p_str = str;
	return 0;
}

static int port_type_get(const char *typestr, enum devlink_port_type *p_type)
{
	if (strcmp(typestr, "auto") == 0) {
		*p_type = DEVLINK_PORT_TYPE_AUTO;
	} else if (strcmp(typestr, "eth") == 0) {
		*p_type = DEVLINK_PORT_TYPE_ETH;
	} else if (strcmp(typestr, "ib") == 0) {
		*p_type = DEVLINK_PORT_TYPE_IB;
	} else {
		pr_err("Unknown port type \"%s\"\n", typestr);
		return -EINVAL;
	}
	return 0;
}

static int pool_type_get(const char *typestr, enum devlink_sb_pool_type *p_type)
{
	if (strcmp(typestr, "ingress") == 0) {
		*p_type = DEVLINK_SB_POOL_TYPE_INGRESS;
	} else if (strcmp(typestr, "egress") == 0) {
		*p_type = DEVLINK_SB_POOL_TYPE_EGRESS;
	} else {
		pr_err("Unknown pool type \"%s\"\n", typestr);
		return -EINVAL;
	}
	return 0;
}

static int threshold_type_get(const char *typestr,
			      enum devlink_sb_threshold_type *p_type)
{
	if (strcmp(typestr, "static") == 0) {
		*p_type = DEVLINK_SB_THRESHOLD_TYPE_STATIC;
	} else if (strcmp(typestr, "dynamic") == 0) {
		*p_type = DEVLINK_SB_THRESHOLD_TYPE_DYNAMIC;
	} else {
		pr_err("Unknown threshold type \"%s\"\n", typestr);
		return -EINVAL;
	}
	return 0;
}

static int eswitch_mode_get(const char *typestr,
			    enum devlink_eswitch_mode *p_mode)
{
	if (strcmp(typestr, ESWITCH_MODE_LEGACY) == 0) {
		*p_mode = DEVLINK_ESWITCH_MODE_LEGACY;
	} else if (strcmp(typestr, ESWITCH_MODE_SWITCHDEV) == 0) {
		*p_mode = DEVLINK_ESWITCH_MODE_SWITCHDEV;
	} else {
		pr_err("Unknown eswitch mode \"%s\"\n", typestr);
		return -EINVAL;
	}
	return 0;
}

static int eswitch_inline_mode_get(const char *typestr,
				   enum devlink_eswitch_inline_mode *p_mode)
{
	if (strcmp(typestr, ESWITCH_INLINE_MODE_NONE) == 0) {
		*p_mode = DEVLINK_ESWITCH_INLINE_MODE_NONE;
	} else if (strcmp(typestr, ESWITCH_INLINE_MODE_LINK) == 0) {
		*p_mode = DEVLINK_ESWITCH_INLINE_MODE_LINK;
	} else if (strcmp(typestr, ESWITCH_INLINE_MODE_NETWORK) == 0) {
		*p_mode = DEVLINK_ESWITCH_INLINE_MODE_NETWORK;
	} else if (strcmp(typestr, ESWITCH_INLINE_MODE_TRANSPORT) == 0) {
		*p_mode = DEVLINK_ESWITCH_INLINE_MODE_TRANSPORT;
	} else {
		pr_err("Unknown eswitch inline mode \"%s\"\n", typestr);
		return -EINVAL;
	}
	return 0;
}

static int
eswitch_encap_mode_get(const char *typestr,
		       enum devlink_eswitch_encap_mode *p_encap_mode)
{
	/* The initial implementation incorrectly accepted "enable"/"disable".
	 * Carry it to maintain backward compatibility.
	 */
	if (strcmp(typestr, "disable") == 0 ||
		   strcmp(typestr, ESWITCH_ENCAP_MODE_NONE) == 0) {
		*p_encap_mode = DEVLINK_ESWITCH_ENCAP_MODE_NONE;
	} else if (strcmp(typestr, "enable") == 0 ||
		   strcmp(typestr, ESWITCH_ENCAP_MODE_BASIC) == 0) {
		*p_encap_mode = DEVLINK_ESWITCH_ENCAP_MODE_BASIC;
	} else {
		pr_err("Unknown eswitch encap mode \"%s\"\n", typestr);
		return -EINVAL;
	}
	return 0;
}

static int flash_overwrite_section_get(const char *sectionstr, uint32_t *mask)
{
	if (strcmp(sectionstr, "settings") == 0) {
		*mask |= DEVLINK_FLASH_OVERWRITE_SETTINGS;
	} else if (strcmp(sectionstr, "identifiers") == 0) {
		*mask |= DEVLINK_FLASH_OVERWRITE_IDENTIFIERS;
	} else {
		pr_err("Unknown overwrite section \"%s\"\n", sectionstr);
		return -EINVAL;
	}
	return 0;
}

static int param_cmode_get(const char *cmodestr,
			   enum devlink_param_cmode *cmode)
{
	if (strcmp(cmodestr, PARAM_CMODE_RUNTIME_STR) == 0) {
		*cmode = DEVLINK_PARAM_CMODE_RUNTIME;
	} else if (strcmp(cmodestr, PARAM_CMODE_DRIVERINIT_STR) == 0) {
		*cmode = DEVLINK_PARAM_CMODE_DRIVERINIT;
	} else if (strcmp(cmodestr, PARAM_CMODE_PERMANENT_STR) == 0) {
		*cmode = DEVLINK_PARAM_CMODE_PERMANENT;
	} else {
		pr_err("Unknown configuration mode \"%s\"\n", cmodestr);
		return -EINVAL;
	}
	return 0;
}

static int trap_action_get(const char *actionstr,
			   enum devlink_trap_action *p_action)
{
	if (strcmp(actionstr, "drop") == 0) {
		*p_action = DEVLINK_TRAP_ACTION_DROP;
	} else if (strcmp(actionstr, "trap") == 0) {
		*p_action = DEVLINK_TRAP_ACTION_TRAP;
	} else if (strcmp(actionstr, "mirror") == 0) {
		*p_action = DEVLINK_TRAP_ACTION_MIRROR;
	} else {
		pr_err("Unknown trap action \"%s\"\n", actionstr);
		return -EINVAL;
	}
	return 0;
}

static int hw_addr_parse(const char *addrstr, char *hw_addr, uint32_t *len)
{
	int alen;

	alen = ll_addr_a2n(hw_addr, MAX_ADDR_LEN, addrstr);
	if (alen < 0)
		return -EINVAL;
	*len = alen;
	return 0;
}

static int reload_action_get(struct dl *dl, const char *actionstr,
			     enum devlink_reload_action *action)
{
	if (strcmp(actionstr, "driver_reinit") == 0) {
		*action = DEVLINK_RELOAD_ACTION_DRIVER_REINIT;
	} else if (strcmp(actionstr, "fw_activate") == 0) {
		*action = DEVLINK_RELOAD_ACTION_FW_ACTIVATE;
	} else {
		pr_err("Unknown reload action \"%s\"\n", actionstr);
		return -EINVAL;
	}
	return 0;
}

static int reload_limit_get(struct dl *dl, const char *limitstr,
			     enum devlink_reload_limit *limit)
{
	if (strcmp(limitstr, "no_reset") == 0) {
		*limit = DEVLINK_RELOAD_LIMIT_NO_RESET;
	} else {
		pr_err("Unknown reload limit \"%s\"\n", limitstr);
		return -EINVAL;
	}
	return 0;
}

static struct str_num_map port_flavour_map[] = {
	{ .str = "physical", .num = DEVLINK_PORT_FLAVOUR_PHYSICAL },
	{ .str = "cpu", .num = DEVLINK_PORT_FLAVOUR_CPU },
	{ .str = "dsa", .num = DEVLINK_PORT_FLAVOUR_DSA },
	{ .str = "pcipf", .num = DEVLINK_PORT_FLAVOUR_PCI_PF },
	{ .str = "pcivf", .num = DEVLINK_PORT_FLAVOUR_PCI_VF },
	{ .str = "pcisf", .num = DEVLINK_PORT_FLAVOUR_PCI_SF },
	{ .str = "virtual", .num = DEVLINK_PORT_FLAVOUR_VIRTUAL},
	{ .str = NULL, },
};

static struct str_num_map port_fn_state_map[] = {
	{ .str = "inactive", .num = DEVLINK_PORT_FN_STATE_INACTIVE},
	{ .str = "active", .num = DEVLINK_PORT_FN_STATE_ACTIVE },
	{ .str = NULL, }
};

static struct str_num_map port_fn_opstate_map[] = {
	{ .str = "attached", .num = DEVLINK_PORT_FN_OPSTATE_ATTACHED},
	{ .str = "detached", .num = DEVLINK_PORT_FN_OPSTATE_DETACHED},
	{ .str = NULL, }
};

static int port_flavour_parse(const char *flavour, uint16_t *value)
{
	int num;

	num = str_map_lookup_str(port_flavour_map, flavour);
	if (num < 0)
		return num;
	*value = num;
	return 0;
}

static int port_fn_state_parse(const char *statestr, uint8_t *state)
{
	int num;

	num = str_map_lookup_str(port_fn_state_map, statestr);
	if (num < 0)
		return num;
	*state = num;
	return 0;
}

struct dl_args_metadata {
	uint64_t o_flag;
	char err_msg[DL_ARGS_REQUIRED_MAX_ERR_LEN];
};

static const struct dl_args_metadata dl_args_required[] = {
	{DL_OPT_PORT_TYPE,	      "Port type not set."},
	{DL_OPT_PORT_COUNT,	      "Port split count option expected."},
	{DL_OPT_SB_POOL,	      "Pool index option expected."},
	{DL_OPT_SB_SIZE,	      "Pool size option expected."},
	{DL_OPT_SB_TYPE,	      "Pool type option expected."},
	{DL_OPT_SB_THTYPE,	      "Pool threshold type option expected."},
	{DL_OPT_SB_TH,		      "Threshold option expected."},
	{DL_OPT_SB_TC,		      "TC index option expected."},
	{DL_OPT_ESWITCH_MODE,	      "E-Switch mode option expected."},
	{DL_OPT_ESWITCH_INLINE_MODE,  "E-Switch inline-mode option expected."},
	{DL_OPT_DPIPE_TABLE_NAME,     "Dpipe table name expected."},
	{DL_OPT_DPIPE_TABLE_COUNTERS, "Dpipe table counter state expected."},
	{DL_OPT_ESWITCH_ENCAP_MODE,   "E-Switch encapsulation option expected."},
	{DL_OPT_RESOURCE_PATH,	      "Resource path expected."},
	{DL_OPT_RESOURCE_SIZE,	      "Resource size expected."},
	{DL_OPT_PARAM_NAME,	      "Parameter name expected."},
	{DL_OPT_PARAM_VALUE,	      "Value to set expected."},
	{DL_OPT_PARAM_CMODE,	      "Configuration mode expected."},
	{DL_OPT_REGION_SNAPSHOT_ID,   "Region snapshot id expected."},
	{DL_OPT_REGION_ADDRESS,	      "Region address value expected."},
	{DL_OPT_REGION_LENGTH,	      "Region length value expected."},
	{DL_OPT_HEALTH_REPORTER_NAME, "Reporter's name is expected."},
	{DL_OPT_TRAP_NAME,            "Trap's name is expected."},
	{DL_OPT_TRAP_GROUP_NAME,      "Trap group's name is expected."},
	{DL_OPT_PORT_FUNCTION_HW_ADDR, "Port function's hardware address is expected."},
	{DL_OPT_PORT_FLAVOUR,          "Port flavour is expected."},
	{DL_OPT_PORT_PFNUMBER,         "Port PCI PF number is expected."},
};

static int dl_args_finding_required_validate(uint64_t o_required,
					     uint64_t o_found)
{
	uint64_t o_flag;
	int i;

	for (i = 0; i < ARRAY_SIZE(dl_args_required); i++) {
		o_flag = dl_args_required[i].o_flag;
		if ((o_required & o_flag) && !(o_found & o_flag)) {
			pr_err("%s\n", dl_args_required[i].err_msg);
			return -EINVAL;
		}
	}
	if (o_required & ~o_found) {
		pr_err("BUG: unknown argument required but not found\n");
		return -EINVAL;
	}
	return 0;
}

static int dl_argv_parse(struct dl *dl, uint64_t o_required,
			 uint64_t o_optional)
{
	struct dl_opts *opts = &dl->opts;
	uint64_t o_all = o_required | o_optional;
	uint64_t o_found = 0;
	int err;

	if (o_required & DL_OPT_HANDLE && o_required & DL_OPT_HANDLEP) {
		uint64_t handle_bit;

		err = dl_argv_handle_both(dl, &opts->bus_name, &opts->dev_name,
					  &opts->port_index, &handle_bit);
		if (err)
			return err;
		o_required &= ~(DL_OPT_HANDLE | DL_OPT_HANDLEP) | handle_bit;
		o_found |= handle_bit;
	} else if (o_required & DL_OPT_HANDLE) {
		err = dl_argv_handle(dl, &opts->bus_name, &opts->dev_name);
		if (err)
			return err;
		o_found |= DL_OPT_HANDLE;
	} else if (o_required & DL_OPT_HANDLEP) {
		err = dl_argv_handle_port(dl, &opts->bus_name, &opts->dev_name,
					  &opts->port_index);
		if (err)
			return err;
		o_found |= DL_OPT_HANDLEP;
	} else if (o_required & DL_OPT_HANDLE_REGION) {
		err = dl_argv_handle_region(dl, &opts->bus_name,
					    &opts->dev_name,
					    &opts->region_name);
		if (err)
			return err;
		o_found |= DL_OPT_HANDLE_REGION;
	}

	while (dl_argc(dl)) {
		if (dl_argv_match(dl, "type") &&
		    (o_all & DL_OPT_PORT_TYPE)) {
			const char *typestr;

			dl_arg_inc(dl);
			err = dl_argv_str(dl, &typestr);
			if (err)
				return err;
			err = port_type_get(typestr, &opts->port_type);
			if (err)
				return err;
			o_found |= DL_OPT_PORT_TYPE;
		} else if (dl_argv_match(dl, "count") &&
			   (o_all & DL_OPT_PORT_COUNT)) {
			dl_arg_inc(dl);
			err = dl_argv_uint32_t(dl, &opts->port_count);
			if (err)
				return err;
			o_found |= DL_OPT_PORT_COUNT;
		} else if (dl_argv_match(dl, "sb") &&
			   (o_all & DL_OPT_SB)) {
			dl_arg_inc(dl);
			err = dl_argv_uint32_t(dl, &opts->sb_index);
			if (err)
				return err;
			o_found |= DL_OPT_SB;
		} else if (dl_argv_match(dl, "pool") &&
			   (o_all & DL_OPT_SB_POOL)) {
			dl_arg_inc(dl);
			err = dl_argv_uint16_t(dl, &opts->sb_pool_index);
			if (err)
				return err;
			o_found |= DL_OPT_SB_POOL;
		} else if (dl_argv_match(dl, "size") &&
			   (o_all & DL_OPT_SB_SIZE)) {
			dl_arg_inc(dl);
			err = dl_argv_uint32_t(dl, &opts->sb_pool_size);
			if (err)
				return err;
			o_found |= DL_OPT_SB_SIZE;
		} else if (dl_argv_match(dl, "type") &&
			   (o_all & DL_OPT_SB_TYPE)) {
			const char *typestr;

			dl_arg_inc(dl);
			err = dl_argv_str(dl, &typestr);
			if (err)
				return err;
			err = pool_type_get(typestr, &opts->sb_pool_type);
			if (err)
				return err;
			o_found |= DL_OPT_SB_TYPE;
		} else if (dl_argv_match(dl, "thtype") &&
			   (o_all & DL_OPT_SB_THTYPE)) {
			const char *typestr;

			dl_arg_inc(dl);
			err = dl_argv_str(dl, &typestr);
			if (err)
				return err;
			err = threshold_type_get(typestr,
						 &opts->sb_pool_thtype);
			if (err)
				return err;
			o_found |= DL_OPT_SB_THTYPE;
		} else if (dl_argv_match(dl, "th") &&
			   (o_all & DL_OPT_SB_TH)) {
			dl_arg_inc(dl);
			err = dl_argv_uint32_t(dl, &opts->sb_threshold);
			if (err)
				return err;
			o_found |= DL_OPT_SB_TH;
		} else if (dl_argv_match(dl, "tc") &&
			   (o_all & DL_OPT_SB_TC)) {
			dl_arg_inc(dl);
			err = dl_argv_uint16_t(dl, &opts->sb_tc_index);
			if (err)
				return err;
			o_found |= DL_OPT_SB_TC;
		} else if (dl_argv_match(dl, "mode") &&
			   (o_all & DL_OPT_ESWITCH_MODE)) {
			const char *typestr;

			dl_arg_inc(dl);
			err = dl_argv_str(dl, &typestr);
			if (err)
				return err;
			err = eswitch_mode_get(typestr, &opts->eswitch_mode);
			if (err)
				return err;
			o_found |= DL_OPT_ESWITCH_MODE;
		} else if (dl_argv_match(dl, "inline-mode") &&
			   (o_all & DL_OPT_ESWITCH_INLINE_MODE)) {
			const char *typestr;

			dl_arg_inc(dl);
			err = dl_argv_str(dl, &typestr);
			if (err)
				return err;
			err = eswitch_inline_mode_get(
				typestr, &opts->eswitch_inline_mode);
			if (err)
				return err;
			o_found |= DL_OPT_ESWITCH_INLINE_MODE;
		} else if (dl_argv_match(dl, "name") &&
			   (o_all & DL_OPT_DPIPE_TABLE_NAME)) {
			dl_arg_inc(dl);
			err = dl_argv_str(dl, &opts->dpipe_table_name);
			if (err)
				return err;
			o_found |= DL_OPT_DPIPE_TABLE_NAME;
		} else if ((dl_argv_match(dl, "counters") ||
			    dl_argv_match(dl, "counters_enabled")) &&
			   (o_all & DL_OPT_DPIPE_TABLE_COUNTERS)) {
			dl_arg_inc(dl);
			err = dl_argv_bool(dl, &opts->dpipe_counters_enabled);
			if (err)
				return err;
			o_found |= DL_OPT_DPIPE_TABLE_COUNTERS;
		} else if ((dl_argv_match(dl, "encap") || /* Original incorrect implementation */
			    dl_argv_match(dl, "encap-mode")) &&
			   (o_all & DL_OPT_ESWITCH_ENCAP_MODE)) {
			const char *typestr;

			dl_arg_inc(dl);
			err = dl_argv_str(dl, &typestr);
			if (err)
				return err;
			err = eswitch_encap_mode_get(typestr,
						     &opts->eswitch_encap_mode);
			if (err)
				return err;
			o_found |= DL_OPT_ESWITCH_ENCAP_MODE;
		} else if (dl_argv_match(dl, "path") &&
			   (o_all & DL_OPT_RESOURCE_PATH)) {
			dl_arg_inc(dl);
			err = dl_argv_str(dl, &opts->resource_path);
			if (err)
				return err;
			o_found |= DL_OPT_RESOURCE_PATH;
		} else if (dl_argv_match(dl, "size") &&
			   (o_all & DL_OPT_RESOURCE_SIZE)) {
			dl_arg_inc(dl);
			err = dl_argv_uint64_t(dl, &opts->resource_size);
			if (err)
				return err;
			o_found |= DL_OPT_RESOURCE_SIZE;
		} else if (dl_argv_match(dl, "name") &&
			   (o_all & DL_OPT_PARAM_NAME)) {
			dl_arg_inc(dl);
			err = dl_argv_str(dl, &opts->param_name);
			if (err)
				return err;
			o_found |= DL_OPT_PARAM_NAME;
		} else if (dl_argv_match(dl, "value") &&
			   (o_all & DL_OPT_PARAM_VALUE)) {
			dl_arg_inc(dl);
			err = dl_argv_str(dl, &opts->param_value);
			if (err)
				return err;
			o_found |= DL_OPT_PARAM_VALUE;
		} else if (dl_argv_match(dl, "cmode") &&
			   (o_all & DL_OPT_PARAM_CMODE)) {
			const char *cmodestr;

			dl_arg_inc(dl);
			err = dl_argv_str(dl, &cmodestr);
			if (err)
				return err;
			err = param_cmode_get(cmodestr, &opts->cmode);
			if (err)
				return err;
			o_found |= DL_OPT_PARAM_CMODE;
		} else if (dl_argv_match(dl, "snapshot") &&
			   (o_all & DL_OPT_REGION_SNAPSHOT_ID)) {
			dl_arg_inc(dl);
			err = dl_argv_uint32_t(dl, &opts->region_snapshot_id);
			if (err)
				return err;
			o_found |= DL_OPT_REGION_SNAPSHOT_ID;
		} else if (dl_argv_match(dl, "address") &&
			   (o_all & DL_OPT_REGION_ADDRESS)) {
			dl_arg_inc(dl);
			err = dl_argv_uint64_t(dl, &opts->region_address);
			if (err)
				return err;
			o_found |= DL_OPT_REGION_ADDRESS;
		} else if (dl_argv_match(dl, "length") &&
			   (o_all & DL_OPT_REGION_LENGTH)) {
			dl_arg_inc(dl);
			err = dl_argv_uint64_t(dl, &opts->region_length);
			if (err)
				return err;
			o_found |= DL_OPT_REGION_LENGTH;
		} else if (dl_argv_match(dl, "file") &&
			   (o_all & DL_OPT_FLASH_FILE_NAME)) {
			dl_arg_inc(dl);
			err = dl_argv_str(dl, &opts->flash_file_name);
			if (err)
				return err;
			o_found |= DL_OPT_FLASH_FILE_NAME;
		} else if (dl_argv_match(dl, "component") &&
			   (o_all & DL_OPT_FLASH_COMPONENT)) {
			dl_arg_inc(dl);
			err = dl_argv_str(dl, &opts->flash_component);
			if (err)
				return err;
			o_found |= DL_OPT_FLASH_COMPONENT;

		} else if (dl_argv_match(dl, "overwrite") &&
				(o_all & DL_OPT_FLASH_OVERWRITE)) {
			const char *sectionstr;

			dl_arg_inc(dl);
			err = dl_argv_str(dl, &sectionstr);
			if(err)
				return err;
			err = flash_overwrite_section_get(sectionstr,
							  &opts->overwrite_mask);
			if (err)
				return err;
			o_found |= DL_OPT_FLASH_OVERWRITE;

		} else if (dl_argv_match(dl, "reporter") &&
			   (o_all & DL_OPT_HEALTH_REPORTER_NAME)) {
			dl_arg_inc(dl);
			err = dl_argv_str(dl, &opts->reporter_name);
			if (err)
				return err;
			o_found |= DL_OPT_HEALTH_REPORTER_NAME;
		} else if (dl_argv_match(dl, "grace_period") &&
			   (o_all & DL_OPT_HEALTH_REPORTER_GRACEFUL_PERIOD)) {
			dl_arg_inc(dl);
			err = dl_argv_uint64_t(dl,
					       &opts->reporter_graceful_period);
			if (err)
				return err;
			o_found |= DL_OPT_HEALTH_REPORTER_GRACEFUL_PERIOD;
		} else if (dl_argv_match(dl, "auto_recover") &&
			(o_all & DL_OPT_HEALTH_REPORTER_AUTO_RECOVER)) {
			dl_arg_inc(dl);
			err = dl_argv_bool(dl, &opts->reporter_auto_recover);
			if (err)
				return err;
			o_found |= DL_OPT_HEALTH_REPORTER_AUTO_RECOVER;
		} else if (dl_argv_match(dl, "auto_dump") &&
			(o_all & DL_OPT_HEALTH_REPORTER_AUTO_DUMP)) {
			dl_arg_inc(dl);
			err = dl_argv_bool(dl, &opts->reporter_auto_dump);
			if (err)
				return err;
			o_found |= DL_OPT_HEALTH_REPORTER_AUTO_DUMP;
		} else if (dl_argv_match(dl, "trap") &&
			   (o_all & DL_OPT_TRAP_NAME)) {
			dl_arg_inc(dl);
			err = dl_argv_str(dl, &opts->trap_name);
			if (err)
				return err;
			o_found |= DL_OPT_TRAP_NAME;
		} else if (dl_argv_match(dl, "group") &&
			   (o_all & DL_OPT_TRAP_GROUP_NAME)) {
			dl_arg_inc(dl);
			err = dl_argv_str(dl, &opts->trap_group_name);
			if (err)
				return err;
			o_found |= DL_OPT_TRAP_GROUP_NAME;
		} else if (dl_argv_match(dl, "action") &&
			   (o_all & DL_OPT_TRAP_ACTION)) {
			const char *actionstr;

			dl_arg_inc(dl);
			err = dl_argv_str(dl, &actionstr);
			if (err)
				return err;
			err = trap_action_get(actionstr, &opts->trap_action);
			if (err)
				return err;
			o_found |= DL_OPT_TRAP_ACTION;
		} else if (dl_argv_match(dl, "netns") &&
			(o_all & DL_OPT_NETNS)) {
			const char *netns_str;

			dl_arg_inc(dl);
			err = dl_argv_str(dl, &netns_str);
			if (err)
				return err;
			opts->netns = netns_get_fd(netns_str);
			if ((int)opts->netns < 0) {
				dl_arg_dec(dl);
				err = dl_argv_uint32_t(dl, &opts->netns);
				if (err)
					return err;
				opts->netns_is_pid = true;
			}
			o_found |= DL_OPT_NETNS;
		} else if (dl_argv_match(dl, "action") &&
			   (o_all & DL_OPT_RELOAD_ACTION)) {
			const char *actionstr;

			dl_arg_inc(dl);
			err = dl_argv_str(dl, &actionstr);
			if (err)
				return err;
			err = reload_action_get(dl, actionstr, &opts->reload_action);
			if (err)
				return err;
			o_found |= DL_OPT_RELOAD_ACTION;
		} else if (dl_argv_match(dl, "limit") &&
			   (o_all & DL_OPT_RELOAD_LIMIT)) {
			const char *limitstr;

			dl_arg_inc(dl);
			err = dl_argv_str(dl, &limitstr);
			if (err)
				return err;
			err = reload_limit_get(dl, limitstr, &opts->reload_limit);
			if (err)
				return err;
			o_found |= DL_OPT_RELOAD_LIMIT;
		} else if (dl_argv_match(dl, "policer") &&
			   (o_all & DL_OPT_TRAP_POLICER_ID)) {
			dl_arg_inc(dl);
			err = dl_argv_uint32_t(dl, &opts->trap_policer_id);
			if (err)
				return err;
			o_found |= DL_OPT_TRAP_POLICER_ID;
		} else if (dl_argv_match(dl, "nopolicer") &&
			   (o_all & DL_OPT_TRAP_POLICER_ID)) {
			dl_arg_inc(dl);
			opts->trap_policer_id = 0;
			o_found |= DL_OPT_TRAP_POLICER_ID;
		} else if (dl_argv_match(dl, "rate") &&
			   (o_all & DL_OPT_TRAP_POLICER_RATE)) {
			dl_arg_inc(dl);
			err = dl_argv_uint64_t(dl, &opts->trap_policer_rate);
			if (err)
				return err;
			o_found |= DL_OPT_TRAP_POLICER_RATE;
		} else if (dl_argv_match(dl, "burst") &&
			   (o_all & DL_OPT_TRAP_POLICER_BURST)) {
			dl_arg_inc(dl);
			err = dl_argv_uint64_t(dl, &opts->trap_policer_burst);
			if (err)
				return err;
			o_found |= DL_OPT_TRAP_POLICER_BURST;
		} else if (dl_argv_match(dl, "hw_addr") &&
			   (o_all & DL_OPT_PORT_FUNCTION_HW_ADDR)) {
			const char *addrstr;

			dl_arg_inc(dl);
			err = dl_argv_str(dl, &addrstr);
			if (err)
				return err;
			err = hw_addr_parse(addrstr, opts->port_function_hw_addr,
					    &opts->port_function_hw_addr_len);
			if (err)
				return err;
			o_found |= DL_OPT_PORT_FUNCTION_HW_ADDR;
		} else if (dl_argv_match(dl, "state") &&
			   (o_all & DL_OPT_PORT_FUNCTION_STATE)) {
			const char *statestr;

			dl_arg_inc(dl);
			err = dl_argv_str(dl, &statestr);
			if (err)
				return err;
			err = port_fn_state_parse(statestr, &opts->port_fn_state);
			if (err)
				return err;

			o_found |= DL_OPT_PORT_FUNCTION_STATE;
		} else if (dl_argv_match(dl, "flavour") && (o_all & DL_OPT_PORT_FLAVOUR)) {
			const char *flavourstr;

			dl_arg_inc(dl);
			err = dl_argv_str(dl, &flavourstr);
			if (err)
				return err;
			err = port_flavour_parse(flavourstr, &opts->port_flavour);
			if (err)
				return err;
			o_found |= DL_OPT_PORT_FLAVOUR;
		} else if (dl_argv_match(dl, "pfnum") && (o_all & DL_OPT_PORT_PFNUMBER)) {
			dl_arg_inc(dl);
			err = dl_argv_uint16_t(dl, &opts->port_pfnumber);
			if (err)
				return err;
			o_found |= DL_OPT_PORT_PFNUMBER;
		} else if (dl_argv_match(dl, "sfnum") && (o_all & DL_OPT_PORT_SFNUMBER)) {
			dl_arg_inc(dl);
			err = dl_argv_uint32_t(dl, &opts->port_sfnumber);
			if (err)
				return err;
			o_found |= DL_OPT_PORT_SFNUMBER;
		} else {
			pr_err("Unknown option \"%s\"\n", dl_argv(dl));
			return -EINVAL;
		}
	}

	opts->present = o_found;

	if ((o_optional & DL_OPT_SB) && !(o_found & DL_OPT_SB)) {
		opts->sb_index = 0;
		opts->present |= DL_OPT_SB;
	}

	return dl_args_finding_required_validate(o_required, o_found);
}

static void
dl_function_attr_put(struct nlmsghdr *nlh, const struct dl_opts *opts)
{
	struct nlattr *nest;

	nest = mnl_attr_nest_start(nlh, DEVLINK_ATTR_PORT_FUNCTION);

	if (opts->present & DL_OPT_PORT_FUNCTION_HW_ADDR)
		mnl_attr_put(nlh, DEVLINK_PORT_FUNCTION_ATTR_HW_ADDR,
			     opts->port_function_hw_addr_len,
			     opts->port_function_hw_addr);
	if (opts->present & DL_OPT_PORT_FUNCTION_STATE)
		mnl_attr_put_u8(nlh, DEVLINK_PORT_FN_ATTR_STATE,
				opts->port_fn_state);
	mnl_attr_nest_end(nlh, nest);
}

static void
dl_flash_update_overwrite_put(struct nlmsghdr *nlh, const struct dl_opts *opts)
{
	struct nla_bitfield32 overwrite_mask;

	overwrite_mask.selector = DEVLINK_SUPPORTED_FLASH_OVERWRITE_SECTIONS;
	overwrite_mask.value = opts->overwrite_mask;

	mnl_attr_put(nlh, DEVLINK_ATTR_FLASH_UPDATE_OVERWRITE_MASK,
		     sizeof(overwrite_mask), &overwrite_mask);
}

static void
dl_reload_limits_put(struct nlmsghdr *nlh, const struct dl_opts *opts)
{
	struct nla_bitfield32 limits;

	limits.selector = DEVLINK_RELOAD_LIMITS_VALID_MASK;
	limits.value = BIT(opts->reload_limit);
	mnl_attr_put(nlh, DEVLINK_ATTR_RELOAD_LIMITS, sizeof(limits), &limits);
}

static void dl_opts_put(struct nlmsghdr *nlh, struct dl *dl)
{
	struct dl_opts *opts = &dl->opts;

	if (opts->present & DL_OPT_HANDLE) {
		mnl_attr_put_strz(nlh, DEVLINK_ATTR_BUS_NAME, opts->bus_name);
		mnl_attr_put_strz(nlh, DEVLINK_ATTR_DEV_NAME, opts->dev_name);
	} else if (opts->present & DL_OPT_HANDLEP) {
		mnl_attr_put_strz(nlh, DEVLINK_ATTR_BUS_NAME, opts->bus_name);
		mnl_attr_put_strz(nlh, DEVLINK_ATTR_DEV_NAME, opts->dev_name);
		mnl_attr_put_u32(nlh, DEVLINK_ATTR_PORT_INDEX,
				 opts->port_index);
	} else if (opts->present & DL_OPT_HANDLE_REGION) {
		mnl_attr_put_strz(nlh, DEVLINK_ATTR_BUS_NAME, opts->bus_name);
		mnl_attr_put_strz(nlh, DEVLINK_ATTR_DEV_NAME, opts->dev_name);
		mnl_attr_put_strz(nlh, DEVLINK_ATTR_REGION_NAME,
				  opts->region_name);
	}
	if (opts->present & DL_OPT_PORT_TYPE)
		mnl_attr_put_u16(nlh, DEVLINK_ATTR_PORT_TYPE,
				 opts->port_type);
	if (opts->present & DL_OPT_PORT_COUNT)
		mnl_attr_put_u32(nlh, DEVLINK_ATTR_PORT_SPLIT_COUNT,
				 opts->port_count);
	if (opts->present & DL_OPT_SB)
		mnl_attr_put_u32(nlh, DEVLINK_ATTR_SB_INDEX,
				 opts->sb_index);
	if (opts->present & DL_OPT_SB_POOL)
		mnl_attr_put_u16(nlh, DEVLINK_ATTR_SB_POOL_INDEX,
				 opts->sb_pool_index);
	if (opts->present & DL_OPT_SB_SIZE)
		mnl_attr_put_u32(nlh, DEVLINK_ATTR_SB_POOL_SIZE,
				 opts->sb_pool_size);
	if (opts->present & DL_OPT_SB_TYPE)
		mnl_attr_put_u8(nlh, DEVLINK_ATTR_SB_POOL_TYPE,
				opts->sb_pool_type);
	if (opts->present & DL_OPT_SB_THTYPE)
		mnl_attr_put_u8(nlh, DEVLINK_ATTR_SB_POOL_THRESHOLD_TYPE,
				opts->sb_pool_thtype);
	if (opts->present & DL_OPT_SB_TH)
		mnl_attr_put_u32(nlh, DEVLINK_ATTR_SB_THRESHOLD,
				 opts->sb_threshold);
	if (opts->present & DL_OPT_SB_TC)
		mnl_attr_put_u16(nlh, DEVLINK_ATTR_SB_TC_INDEX,
				 opts->sb_tc_index);
	if (opts->present & DL_OPT_ESWITCH_MODE)
		mnl_attr_put_u16(nlh, DEVLINK_ATTR_ESWITCH_MODE,
				 opts->eswitch_mode);
	if (opts->present & DL_OPT_ESWITCH_INLINE_MODE)
		mnl_attr_put_u8(nlh, DEVLINK_ATTR_ESWITCH_INLINE_MODE,
				opts->eswitch_inline_mode);
	if (opts->present & DL_OPT_DPIPE_TABLE_NAME)
		mnl_attr_put_strz(nlh, DEVLINK_ATTR_DPIPE_TABLE_NAME,
				  opts->dpipe_table_name);
	if (opts->present & DL_OPT_DPIPE_TABLE_COUNTERS)
		mnl_attr_put_u8(nlh, DEVLINK_ATTR_DPIPE_TABLE_COUNTERS_ENABLED,
				opts->dpipe_counters_enabled);
	if (opts->present & DL_OPT_ESWITCH_ENCAP_MODE)
		mnl_attr_put_u8(nlh, DEVLINK_ATTR_ESWITCH_ENCAP_MODE,
				opts->eswitch_encap_mode);
	if ((opts->present & DL_OPT_RESOURCE_PATH) && opts->resource_id_valid)
		mnl_attr_put_u64(nlh, DEVLINK_ATTR_RESOURCE_ID,
				 opts->resource_id);
	if (opts->present & DL_OPT_RESOURCE_SIZE)
		mnl_attr_put_u64(nlh, DEVLINK_ATTR_RESOURCE_SIZE,
				 opts->resource_size);
	if (opts->present & DL_OPT_PARAM_NAME)
		mnl_attr_put_strz(nlh, DEVLINK_ATTR_PARAM_NAME,
				  opts->param_name);
	if (opts->present & DL_OPT_PARAM_CMODE)
		mnl_attr_put_u8(nlh, DEVLINK_ATTR_PARAM_VALUE_CMODE,
				opts->cmode);
	if (opts->present & DL_OPT_REGION_SNAPSHOT_ID)
		mnl_attr_put_u32(nlh, DEVLINK_ATTR_REGION_SNAPSHOT_ID,
				 opts->region_snapshot_id);
	if (opts->present & DL_OPT_REGION_ADDRESS)
		mnl_attr_put_u64(nlh, DEVLINK_ATTR_REGION_CHUNK_ADDR,
				 opts->region_address);
	if (opts->present & DL_OPT_REGION_LENGTH)
		mnl_attr_put_u64(nlh, DEVLINK_ATTR_REGION_CHUNK_LEN,
				 opts->region_length);
	if (opts->present & DL_OPT_FLASH_FILE_NAME)
		mnl_attr_put_strz(nlh, DEVLINK_ATTR_FLASH_UPDATE_FILE_NAME,
				  opts->flash_file_name);
	if (opts->present & DL_OPT_FLASH_COMPONENT)
		mnl_attr_put_strz(nlh, DEVLINK_ATTR_FLASH_UPDATE_COMPONENT,
				  opts->flash_component);
	if (opts->present & DL_OPT_FLASH_OVERWRITE)
		dl_flash_update_overwrite_put(nlh, opts);
	if (opts->present & DL_OPT_HEALTH_REPORTER_NAME)
		mnl_attr_put_strz(nlh, DEVLINK_ATTR_HEALTH_REPORTER_NAME,
				  opts->reporter_name);
	if (opts->present & DL_OPT_HEALTH_REPORTER_GRACEFUL_PERIOD)
		mnl_attr_put_u64(nlh,
				 DEVLINK_ATTR_HEALTH_REPORTER_GRACEFUL_PERIOD,
				 opts->reporter_graceful_period);
	if (opts->present & DL_OPT_HEALTH_REPORTER_AUTO_RECOVER)
		mnl_attr_put_u8(nlh, DEVLINK_ATTR_HEALTH_REPORTER_AUTO_RECOVER,
				opts->reporter_auto_recover);
	if (opts->present & DL_OPT_HEALTH_REPORTER_AUTO_DUMP)
		mnl_attr_put_u8(nlh, DEVLINK_ATTR_HEALTH_REPORTER_AUTO_DUMP,
				opts->reporter_auto_dump);
	if (opts->present & DL_OPT_TRAP_NAME)
		mnl_attr_put_strz(nlh, DEVLINK_ATTR_TRAP_NAME,
				  opts->trap_name);
	if (opts->present & DL_OPT_TRAP_GROUP_NAME)
		mnl_attr_put_strz(nlh, DEVLINK_ATTR_TRAP_GROUP_NAME,
				  opts->trap_group_name);
	if (opts->present & DL_OPT_TRAP_ACTION)
		mnl_attr_put_u8(nlh, DEVLINK_ATTR_TRAP_ACTION,
				opts->trap_action);
	if (opts->present & DL_OPT_NETNS)
		mnl_attr_put_u32(nlh,
				 opts->netns_is_pid ? DEVLINK_ATTR_NETNS_PID :
						      DEVLINK_ATTR_NETNS_FD,
				 opts->netns);
	if (opts->present & DL_OPT_RELOAD_ACTION)
		mnl_attr_put_u8(nlh, DEVLINK_ATTR_RELOAD_ACTION,
				opts->reload_action);
	if (opts->present & DL_OPT_RELOAD_LIMIT)
		dl_reload_limits_put(nlh, opts);
	if (opts->present & DL_OPT_TRAP_POLICER_ID)
		mnl_attr_put_u32(nlh, DEVLINK_ATTR_TRAP_POLICER_ID,
				 opts->trap_policer_id);
	if (opts->present & DL_OPT_TRAP_POLICER_RATE)
		mnl_attr_put_u64(nlh, DEVLINK_ATTR_TRAP_POLICER_RATE,
				 opts->trap_policer_rate);
	if (opts->present & DL_OPT_TRAP_POLICER_BURST)
		mnl_attr_put_u64(nlh, DEVLINK_ATTR_TRAP_POLICER_BURST,
				 opts->trap_policer_burst);
	if (opts->present & (DL_OPT_PORT_FUNCTION_HW_ADDR | DL_OPT_PORT_FUNCTION_STATE))
		dl_function_attr_put(nlh, opts);
	if (opts->present & DL_OPT_PORT_FLAVOUR)
		mnl_attr_put_u16(nlh, DEVLINK_ATTR_PORT_FLAVOUR, opts->port_flavour);
	if (opts->present & DL_OPT_PORT_PFNUMBER)
		mnl_attr_put_u16(nlh, DEVLINK_ATTR_PORT_PCI_PF_NUMBER, opts->port_pfnumber);
	if (opts->present & DL_OPT_PORT_SFNUMBER)
		mnl_attr_put_u32(nlh, DEVLINK_ATTR_PORT_PCI_SF_NUMBER, opts->port_sfnumber);
}

static int dl_argv_parse_put(struct nlmsghdr *nlh, struct dl *dl,
			     uint64_t o_required, uint64_t o_optional)
{
	int err;

	err = dl_argv_parse(dl, o_required, o_optional);
	if (err)
		return err;
	dl_opts_put(nlh, dl);
	return 0;
}

static bool dl_dump_filter(struct dl *dl, struct nlattr **tb)
{
	struct dl_opts *opts = &dl->opts;
	struct nlattr *attr_bus_name = tb[DEVLINK_ATTR_BUS_NAME];
	struct nlattr *attr_dev_name = tb[DEVLINK_ATTR_DEV_NAME];
	struct nlattr *attr_port_index = tb[DEVLINK_ATTR_PORT_INDEX];
	struct nlattr *attr_sb_index = tb[DEVLINK_ATTR_SB_INDEX];

	if (opts->present & DL_OPT_HANDLE &&
	    attr_bus_name && attr_dev_name) {
		const char *bus_name = mnl_attr_get_str(attr_bus_name);
		const char *dev_name = mnl_attr_get_str(attr_dev_name);

		if (strcmp(bus_name, opts->bus_name) != 0 ||
		    strcmp(dev_name, opts->dev_name) != 0)
			return false;
	}
	if (opts->present & DL_OPT_HANDLEP &&
	    attr_bus_name && attr_dev_name && attr_port_index) {
		const char *bus_name = mnl_attr_get_str(attr_bus_name);
		const char *dev_name = mnl_attr_get_str(attr_dev_name);
		uint32_t port_index = mnl_attr_get_u32(attr_port_index);

		if (strcmp(bus_name, opts->bus_name) != 0 ||
		    strcmp(dev_name, opts->dev_name) != 0 ||
		    port_index != opts->port_index)
			return false;
	}
	if (opts->present & DL_OPT_SB && attr_sb_index) {
		uint32_t sb_index = mnl_attr_get_u32(attr_sb_index);

		if (sb_index != opts->sb_index)
			return false;
	}
	return true;
}

static void cmd_dev_help(void)
{
	pr_err("Usage: devlink dev show [ DEV ]\n");
	pr_err("       devlink dev eswitch set DEV [ mode { legacy | switchdev } ]\n");
	pr_err("                               [ inline-mode { none | link | network | transport } ]\n");
	pr_err("                               [ encap-mode { none | basic } ]\n");
	pr_err("       devlink dev eswitch show DEV\n");
	pr_err("       devlink dev param set DEV name PARAMETER value VALUE cmode { permanent | driverinit | runtime }\n");
	pr_err("       devlink dev param show [DEV name PARAMETER]\n");
	pr_err("       devlink dev reload DEV [ netns { PID | NAME | ID } ]\n");
	pr_err("                              [ action { driver_reinit | fw_activate } ] [ limit no_reset ]\n");
	pr_err("       devlink dev info [ DEV ]\n");
	pr_err("       devlink dev flash DEV file PATH [ component NAME ] [ overwrite SECTION ]\n");
}

static bool cmp_arr_last_handle(struct dl *dl, const char *bus_name,
				const char *dev_name)
{
	if (!dl->arr_last.present)
		return false;
	return strcmp(dl->arr_last.bus_name, bus_name) == 0 &&
	       strcmp(dl->arr_last.dev_name, dev_name) == 0;
}

static void arr_last_handle_set(struct dl *dl, const char *bus_name,
				const char *dev_name)
{
	dl->arr_last.present = true;
	free(dl->arr_last.dev_name);
	free(dl->arr_last.bus_name);
	dl->arr_last.bus_name = strdup(bus_name);
	dl->arr_last.dev_name = strdup(dev_name);
}

static bool should_arr_last_handle_start(struct dl *dl, const char *bus_name,
					 const char *dev_name)
{
	return !cmp_arr_last_handle(dl, bus_name, dev_name);
}

static bool should_arr_last_handle_end(struct dl *dl, const char *bus_name,
				       const char *dev_name)
{
	return dl->arr_last.present &&
	       !cmp_arr_last_handle(dl, bus_name, dev_name);
}

static void __pr_out_handle_start(struct dl *dl, struct nlattr **tb,
				  bool content, bool array)
{
	const char *bus_name = mnl_attr_get_str(tb[DEVLINK_ATTR_BUS_NAME]);
	const char *dev_name = mnl_attr_get_str(tb[DEVLINK_ATTR_DEV_NAME]);
	char buf[64];

	sprintf(buf, "%s/%s", bus_name, dev_name);

	if (dl->json_output) {
		if (array) {
			if (should_arr_last_handle_end(dl, bus_name, dev_name))
				close_json_array(PRINT_JSON, NULL);
			if (should_arr_last_handle_start(dl, bus_name,
							 dev_name)) {
				open_json_array(PRINT_JSON, buf);
				open_json_object(NULL);
				arr_last_handle_set(dl, bus_name, dev_name);
			} else {
				open_json_object(NULL);
			}
		} else {
			open_json_object(buf);
		}
	} else {
		if (array) {
			if (should_arr_last_handle_end(dl, bus_name, dev_name))
				__pr_out_indent_dec();
			if (should_arr_last_handle_start(dl, bus_name,
							 dev_name)) {
				pr_out("%s%s", buf, content ? ":" : "");
				__pr_out_newline();
				__pr_out_indent_inc();
				arr_last_handle_set(dl, bus_name, dev_name);
			}
		} else {
			pr_out("%s%s", buf, content ? ":" : "");
		}
	}
}

static void pr_out_handle_start_arr(struct dl *dl, struct nlattr **tb)
{
	__pr_out_handle_start(dl, tb, true, true);
}

static void pr_out_handle_end(struct dl *dl)
{
	if (dl->json_output)
		close_json_object();
	else
		__pr_out_newline();
}

static void pr_out_handle(struct dl *dl, struct nlattr **tb)
{
	__pr_out_handle_start(dl, tb, false, false);
	pr_out_handle_end(dl);
}

static bool cmp_arr_last_port_handle(struct dl *dl, const char *bus_name,
				     const char *dev_name, uint32_t port_index)
{
	return cmp_arr_last_handle(dl, bus_name, dev_name) &&
	       dl->arr_last.port_index == port_index;
}

static void arr_last_port_handle_set(struct dl *dl, const char *bus_name,
				     const char *dev_name, uint32_t port_index)
{
	arr_last_handle_set(dl, bus_name, dev_name);
	dl->arr_last.port_index = port_index;
}

static bool should_arr_last_port_handle_start(struct dl *dl,
					      const char *bus_name,
					      const char *dev_name,
					      uint32_t port_index)
{
	return !cmp_arr_last_port_handle(dl, bus_name, dev_name, port_index);
}

static bool should_arr_last_port_handle_end(struct dl *dl,
					    const char *bus_name,
					    const char *dev_name,
					    uint32_t port_index)
{
	return dl->arr_last.present &&
	       !cmp_arr_last_port_handle(dl, bus_name, dev_name, port_index);
}

static void __pr_out_port_handle_start(struct dl *dl, const char *bus_name,
				       const char *dev_name,
				       uint32_t port_index, bool try_nice,
				       bool array)
{
	static char buf[64];
	char *ifname = NULL;

	if (dl->no_nice_names || !try_nice ||
	    ifname_map_rev_lookup(dl, bus_name, dev_name,
				  port_index, &ifname) != 0)
		sprintf(buf, "%s/%s/%d", bus_name, dev_name, port_index);
	else
		sprintf(buf, "%s", ifname);

	if (dl->json_output) {
		if (array) {
			if (should_arr_last_port_handle_end(dl, bus_name,
							    dev_name,
							    port_index))
				close_json_array(PRINT_JSON, NULL);
			if (should_arr_last_port_handle_start(dl, bus_name,
							      dev_name,
							      port_index)) {
				open_json_array(PRINT_JSON, buf);
				open_json_object(NULL);
				arr_last_port_handle_set(dl, bus_name, dev_name,
							 port_index);
			} else {
				open_json_object(NULL);
			}
		} else {
			open_json_object(buf);
		}
	} else {
		if (array) {
			if (should_arr_last_port_handle_end(dl, bus_name, dev_name, port_index))
				__pr_out_indent_dec();
			if (should_arr_last_port_handle_start(dl, bus_name,
							      dev_name, port_index)) {
				pr_out("%s:", buf);
				__pr_out_newline();
				__pr_out_indent_inc();
				arr_last_port_handle_set(dl, bus_name, dev_name, port_index);
			}
		} else {
			pr_out("%s:", buf);
		}
	}
}

static void pr_out_port_handle_start(struct dl *dl, struct nlattr **tb, bool try_nice)
{
	const char *bus_name;
	const char *dev_name;
	uint32_t port_index;

	bus_name = mnl_attr_get_str(tb[DEVLINK_ATTR_BUS_NAME]);
	dev_name = mnl_attr_get_str(tb[DEVLINK_ATTR_DEV_NAME]);
	port_index = mnl_attr_get_u32(tb[DEVLINK_ATTR_PORT_INDEX]);
	__pr_out_port_handle_start(dl, bus_name, dev_name, port_index, try_nice, false);
}

static void pr_out_port_handle_start_arr(struct dl *dl, struct nlattr **tb, bool try_nice)
{
	const char *bus_name;
	const char *dev_name;
	uint32_t port_index;

	bus_name = mnl_attr_get_str(tb[DEVLINK_ATTR_BUS_NAME]);
	dev_name = mnl_attr_get_str(tb[DEVLINK_ATTR_DEV_NAME]);
	port_index = mnl_attr_get_u32(tb[DEVLINK_ATTR_PORT_INDEX]);
	__pr_out_port_handle_start(dl, bus_name, dev_name, port_index, try_nice, true);
}

static void pr_out_port_handle_end(struct dl *dl)
{
	if (dl->json_output)
		close_json_object();
	else
		pr_out("\n");
}

static void pr_out_region_chunk_start(struct dl *dl, uint64_t addr)
{
	if (dl->json_output) {
		print_uint(PRINT_JSON, "address", NULL, addr);
		open_json_array(PRINT_JSON, "data");
	}
}

static void pr_out_region_chunk_end(struct dl *dl)
{
	if (dl->json_output)
		close_json_array(PRINT_JSON, NULL);
}

static void pr_out_region_chunk(struct dl *dl, uint8_t *data, uint32_t len,
				uint64_t addr)
{
	static uint64_t align_val;
	uint32_t i = 0;

	pr_out_region_chunk_start(dl, addr);
	while (i < len) {
		if (!dl->json_output)
			if (!(align_val % 16))
				pr_out("%s%016"PRIx64" ",
				       align_val ? "\n" : "",
				       addr);

		align_val++;

		if (dl->json_output)
			print_int(PRINT_JSON, NULL, NULL, data[i]);
		else
			pr_out("%02x ", data[i]);

		addr++;
		i++;
	}
	pr_out_region_chunk_end(dl);
}

static void pr_out_stats(struct dl *dl, struct nlattr *nla_stats)
{
	struct nlattr *tb[DEVLINK_ATTR_STATS_MAX + 1] = {};
	int err;

	if (!dl->stats)
		return;

	err = mnl_attr_parse_nested(nla_stats, attr_stats_cb, tb);
	if (err != MNL_CB_OK)
		return;

	pr_out_object_start(dl, "stats");
	pr_out_object_start(dl, "rx");
	if (tb[DEVLINK_ATTR_STATS_RX_BYTES])
		pr_out_u64(dl, "bytes",
			   mnl_attr_get_u64(tb[DEVLINK_ATTR_STATS_RX_BYTES]));
	if (tb[DEVLINK_ATTR_STATS_RX_PACKETS])
		pr_out_u64(dl, "packets",
			   mnl_attr_get_u64(tb[DEVLINK_ATTR_STATS_RX_PACKETS]));
	if (tb[DEVLINK_ATTR_STATS_RX_DROPPED])
		pr_out_u64(dl, "dropped",
			   mnl_attr_get_u64(tb[DEVLINK_ATTR_STATS_RX_DROPPED]));
	pr_out_object_end(dl);
	pr_out_object_end(dl);
}

static const char *param_cmode_name(uint8_t cmode)
{
	switch (cmode) {
	case DEVLINK_PARAM_CMODE_RUNTIME:
		return PARAM_CMODE_RUNTIME_STR;
	case DEVLINK_PARAM_CMODE_DRIVERINIT:
		return PARAM_CMODE_DRIVERINIT_STR;
	case DEVLINK_PARAM_CMODE_PERMANENT:
		return PARAM_CMODE_PERMANENT_STR;
	default: return "<unknown type>";
	}
}

static const char *reload_action_name(uint8_t reload_action)
{
	switch (reload_action) {
	case DEVLINK_RELOAD_ACTION_DRIVER_REINIT:
		return "driver_reinit";
	case DEVLINK_RELOAD_ACTION_FW_ACTIVATE:
		return "fw_activate";
	default:
		return "<unknown reload action>";
	}
}

static const char *reload_limit_name(uint8_t reload_limit)
{
	switch (reload_limit) {
	case DEVLINK_RELOAD_LIMIT_UNSPEC:
		return "unspecified";
	case DEVLINK_RELOAD_LIMIT_NO_RESET:
		return "no_reset";
	default:
		return "<unknown reload action>";
	}
}

static const char *eswitch_mode_name(uint32_t mode)
{
	switch (mode) {
	case DEVLINK_ESWITCH_MODE_LEGACY: return ESWITCH_MODE_LEGACY;
	case DEVLINK_ESWITCH_MODE_SWITCHDEV: return ESWITCH_MODE_SWITCHDEV;
	default: return "<unknown mode>";
	}
}

static const char *eswitch_inline_mode_name(uint32_t mode)
{
	switch (mode) {
	case DEVLINK_ESWITCH_INLINE_MODE_NONE:
		return ESWITCH_INLINE_MODE_NONE;
	case DEVLINK_ESWITCH_INLINE_MODE_LINK:
		return ESWITCH_INLINE_MODE_LINK;
	case DEVLINK_ESWITCH_INLINE_MODE_NETWORK:
		return ESWITCH_INLINE_MODE_NETWORK;
	case DEVLINK_ESWITCH_INLINE_MODE_TRANSPORT:
		return ESWITCH_INLINE_MODE_TRANSPORT;
	default:
		return "<unknown mode>";
	}
}

static const char *eswitch_encap_mode_name(uint32_t mode)
{
	switch (mode) {
	case DEVLINK_ESWITCH_ENCAP_MODE_NONE:
		return ESWITCH_ENCAP_MODE_NONE;
	case DEVLINK_ESWITCH_ENCAP_MODE_BASIC:
		return ESWITCH_ENCAP_MODE_BASIC;
	default:
		return "<unknown mode>";
	}
}

static void pr_out_eswitch(struct dl *dl, struct nlattr **tb)
{
	__pr_out_handle_start(dl, tb, true, false);

	if (tb[DEVLINK_ATTR_ESWITCH_MODE]) {
		check_indent_newline(dl);
		print_string(PRINT_ANY, "mode", "mode %s",
			     eswitch_mode_name(mnl_attr_get_u16(
				     tb[DEVLINK_ATTR_ESWITCH_MODE])));
	}
	if (tb[DEVLINK_ATTR_ESWITCH_INLINE_MODE]) {
		check_indent_newline(dl);
		print_string(PRINT_ANY, "inline-mode", "inline-mode %s",
			     eswitch_inline_mode_name(mnl_attr_get_u8(
				     tb[DEVLINK_ATTR_ESWITCH_INLINE_MODE])));
	}
	if (tb[DEVLINK_ATTR_ESWITCH_ENCAP_MODE]) {
		check_indent_newline(dl);
		print_string(PRINT_ANY, "encap-mode", "encap-mode %s",
			     eswitch_encap_mode_name(mnl_attr_get_u8(
				    tb[DEVLINK_ATTR_ESWITCH_ENCAP_MODE])));
	}

	pr_out_handle_end(dl);
}

static int cmd_dev_eswitch_show_cb(const struct nlmsghdr *nlh, void *data)
{
	struct dl *dl = data;
	struct nlattr *tb[DEVLINK_ATTR_MAX + 1] = {};
	struct genlmsghdr *genl = mnl_nlmsg_get_payload(nlh);

	mnl_attr_parse(nlh, sizeof(*genl), attr_cb, tb);
	if (!tb[DEVLINK_ATTR_BUS_NAME] || !tb[DEVLINK_ATTR_DEV_NAME])
		return MNL_CB_ERROR;
	pr_out_eswitch(dl, tb);
	return MNL_CB_OK;
}

static int cmd_dev_eswitch_show(struct dl *dl)
{
	struct nlmsghdr *nlh;
	int err;

	nlh = mnlg_msg_prepare(dl->nlg, DEVLINK_CMD_ESWITCH_GET,
			       NLM_F_REQUEST | NLM_F_ACK);

	err = dl_argv_parse_put(nlh, dl, DL_OPT_HANDLE, 0);
	if (err)
		return err;

	pr_out_section_start(dl, "dev");
	err = _mnlg_socket_sndrcv(dl->nlg, nlh, cmd_dev_eswitch_show_cb, dl);
	pr_out_section_end(dl);
	return err;
}

static int cmd_dev_eswitch_set(struct dl *dl)
{
	struct nlmsghdr *nlh;
	int err;

	nlh = mnlg_msg_prepare(dl->nlg, DEVLINK_CMD_ESWITCH_SET,
			       NLM_F_REQUEST | NLM_F_ACK);

	err = dl_argv_parse_put(nlh, dl, DL_OPT_HANDLE,
				DL_OPT_ESWITCH_MODE |
				DL_OPT_ESWITCH_INLINE_MODE |
				DL_OPT_ESWITCH_ENCAP_MODE);

	if (err)
		return err;

	if (dl->opts.present == 1) {
		pr_err("Need to set at least one option\n");
		return -ENOENT;
	}

	return _mnlg_socket_sndrcv(dl->nlg, nlh, NULL, NULL);
}

static int cmd_dev_eswitch(struct dl *dl)
{
	if (dl_argv_match(dl, "help") || dl_no_arg(dl)) {
		cmd_dev_help();
		return 0;
	} else if (dl_argv_match(dl, "set")) {
		dl_arg_inc(dl);
		return cmd_dev_eswitch_set(dl);
	} else if (dl_argv_match(dl, "show")) {
		dl_arg_inc(dl);
		return cmd_dev_eswitch_show(dl);
	}
	pr_err("Command \"%s\" not found\n", dl_argv(dl));
	return -ENOENT;
}

struct param_val_conv {
	const char *name;
	const char *vstr;
	uint32_t vuint;
};

static bool param_val_conv_exists(const struct param_val_conv *param_val_conv,
				  uint32_t len, const char *name)
{
	uint32_t i;

	for (i = 0; i < len; i++)
		if (!strcmp(param_val_conv[i].name, name))
			return true;

	return false;
}

static int
param_val_conv_uint_get(const struct param_val_conv *param_val_conv,
			uint32_t len, const char *name, const char *vstr,
			uint32_t *vuint)
{
	uint32_t i;

	for (i = 0; i < len; i++)
		if (!strcmp(param_val_conv[i].name, name) &&
		    !strcmp(param_val_conv[i].vstr, vstr)) {
			*vuint = param_val_conv[i].vuint;
			return 0;
		}

	return -ENOENT;
}

static int
param_val_conv_str_get(const struct param_val_conv *param_val_conv,
		       uint32_t len, const char *name, uint32_t vuint,
		       const char **vstr)
{
	uint32_t i;

	for (i = 0; i < len; i++)
		if (!strcmp(param_val_conv[i].name, name) &&
		    param_val_conv[i].vuint == vuint) {
			*vstr = param_val_conv[i].vstr;
			return 0;
		}

	return -ENOENT;
}

static const struct param_val_conv param_val_conv[] = {
	{
		.name = "fw_load_policy",
		.vstr = "driver",
		.vuint = DEVLINK_PARAM_FW_LOAD_POLICY_VALUE_DRIVER,
	},
	{
		.name = "fw_load_policy",
		.vstr = "flash",
		.vuint = DEVLINK_PARAM_FW_LOAD_POLICY_VALUE_FLASH,
	},
	{
		.name = "fw_load_policy",
		.vstr = "disk",
		.vuint = DEVLINK_PARAM_FW_LOAD_POLICY_VALUE_DISK,
	},
	{
		.name = "reset_dev_on_drv_probe",
		.vstr = "unknown",
		.vuint = DEVLINK_PARAM_RESET_DEV_ON_DRV_PROBE_VALUE_UNKNOWN,
	},
	{
		.name = "fw_load_policy",
		.vstr = "unknown",
		.vuint = DEVLINK_PARAM_FW_LOAD_POLICY_VALUE_UNKNOWN,
	},
	{
		.name = "reset_dev_on_drv_probe",
		.vstr = "always",
		.vuint = DEVLINK_PARAM_RESET_DEV_ON_DRV_PROBE_VALUE_ALWAYS,
	},
	{
		.name = "reset_dev_on_drv_probe",
		.vstr = "never",
		.vuint = DEVLINK_PARAM_RESET_DEV_ON_DRV_PROBE_VALUE_NEVER,
	},
	{
		.name = "reset_dev_on_drv_probe",
		.vstr = "disk",
		.vuint = DEVLINK_PARAM_RESET_DEV_ON_DRV_PROBE_VALUE_DISK,
	},
};

#define PARAM_VAL_CONV_LEN ARRAY_SIZE(param_val_conv)

static void pr_out_param_value(struct dl *dl, const char *nla_name,
			       int nla_type, struct nlattr *nl)
{
	struct nlattr *nla_value[DEVLINK_ATTR_MAX + 1] = {};
	struct nlattr *val_attr;
	const char *vstr;
	bool conv_exists;
	int err;

	err = mnl_attr_parse_nested(nl, attr_cb, nla_value);
	if (err != MNL_CB_OK)
		return;

	if (!nla_value[DEVLINK_ATTR_PARAM_VALUE_CMODE] ||
	    (nla_type != MNL_TYPE_FLAG &&
	     !nla_value[DEVLINK_ATTR_PARAM_VALUE_DATA]))
		return;

	check_indent_newline(dl);
	print_string(PRINT_ANY, "cmode", "cmode %s",
		     param_cmode_name(mnl_attr_get_u8(nla_value[DEVLINK_ATTR_PARAM_VALUE_CMODE])));

	val_attr = nla_value[DEVLINK_ATTR_PARAM_VALUE_DATA];

	conv_exists = param_val_conv_exists(param_val_conv, PARAM_VAL_CONV_LEN,
					    nla_name);

	switch (nla_type) {
	case MNL_TYPE_U8:
		if (conv_exists) {
			err = param_val_conv_str_get(param_val_conv,
						     PARAM_VAL_CONV_LEN,
						     nla_name,
						     mnl_attr_get_u8(val_attr),
						     &vstr);
			if (err)
				return;
			print_string(PRINT_ANY, "value", " value %s", vstr);
		} else {
			print_uint(PRINT_ANY, "value", " value %u",
				   mnl_attr_get_u8(val_attr));
		}
		break;
	case MNL_TYPE_U16:
		if (conv_exists) {
			err = param_val_conv_str_get(param_val_conv,
						     PARAM_VAL_CONV_LEN,
						     nla_name,
						     mnl_attr_get_u16(val_attr),
						     &vstr);
			if (err)
				return;
			print_string(PRINT_ANY, "value", " value %s", vstr);
		} else {
			print_uint(PRINT_ANY, "value", " value %u",
				   mnl_attr_get_u16(val_attr));
		}
		break;
	case MNL_TYPE_U32:
		if (conv_exists) {
			err = param_val_conv_str_get(param_val_conv,
						     PARAM_VAL_CONV_LEN,
						     nla_name,
						     mnl_attr_get_u32(val_attr),
						     &vstr);
			if (err)
				return;
			print_string(PRINT_ANY, "value", " value %s", vstr);
		} else {
			print_uint(PRINT_ANY, "value", " value %u",
				   mnl_attr_get_u32(val_attr));
		}
		break;
	case MNL_TYPE_STRING:
		print_string(PRINT_ANY, "value", " value %s",
			     mnl_attr_get_str(val_attr));
		break;
	case MNL_TYPE_FLAG:
		print_bool(PRINT_ANY, "value", " value %s", val_attr);
		break;
	}
}

static void pr_out_param(struct dl *dl, struct nlattr **tb, bool array,
			 bool is_port_param)
{
	struct nlattr *nla_param[DEVLINK_ATTR_MAX + 1] = {};
	struct nlattr *param_value_attr;
	const char *nla_name;
	int nla_type;
	int err;

	err = mnl_attr_parse_nested(tb[DEVLINK_ATTR_PARAM], attr_cb, nla_param);
	if (err != MNL_CB_OK)
		return;
	if (!nla_param[DEVLINK_ATTR_PARAM_NAME] ||
	    !nla_param[DEVLINK_ATTR_PARAM_TYPE] ||
	    !nla_param[DEVLINK_ATTR_PARAM_VALUES_LIST])
		return;

	if (array)
		if (is_port_param)
			pr_out_port_handle_start_arr(dl, tb, false);
		else
			pr_out_handle_start_arr(dl, tb);
	else
		if (is_port_param)
			pr_out_port_handle_start(dl, tb, false);
		else
			__pr_out_handle_start(dl, tb, true, false);

	nla_type = mnl_attr_get_u8(nla_param[DEVLINK_ATTR_PARAM_TYPE]);

	nla_name = mnl_attr_get_str(nla_param[DEVLINK_ATTR_PARAM_NAME]);
	check_indent_newline(dl);
	print_string(PRINT_ANY, "name", "name %s ", nla_name);
	if (!nla_param[DEVLINK_ATTR_PARAM_GENERIC])
		print_string(PRINT_ANY, "type", "type %s", "driver-specific");
	else
		print_string(PRINT_ANY, "type", "type %s", "generic");

	pr_out_array_start(dl, "values");
	mnl_attr_for_each_nested(param_value_attr,
				 nla_param[DEVLINK_ATTR_PARAM_VALUES_LIST]) {
		pr_out_entry_start(dl);
		pr_out_param_value(dl, nla_name, nla_type, param_value_attr);
		pr_out_entry_end(dl);
	}
	pr_out_array_end(dl);
	if (is_port_param)
		pr_out_port_handle_end(dl);
	else
		pr_out_handle_end(dl);
}

static int cmd_dev_param_show_cb(const struct nlmsghdr *nlh, void *data)
{
	struct genlmsghdr *genl = mnl_nlmsg_get_payload(nlh);
	struct nlattr *tb[DEVLINK_ATTR_MAX + 1] = {};
	struct dl *dl = data;

	mnl_attr_parse(nlh, sizeof(*genl), attr_cb, tb);
	if (!tb[DEVLINK_ATTR_BUS_NAME] || !tb[DEVLINK_ATTR_DEV_NAME] ||
	    !tb[DEVLINK_ATTR_PARAM])
		return MNL_CB_ERROR;
	pr_out_param(dl, tb, true, false);
	return MNL_CB_OK;
}

struct param_ctx {
	struct dl *dl;
	int nla_type;
	union {
		uint8_t vu8;
		uint16_t vu16;
		uint32_t vu32;
		const char *vstr;
		bool vbool;
	} value;
};

static int cmd_dev_param_set_cb(const struct nlmsghdr *nlh, void *data)
{
	struct genlmsghdr *genl = mnl_nlmsg_get_payload(nlh);
	struct nlattr *nla_param[DEVLINK_ATTR_MAX + 1] = {};
	struct nlattr *tb[DEVLINK_ATTR_MAX + 1] = {};
	struct nlattr *param_value_attr;
	enum devlink_param_cmode cmode;
	struct param_ctx *ctx = data;
	struct dl *dl = ctx->dl;
	int nla_type;
	int err;

	mnl_attr_parse(nlh, sizeof(*genl), attr_cb, tb);
	if (!tb[DEVLINK_ATTR_BUS_NAME] || !tb[DEVLINK_ATTR_DEV_NAME] ||
	    !tb[DEVLINK_ATTR_PARAM])
		return MNL_CB_ERROR;

	err = mnl_attr_parse_nested(tb[DEVLINK_ATTR_PARAM], attr_cb, nla_param);
	if (err != MNL_CB_OK)
		return MNL_CB_ERROR;

	if (!nla_param[DEVLINK_ATTR_PARAM_TYPE] ||
	    !nla_param[DEVLINK_ATTR_PARAM_VALUES_LIST])
		return MNL_CB_ERROR;

	nla_type = mnl_attr_get_u8(nla_param[DEVLINK_ATTR_PARAM_TYPE]);
	mnl_attr_for_each_nested(param_value_attr,
				 nla_param[DEVLINK_ATTR_PARAM_VALUES_LIST]) {
		struct nlattr *nla_value[DEVLINK_ATTR_MAX + 1] = {};
		struct nlattr *val_attr;

		err = mnl_attr_parse_nested(param_value_attr,
					    attr_cb, nla_value);
		if (err != MNL_CB_OK)
			return MNL_CB_ERROR;

		if (!nla_value[DEVLINK_ATTR_PARAM_VALUE_CMODE] ||
		    (nla_type != MNL_TYPE_FLAG &&
		     !nla_value[DEVLINK_ATTR_PARAM_VALUE_DATA]))
			return MNL_CB_ERROR;

		cmode = mnl_attr_get_u8(nla_value[DEVLINK_ATTR_PARAM_VALUE_CMODE]);
		if (cmode == dl->opts.cmode) {
			val_attr = nla_value[DEVLINK_ATTR_PARAM_VALUE_DATA];
			switch (nla_type) {
			case MNL_TYPE_U8:
				ctx->value.vu8 = mnl_attr_get_u8(val_attr);
				break;
			case MNL_TYPE_U16:
				ctx->value.vu16 = mnl_attr_get_u16(val_attr);
				break;
			case MNL_TYPE_U32:
				ctx->value.vu32 = mnl_attr_get_u32(val_attr);
				break;
			case MNL_TYPE_STRING:
				ctx->value.vstr = mnl_attr_get_str(val_attr);
				break;
			case MNL_TYPE_FLAG:
				ctx->value.vbool = val_attr ? true : false;
				break;
			}
			break;
		}
	}
	ctx->nla_type = nla_type;
	return MNL_CB_OK;
}

static int cmd_dev_param_set(struct dl *dl)
{
	struct param_ctx ctx = {};
	struct nlmsghdr *nlh;
	bool conv_exists;
	uint32_t val_u32 = 0;
	uint16_t val_u16;
	uint8_t val_u8;
	bool val_bool;
	int err;

	err = dl_argv_parse(dl, DL_OPT_HANDLE |
			    DL_OPT_PARAM_NAME |
			    DL_OPT_PARAM_VALUE |
			    DL_OPT_PARAM_CMODE, 0);
	if (err)
		return err;

	/* Get value type */
	nlh = mnlg_msg_prepare(dl->nlg, DEVLINK_CMD_PARAM_GET,
			       NLM_F_REQUEST | NLM_F_ACK);
	dl_opts_put(nlh, dl);

	ctx.dl = dl;
	err = _mnlg_socket_sndrcv(dl->nlg, nlh, cmd_dev_param_set_cb, &ctx);
	if (err)
		return err;

	nlh = mnlg_msg_prepare(dl->nlg, DEVLINK_CMD_PARAM_SET,
			       NLM_F_REQUEST | NLM_F_ACK);
	dl_opts_put(nlh, dl);

	conv_exists = param_val_conv_exists(param_val_conv, PARAM_VAL_CONV_LEN,
					    dl->opts.param_name);

	mnl_attr_put_u8(nlh, DEVLINK_ATTR_PARAM_TYPE, ctx.nla_type);
	switch (ctx.nla_type) {
	case MNL_TYPE_U8:
		if (conv_exists) {
			err = param_val_conv_uint_get(param_val_conv,
						      PARAM_VAL_CONV_LEN,
						      dl->opts.param_name,
						      dl->opts.param_value,
						      &val_u32);
			val_u8 = val_u32;
		} else {
			err = strtouint8_t(dl->opts.param_value, &val_u8);
		}
		if (err)
			goto err_param_value_parse;
		if (val_u8 == ctx.value.vu8)
			return 0;
		mnl_attr_put_u8(nlh, DEVLINK_ATTR_PARAM_VALUE_DATA, val_u8);
		break;
	case MNL_TYPE_U16:
		if (conv_exists) {
			err = param_val_conv_uint_get(param_val_conv,
						      PARAM_VAL_CONV_LEN,
						      dl->opts.param_name,
						      dl->opts.param_value,
						      &val_u32);
			val_u16 = val_u32;
		} else {
			err = strtouint16_t(dl->opts.param_value, &val_u16);
		}
		if (err)
			goto err_param_value_parse;
		if (val_u16 == ctx.value.vu16)
			return 0;
		mnl_attr_put_u16(nlh, DEVLINK_ATTR_PARAM_VALUE_DATA, val_u16);
		break;
	case MNL_TYPE_U32:
		if (conv_exists)
			err = param_val_conv_uint_get(param_val_conv,
						      PARAM_VAL_CONV_LEN,
						      dl->opts.param_name,
						      dl->opts.param_value,
						      &val_u32);
		else
			err = strtouint32_t(dl->opts.param_value, &val_u32);
		if (err)
			goto err_param_value_parse;
		if (val_u32 == ctx.value.vu32)
			return 0;
		mnl_attr_put_u32(nlh, DEVLINK_ATTR_PARAM_VALUE_DATA, val_u32);
		break;
	case MNL_TYPE_FLAG:
		err = strtobool(dl->opts.param_value, &val_bool);
		if (err)
			goto err_param_value_parse;
		if (val_bool == ctx.value.vbool)
			return 0;
		if (val_bool)
			mnl_attr_put(nlh, DEVLINK_ATTR_PARAM_VALUE_DATA,
				     0, NULL);
		break;
	case MNL_TYPE_STRING:
		mnl_attr_put_strz(nlh, DEVLINK_ATTR_PARAM_VALUE_DATA,
				  dl->opts.param_value);
		if (!strcmp(dl->opts.param_value, ctx.value.vstr))
			return 0;
		break;
	default:
		printf("Value type not supported\n");
		return -ENOTSUP;
	}
	return _mnlg_socket_sndrcv(dl->nlg, nlh, NULL, NULL);

err_param_value_parse:
	pr_err("Value \"%s\" is not a number or not within range\n",
	       dl->opts.param_value);
	return err;
}

static int cmd_port_param_show_cb(const struct nlmsghdr *nlh, void *data)
{
	struct genlmsghdr *genl = mnl_nlmsg_get_payload(nlh);
	struct nlattr *tb[DEVLINK_ATTR_MAX + 1] = {};
	struct dl *dl = data;

	mnl_attr_parse(nlh, sizeof(*genl), attr_cb, tb);
	if (!tb[DEVLINK_ATTR_BUS_NAME] || !tb[DEVLINK_ATTR_DEV_NAME] ||
	    !tb[DEVLINK_ATTR_PORT_INDEX] || !tb[DEVLINK_ATTR_PARAM])
		return MNL_CB_ERROR;

	pr_out_param(dl, tb, true, true);
	return MNL_CB_OK;
}

static int cmd_dev_param_show(struct dl *dl)
{
	uint16_t flags = NLM_F_REQUEST | NLM_F_ACK;
	struct nlmsghdr *nlh;
	int err;

	if (dl_argc(dl) == 0)
		flags |= NLM_F_DUMP;

	nlh = mnlg_msg_prepare(dl->nlg, DEVLINK_CMD_PARAM_GET, flags);

	if (dl_argc(dl) > 0) {
		err = dl_argv_parse_put(nlh, dl, DL_OPT_HANDLE |
					DL_OPT_PARAM_NAME, 0);
		if (err)
			return err;
	}

	pr_out_section_start(dl, "param");
	err = _mnlg_socket_sndrcv(dl->nlg, nlh, cmd_dev_param_show_cb, dl);
	pr_out_section_end(dl);
	return err;
}

static int cmd_dev_param(struct dl *dl)
{
	if (dl_argv_match(dl, "help")) {
		cmd_dev_help();
		return 0;
	} else if (dl_argv_match(dl, "show") ||
		   dl_argv_match(dl, "list") || dl_no_arg(dl)) {
		dl_arg_inc(dl);
		return cmd_dev_param_show(dl);
	} else if (dl_argv_match(dl, "set")) {
		dl_arg_inc(dl);
		return cmd_dev_param_set(dl);
	}
	pr_err("Command \"%s\" not found\n", dl_argv(dl));
	return -ENOENT;
}

static void pr_out_action_stats(struct dl *dl, struct nlattr *action_stats)
{
	struct nlattr *tb_stats_entry[DEVLINK_ATTR_MAX + 1] = {};
	struct nlattr *nla_reload_stats_entry, *nla_limit, *nla_value;
	enum devlink_reload_limit limit;
	uint32_t value;
	int err;

	mnl_attr_for_each_nested(nla_reload_stats_entry, action_stats) {
		err = mnl_attr_parse_nested(nla_reload_stats_entry, attr_cb,
					    tb_stats_entry);
		if (err != MNL_CB_OK)
			return;

		nla_limit = tb_stats_entry[DEVLINK_ATTR_RELOAD_STATS_LIMIT];
		nla_value = tb_stats_entry[DEVLINK_ATTR_RELOAD_STATS_VALUE];
		if (!nla_limit || !nla_value)
			return;

		check_indent_newline(dl);
		limit = mnl_attr_get_u8(nla_limit);
		value = mnl_attr_get_u32(nla_value);
		print_uint_name_value(reload_limit_name(limit), value);
	}
}

static void pr_out_reload_stats(struct dl *dl, struct nlattr *reload_stats)
{
	struct nlattr *nla_action_info, *nla_action, *nla_action_stats;
	struct nlattr *tb[DEVLINK_ATTR_MAX + 1] = {};
	enum devlink_reload_action action;
	int err;

	mnl_attr_for_each_nested(nla_action_info, reload_stats) {
		err = mnl_attr_parse_nested(nla_action_info, attr_cb, tb);
		if (err != MNL_CB_OK)
			return;
		nla_action = tb[DEVLINK_ATTR_RELOAD_ACTION];
		nla_action_stats = tb[DEVLINK_ATTR_RELOAD_ACTION_STATS];
		if (!nla_action || !nla_action_stats)
			return;

		action = mnl_attr_get_u8(nla_action);
		pr_out_object_start(dl, reload_action_name(action));
		pr_out_action_stats(dl, nla_action_stats);
		pr_out_object_end(dl);
	}
}

static void pr_out_reload_data(struct dl *dl, struct nlattr **tb)
{
	struct nlattr *nla_reload_stats, *nla_remote_reload_stats;
	struct nlattr *tb_stats[DEVLINK_ATTR_MAX + 1] = {};
	uint8_t reload_failed = 0;
	int err;

	if (tb[DEVLINK_ATTR_RELOAD_FAILED])
		reload_failed = mnl_attr_get_u8(tb[DEVLINK_ATTR_RELOAD_FAILED]);

	if (reload_failed) {
		check_indent_newline(dl);
		print_bool(PRINT_ANY, "reload_failed", "reload_failed %s", true);
	}
	if (!tb[DEVLINK_ATTR_DEV_STATS] || !dl->stats)
		return;
	err = mnl_attr_parse_nested(tb[DEVLINK_ATTR_DEV_STATS], attr_cb,
				    tb_stats);
	if (err != MNL_CB_OK)
		return;

	pr_out_object_start(dl, "stats");

	nla_reload_stats = tb_stats[DEVLINK_ATTR_RELOAD_STATS];
	if (nla_reload_stats) {
		pr_out_object_start(dl, "reload");
		pr_out_reload_stats(dl, nla_reload_stats);
		pr_out_object_end(dl);
	}
	nla_remote_reload_stats = tb_stats[DEVLINK_ATTR_REMOTE_RELOAD_STATS];
	if (nla_remote_reload_stats) {
		pr_out_object_start(dl, "remote_reload");
		pr_out_reload_stats(dl, nla_remote_reload_stats);
		pr_out_object_end(dl);
	}

	pr_out_object_end(dl);
}


static void pr_out_dev(struct dl *dl, struct nlattr **tb)
{
	if ((tb[DEVLINK_ATTR_RELOAD_FAILED] && mnl_attr_get_u8(tb[DEVLINK_ATTR_RELOAD_FAILED])) ||
	    (tb[DEVLINK_ATTR_DEV_STATS] && dl->stats)) {
		__pr_out_handle_start(dl, tb, true, false);
		pr_out_reload_data(dl, tb);
		pr_out_handle_end(dl);
	} else {
		pr_out_handle(dl, tb);
	}
}

static int cmd_dev_show_cb(const struct nlmsghdr *nlh, void *data)
{
	struct dl *dl = data;
	struct nlattr *tb[DEVLINK_ATTR_MAX + 1] = {};
	struct genlmsghdr *genl = mnl_nlmsg_get_payload(nlh);

	mnl_attr_parse(nlh, sizeof(*genl), attr_cb, tb);
	if (!tb[DEVLINK_ATTR_BUS_NAME] || !tb[DEVLINK_ATTR_DEV_NAME])
		return MNL_CB_ERROR;

	pr_out_dev(dl, tb);
	return MNL_CB_OK;
}

static int cmd_dev_show(struct dl *dl)
{
	struct nlmsghdr *nlh;
	uint16_t flags = NLM_F_REQUEST | NLM_F_ACK;
	int err;

	if (dl_argc(dl) == 0)
		flags |= NLM_F_DUMP;

	nlh = mnlg_msg_prepare(dl->nlg, DEVLINK_CMD_GET, flags);

	if (dl_argc(dl) > 0) {
		err = dl_argv_parse_put(nlh, dl, DL_OPT_HANDLE, 0);
		if (err)
			return err;
	}

	pr_out_section_start(dl, "dev");
	err = _mnlg_socket_sndrcv(dl->nlg, nlh, cmd_dev_show_cb, dl);
	pr_out_section_end(dl);
	return err;
}

static void pr_out_reload_actions_performed(struct dl *dl, struct nlattr **tb)
{
	struct nlattr *nla_actions_performed;
	struct nla_bitfield32 *actions;
	uint32_t actions_performed;
	uint16_t len;
	int action;

	if (!tb[DEVLINK_ATTR_RELOAD_ACTIONS_PERFORMED])
		return;

	nla_actions_performed = tb[DEVLINK_ATTR_RELOAD_ACTIONS_PERFORMED];
	len = mnl_attr_get_payload_len(nla_actions_performed);
	if (len != sizeof(*actions))
		return;
	actions = mnl_attr_get_payload(nla_actions_performed);
	if (!actions)
		return;
	g_new_line_count = 1; /* Avoid extra new line in non-json print */
	pr_out_array_start(dl, "reload_actions_performed");
	actions_performed = actions->value & actions->selector;
	for (action = 0; action <= DEVLINK_RELOAD_ACTION_MAX; action++) {
		if (BIT(action) & actions_performed) {
			check_indent_newline(dl);
			print_string(PRINT_ANY, NULL, "%s",
				     reload_action_name(action));
		}
	}
	pr_out_array_end(dl);
	if (!dl->json_output)
		__pr_out_newline();
}

static int cmd_dev_reload_cb(const struct nlmsghdr *nlh, void *data)
{
	struct genlmsghdr *genl = mnl_nlmsg_get_payload(nlh);
	struct nlattr *tb[DEVLINK_ATTR_MAX + 1] = {};
	struct dl *dl = data;

	mnl_attr_parse(nlh, sizeof(*genl), attr_cb, tb);
	if (!tb[DEVLINK_ATTR_BUS_NAME] || !tb[DEVLINK_ATTR_DEV_NAME] ||
	    !tb[DEVLINK_ATTR_RELOAD_ACTIONS_PERFORMED])
		return MNL_CB_ERROR;

	pr_out_section_start(dl, "reload");
	pr_out_reload_actions_performed(dl, tb);
	pr_out_section_end(dl);

	return MNL_CB_OK;
}

static int cmd_dev_reload(struct dl *dl)
{
	struct nlmsghdr *nlh;
	int err;

	if (dl_argv_match(dl, "help") || dl_no_arg(dl)) {
		cmd_dev_help();
		return 0;
	}

	nlh = mnlg_msg_prepare(dl->nlg, DEVLINK_CMD_RELOAD,
			       NLM_F_REQUEST | NLM_F_ACK);

	err = dl_argv_parse_put(nlh, dl, DL_OPT_HANDLE,
				DL_OPT_NETNS | DL_OPT_RELOAD_ACTION |
				DL_OPT_RELOAD_LIMIT);
	if (err)
		return err;

	return _mnlg_socket_sndrcv(dl->nlg, nlh, cmd_dev_reload_cb, dl);
}

static void pr_out_versions_single(struct dl *dl, const struct nlmsghdr *nlh,
				   const char *name, int type)
{
	struct nlattr *version;

	mnl_attr_for_each(version, nlh, sizeof(struct genlmsghdr)) {
		struct nlattr *tb[DEVLINK_ATTR_MAX + 1] = {};
		const char *ver_value;
		const char *ver_name;
		int err;

		if (mnl_attr_get_type(version) != type)
			continue;

		err = mnl_attr_parse_nested(version, attr_cb, tb);
		if (err != MNL_CB_OK)
			continue;

		if (!tb[DEVLINK_ATTR_INFO_VERSION_NAME] ||
		    !tb[DEVLINK_ATTR_INFO_VERSION_VALUE])
			continue;

		if (name) {
			pr_out_object_start(dl, name);
			name = NULL;
		}

		ver_name = mnl_attr_get_str(tb[DEVLINK_ATTR_INFO_VERSION_NAME]);
		ver_value = mnl_attr_get_str(tb[DEVLINK_ATTR_INFO_VERSION_VALUE]);

		check_indent_newline(dl);
		print_string_name_value(ver_name, ver_value);
		if (!dl->json_output)
			__pr_out_newline();
	}

	if (!name)
		pr_out_object_end(dl);
}

static void pr_out_info(struct dl *dl, const struct nlmsghdr *nlh,
			struct nlattr **tb, bool has_versions)
{
	__pr_out_handle_start(dl, tb, true, false);

	__pr_out_indent_inc();
	if (tb[DEVLINK_ATTR_INFO_DRIVER_NAME]) {
		struct nlattr *nla_drv = tb[DEVLINK_ATTR_INFO_DRIVER_NAME];

		if (!dl->json_output)
			__pr_out_newline();
		check_indent_newline(dl);
		print_string(PRINT_ANY, "driver", "driver %s",
			     mnl_attr_get_str(nla_drv));
	}

	if (tb[DEVLINK_ATTR_INFO_SERIAL_NUMBER]) {
		struct nlattr *nla_sn = tb[DEVLINK_ATTR_INFO_SERIAL_NUMBER];

		if (!dl->json_output)
			__pr_out_newline();
		check_indent_newline(dl);
		print_string(PRINT_ANY, "serial_number", "serial_number %s",
			     mnl_attr_get_str(nla_sn));
	}

	if (tb[DEVLINK_ATTR_INFO_BOARD_SERIAL_NUMBER]) {
		struct nlattr *nla_bsn = tb[DEVLINK_ATTR_INFO_BOARD_SERIAL_NUMBER];

		if (!dl->json_output)
			__pr_out_newline();
		check_indent_newline(dl);
		print_string(PRINT_ANY, "board.serial_number", "board.serial_number %s",
			     mnl_attr_get_str(nla_bsn));
	}
	__pr_out_indent_dec();

	if (has_versions) {
		pr_out_object_start(dl, "versions");

		pr_out_versions_single(dl, nlh, "fixed",
				       DEVLINK_ATTR_INFO_VERSION_FIXED);
		pr_out_versions_single(dl, nlh, "running",
				       DEVLINK_ATTR_INFO_VERSION_RUNNING);
		pr_out_versions_single(dl, nlh, "stored",
				       DEVLINK_ATTR_INFO_VERSION_STORED);

		pr_out_object_end(dl);
	}

	pr_out_handle_end(dl);
}

static int cmd_versions_show_cb(const struct nlmsghdr *nlh, void *data)
{
	struct genlmsghdr *genl = mnl_nlmsg_get_payload(nlh);
	struct nlattr *tb[DEVLINK_ATTR_MAX + 1] = {};
	bool has_versions, has_info;
	struct dl *dl = data;

	mnl_attr_parse(nlh, sizeof(*genl), attr_cb, tb);

	if (!tb[DEVLINK_ATTR_BUS_NAME] || !tb[DEVLINK_ATTR_DEV_NAME])
		return MNL_CB_ERROR;

	has_versions = tb[DEVLINK_ATTR_INFO_VERSION_FIXED] ||
		tb[DEVLINK_ATTR_INFO_VERSION_RUNNING] ||
		tb[DEVLINK_ATTR_INFO_VERSION_STORED];
	has_info = tb[DEVLINK_ATTR_INFO_DRIVER_NAME] ||
		tb[DEVLINK_ATTR_INFO_SERIAL_NUMBER] ||
		tb[DEVLINK_ATTR_INFO_BOARD_SERIAL_NUMBER] ||
		has_versions;

	if (has_info)
		pr_out_info(dl, nlh, tb, has_versions);

	return MNL_CB_OK;
}

static int cmd_dev_info(struct dl *dl)
{
	struct nlmsghdr *nlh;
	uint16_t flags = NLM_F_REQUEST | NLM_F_ACK;
	int err;

	if (dl_argv_match(dl, "help")) {
		cmd_dev_help();
		return 0;
	}

	if (dl_argc(dl) == 0)
		flags |= NLM_F_DUMP;

	nlh = mnlg_msg_prepare(dl->nlg, DEVLINK_CMD_INFO_GET, flags);

	if (dl_argc(dl) > 0) {
		err = dl_argv_parse_put(nlh, dl, DL_OPT_HANDLE, 0);
		if (err)
			return err;
	}

	pr_out_section_start(dl, "info");
	err = _mnlg_socket_sndrcv(dl->nlg, nlh, cmd_versions_show_cb, dl);
	pr_out_section_end(dl);
	return err;
}

struct cmd_dev_flash_status_ctx {
	struct dl *dl;
	struct timespec time_of_last_status;
	uint64_t status_msg_timeout;
	size_t elapsed_time_msg_len;
	char *last_msg;
	char *last_component;
	uint8_t not_first:1,
		last_pc:1,
		received_end:1,
		flash_done:1;
};

static int nullstrcmp(const char *str1, const char *str2)
{
	if (str1 && str2)
		return strcmp(str1, str2);
	if (!str1 && !str2)
		return 0;
	return str1 ? 1 : -1;
}

static void cmd_dev_flash_clear_elapsed_time(struct cmd_dev_flash_status_ctx *ctx)
{
	int i;

	for (i = 0; i < ctx->elapsed_time_msg_len; i++)
		pr_out_tty("\b \b");

	ctx->elapsed_time_msg_len = 0;
}

static int cmd_dev_flash_status_cb(const struct nlmsghdr *nlh, void *data)
{
	struct cmd_dev_flash_status_ctx *ctx = data;
	struct dl_opts *opts = &ctx->dl->opts;
	struct genlmsghdr *genl = mnl_nlmsg_get_payload(nlh);
	struct nlattr *tb[DEVLINK_ATTR_MAX + 1] = {};
	const char *component = NULL;
	uint64_t done = 0, total = 0;
	const char *msg = NULL;
	const char *bus_name;
	const char *dev_name;

	cmd_dev_flash_clear_elapsed_time(ctx);

	if (genl->cmd != DEVLINK_CMD_FLASH_UPDATE_STATUS &&
	    genl->cmd != DEVLINK_CMD_FLASH_UPDATE_END)
		return MNL_CB_STOP;

	mnl_attr_parse(nlh, sizeof(*genl), attr_cb, tb);
	if (!tb[DEVLINK_ATTR_BUS_NAME] || !tb[DEVLINK_ATTR_DEV_NAME])
		return MNL_CB_ERROR;
	bus_name = mnl_attr_get_str(tb[DEVLINK_ATTR_BUS_NAME]);
	dev_name = mnl_attr_get_str(tb[DEVLINK_ATTR_DEV_NAME]);
	if (strcmp(bus_name, opts->bus_name) ||
	    strcmp(dev_name, opts->dev_name))
		return MNL_CB_ERROR;

	if (genl->cmd == DEVLINK_CMD_FLASH_UPDATE_END && ctx->not_first) {
		pr_out("\n");
		free(ctx->last_msg);
		free(ctx->last_component);
		ctx->received_end = 1;
		return MNL_CB_STOP;
	}

	if (tb[DEVLINK_ATTR_FLASH_UPDATE_STATUS_MSG])
		msg = mnl_attr_get_str(tb[DEVLINK_ATTR_FLASH_UPDATE_STATUS_MSG]);
	if (tb[DEVLINK_ATTR_FLASH_UPDATE_COMPONENT])
		component = mnl_attr_get_str(tb[DEVLINK_ATTR_FLASH_UPDATE_COMPONENT]);
	if (tb[DEVLINK_ATTR_FLASH_UPDATE_STATUS_DONE])
		done = mnl_attr_get_u64(tb[DEVLINK_ATTR_FLASH_UPDATE_STATUS_DONE]);
	if (tb[DEVLINK_ATTR_FLASH_UPDATE_STATUS_TOTAL])
		total = mnl_attr_get_u64(tb[DEVLINK_ATTR_FLASH_UPDATE_STATUS_TOTAL]);
	if (tb[DEVLINK_ATTR_FLASH_UPDATE_STATUS_TIMEOUT])
		ctx->status_msg_timeout = mnl_attr_get_u64(tb[DEVLINK_ATTR_FLASH_UPDATE_STATUS_TIMEOUT]);
	else
		ctx->status_msg_timeout = 0;

	if (!nullstrcmp(msg, ctx->last_msg) &&
	    !nullstrcmp(component, ctx->last_component) &&
	    ctx->last_pc && ctx->not_first) {
		pr_out_tty("\b\b\b\b\b"); /* clean percentage */
	} else {
		/* only update the last status timestamp if the message changed */
		clock_gettime(CLOCK_MONOTONIC, &ctx->time_of_last_status);

		if (ctx->not_first)
			pr_out("\n");
		if (component) {
			pr_out("[%s] ", component);
			free(ctx->last_component);
			ctx->last_component = strdup(component);
		}
		if (msg) {
			pr_out("%s", msg);
			free(ctx->last_msg);
			ctx->last_msg = strdup(msg);
		}
	}
	if (total) {
		pr_out_tty(" %3lu%%", (done * 100) / total);
		ctx->last_pc = 1;
	} else {
		ctx->last_pc = 0;
	}
	fflush(stdout);
	ctx->not_first = 1;

	return MNL_CB_STOP;
}

static void cmd_dev_flash_time_elapsed(struct cmd_dev_flash_status_ctx *ctx)
{
	struct timespec now, res;

	clock_gettime(CLOCK_MONOTONIC, &now);

	res.tv_sec = now.tv_sec - ctx->time_of_last_status.tv_sec;
	res.tv_nsec = now.tv_nsec - ctx->time_of_last_status.tv_nsec;
	if (res.tv_nsec < 0) {
		res.tv_sec--;
		res.tv_nsec += 1000000000L;
	}

	/* Only begin displaying an elapsed time message if we've waited a few
	 * seconds with no response, or the status message included a timeout
	 * value.
	 */
	if (res.tv_sec > 2 || ctx->status_msg_timeout) {
		uint64_t elapsed_m, elapsed_s;
		char msg[128];
		size_t len;

		/* clear the last elapsed time message, if we have one */
		cmd_dev_flash_clear_elapsed_time(ctx);

		elapsed_m = res.tv_sec / 60;
		elapsed_s = res.tv_sec % 60;

		/**
		 * If we've elapsed a few seconds without receiving any status
		 * notification from the device, we display a time elapsed
		 * message. This has a few possible formats:
		 *
		 * 1) just time elapsed, when no timeout was provided
		 *    " ( Xm Ys )"
		 * 2) time elapsed out of a timeout that came from the device
		 *    driver via DEVLINK_CMD_FLASH_UPDATE_STATUS_TIMEOUT
		 *    " ( Xm Ys : Am Ys)"
		 * 3) time elapsed if we still receive no status after
		 *    reaching the provided timeout.
		 *    " ( Xm Ys : timeout reached )"
		 */
		if (!ctx->status_msg_timeout) {
			len = snprintf(msg, sizeof(msg),
				       " ( %lum %lus )", elapsed_m, elapsed_s);
		} else if (res.tv_sec <= ctx->status_msg_timeout) {
			uint64_t timeout_m, timeout_s;

			timeout_m = ctx->status_msg_timeout / 60;
			timeout_s = ctx->status_msg_timeout % 60;

			len = snprintf(msg, sizeof(msg),
				       " ( %lum %lus : %lum %lus )",
				       elapsed_m, elapsed_s, timeout_m, timeout_s);
		} else {
			len = snprintf(msg, sizeof(msg),
				       " ( %lum %lus : timeout reached )", elapsed_m, elapsed_s);
		}

		ctx->elapsed_time_msg_len = len;

		pr_out_tty("%s", msg);
		fflush(stdout);
	}
}

static int cmd_dev_flash_fds_process(struct cmd_dev_flash_status_ctx *ctx,
				     struct mnlg_socket *nlg_ntf,
				     int pipe_r)
{
	int nlfd = mnlg_socket_get_fd(nlg_ntf);
	struct timeval timeout;
	fd_set fds[3];
	int fdmax;
	int i;
	int err;
	int err2;

	for (i = 0; i < 3; i++)
		FD_ZERO(&fds[i]);
	FD_SET(pipe_r, &fds[0]);
	fdmax = pipe_r + 1;
	FD_SET(nlfd, &fds[0]);
	if (nlfd >= fdmax)
		fdmax = nlfd + 1;

	/* select only for a short while (1/10th of a second) in order to
	 * allow periodically updating the screen with an elapsed time
	 * indicator.
	 */
	timeout.tv_sec = 0;
	timeout.tv_usec = 100000;

	while (select(fdmax, &fds[0], &fds[1], &fds[2], &timeout) < 0) {
		if (errno == EINTR)
			continue;
		pr_err("select() failed\n");
		return -errno;
	}
	if (FD_ISSET(nlfd, &fds[0])) {
		err = _mnlg_socket_recv_run(nlg_ntf,
					    cmd_dev_flash_status_cb, ctx);
		if (err)
			return err;
	}
	if (FD_ISSET(pipe_r, &fds[0])) {
		err = read(pipe_r, &err2, sizeof(err2));
		if (err == -1) {
			pr_err("Failed to read pipe\n");
			return -errno;
		}
		if (err2)
			return err2;
		ctx->flash_done = 1;
	}
	cmd_dev_flash_time_elapsed(ctx);
	return 0;
}


static int cmd_dev_flash(struct dl *dl)
{
	struct cmd_dev_flash_status_ctx ctx = {.dl = dl,};
	struct mnlg_socket *nlg_ntf;
	struct nlmsghdr *nlh;
	int pipe_r, pipe_w;
	int pipe_fds[2];
	pid_t pid;
	int err;

	if (dl_argv_match(dl, "help") || dl_no_arg(dl)) {
		cmd_dev_help();
		return 0;
	}

	nlh = mnlg_msg_prepare(dl->nlg, DEVLINK_CMD_FLASH_UPDATE,
			       NLM_F_REQUEST | NLM_F_ACK);

	err = dl_argv_parse_put(nlh, dl, DL_OPT_HANDLE | DL_OPT_FLASH_FILE_NAME,
				DL_OPT_FLASH_COMPONENT | DL_OPT_FLASH_OVERWRITE);
	if (err)
		return err;

	nlg_ntf = mnlg_socket_open(DEVLINK_GENL_NAME, DEVLINK_GENL_VERSION);
	if (!nlg_ntf)
		return err;

	err = _mnlg_socket_group_add(nlg_ntf, DEVLINK_GENL_MCGRP_CONFIG_NAME);
	if (err)
		goto err_socket;

	err = pipe(pipe_fds);
	if (err == -1) {
		err = -errno;
		goto err_socket;
	}
	pipe_r = pipe_fds[0];
	pipe_w = pipe_fds[1];

	pid = fork();
	if (pid == -1) {
		close(pipe_w);
		err = -errno;
		goto out;
	} else if (!pid) {
		/* In child, just execute the flash and pass returned
		 * value through pipe once it is done.
		 */
		int cc;

		close(pipe_r);
		err = _mnlg_socket_send(dl->nlg, nlh);
		cc = write(pipe_w, &err, sizeof(err));
		close(pipe_w);
		exit(cc != sizeof(err));
	}
	close(pipe_w);

	/* initialize starting time to allow comparison for when to begin
	 * displaying a time elapsed message.
	 */
	clock_gettime(CLOCK_MONOTONIC, &ctx.time_of_last_status);

	do {
		err = cmd_dev_flash_fds_process(&ctx, nlg_ntf, pipe_r);
		if (err)
			goto out;
	} while (!ctx.flash_done || (ctx.not_first && !ctx.received_end));

	err = _mnlg_socket_recv_run(dl->nlg, NULL, NULL);
out:
	close(pipe_r);
err_socket:
	mnlg_socket_close(nlg_ntf);
	return err;
}

static int cmd_dev(struct dl *dl)
{
	if (dl_argv_match(dl, "help")) {
		cmd_dev_help();
		return 0;
	} else if (dl_argv_match(dl, "show") ||
		   dl_argv_match(dl, "list") || dl_no_arg(dl)) {
		dl_arg_inc(dl);
		return cmd_dev_show(dl);
	} else if (dl_argv_match(dl, "eswitch")) {
		dl_arg_inc(dl);
		return cmd_dev_eswitch(dl);
	} else if (dl_argv_match(dl, "reload")) {
		dl_arg_inc(dl);
		return cmd_dev_reload(dl);
	} else if (dl_argv_match(dl, "param")) {
		dl_arg_inc(dl);
		return cmd_dev_param(dl);
	} else if (dl_argv_match(dl, "info")) {
		dl_arg_inc(dl);
		return cmd_dev_info(dl);
	} else if (dl_argv_match(dl, "flash")) {
		dl_arg_inc(dl);
		return cmd_dev_flash(dl);
	}
	pr_err("Command \"%s\" not found\n", dl_argv(dl));
	return -ENOENT;
}

static void cmd_port_help(void)
{
	pr_err("Usage: devlink port show [ DEV/PORT_INDEX ]\n");
	pr_err("       devlink port set DEV/PORT_INDEX [ type { eth | ib | auto} ]\n");
	pr_err("       devlink port split DEV/PORT_INDEX count COUNT\n");
	pr_err("       devlink port unsplit DEV/PORT_INDEX\n");
	pr_err("       devlink port function set DEV/PORT_INDEX [ hw_addr ADDR ] [ state STATE ]\n");
	pr_err("       devlink port param set DEV/PORT_INDEX name PARAMETER value VALUE cmode { permanent | driverinit | runtime }\n");
	pr_err("       devlink port param show [DEV/PORT_INDEX name PARAMETER]\n");
	pr_err("       devlink port health show [ DEV/PORT_INDEX reporter REPORTER_NAME ]\n");
	pr_err("       devlink port add DEV/PORT_INDEX flavour FLAVOUR pfnum PFNUM [ sfnum SFNUM ]\n");
	pr_err("       devlink port del DEV/PORT_INDEX\n");
}

static const char *port_type_name(uint32_t type)
{
	switch (type) {
	case DEVLINK_PORT_TYPE_NOTSET: return "notset";
	case DEVLINK_PORT_TYPE_AUTO: return "auto";
	case DEVLINK_PORT_TYPE_ETH: return "eth";
	case DEVLINK_PORT_TYPE_IB: return "ib";
	default: return "<unknown type>";
	}
}

static const char *port_flavour_name(uint16_t flavour)
{
	const char *str;

	str = str_map_lookup_u16(port_flavour_map, flavour);
	return str ? str : "<unknown flavour>";
}

static void pr_out_port_pfvfsf_num(struct dl *dl, struct nlattr **tb)
{
	uint16_t fn_num;

	if (tb[DEVLINK_ATTR_PORT_CONTROLLER_NUMBER])
		print_uint(PRINT_ANY, "controller", " controller %u",
			   mnl_attr_get_u32(tb[DEVLINK_ATTR_PORT_CONTROLLER_NUMBER]));
	if (tb[DEVLINK_ATTR_PORT_PCI_PF_NUMBER]) {
		fn_num = mnl_attr_get_u16(tb[DEVLINK_ATTR_PORT_PCI_PF_NUMBER]);
		print_uint(PRINT_ANY, "pfnum", " pfnum %u", fn_num);
	}
	if (tb[DEVLINK_ATTR_PORT_PCI_VF_NUMBER]) {
		fn_num = mnl_attr_get_u16(tb[DEVLINK_ATTR_PORT_PCI_VF_NUMBER]);
		print_uint(PRINT_ANY, "vfnum", " vfnum %u", fn_num);
	}
	if (tb[DEVLINK_ATTR_PORT_PCI_SF_NUMBER]) {
		fn_num = mnl_attr_get_u32(tb[DEVLINK_ATTR_PORT_PCI_SF_NUMBER]);
		print_uint(PRINT_ANY, "sfnum", " sfnum %u", fn_num);
	}
	if (tb[DEVLINK_ATTR_PORT_EXTERNAL]) {
		uint8_t external;

		external = mnl_attr_get_u8(tb[DEVLINK_ATTR_PORT_EXTERNAL]);
		print_bool(PRINT_ANY, "external", " external %s", external);
	}
}

static const char *port_fn_state(uint8_t state)
{
	const char *str;

	str = str_map_lookup_u8(port_fn_state_map, state);
	return str ? str : "<unknown state>";
}

static const char *port_fn_opstate(uint8_t state)
{
	const char *str;

	str = str_map_lookup_u8(port_fn_opstate_map, state);
	return str ? str : "<unknown state>";
}

static void pr_out_port_function(struct dl *dl, struct nlattr **tb_port)
{
	struct nlattr *tb[DEVLINK_PORT_FUNCTION_ATTR_MAX + 1] = {};
	unsigned char *data;
	SPRINT_BUF(hw_addr);
	uint32_t len;
	int err;

	if (!tb_port[DEVLINK_ATTR_PORT_FUNCTION])
		return;

	err = mnl_attr_parse_nested(tb_port[DEVLINK_ATTR_PORT_FUNCTION],
				    function_attr_cb, tb);
	if (err != MNL_CB_OK)
		return;

	pr_out_object_start(dl, "function");
	check_indent_newline(dl);

	if (tb[DEVLINK_PORT_FUNCTION_ATTR_HW_ADDR]) {
		len = mnl_attr_get_payload_len(tb[DEVLINK_PORT_FUNCTION_ATTR_HW_ADDR]);
		data = mnl_attr_get_payload(tb[DEVLINK_PORT_FUNCTION_ATTR_HW_ADDR]);

		print_string(PRINT_ANY, "hw_addr", "hw_addr %s",
			     ll_addr_n2a(data, len, 0, hw_addr, sizeof(hw_addr)));
	}
	if (tb[DEVLINK_PORT_FN_ATTR_STATE]) {
		uint8_t state;

		state = mnl_attr_get_u8(tb[DEVLINK_PORT_FN_ATTR_STATE]);

		print_string(PRINT_ANY, "state", " state %s",
			     port_fn_state(state));
	}
	if (tb[DEVLINK_PORT_FN_ATTR_OPSTATE]) {
		uint8_t state;

		state = mnl_attr_get_u8(tb[DEVLINK_PORT_FN_ATTR_OPSTATE]);

		print_string(PRINT_ANY, "opstate", " opstate %s",
			     port_fn_opstate(state));
	}

	if (!dl->json_output)
		__pr_out_indent_dec();
	pr_out_object_end(dl);
}

static void pr_out_port(struct dl *dl, struct nlattr **tb)
{
	struct nlattr *pt_attr = tb[DEVLINK_ATTR_PORT_TYPE];
	struct nlattr *dpt_attr = tb[DEVLINK_ATTR_PORT_DESIRED_TYPE];

	pr_out_port_handle_start(dl, tb, false);
	check_indent_newline(dl);
	if (pt_attr) {
		uint16_t port_type = mnl_attr_get_u16(pt_attr);

		print_string(PRINT_ANY, "type", "type %s",
			     port_type_name(port_type));
		if (dpt_attr) {
			uint16_t des_port_type = mnl_attr_get_u16(dpt_attr);

			if (port_type != des_port_type)
				print_string(PRINT_ANY, "des_type", " des_type %s",
					     port_type_name(des_port_type));
		}
	}
	if (tb[DEVLINK_ATTR_PORT_NETDEV_NAME]) {
		print_string(PRINT_ANY, "netdev", " netdev %s",
			     mnl_attr_get_str(tb[DEVLINK_ATTR_PORT_NETDEV_NAME]));
	}
	if (tb[DEVLINK_ATTR_PORT_IBDEV_NAME]) {
		print_string(PRINT_ANY, "ibdev", " ibdev %s",
			     mnl_attr_get_str(tb[DEVLINK_ATTR_PORT_IBDEV_NAME]));
		}
	if (tb[DEVLINK_ATTR_PORT_FLAVOUR]) {
		uint16_t port_flavour =
				mnl_attr_get_u16(tb[DEVLINK_ATTR_PORT_FLAVOUR]);

		print_string(PRINT_ANY, "flavour", " flavour %s",
			     port_flavour_name(port_flavour));

		switch (port_flavour) {
		case DEVLINK_PORT_FLAVOUR_PCI_PF:
		case DEVLINK_PORT_FLAVOUR_PCI_VF:
		case DEVLINK_PORT_FLAVOUR_PCI_SF:
			pr_out_port_pfvfsf_num(dl, tb);
			break;
		default:
			break;
		}
	}
	if (tb[DEVLINK_ATTR_PORT_NUMBER]) {
		uint32_t port_number;

		port_number = mnl_attr_get_u32(tb[DEVLINK_ATTR_PORT_NUMBER]);
		print_uint(PRINT_ANY, "port", " port %u", port_number);
	}
	if (tb[DEVLINK_ATTR_PORT_SPLIT_GROUP])
		print_uint(PRINT_ANY, "split_group", " split_group %u",
			   mnl_attr_get_u32(tb[DEVLINK_ATTR_PORT_SPLIT_GROUP]));
	if (tb[DEVLINK_ATTR_PORT_SPLITTABLE])
		print_bool(PRINT_ANY, "splittable", " splittable %s",
			   mnl_attr_get_u8(tb[DEVLINK_ATTR_PORT_SPLITTABLE]));
	if (tb[DEVLINK_ATTR_PORT_LANES])
		print_uint(PRINT_ANY, "lanes", " lanes %u",
			   mnl_attr_get_u32(tb[DEVLINK_ATTR_PORT_LANES]));

	pr_out_port_function(dl, tb);
	pr_out_port_handle_end(dl);
}

static int cmd_port_show_cb(const struct nlmsghdr *nlh, void *data)
{
	struct dl *dl = data;
	struct nlattr *tb[DEVLINK_ATTR_MAX + 1] = {};
	struct genlmsghdr *genl = mnl_nlmsg_get_payload(nlh);

	mnl_attr_parse(nlh, sizeof(*genl), attr_cb, tb);
	if (!tb[DEVLINK_ATTR_BUS_NAME] || !tb[DEVLINK_ATTR_DEV_NAME] ||
	    !tb[DEVLINK_ATTR_PORT_INDEX])
		return MNL_CB_ERROR;
	pr_out_port(dl, tb);
	return MNL_CB_OK;
}

static int cmd_port_show(struct dl *dl)
{
	struct nlmsghdr *nlh;
	uint16_t flags = NLM_F_REQUEST | NLM_F_ACK;
	int err;

	if (dl_argc(dl) == 0)
		flags |= NLM_F_DUMP;

	nlh = mnlg_msg_prepare(dl->nlg, DEVLINK_CMD_PORT_GET, flags);

	if (dl_argc(dl) > 0) {
		err = dl_argv_parse_put(nlh, dl, DL_OPT_HANDLEP, 0);
		if (err)
			return err;
	}

	pr_out_section_start(dl, "port");
	err = _mnlg_socket_sndrcv(dl->nlg, nlh, cmd_port_show_cb, dl);
	pr_out_section_end(dl);
	return err;
}

static int cmd_port_set(struct dl *dl)
{
	struct nlmsghdr *nlh;
	int err;

	nlh = mnlg_msg_prepare(dl->nlg, DEVLINK_CMD_PORT_SET,
			       NLM_F_REQUEST | NLM_F_ACK);

	err = dl_argv_parse_put(nlh, dl, DL_OPT_HANDLEP | DL_OPT_PORT_TYPE, 0);
	if (err)
		return err;

	return _mnlg_socket_sndrcv(dl->nlg, nlh, NULL, NULL);
}

static int cmd_port_split(struct dl *dl)
{
	struct nlmsghdr *nlh;
	int err;

	nlh = mnlg_msg_prepare(dl->nlg, DEVLINK_CMD_PORT_SPLIT,
			       NLM_F_REQUEST | NLM_F_ACK);

	err = dl_argv_parse_put(nlh, dl, DL_OPT_HANDLEP | DL_OPT_PORT_COUNT, 0);
	if (err)
		return err;

	return _mnlg_socket_sndrcv(dl->nlg, nlh, NULL, NULL);
}

static int cmd_port_unsplit(struct dl *dl)
{
	struct nlmsghdr *nlh;
	int err;

	nlh = mnlg_msg_prepare(dl->nlg, DEVLINK_CMD_PORT_UNSPLIT,
			       NLM_F_REQUEST | NLM_F_ACK);

	err = dl_argv_parse_put(nlh, dl, DL_OPT_HANDLEP, 0);
	if (err)
		return err;

	return _mnlg_socket_sndrcv(dl->nlg, nlh, NULL, NULL);
}

static int cmd_port_param_show(struct dl *dl)
{
	uint16_t flags = NLM_F_REQUEST | NLM_F_ACK;
	struct nlmsghdr *nlh;
	int err;

	if (dl_argc(dl) == 0)
		flags |= NLM_F_DUMP;

	nlh = mnlg_msg_prepare(dl->nlg, DEVLINK_CMD_PORT_PARAM_GET, flags);

	if (dl_argc(dl) > 0) {
		err = dl_argv_parse_put(nlh, dl, DL_OPT_HANDLEP |
					DL_OPT_PARAM_NAME, 0);
		if (err)
			return err;
	}

	pr_out_section_start(dl, "param");
	err = _mnlg_socket_sndrcv(dl->nlg, nlh, cmd_port_param_show_cb, dl);
	pr_out_section_end(dl);

	return err;
}

static void cmd_port_function_help(void)
{
	pr_err("Usage: devlink port function set DEV/PORT_INDEX [ hw_addr ADDR ] [ state STATE ]\n");
}

static int cmd_port_function_set(struct dl *dl)
{
	struct nlmsghdr *nlh;
	int err;

	if (dl_no_arg(dl)) {
		cmd_port_function_help();
		return 0;
	}
	nlh = mnlg_msg_prepare(dl->nlg, DEVLINK_CMD_PORT_SET, NLM_F_REQUEST | NLM_F_ACK);

	err = dl_argv_parse_put(nlh, dl, DL_OPT_HANDLEP,
				DL_OPT_PORT_FUNCTION_HW_ADDR | DL_OPT_PORT_FUNCTION_STATE);
	if (err)
		return err;

	return _mnlg_socket_sndrcv(dl->nlg, nlh, NULL, NULL);
}

static int cmd_port_param_set_cb(const struct nlmsghdr *nlh, void *data)
{
	struct genlmsghdr *genl = mnl_nlmsg_get_payload(nlh);
	struct nlattr *nla_param[DEVLINK_ATTR_MAX + 1] = {};
	struct nlattr *tb[DEVLINK_ATTR_MAX + 1] = {};
	struct nlattr *param_value_attr;
	enum devlink_param_cmode cmode;
	struct param_ctx *ctx = data;
	struct dl *dl = ctx->dl;
	int nla_type;
	int err;

	mnl_attr_parse(nlh, sizeof(*genl), attr_cb, tb);
	if (!tb[DEVLINK_ATTR_BUS_NAME] || !tb[DEVLINK_ATTR_DEV_NAME] ||
	    !tb[DEVLINK_ATTR_PORT_INDEX] || !tb[DEVLINK_ATTR_PARAM])
		return MNL_CB_ERROR;

	err = mnl_attr_parse_nested(tb[DEVLINK_ATTR_PARAM], attr_cb, nla_param);
	if (err != MNL_CB_OK)
		return MNL_CB_ERROR;

	if (!nla_param[DEVLINK_ATTR_PARAM_TYPE] ||
	    !nla_param[DEVLINK_ATTR_PARAM_VALUES_LIST])
		return MNL_CB_ERROR;

	nla_type = mnl_attr_get_u8(nla_param[DEVLINK_ATTR_PARAM_TYPE]);
	mnl_attr_for_each_nested(param_value_attr,
				 nla_param[DEVLINK_ATTR_PARAM_VALUES_LIST]) {
		struct nlattr *nla_value[DEVLINK_ATTR_MAX + 1] = {};
		struct nlattr *val_attr;

		err = mnl_attr_parse_nested(param_value_attr,
					    attr_cb, nla_value);
		if (err != MNL_CB_OK)
			return MNL_CB_ERROR;

		if (!nla_value[DEVLINK_ATTR_PARAM_VALUE_CMODE] ||
		    (nla_type != MNL_TYPE_FLAG &&
		     !nla_value[DEVLINK_ATTR_PARAM_VALUE_DATA]))
			return MNL_CB_ERROR;

		cmode = mnl_attr_get_u8(nla_value[DEVLINK_ATTR_PARAM_VALUE_CMODE]);
		if (cmode == dl->opts.cmode) {
			val_attr = nla_value[DEVLINK_ATTR_PARAM_VALUE_DATA];
			switch (nla_type) {
			case MNL_TYPE_U8:
				ctx->value.vu8 = mnl_attr_get_u8(val_attr);
				break;
			case MNL_TYPE_U16:
				ctx->value.vu16 = mnl_attr_get_u16(val_attr);
				break;
			case MNL_TYPE_U32:
				ctx->value.vu32 = mnl_attr_get_u32(val_attr);
				break;
			case MNL_TYPE_STRING:
				ctx->value.vstr = mnl_attr_get_str(val_attr);
				break;
			case MNL_TYPE_FLAG:
				ctx->value.vbool = val_attr ? true : false;
				break;
			}
			break;
		}
	}
	ctx->nla_type = nla_type;
	return MNL_CB_OK;
}

static int cmd_port_param_set(struct dl *dl)
{
	struct param_ctx ctx = {};
	struct nlmsghdr *nlh;
	bool conv_exists;
	uint32_t val_u32 = 0;
	uint16_t val_u16;
	uint8_t val_u8;
	bool val_bool;
	int err;

	err = dl_argv_parse(dl, DL_OPT_HANDLEP |
			    DL_OPT_PARAM_NAME |
			    DL_OPT_PARAM_VALUE |
			    DL_OPT_PARAM_CMODE, 0);
	if (err)
		return err;

	/* Get value type */
	nlh = mnlg_msg_prepare(dl->nlg, DEVLINK_CMD_PORT_PARAM_GET,
			       NLM_F_REQUEST | NLM_F_ACK);
	dl_opts_put(nlh, dl);

	ctx.dl = dl;
	err = _mnlg_socket_sndrcv(dl->nlg, nlh, cmd_port_param_set_cb, &ctx);
	if (err)
		return err;

	nlh = mnlg_msg_prepare(dl->nlg, DEVLINK_CMD_PORT_PARAM_SET,
			       NLM_F_REQUEST | NLM_F_ACK);
	dl_opts_put(nlh, dl);

	conv_exists = param_val_conv_exists(param_val_conv, PARAM_VAL_CONV_LEN,
					    dl->opts.param_name);

	mnl_attr_put_u8(nlh, DEVLINK_ATTR_PARAM_TYPE, ctx.nla_type);
	switch (ctx.nla_type) {
	case MNL_TYPE_U8:
		if (conv_exists) {
			err = param_val_conv_uint_get(param_val_conv,
						      PARAM_VAL_CONV_LEN,
						      dl->opts.param_name,
						      dl->opts.param_value,
						      &val_u32);
			val_u8 = val_u32;
		} else {
			err = strtouint8_t(dl->opts.param_value, &val_u8);
		}
		if (err)
			goto err_param_value_parse;
		if (val_u8 == ctx.value.vu8)
			return 0;
		mnl_attr_put_u8(nlh, DEVLINK_ATTR_PARAM_VALUE_DATA, val_u8);
		break;
	case MNL_TYPE_U16:
		if (conv_exists) {
			err = param_val_conv_uint_get(param_val_conv,
						      PARAM_VAL_CONV_LEN,
						      dl->opts.param_name,
						      dl->opts.param_value,
						      &val_u32);
			val_u16 = val_u32;
		} else {
			err = strtouint16_t(dl->opts.param_value, &val_u16);
		}
		if (err)
			goto err_param_value_parse;
		if (val_u16 == ctx.value.vu16)
			return 0;
		mnl_attr_put_u16(nlh, DEVLINK_ATTR_PARAM_VALUE_DATA, val_u16);
		break;
	case MNL_TYPE_U32:
		if (conv_exists)
			err = param_val_conv_uint_get(param_val_conv,
						      PARAM_VAL_CONV_LEN,
						      dl->opts.param_name,
						      dl->opts.param_value,
						      &val_u32);
		else
			err = strtouint32_t(dl->opts.param_value, &val_u32);
		if (err)
			goto err_param_value_parse;
		if (val_u32 == ctx.value.vu32)
			return 0;
		mnl_attr_put_u32(nlh, DEVLINK_ATTR_PARAM_VALUE_DATA, val_u32);
		break;
	case MNL_TYPE_FLAG:
		err = strtobool(dl->opts.param_value, &val_bool);
		if (err)
			goto err_param_value_parse;
		if (val_bool == ctx.value.vbool)
			return 0;
		if (val_bool)
			mnl_attr_put(nlh, DEVLINK_ATTR_PARAM_VALUE_DATA,
				     0, NULL);
		break;
	case MNL_TYPE_STRING:
		mnl_attr_put_strz(nlh, DEVLINK_ATTR_PARAM_VALUE_DATA,
				  dl->opts.param_value);
		if (!strcmp(dl->opts.param_value, ctx.value.vstr))
			return 0;
		break;
	default:
		printf("Value type not supported\n");
		return -ENOTSUP;
	}
	return _mnlg_socket_sndrcv(dl->nlg, nlh, NULL, NULL);

err_param_value_parse:
	pr_err("Value \"%s\" is not a number or not within range\n",
	       dl->opts.param_value);
	return err;
}

static int cmd_port_param(struct dl *dl)
{
	if (dl_argv_match(dl, "help")) {
		cmd_port_help();
		return 0;
	} else if (dl_argv_match(dl, "show") ||
		   dl_argv_match(dl, "list") || dl_no_arg(dl)) {
		dl_arg_inc(dl);
		return cmd_port_param_show(dl);
	} else if (dl_argv_match(dl, "set")) {
		dl_arg_inc(dl);
		return cmd_port_param_set(dl);
	}
	pr_err("Command \"%s\" not found\n", dl_argv(dl));
	return -ENOENT;
}

static int cmd_port_function(struct dl *dl)
{
	if (dl_argv_match(dl, "help") || dl_no_arg(dl)) {
		cmd_port_function_help();
		return 0;
	} else if (dl_argv_match(dl, "set")) {
		dl_arg_inc(dl);
		return cmd_port_function_set(dl);
	}
	pr_err("Command \"%s\" not found\n", dl_argv(dl));
	return -ENOENT;
}

static int cmd_health(struct dl *dl);
static int __cmd_health_show(struct dl *dl, bool show_device, bool show_port);

static void cmd_port_add_help(void)
{
	pr_err("       devlink port add { DEV | DEV/PORT_INDEX } flavour FLAVOUR pfnum PFNUM [ sfnum SFNUM ]\n");
}

static int cmd_port_add(struct dl *dl)
{
	struct nlmsghdr *nlh;
	int err;

	if (dl_argv_match(dl, "help") || dl_no_arg(dl)) {
		cmd_port_add_help();
		return 0;
	}

	nlh = mnlg_msg_prepare(dl->nlg, DEVLINK_CMD_PORT_NEW,
			       NLM_F_REQUEST | NLM_F_ACK);

	err = dl_argv_parse_put(nlh, dl, DL_OPT_HANDLE | DL_OPT_HANDLEP |
				DL_OPT_PORT_FLAVOUR | DL_OPT_PORT_PFNUMBER,
				DL_OPT_PORT_SFNUMBER);
	if (err)
		return err;

	return _mnlg_socket_sndrcv(dl->nlg, nlh, cmd_port_show_cb, dl);
}

static void cmd_port_del_help(void)
{
	pr_err("       devlink port del DEV/PORT_INDEX\n");
}

static int cmd_port_del(struct dl *dl)
{
	struct nlmsghdr *nlh;
	int err;

	if (dl_argv_match(dl, "help") || dl_no_arg(dl)) {
		cmd_port_del_help();
		return 0;
	}

	nlh = mnlg_msg_prepare(dl->nlg, DEVLINK_CMD_PORT_DEL,
			       NLM_F_REQUEST | NLM_F_ACK);

	err = dl_argv_parse_put(nlh, dl, DL_OPT_HANDLEP, 0);
	if (err)
		return err;

	return _mnlg_socket_sndrcv(dl->nlg, nlh, NULL, NULL);
}

static int cmd_port(struct dl *dl)
{
	if (dl_argv_match(dl, "help")) {
		cmd_port_help();
		return 0;
	} else if (dl_argv_match(dl, "show") ||
		   dl_argv_match(dl, "list") ||  dl_no_arg(dl)) {
		dl_arg_inc(dl);
		return cmd_port_show(dl);
	} else if (dl_argv_match(dl, "set")) {
		dl_arg_inc(dl);
		return cmd_port_set(dl);
	} else if (dl_argv_match(dl, "split")) {
		dl_arg_inc(dl);
		return cmd_port_split(dl);
	} else if (dl_argv_match(dl, "unsplit")) {
		dl_arg_inc(dl);
		return cmd_port_unsplit(dl);
	} else if (dl_argv_match(dl, "param")) {
		dl_arg_inc(dl);
		return cmd_port_param(dl);
	} else if (dl_argv_match(dl, "function")) {
		dl_arg_inc(dl);
		return cmd_port_function(dl);
	} else if (dl_argv_match(dl, "health")) {
		dl_arg_inc(dl);
		if (dl_argv_match(dl, "list") || dl_no_arg(dl)
		    || (dl_argv_match(dl, "show") && dl_argc(dl) == 1)) {
			dl_arg_inc(dl);
			return __cmd_health_show(dl, false, true);
		} else {
			return cmd_health(dl);
		}
	} else if (dl_argv_match(dl, "add")) {
		dl_arg_inc(dl);
		return cmd_port_add(dl);
	} else if (dl_argv_match(dl, "del")) {
		dl_arg_inc(dl);
		return cmd_port_del(dl);
	}

	pr_err("Command \"%s\" not found\n", dl_argv(dl));
	return -ENOENT;
}

static void cmd_sb_help(void)
{
	pr_err("Usage: devlink sb show [ DEV [ sb SB_INDEX ] ]\n");
	pr_err("       devlink sb pool show [ DEV [ sb SB_INDEX ] pool POOL_INDEX ]\n");
	pr_err("       devlink sb pool set DEV [ sb SB_INDEX ] pool POOL_INDEX\n");
	pr_err("                           size POOL_SIZE thtype { static | dynamic }\n");
	pr_err("       devlink sb port pool show [ DEV/PORT_INDEX [ sb SB_INDEX ]\n");
	pr_err("                                   pool POOL_INDEX ]\n");
	pr_err("       devlink sb port pool set DEV/PORT_INDEX [ sb SB_INDEX ]\n");
	pr_err("                                pool POOL_INDEX th THRESHOLD\n");
	pr_err("       devlink sb tc bind show [ DEV/PORT_INDEX [ sb SB_INDEX ] tc TC_INDEX\n");
	pr_err("                                 type { ingress | egress } ]\n");
	pr_err("       devlink sb tc bind set DEV/PORT_INDEX [ sb SB_INDEX ] tc TC_INDEX\n");
	pr_err("                              type { ingress | egress } pool POOL_INDEX\n");
	pr_err("                              th THRESHOLD\n");
	pr_err("       devlink sb occupancy show { DEV | DEV/PORT_INDEX } [ sb SB_INDEX ]\n");
	pr_err("       devlink sb occupancy snapshot DEV [ sb SB_INDEX ]\n");
	pr_err("       devlink sb occupancy clearmax DEV [ sb SB_INDEX ]\n");
}

static void pr_out_sb(struct dl *dl, struct nlattr **tb)
{
	pr_out_handle_start_arr(dl, tb);
	check_indent_newline(dl);
	print_uint(PRINT_ANY, "sb", "sb %u",
		   mnl_attr_get_u32(tb[DEVLINK_ATTR_SB_INDEX]));
	print_uint(PRINT_ANY, "size", " size %u",
		   mnl_attr_get_u32(tb[DEVLINK_ATTR_SB_SIZE]));
	print_uint(PRINT_ANY, "ing_pools", " ing_pools %u",
		   mnl_attr_get_u16(tb[DEVLINK_ATTR_SB_INGRESS_POOL_COUNT]));
	print_uint(PRINT_ANY, "eg_pools", " eg_pools %u",
		   mnl_attr_get_u16(tb[DEVLINK_ATTR_SB_EGRESS_POOL_COUNT]));
	print_uint(PRINT_ANY, "ing_tcs", " ing_tcs %u",
		   mnl_attr_get_u16(tb[DEVLINK_ATTR_SB_INGRESS_TC_COUNT]));
	print_uint(PRINT_ANY, "eg_tcs", " eg_tcs %u",
		   mnl_attr_get_u16(tb[DEVLINK_ATTR_SB_EGRESS_TC_COUNT]));
	pr_out_handle_end(dl);
}

static int cmd_sb_show_cb(const struct nlmsghdr *nlh, void *data)
{
	struct dl *dl = data;
	struct nlattr *tb[DEVLINK_ATTR_MAX + 1] = {};
	struct genlmsghdr *genl = mnl_nlmsg_get_payload(nlh);

	mnl_attr_parse(nlh, sizeof(*genl), attr_cb, tb);
	if (!tb[DEVLINK_ATTR_BUS_NAME] || !tb[DEVLINK_ATTR_DEV_NAME] ||
	    !tb[DEVLINK_ATTR_SB_INDEX] || !tb[DEVLINK_ATTR_SB_SIZE] ||
	    !tb[DEVLINK_ATTR_SB_INGRESS_POOL_COUNT] ||
	    !tb[DEVLINK_ATTR_SB_EGRESS_POOL_COUNT] ||
	    !tb[DEVLINK_ATTR_SB_INGRESS_TC_COUNT] ||
	    !tb[DEVLINK_ATTR_SB_EGRESS_TC_COUNT])
		return MNL_CB_ERROR;
	pr_out_sb(dl, tb);
	return MNL_CB_OK;
}

static int cmd_sb_show(struct dl *dl)
{
	struct nlmsghdr *nlh;
	uint16_t flags = NLM_F_REQUEST | NLM_F_ACK;
	int err;

	if (dl_argc(dl) == 0)
		flags |= NLM_F_DUMP;

	nlh = mnlg_msg_prepare(dl->nlg, DEVLINK_CMD_SB_GET, flags);

	if (dl_argc(dl) > 0) {
		err = dl_argv_parse_put(nlh, dl, DL_OPT_HANDLE, DL_OPT_SB);
		if (err)
			return err;
	}

	pr_out_section_start(dl, "sb");
	err = _mnlg_socket_sndrcv(dl->nlg, nlh, cmd_sb_show_cb, dl);
	pr_out_section_end(dl);
	return err;
}

static const char *pool_type_name(uint8_t type)
{
	switch (type) {
	case DEVLINK_SB_POOL_TYPE_INGRESS: return "ingress";
	case DEVLINK_SB_POOL_TYPE_EGRESS: return "egress";
	default: return "<unknown type>";
	}
}

static const char *threshold_type_name(uint8_t type)
{
	switch (type) {
	case DEVLINK_SB_THRESHOLD_TYPE_STATIC: return "static";
	case DEVLINK_SB_THRESHOLD_TYPE_DYNAMIC: return "dynamic";
	default: return "<unknown type>";
	}
}

static void pr_out_sb_pool(struct dl *dl, struct nlattr **tb)
{
	pr_out_handle_start_arr(dl, tb);
	check_indent_newline(dl);
	print_uint(PRINT_ANY, "sb", "sb %u",
		   mnl_attr_get_u32(tb[DEVLINK_ATTR_SB_INDEX]));
	print_uint(PRINT_ANY, "pool", " pool %u",
		   mnl_attr_get_u16(tb[DEVLINK_ATTR_SB_POOL_INDEX]));
	print_string(PRINT_ANY, "type", " type %s",
		     pool_type_name(mnl_attr_get_u8(tb[DEVLINK_ATTR_SB_POOL_TYPE])));
	print_uint(PRINT_ANY, "size", " size %u",
		   mnl_attr_get_u32(tb[DEVLINK_ATTR_SB_POOL_SIZE]));
	print_string(PRINT_ANY, "thtype", " thtype %s",
		     threshold_type_name(mnl_attr_get_u8(tb[DEVLINK_ATTR_SB_POOL_THRESHOLD_TYPE])));
	if (tb[DEVLINK_ATTR_SB_POOL_CELL_SIZE])
		print_uint(PRINT_ANY, "cell_size", " cell size %u",
			   mnl_attr_get_u32(tb[DEVLINK_ATTR_SB_POOL_CELL_SIZE]));
	pr_out_handle_end(dl);
}

static int cmd_sb_pool_show_cb(const struct nlmsghdr *nlh, void *data)
{
	struct dl *dl = data;
	struct nlattr *tb[DEVLINK_ATTR_MAX + 1] = {};
	struct genlmsghdr *genl = mnl_nlmsg_get_payload(nlh);

	mnl_attr_parse(nlh, sizeof(*genl), attr_cb, tb);
	if (!tb[DEVLINK_ATTR_BUS_NAME] || !tb[DEVLINK_ATTR_DEV_NAME] ||
	    !tb[DEVLINK_ATTR_SB_INDEX] || !tb[DEVLINK_ATTR_SB_POOL_INDEX] ||
	    !tb[DEVLINK_ATTR_SB_POOL_TYPE] || !tb[DEVLINK_ATTR_SB_POOL_SIZE] ||
	    !tb[DEVLINK_ATTR_SB_POOL_THRESHOLD_TYPE])
		return MNL_CB_ERROR;
	pr_out_sb_pool(dl, tb);
	return MNL_CB_OK;
}

static int cmd_sb_pool_show(struct dl *dl)
{
	struct nlmsghdr *nlh;
	uint16_t flags = NLM_F_REQUEST | NLM_F_ACK;
	int err;

	if (dl_argc(dl) == 0)
		flags |= NLM_F_DUMP;

	nlh = mnlg_msg_prepare(dl->nlg, DEVLINK_CMD_SB_POOL_GET, flags);

	if (dl_argc(dl) > 0) {
		err = dl_argv_parse_put(nlh, dl, DL_OPT_HANDLE | DL_OPT_SB_POOL,
					DL_OPT_SB);
		if (err)
			return err;
	}

	pr_out_section_start(dl, "pool");
	err = _mnlg_socket_sndrcv(dl->nlg, nlh, cmd_sb_pool_show_cb, dl);
	pr_out_section_end(dl);
	return err;
}

static int cmd_sb_pool_set(struct dl *dl)
{
	struct nlmsghdr *nlh;
	int err;

	nlh = mnlg_msg_prepare(dl->nlg, DEVLINK_CMD_SB_POOL_SET,
			       NLM_F_REQUEST | NLM_F_ACK);

	err = dl_argv_parse_put(nlh, dl, DL_OPT_HANDLE | DL_OPT_SB_POOL |
				DL_OPT_SB_SIZE | DL_OPT_SB_THTYPE, DL_OPT_SB);
	if (err)
		return err;

	return _mnlg_socket_sndrcv(dl->nlg, nlh, NULL, NULL);
}

static int cmd_sb_pool(struct dl *dl)
{
	if (dl_argv_match(dl, "help")) {
		cmd_sb_help();
		return 0;
	} else if (dl_argv_match(dl, "show") ||
		   dl_argv_match(dl, "list") || dl_no_arg(dl)) {
		dl_arg_inc(dl);
		return cmd_sb_pool_show(dl);
	} else if (dl_argv_match(dl, "set")) {
		dl_arg_inc(dl);
		return cmd_sb_pool_set(dl);
	}
	pr_err("Command \"%s\" not found\n", dl_argv(dl));
	return -ENOENT;
}

static void pr_out_sb_port_pool(struct dl *dl, struct nlattr **tb)
{
	pr_out_port_handle_start_arr(dl, tb, true);
	check_indent_newline(dl);
	print_uint(PRINT_ANY, "sb", "sb %u",
		   mnl_attr_get_u32(tb[DEVLINK_ATTR_SB_INDEX]));
	print_uint(PRINT_ANY, "pool", " pool %u",
		   mnl_attr_get_u16(tb[DEVLINK_ATTR_SB_POOL_INDEX]));
	print_uint(PRINT_ANY, "threshold", " threshold %u",
		   mnl_attr_get_u32(tb[DEVLINK_ATTR_SB_THRESHOLD]));
	pr_out_port_handle_end(dl);
}

static int cmd_sb_port_pool_show_cb(const struct nlmsghdr *nlh, void *data)
{
	struct dl *dl = data;
	struct nlattr *tb[DEVLINK_ATTR_MAX + 1] = {};
	struct genlmsghdr *genl = mnl_nlmsg_get_payload(nlh);

	mnl_attr_parse(nlh, sizeof(*genl), attr_cb, tb);
	if (!tb[DEVLINK_ATTR_BUS_NAME] || !tb[DEVLINK_ATTR_DEV_NAME] ||
	    !tb[DEVLINK_ATTR_PORT_INDEX] || !tb[DEVLINK_ATTR_SB_INDEX] ||
	    !tb[DEVLINK_ATTR_SB_POOL_INDEX] || !tb[DEVLINK_ATTR_SB_THRESHOLD])
		return MNL_CB_ERROR;
	pr_out_sb_port_pool(dl, tb);
	return MNL_CB_OK;
}

static int cmd_sb_port_pool_show(struct dl *dl)
{
	struct nlmsghdr *nlh;
	uint16_t flags = NLM_F_REQUEST | NLM_F_ACK;
	int err;

	if (dl_argc(dl) == 0)
		flags |= NLM_F_DUMP;

	nlh = mnlg_msg_prepare(dl->nlg, DEVLINK_CMD_SB_PORT_POOL_GET, flags);

	if (dl_argc(dl) > 0) {
		err = dl_argv_parse_put(nlh, dl,
					DL_OPT_HANDLEP | DL_OPT_SB_POOL,
					DL_OPT_SB);
		if (err)
			return err;
	}

	pr_out_section_start(dl, "port_pool");
	err = _mnlg_socket_sndrcv(dl->nlg, nlh, cmd_sb_port_pool_show_cb, dl);
	pr_out_section_end(dl);
	return 0;
}

static int cmd_sb_port_pool_set(struct dl *dl)
{
	struct nlmsghdr *nlh;
	int err;

	nlh = mnlg_msg_prepare(dl->nlg, DEVLINK_CMD_SB_PORT_POOL_SET,
			       NLM_F_REQUEST | NLM_F_ACK);

	err = dl_argv_parse_put(nlh, dl, DL_OPT_HANDLEP | DL_OPT_SB_POOL |
				DL_OPT_SB_TH, DL_OPT_SB);
	if (err)
		return err;

	return _mnlg_socket_sndrcv(dl->nlg, nlh, NULL, NULL);
}

static int cmd_sb_port_pool(struct dl *dl)
{
	if (dl_argv_match(dl, "help")) {
		cmd_sb_help();
		return 0;
	} else if (dl_argv_match(dl, "show") ||
		   dl_argv_match(dl, "list") || dl_no_arg(dl)) {
		dl_arg_inc(dl);
		return cmd_sb_port_pool_show(dl);
	} else if (dl_argv_match(dl, "set")) {
		dl_arg_inc(dl);
		return cmd_sb_port_pool_set(dl);
	}
	pr_err("Command \"%s\" not found\n", dl_argv(dl));
	return -ENOENT;
}

static int cmd_sb_port(struct dl *dl)
{
	if (dl_argv_match(dl, "help") || dl_no_arg(dl)) {
		cmd_sb_help();
		return 0;
	} else if (dl_argv_match(dl, "pool")) {
		dl_arg_inc(dl);
		return cmd_sb_port_pool(dl);
	}
	pr_err("Command \"%s\" not found\n", dl_argv(dl));
	return -ENOENT;
}

static void pr_out_sb_tc_bind(struct dl *dl, struct nlattr **tb)
{
	pr_out_port_handle_start_arr(dl, tb, true);
	check_indent_newline(dl);
	print_uint(PRINT_ANY, "sb", "sb %u",
		   mnl_attr_get_u32(tb[DEVLINK_ATTR_SB_INDEX]));
	print_uint(PRINT_ANY, "tc", " tc %u",
		   mnl_attr_get_u16(tb[DEVLINK_ATTR_SB_TC_INDEX]));
	print_string(PRINT_ANY, "type", " type %s",
		     pool_type_name(mnl_attr_get_u8(tb[DEVLINK_ATTR_SB_POOL_TYPE])));
	print_uint(PRINT_ANY, "pool", " pool %u",
		   mnl_attr_get_u16(tb[DEVLINK_ATTR_SB_POOL_INDEX]));
	print_uint(PRINT_ANY, "threshold", " threshold %u",
		   mnl_attr_get_u32(tb[DEVLINK_ATTR_SB_THRESHOLD]));
	pr_out_port_handle_end(dl);
}

static int cmd_sb_tc_bind_show_cb(const struct nlmsghdr *nlh, void *data)
{
	struct dl *dl = data;
	struct nlattr *tb[DEVLINK_ATTR_MAX + 1] = {};
	struct genlmsghdr *genl = mnl_nlmsg_get_payload(nlh);

	mnl_attr_parse(nlh, sizeof(*genl), attr_cb, tb);
	if (!tb[DEVLINK_ATTR_BUS_NAME] || !tb[DEVLINK_ATTR_DEV_NAME] ||
	    !tb[DEVLINK_ATTR_PORT_INDEX] || !tb[DEVLINK_ATTR_SB_INDEX] ||
	    !tb[DEVLINK_ATTR_SB_TC_INDEX] || !tb[DEVLINK_ATTR_SB_POOL_TYPE] ||
	    !tb[DEVLINK_ATTR_SB_POOL_INDEX] || !tb[DEVLINK_ATTR_SB_THRESHOLD])
		return MNL_CB_ERROR;
	pr_out_sb_tc_bind(dl, tb);
	return MNL_CB_OK;
}

static int cmd_sb_tc_bind_show(struct dl *dl)
{
	struct nlmsghdr *nlh;
	uint16_t flags = NLM_F_REQUEST | NLM_F_ACK;
	int err;

	if (dl_argc(dl) == 0)
		flags |= NLM_F_DUMP;

	nlh = mnlg_msg_prepare(dl->nlg, DEVLINK_CMD_SB_TC_POOL_BIND_GET, flags);

	if (dl_argc(dl) > 0) {
		err = dl_argv_parse_put(nlh, dl, DL_OPT_HANDLEP | DL_OPT_SB_TC |
					DL_OPT_SB_TYPE, DL_OPT_SB);
		if (err)
			return err;
	}

	pr_out_section_start(dl, "tc_bind");
	err = _mnlg_socket_sndrcv(dl->nlg, nlh, cmd_sb_tc_bind_show_cb, dl);
	pr_out_section_end(dl);
	return err;
}

static int cmd_sb_tc_bind_set(struct dl *dl)
{
	struct nlmsghdr *nlh;
	int err;

	nlh = mnlg_msg_prepare(dl->nlg, DEVLINK_CMD_SB_TC_POOL_BIND_SET,
			       NLM_F_REQUEST | NLM_F_ACK);

	err = dl_argv_parse_put(nlh, dl, DL_OPT_HANDLEP | DL_OPT_SB_TC |
				DL_OPT_SB_TYPE | DL_OPT_SB_POOL | DL_OPT_SB_TH,
				DL_OPT_SB);
	if (err)
		return err;

	return _mnlg_socket_sndrcv(dl->nlg, nlh, NULL, NULL);
}

static int cmd_sb_tc_bind(struct dl *dl)
{
	if (dl_argv_match(dl, "help")) {
		cmd_sb_help();
		return 0;
	} else if (dl_argv_match(dl, "show") ||
		   dl_argv_match(dl, "list") || dl_no_arg(dl)) {
		dl_arg_inc(dl);
		return cmd_sb_tc_bind_show(dl);
	} else if (dl_argv_match(dl, "set")) {
		dl_arg_inc(dl);
		return cmd_sb_tc_bind_set(dl);
	}
	pr_err("Command \"%s\" not found\n", dl_argv(dl));
	return -ENOENT;
}

static int cmd_sb_tc(struct dl *dl)
{
	if (dl_argv_match(dl, "help") || dl_no_arg(dl)) {
		cmd_sb_help();
		return 0;
	} else if (dl_argv_match(dl, "bind")) {
		dl_arg_inc(dl);
		return cmd_sb_tc_bind(dl);
	}
	pr_err("Command \"%s\" not found\n", dl_argv(dl));
	return -ENOENT;
}

struct occ_item {
	struct list_head list;
	uint32_t index;
	uint32_t cur;
	uint32_t max;
	uint32_t bound_pool_index;
};

struct occ_port {
	struct list_head list;
	char *bus_name;
	char *dev_name;
	uint32_t port_index;
	uint32_t sb_index;
	struct list_head pool_list;
	struct list_head ing_tc_list;
	struct list_head eg_tc_list;
};

struct occ_show {
	struct dl *dl;
	int err;
	struct list_head port_list;
};

static struct occ_item *occ_item_alloc(void)
{
	return calloc(1, sizeof(struct occ_item));
}

static void occ_item_free(struct occ_item *occ_item)
{
	free(occ_item);
}

static struct occ_port *occ_port_alloc(uint32_t port_index)
{
	struct occ_port *occ_port;

	occ_port = calloc(1, sizeof(*occ_port));
	if (!occ_port)
		return NULL;
	occ_port->port_index = port_index;
	INIT_LIST_HEAD(&occ_port->pool_list);
	INIT_LIST_HEAD(&occ_port->ing_tc_list);
	INIT_LIST_HEAD(&occ_port->eg_tc_list);
	return occ_port;
}

static void occ_port_free(struct occ_port *occ_port)
{
	struct occ_item *occ_item, *tmp;

	list_for_each_entry_safe(occ_item, tmp, &occ_port->pool_list, list)
		occ_item_free(occ_item);
	list_for_each_entry_safe(occ_item, tmp, &occ_port->ing_tc_list, list)
		occ_item_free(occ_item);
	list_for_each_entry_safe(occ_item, tmp, &occ_port->eg_tc_list, list)
		occ_item_free(occ_item);
}

static struct occ_show *occ_show_alloc(struct dl *dl)
{
	struct occ_show *occ_show;

	occ_show = calloc(1, sizeof(*occ_show));
	if (!occ_show)
		return NULL;
	occ_show->dl = dl;
	INIT_LIST_HEAD(&occ_show->port_list);
	return occ_show;
}

static void occ_show_free(struct occ_show *occ_show)
{
	struct occ_port *occ_port, *tmp;

	list_for_each_entry_safe(occ_port, tmp, &occ_show->port_list, list)
		occ_port_free(occ_port);
}

static struct occ_port *occ_port_get(struct occ_show *occ_show,
				     struct nlattr **tb)
{
	struct occ_port *occ_port;
	uint32_t port_index;

	port_index = mnl_attr_get_u32(tb[DEVLINK_ATTR_PORT_INDEX]);

	list_for_each_entry_reverse(occ_port, &occ_show->port_list, list) {
		if (occ_port->port_index == port_index)
			return occ_port;
	}
	occ_port = occ_port_alloc(port_index);
	if (!occ_port)
		return NULL;
	list_add_tail(&occ_port->list, &occ_show->port_list);
	return occ_port;
}

static void pr_out_occ_show_item_list(const char *label, struct list_head *list,
				      bool bound_pool)
{
	struct occ_item *occ_item;
	int i = 1;

	pr_out_sp(7, "  %s:", label);
	list_for_each_entry(occ_item, list, list) {
		if ((i - 1) % 4 == 0 && i != 1)
			pr_out_sp(7, " ");
		if (bound_pool)
			pr_out_sp(7, "%2u(%u):", occ_item->index,
				  occ_item->bound_pool_index);
		else
			pr_out_sp(7, "%2u:", occ_item->index);
		pr_out_sp(21, "%10u/%u", occ_item->cur, occ_item->max);
		if (i++ % 4 == 0)
			pr_out("\n");
	}
	if ((i - 1) % 4 != 0)
		pr_out("\n");
}

static void pr_out_json_occ_show_item_list(struct dl *dl, const char *label,
					   struct list_head *list,
					   bool bound_pool)
{
	struct occ_item *occ_item;
	char buf[32];

	open_json_object(label);
	list_for_each_entry(occ_item, list, list) {
		sprintf(buf, "%u", occ_item->index);
		open_json_object(buf);
		if (bound_pool)
			print_uint(PRINT_JSON, "bound_pool", NULL,
				   occ_item->bound_pool_index);
		print_uint(PRINT_JSON, "current", NULL, occ_item->cur);
		print_uint(PRINT_JSON, "max", NULL, occ_item->max);
		close_json_object();
	}
	close_json_object();
}

static void pr_out_occ_show_port(struct dl *dl, struct occ_port *occ_port)
{
	if (dl->json_output) {
		pr_out_json_occ_show_item_list(dl, "pool",
					       &occ_port->pool_list, false);
		pr_out_json_occ_show_item_list(dl, "itc",
					       &occ_port->ing_tc_list, true);
		pr_out_json_occ_show_item_list(dl, "etc",
					       &occ_port->eg_tc_list, true);
	} else {
		pr_out("\n");
		pr_out_occ_show_item_list("pool", &occ_port->pool_list, false);
		pr_out_occ_show_item_list("itc", &occ_port->ing_tc_list, true);
		pr_out_occ_show_item_list("etc", &occ_port->eg_tc_list, true);
	}
}

static void pr_out_occ_show(struct occ_show *occ_show)
{
	struct dl *dl = occ_show->dl;
	struct dl_opts *opts = &dl->opts;
	struct occ_port *occ_port;

	list_for_each_entry(occ_port, &occ_show->port_list, list) {
		__pr_out_port_handle_start(dl, opts->bus_name, opts->dev_name,
					   occ_port->port_index, true, false);
		pr_out_occ_show_port(dl, occ_port);
		pr_out_port_handle_end(dl);
	}
}

static void cmd_sb_occ_port_pool_process(struct occ_show *occ_show,
					 struct nlattr **tb)
{
	struct occ_port *occ_port;
	struct occ_item *occ_item;

	if (occ_show->err || !dl_dump_filter(occ_show->dl, tb))
		return;

	occ_port = occ_port_get(occ_show, tb);
	if (!occ_port) {
		occ_show->err = -ENOMEM;
		return;
	}

	occ_item = occ_item_alloc();
	if (!occ_item) {
		occ_show->err = -ENOMEM;
		return;
	}
	occ_item->index = mnl_attr_get_u16(tb[DEVLINK_ATTR_SB_POOL_INDEX]);
	occ_item->cur = mnl_attr_get_u32(tb[DEVLINK_ATTR_SB_OCC_CUR]);
	occ_item->max = mnl_attr_get_u32(tb[DEVLINK_ATTR_SB_OCC_MAX]);
	list_add_tail(&occ_item->list, &occ_port->pool_list);
}

static int cmd_sb_occ_port_pool_process_cb(const struct nlmsghdr *nlh, void *data)
{
	struct occ_show *occ_show = data;
	struct nlattr *tb[DEVLINK_ATTR_MAX + 1] = {};
	struct genlmsghdr *genl = mnl_nlmsg_get_payload(nlh);

	mnl_attr_parse(nlh, sizeof(*genl), attr_cb, tb);
	if (!tb[DEVLINK_ATTR_BUS_NAME] || !tb[DEVLINK_ATTR_DEV_NAME] ||
	    !tb[DEVLINK_ATTR_PORT_INDEX] || !tb[DEVLINK_ATTR_SB_INDEX] ||
	    !tb[DEVLINK_ATTR_SB_POOL_INDEX] ||
	    !tb[DEVLINK_ATTR_SB_OCC_CUR] || !tb[DEVLINK_ATTR_SB_OCC_MAX])
		return MNL_CB_ERROR;
	cmd_sb_occ_port_pool_process(occ_show, tb);
	return MNL_CB_OK;
}

static void cmd_sb_occ_tc_pool_process(struct occ_show *occ_show,
				       struct nlattr **tb)
{
	struct occ_port *occ_port;
	struct occ_item *occ_item;
	uint8_t pool_type;

	if (occ_show->err || !dl_dump_filter(occ_show->dl, tb))
		return;

	occ_port = occ_port_get(occ_show, tb);
	if (!occ_port) {
		occ_show->err = -ENOMEM;
		return;
	}

	occ_item = occ_item_alloc();
	if (!occ_item) {
		occ_show->err = -ENOMEM;
		return;
	}
	occ_item->index = mnl_attr_get_u16(tb[DEVLINK_ATTR_SB_TC_INDEX]);
	occ_item->cur = mnl_attr_get_u32(tb[DEVLINK_ATTR_SB_OCC_CUR]);
	occ_item->max = mnl_attr_get_u32(tb[DEVLINK_ATTR_SB_OCC_MAX]);
	occ_item->bound_pool_index =
			mnl_attr_get_u16(tb[DEVLINK_ATTR_SB_POOL_INDEX]);
	pool_type = mnl_attr_get_u8(tb[DEVLINK_ATTR_SB_POOL_TYPE]);
	if (pool_type == DEVLINK_SB_POOL_TYPE_INGRESS)
		list_add_tail(&occ_item->list, &occ_port->ing_tc_list);
	else if (pool_type == DEVLINK_SB_POOL_TYPE_EGRESS)
		list_add_tail(&occ_item->list, &occ_port->eg_tc_list);
	else
		occ_item_free(occ_item);
}

static int cmd_sb_occ_tc_pool_process_cb(const struct nlmsghdr *nlh, void *data)
{
	struct occ_show *occ_show = data;
	struct nlattr *tb[DEVLINK_ATTR_MAX + 1] = {};
	struct genlmsghdr *genl = mnl_nlmsg_get_payload(nlh);

	mnl_attr_parse(nlh, sizeof(*genl), attr_cb, tb);
	if (!tb[DEVLINK_ATTR_BUS_NAME] || !tb[DEVLINK_ATTR_DEV_NAME] ||
	    !tb[DEVLINK_ATTR_PORT_INDEX] || !tb[DEVLINK_ATTR_SB_INDEX] ||
	    !tb[DEVLINK_ATTR_SB_TC_INDEX] || !tb[DEVLINK_ATTR_SB_POOL_TYPE] ||
	    !tb[DEVLINK_ATTR_SB_POOL_INDEX] ||
	    !tb[DEVLINK_ATTR_SB_OCC_CUR] || !tb[DEVLINK_ATTR_SB_OCC_MAX])
		return MNL_CB_ERROR;
	cmd_sb_occ_tc_pool_process(occ_show, tb);
	return MNL_CB_OK;
}

static int cmd_sb_occ_show(struct dl *dl)
{
	struct nlmsghdr *nlh;
	struct occ_show *occ_show;
	uint16_t flags = NLM_F_REQUEST | NLM_F_ACK | NLM_F_DUMP;
	int err;

	err = dl_argv_parse(dl, DL_OPT_HANDLE | DL_OPT_HANDLEP, DL_OPT_SB);
	if (err)
		return err;

	occ_show = occ_show_alloc(dl);
	if (!occ_show)
		return -ENOMEM;

	nlh = mnlg_msg_prepare(dl->nlg, DEVLINK_CMD_SB_PORT_POOL_GET, flags);

	err = _mnlg_socket_sndrcv(dl->nlg, nlh,
				  cmd_sb_occ_port_pool_process_cb, occ_show);
	if (err)
		goto out;

	nlh = mnlg_msg_prepare(dl->nlg, DEVLINK_CMD_SB_TC_POOL_BIND_GET, flags);

	err = _mnlg_socket_sndrcv(dl->nlg, nlh,
				  cmd_sb_occ_tc_pool_process_cb, occ_show);
	if (err)
		goto out;

	pr_out_section_start(dl, "occupancy");
	pr_out_occ_show(occ_show);
	pr_out_section_end(dl);

out:
	occ_show_free(occ_show);
	return err;
}

static int cmd_sb_occ_snapshot(struct dl *dl)
{
	struct nlmsghdr *nlh;
	int err;

	nlh = mnlg_msg_prepare(dl->nlg, DEVLINK_CMD_SB_OCC_SNAPSHOT,
			       NLM_F_REQUEST | NLM_F_ACK);

	err = dl_argv_parse_put(nlh, dl, DL_OPT_HANDLE, DL_OPT_SB);
	if (err)
		return err;

	return _mnlg_socket_sndrcv(dl->nlg, nlh, NULL, NULL);
}

static int cmd_sb_occ_clearmax(struct dl *dl)
{
	struct nlmsghdr *nlh;
	int err;

	nlh = mnlg_msg_prepare(dl->nlg, DEVLINK_CMD_SB_OCC_MAX_CLEAR,
			       NLM_F_REQUEST | NLM_F_ACK);

	err = dl_argv_parse_put(nlh, dl, DL_OPT_HANDLE, DL_OPT_SB);
	if (err)
		return err;

	return _mnlg_socket_sndrcv(dl->nlg, nlh, NULL, NULL);
}

static int cmd_sb_occ(struct dl *dl)
{
	if (dl_argv_match(dl, "help") || dl_no_arg(dl)) {
		cmd_sb_help();
		return 0;
	} else if (dl_argv_match(dl, "show") ||
		   dl_argv_match(dl, "list")) {
		dl_arg_inc(dl);
		return cmd_sb_occ_show(dl);
	} else if (dl_argv_match(dl, "snapshot")) {
		dl_arg_inc(dl);
		return cmd_sb_occ_snapshot(dl);
	} else if (dl_argv_match(dl, "clearmax")) {
		dl_arg_inc(dl);
		return cmd_sb_occ_clearmax(dl);
	}
	pr_err("Command \"%s\" not found\n", dl_argv(dl));
	return -ENOENT;
}

static int cmd_sb(struct dl *dl)
{
	if (dl_argv_match(dl, "help")) {
		cmd_sb_help();
		return 0;
	} else if (dl_argv_match(dl, "show") ||
		   dl_argv_match(dl, "list") || dl_no_arg(dl)) {
		dl_arg_inc(dl);
		return cmd_sb_show(dl);
	} else if (dl_argv_match(dl, "pool")) {
		dl_arg_inc(dl);
		return cmd_sb_pool(dl);
	} else if (dl_argv_match(dl, "port")) {
		dl_arg_inc(dl);
		return cmd_sb_port(dl);
	} else if (dl_argv_match(dl, "tc")) {
		dl_arg_inc(dl);
		return cmd_sb_tc(dl);
	} else if (dl_argv_match(dl, "occupancy")) {
		dl_arg_inc(dl);
		return cmd_sb_occ(dl);
	}
	pr_err("Command \"%s\" not found\n", dl_argv(dl));
	return -ENOENT;
}

static const char *cmd_name(uint8_t cmd)
{
	switch (cmd) {
	case DEVLINK_CMD_UNSPEC: return "unspec";
	case DEVLINK_CMD_GET: return "get";
	case DEVLINK_CMD_SET: return "set";
	case DEVLINK_CMD_NEW: return "new";
	case DEVLINK_CMD_DEL: return "del";
	case DEVLINK_CMD_PORT_GET: return "get";
	case DEVLINK_CMD_PORT_SET: return "set";
	case DEVLINK_CMD_PORT_NEW: return "new";
	case DEVLINK_CMD_PORT_DEL: return "del";
	case DEVLINK_CMD_PARAM_GET: return "get";
	case DEVLINK_CMD_PARAM_SET: return "set";
	case DEVLINK_CMD_PARAM_NEW: return "new";
	case DEVLINK_CMD_PARAM_DEL: return "del";
	case DEVLINK_CMD_REGION_GET: return "get";
	case DEVLINK_CMD_REGION_SET: return "set";
	case DEVLINK_CMD_REGION_NEW: return "new";
	case DEVLINK_CMD_REGION_DEL: return "del";
	case DEVLINK_CMD_PORT_PARAM_GET: return "get";
	case DEVLINK_CMD_PORT_PARAM_SET: return "set";
	case DEVLINK_CMD_PORT_PARAM_NEW: return "new";
	case DEVLINK_CMD_PORT_PARAM_DEL: return "del";
	case DEVLINK_CMD_FLASH_UPDATE: return "begin";
	case DEVLINK_CMD_FLASH_UPDATE_END: return "end";
	case DEVLINK_CMD_FLASH_UPDATE_STATUS: return "status";
	case DEVLINK_CMD_HEALTH_REPORTER_RECOVER: return "status";
	case DEVLINK_CMD_TRAP_GET: return "get";
	case DEVLINK_CMD_TRAP_SET: return "set";
	case DEVLINK_CMD_TRAP_NEW: return "new";
	case DEVLINK_CMD_TRAP_DEL: return "del";
	case DEVLINK_CMD_TRAP_GROUP_GET: return "get";
	case DEVLINK_CMD_TRAP_GROUP_SET: return "set";
	case DEVLINK_CMD_TRAP_GROUP_NEW: return "new";
	case DEVLINK_CMD_TRAP_GROUP_DEL: return "del";
	case DEVLINK_CMD_TRAP_POLICER_GET: return "get";
	case DEVLINK_CMD_TRAP_POLICER_SET: return "set";
	case DEVLINK_CMD_TRAP_POLICER_NEW: return "new";
	case DEVLINK_CMD_TRAP_POLICER_DEL: return "del";
	default: return "<unknown cmd>";
	}
}

static const char *cmd_obj(uint8_t cmd)
{
	switch (cmd) {
	case DEVLINK_CMD_UNSPEC: return "unspec";
	case DEVLINK_CMD_GET:
	case DEVLINK_CMD_SET:
	case DEVLINK_CMD_NEW:
	case DEVLINK_CMD_DEL:
		return "dev";
	case DEVLINK_CMD_PORT_GET:
	case DEVLINK_CMD_PORT_SET:
	case DEVLINK_CMD_PORT_NEW:
	case DEVLINK_CMD_PORT_DEL:
		return "port";
	case DEVLINK_CMD_PARAM_GET:
	case DEVLINK_CMD_PARAM_SET:
	case DEVLINK_CMD_PARAM_NEW:
	case DEVLINK_CMD_PARAM_DEL:
	case DEVLINK_CMD_PORT_PARAM_GET:
	case DEVLINK_CMD_PORT_PARAM_SET:
	case DEVLINK_CMD_PORT_PARAM_NEW:
	case DEVLINK_CMD_PORT_PARAM_DEL:
		return "param";
	case DEVLINK_CMD_REGION_GET:
	case DEVLINK_CMD_REGION_SET:
	case DEVLINK_CMD_REGION_NEW:
	case DEVLINK_CMD_REGION_DEL:
		return "region";
	case DEVLINK_CMD_FLASH_UPDATE:
	case DEVLINK_CMD_FLASH_UPDATE_END:
	case DEVLINK_CMD_FLASH_UPDATE_STATUS:
		return "flash";
	case DEVLINK_CMD_HEALTH_REPORTER_RECOVER:
		return "health";
	case DEVLINK_CMD_TRAP_GET:
	case DEVLINK_CMD_TRAP_SET:
	case DEVLINK_CMD_TRAP_NEW:
	case DEVLINK_CMD_TRAP_DEL:
		return "trap";
	case DEVLINK_CMD_TRAP_GROUP_GET:
	case DEVLINK_CMD_TRAP_GROUP_SET:
	case DEVLINK_CMD_TRAP_GROUP_NEW:
	case DEVLINK_CMD_TRAP_GROUP_DEL:
		return "trap-group";
	case DEVLINK_CMD_TRAP_POLICER_GET:
	case DEVLINK_CMD_TRAP_POLICER_SET:
	case DEVLINK_CMD_TRAP_POLICER_NEW:
	case DEVLINK_CMD_TRAP_POLICER_DEL:
		return "trap-policer";
	default: return "<unknown obj>";
	}
}

static void pr_out_mon_header(uint8_t cmd)
{
	if (!is_json_context()) {
		pr_out("[%s,%s] ", cmd_obj(cmd), cmd_name(cmd));
	} else {
		open_json_object(NULL);
		print_string(PRINT_JSON, "command", NULL, cmd_name(cmd));
		open_json_object(cmd_obj(cmd));
	}
}

static void pr_out_mon_footer(void)
{
	if (is_json_context()) {
		close_json_object();
		close_json_object();
	}
}

static bool cmd_filter_check(struct dl *dl, uint8_t cmd)
{
	const char *obj = cmd_obj(cmd);
	unsigned int index = 0;
	const char *cur_obj;

	if (dl_no_arg(dl))
		return true;
	while ((cur_obj = dl_argv_index(dl, index++))) {
		if (strcmp(cur_obj, obj) == 0 || strcmp(cur_obj, "all") == 0)
			return true;
	}
	return false;
}

static void pr_out_flash_update(struct dl *dl, struct nlattr **tb)
{
	__pr_out_handle_start(dl, tb, true, false);

	if (tb[DEVLINK_ATTR_FLASH_UPDATE_STATUS_MSG]) {
		check_indent_newline(dl);
		print_string(PRINT_ANY, "msg", "msg %s",
			     mnl_attr_get_str(tb[DEVLINK_ATTR_FLASH_UPDATE_STATUS_MSG]));
	}
	if (tb[DEVLINK_ATTR_FLASH_UPDATE_COMPONENT]) {
		check_indent_newline(dl);
		print_string(PRINT_ANY, "component", "component %s",
			     mnl_attr_get_str(tb[DEVLINK_ATTR_FLASH_UPDATE_COMPONENT]));
	}

	if (tb[DEVLINK_ATTR_FLASH_UPDATE_STATUS_DONE])
		pr_out_u64(dl, "done",
			   mnl_attr_get_u64(tb[DEVLINK_ATTR_FLASH_UPDATE_STATUS_DONE]));

	if (tb[DEVLINK_ATTR_FLASH_UPDATE_STATUS_TOTAL])
		pr_out_u64(dl, "total",
			   mnl_attr_get_u64(tb[DEVLINK_ATTR_FLASH_UPDATE_STATUS_TOTAL]));

	pr_out_handle_end(dl);
}

static void pr_out_region(struct dl *dl, struct nlattr **tb);
static void pr_out_health(struct dl *dl, struct nlattr **tb_health,
			  bool show_device, bool show_port);
static void pr_out_trap(struct dl *dl, struct nlattr **tb, bool array);
static void pr_out_trap_group(struct dl *dl, struct nlattr **tb, bool array);
static void pr_out_trap_policer(struct dl *dl, struct nlattr **tb, bool array);

static int cmd_mon_show_cb(const struct nlmsghdr *nlh, void *data)
{
	struct dl *dl = data;
	struct nlattr *tb[DEVLINK_ATTR_MAX + 1] = {};
	struct genlmsghdr *genl = mnl_nlmsg_get_payload(nlh);
	uint8_t cmd = genl->cmd;

	if (!cmd_filter_check(dl, cmd))
		return MNL_CB_OK;

	switch (cmd) {
	case DEVLINK_CMD_GET: /* fall through */
	case DEVLINK_CMD_SET: /* fall through */
	case DEVLINK_CMD_NEW: /* fall through */
	case DEVLINK_CMD_DEL:
		mnl_attr_parse(nlh, sizeof(*genl), attr_cb, tb);
		if (!tb[DEVLINK_ATTR_BUS_NAME] || !tb[DEVLINK_ATTR_DEV_NAME])
			return MNL_CB_ERROR;
		pr_out_mon_header(genl->cmd);
		dl->stats = true;
		pr_out_dev(dl, tb);
		pr_out_mon_footer();
		break;
	case DEVLINK_CMD_PORT_GET: /* fall through */
	case DEVLINK_CMD_PORT_SET: /* fall through */
	case DEVLINK_CMD_PORT_NEW: /* fall through */
	case DEVLINK_CMD_PORT_DEL:
		mnl_attr_parse(nlh, sizeof(*genl), attr_cb, tb);
		if (!tb[DEVLINK_ATTR_BUS_NAME] || !tb[DEVLINK_ATTR_DEV_NAME] ||
		    !tb[DEVLINK_ATTR_PORT_INDEX])
			return MNL_CB_ERROR;
		pr_out_mon_header(genl->cmd);
		pr_out_port(dl, tb);
		pr_out_mon_footer();
		break;
	case DEVLINK_CMD_PARAM_GET: /* fall through */
	case DEVLINK_CMD_PARAM_SET: /* fall through */
	case DEVLINK_CMD_PARAM_NEW: /* fall through */
	case DEVLINK_CMD_PARAM_DEL:
		mnl_attr_parse(nlh, sizeof(*genl), attr_cb, tb);
		if (!tb[DEVLINK_ATTR_BUS_NAME] || !tb[DEVLINK_ATTR_DEV_NAME] ||
		    !tb[DEVLINK_ATTR_PARAM])
			return MNL_CB_ERROR;
		pr_out_mon_header(genl->cmd);
		pr_out_param(dl, tb, false, false);
		pr_out_mon_footer();
		break;
	case DEVLINK_CMD_REGION_GET: /* fall through */
	case DEVLINK_CMD_REGION_SET: /* fall through */
	case DEVLINK_CMD_REGION_NEW: /* fall through */
	case DEVLINK_CMD_REGION_DEL:
		mnl_attr_parse(nlh, sizeof(*genl), attr_cb, tb);
		if (!tb[DEVLINK_ATTR_BUS_NAME] || !tb[DEVLINK_ATTR_DEV_NAME] ||
		    !tb[DEVLINK_ATTR_REGION_NAME])
			return MNL_CB_ERROR;
		pr_out_mon_header(genl->cmd);
		pr_out_region(dl, tb);
		pr_out_mon_footer();
		break;
	case DEVLINK_CMD_FLASH_UPDATE: /* fall through */
	case DEVLINK_CMD_FLASH_UPDATE_END: /* fall through */
	case DEVLINK_CMD_FLASH_UPDATE_STATUS:
		mnl_attr_parse(nlh, sizeof(*genl), attr_cb, tb);
		if (!tb[DEVLINK_ATTR_BUS_NAME] || !tb[DEVLINK_ATTR_DEV_NAME])
			return MNL_CB_ERROR;
		pr_out_mon_header(genl->cmd);
		pr_out_flash_update(dl, tb);
		pr_out_mon_footer();
		break;
	case DEVLINK_CMD_HEALTH_REPORTER_RECOVER:
		mnl_attr_parse(nlh, sizeof(*genl), attr_cb, tb);
		if (!tb[DEVLINK_ATTR_BUS_NAME] || !tb[DEVLINK_ATTR_DEV_NAME] ||
		    !tb[DEVLINK_ATTR_HEALTH_REPORTER])
			return MNL_CB_ERROR;
		pr_out_mon_header(genl->cmd);
		pr_out_health(dl, tb, true, true);
		pr_out_mon_footer();
		break;
	case DEVLINK_CMD_TRAP_GET: /* fall through */
	case DEVLINK_CMD_TRAP_SET: /* fall through */
	case DEVLINK_CMD_TRAP_NEW: /* fall through */
	case DEVLINK_CMD_TRAP_DEL:
		mnl_attr_parse(nlh, sizeof(*genl), attr_cb, tb);
		if (!tb[DEVLINK_ATTR_BUS_NAME] || !tb[DEVLINK_ATTR_DEV_NAME] ||
		    !tb[DEVLINK_ATTR_TRAP_NAME] ||
		    !tb[DEVLINK_ATTR_TRAP_TYPE] ||
		    !tb[DEVLINK_ATTR_TRAP_ACTION] ||
		    !tb[DEVLINK_ATTR_TRAP_GROUP_NAME] ||
		    !tb[DEVLINK_ATTR_TRAP_METADATA] ||
		    !tb[DEVLINK_ATTR_STATS])
			return MNL_CB_ERROR;
		pr_out_mon_header(genl->cmd);
		pr_out_trap(dl, tb, false);
		pr_out_mon_footer();
		break;
	case DEVLINK_CMD_TRAP_GROUP_GET: /* fall through */
	case DEVLINK_CMD_TRAP_GROUP_SET: /* fall through */
	case DEVLINK_CMD_TRAP_GROUP_NEW: /* fall through */
	case DEVLINK_CMD_TRAP_GROUP_DEL:
		mnl_attr_parse(nlh, sizeof(*genl), attr_cb, tb);
		if (!tb[DEVLINK_ATTR_BUS_NAME] || !tb[DEVLINK_ATTR_DEV_NAME] ||
		    !tb[DEVLINK_ATTR_TRAP_GROUP_NAME] ||
		    !tb[DEVLINK_ATTR_STATS])
			return MNL_CB_ERROR;
		pr_out_mon_header(genl->cmd);
		pr_out_trap_group(dl, tb, false);
		pr_out_mon_footer();
		break;
	case DEVLINK_CMD_TRAP_POLICER_GET: /* fall through */
	case DEVLINK_CMD_TRAP_POLICER_SET: /* fall through */
	case DEVLINK_CMD_TRAP_POLICER_NEW: /* fall through */
	case DEVLINK_CMD_TRAP_POLICER_DEL: /* fall through */
		mnl_attr_parse(nlh, sizeof(*genl), attr_cb, tb);
		if (!tb[DEVLINK_ATTR_BUS_NAME] || !tb[DEVLINK_ATTR_DEV_NAME] ||
		    !tb[DEVLINK_ATTR_TRAP_POLICER_ID] ||
		    !tb[DEVLINK_ATTR_TRAP_POLICER_RATE] ||
		    !tb[DEVLINK_ATTR_TRAP_POLICER_BURST])
			return MNL_CB_ERROR;
		pr_out_mon_header(genl->cmd);
		pr_out_trap_policer(dl, tb, false);
		break;
	}
	fflush(stdout);
	return MNL_CB_OK;
}

static int cmd_mon_show(struct dl *dl)
{
	int err;
	unsigned int index = 0;
	const char *cur_obj;

	while ((cur_obj = dl_argv_index(dl, index++))) {
		if (strcmp(cur_obj, "all") != 0 &&
		    strcmp(cur_obj, "dev") != 0 &&
		    strcmp(cur_obj, "port") != 0 &&
		    strcmp(cur_obj, "health") != 0 &&
		    strcmp(cur_obj, "trap") != 0 &&
		    strcmp(cur_obj, "trap-group") != 0 &&
		    strcmp(cur_obj, "trap-policer") != 0) {
			pr_err("Unknown object \"%s\"\n", cur_obj);
			return -EINVAL;
		}
	}
	err = _mnlg_socket_group_add(dl->nlg, DEVLINK_GENL_MCGRP_CONFIG_NAME);
	if (err)
		return err;
	open_json_object(NULL);
	open_json_array(PRINT_JSON, "mon");
	err = _mnlg_socket_recv_run_intr(dl->nlg, cmd_mon_show_cb, dl);
	close_json_array(PRINT_JSON, NULL);
	close_json_object();
	if (err)
		return err;
	return 0;
}

static void cmd_mon_help(void)
{
	pr_err("Usage: devlink monitor [ all | OBJECT-LIST ]\n"
	       "where  OBJECT-LIST := { dev | port | health | trap | trap-group | trap-policer }\n");
}

static int cmd_mon(struct dl *dl)
{
	if (dl_argv_match(dl, "help")) {
		cmd_mon_help();
		return 0;
	}
	return cmd_mon_show(dl);
}

struct dpipe_field {
	char *name;
	unsigned int id;
	unsigned int bitwidth;
	enum devlink_dpipe_field_mapping_type mapping_type;
};

struct dpipe_header {
	struct list_head list;
	char *name;
	unsigned int id;
	struct dpipe_field *fields;
	unsigned int fields_count;
};

struct dpipe_table {
	struct list_head list;
	char *name;
	unsigned int resource_id;
	bool resource_valid;
};

struct dpipe_tables {
	struct list_head table_list;
};

struct resource {
	char *name;
	uint64_t size;
	uint64_t size_new;
	uint64_t size_min;
	uint64_t size_max;
	uint64_t size_gran;
	enum devlink_resource_unit unit;
	bool size_valid;
	uint64_t size_occ;
	bool occ_valid;
	uint64_t id;
	struct list_head list;
	struct list_head resource_list;
	struct resource *parent;
};

struct resources {
	struct list_head resource_list;
};

struct resource_ctx {
	struct dl *dl;
	int err;
	struct resources *resources;
	struct dpipe_tables *tables;
	bool print_resources;
	bool pending_change;
};

static struct resource *resource_alloc(void)
{
	struct resource *resource;

	resource = calloc(1, sizeof(struct resource));
	if (!resource)
		return NULL;
	INIT_LIST_HEAD(&resource->resource_list);
	return resource;
}

static void resource_free(struct resource *resource)
{
	struct resource *child_resource, *tmp;

	list_for_each_entry_safe(child_resource, tmp, &resource->resource_list,
				 list) {
		free(child_resource->name);
		resource_free(child_resource);
	}
	free(resource);
}

static struct resources *resources_alloc(void)
{
	struct resources *resources;

	resources = calloc(1, sizeof(struct resources));
	if (!resources)
		return NULL;
	INIT_LIST_HEAD(&resources->resource_list);
	return resources;
}

static void resources_free(struct resources *resources)
{
	struct resource *resource, *tmp;

	list_for_each_entry_safe(resource, tmp, &resources->resource_list, list)
		resource_free(resource);
}

static int resource_ctx_init(struct resource_ctx *ctx, struct dl *dl)
{
	ctx->resources = resources_alloc();
	if (!ctx->resources)
		return -ENOMEM;
	ctx->dl = dl;
	return 0;
}

static void resource_ctx_fini(struct resource_ctx *ctx)
{
	resources_free(ctx->resources);
}

struct dpipe_ctx {
	struct dl *dl;
	int err;
	struct list_head global_headers;
	struct list_head local_headers;
	struct dpipe_tables *tables;
	struct resources *resources;
	bool print_headers;
	bool print_tables;
};

static struct dpipe_header *dpipe_header_alloc(unsigned int fields_count)
{
	struct dpipe_header *header;

	header = calloc(1, sizeof(struct dpipe_header));
	if (!header)
		return NULL;
	header->fields = calloc(fields_count, sizeof(struct dpipe_field));
	if (!header->fields)
		goto err_fields_alloc;
	header->fields_count = fields_count;
	return header;

err_fields_alloc:
	free(header);
	return NULL;
}

static void dpipe_header_free(struct dpipe_header *header)
{
	free(header->fields);
	free(header);
}

static void dpipe_header_clear(struct dpipe_header *header)
{
	struct dpipe_field *field;
	int i;

	for (i = 0; i < header->fields_count; i++) {
		field = &header->fields[i];
		free(field->name);
	}
	free(header->name);
}

static void dpipe_header_add(struct dpipe_ctx *ctx,
			     struct dpipe_header *header, bool global)
{
	if (global)
		list_add(&header->list, &ctx->global_headers);
	else
		list_add(&header->list, &ctx->local_headers);
}

static void dpipe_header_del(struct dpipe_header *header)
{
	list_del(&header->list);
}

static struct dpipe_table *dpipe_table_alloc(void)
{
	return calloc(1, sizeof(struct dpipe_table));
}

static void dpipe_table_free(struct dpipe_table *table)
{
	free(table);
}

static struct dpipe_tables *dpipe_tables_alloc(void)
{
	struct dpipe_tables *tables;

	tables = calloc(1, sizeof(struct dpipe_tables));
	if (!tables)
		return NULL;
	INIT_LIST_HEAD(&tables->table_list);
	return tables;
}

static void dpipe_tables_free(struct dpipe_tables *tables)
{
	struct dpipe_table *table, *tmp;

	list_for_each_entry_safe(table, tmp, &tables->table_list, list)
		dpipe_table_free(table);
	free(tables);
}

static int dpipe_ctx_init(struct dpipe_ctx *ctx, struct dl *dl)
{
	ctx->tables = dpipe_tables_alloc();
	if (!ctx->tables)
		return -ENOMEM;

	ctx->dl = dl;
	INIT_LIST_HEAD(&ctx->global_headers);
	INIT_LIST_HEAD(&ctx->local_headers);
	return 0;
}

static void dpipe_ctx_fini(struct dpipe_ctx *ctx)
{
	struct dpipe_header *header, *tmp;

	list_for_each_entry_safe(header, tmp, &ctx->global_headers,
				 list) {
		dpipe_header_del(header);
		dpipe_header_clear(header);
		dpipe_header_free(header);
	}
	list_for_each_entry_safe(header, tmp, &ctx->local_headers,
				 list) {
		dpipe_header_del(header);
		dpipe_header_clear(header);
		dpipe_header_free(header);
	}
	dpipe_tables_free(ctx->tables);
}

static const char *dpipe_header_id2s(struct dpipe_ctx *ctx,
				     uint32_t header_id, bool global)
{
	struct list_head *header_list;
	struct dpipe_header *header;

	if (global)
		header_list = &ctx->global_headers;
	else
		header_list = &ctx->local_headers;
	list_for_each_entry(header, header_list, list) {
		if (header->id != header_id)
			continue;
		return header->name;
	}
	return NULL;
}

static const char *dpipe_field_id2s(struct dpipe_ctx *ctx,
				    uint32_t header_id,
				    uint32_t field_id, bool global)
{
	struct list_head *header_list;
	struct dpipe_header *header;

	if (global)
		header_list = &ctx->global_headers;
	else
		header_list = &ctx->local_headers;
	list_for_each_entry(header, header_list, list) {
		if (header->id != header_id)
			continue;
		return header->fields[field_id].name;
	}
	return NULL;
}

static const char *
dpipe_field_mapping_e2s(enum devlink_dpipe_field_mapping_type mapping_type)
{
	switch (mapping_type) {
	case DEVLINK_DPIPE_FIELD_MAPPING_TYPE_NONE:
		return NULL;
	case DEVLINK_DPIPE_FIELD_MAPPING_TYPE_IFINDEX:
		return "ifindex";
	default:
		return "<unknown>";
	}
}

static const char *
dpipe_mapping_get(struct dpipe_ctx *ctx, uint32_t header_id,
		  uint32_t field_id, bool global)
{
	enum devlink_dpipe_field_mapping_type mapping_type;
	struct list_head *header_list;
	struct dpipe_header *header;

	if (global)
		header_list = &ctx->global_headers;
	else
		header_list = &ctx->local_headers;
	list_for_each_entry(header, header_list, list) {
		if (header->id != header_id)
			continue;
		mapping_type = header->fields[field_id].mapping_type;
		return dpipe_field_mapping_e2s(mapping_type);
	}
	return NULL;
}

static void pr_out_dpipe_fields(struct dpipe_ctx *ctx,
				struct dpipe_field *fields,
				unsigned int field_count)
{
	struct dpipe_field *field;
	int i;

	for (i = 0; i < field_count; i++) {
		field = &fields[i];
		pr_out_entry_start(ctx->dl);
		check_indent_newline(ctx->dl);
		print_string(PRINT_ANY, "name", "name %s", field->name);
		if (ctx->dl->verbose)
			print_uint(PRINT_ANY, "id", " id %u", field->id);
		print_uint(PRINT_ANY, "bitwidth", " bitwidth %u", field->bitwidth);
		if (field->mapping_type) {
			print_string(PRINT_ANY, "mapping_type", " mapping_type %s",
				     dpipe_field_mapping_e2s(field->mapping_type));
		}
		pr_out_entry_end(ctx->dl);
	}
}

static void
pr_out_dpipe_header(struct dpipe_ctx *ctx, struct nlattr **tb,
		    struct dpipe_header *header, bool global)
{
	pr_out_handle_start_arr(ctx->dl, tb);
	check_indent_newline(ctx->dl);
	print_string(PRINT_ANY, "name", "name %s", header->name);
	if (ctx->dl->verbose) {
		print_uint(PRINT_ANY, "id", " id %u", header->id);
		print_bool(PRINT_ANY, "global", " global %s", global);
	}
	pr_out_array_start(ctx->dl, "field");
	pr_out_dpipe_fields(ctx, header->fields,
			    header->fields_count);
	pr_out_array_end(ctx->dl);
	pr_out_handle_end(ctx->dl);
}

static void pr_out_dpipe_headers(struct dpipe_ctx *ctx,
				 struct nlattr **tb)
{
	struct dpipe_header *header;

	list_for_each_entry(header, &ctx->local_headers, list)
		pr_out_dpipe_header(ctx, tb, header, false);

	list_for_each_entry(header, &ctx->global_headers, list)
		pr_out_dpipe_header(ctx, tb, header, true);
}

static int dpipe_header_field_get(struct nlattr *nl, struct dpipe_field *field)
{
	struct nlattr *nla_field[DEVLINK_ATTR_MAX + 1] = {};
	const char *name;
	int err;

	err = mnl_attr_parse_nested(nl, attr_cb, nla_field);
	if (err != MNL_CB_OK)
		return -EINVAL;
	if (!nla_field[DEVLINK_ATTR_DPIPE_FIELD_ID] ||
	    !nla_field[DEVLINK_ATTR_DPIPE_FIELD_NAME] ||
	    !nla_field[DEVLINK_ATTR_DPIPE_FIELD_BITWIDTH] ||
	    !nla_field[DEVLINK_ATTR_DPIPE_FIELD_MAPPING_TYPE])
		return -EINVAL;

	name = mnl_attr_get_str(nla_field[DEVLINK_ATTR_DPIPE_FIELD_NAME]);
	field->id = mnl_attr_get_u32(nla_field[DEVLINK_ATTR_DPIPE_FIELD_ID]);
	field->bitwidth = mnl_attr_get_u32(nla_field[DEVLINK_ATTR_DPIPE_FIELD_BITWIDTH]);
	field->name = strdup(name);
	if (!field->name)
		return -ENOMEM;
	field->mapping_type = mnl_attr_get_u32(nla_field[DEVLINK_ATTR_DPIPE_FIELD_MAPPING_TYPE]);
	return 0;
}

static int dpipe_header_fields_get(struct nlattr *nla_fields,
				   struct dpipe_field *fields)
{
	struct nlattr *nla_field;
	int count = 0;
	int err;

	mnl_attr_for_each_nested(nla_field, nla_fields) {
		err = dpipe_header_field_get(nla_field, &fields[count]);
		if (err)
			return err;
		count++;
	}
	return 0;
}

static unsigned int dpipe_header_field_count_get(struct nlattr *nla_fields)
{
	struct nlattr *nla_field;
	unsigned int count = 0;

	mnl_attr_for_each_nested(nla_field, nla_fields)
		count++;
	return count;
}

static int dpipe_header_get(struct dpipe_ctx *ctx, struct nlattr *nl)
{
	struct nlattr *nla_header[DEVLINK_ATTR_MAX + 1] = {};
	struct dpipe_header *header;
	unsigned int fields_count;
	const char *header_name;
	bool global;
	int err;

	err = mnl_attr_parse_nested(nl, attr_cb, nla_header);
	if (err != MNL_CB_OK)
		return -EINVAL;

	if (!nla_header[DEVLINK_ATTR_DPIPE_HEADER_NAME] ||
	    !nla_header[DEVLINK_ATTR_DPIPE_HEADER_ID] ||
	    !nla_header[DEVLINK_ATTR_DPIPE_HEADER_FIELDS])
		return -EINVAL;

	fields_count = dpipe_header_field_count_get(nla_header[DEVLINK_ATTR_DPIPE_HEADER_FIELDS]);
	header = dpipe_header_alloc(fields_count);
	if (!header)
		return -ENOMEM;

	header_name = mnl_attr_get_str(nla_header[DEVLINK_ATTR_DPIPE_HEADER_NAME]);
	header->name = strdup(header_name);
	header->id = mnl_attr_get_u32(nla_header[DEVLINK_ATTR_DPIPE_HEADER_ID]);
	header->fields_count = fields_count;
	global = !!mnl_attr_get_u8(nla_header[DEVLINK_ATTR_DPIPE_HEADER_GLOBAL]);

	err = dpipe_header_fields_get(nla_header[DEVLINK_ATTR_DPIPE_HEADER_FIELDS],
				      header->fields);
	if (err)
		goto err_field_get;
	dpipe_header_add(ctx, header, global);
	return 0;

err_field_get:
	dpipe_header_free(header);
	return err;
}

static int dpipe_headers_get(struct dpipe_ctx *ctx, struct nlattr **tb)
{
	struct nlattr *nla_headers = tb[DEVLINK_ATTR_DPIPE_HEADERS];
	struct nlattr *nla_header;
	int err;

	mnl_attr_for_each_nested(nla_header, nla_headers) {
		err = dpipe_header_get(ctx, nla_header);
		if (err)
			return err;
	}
	return 0;
}

static int cmd_dpipe_header_cb(const struct nlmsghdr *nlh, void *data)
{
	struct dpipe_ctx *ctx = data;
	struct nlattr *tb[DEVLINK_ATTR_MAX + 1] = {};
	struct genlmsghdr *genl = mnl_nlmsg_get_payload(nlh);
	int err;

	mnl_attr_parse(nlh, sizeof(*genl), attr_cb, tb);
	if (!tb[DEVLINK_ATTR_BUS_NAME] || !tb[DEVLINK_ATTR_DEV_NAME] ||
	    !tb[DEVLINK_ATTR_DPIPE_HEADERS])
		return MNL_CB_ERROR;
	err = dpipe_headers_get(ctx, tb);
	if (err) {
		ctx->err = err;
		return MNL_CB_ERROR;
	}

	if (ctx->print_headers)
		pr_out_dpipe_headers(ctx, tb);
	return MNL_CB_OK;
}

static int cmd_dpipe_headers_show(struct dl *dl)
{
	struct nlmsghdr *nlh;
	struct dpipe_ctx ctx = {};
	uint16_t flags = NLM_F_REQUEST | NLM_F_ACK;
	int err;

	nlh = mnlg_msg_prepare(dl->nlg, DEVLINK_CMD_DPIPE_HEADERS_GET, flags);

	err = dl_argv_parse_put(nlh, dl, DL_OPT_HANDLE, 0);
	if (err)
		return err;

	err = dpipe_ctx_init(&ctx, dl);
	if (err)
		return err;

	ctx.print_headers = true;

	pr_out_section_start(dl, "header");
	err = _mnlg_socket_sndrcv(dl->nlg, nlh, cmd_dpipe_header_cb, &ctx);
	if (err)
		pr_err("error get headers %s\n", strerror(ctx.err));
	pr_out_section_end(dl);

	dpipe_ctx_fini(&ctx);
	return err;
}

static void cmd_dpipe_help(void)
{
	pr_err("Usage: devlink dpipe table show DEV [ name TABLE_NAME ]\n");
	pr_err("       devlink dpipe table set DEV name TABLE_NAME\n");
	pr_err("                               [ counters_enabled { true | false } ]\n");
	pr_err("       devlink dpipe table dump DEV name TABLE_NAME\n");
	pr_err("       devlink dpipe header show DEV\n");
}

static int cmd_dpipe_header(struct dl *dl)
{
	if (dl_argv_match(dl, "help") || dl_no_arg(dl)) {
		cmd_dpipe_help();
		return 0;
	} else if (dl_argv_match(dl, "show")) {
		dl_arg_inc(dl);
		return cmd_dpipe_headers_show(dl);
	}
	pr_err("Command \"%s\" not found\n", dl_argv(dl));
	return -ENOENT;
}

static const char
*dpipe_action_type_e2s(enum devlink_dpipe_action_type action_type)
{
	switch (action_type) {
	case DEVLINK_DPIPE_ACTION_TYPE_FIELD_MODIFY:
		return "field_modify";
	default:
		return "<unknown>";
	}
}

struct dpipe_op_info {
	uint32_t header_id;
	uint32_t field_id;
	bool header_global;
};

struct dpipe_action {
	struct dpipe_op_info info;
	uint32_t type;
};

static void pr_out_dpipe_action(struct dpipe_action *action,
				struct dpipe_ctx *ctx)
{
	struct dpipe_op_info *op_info = &action->info;
	const char *mapping;

	check_indent_newline(ctx->dl);
	print_string(PRINT_ANY, "type", "type %s",
		     dpipe_action_type_e2s(action->type));
	print_string(PRINT_ANY, "header", " header %s",
		     dpipe_header_id2s(ctx, op_info->header_id,
				       op_info->header_global));
	print_string(PRINT_ANY, "field", " field %s",
		     dpipe_field_id2s(ctx, op_info->header_id,
				      op_info->field_id,
				      op_info->header_global));
	mapping = dpipe_mapping_get(ctx, op_info->header_id,
				    op_info->field_id,
				    op_info->header_global);
	if (mapping)
		print_string(PRINT_ANY, "mapping", " mapping %s", mapping);
}

static int dpipe_action_parse(struct dpipe_action *action, struct nlattr *nl)
{
	struct nlattr *nla_action[DEVLINK_ATTR_MAX + 1] = {};
	int err;

	err = mnl_attr_parse_nested(nl, attr_cb, nla_action);
	if (err != MNL_CB_OK)
		return -EINVAL;

	if (!nla_action[DEVLINK_ATTR_DPIPE_ACTION_TYPE] ||
	    !nla_action[DEVLINK_ATTR_DPIPE_HEADER_INDEX] ||
	    !nla_action[DEVLINK_ATTR_DPIPE_HEADER_ID] ||
	    !nla_action[DEVLINK_ATTR_DPIPE_FIELD_ID]) {
		return -EINVAL;
	}

	action->type = mnl_attr_get_u32(nla_action[DEVLINK_ATTR_DPIPE_ACTION_TYPE]);
	action->info.header_id = mnl_attr_get_u32(nla_action[DEVLINK_ATTR_DPIPE_HEADER_ID]);
	action->info.field_id = mnl_attr_get_u32(nla_action[DEVLINK_ATTR_DPIPE_FIELD_ID]);
	action->info.header_global = !!mnl_attr_get_u8(nla_action[DEVLINK_ATTR_DPIPE_HEADER_GLOBAL]);

	return 0;
}

static int dpipe_table_actions_show(struct dpipe_ctx *ctx,
				    struct nlattr *nla_actions)
{
	struct nlattr *nla_action;
	struct dpipe_action action;

	mnl_attr_for_each_nested(nla_action, nla_actions) {
		pr_out_entry_start(ctx->dl);
		if (dpipe_action_parse(&action, nla_action))
			goto err_action_parse;
		pr_out_dpipe_action(&action, ctx);
		pr_out_entry_end(ctx->dl);
	}
	return 0;

err_action_parse:
	pr_out_entry_end(ctx->dl);
	return -EINVAL;
}

static const char *
dpipe_match_type_e2s(enum devlink_dpipe_match_type match_type)
{
	switch (match_type) {
	case DEVLINK_DPIPE_MATCH_TYPE_FIELD_EXACT:
		return "field_exact";
	default:
		return "<unknown>";
	}
}

struct dpipe_match {
	struct dpipe_op_info info;
	uint32_t type;
};

static void pr_out_dpipe_match(struct dpipe_match *match,
			       struct dpipe_ctx *ctx)
{
	struct dpipe_op_info *op_info = &match->info;
	const char *mapping;

	check_indent_newline(ctx->dl);
	print_string(PRINT_ANY, "type", "type %s",
		     dpipe_match_type_e2s(match->type));
	print_string(PRINT_ANY, "header", " header %s",
		     dpipe_header_id2s(ctx, op_info->header_id,
				       op_info->header_global));
	print_string(PRINT_ANY, "field", " field %s",
		     dpipe_field_id2s(ctx, op_info->header_id,
				      op_info->field_id,
				      op_info->header_global));
	mapping = dpipe_mapping_get(ctx, op_info->header_id,
				    op_info->field_id,
				    op_info->header_global);
	if (mapping)
		print_string(PRINT_ANY, "mapping", " mapping %s", mapping);
}

static int dpipe_match_parse(struct dpipe_match *match,
			     struct nlattr *nl)

{
	struct nlattr *nla_match[DEVLINK_ATTR_MAX + 1] = {};
	int err;

	err = mnl_attr_parse_nested(nl, attr_cb, nla_match);
	if (err != MNL_CB_OK)
		return -EINVAL;

	if (!nla_match[DEVLINK_ATTR_DPIPE_MATCH_TYPE] ||
	    !nla_match[DEVLINK_ATTR_DPIPE_HEADER_INDEX] ||
	    !nla_match[DEVLINK_ATTR_DPIPE_HEADER_ID] ||
	    !nla_match[DEVLINK_ATTR_DPIPE_FIELD_ID]) {
		return -EINVAL;
	}

	match->type = mnl_attr_get_u32(nla_match[DEVLINK_ATTR_DPIPE_MATCH_TYPE]);
	match->info.header_id = mnl_attr_get_u32(nla_match[DEVLINK_ATTR_DPIPE_HEADER_ID]);
	match->info.field_id = mnl_attr_get_u32(nla_match[DEVLINK_ATTR_DPIPE_FIELD_ID]);
	match->info.header_global = !!mnl_attr_get_u8(nla_match[DEVLINK_ATTR_DPIPE_HEADER_GLOBAL]);

	return 0;
}

static int dpipe_table_matches_show(struct dpipe_ctx *ctx,
				    struct nlattr *nla_matches)
{
	struct nlattr *nla_match;
	struct dpipe_match match;

	mnl_attr_for_each_nested(nla_match, nla_matches) {
		pr_out_entry_start(ctx->dl);
		if (dpipe_match_parse(&match, nla_match))
			goto err_match_parse;
		pr_out_dpipe_match(&match, ctx);
		pr_out_entry_end(ctx->dl);
	}
	return 0;

err_match_parse:
	pr_out_entry_end(ctx->dl);
	return -EINVAL;
}

static struct resource *
resource_find(struct resources *resources, struct resource *resource,
	      uint64_t resource_id)
{
	struct list_head *list_head;

	if (!resource)
		list_head = &resources->resource_list;
	else
		list_head = &resource->resource_list;

	list_for_each_entry(resource, list_head, list) {
		struct resource *child_resource;

		if (resource->id == resource_id)
			return resource;

		child_resource = resource_find(resources, resource,
					       resource_id);
		if (child_resource)
			return child_resource;
	}
	return NULL;
}

static void
resource_path_print(struct dl *dl, struct resources *resources,
		    uint64_t resource_id)
{
	struct resource *resource, *parent_resource;
	const char del[] = "/";
	int path_len = 0;
	char *path;

	resource = resource_find(resources, NULL, resource_id);
	if (!resource)
		return;

	for (parent_resource = resource; parent_resource;
	     parent_resource = parent_resource->parent)
		path_len += strlen(parent_resource->name) + 1;

	path_len++;
	path = calloc(1, path_len);
	if (!path)
		return;

	path += path_len - 1;
	for (parent_resource = resource; parent_resource;
		parent_resource = parent_resource->parent) {
		path -= strlen(parent_resource->name);
		memcpy(path, parent_resource->name,
		       strlen(parent_resource->name));
		path -= strlen(del);
		memcpy(path, del, strlen(del));
	}
	check_indent_newline(dl);
	print_string(PRINT_ANY, "resource_path", "resource_path %s", path);
	free(path);
}

static int dpipe_table_show(struct dpipe_ctx *ctx, struct nlattr *nl)
{
	struct nlattr *nla_table[DEVLINK_ATTR_MAX + 1] = {};
	struct dpipe_table *table;
	uint32_t resource_units;
	bool counters_enabled;
	bool resource_valid;
	uint32_t size;
	int err;

	err = mnl_attr_parse_nested(nl, attr_cb, nla_table);
	if (err != MNL_CB_OK)
		return -EINVAL;

	if (!nla_table[DEVLINK_ATTR_DPIPE_TABLE_NAME] ||
	    !nla_table[DEVLINK_ATTR_DPIPE_TABLE_SIZE] ||
	    !nla_table[DEVLINK_ATTR_DPIPE_TABLE_ACTIONS] ||
	    !nla_table[DEVLINK_ATTR_DPIPE_TABLE_MATCHES] ||
	    !nla_table[DEVLINK_ATTR_DPIPE_TABLE_COUNTERS_ENABLED]) {
		return -EINVAL;
	}

	table = dpipe_table_alloc();
	if (!table)
		return -ENOMEM;

	table->name = strdup(mnl_attr_get_str(nla_table[DEVLINK_ATTR_DPIPE_TABLE_NAME]));
	size = mnl_attr_get_u32(nla_table[DEVLINK_ATTR_DPIPE_TABLE_SIZE]);
	counters_enabled = !!mnl_attr_get_u8(nla_table[DEVLINK_ATTR_DPIPE_TABLE_COUNTERS_ENABLED]);

	resource_valid = nla_table[DEVLINK_ATTR_DPIPE_TABLE_RESOURCE_ID] &&
			 ctx->resources;
	if (resource_valid) {
		table->resource_id = mnl_attr_get_u64(nla_table[DEVLINK_ATTR_DPIPE_TABLE_RESOURCE_ID]);
		table->resource_valid = true;
	}

	list_add_tail(&table->list, &ctx->tables->table_list);
	if (!ctx->print_tables)
		return 0;

	check_indent_newline(ctx->dl);
	print_string(PRINT_ANY, "name", "name %s", table->name);
	print_uint(PRINT_ANY, "size", " size %u", size);
	print_bool(PRINT_ANY, "counters_enabled", " counters_enabled %s", counters_enabled);

	if (resource_valid) {
		resource_units = mnl_attr_get_u32(nla_table[DEVLINK_ATTR_DPIPE_TABLE_RESOURCE_UNITS]);
		resource_path_print(ctx->dl, ctx->resources,
				    table->resource_id);
		print_uint(PRINT_ANY, "resource_units", " resource_units %u",
			   resource_units);
	}

	pr_out_array_start(ctx->dl, "match");
	if (dpipe_table_matches_show(ctx, nla_table[DEVLINK_ATTR_DPIPE_TABLE_MATCHES]))
		goto err_matches_show;
	pr_out_array_end(ctx->dl);

	pr_out_array_start(ctx->dl, "action");
	if (dpipe_table_actions_show(ctx, nla_table[DEVLINK_ATTR_DPIPE_TABLE_ACTIONS]))
		goto err_actions_show;
	pr_out_array_end(ctx->dl);

	return 0;

err_actions_show:
err_matches_show:
	pr_out_array_end(ctx->dl);
	return -EINVAL;
}

static int dpipe_tables_show(struct dpipe_ctx *ctx, struct nlattr **tb)
{
	struct nlattr *nla_tables = tb[DEVLINK_ATTR_DPIPE_TABLES];
	struct nlattr *nla_table;

	mnl_attr_for_each_nested(nla_table, nla_tables) {
		if (ctx->print_tables)
			pr_out_handle_start_arr(ctx->dl, tb);
		if (dpipe_table_show(ctx, nla_table))
			goto err_table_show;
		if (ctx->print_tables)
			pr_out_handle_end(ctx->dl);
	}
	return 0;

err_table_show:
	if (ctx->print_tables)
		pr_out_handle_end(ctx->dl);
	return -EINVAL;
}

static int cmd_dpipe_table_show_cb(const struct nlmsghdr *nlh, void *data)
{
	struct dpipe_ctx *ctx = data;
	struct nlattr *tb[DEVLINK_ATTR_MAX + 1] = {};
	struct genlmsghdr *genl = mnl_nlmsg_get_payload(nlh);

	mnl_attr_parse(nlh, sizeof(*genl), attr_cb, tb);
	if (!tb[DEVLINK_ATTR_BUS_NAME] || !tb[DEVLINK_ATTR_DEV_NAME] ||
	    !tb[DEVLINK_ATTR_DPIPE_TABLES])
		return MNL_CB_ERROR;

	if (dpipe_tables_show(ctx, tb))
		return MNL_CB_ERROR;
	return MNL_CB_OK;
}

static int cmd_resource_dump_cb(const struct nlmsghdr *nlh, void *data);

static int cmd_dpipe_table_show(struct dl *dl)
{
	struct nlmsghdr *nlh;
	struct dpipe_ctx dpipe_ctx = {};
	struct resource_ctx resource_ctx = {};
	uint16_t flags = NLM_F_REQUEST;
	int err;

	err = dl_argv_parse(dl, DL_OPT_HANDLE, DL_OPT_DPIPE_TABLE_NAME);
	if (err)
		return err;

	nlh = mnlg_msg_prepare(dl->nlg, DEVLINK_CMD_DPIPE_HEADERS_GET, flags);

	err = dpipe_ctx_init(&dpipe_ctx, dl);
	if (err)
		return err;

	dpipe_ctx.print_tables = true;

	dl_opts_put(nlh, dl);
	err = _mnlg_socket_sndrcv(dl->nlg, nlh, cmd_dpipe_header_cb,
				  &dpipe_ctx);
	if (err) {
		pr_err("error get headers %s\n", strerror(dpipe_ctx.err));
		goto err_headers_get;
	}

	err = resource_ctx_init(&resource_ctx, dl);
	if (err)
		goto err_resource_ctx_init;

	resource_ctx.print_resources = false;
	nlh = mnlg_msg_prepare(dl->nlg, DEVLINK_CMD_RESOURCE_DUMP, flags);
	dl_opts_put(nlh, dl);
	err = _mnlg_socket_sndrcv(dl->nlg, nlh, cmd_resource_dump_cb,
				  &resource_ctx);
	if (!err)
		dpipe_ctx.resources = resource_ctx.resources;

	flags = NLM_F_REQUEST | NLM_F_ACK;
	nlh = mnlg_msg_prepare(dl->nlg, DEVLINK_CMD_DPIPE_TABLE_GET, flags);
	dl_opts_put(nlh, dl);

	pr_out_section_start(dl, "table");
	_mnlg_socket_sndrcv(dl->nlg, nlh, cmd_dpipe_table_show_cb, &dpipe_ctx);
	pr_out_section_end(dl);

	resource_ctx_fini(&resource_ctx);
	dpipe_ctx_fini(&dpipe_ctx);
	return 0;

err_resource_ctx_init:
err_headers_get:
	dpipe_ctx_fini(&dpipe_ctx);
	return err;
}

static int cmd_dpipe_table_set(struct dl *dl)
{
	struct nlmsghdr *nlh;
	int err;

	nlh = mnlg_msg_prepare(dl->nlg, DEVLINK_CMD_DPIPE_TABLE_COUNTERS_SET,
			       NLM_F_REQUEST | NLM_F_ACK);

	err = dl_argv_parse_put(nlh, dl,
				DL_OPT_HANDLE | DL_OPT_DPIPE_TABLE_NAME |
				DL_OPT_DPIPE_TABLE_COUNTERS, 0);
	if (err)
		return err;

	return _mnlg_socket_sndrcv(dl->nlg, nlh, NULL, NULL);
}

enum dpipe_value_type {
	DPIPE_VALUE_TYPE_VALUE,
	DPIPE_VALUE_TYPE_MASK,
};

static const char *
dpipe_value_type_e2s(enum dpipe_value_type type)
{
	switch (type) {
	case DPIPE_VALUE_TYPE_VALUE:
		return "value";
	case DPIPE_VALUE_TYPE_MASK:
		return "value_mask";
	default:
		return "<unknown>";
	}
}

struct dpipe_field_printer {
	unsigned int field_id;
	void (*printer)(struct dpipe_ctx *, enum dpipe_value_type, void *);
};

struct dpipe_header_printer {
	struct dpipe_field_printer *printers;
	unsigned int printers_count;
	unsigned int header_id;
};

static void dpipe_field_printer_ipv4_addr(struct dpipe_ctx *ctx,
					  enum dpipe_value_type type,
					  void *value)
{
	struct in_addr ip_addr;

	ip_addr.s_addr = htonl(*(uint32_t *)value);
	check_indent_newline(ctx->dl);
	print_string_name_value(dpipe_value_type_e2s(type), inet_ntoa(ip_addr));
}

static void
dpipe_field_printer_ethernet_addr(struct dpipe_ctx *ctx,
				  enum dpipe_value_type type,
				  void *value)
{
	check_indent_newline(ctx->dl);
	print_string_name_value(dpipe_value_type_e2s(type),
				ether_ntoa((struct ether_addr *)value));
}

static void dpipe_field_printer_ipv6_addr(struct dpipe_ctx *ctx,
					  enum dpipe_value_type type,
					  void *value)
{
	char str[INET6_ADDRSTRLEN];

	inet_ntop(AF_INET6, value, str, INET6_ADDRSTRLEN);
	check_indent_newline(ctx->dl);
	print_string_name_value(dpipe_value_type_e2s(type), str);
}

static struct dpipe_field_printer dpipe_field_printers_ipv4[] = {
	{
		.printer = dpipe_field_printer_ipv4_addr,
		.field_id = DEVLINK_DPIPE_FIELD_IPV4_DST_IP,
	}
};

static struct dpipe_header_printer dpipe_header_printer_ipv4  = {
	.printers = dpipe_field_printers_ipv4,
	.printers_count = ARRAY_SIZE(dpipe_field_printers_ipv4),
	.header_id = DEVLINK_DPIPE_HEADER_IPV4,
};

static struct dpipe_field_printer dpipe_field_printers_ethernet[] = {
	{
		.printer = dpipe_field_printer_ethernet_addr,
		.field_id = DEVLINK_DPIPE_FIELD_ETHERNET_DST_MAC,
	},
};

static struct dpipe_header_printer dpipe_header_printer_ethernet = {
	.printers = dpipe_field_printers_ethernet,
	.printers_count = ARRAY_SIZE(dpipe_field_printers_ethernet),
	.header_id = DEVLINK_DPIPE_HEADER_ETHERNET,
};

static struct dpipe_field_printer dpipe_field_printers_ipv6[] = {
	{
		.printer = dpipe_field_printer_ipv6_addr,
		.field_id = DEVLINK_DPIPE_FIELD_IPV6_DST_IP,
	}
};

static struct dpipe_header_printer dpipe_header_printer_ipv6 = {
	.printers = dpipe_field_printers_ipv6,
	.printers_count = ARRAY_SIZE(dpipe_field_printers_ipv6),
	.header_id = DEVLINK_DPIPE_HEADER_IPV6,
};

static struct dpipe_header_printer *dpipe_header_printers[] = {
	&dpipe_header_printer_ipv4,
	&dpipe_header_printer_ethernet,
	&dpipe_header_printer_ipv6,
};

static int dpipe_print_prot_header(struct dpipe_ctx *ctx,
				   struct dpipe_op_info *info,
				   enum dpipe_value_type type,
				   void *value)
{
	unsigned int header_printers_count = ARRAY_SIZE(dpipe_header_printers);
	struct dpipe_header_printer *header_printer;
	struct dpipe_field_printer *field_printer;
	unsigned int field_printers_count;
	int j;
	int i;

	for (i = 0; i < header_printers_count; i++) {
		header_printer = dpipe_header_printers[i];
		if (header_printer->header_id != info->header_id)
			continue;
		field_printers_count = header_printer->printers_count;
		for (j = 0; j < field_printers_count; j++) {
			field_printer = &header_printer->printers[j];
			if (field_printer->field_id != info->field_id)
				continue;
			field_printer->printer(ctx, type, value);
			return 0;
		}
	}

	return -EINVAL;
}

static void __pr_out_entry_value(struct dpipe_ctx *ctx,
				 void *value,
				 unsigned int value_len,
				 struct dpipe_op_info *info,
				 enum dpipe_value_type type)
{
	if (info->header_global &&
	    !dpipe_print_prot_header(ctx, info, type, value))
		return;

	if (value_len == sizeof(uint32_t)) {
		uint32_t *value_32 = value;

		check_indent_newline(ctx->dl);
		print_uint_name_value(dpipe_value_type_e2s(type), *value_32);
	}
}

static void pr_out_dpipe_entry_value(struct dpipe_ctx *ctx,
				     struct nlattr **nla_match_value,
				     struct dpipe_op_info *info)
{
	void *value, *value_mask;
	uint32_t value_mapping;
	uint16_t value_len;
	bool mask, mapping;

	mask = !!nla_match_value[DEVLINK_ATTR_DPIPE_VALUE_MASK];
	mapping = !!nla_match_value[DEVLINK_ATTR_DPIPE_VALUE_MAPPING];

	value_len = mnl_attr_get_payload_len(nla_match_value[DEVLINK_ATTR_DPIPE_VALUE]);
	value = mnl_attr_get_payload(nla_match_value[DEVLINK_ATTR_DPIPE_VALUE]);

	if (mapping) {
		value_mapping = mnl_attr_get_u32(nla_match_value[DEVLINK_ATTR_DPIPE_VALUE_MAPPING]);
		check_indent_newline(ctx->dl);
		print_uint(PRINT_ANY, "mapping_value", "mapping_value %u", value_mapping);
	}

	if (mask) {
		value_mask = mnl_attr_get_payload(nla_match_value[DEVLINK_ATTR_DPIPE_VALUE]);
		__pr_out_entry_value(ctx, value_mask, value_len, info,
				     DPIPE_VALUE_TYPE_MASK);
	}

	__pr_out_entry_value(ctx, value, value_len, info, DPIPE_VALUE_TYPE_VALUE);
}

static int dpipe_entry_match_value_show(struct dpipe_ctx *ctx,
					struct nlattr *nl)
{
	struct nlattr *nla_match_value[DEVLINK_ATTR_MAX + 1] = {};
	struct dpipe_match match;
	int err;

	err = mnl_attr_parse_nested(nl, attr_cb, nla_match_value);
	if (err != MNL_CB_OK)
		return -EINVAL;

	if (!nla_match_value[DEVLINK_ATTR_DPIPE_MATCH] ||
	    !nla_match_value[DEVLINK_ATTR_DPIPE_VALUE]) {
		return -EINVAL;
	}

	pr_out_entry_start(ctx->dl);
	if (dpipe_match_parse(&match,
			      nla_match_value[DEVLINK_ATTR_DPIPE_MATCH]))
		goto err_match_parse;
	pr_out_dpipe_match(&match, ctx);
	pr_out_dpipe_entry_value(ctx, nla_match_value, &match.info);
	pr_out_entry_end(ctx->dl);

	return 0;

err_match_parse:
	pr_out_entry_end(ctx->dl);
	return -EINVAL;
}

static int dpipe_entry_action_value_show(struct dpipe_ctx *ctx,
					 struct nlattr *nl)
{
	struct nlattr *nla_action_value[DEVLINK_ATTR_MAX + 1] = {};
	struct dpipe_action action;
	int err;

	err = mnl_attr_parse_nested(nl, attr_cb, nla_action_value);
	if (err != MNL_CB_OK)
		return -EINVAL;

	if (!nla_action_value[DEVLINK_ATTR_DPIPE_ACTION] ||
	    !nla_action_value[DEVLINK_ATTR_DPIPE_VALUE]) {
		return -EINVAL;
	}

	pr_out_entry_start(ctx->dl);
	if (dpipe_action_parse(&action,
			       nla_action_value[DEVLINK_ATTR_DPIPE_ACTION]))
		goto err_action_parse;
	pr_out_dpipe_action(&action, ctx);
	pr_out_dpipe_entry_value(ctx, nla_action_value, &action.info);
	pr_out_entry_end(ctx->dl);

	return 0;

err_action_parse:
	pr_out_entry_end(ctx->dl);
	return -EINVAL;
}

static int
dpipe_tables_action_values_show(struct dpipe_ctx *ctx,
				struct nlattr *nla_action_values)
{
	struct nlattr *nla_action_value;

	mnl_attr_for_each_nested(nla_action_value, nla_action_values) {
		if (dpipe_entry_action_value_show(ctx, nla_action_value))
			return -EINVAL;
	}
	return 0;
}

static int
dpipe_tables_match_values_show(struct dpipe_ctx *ctx,
			       struct nlattr *nla_match_values)
{
	struct nlattr *nla_match_value;

	mnl_attr_for_each_nested(nla_match_value, nla_match_values) {
		if (dpipe_entry_match_value_show(ctx, nla_match_value))
			return -EINVAL;
	}
	return 0;
}

static int dpipe_entry_show(struct dpipe_ctx *ctx, struct nlattr *nl)
{
	struct nlattr *nla_entry[DEVLINK_ATTR_MAX + 1] = {};
	uint32_t entry_index;
	uint64_t counter;
	int err;

	err = mnl_attr_parse_nested(nl, attr_cb, nla_entry);
	if (err != MNL_CB_OK)
		return -EINVAL;

	if (!nla_entry[DEVLINK_ATTR_DPIPE_ENTRY_INDEX] ||
	    !nla_entry[DEVLINK_ATTR_DPIPE_ENTRY_MATCH_VALUES] ||
	    !nla_entry[DEVLINK_ATTR_DPIPE_ENTRY_ACTION_VALUES]) {
		return -EINVAL;
	}

	check_indent_newline(ctx->dl);
	entry_index = mnl_attr_get_u32(nla_entry[DEVLINK_ATTR_DPIPE_ENTRY_INDEX]);
	print_uint(PRINT_ANY, "index", "index %u", entry_index);

	if (nla_entry[DEVLINK_ATTR_DPIPE_ENTRY_COUNTER]) {
		counter = mnl_attr_get_u64(nla_entry[DEVLINK_ATTR_DPIPE_ENTRY_COUNTER]);
		print_uint(PRINT_ANY, "counter", " counter %u", counter);
	}

	pr_out_array_start(ctx->dl, "match_value");
	if (dpipe_tables_match_values_show(ctx,
					   nla_entry[DEVLINK_ATTR_DPIPE_ENTRY_MATCH_VALUES]))
		goto err_match_values_show;
	pr_out_array_end(ctx->dl);

	pr_out_array_start(ctx->dl, "action_value");
	if (dpipe_tables_action_values_show(ctx,
					    nla_entry[DEVLINK_ATTR_DPIPE_ENTRY_ACTION_VALUES]))
		goto err_action_values_show;
	pr_out_array_end(ctx->dl);
	return 0;

err_action_values_show:
err_match_values_show:
	pr_out_array_end(ctx->dl);
	return -EINVAL;
}

static int dpipe_table_entries_show(struct dpipe_ctx *ctx, struct nlattr **tb)
{
	struct nlattr *nla_entries = tb[DEVLINK_ATTR_DPIPE_ENTRIES];
	struct nlattr *nla_entry;

	mnl_attr_for_each_nested(nla_entry, nla_entries) {
		pr_out_handle_start_arr(ctx->dl, tb);
		if (dpipe_entry_show(ctx, nla_entry))
			goto err_entry_show;
		pr_out_handle_end(ctx->dl);
	}
	return 0;

err_entry_show:
	pr_out_handle_end(ctx->dl);
	return -EINVAL;
}

static int cmd_dpipe_table_entry_dump_cb(const struct nlmsghdr *nlh, void *data)
{
	struct dpipe_ctx *ctx = data;
	struct nlattr *tb[DEVLINK_ATTR_MAX + 1] = {};
	struct genlmsghdr *genl = mnl_nlmsg_get_payload(nlh);

	mnl_attr_parse(nlh, sizeof(*genl), attr_cb, tb);
	if (!tb[DEVLINK_ATTR_BUS_NAME] || !tb[DEVLINK_ATTR_DEV_NAME] ||
	    !tb[DEVLINK_ATTR_DPIPE_ENTRIES])
		return MNL_CB_ERROR;

	if (dpipe_table_entries_show(ctx, tb))
		return MNL_CB_ERROR;
	return MNL_CB_OK;
}

static int cmd_dpipe_table_dump(struct dl *dl)
{
	struct nlmsghdr *nlh;
	struct dpipe_ctx ctx = {};
	uint16_t flags = NLM_F_REQUEST;
	int err;

	err = dpipe_ctx_init(&ctx, dl);
	if (err)
		return err;

	err = dl_argv_parse(dl, DL_OPT_HANDLE | DL_OPT_DPIPE_TABLE_NAME, 0);
	if (err)
		goto out;

	nlh = mnlg_msg_prepare(dl->nlg, DEVLINK_CMD_DPIPE_HEADERS_GET, flags);
	dl_opts_put(nlh, dl);
	err = _mnlg_socket_sndrcv(dl->nlg, nlh, cmd_dpipe_header_cb, &ctx);
	if (err) {
		pr_err("error get headers %s\n", strerror(ctx.err));
		goto out;
	}

	flags = NLM_F_REQUEST | NLM_F_ACK;
	nlh = mnlg_msg_prepare(dl->nlg, DEVLINK_CMD_DPIPE_ENTRIES_GET, flags);
	dl_opts_put(nlh, dl);

	pr_out_section_start(dl, "table_entry");
	_mnlg_socket_sndrcv(dl->nlg, nlh, cmd_dpipe_table_entry_dump_cb, &ctx);
	pr_out_section_end(dl);
out:
	dpipe_ctx_fini(&ctx);
	return err;
}

static int cmd_dpipe_table(struct dl *dl)
{
	if (dl_argv_match(dl, "help") || dl_no_arg(dl)) {
		cmd_dpipe_help();
		return 0;
	} else if (dl_argv_match(dl, "show")) {
		dl_arg_inc(dl);
		return cmd_dpipe_table_show(dl);
	} else if (dl_argv_match(dl, "set")) {
		dl_arg_inc(dl);
		return cmd_dpipe_table_set(dl);
	}  else if (dl_argv_match(dl, "dump")) {
		dl_arg_inc(dl);
		return cmd_dpipe_table_dump(dl);
	}
	pr_err("Command \"%s\" not found\n", dl_argv(dl));
	return -ENOENT;
}

static int cmd_dpipe(struct dl *dl)
{
	if (dl_argv_match(dl, "help") || dl_no_arg(dl)) {
		cmd_dpipe_help();
		return 0;
	} else if (dl_argv_match(dl, "header")) {
		dl_arg_inc(dl);
		return cmd_dpipe_header(dl);
	} else if (dl_argv_match(dl, "table")) {
		dl_arg_inc(dl);
		return cmd_dpipe_table(dl);
	}
	pr_err("Command \"%s\" not found\n", dl_argv(dl));
	return -ENOENT;
}

static int
resource_parse(struct resource_ctx *ctx, struct resource *resource,
	       struct nlattr **nla_resource)
{
	if (!nla_resource[DEVLINK_ATTR_RESOURCE_NAME] ||
	    !nla_resource[DEVLINK_ATTR_RESOURCE_SIZE] ||
	    !nla_resource[DEVLINK_ATTR_RESOURCE_ID] ||
	    !nla_resource[DEVLINK_ATTR_RESOURCE_UNIT] ||
	    !nla_resource[DEVLINK_ATTR_RESOURCE_SIZE_MIN] ||
	    !nla_resource[DEVLINK_ATTR_RESOURCE_SIZE_MAX] ||
	    !nla_resource[DEVLINK_ATTR_RESOURCE_SIZE_GRAN]) {
		return -EINVAL;
	}

	resource->name = strdup(mnl_attr_get_str(nla_resource[DEVLINK_ATTR_RESOURCE_NAME]));
	resource->size = mnl_attr_get_u64(nla_resource[DEVLINK_ATTR_RESOURCE_SIZE]);
	resource->id = mnl_attr_get_u64(nla_resource[DEVLINK_ATTR_RESOURCE_ID]);
	resource->unit = mnl_attr_get_u8(nla_resource[DEVLINK_ATTR_RESOURCE_UNIT]);
	resource->size_min = mnl_attr_get_u64(nla_resource[DEVLINK_ATTR_RESOURCE_SIZE_MIN]);
	resource->size_max = mnl_attr_get_u64(nla_resource[DEVLINK_ATTR_RESOURCE_SIZE_MAX]);
	resource->size_gran = mnl_attr_get_u64(nla_resource[DEVLINK_ATTR_RESOURCE_SIZE_GRAN]);

	if (nla_resource[DEVLINK_ATTR_RESOURCE_SIZE_NEW])
		resource->size_new = mnl_attr_get_u64(nla_resource[DEVLINK_ATTR_RESOURCE_SIZE_NEW]);
	else
		resource->size_new = resource->size;

	if (nla_resource[DEVLINK_ATTR_RESOURCE_OCC]) {
		resource->size_occ = mnl_attr_get_u64(nla_resource[DEVLINK_ATTR_RESOURCE_OCC]);
		resource->occ_valid = true;
	}

	if (resource->size_new != resource->size)
		ctx->pending_change = true;

	return 0;
}

static int
resource_get(struct resource_ctx *ctx, struct resource *resource,
	     struct resource *parent_resource, struct nlattr *nl)
{
	struct nlattr *nla_resource[DEVLINK_ATTR_MAX + 1] = {};
	struct nlattr *nla_child_resource;
	struct nlattr *nla_resources;
	bool top = false;
	int err;

	if (!resource) {
		nla_resources = nl;
		top = true;
		goto out;
	}

	err = mnl_attr_parse_nested(nl, attr_cb, nla_resource);
	if (err != MNL_CB_OK)
		return -EINVAL;

	err = resource_parse(ctx, resource, nla_resource);
	if (err)
		return err;

	resource->parent = parent_resource;
	if (!nla_resource[DEVLINK_ATTR_RESOURCE_LIST])
		return 0;

	resource->size_valid = !!mnl_attr_get_u8(nla_resource[DEVLINK_ATTR_RESOURCE_SIZE_VALID]);
	nla_resources = nla_resource[DEVLINK_ATTR_RESOURCE_LIST];
out:
	mnl_attr_for_each_nested(nla_child_resource, nla_resources) {
		struct resource *child_resource;
		struct list_head *list;

		child_resource = resource_alloc();
		if (!child_resource)
			return -ENOMEM;

		if (top)
			list = &ctx->resources->resource_list;
		else
			list = &resource->resource_list;

		list_add_tail(&child_resource->list, list);
		err = resource_get(ctx, child_resource, resource,
				   nla_child_resource);
		if (err)
			return err;
	}

	return 0;
}

static const char *resource_unit_str_get(enum devlink_resource_unit unit)
{
	switch (unit) {
	case DEVLINK_RESOURCE_UNIT_ENTRY: return "entry";
	default: return "<unknown unit>";
	}
}

static void resource_show(struct resource *resource,
			  struct resource_ctx *ctx)
{
	struct resource *child_resource;
	struct dpipe_table *table;
	struct dl *dl = ctx->dl;
	bool array = false;

	check_indent_newline(dl);
	print_string(PRINT_ANY, "name", "name %s", resource->name);
	if (dl->verbose)
		resource_path_print(dl, ctx->resources, resource->id);
	pr_out_u64(dl, "size", resource->size);
	if (resource->size != resource->size_new)
		pr_out_u64(dl, "size_new", resource->size_new);
	if (resource->occ_valid)
		print_uint(PRINT_ANY, "occ", " occ %u",  resource->size_occ);
	print_string(PRINT_ANY, "unit", " unit %s",
		     resource_unit_str_get(resource->unit));

	if (resource->size_min != resource->size_max) {
		print_uint(PRINT_ANY, "size_min", " size_min %u",
			   resource->size_min);
		pr_out_u64(dl, "size_max", resource->size_max);
		print_uint(PRINT_ANY, "size_gran", " size_gran %u",
			   resource->size_gran);
	}

	list_for_each_entry(table, &ctx->tables->table_list, list)
		if (table->resource_id == resource->id &&
		    table->resource_valid)
			array = true;

	if (array)
		pr_out_array_start(dl, "dpipe_tables");
	else
		print_string(PRINT_ANY, "dpipe_tables", " dpipe_tables none",
			     "none");

	list_for_each_entry(table, &ctx->tables->table_list, list) {
		if (table->resource_id != resource->id ||
		    !table->resource_valid)
			continue;
		pr_out_entry_start(dl);
		check_indent_newline(dl);
		print_string(PRINT_ANY, "table_name", "table_name %s",
			     table->name);
		pr_out_entry_end(dl);
	}
	if (array)
		pr_out_array_end(dl);

	if (list_empty(&resource->resource_list))
		return;

	if (ctx->pending_change) {
		check_indent_newline(dl);
		print_string(PRINT_ANY, "size_valid", "size_valid %s",
			     resource->size_valid ? "true" : "false");
	}
	pr_out_array_start(dl, "resources");
	list_for_each_entry(child_resource, &resource->resource_list, list) {
		pr_out_entry_start(dl);
		resource_show(child_resource, ctx);
		pr_out_entry_end(dl);
	}
	pr_out_array_end(dl);
}

static void
resources_show(struct resource_ctx *ctx, struct nlattr **tb)
{
	struct resources *resources = ctx->resources;
	struct resource *resource;

	list_for_each_entry(resource, &resources->resource_list, list) {
		pr_out_handle_start_arr(ctx->dl, tb);
		resource_show(resource, ctx);
		pr_out_handle_end(ctx->dl);
	}
}

static int resources_get(struct resource_ctx *ctx, struct nlattr **tb)
{
	return resource_get(ctx, NULL, NULL, tb[DEVLINK_ATTR_RESOURCE_LIST]);
}

static int cmd_resource_dump_cb(const struct nlmsghdr *nlh, void *data)
{
	struct resource_ctx *ctx = data;
	struct nlattr *tb[DEVLINK_ATTR_MAX + 1] = {};
	struct genlmsghdr *genl = mnl_nlmsg_get_payload(nlh);
	int err;

	mnl_attr_parse(nlh, sizeof(*genl), attr_cb, tb);
	if (!tb[DEVLINK_ATTR_BUS_NAME] || !tb[DEVLINK_ATTR_DEV_NAME] ||
	    !tb[DEVLINK_ATTR_RESOURCE_LIST])
		return MNL_CB_ERROR;

	err = resources_get(ctx, tb);
	if (err) {
		ctx->err = err;
		return MNL_CB_ERROR;
	}

	if (ctx->print_resources)
		resources_show(ctx, tb);

	return MNL_CB_OK;
}

static int cmd_resource_show(struct dl *dl)
{
	struct nlmsghdr *nlh;
	struct dpipe_ctx dpipe_ctx = {};
	struct resource_ctx resource_ctx = {};
	int err;

	err = dl_argv_parse(dl, DL_OPT_HANDLE, 0);
	if (err)
		return err;

	nlh = mnlg_msg_prepare(dl->nlg, DEVLINK_CMD_DPIPE_TABLE_GET,
			       NLM_F_REQUEST);
	dl_opts_put(nlh, dl);

	err = dpipe_ctx_init(&dpipe_ctx, dl);
	if (err)
		return err;

	err = _mnlg_socket_sndrcv(dl->nlg, nlh, cmd_dpipe_table_show_cb,
				  &dpipe_ctx);
	if (err) {
		pr_err("error get tables %s\n", strerror(dpipe_ctx.err));
		goto out;
	}

	err = resource_ctx_init(&resource_ctx, dl);
	if (err)
		goto out;

	resource_ctx.print_resources = true;
	resource_ctx.tables = dpipe_ctx.tables;
	nlh = mnlg_msg_prepare(dl->nlg, DEVLINK_CMD_RESOURCE_DUMP,
			       NLM_F_REQUEST | NLM_F_ACK);
	dl_opts_put(nlh, dl);
	pr_out_section_start(dl, "resources");
	err = _mnlg_socket_sndrcv(dl->nlg, nlh, cmd_resource_dump_cb,
				  &resource_ctx);
	pr_out_section_end(dl);
	resource_ctx_fini(&resource_ctx);
out:
	dpipe_ctx_fini(&dpipe_ctx);
	return err;
}

static void cmd_resource_help(void)
{
	pr_err("Usage: devlink resource show DEV\n"
	       "       devlink resource set DEV path PATH size SIZE\n");
}

static struct resource *
resource_find_by_name(struct list_head *list, char *name)
{
	struct resource *resource;

	list_for_each_entry(resource, list, list) {
		if (!strcmp(resource->name, name))
			return resource;
	}
	return NULL;
}

static int
resource_path_parse(struct resource_ctx *ctx, const char *resource_path,
		    uint32_t *p_resource_id, bool *p_resource_valid)
{
	struct resource *resource;
	uint32_t resource_id = 0;
	char *resource_path_dup;
	struct list_head *list;
	const char del[] = "/";
	char *resource_name;

	resource_path_dup = strdup(resource_path);
	list = &ctx->resources->resource_list;
	resource_name = strtok(resource_path_dup, del);
	while (resource_name != NULL) {
		resource = resource_find_by_name(list, resource_name);
		if (!resource)
			goto err_resource_lookup;

		list = &resource->resource_list;
		resource_name = strtok(NULL, del);
		resource_id = resource->id;
	}
	free(resource_path_dup);
	*p_resource_valid = true;
	*p_resource_id = resource_id;
	return 0;

err_resource_lookup:
	free(resource_path_dup);
	return -EINVAL;
}

static int cmd_resource_set(struct dl *dl)
{
	struct nlmsghdr *nlh;
	struct resource_ctx ctx = {};
	int err;

	err = resource_ctx_init(&ctx, dl);
	if (err)
		return err;

	ctx.print_resources = false;
	err = dl_argv_parse(dl, DL_OPT_HANDLE | DL_OPT_RESOURCE_PATH |
			    DL_OPT_RESOURCE_SIZE, 0);
	if (err)
		goto out;

	nlh = mnlg_msg_prepare(dl->nlg, DEVLINK_CMD_RESOURCE_DUMP,
			       NLM_F_REQUEST);
	dl_opts_put(nlh, dl);
	err = _mnlg_socket_sndrcv(dl->nlg, nlh, cmd_resource_dump_cb, &ctx);
	if (err) {
		pr_err("error getting resources %s\n", strerror(ctx.err));
		goto out;
	}

	err = resource_path_parse(&ctx, dl->opts.resource_path,
				  &dl->opts.resource_id,
				  &dl->opts.resource_id_valid);
	if (err) {
		pr_err("error parsing resource path %s\n", strerror(-err));
		goto out;
	}

	nlh = mnlg_msg_prepare(dl->nlg, DEVLINK_CMD_RESOURCE_SET,
			       NLM_F_REQUEST | NLM_F_ACK);

	dl_opts_put(nlh, dl);
	err = _mnlg_socket_sndrcv(dl->nlg, nlh, NULL, NULL);
out:
	resource_ctx_fini(&ctx);
	return err;
}

static int cmd_resource(struct dl *dl)
{
	if (dl_argv_match(dl, "help") || dl_no_arg(dl)) {
		cmd_resource_help();
		return 0;
	} else if (dl_argv_match(dl, "show")) {
		dl_arg_inc(dl);
		return cmd_resource_show(dl);
	} else if (dl_argv_match(dl, "set")) {
		dl_arg_inc(dl);
		return cmd_resource_set(dl);
	}
	pr_err("Command \"%s\" not found\n", dl_argv(dl));
	return -ENOENT;
}

static void pr_out_region_handle_start(struct dl *dl, struct nlattr **tb)
{
	const char *bus_name = mnl_attr_get_str(tb[DEVLINK_ATTR_BUS_NAME]);
	const char *dev_name = mnl_attr_get_str(tb[DEVLINK_ATTR_DEV_NAME]);
	const char *region_name = mnl_attr_get_str(tb[DEVLINK_ATTR_REGION_NAME]);
	char buf[256];

	sprintf(buf, "%s/%s/%s", bus_name, dev_name, region_name);
	if (dl->json_output)
		open_json_object(buf);
	else
		pr_out("%s:", buf);
}

static void pr_out_region_handle_end(struct dl *dl)
{
	if (dl->json_output)
		close_json_object();
	else
		pr_out("\n");
}

static void pr_out_region_snapshots_start(struct dl *dl, bool array)
{
	__pr_out_indent_newline(dl);
	if (dl->json_output)
		open_json_array(PRINT_JSON, "snapshot");
	else
		pr_out("snapshot %s", array ? "[" : "");
}

static void pr_out_region_snapshots_end(struct dl *dl, bool array)
{
	if (dl->json_output)
		close_json_array(PRINT_JSON, NULL);
	else if (array)
		pr_out("]");
}

static void pr_out_region_snapshots_id(struct dl *dl, struct nlattr **tb, int index)
{
	uint32_t snapshot_id;

	if (!tb[DEVLINK_ATTR_REGION_SNAPSHOT_ID])
		return;

	snapshot_id = mnl_attr_get_u32(tb[DEVLINK_ATTR_REGION_SNAPSHOT_ID]);

	if (dl->json_output)
		print_uint(PRINT_JSON, NULL, NULL, snapshot_id);
	else
		pr_out("%s%u", index ? " " : "", snapshot_id);
}

static void pr_out_snapshots(struct dl *dl, struct nlattr **tb)
{
	struct nlattr *tb_snapshot[DEVLINK_ATTR_MAX + 1] = {};
	struct nlattr *nla_sanpshot;
	int err, index = 0;

	pr_out_region_snapshots_start(dl, true);
	mnl_attr_for_each_nested(nla_sanpshot, tb[DEVLINK_ATTR_REGION_SNAPSHOTS]) {
		err = mnl_attr_parse_nested(nla_sanpshot, attr_cb, tb_snapshot);
		if (err != MNL_CB_OK)
			return;
		pr_out_region_snapshots_id(dl, tb_snapshot, index++);
	}
	pr_out_region_snapshots_end(dl, true);
}

static void pr_out_snapshot(struct dl *dl, struct nlattr **tb)
{
	pr_out_region_snapshots_start(dl, false);
	pr_out_region_snapshots_id(dl, tb, 0);
	pr_out_region_snapshots_end(dl, false);
}

static void pr_out_region(struct dl *dl, struct nlattr **tb)
{
	pr_out_region_handle_start(dl, tb);

	if (tb[DEVLINK_ATTR_REGION_SIZE])
		pr_out_u64(dl, "size",
			   mnl_attr_get_u64(tb[DEVLINK_ATTR_REGION_SIZE]));

	if (tb[DEVLINK_ATTR_REGION_SNAPSHOTS])
		pr_out_snapshots(dl, tb);

	if (tb[DEVLINK_ATTR_REGION_SNAPSHOT_ID])
		pr_out_snapshot(dl, tb);

	pr_out_region_handle_end(dl);
}

static int cmd_region_show_cb(const struct nlmsghdr *nlh, void *data)
{
	struct genlmsghdr *genl = mnl_nlmsg_get_payload(nlh);
	struct nlattr *tb[DEVLINK_ATTR_MAX + 1] = {};
	struct dl *dl = data;

	mnl_attr_parse(nlh, sizeof(*genl), attr_cb, tb);
	if (!tb[DEVLINK_ATTR_BUS_NAME] || !tb[DEVLINK_ATTR_DEV_NAME] ||
	    !tb[DEVLINK_ATTR_REGION_NAME] || !tb[DEVLINK_ATTR_REGION_SIZE])
		return MNL_CB_ERROR;

	pr_out_region(dl, tb);

	return MNL_CB_OK;
}

static int cmd_region_show(struct dl *dl)
{
	struct nlmsghdr *nlh;
	uint16_t flags = NLM_F_REQUEST | NLM_F_ACK;
	int err;

	if (dl_argc(dl) == 0)
		flags |= NLM_F_DUMP;

	nlh = mnlg_msg_prepare(dl->nlg, DEVLINK_CMD_REGION_GET, flags);

	if (dl_argc(dl) > 0) {
		err = dl_argv_parse_put(nlh, dl, DL_OPT_HANDLE_REGION, 0);
		if (err)
			return err;
	}

	pr_out_section_start(dl, "regions");
	err = _mnlg_socket_sndrcv(dl->nlg, nlh, cmd_region_show_cb, dl);
	pr_out_section_end(dl);
	return err;
}

static int cmd_region_snapshot_del(struct dl *dl)
{
	struct nlmsghdr *nlh;
	int err;

	nlh = mnlg_msg_prepare(dl->nlg, DEVLINK_CMD_REGION_DEL,
			       NLM_F_REQUEST | NLM_F_ACK);

	err = dl_argv_parse_put(nlh, dl, DL_OPT_HANDLE_REGION |
				DL_OPT_REGION_SNAPSHOT_ID, 0);
	if (err)
		return err;

	return _mnlg_socket_sndrcv(dl->nlg, nlh, NULL, NULL);
}

static int cmd_region_read_cb(const struct nlmsghdr *nlh, void *data)
{
	struct nlattr *nla_entry, *nla_chunk_data, *nla_chunk_addr;
	struct genlmsghdr *genl = mnl_nlmsg_get_payload(nlh);
	struct nlattr *tb_field[DEVLINK_ATTR_MAX + 1] = {};
	struct nlattr *tb[DEVLINK_ATTR_MAX + 1] = {};
	struct dl *dl = data;
	int err;

	mnl_attr_parse(nlh, sizeof(*genl), attr_cb, tb);
	if (!tb[DEVLINK_ATTR_BUS_NAME] || !tb[DEVLINK_ATTR_DEV_NAME] ||
	    !tb[DEVLINK_ATTR_REGION_CHUNKS])
		return MNL_CB_ERROR;

	mnl_attr_for_each_nested(nla_entry, tb[DEVLINK_ATTR_REGION_CHUNKS]) {
		err = mnl_attr_parse_nested(nla_entry, attr_cb, tb_field);
		if (err != MNL_CB_OK)
			return MNL_CB_ERROR;

		nla_chunk_data = tb_field[DEVLINK_ATTR_REGION_CHUNK_DATA];
		if (!nla_chunk_data)
			continue;

		nla_chunk_addr = tb_field[DEVLINK_ATTR_REGION_CHUNK_ADDR];
		if (!nla_chunk_addr)
			continue;

		pr_out_region_chunk(dl, mnl_attr_get_payload(nla_chunk_data),
				    mnl_attr_get_payload_len(nla_chunk_data),
				    mnl_attr_get_u64(nla_chunk_addr));
	}
	return MNL_CB_OK;
}

static int cmd_region_dump(struct dl *dl)
{
	struct nlmsghdr *nlh;
	int err;

	nlh = mnlg_msg_prepare(dl->nlg, DEVLINK_CMD_REGION_READ,
			       NLM_F_REQUEST | NLM_F_ACK | NLM_F_DUMP);

	err = dl_argv_parse_put(nlh, dl, DL_OPT_HANDLE_REGION |
				DL_OPT_REGION_SNAPSHOT_ID, 0);
	if (err)
		return err;

	pr_out_section_start(dl, "dump");
	err = _mnlg_socket_sndrcv(dl->nlg, nlh, cmd_region_read_cb, dl);
	pr_out_section_end(dl);
	if (!dl->json_output)
		pr_out("\n");
	return err;
}

static int cmd_region_read(struct dl *dl)
{
	struct nlmsghdr *nlh;
	int err;

	nlh = mnlg_msg_prepare(dl->nlg, DEVLINK_CMD_REGION_READ,
			       NLM_F_REQUEST | NLM_F_ACK | NLM_F_DUMP);

	err = dl_argv_parse_put(nlh, dl, DL_OPT_HANDLE_REGION |
				DL_OPT_REGION_ADDRESS | DL_OPT_REGION_LENGTH |
				DL_OPT_REGION_SNAPSHOT_ID, 0);
	if (err)
		return err;

	pr_out_section_start(dl, "read");
	err = _mnlg_socket_sndrcv(dl->nlg, nlh, cmd_region_read_cb, dl);
	pr_out_section_end(dl);
	if (!dl->json_output)
		pr_out("\n");
	return err;
}

static int cmd_region_snapshot_new_cb(const struct nlmsghdr *nlh, void *data)
{
	struct genlmsghdr *genl = mnl_nlmsg_get_payload(nlh);
	struct nlattr *tb[DEVLINK_ATTR_MAX + 1] = {};
	struct dl *dl = data;

	mnl_attr_parse(nlh, sizeof(*genl), attr_cb, tb);
	if (!tb[DEVLINK_ATTR_BUS_NAME] || !tb[DEVLINK_ATTR_DEV_NAME] ||
	    !tb[DEVLINK_ATTR_REGION_NAME] ||
	    !tb[DEVLINK_ATTR_REGION_SNAPSHOT_ID])
		return MNL_CB_ERROR;

	pr_out_region(dl, tb);

	return MNL_CB_OK;
}

static int cmd_region_snapshot_new(struct dl *dl)
{
	struct nlmsghdr *nlh;
	int err;

	nlh = mnlg_msg_prepare(dl->nlg, DEVLINK_CMD_REGION_NEW,
			       NLM_F_REQUEST | NLM_F_ACK);

	err = dl_argv_parse_put(nlh, dl, DL_OPT_HANDLE_REGION,
				DL_OPT_REGION_SNAPSHOT_ID);
	if (err)
		return err;

	pr_out_section_start(dl, "regions");
	err = _mnlg_socket_sndrcv(dl->nlg, nlh, cmd_region_snapshot_new_cb, dl);
	pr_out_section_end(dl);
	return err;
}

static void cmd_region_help(void)
{
	pr_err("Usage: devlink region show [ DEV/REGION ]\n");
	pr_err("       devlink region del DEV/REGION snapshot SNAPSHOT_ID\n");
	pr_err("       devlink region new DEV/REGION snapshot SNAPSHOT_ID\n");
	pr_err("       devlink region dump DEV/REGION [ snapshot SNAPSHOT_ID ]\n");
	pr_err("       devlink region read DEV/REGION [ snapshot SNAPSHOT_ID ] address ADDRESS length LENGTH\n");
}

static int cmd_region(struct dl *dl)
{
	if (dl_no_arg(dl)) {
		return cmd_region_show(dl);
	} else if (dl_argv_match(dl, "help")) {
		cmd_region_help();
		return 0;
	} else if (dl_argv_match(dl, "show")) {
		dl_arg_inc(dl);
		return cmd_region_show(dl);
	} else if (dl_argv_match(dl, "del")) {
		dl_arg_inc(dl);
		return cmd_region_snapshot_del(dl);
	} else if (dl_argv_match(dl, "dump")) {
		dl_arg_inc(dl);
		return cmd_region_dump(dl);
	} else if (dl_argv_match(dl, "read")) {
		dl_arg_inc(dl);
		return cmd_region_read(dl);
	} else if (dl_argv_match(dl, "new")) {
		dl_arg_inc(dl);
		return cmd_region_snapshot_new(dl);
	}
	pr_err("Command \"%s\" not found\n", dl_argv(dl));
	return -ENOENT;
}

static int cmd_health_set_params(struct dl *dl)
{
	struct nlmsghdr *nlh;
	int err;

	nlh = mnlg_msg_prepare(dl->nlg, DEVLINK_CMD_HEALTH_REPORTER_SET,
			       NLM_F_REQUEST | NLM_F_ACK);
	err = dl_argv_parse(dl, DL_OPT_HANDLE | DL_OPT_HANDLEP | DL_OPT_HEALTH_REPORTER_NAME,
			    DL_OPT_HEALTH_REPORTER_GRACEFUL_PERIOD |
			    DL_OPT_HEALTH_REPORTER_AUTO_RECOVER |
			    DL_OPT_HEALTH_REPORTER_AUTO_DUMP);
	if (err)
		return err;

	dl_opts_put(nlh, dl);
	return _mnlg_socket_sndrcv(dl->nlg, nlh, NULL, NULL);
}

static int cmd_health_dump_clear(struct dl *dl)
{
	struct nlmsghdr *nlh;
	int err;

	nlh = mnlg_msg_prepare(dl->nlg, DEVLINK_CMD_HEALTH_REPORTER_DUMP_CLEAR,
			       NLM_F_REQUEST | NLM_F_ACK);

	err = dl_argv_parse_put(nlh, dl,
				DL_OPT_HANDLE | DL_OPT_HANDLEP |
				DL_OPT_HEALTH_REPORTER_NAME, 0);
	if (err)
		return err;

	dl_opts_put(nlh, dl);
	return _mnlg_socket_sndrcv(dl->nlg, nlh, NULL, NULL);
}

static int fmsg_value_show(struct dl *dl, int type, struct nlattr *nl_data)
{
	uint8_t *data;
	uint32_t len;

	check_indent_newline(dl);
	switch (type) {
	case MNL_TYPE_FLAG:
		print_bool(PRINT_ANY, NULL, "%s", mnl_attr_get_u8(nl_data));
		break;
	case MNL_TYPE_U8:
		print_uint(PRINT_ANY, NULL, "%u", mnl_attr_get_u8(nl_data));
		break;
	case MNL_TYPE_U16:
		print_uint(PRINT_ANY, NULL, "%u", mnl_attr_get_u16(nl_data));
		break;
	case MNL_TYPE_U32:
		print_uint(PRINT_ANY, NULL, "%u", mnl_attr_get_u32(nl_data));
		break;
	case MNL_TYPE_U64:
		print_u64(PRINT_ANY, NULL, "%"PRIu64, mnl_attr_get_u64(nl_data));
		break;
	case MNL_TYPE_NUL_STRING:
		print_string(PRINT_ANY, NULL, "%s", mnl_attr_get_str(nl_data));
		break;
	case MNL_TYPE_BINARY:
		len = mnl_attr_get_payload_len(nl_data);
		data = mnl_attr_get_payload(nl_data);
		pr_out_binary_value(dl, data, len);
		break;
	default:
		return -EINVAL;
	}
	return MNL_CB_OK;
}

static void pr_out_fmsg_name(struct dl *dl, char **name)
{
	if (!*name)
		return;

	pr_out_name(dl, *name);
	free(*name);
	*name = NULL;
}

struct nest_entry {
	int attr_type;
	struct list_head list;
};

struct fmsg_cb_data {
	char *name;
	struct dl *dl;
	uint8_t value_type;
	struct list_head entry_list;
};

static int cmd_fmsg_nest_queue(struct fmsg_cb_data *fmsg_data,
			       uint8_t *attr_value, bool insert)
{
	struct nest_entry *entry;

	if (insert) {
		entry = malloc(sizeof(struct nest_entry));
		if (!entry)
			return -ENOMEM;

		entry->attr_type = *attr_value;
		list_add(&entry->list, &fmsg_data->entry_list);
	} else {
		if (list_empty(&fmsg_data->entry_list))
			return MNL_CB_ERROR;
		entry = list_first_entry(&fmsg_data->entry_list,
					 struct nest_entry, list);
		*attr_value = entry->attr_type;
		list_del(&entry->list);
		free(entry);
	}
	return MNL_CB_OK;
}

static void pr_out_fmsg_group_start(struct dl *dl, char **name)
{
	__pr_out_newline();
	pr_out_fmsg_name(dl, name);
	__pr_out_newline();
	__pr_out_indent_inc();
}

static void pr_out_fmsg_group_end(struct dl *dl)
{
	__pr_out_newline();
	__pr_out_indent_dec();
}

static void pr_out_fmsg_start_object(struct dl *dl, char **name)
{
	if (dl->json_output) {
		pr_out_fmsg_name(dl, name);
		open_json_object(NULL);
	} else {
		pr_out_fmsg_group_start(dl, name);
	}
}

static void pr_out_fmsg_end_object(struct dl *dl)
{
	if (dl->json_output)
		close_json_object();
	else
		pr_out_fmsg_group_end(dl);
}

static void pr_out_fmsg_start_array(struct dl *dl, char **name)
{
	if (dl->json_output) {
		pr_out_fmsg_name(dl, name);
		open_json_array(PRINT_JSON, NULL);
	} else {
		pr_out_fmsg_group_start(dl, name);
	}
}

static void pr_out_fmsg_end_array(struct dl *dl)
{
	if (dl->json_output)
		close_json_array(PRINT_JSON, NULL);
	else
		pr_out_fmsg_group_end(dl);
}

static int cmd_fmsg_nest(struct fmsg_cb_data *fmsg_data, uint8_t nest_value,
			 bool start)
{
	struct dl *dl = fmsg_data->dl;
	uint8_t value = nest_value;
	int err;

	err = cmd_fmsg_nest_queue(fmsg_data, &value, start);
	if (err != MNL_CB_OK)
		return err;

	switch (value) {
	case DEVLINK_ATTR_FMSG_OBJ_NEST_START:
		if (start)
			pr_out_fmsg_start_object(dl, &fmsg_data->name);
		else
			pr_out_fmsg_end_object(dl);
		break;
	case DEVLINK_ATTR_FMSG_PAIR_NEST_START:
		break;
	case DEVLINK_ATTR_FMSG_ARR_NEST_START:
		if (start)
			pr_out_fmsg_start_array(dl, &fmsg_data->name);
		else
			pr_out_fmsg_end_array(dl);
		break;
	default:
		return -EINVAL;
	}
	return MNL_CB_OK;
}

static int cmd_fmsg_object_cb(const struct nlmsghdr *nlh, void *data)
{
	struct genlmsghdr *genl = mnl_nlmsg_get_payload(nlh);
	struct nlattr *tb[DEVLINK_ATTR_MAX + 1] = {};
	struct fmsg_cb_data *fmsg_data = data;
	struct dl *dl = fmsg_data->dl;
	struct nlattr *nla_object;
	int attr_type;
	int err;

	mnl_attr_parse(nlh, sizeof(*genl), attr_cb, tb);
	if (!tb[DEVLINK_ATTR_FMSG])
		return MNL_CB_ERROR;

	mnl_attr_for_each_nested(nla_object, tb[DEVLINK_ATTR_FMSG]) {
		attr_type = mnl_attr_get_type(nla_object);
		switch (attr_type) {
		case DEVLINK_ATTR_FMSG_OBJ_NEST_START:
		case DEVLINK_ATTR_FMSG_PAIR_NEST_START:
		case DEVLINK_ATTR_FMSG_ARR_NEST_START:
			err = cmd_fmsg_nest(fmsg_data, attr_type, true);
			if (err != MNL_CB_OK)
				return err;
			break;
		case DEVLINK_ATTR_FMSG_NEST_END:
			err = cmd_fmsg_nest(fmsg_data, attr_type, false);
			if (err != MNL_CB_OK)
				return err;
			break;
		case DEVLINK_ATTR_FMSG_OBJ_NAME:
			free(fmsg_data->name);
			fmsg_data->name = strdup(mnl_attr_get_str(nla_object));
			if (!fmsg_data->name)
				return -ENOMEM;
			break;
		case DEVLINK_ATTR_FMSG_OBJ_VALUE_TYPE:
			fmsg_data->value_type = mnl_attr_get_u8(nla_object);
			break;
		case DEVLINK_ATTR_FMSG_OBJ_VALUE_DATA:
			pr_out_fmsg_name(dl, &fmsg_data->name);
			err = fmsg_value_show(dl, fmsg_data->value_type,
					      nla_object);
			if (err != MNL_CB_OK)
				return err;
			break;
		default:
			return -EINVAL;
		}
	}
	return MNL_CB_OK;
}

static void cmd_fmsg_init(struct dl *dl, struct fmsg_cb_data *data)
{
	/* FMSG is dynamic: opening of an object or array causes a
	 * newline. JSON starts with an { or [, but plain text should
	 * not start with a new line. Ensure this by setting
	 * g_new_line_count to 1: avoiding newline before the first
	 * print.
	 */
	g_new_line_count = 1;
	data->name = NULL;
	data->dl = dl;
	INIT_LIST_HEAD(&data->entry_list);
}

static int cmd_health_object_common(struct dl *dl, uint8_t cmd, uint16_t flags)
{
	struct fmsg_cb_data data;
	struct nlmsghdr *nlh;
	int err;

	nlh = mnlg_msg_prepare(dl->nlg, cmd, flags | NLM_F_REQUEST | NLM_F_ACK);

	err = dl_argv_parse_put(nlh, dl,
				DL_OPT_HANDLE | DL_OPT_HANDLEP |
				DL_OPT_HEALTH_REPORTER_NAME, 0);
	if (err)
		return err;

	cmd_fmsg_init(dl, &data);
	err = _mnlg_socket_sndrcv(dl->nlg, nlh, cmd_fmsg_object_cb, &data);
	free(data.name);
	return err;
}

static int cmd_health_dump_show(struct dl *dl)
{
	return cmd_health_object_common(dl,
					DEVLINK_CMD_HEALTH_REPORTER_DUMP_GET,
					NLM_F_DUMP);
}

static int cmd_health_diagnose(struct dl *dl)
{
	return cmd_health_object_common(dl,
					DEVLINK_CMD_HEALTH_REPORTER_DIAGNOSE,
					0);
}

static int cmd_health_test(struct dl *dl)
{
	return cmd_health_object_common(dl,
					DEVLINK_CMD_HEALTH_REPORTER_TEST,
					0);
}

static int cmd_health_recover(struct dl *dl)
{
	struct nlmsghdr *nlh;
	int err;

	nlh = mnlg_msg_prepare(dl->nlg, DEVLINK_CMD_HEALTH_REPORTER_RECOVER,
			       NLM_F_REQUEST | NLM_F_ACK);

	err = dl_argv_parse_put(nlh, dl,
				DL_OPT_HANDLE | DL_OPT_HANDLEP |
				DL_OPT_HEALTH_REPORTER_NAME, 0);
	if (err)
		return err;

	dl_opts_put(nlh, dl);
	return _mnlg_socket_sndrcv(dl->nlg, nlh, NULL, NULL);
}

enum devlink_health_reporter_state {
	DEVLINK_HEALTH_REPORTER_STATE_HEALTHY,
	DEVLINK_HEALTH_REPORTER_STATE_ERROR,
};

static const char *health_state_name(uint8_t state)
{
	switch (state) {
	case DEVLINK_HEALTH_REPORTER_STATE_HEALTHY:
		return HEALTH_REPORTER_STATE_HEALTHY_STR;
	case DEVLINK_HEALTH_REPORTER_STATE_ERROR:
		return HEALTH_REPORTER_STATE_ERROR_STR;
	default:
		return "<unknown state>";
	}
}

static void pr_out_dump_reporter_format_logtime(struct dl *dl, const struct nlattr *attr)
{
	char dump_date[HEALTH_REPORTER_TIMESTAMP_FMT_LEN];
	char dump_time[HEALTH_REPORTER_TIMESTAMP_FMT_LEN];
	uint64_t time_ms = mnl_attr_get_u64(attr);
	struct sysinfo s_info;
	struct tm *info;
	time_t now, sec;
	int err;

	time(&now);
	info = localtime(&now);
	err = sysinfo(&s_info);
	if (err)
		goto out;
	/* Subtract uptime in sec from now yields the time of system
	 * uptime. To this, add time_ms which is the amount of
	 * milliseconds elapsed between uptime and the dump taken.
	 */
	sec = now - s_info.uptime + time_ms / 1000;
	info = localtime(&sec);
out:
	strftime(dump_date, HEALTH_REPORTER_TIMESTAMP_FMT_LEN, "%Y-%m-%d", info);
	strftime(dump_time, HEALTH_REPORTER_TIMESTAMP_FMT_LEN, "%H:%M:%S", info);
	check_indent_newline(dl);
	print_string(PRINT_ANY, "last_dump_date", "last_dump_date %s", dump_date);
	print_string(PRINT_ANY, "last_dump_time", " last_dump_time %s", dump_time);
}

static void pr_out_dump_report_timestamp(struct dl *dl, const struct nlattr *attr)
{
	char dump_date[HEALTH_REPORTER_TIMESTAMP_FMT_LEN];
	char dump_time[HEALTH_REPORTER_TIMESTAMP_FMT_LEN];
	time_t tv_sec;
	struct tm *tm;
	uint64_t ts;

	ts = mnl_attr_get_u64(attr);
	tv_sec = ts / 1000000000;
	tm = localtime(&tv_sec);

	strftime(dump_date, HEALTH_REPORTER_TIMESTAMP_FMT_LEN, "%Y-%m-%d", tm);
	strftime(dump_time, HEALTH_REPORTER_TIMESTAMP_FMT_LEN, "%H:%M:%S", tm);

	check_indent_newline(dl);
	print_string(PRINT_ANY, "last_dump_date", "last_dump_date %s", dump_date);
	print_string(PRINT_ANY, "last_dump_time", " last_dump_time %s", dump_time);
}

static void pr_out_health(struct dl *dl, struct nlattr **tb_health,
			  bool print_device, bool print_port)
{
	struct nlattr *tb[DEVLINK_ATTR_MAX + 1] = {};
	enum devlink_health_reporter_state state;
	int err;

	err = mnl_attr_parse_nested(tb_health[DEVLINK_ATTR_HEALTH_REPORTER],
				    attr_cb, tb);
	if (err != MNL_CB_OK)
		return;

	if (!tb[DEVLINK_ATTR_HEALTH_REPORTER_NAME] ||
	    !tb[DEVLINK_ATTR_HEALTH_REPORTER_ERR_COUNT] ||
	    !tb[DEVLINK_ATTR_HEALTH_REPORTER_RECOVER_COUNT] ||
	    !tb[DEVLINK_ATTR_HEALTH_REPORTER_STATE])
		return;

	if (!print_device && !print_port)
		return;
	if (print_port) {
		if (!print_device && !tb_health[DEVLINK_ATTR_PORT_INDEX])
			return;
		else if (tb_health[DEVLINK_ATTR_PORT_INDEX])
			pr_out_port_handle_start_arr(dl, tb_health, false);
	}
	if (print_device) {
		if (!print_port && tb_health[DEVLINK_ATTR_PORT_INDEX])
			return;
		else if (!tb_health[DEVLINK_ATTR_PORT_INDEX])
			pr_out_handle_start_arr(dl, tb_health);
	}

	check_indent_newline(dl);
	print_string(PRINT_ANY, "reporter", "reporter %s",
		     mnl_attr_get_str(tb[DEVLINK_ATTR_HEALTH_REPORTER_NAME]));
	if (!dl->json_output) {
		__pr_out_newline();
		__pr_out_indent_inc();
	}
	state = mnl_attr_get_u8(tb[DEVLINK_ATTR_HEALTH_REPORTER_STATE]);
	check_indent_newline(dl);
	print_string(PRINT_ANY, "state", "state %s", health_state_name(state));
	pr_out_u64(dl, "error",
		   mnl_attr_get_u64(tb[DEVLINK_ATTR_HEALTH_REPORTER_ERR_COUNT]));
	pr_out_u64(dl, "recover",
		   mnl_attr_get_u64(tb[DEVLINK_ATTR_HEALTH_REPORTER_RECOVER_COUNT]));
	if (tb[DEVLINK_ATTR_HEALTH_REPORTER_DUMP_TS_NS])
		pr_out_dump_report_timestamp(dl, tb[DEVLINK_ATTR_HEALTH_REPORTER_DUMP_TS_NS]);
	else if (tb[DEVLINK_ATTR_HEALTH_REPORTER_DUMP_TS])
		pr_out_dump_reporter_format_logtime(dl, tb[DEVLINK_ATTR_HEALTH_REPORTER_DUMP_TS]);
	if (tb[DEVLINK_ATTR_HEALTH_REPORTER_GRACEFUL_PERIOD])
		pr_out_u64(dl, "grace_period",
			   mnl_attr_get_u64(tb[DEVLINK_ATTR_HEALTH_REPORTER_GRACEFUL_PERIOD]));
	if (tb[DEVLINK_ATTR_HEALTH_REPORTER_AUTO_RECOVER])
		print_bool(PRINT_ANY, "auto_recover", " auto_recover %s",
			   mnl_attr_get_u8(tb[DEVLINK_ATTR_HEALTH_REPORTER_AUTO_RECOVER]));
	if (tb[DEVLINK_ATTR_HEALTH_REPORTER_AUTO_DUMP])
		print_bool(PRINT_ANY, "auto_dump", " auto_dump %s",
			   mnl_attr_get_u8(tb[DEVLINK_ATTR_HEALTH_REPORTER_AUTO_DUMP]));

	__pr_out_indent_dec();
	pr_out_handle_end(dl);
}

struct health_ctx {
	struct dl *dl;
	bool show_device;
	bool show_port;
};

static int cmd_health_show_cb(const struct nlmsghdr *nlh, void *data)
{
	struct genlmsghdr *genl = mnl_nlmsg_get_payload(nlh);
	struct nlattr *tb[DEVLINK_ATTR_MAX + 1] = {};
	struct health_ctx *ctx = data;
	struct dl *dl = ctx->dl;

	mnl_attr_parse(nlh, sizeof(*genl), attr_cb, tb);
	if (!tb[DEVLINK_ATTR_BUS_NAME] || !tb[DEVLINK_ATTR_DEV_NAME] ||
	    !tb[DEVLINK_ATTR_HEALTH_REPORTER])
		return MNL_CB_ERROR;

	pr_out_health(dl, tb, ctx->show_device, ctx->show_port);

	return MNL_CB_OK;
}

static int __cmd_health_show(struct dl *dl, bool show_device, bool show_port)
{
	struct nlmsghdr *nlh;
	struct health_ctx ctx = { dl, show_device, show_port };
	uint16_t flags = NLM_F_REQUEST | NLM_F_ACK;
	int err;

	if (dl_argc(dl) == 0)
		flags |= NLM_F_DUMP;
	nlh = mnlg_msg_prepare(dl->nlg, DEVLINK_CMD_HEALTH_REPORTER_GET,
			       flags);

	if (dl_argc(dl) > 0) {
		ctx.show_port = true;
		err = dl_argv_parse_put(nlh, dl,
					DL_OPT_HANDLE | DL_OPT_HANDLEP |
					DL_OPT_HEALTH_REPORTER_NAME, 0);
		if (err)
			return err;
	}
	pr_out_section_start(dl, "health");

	err = _mnlg_socket_sndrcv(dl->nlg, nlh, cmd_health_show_cb, &ctx);
	pr_out_section_end(dl);
	return err;
}

static void cmd_health_help(void)
{
	pr_err("Usage: devlink health show [ { DEV | DEV/PORT_INDEX } reporter REPORTER_NAME ]\n");
	pr_err("       devlink health recover { DEV | DEV/PORT_INDEX } reporter REPORTER_NAME\n");
	pr_err("       devlink health diagnose { DEV | DEV/PORT_INDEX } reporter REPORTER_NAME\n");
	pr_err("       devlink health test { DEV | DEV/PORT_INDEX } reporter REPORTER_NAME\n");
	pr_err("       devlink health dump show { DEV | DEV/PORT_INDEX } reporter REPORTER_NAME\n");
	pr_err("       devlink health dump clear { DEV | DEV/PORT_INDEX } reporter REPORTER_NAME\n");
	pr_err("       devlink health set { DEV | DEV/PORT_INDEX } reporter REPORTER_NAME\n");
	pr_err("                          [ grace_period MSEC ]\n");
	pr_err("                          [ auto_recover { true | false } ]\n");
	pr_err("                          [ auto_dump    { true | false } ]\n");
}

static int cmd_health(struct dl *dl)
{
	if (dl_argv_match(dl, "help")) {
		cmd_health_help();
		return 0;
	} else if (dl_argv_match(dl, "show") ||
		   dl_argv_match(dl, "list") || dl_no_arg(dl)) {
		dl_arg_inc(dl);
		return __cmd_health_show(dl, true, true);
	} else if (dl_argv_match(dl, "recover")) {
		dl_arg_inc(dl);
		return cmd_health_recover(dl);
	} else if (dl_argv_match(dl, "diagnose")) {
		dl_arg_inc(dl);
		return cmd_health_diagnose(dl);
	} else if (dl_argv_match(dl, "test")) {
		dl_arg_inc(dl);
		return cmd_health_test(dl);
	} else if (dl_argv_match(dl, "dump")) {
		dl_arg_inc(dl);
		if (dl_argv_match(dl, "show")) {
			dl_arg_inc(dl);
			return cmd_health_dump_show(dl);
		} else if (dl_argv_match(dl, "clear")) {
			dl_arg_inc(dl);
			return cmd_health_dump_clear(dl);
		}
	} else if (dl_argv_match(dl, "set")) {
		dl_arg_inc(dl);
		return cmd_health_set_params(dl);
	}
	pr_err("Command \"%s\" not found\n", dl_argv(dl));
	return -ENOENT;
}

static const char *trap_type_name(uint8_t type)
{
	switch (type) {
	case DEVLINK_TRAP_TYPE_DROP:
		return "drop";
	case DEVLINK_TRAP_TYPE_EXCEPTION:
		return "exception";
	case DEVLINK_TRAP_TYPE_CONTROL:
		return "control";
	default:
		return "<unknown type>";
	}
}

static const char *trap_action_name(uint8_t action)
{
	switch (action) {
	case DEVLINK_TRAP_ACTION_DROP:
		return "drop";
	case DEVLINK_TRAP_ACTION_TRAP:
		return "trap";
	case DEVLINK_TRAP_ACTION_MIRROR:
		return "mirror";
	default:
		return "<unknown action>";
	}
}

static const char *trap_metadata_name(const struct nlattr *attr)
{
	switch (attr->nla_type) {
	case DEVLINK_ATTR_TRAP_METADATA_TYPE_IN_PORT:
		return "input_port";
	case DEVLINK_ATTR_TRAP_METADATA_TYPE_FA_COOKIE:
		return "flow_action_cookie";
	default:
		return "<unknown metadata type>";
	}
}
static void pr_out_trap_metadata(struct dl *dl, struct nlattr *attr)
{
	struct nlattr *attr_metadata;

	pr_out_array_start(dl, "metadata");
	mnl_attr_for_each_nested(attr_metadata, attr) {
		check_indent_newline(dl);
		print_string(PRINT_ANY, NULL, "%s",
			     trap_metadata_name(attr_metadata));
	}
	pr_out_array_end(dl);
}

static void pr_out_trap(struct dl *dl, struct nlattr **tb, bool array)
{
	uint8_t action = mnl_attr_get_u8(tb[DEVLINK_ATTR_TRAP_ACTION]);
	uint8_t type = mnl_attr_get_u8(tb[DEVLINK_ATTR_TRAP_TYPE]);

	if (array)
		pr_out_handle_start_arr(dl, tb);
	else
		__pr_out_handle_start(dl, tb, true, false);

	check_indent_newline(dl);
	print_string(PRINT_ANY, "name", "name %s",
		     mnl_attr_get_str(tb[DEVLINK_ATTR_TRAP_NAME]));
	print_string(PRINT_ANY, "type", " type %s", trap_type_name(type));
	print_bool(PRINT_ANY, "generic", " generic %s", !!tb[DEVLINK_ATTR_TRAP_GENERIC]);
	print_string(PRINT_ANY, "action", " action %s", trap_action_name(action));
	print_string(PRINT_ANY, "group", " group %s",
		     mnl_attr_get_str(tb[DEVLINK_ATTR_TRAP_GROUP_NAME]));
	if (dl->verbose)
		pr_out_trap_metadata(dl, tb[DEVLINK_ATTR_TRAP_METADATA]);
	pr_out_stats(dl, tb[DEVLINK_ATTR_STATS]);
	pr_out_handle_end(dl);
}

static int cmd_trap_show_cb(const struct nlmsghdr *nlh, void *data)
{
	struct genlmsghdr *genl = mnl_nlmsg_get_payload(nlh);
	struct nlattr *tb[DEVLINK_ATTR_MAX + 1] = {};
	struct dl *dl = data;

	mnl_attr_parse(nlh, sizeof(*genl), attr_cb, tb);
	if (!tb[DEVLINK_ATTR_BUS_NAME] || !tb[DEVLINK_ATTR_DEV_NAME] ||
	    !tb[DEVLINK_ATTR_TRAP_NAME] || !tb[DEVLINK_ATTR_TRAP_TYPE] ||
	    !tb[DEVLINK_ATTR_TRAP_ACTION] ||
	    !tb[DEVLINK_ATTR_TRAP_GROUP_NAME] ||
	    !tb[DEVLINK_ATTR_TRAP_METADATA] || !tb[DEVLINK_ATTR_STATS])
		return MNL_CB_ERROR;

	pr_out_trap(dl, tb, true);

	return MNL_CB_OK;
}

static void cmd_trap_help(void)
{
	pr_err("Usage: devlink trap set DEV trap TRAP [ action { trap | drop | mirror } ]\n");
	pr_err("       devlink trap show [ DEV trap TRAP ]\n");
	pr_err("       devlink trap group set DEV group GROUP [ action { trap | drop | mirror } ]\n");
	pr_err("                              [ policer POLICER ] [ nopolicer ]\n");
	pr_err("       devlink trap group show [ DEV group GROUP ]\n");
	pr_err("       devlink trap policer set DEV policer POLICER [ rate RATE ] [ burst BURST ]\n");
	pr_err("       devlink trap policer show DEV policer POLICER\n");
}

static int cmd_trap_show(struct dl *dl)
{
	uint16_t flags = NLM_F_REQUEST | NLM_F_ACK;
	struct nlmsghdr *nlh;
	int err;

	if (dl_argc(dl) == 0)
		flags |= NLM_F_DUMP;

	nlh = mnlg_msg_prepare(dl->nlg, DEVLINK_CMD_TRAP_GET, flags);

	if (dl_argc(dl) > 0) {
		err = dl_argv_parse_put(nlh, dl,
					DL_OPT_HANDLE | DL_OPT_TRAP_NAME, 0);
		if (err)
			return err;
	}

	pr_out_section_start(dl, "trap");
	err = _mnlg_socket_sndrcv(dl->nlg, nlh, cmd_trap_show_cb, dl);
	pr_out_section_end(dl);

	return err;
}

static int cmd_trap_set(struct dl *dl)
{
	struct nlmsghdr *nlh;
	int err;

	nlh = mnlg_msg_prepare(dl->nlg, DEVLINK_CMD_TRAP_SET,
			       NLM_F_REQUEST | NLM_F_ACK);

	err = dl_argv_parse_put(nlh, dl, DL_OPT_HANDLE | DL_OPT_TRAP_NAME,
				DL_OPT_TRAP_ACTION);
	if (err)
		return err;

	return _mnlg_socket_sndrcv(dl->nlg, nlh, NULL, NULL);
}

static void pr_out_trap_group(struct dl *dl, struct nlattr **tb, bool array)
{
	if (array)
		pr_out_handle_start_arr(dl, tb);
	else
		__pr_out_handle_start(dl, tb, true, false);

	check_indent_newline(dl);
	print_string(PRINT_ANY, "name", "name %s",
		     mnl_attr_get_str(tb[DEVLINK_ATTR_TRAP_GROUP_NAME]));
	print_bool(PRINT_ANY, "generic", " generic %s", !!tb[DEVLINK_ATTR_TRAP_GENERIC]);
	if (tb[DEVLINK_ATTR_TRAP_POLICER_ID])
		print_uint(PRINT_ANY, "policer", " policer %u",
			   mnl_attr_get_u32(tb[DEVLINK_ATTR_TRAP_POLICER_ID]));
	pr_out_stats(dl, tb[DEVLINK_ATTR_STATS]);
	pr_out_handle_end(dl);
}

static int cmd_trap_group_show_cb(const struct nlmsghdr *nlh, void *data)
{
	struct genlmsghdr *genl = mnl_nlmsg_get_payload(nlh);
	struct nlattr *tb[DEVLINK_ATTR_MAX + 1] = {};
	struct dl *dl = data;

	mnl_attr_parse(nlh, sizeof(*genl), attr_cb, tb);
	if (!tb[DEVLINK_ATTR_BUS_NAME] || !tb[DEVLINK_ATTR_DEV_NAME] ||
	    !tb[DEVLINK_ATTR_TRAP_GROUP_NAME] || !tb[DEVLINK_ATTR_STATS])
		return MNL_CB_ERROR;

	pr_out_trap_group(dl, tb, true);

	return MNL_CB_OK;
}

static int cmd_trap_group_show(struct dl *dl)
{
	uint16_t flags = NLM_F_REQUEST | NLM_F_ACK;
	struct nlmsghdr *nlh;
	int err;

	if (dl_argc(dl) == 0)
		flags |= NLM_F_DUMP;

	nlh = mnlg_msg_prepare(dl->nlg, DEVLINK_CMD_TRAP_GROUP_GET, flags);

	if (dl_argc(dl) > 0) {
		err = dl_argv_parse_put(nlh, dl,
					DL_OPT_HANDLE | DL_OPT_TRAP_GROUP_NAME,
					0);
		if (err)
			return err;
	}

	pr_out_section_start(dl, "trap_group");
	err = _mnlg_socket_sndrcv(dl->nlg, nlh, cmd_trap_group_show_cb, dl);
	pr_out_section_end(dl);

	return err;
}

static int cmd_trap_group_set(struct dl *dl)
{
	struct nlmsghdr *nlh;
	int err;

	nlh = mnlg_msg_prepare(dl->nlg, DEVLINK_CMD_TRAP_GROUP_SET,
			       NLM_F_REQUEST | NLM_F_ACK);

	err = dl_argv_parse_put(nlh, dl,
				DL_OPT_HANDLE | DL_OPT_TRAP_GROUP_NAME,
				DL_OPT_TRAP_ACTION | DL_OPT_TRAP_POLICER_ID);
	if (err)
		return err;

	return _mnlg_socket_sndrcv(dl->nlg, nlh, NULL, NULL);
}

static int cmd_trap_group(struct dl *dl)
{
	if (dl_argv_match(dl, "help")) {
		cmd_trap_help();
		return 0;
	} else if (dl_argv_match(dl, "show") ||
		   dl_argv_match(dl, "list") || dl_no_arg(dl)) {
		dl_arg_inc(dl);
		return cmd_trap_group_show(dl);
	} else if (dl_argv_match(dl, "set")) {
		dl_arg_inc(dl);
		return cmd_trap_group_set(dl);
	}
	pr_err("Command \"%s\" not found\n", dl_argv(dl));
	return -ENOENT;
}

static void pr_out_trap_policer(struct dl *dl, struct nlattr **tb, bool array)
{
	if (array)
		pr_out_handle_start_arr(dl, tb);
	else
		__pr_out_handle_start(dl, tb, true, false);

	check_indent_newline(dl);
	print_uint(PRINT_ANY, "policer", "policer %u",
		   mnl_attr_get_u32(tb[DEVLINK_ATTR_TRAP_POLICER_ID]));
	print_u64(PRINT_ANY, "rate", " rate %llu",
		   mnl_attr_get_u64(tb[DEVLINK_ATTR_TRAP_POLICER_RATE]));
	print_u64(PRINT_ANY, "burst", " burst %llu",
		   mnl_attr_get_u64(tb[DEVLINK_ATTR_TRAP_POLICER_BURST]));
	if (tb[DEVLINK_ATTR_STATS])
		pr_out_stats(dl, tb[DEVLINK_ATTR_STATS]);
	pr_out_handle_end(dl);
}

static int cmd_trap_policer_show_cb(const struct nlmsghdr *nlh, void *data)
{
	struct genlmsghdr *genl = mnl_nlmsg_get_payload(nlh);
	struct nlattr *tb[DEVLINK_ATTR_MAX + 1] = {};
	struct dl *dl = data;

	mnl_attr_parse(nlh, sizeof(*genl), attr_cb, tb);
	if (!tb[DEVLINK_ATTR_BUS_NAME] || !tb[DEVLINK_ATTR_DEV_NAME] ||
	    !tb[DEVLINK_ATTR_TRAP_POLICER_ID] ||
	    !tb[DEVLINK_ATTR_TRAP_POLICER_RATE] ||
	    !tb[DEVLINK_ATTR_TRAP_POLICER_BURST])
		return MNL_CB_ERROR;

	pr_out_trap_policer(dl, tb, true);

	return MNL_CB_OK;
}

static int cmd_trap_policer_show(struct dl *dl)
{
	uint16_t flags = NLM_F_REQUEST | NLM_F_ACK;
	struct nlmsghdr *nlh;
	int err;

	if (dl_argc(dl) == 0)
		flags |= NLM_F_DUMP;

	nlh = mnlg_msg_prepare(dl->nlg, DEVLINK_CMD_TRAP_POLICER_GET, flags);

	if (dl_argc(dl) > 0) {
		err = dl_argv_parse_put(nlh, dl,
					DL_OPT_HANDLE | DL_OPT_TRAP_POLICER_ID,
					0);
		if (err)
			return err;
	}

	pr_out_section_start(dl, "trap_policer");
	err = _mnlg_socket_sndrcv(dl->nlg, nlh, cmd_trap_policer_show_cb, dl);
	pr_out_section_end(dl);

	return err;
}

static int cmd_trap_policer_set(struct dl *dl)
{
	struct nlmsghdr *nlh;
	int err;

	nlh = mnlg_msg_prepare(dl->nlg, DEVLINK_CMD_TRAP_POLICER_SET,
			       NLM_F_REQUEST | NLM_F_ACK);

	err = dl_argv_parse_put(nlh, dl,
				DL_OPT_HANDLE | DL_OPT_TRAP_POLICER_ID,
				DL_OPT_TRAP_POLICER_RATE |
				DL_OPT_TRAP_POLICER_BURST);
	if (err)
		return err;

	return _mnlg_socket_sndrcv(dl->nlg, nlh, NULL, NULL);
}

static int cmd_trap_policer(struct dl *dl)
{
	if (dl_argv_match(dl, "help")) {
		cmd_trap_help();
		return 0;
	} else if (dl_argv_match(dl, "show") ||
		   dl_argv_match(dl, "list") || dl_no_arg(dl)) {
		dl_arg_inc(dl);
		return cmd_trap_policer_show(dl);
	} else if (dl_argv_match(dl, "set")) {
		dl_arg_inc(dl);
		return cmd_trap_policer_set(dl);
	}
	pr_err("Command \"%s\" not found\n", dl_argv(dl));
	return -ENOENT;
}

static int cmd_trap(struct dl *dl)
{
	if (dl_argv_match(dl, "help")) {
		cmd_trap_help();
		return 0;
	} else if (dl_argv_match(dl, "show") ||
		   dl_argv_match(dl, "list") || dl_no_arg(dl)) {
		dl_arg_inc(dl);
		return cmd_trap_show(dl);
	} else if (dl_argv_match(dl, "set")) {
		dl_arg_inc(dl);
		return cmd_trap_set(dl);
	} else if (dl_argv_match(dl, "group")) {
		dl_arg_inc(dl);
		return cmd_trap_group(dl);
	} else if (dl_argv_match(dl, "policer")) {
		dl_arg_inc(dl);
		return cmd_trap_policer(dl);
	}
	pr_err("Command \"%s\" not found\n", dl_argv(dl));
	return -ENOENT;
}

static void help(void)
{
	pr_err("Usage: devlink [ OPTIONS ] OBJECT { COMMAND | help }\n"
	       "       devlink [ -f[orce] ] -b[atch] filename -N[etns] netnsname\n"
	       "where  OBJECT := { dev | port | sb | monitor | dpipe | resource | region | health | trap }\n"
	       "       OPTIONS := { -V[ersion] | -n[o-nice-names] | -j[son] | -p[retty] | -v[erbose] -s[tatistics] }\n");
}

static int dl_cmd(struct dl *dl, int argc, char **argv)
{
	dl->argc = argc;
	dl->argv = argv;

	if (dl_argv_match(dl, "help") || dl_no_arg(dl)) {
		help();
		return 0;
	} else if (dl_argv_match(dl, "dev")) {
		dl_arg_inc(dl);
		return cmd_dev(dl);
	} else if (dl_argv_match(dl, "port")) {
		dl_arg_inc(dl);
		return cmd_port(dl);
	} else if (dl_argv_match(dl, "sb")) {
		dl_arg_inc(dl);
		return cmd_sb(dl);
	} else if (dl_argv_match(dl, "monitor")) {
		dl_arg_inc(dl);
		return cmd_mon(dl);
	} else if (dl_argv_match(dl, "dpipe")) {
		dl_arg_inc(dl);
		return cmd_dpipe(dl);
	} else if (dl_argv_match(dl, "resource")) {
		dl_arg_inc(dl);
		return cmd_resource(dl);
	} else if (dl_argv_match(dl, "region")) {
		dl_arg_inc(dl);
		return cmd_region(dl);
	} else if (dl_argv_match(dl, "health")) {
		dl_arg_inc(dl);
		return cmd_health(dl);
	} else if (dl_argv_match(dl, "trap")) {
		dl_arg_inc(dl);
		return cmd_trap(dl);
	}
	pr_err("Object \"%s\" not found\n", dl_argv(dl));
	return -ENOENT;
}

static int dl_init(struct dl *dl)
{
	int err;

	dl->nlg = mnlg_socket_open(DEVLINK_GENL_NAME, DEVLINK_GENL_VERSION);
	if (!dl->nlg) {
		pr_err("Failed to connect to devlink Netlink\n");
		return -errno;
	}

	err = ifname_map_init(dl);
	if (err) {
		pr_err("Failed to create index map\n");
		goto err_ifname_map_create;
	}
	new_json_obj_plain(dl->json_output);
	return 0;

err_ifname_map_create:
	mnlg_socket_close(dl->nlg);
	return err;
}

static void dl_fini(struct dl *dl)
{
	delete_json_obj_plain();
	ifname_map_fini(dl);
	mnlg_socket_close(dl->nlg);
}

static struct dl *dl_alloc(void)
{
	struct dl *dl;

	dl = calloc(1, sizeof(*dl));
	if (!dl)
		return NULL;
	return dl;
}

static void dl_free(struct dl *dl)
{
	free(dl);
}

static int dl_batch_cmd(int argc, char *argv[], void *data)
{
	struct dl *dl = data;

	return dl_cmd(dl, argc, argv);
}

static int dl_batch(struct dl *dl, const char *name, bool force)
{
	return do_batch(name, force, dl_batch_cmd, dl);
}

int main(int argc, char **argv)
{
	static const struct option long_options[] = {
		{ "Version",		no_argument,		NULL, 'V' },
		{ "force",		no_argument,		NULL, 'f' },
		{ "batch",		required_argument,	NULL, 'b' },
		{ "no-nice-names",	no_argument,		NULL, 'n' },
		{ "json",		no_argument,		NULL, 'j' },
		{ "pretty",		no_argument,		NULL, 'p' },
		{ "verbose",		no_argument,		NULL, 'v' },
		{ "statistics",		no_argument,		NULL, 's' },
		{ "Netns",		required_argument,	NULL, 'N' },
		{ NULL, 0, NULL, 0 }
	};
	const char *batch_file = NULL;
	bool force = false;
	struct dl *dl;
	int opt;
	int err;
	int ret;

	dl = dl_alloc();
	if (!dl) {
		pr_err("Failed to allocate memory for devlink\n");
		return EXIT_FAILURE;
	}

	while ((opt = getopt_long(argc, argv, "Vfb:njpvsN:",
				  long_options, NULL)) >= 0) {

		switch (opt) {
		case 'V':
			printf("devlink utility, iproute2-%s\n", version);
			ret = EXIT_SUCCESS;
			goto dl_free;
		case 'f':
			force = true;
			break;
		case 'b':
			batch_file = optarg;
			break;
		case 'n':
			dl->no_nice_names = true;
			break;
		case 'j':
			dl->json_output = true;
			break;
		case 'p':
			pretty = true;
			break;
		case 'v':
			dl->verbose = true;
			break;
		case 's':
			dl->stats = true;
			break;
		case 'N':
			if (netns_switch(optarg)) {
				ret = EXIT_FAILURE;
				goto dl_free;
			}
			break;
		default:
			pr_err("Unknown option.\n");
			help();
			ret = EXIT_FAILURE;
			goto dl_free;
		}
	}

	argc -= optind;
	argv += optind;

	err = dl_init(dl);
	if (err) {
		ret = EXIT_FAILURE;
		goto dl_free;
	}

	if (batch_file)
		err = dl_batch(dl, batch_file, force);
	else
		err = dl_cmd(dl, argc, argv);

	if (err) {
		ret = EXIT_FAILURE;
		goto dl_fini;
	}

	ret = EXIT_SUCCESS;

dl_fini:
	dl_fini(dl);
dl_free:
	dl_free(dl);

	return ret;
}
