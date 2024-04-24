/*
 * Copyright (C) 2019-2023 Tobias Brunner
 *
 * Copyright (C) secunet Security Networks AG
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <errno.h>
#include <net/if.h>

#include "kernel_netlink_xfrmi.h"

/**
 * Default MTU
 */
#define XFRMI_DEFAULT_MTU 1400

/**
 * Manager for XFRM interfaces
 */
static kernel_netlink_xfrmi_t *manager;

/**
 * Destroy the allocated manager
 */
static void destroy_manager()
{
	if (manager)
	{
		kernel_netlink_xfrmi_destroy(manager);
	}
}

/**
 * List all installed XFRM interfaces
 */
static void list_xfrm_interfaces(kernel_netlink_xfrmi_t *manager)
{
	enumerator_t *enumerator;
	char *name, *dev;
	uint32_t xfrm_id, mtu;

	enumerator = manager->create_enumerator(manager);
	while (enumerator->enumerate(enumerator, &name, &xfrm_id, &dev, &mtu))
	{
		printf("%2u: %-16s dev %-12s if_id 0x%.8x [%10u] mtu %u\n",
			   if_nametoindex(name), name, dev ?: "-", xfrm_id, xfrm_id, mtu);
	}
	enumerator->destroy(enumerator);
}

static void usage(FILE *out, char *name)
{
	fprintf(out, "Create XFRM interfaces\n\n");
	fprintf(out, "%s [OPTIONS]\n\n", name);
	fprintf(out, "Options:\n");
	fprintf(out, "  -h, --help          print this help.\n");
	fprintf(out, "  -v, --debug         set debug level, default: 1.\n");
	fprintf(out, "  -l, --list          list XFRM interfaces.\n");
	fprintf(out, "  -n, --name=NAME     name of the XFRM interface.\n");
	fprintf(out, "  -i, --id=ID         optional numeric XFRM ID.\n");
	fprintf(out, "  -d, --dev=DEVICE    optional underlying physical interface.\n");
	fprintf(out, "  -m, --mtu=MTU       optional MTU, default: 1400 (use 0 for kernel default).\n");
	fprintf(out, "\n");
}

int main(int argc, char *argv[])
{
	char *name = NULL, *dev = NULL, *end;
	uint32_t xfrm_id = 0, mtu = XFRMI_DEFAULT_MTU;

	library_init(NULL, "xfrmi");
	atexit(library_deinit);

	manager = kernel_netlink_xfrmi_create(FALSE);
	atexit(destroy_manager);

	while (true)
	{
		struct option long_opts[] = {
			{"help",		no_argument,		NULL,	'h' },
			{"debug",		no_argument,		NULL,	'v' },
			{"list",		no_argument,		NULL,	'l' },
			{"name",		required_argument,	NULL,	'n' },
			{"id",			required_argument,	NULL,	'i' },
			{"dev",			required_argument,	NULL,	'd' },
			{"mtu",			required_argument,	NULL,	'm' },
			{0,0,0,0 },
		};
		switch (getopt_long(argc, argv, "hvln:i:d:m:", long_opts, NULL))
		{
			case EOF:
				break;
			case 'h':
				usage(stdout, argv[0]);
				return 0;
			case 'l':
				list_xfrm_interfaces(manager);
				return 0;
			case 'v':
				dbg_default_set_level(atoi(optarg));
				continue;
			case 'n':
				name = optarg;
				continue;
			case 'i':
				errno = 0;
				xfrm_id = strtoul(optarg, &end, 0);
				if (errno || *end)
				{
					fprintf(stderr, "invalid XFRM ID: %s\n",
							errno ? strerror(errno) : end);
					return 1;
				}
				continue;
			case 'd':
				dev = optarg;
				continue;
			case 'm':
				errno = 0;
				mtu = strtoul(optarg, &end, 0);
				if (errno || *end)
				{
					fprintf(stderr, "invalid MTU: %s\n",
							errno ? strerror(errno) : end);
					return 1;
				}
				continue;
			default:
				usage(stderr, argv[0]);
				return 1;
		}
		break;
	}
	if (!name)
	{
		fprintf(stderr, "please specify a name\n");
		return 1;
	}
	return !manager->create(manager, name, xfrm_id, dev, mtu);
}
