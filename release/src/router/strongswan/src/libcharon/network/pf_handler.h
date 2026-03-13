/*
 * Copyright (C) 2024 Tobias Brunner
 * Copyright (C) 2020-2023 Dan James <sddj@me.com>
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

#ifndef PF_HANDLER_H_
#define PF_HANDLER_H_

#include <sys/types.h>
#include <utils/chunk.h>

typedef struct pf_handler_t pf_handler_t;

/**
 * BPF implementation for different platforms
 */
struct pf_handler_t {

	/**
	 * Destroy this instance.
	 */
	void (*destroy)(pf_handler_t *this);
};

/**
 * Callback that's called for received packets.
 *
 * @param ctx		context as passed in the constructor
 * @param if_name	name of the interface on which the packet was received
 * @param if_index	index of the interface on which the packet was received
 * @param mac		MAC address of the interface on which the packet was received
 * @param fd		file descriptor of the receiving socket (may be used to send
 *					a response)
 * @param packet	the received packet
 */
typedef void (*pf_packet_handler_t)(void *ctx, char *if_name, int if_index,
									chunk_t mac, int fd, chunk_t packet);

/**
 * Type for BFP programs on different platforms
 */
#if !defined(__APPLE__) && !defined(__FreeBSD__)
typedef struct sock_fprog pf_program_t;
#else
typedef struct bpf_program pf_program_t;
#endif /* !defined(__APPLE__) && !defined(__FreeBSD__) */

/**
 * Create a pf_handler_t instance.
 *
 * @param name		name to identify this handler ("ARP" is treated specially)
 * @param iface		optional interface to limit capturing to
 * @param handler	handler for received packets
 * @param ctx		context passed to handler
 * @param program	BPF filter program
 */
pf_handler_t *pf_handler_create(const char *name, char *iface,
								pf_packet_handler_t handler, void *ctx,
								pf_program_t *program);

/**
 * Bind a socket to a particular network interface
 *
 * @param fd		file descriptor of the socket
 * @param iface		name of the interface
 * @return			whether the socket was successfully bound
 */
bool bind_to_device(int fd, char *iface);

#endif /** PF_HANDLER_H_ */
