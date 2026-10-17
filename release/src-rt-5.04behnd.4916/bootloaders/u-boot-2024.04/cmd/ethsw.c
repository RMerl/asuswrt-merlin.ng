// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2015 Freescale Semiconductor, Inc.
 *
 * Ethernet Switch commands
 */

#include <common.h>
#include <command.h>
#include <env.h>
#include <errno.h>
#include <env_flags.h>
#include <ethsw.h>
#include <net.h>

static const char *ethsw_name;

#define ETHSW_PORT_STATS_HELP "ethsw [port <port_no>] statistics " \
"{ [help] | [clear] } - show an l2 switch port's statistics"

static int ethsw_port_stats_help_key_func(struct ethsw_command_def *parsed_cmd)
{
	printf(ETHSW_PORT_STATS_HELP"\n");

	return CMD_RET_SUCCESS;
}

#define ETHSW_LEARN_HELP "ethsw [port <port_no>] learning " \
"{ [help] | show | auto | disable } " \
"- enable/disable/show learning configuration on a port"

static int ethsw_learn_help_key_func(struct ethsw_command_def *parsed_cmd)
{
	printf(ETHSW_LEARN_HELP"\n");

	return CMD_RET_SUCCESS;
}

#define ETHSW_FDB_HELP "ethsw [port <port_no>] [vlan <vid>] fdb " \
"{ [help] | show | flush | { add | del } <mac> } " \
"- Add/delete a mac entry in FDB; use show to see FDB entries; " \
"if vlan <vid> is missing, VID 1 will be used"

static int ethsw_fdb_help_key_func(struct ethsw_command_def *parsed_cmd)
{
	printf(ETHSW_FDB_HELP"\n");

	return CMD_RET_SUCCESS;
}

#define ETHSW_PVID_HELP "ethsw [port <port_no>] " \
"pvid { [help] | show | <pvid> } " \
"- set/show PVID (ingress and egress VLAN tagging) for a port"

static int ethsw_pvid_help_key_func(struct ethsw_command_def *parsed_cmd)
{
	printf(ETHSW_PVID_HELP"\n");

	return CMD_RET_SUCCESS;
}

#define ETHSW_VLAN_HELP "ethsw [port <port_no>] vlan " \
"{ [help] | show | add <vid> | del <vid> } " \
"- add a VLAN to a port (VLAN members)"

static int ethsw_vlan_help_key_func(struct ethsw_command_def *parsed_cmd)
{
	printf(ETHSW_VLAN_HELP"\n");

	return CMD_RET_SUCCESS;
}

#define ETHSW_PORT_UNTAG_HELP "ethsw [port <port_no>] untagged " \
"{ [help] | show | all | none | pvid } " \
" - set egress tagging mode for a port"

static int ethsw_port_untag_help_key_func(struct ethsw_command_def *parsed_cmd)
{
	printf(ETHSW_PORT_UNTAG_HELP"\n");

	return CMD_RET_SUCCESS;
}

#define ETHSW_EGR_VLAN_TAG_HELP "ethsw [port <port_no>] egress tag " \
"{ [help] | show | pvid | classified } " \
"- Configure VID source for egress tag. " \
"Tag's VID could be the frame's classified VID or the PVID of the port"

static int ethsw_egr_tag_help_key_func(struct ethsw_command_def *parsed_cmd)
{
	printf(ETHSW_EGR_VLAN_TAG_HELP"\n");

	return CMD_RET_SUCCESS;
}

#define ETHSW_VLAN_FDB_HELP "ethsw vlan fdb " \
"{ [help] | show | shared | private } " \
"- make VLAN learning shared or private"

static int ethsw_vlan_learn_help_key_func(struct ethsw_command_def *parsed_cmd)
{
	printf(ETHSW_VLAN_FDB_HELP"\n");

	return CMD_RET_SUCCESS;
}

#define ETHSW_PORT_INGR_FLTR_HELP "ethsw [port <port_no>] ingress filtering" \
" { [help] | show | enable | disable } " \
"- enable/disable VLAN ingress filtering on port"

static int ethsw_ingr_fltr_help_key_func(struct ethsw_command_def *parsed_cmd)
{
	printf(ETHSW_PORT_INGR_FLTR_HELP"\n");

	return CMD_RET_SUCCESS;
}

#define ETHSW_PORT_AGGR_HELP "ethsw [port <port_no>] aggr" \
" { [help] | show | <lag_group_no> } " \
"- get/set LAG group for a port"

static int ethsw_port_aggr_help_key_func(struct ethsw_command_def *parsed_cmd)
{
	printf(ETHSW_PORT_AGGR_HELP"\n");

	return CMD_RET_SUCCESS;
}

