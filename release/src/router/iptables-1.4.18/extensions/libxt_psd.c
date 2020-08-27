/*
  Shared library add-on to iptables to add PSD support

  Copyright (C) 2000,2001 astaro AG

  This file is distributed under the terms of the GNU General Public
  License (GPL). Copies of the GPL can be obtained from:
     ftp://prep.ai.mit.edu/pub/gnu/GPL

  2000-05-04 Markus Hennig <hennig@astaro.de> : initial
  2000-08-18 Dennis Koslowski <koslowski@astaro.de> : first release
  2000-12-01 Dennis Koslowski <koslowski@astaro.de> : UDP scans detection added
  2001-02-04 Jan Rekorajski <baggins@pld.org.pl> : converted from target to match
  2003-03-02 Harald Welte <laforge@netfilter.org>: fix 'storage' bug
  2008-04-03 Mohd Nawawi <nawawi@tracenetworkcorporation.com>: update to 2.6.24 / 1.4 code
  2008-06-24 Mohd Nawawi <nawawi@tracenetworkcorporation.com>: update to 2.6.24 / 1.4.1 code
  2009-08-07 Mohd Nawawi Mohamad Jamili <nawawi@tracenetworkcorporation.com> : ported to xtables-addons
*/

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <syslog.h>
#include <getopt.h>
#include <xtables.h>
#include <linux/netfilter/x_tables.h>
#include <linux/netfilter/xt_psd.h>
//#include "compat_user.h"

#define SCAN_DELAY_THRESHOLD		300

/* Function which prints out usage message. */
static void psd_mt_help(void) {
	printf(
		"psd match options:\n"
		" --psd-weight-threshold threshhold  Portscan detection weight threshold\n"
		" --psd-delay-threshold  delay       Portscan detection delay threshold\n"
		" --psd-lo-ports-weight  lo          Privileged ports weight\n"
		" --psd-hi-ports-weight  hi          High ports weight\n\n");
}

static const struct option psd_mt_opts[] = {
	{.name = "psd-weight-threshold", .has_arg = true, .val = '1'},
	{.name = "psd-delay-threshold", .has_arg = true, .val = '2'},
	{.name = "psd-lo-ports-weight", .has_arg = true, .val = '3'},
	{.name = "psd-hi-ports-weight", .has_arg = true, .val = '4'},
	{NULL}
};

/* Initialize the target. */
static void psd_mt_init(struct xt_entry_match *match) {
	struct xt_psd_info *psdinfo = (struct xt_psd_info *)match->data;
	psdinfo->weight_threshold = SCAN_WEIGHT_THRESHOLD;
	psdinfo->delay_threshold = SCAN_DELAY_THRESHOLD;
	psdinfo->lo_ports_weight = PORT_WEIGHT_PRIV;
	psdinfo->hi_ports_weight = PORT_WEIGHT_HIGH;
}

#define XT_PSD_OPT_CTRESH 0x01
#define XT_PSD_OPT_DTRESH 0x02
#define XT_PSD_OPT_LPWEIGHT 0x04
#define XT_PSD_OPT_HPWEIGHT 0x08

static int psd_mt_parse(int c, char **argv, int invert, unsigned int *flags,
                     const void *entry, struct xt_entry_match **match)
{
	struct xt_psd_info *psdinfo = (struct xt_psd_info *)(*match)->data;
	unsigned int num;

	switch (c) {
		/* PSD-weight-threshold */
		case '1':
			if (*flags & XT_PSD_OPT_CTRESH)
				xtables_error(PARAMETER_PROBLEM,"Can't specify --psd-weight-threshold twice");
			if (!xtables_strtoui(optarg, NULL, &num, 0, PSD_MAX_RATE))
				xtables_error(PARAMETER_PROBLEM, "bad --psd-weight-threshold '%s'", optarg);
			psdinfo->weight_threshold = num;
			*flags |= XT_PSD_OPT_CTRESH;
			return true;

		/* PSD-delay-threshold */
		case '2':
			if (*flags & XT_PSD_OPT_DTRESH)
				xtables_error(PARAMETER_PROBLEM, "Can't specify --psd-delay-threshold twice");
			if (!xtables_strtoui(optarg, NULL, &num, 0, PSD_MAX_RATE))
				xtables_error(PARAMETER_PROBLEM, "bad --psd-delay-threshold '%s'", optarg);
			psdinfo->delay_threshold = num;
			*flags |= XT_PSD_OPT_DTRESH;
			return true;

		/* PSD-lo-ports-weight */
		case '3':
			if (*flags & XT_PSD_OPT_LPWEIGHT)
				xtables_error(PARAMETER_PROBLEM, "Can't specify --psd-lo-ports-weight twice");
			if (!xtables_strtoui(optarg, NULL, &num, 0, PSD_MAX_RATE))
				xtables_error(PARAMETER_PROBLEM, "bad --psd-lo-ports-weight '%s'", optarg);
			psdinfo->lo_ports_weight = num;
			*flags |= XT_PSD_OPT_LPWEIGHT;
			return true;

		/* PSD-hi-ports-weight */
		case '4':
			if (*flags & XT_PSD_OPT_HPWEIGHT)
				xtables_error(PARAMETER_PROBLEM, "Can't specify --psd-hi-ports-weight twice");
			if (!xtables_strtoui(optarg, NULL, &num, 0, PSD_MAX_RATE))
				xtables_error(PARAMETER_PROBLEM, "bad --psd-hi-ports-weight '%s'", optarg);
			psdinfo->hi_ports_weight = num;
			*flags |= XT_PSD_OPT_HPWEIGHT;
			return true;
	}
	return false;
}

/* Final check; nothing. */
static void psd_mt_final_check(unsigned int flags) {}

static void psd_mt_save(const void *ip, const struct xt_entry_match *match)
{
	const struct xt_psd_info *psdinfo = (const struct xt_psd_info *)match->data;
	printf(" --psd-weight-threshold %u ", psdinfo->weight_threshold);
	printf("--psd-delay-threshold %u ", psdinfo->delay_threshold);
	printf("--psd-lo-ports-weight %u ", psdinfo->lo_ports_weight);
	printf("--psd-hi-ports-weight %u ", psdinfo->hi_ports_weight);
}

static void psd_mt_print(const void *ip, const struct xt_entry_match *match, int numeric)
{
	printf(" -m psd");
	psd_mt_save(ip, match);
}

static struct xtables_match psd_mt_reg = {
	.name           = "psd",
	.version        = XTABLES_VERSION,
	.revision       = 1,
	.family         = NFPROTO_UNSPEC,
	.size           = XT_ALIGN(sizeof(struct xt_psd_info)),
	.userspacesize	= XT_ALIGN(sizeof(struct xt_psd_info)),
	.help           = psd_mt_help,
	.init           = psd_mt_init,
	.parse          = psd_mt_parse,
	.final_check    = psd_mt_final_check,
	.print          = psd_mt_print,
	.save           = psd_mt_save,
	.extra_opts     = psd_mt_opts,
};

#if 1
void _init(void)
{
	xtables_register_match(&psd_mt_reg);
}
#else
static __attribute__((constructor)) void psd_mt_ldr(void)
{
	xtables_register_match(&psd_mt_reg);
}
#endif
