/*
 * Copyright (C) 2000 Lennert Buytenhek
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <libbridge.h>
#include <libbridge_private.h>

int br_socket_fd = -1;

int br_init(void)
{
	if ((br_socket_fd = socket(AF_LOCAL, SOCK_STREAM, 0)) < 0)
		return errno;
	return 0;
}

void br_shutdown(void)
{
	close(br_socket_fd);
	br_socket_fd = -1;
}

/* If /sys/class/net/XXX/bridge exists then it must be a bridge */
static int isbridge(const struct dirent *entry)
{
	char path[SYSFS_PATH_MAX];
	struct stat st;
	int ret, saved_errno;

	if (entry->d_name[0] == '.'
	    && (entry->d_name[1] == '\0'
		|| (entry->d_name[1] == '.'
		    && entry->d_name[2] == '\0')))
		return 0;

	snprintf(path, SYSFS_PATH_MAX, SYSFS_CLASS_NET "%s/bridge", entry->d_name);

	/* Workaround old glibc breakage.
	   If errno is set, then it fails scandir! */
	saved_errno = errno;
	ret = (stat(path, &st) == 0 && S_ISDIR(st.st_mode));
	errno = saved_errno;

	return ret;
}

/*
 * New interface uses sysfs to find bridges
 */
static int new_foreach_bridge(int (*iterator)(const char *name, void *), void *arg)
{
	struct dirent **namelist;
	int i, count = 0;

	count = scandir(SYSFS_CLASS_NET, &namelist, isbridge, alphasort);
	if (count < 0)
		return -1;

	for (i = 0; i < count; i++) {
		if (iterator(namelist[i]->d_name, arg))
			break;
	}

	for (i = 0; i < count; i++)
		free(namelist[i]);
	free(namelist);

	return count;
}

/*
 * Old interface uses ioctl
 */
static int old_foreach_bridge(int (*iterator)(const char *, void *),
			      void *iarg)
{
	int i, ret=0, num;
	char ifname[IFNAMSIZ];
	int ifindices[MAX_BRIDGES];
	unsigned long args[3] = { BRCTL_GET_BRIDGES, (unsigned long)ifindices, MAX_BRIDGES };

	num = ioctl(br_socket_fd, SIOCGIFBR, args);
	if (num < 0) {
		dprintf("Get bridge indices failed: %s\n", strerror(errno));
		return -errno;
	}

	for (i = 0; i < num; i++) {
		if (!if_indextoname(ifindices[i], ifname)) {
			dprintf("get find name for ifindex %d\n", ifindices[i]);
			return -errno;
		}

		++ret;
		if(iterator(ifname, iarg))
			break;

	}

	return ret;

}

/*
 * Go over all bridges and call iterator function.
 * if iterator returns non-zero then stop.
 */
int br_foreach_bridge(int (*iterator)(const char *, void *),
		     void *arg)
{
	int ret;

	ret = new_foreach_bridge(iterator, arg);
	if (ret <= 0)
		ret = old_foreach_bridge(iterator, arg);

	return ret;
}

/*
 * Only used if sysfs is not available.
 */
static int old_foreach_port(const char *brname,
			    int (*iterator)(const char *br, const char *port,
					    void *arg),
			    void *arg)
{
	int i, err, count;
	struct ifreq ifr;
	char ifname[IFNAMSIZ];
	int ifindices[MAX_PORTS];
	unsigned long args[4] = { BRCTL_GET_PORT_LIST, (unsigned long)ifindices, MAX_PORTS, 0 };

	memset(ifindices, 0, sizeof(ifindices));
	strncpy(ifr.ifr_name, brname, IFNAMSIZ);
	ifr.ifr_data = (char *) &args;

	err = ioctl(br_socket_fd, SIOCDEVPRIVATE, &ifr);
	if (err < 0) {
		dprintf("list ports for bridge:'%s' failed: %s\n", brname, strerror(errno));
		return -errno;
	}

	count = 0;
	for (i = 0; i < MAX_PORTS; i++) {
		if (!ifindices[i])
			continue;

		if (!if_indextoname(ifindices[i], ifname)) {
			dprintf("can't find name for ifindex:%d\n", ifindices[i]);
			continue;
		}

		++count;
		if (iterator(brname, ifname, arg))
			break;
	}

	return count;
}

/*
 * Iterate over all ports in bridge (using sysfs).
 */
int br_foreach_port(const char *brname,
		    int (*iterator)(const char *br, const char *port, void *arg),
		    void *arg)
{
	int i, count;
	struct dirent **namelist;
	char path[SYSFS_PATH_MAX];

	snprintf(path, SYSFS_PATH_MAX, SYSFS_CLASS_NET "%s/brif", brname);
	count = scandir(path, &namelist, 0, alphasort);
	if (count < 0)
		return old_foreach_port(brname, iterator, arg);

	for (i = 0; i < count; i++) {
		if (namelist[i]->d_name[0] == '.'
		    && (namelist[i]->d_name[1] == '\0'
			|| (namelist[i]->d_name[1] == '.'
			    && namelist[i]->d_name[2] == '\0')))
			continue;

		if (iterator(brname, namelist[i]->d_name, arg))
			break;
	}
	for (i = 0; i < count; i++)
		free(namelist[i]);
	free(namelist);

	return count;
}

