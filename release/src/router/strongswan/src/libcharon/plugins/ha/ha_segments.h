/*
 * Copyright (C) 2008 Martin Willi
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
 * @defgroup ha_segments ha_segments
 * @{ @ingroup ha
 */

#ifndef HA_SEGMENTS_H_
#define HA_SEGMENTS_H_

#include <daemon.h>

typedef struct ha_segments_t ha_segments_t;

typedef uint16_t segment_mask_t;

/**
 * maximum number of segments
 */
#define SEGMENTS_MAX (sizeof(segment_mask_t)*8)

/**
 * Get the bit in the mask of a segment
 */
#define SEGMENTS_BIT(segment) (0x01 << (segment - 1))

#include "ha_socket.h"
#include "ha_tunnel.h"
#include "ha_kernel.h"

/**
 * Segmentation of peers into active and passive.
 */
struct ha_segments_t {

	/**
	 * Implements listener interface to catch daemon shutdown.
	 */
	listener_t listener;

	/**
	 * Activate a set of IKE_SAs identified by a segment.
	 *
	 * @param segment	numerical segment to takeover, 0 for all
	 * @param notify	whether to notify other nodes about activation
	 */
	void (*activate)(ha_segments_t *this, u_int segment, bool notify);

	/**
	 * Deactivate a set of IKE_SAs identified by a segment.
	 *
	 * @param segment	numerical segment to takeover, 0 for all
	 * @param notify	whether to notify other nodes about deactivation
	 */
	void (*deactivate)(ha_segments_t *this, u_int segment, bool notify);

	/**
	 * Handle a status message from the remote node.
	 *
	 * @param mask		segments the remote node is serving actively
	 */
	void (*handle_status)(ha_segments_t *this, segment_mask_t mask);

	/**
	 * Check if a given segment is currently active.
	 *
	 * @param segment	segment to check
	 * @return			TRUE if segment active
	 */
	bool (*is_active)(ha_segments_t *this, u_int segment);

	/**
	 * Return the number of segments
	 *
	 * @return			number of segments
	 */
	u_int (*count)(ha_segments_t *this);

	/**
	 * Destroy a ha_segments_t.
	 */
	void (*destroy)(ha_segments_t *this);
};

/**
 * Create a ha_segments instance.
 *
 * @param socket		socket to communicate segment (de-)activation
 * @param kernel		interface to control segments at kernel level
 * @param tunnel		HA tunnel
 * @param count			number of segments the cluster uses
 * @param node			node, currently 1 or 0
 * @param monitor		should we use monitoring functionality
 * @return				segment object
 */
ha_segments_t *ha_segments_create(ha_socket_t *socket, ha_kernel_t *kernel,
								  ha_tunnel_t *tunnel, u_int count, u_int node,
								  bool monitor);

#endif /** HA_SEGMENTS_ @}*/
