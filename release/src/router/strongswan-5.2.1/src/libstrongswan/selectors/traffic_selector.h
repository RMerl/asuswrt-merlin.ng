/*
 * Copyright (C) 2007-2013 Tobias Brunner
 * Copyright (C) 2005-2006 Martin Willi
 * Copyright (C) 2005 Jan Hutter
 * Hochschule fuer Technik Rapperswil
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
 * @defgroup traffic_selector traffic_selector
 * @{ @ingroup selectors
 */

#ifndef TRAFFIC_SELECTOR_H_
#define TRAFFIC_SELECTOR_H_

typedef enum ts_type_t ts_type_t;
typedef struct traffic_selector_t traffic_selector_t;

#include <library.h>
#include <networking/host.h>

/**
 * Traffic selector types.
 */
enum ts_type_t {

	/**
	 * A range of IPv4 addresses, represented by two four (4) octet
	 * values.  The first value is the beginning IPv4 address
	 * (inclusive) and the second value is the ending IPv4 address
	 * (inclusive). All addresses falling between the two specified
	 * addresses are considered to be within the list.
	 */
	TS_IPV4_ADDR_RANGE = 7,

	/**
	 * A range of IPv6 addresses, represented by two sixteen (16)
	 * octet values.  The first value is the beginning IPv6 address
	 * (inclusive) and the second value is the ending IPv6 address
	 * (inclusive). All addresses falling between the two specified
	 *  addresses are considered to be within the list.
	 */
	TS_IPV6_ADDR_RANGE = 8
};

/**
 * enum names for ts_type_t
 */
extern enum_name_t *ts_type_name;

/**
 * Object representing a traffic selector entry.
 *
 * A traffic selector defines an range of addresses
 * and a range of ports.
 *
 * If the protocol is ICMP or ICMPv6 the ICMP type and code are stored in the
 * port field as follows:  The message type is placed in the most significant
 * 8 bits and the code in the least significant 8 bits.  Utility functions are
 * provided to extract the individual values.
 */
struct traffic_selector_t {

	/**
	 * Compare two traffic selectors, and create a new one
	 * which is the largest subset of both (subnet & port).
	 *
	 * Resulting traffic_selector is newly created and must be destroyed.
	 *
	 * @param other		traffic selector to compare
	 * @return
	 *					- created subset of them
	 *					- or NULL if no match between this and other
	 */
	traffic_selector_t *(*get_subset)  (traffic_selector_t *this,
										traffic_selector_t *other);

	/**
	 * Clone a traffic selector.
	 *
	 * @return			clone of it
	 */
	traffic_selector_t *(*clone) (traffic_selector_t *this);

	/**
	 * Get starting address of this ts as a chunk.
	 *
	 * Chunk is in network order and points to internal data.
	 *
	 * @return			chunk containing the address
	 */
	chunk_t (*get_from_address) (traffic_selector_t *this);

	/**
	 * Get ending address of this ts as a chunk.
	 *
	 * Chunk is in network order and points to internal data.
	 *
	 * @return			chunk containing the address
	 */
	chunk_t (*get_to_address) (traffic_selector_t *this);

	/**
	 * Get starting port of this ts.
	 *
	 * Port is in host order, since the parser converts it.
	 *
	 * If the protocol is ICMP/ICMPv6 the ICMP type and code are stored in this
	 * field as follows:  The message type is placed in the most significant
	 * 8 bits and the code in the least significant 8 bits.  Use the utility
	 * functions to extract them.
	 *
	 * @return			port
	 */
	u_int16_t (*get_from_port) (traffic_selector_t *this);

	/**
	 * Get ending port of this ts.
	 *
	 * Port is in host order, since the parser converts it.
	 *
	 * If the protocol is ICMP/ICMPv6 the ICMP type and code are stored in this
	 * field as follows:  The message type is placed in the most significant
	 * 8 bits and the code in the least significant 8 bits.  Use the utility
	 * functions to extract them.
	 *
	 * @return			port
	 */
	u_int16_t (*get_to_port) (traffic_selector_t *this);

	/**
	 * Get the type of the traffic selector.
	 *
	 * @return			ts_type_t specifying the type
	 */
	ts_type_t (*get_type) (traffic_selector_t *this);