static struct keywords_to_function {
	enum ethsw_keyword_id cmd_keyword[ETHSW_MAX_CMD_PARAMS];
	int cmd_func_offset;
	int (*keyword_function)(struct ethsw_command_def *parsed_cmd);
} ethsw_cmd_def[] = {
		{
			.cmd_keyword = {
					ethsw_id_enable,
					ethsw_id_key_end,
			},
			.cmd_func_offset = offsetof(struct ethsw_command_func,
						    port_enable),
			.keyword_function = NULL,
		}, {
			.cmd_keyword = {
					ethsw_id_disable,
					ethsw_id_key_end,
			},
			.cmd_func_offset = offsetof(struct ethsw_command_func,
						    port_disable),
			.keyword_function = NULL,
		}, {
			.cmd_keyword = {
					ethsw_id_show,
					ethsw_id_key_end,
			},
			.cmd_func_offset = offsetof(struct ethsw_command_func,
						    port_show),
			.keyword_function = NULL,
		}, {
			.cmd_keyword = {
					ethsw_id_statistics,
					ethsw_id_help,
					ethsw_id_key_end,
			},
			.cmd_func_offset = -1,
			.keyword_function = &ethsw_port_stats_help_key_func,
		}, {
			.cmd_keyword = {
					ethsw_id_statistics,
					ethsw_id_key_end,
			},
			.cmd_func_offset = offsetof(struct ethsw_command_func,
						    port_stats),
			.keyword_function = NULL,
		}, {
			.cmd_keyword = {
					ethsw_id_statistics,
					ethsw_id_clear,
					ethsw_id_key_end,
			},
			.cmd_func_offset = offsetof(struct ethsw_command_func,
						    port_stats_clear),
			.keyword_function = NULL,
		}, {
			.cmd_keyword = {
					ethsw_id_learning,
					ethsw_id_key_end,
			},
			.cmd_func_offset = -1,
			.keyword_function = &ethsw_learn_help_key_func,
		}, {
			.cmd_keyword = {
					ethsw_id_learning,
					ethsw_id_help,
					ethsw_id_key_end,
			},
			.cmd_func_offset = -1,
			.keyword_function = &ethsw_learn_help_key_func,
		}, {
			.cmd_keyword = {
					ethsw_id_learning,
					ethsw_id_show,
					ethsw_id_key_end,
			},
			.cmd_func_offset = offsetof(struct ethsw_command_func,
						    port_learn_show),
			.keyword_function = NULL,
		}, {
			.cmd_keyword = {
					ethsw_id_learning,
					ethsw_id_auto,
					ethsw_id_key_end,
			},
			.cmd_func_offset = offsetof(struct ethsw_command_func,
						    port_learn),
			.keyword_function = NULL,
		}, {
			.cmd_keyword = {
					ethsw_id_learning,
					ethsw_id_disable,
					ethsw_id_key_end,
			},
			.cmd_func_offset = offsetof(struct ethsw_command_func,
						    port_learn),
			.keyword_function = NULL,
		}, {
			.cmd_keyword = {
					ethsw_id_fdb,
					ethsw_id_key_end,
			},
			.cmd_func_offset = -1,
			.keyword_function = &ethsw_fdb_help_key_func,
		}, {
			.cmd_keyword = {
					ethsw_id_fdb,
					ethsw_id_help,
					ethsw_id_key_end,
			},
			.cmd_func_offset = -1,
			.keyword_function = &ethsw_fdb_help_key_func,
		}, {
			.cmd_keyword = {
					ethsw_id_fdb,
					ethsw_id_show,
					ethsw_id_key_end,
			},
			.cmd_func_offset = offsetof(struct ethsw_command_func,
						    fdb_show),
			.keyword_function = NULL,
		}, {
			.cmd_keyword = {
					ethsw_id_fdb,
					ethsw_id_flush,
					ethsw_id_key_end,
			},
			.cmd_func_offset = offsetof(struct ethsw_command_func,
						    fdb_flush),
			.keyword_function = NULL,
		}, {
			.cmd_keyword = {
					ethsw_id_fdb,
					ethsw_id_add,
					ethsw_id_add_del_mac,
					ethsw_id_key_end,
			},
			.cmd_func_offset = offsetof(struct ethsw_command_func,
						    fdb_entry_add),
			.keyword_function = NULL,
		}, {
			.cmd_keyword = {
					ethsw_id_fdb,
					ethsw_id_del,
					ethsw_id_add_del_mac,
					ethsw_id_key_end,
			},
			.cmd_func_offset = offsetof(struct ethsw_command_func,
						    fdb_entry_del),
			.keyword_function = NULL,
		}, {
			.cmd_keyword = {
					ethsw_id_pvid,
					ethsw_id_key_end,
			},
			.cmd_func_offset = -1,
			.keyword_function = &ethsw_pvid_help_key_func,
		}, {
			.cmd_keyword = {
					ethsw_id_pvid,
					ethsw_id_help,
					ethsw_id_key_end,
			},
			.cmd_func_offset = -1,
			.keyword_function = &ethsw_pvid_help_key_func,
		}, {
			.cmd_keyword = {
					ethsw_id_pvid,
					ethsw_id_show,
					ethsw_id_key_end,
			},
			.cmd_func_offset = offsetof(struct ethsw_command_func,
						    pvid_show),
			.keyword_function = NULL,
		}, {
			.cmd_keyword = {
					ethsw_id_pvid,
					ethsw_id_pvid_no,
					ethsw_id_key_end,
			},
			.cmd_func_offset = offsetof(struct ethsw_command_func,
						    pvid_set),
			.keyword_function = NULL,
		}, {
			.cmd_keyword = {
					ethsw_id_vlan,
					ethsw_id_key_end,
			},
			.cmd_func_offset = -1,
			.keyword_function = &ethsw_vlan_help_key_func,
		}, {
			.cmd_keyword = {
					ethsw_id_vlan,
					ethsw_id_help,
					ethsw_id_key_end,
			},
			.cmd_func_offset = -1,
			.keyword_function = &ethsw_vlan_help_key_func,
		}, {
			.cmd_keyword = {
					ethsw_id_vlan,
					ethsw_id_show,
					ethsw_id_key_end,
			},
			.cmd_func_offset = offsetof(struct ethsw_command_func,
						    vlan_show),
			.keyword_function = NULL,
		}, {
			.cmd_keyword = {
					ethsw_id_vlan,
					ethsw_id_add,
					ethsw_id_add_del_no,
					ethsw_id_key_end,
			},
			.cmd_func_offset = offsetof(struct ethsw_command_func,
						    vlan_set),
			.keyword_function = NULL,
		}, {
			.cmd_keyword = {
					ethsw_id_vlan,
					ethsw_id_del,
					ethsw_id_add_del_no,
					ethsw_id_key_end,
			},
			.cmd_func_offset = offsetof(struct ethsw_command_func,
						    vlan_set),
			.keyword_function = NULL,
		}, {
			.cmd_keyword = {
					ethsw_id_untagged,
					ethsw_id_key_end,
			},
			.cmd_func_offset = -1,
			.keyword_function = &ethsw_port_untag_help_key_func,
		}, {
			.cmd_keyword = {
					ethsw_id_untagged,
					ethsw_id_help,
					ethsw_id_key_end,
			},
			.cmd_func_offset = -1,
			.keyword_function = &ethsw_port_untag_help_key_func,
		}, {
			.cmd_keyword = {
					ethsw_id_untagged,
					ethsw_id_show,
					ethsw_id_key_end,
			},
			.cmd_func_offset = offsetof(struct ethsw_command_func,
						    port_untag_show),
			.keyword_function = NULL,
		}, {
			.cmd_keyword = {
					ethsw_id_untagged,
					ethsw_id_all,
					ethsw_id_key_end,
			},
			.cmd_func_offset = offsetof(struct ethsw_command_func,
						    port_untag_set),
			.keyword_function = NULL,
		}, {
			.cmd_keyword = {
					ethsw_id_untagged,
					ethsw_id_none,
					ethsw_id_key_end,
			},
			.cmd_func_offset = offsetof(struct ethsw_command_func,
						    port_untag_set),
			.keyword_function = NULL,
		}, {
			.cmd_keyword = {
					ethsw_id_untagged,
					ethsw_id_pvid,
					ethsw_id_key_end,
			},
			.cmd_func_offset = offsetof(struct ethsw_command_func,
						    port_untag_set),
			.keyword_function = NULL,
		}, {
			.cmd_keyword = {
					ethsw_id_egress,
					ethsw_id_tag,
					ethsw_id_key_end,
			},
			.cmd_func_offset = -1,
			.keyword_function = &ethsw_egr_tag_help_key_func,
		}, {
			.cmd_keyword = {
					ethsw_id_egress,
					ethsw_id_tag,
					ethsw_id_help,
					ethsw_id_key_end,
			},
			.cmd_func_offset = -1,
			.keyword_function = &ethsw_egr_tag_help_key_func,
		}, {
			.cmd_keyword = {
					ethsw_id_egress,
					ethsw_id_tag,
					ethsw_id_show,
					ethsw_id_key_end,
			},
			.cmd_func_offset = offsetof(struct ethsw_command_func,
						    port_egr_vlan_show),
			.keyword_function = NULL,
		}, {
			.cmd_keyword = {
					ethsw_id_egress,
					ethsw_id_tag,
					ethsw_id_pvid,
					ethsw_id_key_end,
			},
			.cmd_func_offset = offsetof(struct ethsw_command_func,
						    port_egr_vlan_set),
			.keyword_function = NULL,
		}, {
			.cmd_keyword = {
					ethsw_id_egress,
					ethsw_id_tag,
					ethsw_id_classified,
					ethsw_id_key_end,
			},
			.cmd_func_offset = offsetof(struct ethsw_command_func,
						    port_egr_vlan_set),
			.keyword_function = NULL,
		}, {
			.cmd_keyword = {
					ethsw_id_vlan,
					ethsw_id_fdb,
					ethsw_id_key_end,
			},
			.cmd_func_offset = -1,
			.keyword_function = &ethsw_vlan_learn_help_key_func,
		}, {
			.cmd_keyword = {
					ethsw_id_vlan,
					ethsw_id_fdb,
					ethsw_id_help,
					ethsw_id_key_end,
			},
			.cmd_func_offset = -1,
			.keyword_function = &ethsw_vlan_learn_help_key_func,
		}, {
			.cmd_keyword = {
					ethsw_id_vlan,
					ethsw_id_fdb,
					ethsw_id_show,
					ethsw_id_key_end,
			},
			.cmd_func_offset = offsetof(struct ethsw_command_func,
						    vlan_learn_show),
			.keyword_function = NULL,
		}, {
			.cmd_keyword = {
					ethsw_id_vlan,
					ethsw_id_fdb,
					ethsw_id_shared,
					ethsw_id_key_end,
			},
			.cmd_func_offset = offsetof(struct ethsw_command_func,
						    vlan_learn_set),
			.keyword_function = NULL,
		}, {
			.cmd_keyword = {
					ethsw_id_vlan,
					ethsw_id_fdb,
					ethsw_id_private,
					ethsw_id_key_end,
			},
			.cmd_func_offset = offsetof(struct ethsw_command_func,
						    vlan_learn_set),
			.keyword_function = NULL,
		}, {
			.cmd_keyword = {
					ethsw_id_ingress,
					ethsw_id_filtering,
					ethsw_id_key_end,
			},
			.cmd_func_offset = -1,
			.keyword_function = &ethsw_ingr_fltr_help_key_func,
		}, {
			.cmd_keyword = {
					ethsw_id_ingress,
					ethsw_id_filtering,
					ethsw_id_help,
					ethsw_id_key_end,
			},
			.cmd_func_offset = -1,
			.keyword_function = &ethsw_ingr_fltr_help_key_func,
		}, {
			.cmd_keyword = {
					ethsw_id_ingress,
					ethsw_id_filtering,
					ethsw_id_show,
					ethsw_id_key_end,
			},
			.cmd_func_offset = offsetof(struct ethsw_command_func,
						    port_ingr_filt_show),
			.keyword_function = NULL,
		}, {
			.cmd_keyword = {
					ethsw_id_ingress,
					ethsw_id_filtering,
					ethsw_id_enable,
					ethsw_id_key_end,
			},
			.cmd_func_offset = offsetof(struct ethsw_command_func,
						    port_ingr_filt_set),
			.keyword_function = NULL,
		}, {
			.cmd_keyword = {
					ethsw_id_ingress,
					ethsw_id_filtering,
					ethsw_id_disable,
					ethsw_id_key_end,
			},
			.cmd_func_offset = offsetof(struct ethsw_command_func,
						    port_ingr_filt_set),
			.keyword_function = NULL,
		}, {
			.cmd_keyword = {
					ethsw_id_aggr,
					ethsw_id_key_end,
			},
			.cmd_func_offset = -1,
			.keyword_function = &ethsw_port_aggr_help_key_func,
		}, {
			.cmd_keyword = {
					ethsw_id_aggr,
					ethsw_id_help,
					ethsw_id_key_end,
			},
			.cmd_func_offset = -1,
			.keyword_function = &ethsw_port_aggr_help_key_func,
		}, {
			.cmd_keyword = {
					ethsw_id_aggr,
					ethsw_id_show,
					ethsw_id_key_end,
			},
			.cmd_func_offset = offsetof(struct ethsw_command_func,
						    port_aggr_show),
			.keyword_function = NULL,
		}, {
			.cmd_keyword = {
					ethsw_id_aggr,
					ethsw_id_aggr_no,
					ethsw_id_key_end,
			},
			.cmd_func_offset = offsetof(struct ethsw_command_func,
						    port_aggr_set),
			.keyword_function = NULL,
		},
};

