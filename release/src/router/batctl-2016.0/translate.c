/*
 * Copyright (C) 2007-2016  B.A.T.M.A.N. contributors:
 *
 * Andreas Langer <an.langer@gmx.de>, Marek Lindner <mareklindner@neomailbox.ch>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include "main.h"
#include "translate.h"
#include "functions.h"
#include "bat-hosts.h"


static void translate_usage(void)
{
	fprintf(stderr, "Usage: batctl [options] translate mac|bat-host|host_name|IPv4_address\n");
}

int translate(char *mesh_iface, int argc, char **argv)
{
	struct ether_addr *dst_mac = NULL;
	struct bat_host *bat_host;
	int ret = EXIT_FAILURE;
	char *dst_string, *mac_string;

	if (argc <= 1) {
		fprintf(stderr, "Error - destination not specified\n");
		translate_usage();
		return EXIT_FAILURE;
	}

	dst_string = argv[1];
	bat_hosts_init(0);
	bat_host = bat_hosts_find_by_name(dst_string);

	if (bat_host)
		dst_mac = &bat_host->mac_addr;

	if (!dst_mac) {
		dst_mac = resolve_mac(dst_string);

		if (!dst_mac) {
			fprintf(stderr, "Error - mac address of the ping destination could not be resolved and is not a bat-host name: %s\n", dst_string);
			goto out;
		}
	}

	dst_mac = translate_mac(mesh_iface, dst_mac);
	if (dst_mac) {
		mac_string = ether_ntoa_long(dst_mac);
		printf("%s\n", mac_string);
		ret = EXIT_SUCCESS;
	} else {
		ret = EXIT_NOSUCCESS;
	}

out:
	bat_hosts_free();
	return ret;
}
