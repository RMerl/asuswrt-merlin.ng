/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2015 National Instruments
 *
 * (C) Copyright 2015
 * Joe Hershberger <joe.hershberger@ni.com>
 */

#ifndef __ETH_RAW_OS_H
#define __ETH_RAW_OS_H

#define	IFNAMSIZ	16

/**
 * struct eth_sandbox_raw_priv - raw socket session
 *
 * sd: socket descriptor - the open socket during a session
 * host_ifname: interface name on the host to use for sending our packets
 * host_ifindex: interface index number on the host
 * device: struct sockaddr_ll - the host interface packets move to/from
 * local: 1 or 0 to select the local interface ('lo') or not
 * local_bindsd: socket descriptor to prevent the kernel from sending
 *		 a message to the server claiming the port is
 *		 unreachable
 * local_bind_udp_port: The UDP port number that we bound to
 */
struct eth_sandbox_raw_priv {
	int sd;
	char host_ifname[IFNAMSIZ];
	unsigned int host_ifindex;
	void *device;
	int local;
	int local_bind_sd;
	unsigned short local_bind_udp_port;
};

/* A struct to mimic if_nameindex but that does not depend on Linux headers */
struct sandbox_eth_raw_if_nameindex {
	unsigned int if_index; /* Index of interface (1, 2, ...) */
	char        *if_name;  /* Null-terminated name ("eth0", etc.) */
};

/* Enumerate host network interfaces */
struct sandbox_eth_raw_if_nameindex *sandbox_eth_raw_if_nameindex(void);
/* Free the data structure of enumerated network interfaces */
void sandbox_eth_raw_if_freenameindex(struct sandbox_eth_raw_if_nameindex *ptr);

/*
 * Check if the interface named "ifname" is a localhost interface or not.
 * ifname - the interface name on the host to check
 *
 * returns - 0 if real interface, 1 if local, negative if error
 */
int sandbox_eth_raw_os_is_local(const char *ifname);

/*
 * Look up the name of the interface based on the ifindex populated in priv.
 *
 * Overwrite the host_ifname member in priv based on looking up host_ifindex
 *
 * returns - 0 if success, negative if error
 */
int sandbox_eth_raw_os_idx_to_name(struct eth_sandbox_raw_priv *priv);

int sandbox_eth_raw_os_start(struct eth_sandbox_raw_priv *priv,
			     unsigned char *ethmac);
int sandbox_eth_raw_os_send(void *packet, int length,
			    struct eth_sandbox_raw_priv *priv);
int sandbox_eth_raw_os_recv(void *packet, int *length,
			    const struct eth_sandbox_raw_priv *priv);
void sandbox_eth_raw_os_stop(struct eth_sandbox_raw_priv *priv);

#endif /* __ETH_RAW_OS_H */
