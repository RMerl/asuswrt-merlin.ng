/* SPDX-License-Identifier: GPL-2.0 */
#include <stdio.h>
#include <stdlib.h>

#include "utils.h"
#include "ip_common.h"

static void team_print_help(struct link_util *lu,
			    int argc, char **argv, FILE *f)
{
	fprintf(f, "Usage: ... team\n");
}

static void team_slave_print_help(struct link_util *lu,
				  int argc, char **argv, FILE *f)
{
	fprintf(f, "Usage: ... team_slave\n");
}

struct link_util team_link_util = {
	.id		= "team",
	.print_help	= team_print_help,
}, team_slave_link_util = {
	.id		= "team_slave",
	.print_help	= team_slave_print_help,
};