struct keywords_optional {
	int cmd_keyword[ETHSW_MAX_CMD_PARAMS];
} cmd_opt_def[] = {
		{
				.cmd_keyword = {
						ethsw_id_port,
						ethsw_id_port_no,
						ethsw_id_key_end,
				},
		}, {
				.cmd_keyword = {
						ethsw_id_vlan,
						ethsw_id_vlan_no,
						ethsw_id_key_end,
				},
		}, {
				.cmd_keyword = {
						ethsw_id_port,
						ethsw_id_port_no,
						ethsw_id_vlan,
						ethsw_id_vlan_no,
						ethsw_id_key_end,
				},
		},
};

static int keyword_match_gen(enum ethsw_keyword_id key_id, int argc, char
			     *const argv[], int *argc_nr,
			     struct ethsw_command_def *parsed_cmd);
static int keyword_match_port(enum ethsw_keyword_id key_id, int argc,
			      char *const argv[], int *argc_nr,
			      struct ethsw_command_def *parsed_cmd);
static int keyword_match_vlan(enum ethsw_keyword_id key_id, int argc,
			      char *const argv[], int *argc_nr,
			      struct ethsw_command_def *parsed_cmd);
static int keyword_match_pvid(enum ethsw_keyword_id key_id, int argc,
			      char *const argv[], int *argc_nr,
			      struct ethsw_command_def *parsed_cmd);
