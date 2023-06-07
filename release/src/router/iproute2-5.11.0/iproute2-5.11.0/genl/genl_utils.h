/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _TC_UTIL_H_
#define _TC_UTIL_H_ 1

#include <linux/genetlink.h>
#include "utils.h"

struct genl_util {
	struct  genl_util *next;
	char	name[16];
	int	(*parse_genlopt)(struct genl_util *fu, int argc, char **argv);
	int	(*print_genlopt)(struct nlmsghdr *n, void *arg);
};

#endif
