/* Code to take an arptables-style command line and do it. */

/*
 * arptables:
 * Author: Bart De Schuymer <bdschuym@pandora.be>, but
 * almost all code is from the iptables userspace program, which has main
 * authors: Paul.Russell@rustcorp.com.au and mneuling@radlogic.com.au
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/*
  Currently, only support for specifying hardware addresses for Ethernet
  is available.
  This tool is not luser-proof: you can specify an Ethernet source address
  and set hardware length to something different than 6, f.e.
*/
#include "config.h"
#include <getopt.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <xtables.h>

#include "xshared.h"

#include "nft.h"

static struct option original_opts[] = {
	{ "append", 1, 0, 'A' },
	{ "delete", 1, 0,  'D' },
	{ "check", 1, 0,  'C'},
	{ "insert", 1, 0,  'I' },
	{ "replace", 1, 0,  'R' },
	{ "list", 2, 0,  'L' },
	{ "list-rules", 2, 0,  'S'},
	{ "flush", 2, 0,  'F' },
	{ "zero", 2, 0,  'Z' },
	{ "new-chain", 1, 0,  'N' },
	{ "delete-chain", 2, 0,  'X' },
	{ "rename-chain", 1, 0,  'E' },
	{ "policy", 1, 0,  'P' },
	{ "source-ip", 1, 0, 's' },
	{ "destination-ip", 1, 0,  'd' },
	{ "src-ip", 1, 0,  's' },
	{ "dst-ip", 1, 0,  'd' },
	{ "source-mac", 1, 0, 2},
	{ "destination-mac", 1, 0, 3},
	{ "src-mac", 1, 0, 2},
	{ "dst-mac", 1, 0, 3},
	{ "h-length", 1, 0,  'l' },
	{ "p-length", 1, 0,  8 },
	{ "opcode", 1, 0,  4 },
	{ "h-type", 1, 0,  5 },
	{ "proto-type", 1, 0,  6 },
	{ "in-interface", 1, 0, 'i' },
	{ "jump", 1, 0, 'j' },
	{ "table", 1, 0, 't' },
	{ "match", 1, 0, 'm' },
	{ "numeric", 0, 0, 'n' },
	{ "out-interface", 1, 0, 'o' },
	{ "verbose", 0, 0, 'v' },
	{ "exact", 0, 0, 'x' },
	{ "version", 0, 0, 'V' },
	{ "help", 2, 0, 'h' },
	{ "line-numbers", 0, 0, '0' },
	{ "modprobe", 1, 0, 'M' },
	{ "set-counters", 1, 0, 'c' },
	{ 0 }
};

#define opts xt_params->opts

struct xtables_globals arptables_globals = {
	.option_offset		= 0,
	.program_version	= PACKAGE_VERSION " (nf_tables)",
	.orig_opts		= original_opts,
	.compat_rev		= nft_compatible_revision,
};

int nft_init_arp(struct nft_handle *h, const char *pname)
{
	arptables_globals.program_name = pname;
	if (xtables_init_all(&arptables_globals, NFPROTO_ARP) < 0) {
		fprintf(stderr, "%s/%s Failed to initialize arptables-compat\n",
			arptables_globals.program_name,
			arptables_globals.program_version);
		exit(1);
	}
	init_extensions();
	init_extensionsa();

	if (nft_init(h, NFPROTO_ARP) < 0)
		xtables_error(OTHER_PROBLEM,
			      "Could not initialize nftables layer.");

	return 0;
}
