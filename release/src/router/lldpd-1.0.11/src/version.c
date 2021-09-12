/* -*- mode: c; c-file-style: "openbsd" -*- */
/*
 * Copyright (c) 2016 Vincent Bernat <bernat@luffy.cx>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#if HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include "compat/compat.h"

static void
version_display_array(FILE *destination, const char *prefix, const char *const *items)
{
	fprintf(destination, "%s", prefix);
	size_t count = 0;
	for (const char *const *p = items; *p; p++, count++)
		fprintf(destination, "%s%s", count?", ":"", *p);
	if (count == 0)
		fprintf(destination, "(none)\n");
	else
		fprintf(destination, "\n");
}

void
version_display(FILE *destination, const char *progname, int verbose)
{
	if (!verbose) {
		fprintf(destination, "%s\n", PACKAGE_VERSION);
		return;
	}

	const char *const lldp_features[] = {
#ifdef ENABLE_LLDPMED
		"LLDP-MED",
#endif
#ifdef ENABLE_DOT1
		"Dot1",
#endif
#ifdef ENABLE_DOT3
		"Dot3",
#endif
#ifdef ENABLE_CUSTOM
		"Custom TLV",
#endif
		NULL};
	const char *const protocols[] = {
#ifdef ENABLE_CDP
		"CDP",
#endif
#ifdef ENABLE_FDP
		"FDP",
#endif
#ifdef ENABLE_EDP
		"EDP",
#endif
#ifdef ENABLE_SONMP
		"SONMP",
#endif
		NULL};
	const char *const output_formats[] = {
		"TEXT",
		"KV",
		"JSON",
#ifdef USE_XML
		"XML",
#endif
		NULL};


	fprintf(destination, "%s %s\n", progname, PACKAGE_VERSION);
	fprintf(destination, "  Built on " BUILD_DATE "\n");
	fprintf(destination, "\n");

	/* Features */
	if (!strcmp(progname, "lldpd")) {
		version_display_array(destination,
		    "Additional LLDP features:    ", lldp_features);
		version_display_array(destination,
		    "Additional protocols:        ", protocols);
		fprintf(destination,
		    "SNMP support:                "
#ifdef USE_SNMP
		    "yes\n"
#else
		    "no\n"
#endif
			);
#ifdef HOST_OS_LINUX
		fprintf(destination,
		    "Old kernel support:          "
#ifdef ENABLE_OLDIES
		    "yes"
#else
		    "no"
#endif
		    " (Linux " MIN_LINUX_KERNEL_VERSION "+)\n");
#endif
#ifdef ENABLE_PRIVSEP
		fprintf(destination,
		    "Privilege separation:        " "enabled\n");
		fprintf(destination,
		    "Privilege separation user:   " PRIVSEP_USER "\n");
		fprintf(destination,
		    "Privilege separation group:  " PRIVSEP_GROUP "\n");
		fprintf(destination,
		    "Privilege separation chroot: " PRIVSEP_CHROOT "\n");
#else
		fprintf(destination,
		    "Privilege separation:        " "disabled\n");
#endif
		fprintf(destination,
		    "Configuration directory:     " SYSCONFDIR "\n");
	}

	if (!strcmp(progname, "lldpcli")) {
		version_display_array(destination,
		    "Additional output formats:   ", output_formats);
	}

	fprintf(destination, "\n");

	/* Build */
	fprintf(destination,
	    "C compiler command: %s\n", LLDP_CC);
	fprintf(destination,
	    "Linker command:     %s\n", LLDP_LD);

}
