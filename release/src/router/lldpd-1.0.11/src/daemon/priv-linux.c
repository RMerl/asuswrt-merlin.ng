/* -*- mode: c; c-file-style: "openbsd" -*- */
/*
 * Copyright (c) 2008 Vincent Bernat <bernat@luffy.cx>
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

#include "lldpd.h"

#include <unistd.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>
#include <fcntl.h>
#include <errno.h>
#include <regex.h>
#include <sys/ioctl.h>
#include <netpacket/packet.h> /* For sockaddr_ll */
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#endif
#include <linux/filter.h>     /* For BPF filtering */
#include <linux/sockios.h>
#include <linux/if_ether.h>
#include <linux/ethtool.h>
#if defined(__clang__)
#pragma clang diagnostic pop
#endif

/* Defined in linux/pkt_sched.h */
#define TC_PRIO_CONTROL 7
/* Defined in sysfs/libsysfs.h */
#define SYSFS_PATH_MAX 256

/* Proxy for open */
int
priv_open(char *file)
{
	int len, rc;
	enum priv_cmd cmd = PRIV_OPEN;
	must_write(PRIV_UNPRIVILEGED, &cmd, sizeof(enum priv_cmd));
	len = strlen(file);
	must_write(PRIV_UNPRIVILEGED, &len, sizeof(int));
	must_write(PRIV_UNPRIVILEGED, file, len);
	priv_wait();
	must_read(PRIV_UNPRIVILEGED, &rc, sizeof(int));
	if (rc == -1)
		return rc;
	return receive_fd(PRIV_UNPRIVILEGED);
}

void
asroot_open()
{
	const char* authorized[] = {
		PROCFS_SYS_NET "ipv4/ip_forward",
		PROCFS_SYS_NET "ipv6/conf/all/forwarding",
		"/proc/net/bonding/[^.][^/]*",
		"/proc/self/net/bonding/[^.][^/]*",
#ifdef ENABLE_OLDIES
		SYSFS_CLASS_NET "[^.][^/]*/brforward",
		SYSFS_CLASS_NET "[^.][^/]*/brport",
		SYSFS_CLASS_NET "[^.][^/]*/brif/[^.][^/]*/port_no",
#endif
		SYSFS_CLASS_DMI "product_version",
		SYSFS_CLASS_DMI "product_serial",
		SYSFS_CLASS_DMI "product_name",
		SYSFS_CLASS_DMI "bios_version",
		SYSFS_CLASS_DMI "sys_vendor",
		SYSFS_CLASS_DMI "chassis_asset_tag",
		NULL
	};
	const char **f;
	char *file;
	int fd, len, rc;
	regex_t preg;

	must_read(PRIV_PRIVILEGED, &len, sizeof(len));
	if (len < 0 || len > PATH_MAX)
		fatalx("privsep", "too large value requested");
	if ((file = (char *)malloc(len + 1)) == NULL)
		fatal("privsep", NULL);
	must_read(PRIV_PRIVILEGED, file, len);
	file[len] = '\0';

	for (f=authorized; *f != NULL; f++) {
		if (regcomp(&preg, *f, REG_NOSUB) != 0)
			/* Should not happen */
			fatal("privsep", "unable to compile a regex");
		if (regexec(&preg, file, 0, NULL, 0) == 0) {
			regfree(&preg);
			break;
		}
		regfree(&preg);
	}
	if (*f == NULL) {
		log_warnx("privsep", "not authorized to open %s", file);
		rc = -1;
		must_write(PRIV_PRIVILEGED, &rc, sizeof(int));
		free(file);
		return;
	}
	if ((fd = open(file, O_RDONLY)) == -1) {
		rc = -1;
		must_write(PRIV_PRIVILEGED, &rc, sizeof(int));
		free(file);
		return;
	}
	free(file);
	must_write(PRIV_PRIVILEGED, &fd, sizeof(int));
	send_fd(PRIV_PRIVILEGED, fd);
	close(fd);
}

/* Quirks needed by some additional interfaces. Currently, this is limited to
 * disabling LLDP firmware for i40e. */
