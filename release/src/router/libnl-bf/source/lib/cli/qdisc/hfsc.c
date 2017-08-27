/*
 * lib/cli/qdisc/hfsc.c     	HFSC module for CLI lib
 */
/*
 **************************************************************************
 * Copyright (c) 2015, The Linux Foundation. All rights reserved.
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 **************************************************************************
 */

#include <netlink/cli/utils.h>
#include <netlink/cli/tc.h>
#include <netlink/route/qdisc/hfsc.h>

static void print_qdisc_usage(void)
{
	printf(
"Usage: nl-qdisc-add [...] hfsc [OPTIONS]...\n"
"\n"
"OPTIONS\n"
"     --help                Show this help text.\n"
"     --default=ID          Default class for unclassified traffic.\n"
"\n"
"EXAMPLE"
"    # Create hfsc root qdisc 1: and direct unclassified traffic to class 1:3\n"
"    nl-qdisc-add --dev=eth1 --parent=root --handle=1: hfsc --default=3\n");
}

static void hfsc_parse_qdisc_argv(struct rtnl_tc *tc, int argc, char **argv)
{
	struct rtnl_qdisc *qdisc = (struct rtnl_qdisc *) tc;

	for (;;) {
		int c, optidx = 0;
		enum {
			ARG_DEFAULT = 257,
		};
		static struct option long_opts[] = {
			{ "help", 0, 0, 'h' },
			{ "default", 1, 0, ARG_DEFAULT },
			{ 0, 0, 0, 0 }
		};
	
		c = getopt_long(argc, argv, "hv", long_opts, &optidx);
		if (c == -1)
			break;

		switch (c) {
		case 'h':
			print_qdisc_usage();
			return;
		case ARG_DEFAULT:
			rtnl_hfsc_set_defcls(qdisc, nl_cli_parse_u32(optarg));
			break;
		}
 	}
}

#if 0
static void print_class_usage(void)
{
	printf(
"Usage: nl-class-add [...] hfsc [OPTIONS]...\n"
"\n"
"OPTIONS\n"
"     Usage: ... hfsc [ [ --rt \"SC\" ] [ --ls \"SC\" ] | [--sc \"SC\" ] ] [--ul\"SC\" ]\n"
"     --help                Show this help text.\n"
"     --realtime=\"SC\"     Realtime service curve.  Cannot be used with sc.\n"
"     --linkshare=\"SC\"    Link sharing service curve.  Cannot be used with ls\n"
"     --servicecurve=\"SC\" Combined service curve; Cannot be used with rt or ls.\n"
"     --upperlimit=\"SC\"   Upper limit service curve.\n"
"       Slope Format\n"
"        SC := [ [ m1 BPS ] [ d SEC ] m2 BPS\n"
"         m1 : slope of first segment\n"
"         d  : x-coordinate of intersection\n"
"         m2 : slope of second segment\n"
"       Alternative Format:\n"
"        SC := [ [ umax BYTE ] dmax SEC ] rate BPS\n"
"         umax : maximum unit of work\n"
"         dmax : maximum Delay\n"
"         rate : rate\n"
"\n"
"     Note: Service curve definitions must be in quotes on the command line.\n"
"     Note: Must provide at least one service curve.\n"
"     Note: If Upper limit is provided, linkshare or combined service curve must be given.\n"
"EXAMPLE"
"    # Attach class 1:1 to hfsc qdisc 1: and rate limit it to 20mbit\n"
"    nl-class-add --dev=eth1 --parent=1: --classid=1:1 hfsc --sc=\"rate 20mbit\"\n");
}


static int hfsc_get_sc_slope(char *optarg, struct service_curve *sc_out)
{
	return 0;
}

static int hfsc_get_sc_alt(char *optarg, struct service_curve *sc_out)
{
	return 0;
}

static int hfsc_get_sc(char *optarg, struct service_curve *sc_out)
{
	memset(sc_out, 0, sizeof(*sc_out));
	
	if ((hfsc_get_sc_slope(optarg, sc) < 0) &&
	    (hfsc_get_sc_alt(optarg, sc) < 0))
		return -EINVAL;

	if ((sc->m1 == 0) && (sc->m2 == 0)) {
		return -ENOENT;
	}
	
}

static void hfsc_parse_class_argv(struct rtnl_tc *tc, int argc, char **argv)
{
	struct rtnl_class *class = (struct rtnl_class *) tc;
	struct tc_service_curve sc;
	long rate;

	for (;;) {
		int c, optidx = 0;
		enum {
			ARG_RT = 257,
			ARG_LS = 258,
			ARG_SC = 259,
			ARG_UL = 260,
		};
		static struct option long_opts[] = {
			{ "help", 0, 0, 'h' },
			{ "realtime", 1, 0, ARG_RT },
			{ "linkshare", 1, 0, ARG_LS },
			{ "servicecurve", 1, 0, ARG_SC },
			{ "upperlimit", 1, 0, ARG_UL },
			{ 0, 0, 0, 0 }
		};
	
		c = getopt_long(argc, argv, "h", long_opts, &optidx);
		if (c == -1)
			break;

		switch (c) {
		case 'h':
			print_class_usage();
			return;

		case ARG_RT:
			rate = nl_size2int(optarg);
			if (rate < 0) {
				nl_cli_fatal(rate, "Unable to parse htb rate "
					"\"%s\": Invalid format.", optarg);
			}

			rtnl_htb_set_rate(class, rate);
			break;

		case ARG_LS:
			rate = nl_size2int(optarg);
			if (rate < 0) {
				nl_cli_fatal(rate, "Unable to parse htb ceil rate "
					"\"%s\": Invalid format.", optarg);
			}

			rtnl_htb_set_ceil(class, rate);
			break;

		case ARG_SC:
			rtnl_htb_set_prio(class, nl_cli_parse_u32(optarg));
			break;

		case ARG_UL:
			rate = nl_size2int(optarg);
			if (rate < 0) {
				nl_cli_fatal(rate, "Unable to parse quantum "
					"\"%s\": Invalid format.", optarg);
			}

			rtnl_htb_set_quantum(class, rate);
			break;
 	}
}

#endif

static struct nl_cli_tc_module hfsc_qdisc_module =
{
	.tm_name		= "hfsc",
	.tm_type		= RTNL_TC_TYPE_QDISC,
	.tm_parse_argv		= hfsc_parse_qdisc_argv,
};

#if 0
static struct nl_cli_tc_module hfsc_class_module =
{
	.tm_name		= "hfsc",
	.tm_type		= RTNL_TC_TYPE_CLASS,
	.tm_parse_argv		= hfsc_parse_class_argv,
};
#endif

static void __init hfsc_init(void)
{
	nl_cli_tc_register(&hfsc_qdisc_module);
//	nl_cli_tc_register(&hfsc_class_module);
}

static void __exit hfsc_exit(void)
{
//	nl_cli_tc_unregister(&hfsc_class_module);
	nl_cli_tc_unregister(&hfsc_qdisc_module);
}
