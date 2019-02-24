/*
 * Copyright (C) 2013 Martin Willi
 * Copyright (C) 2013 revosec AG
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
 * @defgroup duplicheck_msg duplicheck_msg
 * @{ @ingroup duplicheck
 */

#ifndef DUPLICHECK_MSG_H_
#define DUPLICHECK_MSG_H_

#include <sys/types.h>

/**
 * Default Unix socket to connect to
 */
#define DUPLICHECK_SOCKET IPSEC_PIDDIR "/charon.dck"

typedef struct duplicheck_msg_t duplicheck_msg_t;

/**
 * Message exchanged over duplicheck socket
 */
struct duplicheck_msg_t {
	/** length of the identity following, in network order (excluding len). */
	uint16_t len;
	/** identity string, not null terminated */
	char identity[];
} __attribute__((__packed__));

#endif /** DUPLICHECK_MSG_H_ @}*/