static void
asroot_iface_init_quirks(int ifindex, char *name)
{
	int s = -1;
	int fd = -1;

	/* Check driver. */
	struct ethtool_drvinfo ethc = {
		.cmd = ETHTOOL_GDRVINFO
	};
	struct ifreq ifr = {
		.ifr_data = (caddr_t)&ethc
	};
	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		log_warn("privsep", "unable to open a socket");
		goto end;
	}
	strlcpy(ifr.ifr_name, name, IFNAMSIZ);
	if (ioctl(s, SIOCETHTOOL, &ifr) != 0 ||
	    strncmp("i40e", ethc.driver, sizeof(ethc.driver))) {
		/* Not i40e */
		goto end;
	}
	log_info("interfaces",
	    "i40e driver detected for %s, disabling LLDP in firmware",
	    name);

	/* We assume debugfs is mounted. Otherwise, we would need to check if it
	 * is mounted, then unshare a new mount namespace, mount it, issues the
	 * command, leave the namespace. Let's see if there is such a need. */

	/* Alternative is to use ethtool (ethtool --set-priv-flags ens5f0
	 * disable-fw-lldp on). However, this requires a recent firmware (from
	 * i40e_ethtool.c):
	 *
	 * If the driver detected FW LLDP was disabled on init, this flag could
	 * be set, however we do not support _changing_ the flag:
	 * - on XL710 if NPAR is enabled or FW API version < 1.7
	 * - on X722 with FW API version < 1.6
	 */

	char command[] = "lldp stop";
	char sysfs_path[SYSFS_PATH_MAX+1];
	if (snprintf(sysfs_path, SYSFS_PATH_MAX,
	    "/sys/kernel/debug/i40e/%.*s/command",
	    (int)sizeof(ethc.bus_info), ethc.bus_info) >= SYSFS_PATH_MAX) {
		log_warnx("interfaces", "path truncated");
		goto end;
	}
	if ((fd = open(sysfs_path, O_WRONLY)) == -1) {
		if (errno == ENOENT) {
			log_info("interfaces",
			    "%s does not exist, "
			    "cannot disable LLDP in firmware for %s",
			    sysfs_path, name);
			goto end;
		}
		log_warn("interfaces",
		    "cannot open %s to disable LLDP in firmware for %s",
		    sysfs_path, name);
		goto end;
	}
	if (write(fd, command, sizeof(command) - 1) == -1) {
		log_warn("interfaces",
		    "cannot disable LLDP in firmware for %s",
		    name);
		goto end;
	}
end:
	if (s != -1) close(s);
	if (fd != -1) close(fd);
}

int
asroot_iface_init_os(int ifindex, char *name, int *fd)
{
	int rc;
	/* Open listening socket to receive/send frames */
	if ((*fd = socket(PF_PACKET, SOCK_RAW,
		    htons(ETH_P_ALL))) < 0) {
		rc = errno;
		return rc;
	}

	struct sockaddr_ll sa = {
		.sll_family = AF_PACKET,
		.sll_ifindex = ifindex
	};
	if (bind(*fd, (struct sockaddr*)&sa, sizeof(sa)) < 0) {
		rc = errno;
		log_warn("privsep",
		    "unable to bind to raw socket for interface %s",
		    name);
		return rc;
	}

	/* Set filter */
	log_debug("privsep", "set BPF filter for %s", name);
	static struct sock_filter lldpd_filter_f[] = { LLDPD_FILTER_F };
	struct sock_fprog prog = {
		.filter = lldpd_filter_f,
		.len = sizeof(lldpd_filter_f) / sizeof(struct sock_filter)
	};
	if (setsockopt(*fd, SOL_SOCKET, SO_ATTACH_FILTER,
	    &prog, sizeof(prog)) < 0) {
		rc = errno;
		log_warn("privsep", "unable to change filter for %s", name);
		return rc;
	}

	/* Set priority to TC_PRIO_CONTROL for ice Intel cards. See #444. */
	int prio = TC_PRIO_CONTROL;
	if (setsockopt(*fd, SOL_SOCKET, SO_PRIORITY, &prio, sizeof(prio)) < 0) {
		rc = errno;
		log_warn("privsep",
		    "unable to set priority \"control\" to socket for interface %s",
		    name);
		return rc;
	}

#ifdef SO_LOCK_FILTER
	int lock = 1;
	if (setsockopt(*fd, SOL_SOCKET, SO_LOCK_FILTER,
	    &lock, sizeof(lock)) < 0) {
		if (errno != ENOPROTOOPT) {
			rc = errno;
			log_warn("privsep", "unable to lock filter for %s", name);
			return rc;
		}
	}
#endif
#ifdef PACKET_IGNORE_OUTGOING
	int ignore = 1;
	if (setsockopt(*fd, SOL_PACKET, PACKET_IGNORE_OUTGOING,
	    &ignore, sizeof(ignore)) < 0) {
		if (errno != ENOPROTOOPT) {
			rc = errno;
			log_warn("privsep",
			    "unable to set packet direction for BPF filter on %s",
			    name);
			return rc;
		}
	}
#endif

	asroot_iface_init_quirks(ifindex, name);
	return 0;
}

