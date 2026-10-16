/*
 * Author: Paul.Russell@rustcorp.com.au and mneuling@radlogic.com.au
 *
 * Based on the ipchains code by Paul Russell and Michael Neuling
 *
 * (C) 2000-2002 by the netfilter coreteam <coreteam@netfilter.org>:
 * 		    Paul 'Rusty' Russell <rusty@rustcorp.com.au>
 * 		    Marc Boucher <marc+nf@mbsi.ca>
 * 		    James Morris <jmorris@intercode.com.au>
 * 		    Harald Welte <laforge@gnumonks.org>
 * 		    Jozsef Kadlecsik <kadlec@blackhole.kfki.hu>
 *
 *	iptables -- IP firewall administration for kernels with
 *	firewall table (aimed for the 2.3 kernels)
 *
 *	See the accompanying manual page iptables(8) for information
 *	about proper usage of this program.
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

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <iptables.h>
#include "xtables-multi.h"
#include "nft.h"

static struct xtables_globals *xtables_globals_lookup(int family)
{
	switch (family) {
	case AF_INET:
	case AF_INET6:
		return &xtables_globals;
	case NFPROTO_ARP:
		return &arptables_globals;
	case NFPROTO_BRIDGE:
		return &ebtables_globals;
	default:
		xtables_error(OTHER_PROBLEM, "Unknown family value %d", family);
	}
}

static int
xtables_main(int family, const char *progname, int argc, char *argv[])
{
	char *table = "filter";
	struct nft_handle h;
	int ret;

	ret = xtables_init_all(xtables_globals_lookup(family), family);
	if (ret < 0) {
		fprintf(stderr, "%s: Failed to initialize xtables\n", progname);
		exit(1);
	}
	xt_params->program_name = progname;
	init_extensions();
	switch (family) {
	case NFPROTO_IPV4:
		init_extensions4();
		break;
	case NFPROTO_IPV6:
		init_extensions6();
		break;
	case NFPROTO_ARP:
		init_extensionsa();
		break;
	case NFPROTO_BRIDGE:
		init_extensionsb();
		break;
	}

	if (nft_init(&h, family) < 0) {
		fprintf(stderr, "%s: Failed to initialize nft: %s\n",
			xt_params->program_name, strerror(errno));
		exit(EXIT_FAILURE);
	}

	ret = do_commandx(&h, argc, argv, &table, false);
	if (ret)
		ret = nft_commit(&h);

	nft_fini(&h);
	xtables_fini();

	if (!ret) {
		fprintf(stderr, "%s: %s.%s\n", progname, nft_strerror(errno),
			(errno == EINVAL ?
			 " Run `dmesg' for more information." : ""));

		if (errno == EAGAIN)
			exit(RESOURCE_PROBLEM);
	}

	exit(!ret);
}

int xtables_ip4_main(int argc, char *argv[])
{
	return xtables_main(NFPROTO_IPV4, "iptables", argc, argv);
}

int xtables_ip6_main(int argc, char *argv[])
{
	return xtables_main(NFPROTO_IPV6, "ip6tables", argc, argv);
}

int xtables_arp_main(int argc, char *argv[])
{
	return xtables_main(NFPROTO_ARP, "arptables", argc, argv);
}