static int keyword_match_mac_addr(enum ethsw_keyword_id key_id, int argc,
				  char *const argv[], int *argc_nr,
				  struct ethsw_command_def *parsed_cmd);
static int keyword_match_aggr(enum ethsw_keyword_id key_id, int argc,
			      char *const argv[], int *argc_nr,
			      struct ethsw_command_def *parsed_cmd);

/*
 * Define properties for each keyword;
 * keep the order synced with enum ethsw_keyword_id
 */
struct keyword_def {
	const char *keyword_name;
	int (*match)(enum ethsw_keyword_id key_id, int argc, char *const argv[],
		     int *argc_nr, struct ethsw_command_def *parsed_cmd);
} keyword[] = {
		{
				.keyword_name = "help",
				.match = &keyword_match_gen,
		}, {
				.keyword_name = "show",
				.match = &keyword_match_gen,
		}, {
				.keyword_name = "port",
				.match = &keyword_match_port
		},  {
				.keyword_name = "enable",
				.match = &keyword_match_gen,
		}, {
				.keyword_name = "disable",
				.match = &keyword_match_gen,
		}, {
				.keyword_name = "statistics",
				.match = &keyword_match_gen,
		}, {
				.keyword_name = "clear",
				.match = &keyword_match_gen,
		}, {
				.keyword_name = "learning",
				.match = &keyword_match_gen,
		}, {
				.keyword_name = "auto",
				.match = &keyword_match_gen,
		}, {
				.keyword_name = "vlan",
				.match = &keyword_match_vlan,
		}, {
				.keyword_name = "fdb",
				.match = &keyword_match_gen,
		}, {
				.keyword_name = "add",
				.match = &keyword_match_mac_addr,
		}, {
				.keyword_name = "del",
				.match = &keyword_match_mac_addr,
		}, {
				.keyword_name = "flush",
				.match = &keyword_match_gen,
		}, {
				.keyword_name = "pvid",
				.match = &keyword_match_pvid,
		}, {
				.keyword_name = "untagged",
				.match = &keyword_match_gen,
		}, {
				.keyword_name = "all",
				.match = &keyword_match_gen,
		}, {
				.keyword_name = "none",
				.match = &keyword_match_gen,
		}, {
				.keyword_name = "egress",
				.match = &keyword_match_gen,
		}, {
				.keyword_name = "tag",
				.match = &keyword_match_gen,
		}, {
				.keyword_name = "classified",
				.match = &keyword_match_gen,
		}, {
				.keyword_name = "shared",
				.match = &keyword_match_gen,
		}, {
				.keyword_name = "private",
				.match = &keyword_match_gen,
		}, {
				.keyword_name = "ingress",
				.match = &keyword_match_gen,
		}, {
				.keyword_name = "filtering",
				.match = &keyword_match_gen,
		}, {
				.keyword_name = "aggr",
				.match = &keyword_match_aggr,
		},
};

