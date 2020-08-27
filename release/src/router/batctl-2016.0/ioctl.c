/*
 * Copyright (C) 2012-2016  B.A.T.M.A.N. contributors:
 *
 * Marek Lindner <mareklindner@neomailbox.ch>
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
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/sockios.h>
#include <linux/ethtool.h>
#include <stdint.h>

#include "ioctl.h"

/* code borrowed from ethtool */
static int statistics_custom_get(int fd, struct ifreq *ifr)
{
	struct ethtool_drvinfo drvinfo;
	struct ethtool_gstrings *strings = NULL;
	struct ethtool_stats *stats = NULL;
	unsigned int n_stats, sz_str, sz_stats, i;
	int err, ret = EXIT_FAILURE;

	drvinfo.cmd = ETHTOOL_GDRVINFO;
	ifr->ifr_data = (void *)&drvinfo;
	err = ioctl(fd, SIOCETHTOOL, ifr);
	if (err < 0) {
		fprintf(stderr, "Error - can't open driver information: %s\n", strerror(errno));
		goto out;
	}

	n_stats = drvinfo.n_stats;
	if (n_stats < 1)
		goto success;

	sz_str = n_stats * ETH_GSTRING_LEN;
	sz_stats = n_stats * sizeof(uint64_t);

	strings = calloc(1, sz_str + sizeof(struct ethtool_gstrings));
	stats = calloc(1, sz_stats + sizeof(struct ethtool_stats));
	if (!strings || !stats) {
		fprintf(stderr, "Error - out of memory\n");
		goto out;
	}

	strings->cmd = ETHTOOL_GSTRINGS;
	strings->string_set = ETH_SS_STATS;
	strings->len = n_stats;
	ifr->ifr_data = (void *)strings;
	err = ioctl(fd, SIOCETHTOOL, ifr);
	if (err < 0) {
		fprintf(stderr, "Error - can't get stats strings information: %s\n", strerror(errno));
		goto out;
	}

	stats->cmd = ETHTOOL_GSTATS;
	stats->n_stats = n_stats;
	ifr->ifr_data = (void *) stats;
	err = ioctl(fd, SIOCETHTOOL, ifr);
	if (err < 0) {
		fprintf(stderr, "Error - can't get stats information: %s\n", strerror(errno));
		goto out;
	}

	for (i = 0; i < n_stats; i++) {
		printf("\t%.*s: %llu\n", ETH_GSTRING_LEN,
		       &strings->data[i * ETH_GSTRING_LEN], stats->data[i]);
	}

success:
	ret = EXIT_SUCCESS;

out:
	free(strings);
	free(stats);
	return ret;
}

int ioctl_statistics_get(char *mesh_iface)
{
	struct ifreq ifr;
	int fd = -1, ret = EXIT_FAILURE;

	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, mesh_iface, sizeof(ifr.ifr_name));
	ifr.ifr_name[sizeof(ifr.ifr_name) - 1] = '\0';

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0) {
		fprintf(stderr, "Error - can't open socket: %s\n", strerror(errno));
		goto out;
	}

	ret = statistics_custom_get(fd, &ifr);

out:
	if (fd >= 0)
		close(fd);
	return ret;
}
