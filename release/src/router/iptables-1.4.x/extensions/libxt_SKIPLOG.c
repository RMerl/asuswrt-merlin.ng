/* Shared library add-on to iptables to add SKIPLOG target support. */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>

#include <xtables.h>
#include <linux/netfilter/x_tables.h>

/* Function which prints out usage message. */
static void SKIPLOG_help(void)
{
	printf(
"SKIPLOG target v%s takes no options\n",
XTABLES_VERSION);
}

/* Function which parses command options; returns true if it
   ate an option */
static int
SKIPLOG_parse(int c, char **argv, int invert, unsigned int *flags,
              const void *entry, struct xt_entry_target **target)
{
	return 0;
}

static struct xtables_target skiplog_target = {
	.family		= AF_INET,
	.name		= "SKIPLOG",
	.version	= XTABLES_VERSION,
	.size		= XT_ALIGN(0),
	.userspacesize	= XT_ALIGN(0),
	.help		= SKIPLOG_help,
	.parse		= SKIPLOG_parse,
};

static struct xtables_target skiplog_target6 = {
	.family		= AF_INET6,
	.name		= "SKIPLOG",
	.version	= XTABLES_VERSION,
	.size		= XT_ALIGN(0),
	.userspacesize	= XT_ALIGN(0),
	.help		= SKIPLOG_help,
	.parse		= SKIPLOG_parse,
};

void _init(void)
{
	xtables_register_target(&skiplog_target);
	xtables_register_target(&skiplog_target6);
}
