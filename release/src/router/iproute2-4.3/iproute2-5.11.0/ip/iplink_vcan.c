/* SPDX-License-Identifier: GPL-2.0 */
#include <stdio.h>
#include <stdlib.h>

#include "utils.h"
#include "ip_common.h"

static void vcan_print_help(struct link_util *lu,
			    int argc, char **argv, FILE *f)
{
	fprintf(f, "Usage: ... vcan\n");
}

struct link_util vcan_link_util = {
	.id		= "vcan",
	.print_help	= vcan_print_help,
};