/*
 * Function used by an Ethernet Switch driver to set the functions
 * that must be called by the parser when an ethsw command is given
 */
int ethsw_define_functions(const struct ethsw_command_func *cmd_func)
{
	int i;
	void **aux_p;
	int (*cmd_func_aux)(struct ethsw_command_def *);

	if (!cmd_func->ethsw_name)
		return -EINVAL;

	ethsw_name = cmd_func->ethsw_name;

	for (i = 0; i < ARRAY_SIZE(ethsw_cmd_def); i++) {
		/*
		 * get the pointer to the function send by the Ethernet Switch
		 * driver that corresponds to the proper ethsw command
		 */
		if (ethsw_cmd_def[i].keyword_function)
			continue;

		aux_p = (void *)cmd_func + ethsw_cmd_def[i].cmd_func_offset;

		cmd_func_aux = (int (*)(struct ethsw_command_def *)) *aux_p;
		ethsw_cmd_def[i].keyword_function = cmd_func_aux;
	}

	return 0;
}

/* Generic function used to match a keyword only by a string */
static int keyword_match_gen(enum ethsw_keyword_id key_id, int argc,
			     char *const argv[], int *argc_nr,
			     struct ethsw_command_def *parsed_cmd)
{
	if (strcmp(argv[*argc_nr], keyword[key_id].keyword_name) == 0) {
		parsed_cmd->cmd_to_keywords[*argc_nr] = key_id;

		return 1;
	}
	return 0;
}

