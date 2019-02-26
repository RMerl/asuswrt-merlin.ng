/*
 * Copyright (C) 2012 Martin Willi
 * Copyright (C) 2012 revosec AG
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
 * @defgroup lookip_msg lookip_msg
 * @{ @ingroup lookip
 */

#ifndef LOOKIP_MSG_H_
#define LOOKIP_MSG_H_

#define LOOKIP_SOCKET IPSEC_PIDDIR "/charon.lkp"

typedef struct lookip_request_t lookip_request_t;
typedef struct lookip_response_t lookip_response_t;

/**
 * Message type.
 *
 * The client can send a batch of request messages, containing DUMP, LOOKUP or
 * REGISTER_* messages. The server immediately starts sending responses for
 * these messages, using ENTRY or NOTIFY_* messages.
 * A client MUST send an END message to complete a batch. The server will
 * send any remaining responses, but will not accept new requests and closes
 * the connection when complete.
 */
enum {
	/** request a dump of all entries */
	LOOKIP_DUMP = 1,
	/** lookup a specific virtual IP */
	LOOKIP_LOOKUP,
	/** reply message for DUMP and LOOKUP */
	LOOKIP_ENTRY,
	/** reply message for LOOKUP if no such IP found */
	LOOKIP_NOT_FOUND,
	/** register for notifications about new virtual IPs */
	LOOKIP_REGISTER_UP,
	/** register for notifications about virtual IPs released */
	LOOKIP_REGISTER_DOWN,
	/** notify reply message for REGISTER_UP */
	LOOKIP_NOTIFY_UP,
	/** notify reply message for REGISTER_DOWN */
	LOOKIP_NOTIFY_DOWN,
	/** end of request batch */
	LOOKIP_END,
};

/**
 * Request message sent from client.
 *
 * Valid request message types are DUMP, LOOKUP, REGISTER_UP/DOWN and END.
 *
 * The vip field is used only in LOOKUP requests, but ignored otherwise.
 */
struct lookip_request_t {
	/** request message type */
	int type;
	/** null terminated string representation of virtual IP */
	char vip[40];
} __attribute__((packed));

/**
 * Response message sent to client.
 *
 * Valid response message types are ENTRY, NOT_FOUND and NOTIFY_UP/DOWN.
 *
 * All fields are set in all messages, except in NOT_FOUND: Only vip is set.
 */
struct lookip_response_t {
	/** response message type */
	int type;
	/** null terminated string representation of virtual IP */
	char vip[40];
	/** null terminated string representation of outer IP */
	char ip[40];
	/** null terminated peer identity */
	char id[256];
	/** null terminated connection name */
	char name[40];
	/** unique connection id */
	unsigned int unique_id;
} __attribute__((packed));

#endif /** LOOKIP_MSG_H_ @}*/
