/* Shared library add-on to iptables to add blog match support. */
#include <stdbool.h>
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <netinet/in.h>
#include <xtables.h>
#include <linux/netfilter/xt_blog.h>

static void blog_match_help(void)
{
	printf(
"blog match options:\n"
"[!] --tcp-pureack              match when blog TCP Pure ACK is\n"
"                               detected.\n");
}

static const struct option blog_match_opts[] = {
	{.name = "tcp-pureack",      .has_arg = false, .val = '1'},
	XT_GETOPT_TABLEEND,
};

static int
blog_match_parse(int c, char **argv, int invert, unsigned int *flags,
          const void *entry, struct xt_entry_match **match)
{
	struct xt_blog *bloginfo = (struct xt_blog *)(*match)->data;

	switch (c) {
	case '1':
		bloginfo->tcp_pure_ack = 1;
		bloginfo->invert = invert;
		break;
	}

	return 1;
}

static void
blog_match_print(const void *ip, const struct xt_entry_match *match, int numeric)
{
	const struct xt_blog *bloginfo = (struct xt_blog *)match->data;

	printf(" blog");
	if (bloginfo->tcp_pure_ack)
		printf(" TCP Pure ACK");
	if (bloginfo->invert)
		printf(" not detected");
	else
		printf(" detected");
}

static void
blog_match_save(const void *ip, const struct xt_entry_match *match)
{
	const struct xt_blog *bloginfo = (struct xt_blog *)match->data;

	printf(" blog");
	if (bloginfo->tcp_pure_ack) {
		if (bloginfo->invert)
			printf(" !");
		printf(" --tcp-pureack ");
	}
}

static struct xtables_match blog_match = {
	.family		= NFPROTO_UNSPEC,
	.name		= "blog",
	.version	= XTABLES_VERSION,
	.size		= XT_ALIGN(sizeof(struct xt_blog)),
	.userspacesize	= XT_ALIGN(sizeof(struct xt_blog)),
	.help		= blog_match_help,
	.parse		= blog_match_parse,
	.print		= blog_match_print,
	.save		= blog_match_save,
	.extra_opts	= blog_match_opts,
};

void
_init(void)
{
	xtables_register_match(&blog_match);
}