/* Function used to match the command's port */
static int keyword_match_port(enum ethsw_keyword_id key_id, int argc,
			      char *const argv[], int *argc_nr,
			      struct ethsw_command_def *parsed_cmd)
{
	unsigned long val;

	if (!keyword_match_gen(key_id, argc, argv, argc_nr, parsed_cmd))
		return 0;

	if (*argc_nr + 1 >= argc)
		return 0;

	if (strict_strtoul(argv[*argc_nr + 1], 10, &val) != -EINVAL) {
		parsed_cmd->port = val;
		(*argc_nr)++;
		parsed_cmd->cmd_to_keywords[*argc_nr] = ethsw_id_port_no;
		return 1;
	}

	return 0;
}

/* Function used to match the command's vlan */
static int keyword_match_vlan(enum ethsw_keyword_id key_id, int argc,
			      char *const argv[], int *argc_nr,
			      struct ethsw_command_def *parsed_cmd)
{
	unsigned long val;
	int aux;

	if (!keyword_match_gen(key_id, argc, argv, argc_nr, parsed_cmd))
		return 0;

	if (*argc_nr + 1 >= argc)
		return 0;

	if (strict_strtoul(argv[*argc_nr + 1], 10, &val) != -EINVAL) {
		parsed_cmd->vid = val;
		(*argc_nr)++;
		parsed_cmd->cmd_to_keywords[*argc_nr] = ethsw_id_vlan_no;
		return 1;
	}

	aux = *argc_nr + 1;

	if (keyword_match_gen(ethsw_id_add, argc, argv, &aux, parsed_cmd))
		parsed_cmd->cmd_to_keywords[*argc_nr + 1] = ethsw_id_add;
	else if (keyword_match_gen(ethsw_id_del, argc, argv, &aux, parsed_cmd))
		parsed_cmd->cmd_to_keywords[*argc_nr + 1] = ethsw_id_del;
	else
		return 0;

	if (*argc_nr + 2 >= argc)
		return 0;

	if (strict_strtoul(argv[*argc_nr + 2], 10, &val) != -EINVAL) {
		parsed_cmd->vid = val;
		(*argc_nr) += 2;
		parsed_cmd->cmd_to_keywords[*argc_nr] = ethsw_id_add_del_no;
		return 1;
	}

	return 0;
}

/* Function used to match the command's pvid */
static int keyword_match_pvid(enum ethsw_keyword_id key_id, int argc,
			      char *const argv[], int *argc_nr,
			      struct ethsw_command_def *parsed_cmd)
{
	unsigned long val;

	if (!keyword_match_gen(key_id, argc, argv, argc_nr, parsed_cmd))
		return 0;

	if (*argc_nr + 1 >= argc)
		return 1;

	if (strict_strtoul(argv[*argc_nr + 1], 10, &val) != -EINVAL) {
		parsed_cmd->vid = val;
		(*argc_nr)++;
		parsed_cmd->cmd_to_keywords[*argc_nr] = ethsw_id_pvid_no;
	}

	return 1;
}