	/**
	 * Get the protocol id of this ts.
	 *
	 * @return			protocol id
	 */
	u_int8_t (*get_protocol) (traffic_selector_t *this);

	/**
	 * Check if the traffic selector is for a single host.
	 *
	 * Traffic selector may describe the end of *-to-host tunnel. In this
	 * case, the address range is a single address equal to the hosts
	 * peer address.
	 * If host is NULL, the traffic selector is checked if it is a single host,
	 * but not a specific one.
	 *
	 * @param host		host_t specifying the address range
	 */
	bool (*is_host) (traffic_selector_t *this, host_t* host);

	/**
	 * Check if a traffic selector has been created by create_dynamic().
	 *
	 * @return			TRUE if TS is dynamic
	 */
	bool (*is_dynamic)(traffic_selector_t *this);

	/**
	 * Update the address of a traffic selector.
	 *
	 * Update the address range of a traffic selector, if it is
	 * constructed with the traffic_selector_create_dynamic().
	 *
	 * @param host		host_t specifying the address
	 */
	void (*set_address) (traffic_selector_t *this, host_t* host);

	/**
	 * Compare two traffic selectors for equality.
	 *
	 * @param other		ts to compare with this
	 * @return			TRUE if equal, FALSE otherwise
	 */
	bool (*equals) (traffic_selector_t *this, traffic_selector_t *other);

	/**
	 * Check if a traffic selector is contained completly in another.
	 *
	 * contains() allows to check if multiple traffic selectors are redundant.
	 *
	 * @param other		ts that contains this
	 * @return			TRUE if other contains this completly, FALSE otherwise
	 */
	bool (*is_contained_in) (traffic_selector_t *this, traffic_selector_t *other);

	/**
	 * Check if a specific host is included in the address range of
	 * this traffic selector.
	 *
	 * @param host		the host to check
	 */
	bool (*includes) (traffic_selector_t *this, host_t *host);

	/**
	 * Convert a traffic selector address range to a subnet
	 * and its net mask.
	 * If from and to ports of this traffic selector are equal,
	 * the port of the returned host_t is set to that port.
	 *
	 * @param net		converted subnet (has to be freed)
	 * @param mask		converted net mask
	 * @return			TRUE if traffic selector matches exactly to the subnet
	 */
	bool (*to_subnet) (traffic_selector_t *this, host_t **net, u_int8_t *mask);

	/**
	 * Destroys the ts object
	 */
	void (*destroy) (traffic_selector_t *this);
};

/**
 * Extract the ICMP/ICMPv6 message type from a port in host order
 *
 * @param port			port number in host order
 * @return				ICMP/ICMPv6 message type
 */
static inline u_int8_t traffic_selector_icmp_type(u_int16_t port)
{
	return port >> 8;
}

/**
 * Extract the ICMP/ICMPv6 message code from a port in host order
 *
 * @param port			port number in host order
 * @return				ICMP/ICMPv6 message code
 */
static inline u_int8_t traffic_selector_icmp_code(u_int16_t port)
{
	return port & 0xff;
}

/**
 * Create a new traffic selector using human readable params.
 *
 * If protocol is ICMP or ICMPv6 the ports are interpreted as follows:  If they
 * are less than 256 the value is assumed to be a message type, if they are
 * greater or equal to 256 they are assumed to be type and code as defined
 * for traffic_selector_t.
 *
 * @param protocol		protocol for this ts, such as TCP or UDP
 * @param type			type of following addresses, such as TS_IPV4_ADDR_RANGE
 * @param from_addr		start of address range as string
 * @param from_port		port number in host order
 * @param to_addr		end of address range as string
 * @param to_port		port number in host order
 * @return
 *						- traffic_selector_t object
 *						- NULL if invalid address strings/protocol
 */
traffic_selector_t *traffic_selector_create_from_string(
									u_int8_t protocol, ts_type_t type,
									char *from_addr, u_int16_t from_port,
									char *to_addr, u_int16_t to_port);



/**
 * Create a traffic selector from a CIDR string.
 *
 * If protocol is ICMP or ICMPv6 the ports are interpreted as follows:  If they
 * are less than 256 the value is assumed to be a message type, if they are
 * greater or equal to 256 they are assumed to be type and code as defined
 * for traffic_selector_t.
 *
 * @param string		CIDR string, such as 10.1.0.0/16
 * @param protocol		protocol for this ts, such as TCP or UDP
 * @param from_port		start of allowed port range
 * @param to_port		end of port range
 * @return				traffic selector, NULL if string invalid
 */
