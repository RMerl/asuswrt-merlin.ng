/* Shared library add-on to ebtables to add blog match support */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "../include/ebtables_u.h"
#include <include/uapi/linux/netfilter_bridge/ebt_blog.h>

#define TCP_PUREACK 0x01

static struct option opts[] =
{
	{ "tcp-pureack"	       , no_argument, 0, TCP_PUREACK },
	{ 0 }
};

static void print_help()
{
	printf(
"blog match options:\n"
"    --tcp-pureack		 match when blog TCP Pure ACK is\n"
"				 detected.\n");
}

static void init(struct ebt_entry_match *match)
{
	struct ebt_blog_info *bloginfo =
	   (struct ebt_blog_info *)match->data;

	bloginfo->tcp_pure_ack = 0;
	bloginfo->invert = 0;
}

static int parse(int c, char **argv, int argc, const struct ebt_u_entry *entry,
   unsigned int *flags, struct ebt_entry_match **match)
{
	struct ebt_blog_info *bloginfo = (struct ebt_blog_info *)(*match)->data;

	switch (c) {
	case TCP_PUREACK:
		ebt_check_option2(flags, TCP_PUREACK);
		bloginfo->tcp_pure_ack = 1;
		if (ebt_check_inverse(optarg))
		{
			bloginfo->invert = 1;
		}
		break;
	default:
		return 0;
	}

	return 1;
}

static void final_check(const struct ebt_u_entry *entry,
   const struct ebt_entry_match *match, const char *name,
   unsigned int hookmask, unsigned int time)
{
}

static void print(const struct ebt_u_entry *entry,
   const struct ebt_entry_match *match)
{
	const struct ebt_blog_info *bloginfo = (struct ebt_blog_info *)match->data;

	printf(" blog match");
	if (bloginfo->tcp_pure_ack)
	{
		printf(" TCP Pure ACK");
		if (bloginfo->invert)
			printf(" not");
		printf(" set");
	}
	printf(" ");
}

static int compare(const struct ebt_entry_match *m1,
   const struct ebt_entry_match *m2)
{
	struct ebt_blog_info *b1 = (struct ebt_blog_info *)m1->data;
	struct ebt_blog_info *b2 = (struct ebt_blog_info *)m2->data;

	if (b1->invert != b2->invert || b1->tcp_pure_ack != b2->tcp_pure_ack)
		return 0;
	return 1;
}

static struct ebt_u_match blog_match =
{
	.name		= "blog",
	.size		= sizeof(struct ebt_blog_info),
	.help		= print_help,
	.init		= init,
	.parse		= parse,
	.final_check	= final_check,
	.print		= print,
	.compare	= compare,
	.extra_ops	= opts,
};

static void _INIT(void)
{
	ebt_register_match(&blog_match);
}
