/* SPDX-License-Identifier: GPL-2.0 */
#include <stdio.h>
#include <stdlib.h>

#include "utils.h"
#include "ip_common.h"

static void ifb_print_help(struct link_util *lu,
			    int argc, char **argv, FILE *f)
{
	fprintf(f, "Usage: ... ifb\n");
}

struct link_util ifb_link_util = {
	.id		= "ifb",
	.print_help	= ifb_print_help,
};
