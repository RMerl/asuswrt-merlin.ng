/*
 * src/nl-pktloc-lookup.c     Lookup packet location alias
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2010 Thomas Graf <tgraf@suug.ch>
 */

#include <netlink/cli/utils.h>
#include <netlink/route/pktloc.h>

static void print_usage(void)
{
printf(
"Usage: nl-pktloc-lookup [OPTIONS] <name>\n"
"\n"
"OPTIONS\n"
" -h, --help                Show this help text.\n"
" -v, --version             Show versioning information.\n"
" -l, --list                List all packet location definitions.\n"
"     --u32=VALUE	    Print in iproute2's u32 selector style\n"
"\n"
"\n"
"EXAMPLE\n"
"   $ nl-pktloc-lookup ip.dst\n"
"   $ nl-pktloc-lookup --list\n"
"\n"
);
	exit(0);
}

static const char *align_txt[] = {
	[TCF_EM_ALIGN_U8] = "u8",
	[TCF_EM_ALIGN_U16] = "u16",
	[TCF_EM_ALIGN_U32] = "u32"
};

static uint32_t align_mask[] = {
	[TCF_EM_ALIGN_U8] = 0xff,
	[TCF_EM_ALIGN_U16] = 0xffff,
	[TCF_EM_ALIGN_U32] = 0xffffffff,
};

static const char *layer_txt[] = {
	[TCF_LAYER_LINK] = "eth",
	[TCF_LAYER_NETWORK] = "ip",
	[TCF_LAYER_TRANSPORT] = "tcp"
};

static void dump_u32_style(struct rtnl_pktloc *loc, uint32_t value)
{
	if (loc->align > 4)
		nl_cli_fatal(EINVAL, "u32 only supports alignments u8|u16|u32.");

	if (loc->layer == TCF_LAYER_LINK)
		nl_cli_fatal(EINVAL, "u32 does not support link "
				"layer locations.");

	if (loc->shift > 0)
		nl_cli_fatal(EINVAL, "u32 does not support shifting.");

	printf("%s %x %x at %s%u\n",
		align_txt[loc->align],
		value, loc->mask ? loc->mask : align_mask[loc->align],
		loc->layer == TCF_LAYER_TRANSPORT ? "nexthdr+" : "",
		loc->offset);
}

static char *get_align_txt(struct rtnl_pktloc *loc)
{
	static char buf[16];

	if (loc->align <= 4)
		strcpy(buf, align_txt[loc->align]);
	else
		snprintf(buf, sizeof(buf), "%u", loc->align);

	return buf;
}

static void dump_loc(struct rtnl_pktloc *loc)
{
	printf("%s = %s at %s+%u & %#x >> %u\n",
		loc->name, get_align_txt(loc), layer_txt[loc->layer],
		loc->offset, loc->mask, loc->shift);
}

static void list_cb(struct rtnl_pktloc *loc, void *arg)
{
	printf("%-26s %-5s %3s+%-4u %#-10x %-8u %u\n",
		loc->name, get_align_txt(loc), layer_txt[loc->layer],
		loc->offset, loc->mask, loc->shift, loc->refcnt);
}

static void do_list(void)
{
	printf(
"name                      align  offset  mask     shift    refcnt\n");
	printf("---------------------------------------------------------\n");

	rtnl_pktloc_foreach(&list_cb, NULL);
}

int main(int argc, char *argv[])
{
	struct rtnl_pktloc *loc;
	int err, ustyle = 0;
	uint32_t uvalue = 0;

	for (;;) {
		int c, optidx = 0;
		enum {
			ARG_U32 = 257,
		};
		static struct option long_opts[] = {
			{ "help", 0, 0, 'h' },
			{ "version", 0, 0, 'v' },
			{ "list", 0, 0, 'l' },
			{ "u32", 1, 0, ARG_U32 },
			{ 0, 0, 0, 0 }
		};
	
		c = getopt_long(argc, argv, "hvl", long_opts, &optidx);
		if (c == -1)
			break;

		switch (c) {
		case 'h': print_usage(); break;
		case 'v': nl_cli_print_version(); break;
		case 'l': do_list(); exit(0);
		case ARG_U32:
			ustyle = 1;
			uvalue = nl_cli_parse_u32(optarg);
			break;
		}
 	}

	if (optind >= argc)
		print_usage();

	if ((err = rtnl_pktloc_lookup(argv[optind++], &loc)) < 0)
		nl_cli_fatal(err, "Unable to lookup packet location: %s",
			nl_geterror(err));

	if (ustyle)
		dump_u32_style(loc, uvalue);
	else
		dump_loc(loc);

	return 0;
}
