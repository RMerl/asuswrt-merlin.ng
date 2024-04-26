/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 */

/*
 * VPN utility library for Asuswrt-Merlin
 * Provides some of the functions found in Asuswrt's
 * proprietary libvpn, either re-implemented, or
 * implemented as wrappers around AM's functions.
 * Also includes additional functions developed
 * for Asuswrt-Merlin's OpenVPN support.
 */

#include <stddef.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <shared.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/if_ether.h>
#include <sys/ioctl.h>
#include <net/if.h>


// Check whether any network interface uses the same "addr". Return 0 if no interface use "addr", otherwise return 1.
int current_addr(in_addr_t addr) {
	int sock;
	struct ifreq ifr;
	struct sockaddr_in *sin;
	char buffer[1024];

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0) {
		perror("socket");
		return -1;
	}

	// Get list of interfaces
	struct ifconf ifc;
	ifc.ifc_len = sizeof(buffer);
	ifc.ifc_buf = buffer;
	if (ioctl(sock, SIOCGIFCONF, &ifc) < 0) {
		perror("ioctl SIOCGIFCONF");
		close(sock);
		return -1;
	}

	struct ifreq* ifr_list = ifc.ifc_req;
	int num_interfaces = ifc.ifc_len / sizeof(struct ifreq);
	for (int i = 0; i < num_interfaces; i++) {
		strncpy(ifr.ifr_name, ifr_list[i].ifr_name, IFNAMSIZ);

		// Get the IP address of the interface
		if (ioctl(sock, SIOCGIFADDR, &ifr) < 0) {
			perror("ioctl SIOCGIFADDR");
			continue;
		}

		sin = (struct sockaddr_in *)&ifr.ifr_addr;

		if (sin->sin_addr.s_addr == addr) {
			close(sock);
			return 1;
		}
	}

	close(sock);
	return 0;
}


// Check whether any routing rule of a network in main table is a subnet of network/netmask. Return 0 if no such routing rule, otherwise return 1.
int current_route(in_addr_t network, in_addr_t netmask) {
	FILE* fp;
	char line[256];
	unsigned long dest, gw, mask;
	int flags, refcnt, use, metric, mtu, window, irtt;

	fp = fopen("/proc/net/route", "r");
	if (fp == NULL)
		return -1;

	// Skip the header line
	if (fgets(line, 256, fp) == NULL) {
		fclose(fp);
		return -1;
	}

	while (fgets(line, 256, fp) != NULL) {
		if (sscanf(line, "%*s %lx %lx %x %d %d %d %lx %d %d %d\n",
		   &dest, &gw, &flags, &refcnt, &use,
		   &metric, &mask, &mtu, &window, &irtt) == 10) {
			// Check if the destination is a subnet of the given network/netmask
			if ((dest & netmask) == (network & netmask)) {
				fclose(fp);
				return 1;
			}
		}
	}

	fclose(fp);

	return 0;
}

// Get the network interface of default route in main table. Return iface if it is found, otherwise return NULL.
char* get_default_gateway_dev(char *iface, size_t len) {
	FILE* fp;
	char line[256];
	char dev_iface[16];
	unsigned long dest, gw, mask;
	int flags, refcnt, use, metric, mtu, window, irtt;

	fp = fopen("/proc/net/route", "r");
	if (fp == NULL)
		return NULL;

	// Skip the header line
	if (fgets(line, 256, fp) == NULL) {
		fclose(fp);
		return NULL;
	}

	while (fgets(line, 256, fp) != NULL) {
		if (sscanf(line, "%15s %lx %lx %x %d %d %d %lx %d %d %d\n",
		   dev_iface, &dest, &gw, &flags, &refcnt, &use,
		   &metric, &mask, &mtu, &window, &irtt) == 11) {
		// Check if it's the default route
			if (dest == 0 && mask == 0 && (flags & 1)) {
				strlcpy(iface, dev_iface, len);
				break;
			}
		}
	}

	// Close the file
	fclose(fp);

	return iface;
}