/* Function used to match the command's MAC address */
static int keyword_match_mac_addr(enum ethsw_keyword_id key_id, int argc,
				     char *const argv[], int *argc_nr,
				     struct ethsw_command_def *parsed_cmd)
{
	if (!keyword_match_gen(key_id, argc, argv, argc_nr, parsed_cmd))
		return 0;

	if ((*argc_nr + 1 >= argc) ||
	    !is_broadcast_ethaddr(parsed_cmd->ethaddr))
		return 1;

	if (eth_validate_ethaddr_str(argv[*argc_nr + 1])) {
		printf("Invalid MAC address: %s\n", argv[*argc_nr + 1]);
		return 0;
	}

	string_to_enetaddr(argv[*argc_nr + 1], parsed_cmd->ethaddr);

	if (is_broadcast_ethaddr(parsed_cmd->ethaddr)) {
		memset(parsed_cmd->ethaddr, 0xFF, sizeof(parsed_cmd->ethaddr));
		return 0;
	}

	parsed_cmd->cmd_to_keywords[*argc_nr + 1] = ethsw_id_add_del_mac;

	return 1;
}

/* Function used to match the command's aggregation number */
static int keyword_match_aggr(enum ethsw_keyword_id key_id, int argc,
			      char *const argv[], int *argc_nr,
			      struct ethsw_command_def *parsed_cmd)
{
	unsigned long val;

	if (!keyword_match_gen(key_id, argc, argv, argc_nr, parsed_cmd))
		return 0;

	if (*argc_nr + 1 >= argc)
		return 1;

	if (strict_strtoul(argv[*argc_nr + 1], 10, &val) != -EINVAL) {
		parsed_cmd->aggr_grp = val;
		(*argc_nr)++;
		parsed_cmd->cmd_to_keywords[*argc_nr] = ethsw_id_aggr_no;
	}

	return 1;
}

/* Finds optional keywords and modifies *argc_va to skip them */
static void cmd_keywords_opt_check(const struct ethsw_command_def *parsed_cmd,
				   int *argc_val)
{
	int i;
	int keyw_opt_matched;
	int argc_val_max;
	int const *cmd_keyw_p;
	int const *cmd_keyw_opt_p;

	/* remember the best match */
	argc_val_max = *argc_val;

	/*
	 * check if our command's optional keywords match the optional
	 * keywords of an available command
	 */
	for (i = 0; i < ARRAY_SIZE(cmd_opt_def); i++) {
		keyw_opt_matched = 0;
		cmd_keyw_p = &parsed_cmd->cmd_to_keywords[keyw_opt_matched];
		cmd_keyw_opt_p = &cmd_opt_def[i].cmd_keyword[keyw_opt_matched];

		/*
		 * increase the number of keywords that
		 * matched with a command
		 */
		while (keyw_opt_matched + *argc_val <
		       parsed_cmd->cmd_keywords_nr &&
		       *cmd_keyw_opt_p != ethsw_id_key_end &&
		       *(cmd_keyw_p + *argc_val) == *cmd_keyw_opt_p) {
			keyw_opt_matched++;
			cmd_keyw_p++;
			cmd_keyw_opt_p++;
		}

		/*
		 * if all our optional command's keywords perfectly match an
		 * optional pattern, then we can move to the next defined
		 * keywords in our command; remember the one that matched the
		 * greatest number of keywords
		 */
		if (keyw_opt_matched + *argc_val <=
		    parsed_cmd->cmd_keywords_nr &&
		    *cmd_keyw_opt_p == ethsw_id_key_end &&
		    *argc_val + keyw_opt_matched > argc_val_max)
			argc_val_max = *argc_val + keyw_opt_matched;
	}

	*argc_val = argc_val_max;
}

/*
 * Finds the function to call based on keywords and
 * modifies *argc_va to skip them
 */
static void cmd_keywords_check(struct ethsw_command_def *parsed_cmd,
			       int *argc_val)
{
	int i;
	int keyw_matched;
	int *cmd_keyw_p;
	int *cmd_keyw_def_p;

	/*
	 * check if our command's keywords match the
	 * keywords of an available command
	 */
	for (i = 0; i < ARRAY_SIZE(ethsw_cmd_def); i++) {
		keyw_matched = 0;
		cmd_keyw_p = &parsed_cmd->cmd_to_keywords[keyw_matched];
		cmd_keyw_def_p = &ethsw_cmd_def[i].cmd_keyword[keyw_matched];

		/*
		 * increase the number of keywords that
		 * matched with a command
		 */
		while (keyw_matched + *argc_val < parsed_cmd->cmd_keywords_nr &&
		       *cmd_keyw_def_p != ethsw_id_key_end &&
		       *(cmd_keyw_p + *argc_val) == *cmd_keyw_def_p) {
			keyw_matched++;
			cmd_keyw_p++;
			cmd_keyw_def_p++;
		}

		/*
		 * if all our command's keywords perfectly match an
		 * available command, then we get the function we need to call
		 * to configure the Ethernet Switch
		 */
		if (keyw_matched && keyw_matched + *argc_val ==
		    parsed_cmd->cmd_keywords_nr &&
		    *cmd_keyw_def_p == ethsw_id_key_end) {
			*argc_val += keyw_matched;
			parsed_cmd->cmd_function =
					ethsw_cmd_def[i].keyword_function;
			return;
		}
	}
}