int
asroot_iface_description_os(const char *name, const char *description)
{
	/* We could use netlink but this is a lot to do in a privileged
	 * process. Just write to /sys/class/net/XXXX/ifalias. */
	char *file;
#ifndef IFALIASZ
# define IFALIASZ 256
#endif
	char descr[IFALIASZ];
	FILE *fp;
	int rc;
	if (name[0] == '\0' || name[0] == '.') {
		log_warnx("privsep", "odd interface name %s", name);
		return -1;
	}
	if (asprintf(&file, SYSFS_CLASS_NET "%s/ifalias", name) == -1) {
		log_warn("privsep", "unable to allocate memory for setting description");
		return -1;
	}
	if ((fp = fopen(file, "r+")) == NULL) {
		rc = errno;
		log_debug("privsep", "cannot open interface description for %s: %s",
		    name, strerror(errno));
		free(file);
		return rc;
	}
	free(file);
	if (strlen(description) == 0 &&
	    fgets(descr, sizeof(descr), fp) != NULL) {
		if (strncmp(descr, "lldpd: ", 7) == 0) {
			if (strncmp(descr + 7, "was ", 4) == 0) {
				/* Already has an old neighbor */
				fclose(fp);
				return 0;
			} else {
				/* Append was */
				memmove(descr + 11, descr + 7,
				    sizeof(descr) - 11);
				memcpy(descr, "lldpd: was ", 11);
			}
		} else {
			/* No description, no neighbor */
			strlcpy(descr, "lldpd: no neighbor", sizeof(descr));
		}
	} else
		snprintf(descr, sizeof(descr), "lldpd: connected to %s", description);
	if (fputs(descr, fp) == EOF) {
		log_debug("privsep", "cannot set interface description for %s",
		    name);
		fclose(fp);
		return -1;
	}
	fclose(fp);
	return 0;
}

int
asroot_iface_promisc_os(const char *name)
{
	int s, rc;
	if ((s = socket(PF_PACKET, SOCK_RAW,
		    htons(ETH_P_ALL))) < 0) {
		rc = errno;
		log_warn("privsep", "unable to open raw socket");
		return rc;
	}

	struct ifreq ifr = {};
	strlcpy(ifr.ifr_name, name, sizeof(ifr.ifr_name));

	if (ioctl(s, SIOCGIFFLAGS, &ifr) == -1) {
		rc = errno;
		log_warn("privsep", "unable to get interface flags for %s",
		    name);
		close(s);
		return rc;
	}

	if (ifr.ifr_flags & IFF_PROMISC) {
		close(s);
		return 0;
	}
	ifr.ifr_flags |= IFF_PROMISC;
	if (ioctl(s, SIOCSIFFLAGS, &ifr) == -1) {
		rc = errno;
		log_warn("privsep", "unable to set promisc mode for %s",
		    name);
		close(s);
		return rc;
	}
	log_info("privsep", "promiscuous mode enabled for %s", name);
	close(s);
	return 0;
}
