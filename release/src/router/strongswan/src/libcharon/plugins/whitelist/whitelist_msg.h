/*
 * Copyright (C) 2011 Martin Willi
 * Copyright (C) 2011 revosec AG
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
 * @defgroup whitelist_msg whitelist_msg
 * @{ @ingroup whitelist
 */

#ifndef WHITELIST_MSG_H_
#define WHITELIST_MSG_H_

#define WHITELIST_SOCKET IPSEC_PIDDIR "/charon.wlst"

typedef struct whitelist_msg_t whitelist_msg_t;

/**
 * Message type.
 */
enum {
	/* add whitelist entry */
	WHITELIST_ADD = 1,
	/* remove whitelist entry */
	WHITELIST_REMOVE = 2,
	/* list identities matching id, gets responded with LIST messages */
	WHITELIST_LIST = 3,
	/* indicates end of list in a series of LIST messages */
	WHITELIST_END = 4,
	/* flush identities matching id */
	WHITELIST_FLUSH = 5,
	/* enable whitelist checking */
	WHITELIST_ENABLE = 6,
	/* disable whitelist checking */
	WHITELIST_DISABLE = 7,
};

/**
 * Message to exchange over whitelist
 */
struct whitelist_msg_t {
	/** message type */
	int type;
	/** null terminated identity */
	char id[128];
} __attribute__((packed));

#endif /** WHITELIST_MSG_H_ @}*/
