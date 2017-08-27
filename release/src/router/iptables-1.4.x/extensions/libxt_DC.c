/*Shared library add-on to iptables to add DC target support. */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>

#include <xtables.h>
#include <linux/netfilter/x_tables.h>

/*Function which prints out usage message*/
static void DC_help(void)
{
    printf("DC target v%s takes no options\n", XTABLES_VERSION);
}

static int DC_parse(int c, char **argv, int invert, unsigned int *flags,
                                    const void *entry, struct xt_entry_target **target)
{
    return 0;
}

static struct xtables_target dc_target = 
{
    .family     = AF_INET,
    .name      = "DC",
    .version  = XTABLES_VERSION,
    .size          = XT_ALIGN(0),
    .userspacesize = XT_ALIGN(0),
    .help         = DC_help,
    .parse      = DC_parse,
};

void  _init(void)
{
    xtables_register_target(&dc_target);
}
