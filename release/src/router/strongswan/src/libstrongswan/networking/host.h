/*
 * Copyright (C) 2006-2014 Tobias Brunner
 * Copyright (C) 2006 Daniel Roethlisberger
 * Copyright (C) 2005-2008 Martin Willi
 * Copyright (C) 2005 Jan Hutter
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

/**
 * @defgroup host host
 * @{ @ingroup networking
 */

#ifndef HOST_H_
#define HOST_H_

#include <utils/utils.h>
#include <utils/chunk.h>

typedef enum host_diff_t host_diff_t;
typedef struct host_t host_t;

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>

/**
 * Represents a Host
 *
 * Host object, identifies a address:port pair and defines some
 * useful functions on it.
 */
struct host_t {

	/**
	 * Build a clone of this host object.
	 *
	 * @return		cloned host
	 */
	host_t *(*clone) (host_t *this);

	/**
	 * Get a pointer to the internal sockaddr struct.
	 *
	 * This is used for sending and receiving via sockets.
	 *
	 * @return		pointer to the internal sockaddr structure
	 */
	sockaddr_t  *(*get_sockaddr) (host_t *this);

	/**
	 * Get the length of the sockaddr struct.
	 *
	 * Depending on the family, the length of the sockaddr struct
	 * is different. Use this function to get the length of the sockaddr
	 * struct returned by get_sock_addr.
	 *
	 * This is used for sending and receiving via sockets.
	 *
	 * @return		length of the sockaddr struct
	 */
	socklen_t *(*get_sockaddr_len) (host_t *this);

	/**
	 * Gets the family of the address
	 *
	 * @return		family
	 */
	int (*get_family) (host_t *this);

	/**
	 * Checks if the ip address of host is set to default route.
	 *
	 * @return		TRUE if host is 0.0.0.0 or 0::0, FALSE otherwise
	 */
	bool (*is_anyaddr) (host_t *this);

	/**
	 * Get the address of this host as chunk_t
	 *
	 * Returned chunk points to internal data.
	 *
	 * @return		address blob
	 */
	chunk_t (*get_address) (host_t *this);

	/**
	 * Get the port of this host
	 *
	 * @return		port number
	 */
	uint16_t (*get_port) (host_t *this);

	/**
	 * Set the port of this host
	 *
	 * @param port	port number
	 */
	void (*set_port) (host_t *this, uint16_t port);

	/**
	 * Compare the ips of two hosts hosts.
	 *
	 * @param other	the other to compare
	 * @return		TRUE if addresses are equal.
	 */
	bool (*ip_equals) (host_t *this, host_t *other);

	/**
	 * Compare two hosts, with port.
	 *
	 * @param other	the other to compare
	 * @return		TRUE if addresses and ports are equal.
	 */
	bool (*equals) (host_t *this, host_t *other);

	/**
	 * Destroy this host object.
	 */
	void (*destroy) (host_t *this);
};

/**
 * Constructor to create a host_t object from an address string.
 *
 * @param string		string of an address, such as "152.96.193.130"
 * @param port			port number
 * @return				host_t, NULL if string not an address.
 */
host_t *host_create_from_string(char *string, uint16_t port);

/**
 * Same as host_create_from_string(), but with the option to enforce a family.
 *
 * @param string		string of an address
 * @param family		address family, or AF_UNSPEC
 * @param port			port number
 * @return				host_t, NULL if string not an address.
 */
host_t *host_create_from_string_and_family(char *string, int family,
										   uint16_t port);

/**
 * Constructor to create a host_t from a DNS name.
 *
 * @param string		hostname to resolve
 * @param family		family to prefer, 0 for first match
 * @param port			port number
 * @return				host_t, NULL lookup failed
 */
host_t *host_create_from_dns(char *string, int family, uint16_t port);

/**
 * Constructor to create a host_t object from an address chunk.
 *
 * If family is AF_UNSPEC, it is guessed using address.len.
 *
 * @param family		Address family, such as AF_INET or AF_INET6
 * @param address		address as chunk_t in network order
 * @param port			port number
 * @return				host_t, NULL if family not supported/chunk invalid
 */
host_t *host_create_from_chunk(int family, chunk_t address, uint16_t port);

/**
 * Constructor to create a host_t object from a sockaddr struct
 *
 * @param sockaddr		sockaddr struct which contains family, address and port
 * @return				host_t, NULL if family not supported
 */
host_t *host_create_from_sockaddr(sockaddr_t *sockaddr);

/**
 * Parse a range definition (1.2.3.0-1.2.3.5), return the two hosts.
 *
 * The two hosts are not ordered, from is simply the first, to is the second,
 * from is not necessarily smaller.
 *
 * @param string		string to parse
 * @param from			returns the first address (out)
 * @param to			returns the second address (out)
 * @return				TRUE if parsed successfully, FALSE otherwise
 */
bool host_create_from_range(char *string, host_t **from, host_t **to);

/**
 * Create a host from a CIDR subnet definition (1.2.3.0/24), return bits.
 *
 * @param string		string to parse
 * @param bits			gets the number of network bits in CIDR notation
 * @return				network start address, NULL on error
 */
host_t *host_create_from_subnet(char *string, int *bits);

/**
 * Create a netmask host having the first netbits bits set.
 *
 * @param family		family of the netmask host
 * @param netbits		number of leading bits set in the host
 * @return				netmask host
 */
host_t *host_create_netmask(int family, int netbits);

/**
 * Create a host without an address, a "any" host.
 *
 * @param family		family of the any host
 * @return				host_t, NULL if family not supported
 */
host_t *host_create_any(int family);

/**
 * printf hook function for host_t.
 *
 * Arguments are:
 *	host_t *host
 * Use #-modifier to include port number
 * Use +-modifier to force numeric representation (instead of e.g. %any)
 */
int host_printf_hook(printf_hook_data_t *data, printf_hook_spec_t *spec,
					 const void *const *args);

#endif /** HOST_H_ @}*/
