/*
 * Copyright (C) 2011 Sansar Choinyambuu
 * HSR Hochschule fuer Technik Rapperswil
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
 * @defgroup pts_proto_caps pts_proto_caps
 * @{ @ingroup pts
 */

#ifndef PTS_PROTO_CAPS_H_
#define PTS_PROTO_CAPS_H_

typedef enum pts_proto_caps_flag_t pts_proto_caps_flag_t;

#include <library.h>

/**
 * PTS Protocol Capabilities Flags
 */
enum pts_proto_caps_flag_t {
	/** XML based Evidence Support flag */
	PTS_PROTO_CAPS_X =		(1<<0),
	/** Trusted Platform Evidence flag */
	PTS_PROTO_CAPS_T =		 (1<<1),
	/** DH Nonce Negotiation Support flag */
	PTS_PROTO_CAPS_D =		 (1<<2),
	/** Verification Support flag */
	PTS_PROTO_CAPS_V =		 (1<<3),
	/** Current (In-Memory) Evidence Support flag */
	PTS_PROTO_CAPS_C =		(1<<4),
};

#endif /** PTS_PROTO_CAPS_H_ @}*/
