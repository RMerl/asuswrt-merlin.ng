#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "../include/ebtables_u.h"

static struct option opts[] =
{
	{ 0 }
};

static void print_help()
{
	printf(
	"skiplog target takes no options:\n");
}

static void init(struct ebt_entry_target *target)
{
}

static int parse(int c, char **argv, int argc,
   const struct ebt_u_entry *entry, unsigned int *flags,
   struct ebt_entry_target **target)
{
	return 1;
}

static void final_check(const struct ebt_u_entry *entry,
   const struct ebt_entry_target *target, const char *name,
   unsigned int hookmask, unsigned int time)
{
}

static void print(const struct ebt_u_entry *entry,
   const struct ebt_entry_target *target)
{
}

static int compare(const struct ebt_entry_target *t1,
   const struct ebt_entry_target *t2)
{
	return 1;
}

static struct ebt_u_target skiplog_target =
{
    "SKIPLOG",
    0,
    print_help,
    init,
    parse,
    final_check,
    print,
    compare,
    opts
};

__attribute__((constructor)) static void extension_init(void)
{
	ebt_register_target(&skiplog_target);
}