traffic_selector_t *traffic_selector_create_from_cidr(
										char *string, u_int8_t protocol,
										u_int16_t from_port, u_int16_t to_port);

/**
 * Create a new traffic selector using data read from the net.
 *
 * There exists a mix of network and host order in the params.
 * But the parser gives us this data in this format, so we
 * don't have to convert twice.
 *
 * If protocol is ICMP or ICMPv6 the ports are interpreted as follows:  If they
 * are less than 256 the value is assumed to be a message type, if they are
 * greater or equal to 256 they are assumed to be type and code as defined
 * for traffic_selector_t.
 *
 * @param protocol		protocol for this ts, such as TCP or UDP
 * @param type			type of following addresses, such as TS_IPV4_ADDR_RANGE
 * @param from_address	start of address range, network order
 * @param from_port		port number, host order
 * @param to_address	end of address range, network order
 * @param to_port		port number, host order
 * @return				traffic_selector_t object
 */
traffic_selector_t *traffic_selector_create_from_bytes(
								u_int8_t protocol, ts_type_t type,
								chunk_t from_address, u_int16_t from_port,
								chunk_t to_address, u_int16_t to_port);

/**
 * Create a new traffic selector using the RFC 3779 ASN.1 min/max address format
 *
 * @param type			type of following addresses, such as TS_IPV4_ADDR_RANGE
 * @param from_addr		start of address range in RFC 3779 ASN.1 BIT STRING format
 * @param to_addr		end of address range in RFC 3779 ASN.1 BIT STRING format
 * @return				traffic_selector_t object
 */
traffic_selector_t *traffic_selector_create_from_rfc3779_format(ts_type_t type,
								chunk_t from_addr, chunk_t to_addr);

/**
 * Create a new traffic selector defining a whole subnet.
 *
 * In most cases, definition of a traffic selector for full subnets
 * is sufficient. This constructor creates a traffic selector for
 * all protocols, all ports and the address range specified by the
 * subnet.
 * Additionally, a protocol and ports may be specified.
 *
 * If protocol is ICMP or ICMPv6 the ports are interpreted as follows:  If they
 * are less than 256 the value is assumed to be a message type, if they are
 * greater or equal to 256 they are assumed to be type and code as defined
 * for traffic_selector_t.
 *
 * @param net			subnet to use
 * @param netbits		size of the subnet, as used in e.g. 192.168.0.0/24 notation
 * @param protocol		protocol for this ts, such as TCP or UDP
 * @param from_port		start of allowed port range
 * @param to_port		end of port range
 * @return
 *						- traffic_selector_t object
 *						- NULL if address family of net not supported
 */
traffic_selector_t *traffic_selector_create_from_subnet(
							host_t *net, u_int8_t netbits, u_int8_t protocol,
							u_int16_t from_port, u_int16_t to_port);

/**
 * Create a traffic selector for host-to-host cases.
 *
 * For host2host or virtual IP setups, the traffic selectors gets
 * created at runtime using the external/virtual IP. Using this constructor,
 * a call to set_address() sets this traffic selector to the supplied host.
 *
 * If protocol is ICMP or ICMPv6 the ports are interpreted as follows:  If they
 * are less than 256 the value is assumed to be a message type, if they are
 * greater or equal to 256 they are assumed to be type and code as defined
 * for traffic_selector_t.
 *
 * @param protocol		upper layer protocl to allow
 * @param from_port		start of allowed port range
 * @param to_port		end of range
 * @return
 *						- traffic_selector_t object
 *						- NULL if type not supported
 */
traffic_selector_t *traffic_selector_create_dynamic(u_int8_t protocol,
									u_int16_t from_port, u_int16_t to_port);

/**
 * printf hook function for traffic_selector_t.
 *
 * Arguments are:
 *	traffic_selector_t *ts
 * With the #-specifier, arguments are:
 *	linked_list_t *list containing traffic_selector_t*
 */
int traffic_selector_printf_hook(printf_hook_data_t *data,
							printf_hook_spec_t *spec, const void *const *args);

#endif /** TRAFFIC_SELECTOR_H_ @}*/