void br_dump_bridge_id(const unsigned char *x)
{
	printf("%.2x%.2x.%.2x%.2x%.2x%.2x%.2x%.2x", x[0], x[1], x[2], x[3],
	       x[4], x[5], x[6], x[7]);
}

void br_show_timer(const struct timeval *tv)
{
	printf("%4i.%.2i", (int)tv->tv_sec, (int)tv->tv_usec/10000);
}

static int first;

static int dump_interface(const char *b, const char *p, void *arg)
{

	if (first)
		first = 0;
	else
		printf("\n\t\t\t\t\t\t\t");

	printf("%s", p);

	return 0;
}

void br_dump_interface_list(const char *br)
{
	int err;

	first = 1;
	err = br_foreach_port(br, dump_interface, NULL);
	if (err < 0)
		printf(" can't get port info: %s\n", strerror(-err));
	else
		printf("\n");
}

int dump_port_info(const char *br, const char *p,  void *arg)
{
	struct port_info pinfo;

	if (br_get_port_info(br, p, &pinfo)) {
		printf("Can't get info for %p",p);
		return 1;
	}

	printf("%s (%d)\n", p, pinfo.port_no);
	printf(" port id\t\t%.4x",  pinfo.port_id);
	printf("\t\t\tstate\t\t%15s\n", br_get_state_name(pinfo.state));
	printf(" designated root\t");
	br_dump_bridge_id((unsigned char *)&pinfo.designated_root);
	printf("\tpath cost\t\t%4i\n", pinfo.path_cost);

	printf(" designated bridge\t");
	br_dump_bridge_id((unsigned char *)&pinfo.designated_bridge);
	printf("\tmessage age timer\t");
	br_show_timer(&pinfo.message_age_timer_value);
	printf("\n designated port\t%.4x", pinfo.designated_port);
	printf("\t\t\tforward delay timer\t");
	br_show_timer(&pinfo.forward_delay_timer_value);
	printf("\n designated cost\t%4i", pinfo.designated_cost);
	printf("\t\t\thold timer\t\t");
	br_show_timer(&pinfo.hold_timer_value);
	printf("\n flags\t\t\t");
	if (pinfo.config_pending)
		printf("CONFIG_PENDING ");
	if (pinfo.top_change_ack)
		printf("TOPOLOGY_CHANGE_ACK ");
	if (pinfo.hairpin_mode)
		printf("\n hairpin mode\t\t\%4i", pinfo.hairpin_mode);
	printf("\n");
	printf("\n");
	return 0;
}

void br_dump_info(const char *br, const struct bridge_info *bri)
{
	int err;

	printf("%s\n", br);
	printf(" bridge id\t\t");
	br_dump_bridge_id((unsigned char *)&bri->bridge_id);
	printf("\n designated root\t");
	br_dump_bridge_id((unsigned char *)&bri->designated_root);
	printf("\n root port\t\t%4i\t\t\t", bri->root_port);
	printf("path cost\t\t%4i\n", bri->root_path_cost);
	printf(" max age\t\t");
	br_show_timer(&bri->max_age);
	printf("\t\t\tbridge max age\t\t");
	br_show_timer(&bri->bridge_max_age);
	printf("\n hello time\t\t");
	br_show_timer(&bri->hello_time);
	printf("\t\t\tbridge hello time\t");
	br_show_timer(&bri->bridge_hello_time);
	printf("\n forward delay\t\t");
	br_show_timer(&bri->forward_delay);
	printf("\t\t\tbridge forward delay\t");
	br_show_timer(&bri->bridge_forward_delay);
	printf("\n ageing time\t\t");
	br_show_timer(&bri->ageing_time);
	printf("\n hello timer\t\t");
	br_show_timer(&bri->hello_timer_value);
	printf("\t\t\ttcn timer\t\t");
	br_show_timer(&bri->tcn_timer_value);
	printf("\n topology change timer\t");
	br_show_timer(&bri->topology_change_timer_value);
	printf("\t\t\tgc timer\t\t");
	br_show_timer(&bri->gc_timer_value);
	printf("\n flags\t\t\t");
	if (bri->topology_change)
		printf("TOPOLOGY_CHANGE ");
	if (bri->topology_change_detected)
		printf("TOPOLOGY_CHANGE_DETECTED ");
	printf("\n");
	printf("\n");
	printf("\n");

	err = br_foreach_port(br, dump_port_info, NULL);
	if (err < 0)
		printf("can't get ports: %s\n", strerror(-err));
}