/* find all the keywords in the command */
static int keywords_find(int argc, char *const argv[],
			 struct ethsw_command_def *parsed_cmd)
{
	int i;
	int j;
	int argc_val;
	int rc = CMD_RET_SUCCESS;

	for (i = 1; i < argc; i++) {
		for (j = 0; j < ethsw_id_count; j++) {
			if (keyword[j].match(j, argc, argv, &i, parsed_cmd))
				break;
		}
	}

	/* if there is no keyword match for a word, the command is invalid */
	for (i = 1; i < argc; i++)
		if (parsed_cmd->cmd_to_keywords[i] == ethsw_id_key_end)
			rc = CMD_RET_USAGE;

	parsed_cmd->cmd_keywords_nr = argc;
	argc_val = 1;

	/* get optional parameters first */
	cmd_keywords_opt_check(parsed_cmd, &argc_val);

	if (argc_val == parsed_cmd->cmd_keywords_nr)
		return CMD_RET_USAGE;

	/*
	 * check the keywords and if a match is found,
	 * get the function to call
	 */
	cmd_keywords_check(parsed_cmd, &argc_val);

	/* error if not all commands' parameters were matched */
	if (argc_val == parsed_cmd->cmd_keywords_nr) {
		if (!parsed_cmd->cmd_function) {
			printf("Command not available for: %s\n", ethsw_name);
			rc = CMD_RET_FAILURE;
		}
	} else {
		rc = CMD_RET_USAGE;
	}

	return rc;
}

static void command_def_init(struct ethsw_command_def *parsed_cmd)
{
	int i;

	for (i = 0; i < ETHSW_MAX_CMD_PARAMS; i++)
		parsed_cmd->cmd_to_keywords[i] = ethsw_id_key_end;

	parsed_cmd->port = ETHSW_CMD_PORT_ALL;
	parsed_cmd->vid = ETHSW_CMD_VLAN_ALL;
	parsed_cmd->aggr_grp = ETHSW_CMD_AGGR_GRP_NONE;
	parsed_cmd->cmd_function = NULL;

	/* We initialize the MAC address with the Broadcast address */
	memset(parsed_cmd->ethaddr, 0xff, sizeof(parsed_cmd->ethaddr));
}

/* function to interpret commands starting with "ethsw " */
static int do_ethsw(struct cmd_tbl *cmdtp, int flag, int argc,
		    char *const argv[])
{
	struct ethsw_command_def parsed_cmd;
	int rc = CMD_RET_SUCCESS;

	if (argc == 1 || argc >= ETHSW_MAX_CMD_PARAMS)
		return CMD_RET_USAGE;

	command_def_init(&parsed_cmd);

	rc = keywords_find(argc, argv, &parsed_cmd);

	if (rc == CMD_RET_SUCCESS)
		rc = parsed_cmd.cmd_function(&parsed_cmd);

	return rc;
}

#define ETHSW_PORT_CONF_HELP "[port <port_no>] { enable | disable | show } " \
"- enable/disable a port; show a port's configuration"

U_BOOT_CMD(ethsw, ETHSW_MAX_CMD_PARAMS, 0, do_ethsw,
	   "Ethernet l2 switch commands",
	   ETHSW_PORT_CONF_HELP"\n"
	   ETHSW_PORT_STATS_HELP"\n"
	   ETHSW_LEARN_HELP"\n"
	   ETHSW_FDB_HELP"\n"
	   ETHSW_PVID_HELP"\n"
	   ETHSW_VLAN_HELP"\n"
	   ETHSW_PORT_UNTAG_HELP"\n"
	   ETHSW_EGR_VLAN_TAG_HELP"\n"
	   ETHSW_VLAN_FDB_HELP"\n"
	   ETHSW_PORT_INGR_FLTR_HELP"\n"
	   ETHSW_PORT_AGGR_HELP"\n"
);
