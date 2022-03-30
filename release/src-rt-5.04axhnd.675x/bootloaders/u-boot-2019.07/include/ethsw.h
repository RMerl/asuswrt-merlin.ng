/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2015 Freescale Semiconductor, Inc.
 *
 * Ethernet Switch commands
 */

#ifndef _CMD_ETHSW_H_
#define _CMD_ETHSW_H_

#define ETHSW_MAX_CMD_PARAMS 20
#define ETHSW_CMD_PORT_ALL -1
#define ETHSW_CMD_VLAN_ALL -1
#define ETHSW_CMD_AGGR_GRP_NONE -1

/* IDs used to track keywords in a command */
enum ethsw_keyword_id {
	ethsw_id_key_end = -1,
	ethsw_id_help,
	ethsw_id_show,
	ethsw_id_port,
	ethsw_id_enable,
	ethsw_id_disable,
	ethsw_id_statistics,
	ethsw_id_clear,
	ethsw_id_learning,
	ethsw_id_auto,
	ethsw_id_vlan,
	ethsw_id_fdb,
	ethsw_id_add,
	ethsw_id_del,
	ethsw_id_flush,
	ethsw_id_pvid,
	ethsw_id_untagged,
	ethsw_id_all,
	ethsw_id_none,
	ethsw_id_egress,
	ethsw_id_tag,
	ethsw_id_classified,
	ethsw_id_shared,
	ethsw_id_private,
	ethsw_id_ingress,
	ethsw_id_filtering,
	ethsw_id_aggr,
	ethsw_id_count,	/* keep last */
};

enum ethsw_keyword_opt_id {
	ethsw_id_port_no = ethsw_id_count + 1,
	ethsw_id_vlan_no,
	ethsw_id_pvid_no,
	ethsw_id_add_del_no,
	ethsw_id_add_del_mac,
	ethsw_id_aggr_no,
	ethsw_id_count_all,	/* keep last */
};

struct ethsw_command_def {
	int cmd_to_keywords[ETHSW_MAX_CMD_PARAMS];
	int cmd_keywords_nr;
	int port;
	int vid;
	int aggr_grp;
	uchar ethaddr[6];
	int (*cmd_function)(struct ethsw_command_def *parsed_cmd);
};

/* Structure to be created and initialized by an Ethernet Switch driver */
struct ethsw_command_func {
	const char *ethsw_name;
	int (*port_enable)(struct ethsw_command_def *parsed_cmd);
	int (*port_disable)(struct ethsw_command_def *parsed_cmd);
	int (*port_show)(struct ethsw_command_def *parsed_cmd);
	int (*port_stats)(struct ethsw_command_def *parsed_cmd);
	int (*port_stats_clear)(struct ethsw_command_def *parsed_cmd);
	int (*port_learn)(struct ethsw_command_def *parsed_cmd);
	int (*port_learn_show)(struct ethsw_command_def *parsed_cmd);
	int (*fdb_show)(struct ethsw_command_def *parsed_cmd);
	int (*fdb_flush)(struct ethsw_command_def *parsed_cmd);
	int (*fdb_entry_add)(struct ethsw_command_def *parsed_cmd);
	int (*fdb_entry_del)(struct ethsw_command_def *parsed_cmd);
	int (*pvid_show)(struct ethsw_command_def *parsed_cmd);
	int (*pvid_set)(struct ethsw_command_def *parsed_cmd);
	int (*vlan_show)(struct ethsw_command_def *parsed_cmd);
	int (*vlan_set)(struct ethsw_command_def *parsed_cmd);
	int (*port_untag_show)(struct ethsw_command_def *parsed_cmd);
	int (*port_untag_set)(struct ethsw_command_def *parsed_cmd);
	int (*port_egr_vlan_show)(struct ethsw_command_def *parsed_cmd);
	int (*port_egr_vlan_set)(struct ethsw_command_def *parsed_cmd);
	int (*vlan_learn_show)(struct ethsw_command_def *parsed_cmd);
	int (*vlan_learn_set)(struct ethsw_command_def *parsed_cmd);
	int (*port_ingr_filt_show)(struct ethsw_command_def *parsed_cmd);
	int (*port_ingr_filt_set)(struct ethsw_command_def *parsed_cmd);
	int (*port_aggr_show)(struct ethsw_command_def *parsed_cmd);
	int (*port_aggr_set)(struct ethsw_command_def *parsed_cmd);
};

int ethsw_define_functions(const struct ethsw_command_func *cmd_func);

#endif /* _CMD_ETHSW_H_ */
