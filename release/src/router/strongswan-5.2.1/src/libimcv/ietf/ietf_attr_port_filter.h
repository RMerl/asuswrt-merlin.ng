/*
 * Copyright (C) 2011-2014 Andreas Steffen
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
 * @defgroup ietf_attr_port_filtert ietf_attr_port_filter
 * @{ @ingroup ietf_attr
 */

#ifndef IETF_ATTR_PORT_FILTER_H_
#define IETF_ATTR_PORT_FILTER_H_

typedef struct ietf_attr_port_filter_t ietf_attr_port_filter_t;

#include "ietf_attr.h"
#include "pa_tnc/pa_tnc_attr.h"


/**
 * Class implementing the IETF PA-TNC Port Filter attribute.
 *
 */
struct ietf_attr_port_filter_t {

	/**
	 * Public PA-TNC attribute interface
	 */
	pa_tnc_attr_t pa_tnc_attribute;

	/**
	 * Add a port entry
	 *
	 * @param blocked		TRUE if blocked, FALSE if allowed
	 * @param protocol		IP protocol type
	 * @param port			TCP/UDP port number
	 */
	void (*add_port)(ietf_attr_port_filter_t *this, bool blocked,
					 u_int8_t protocol, u_int16_t port);

	/**
	 * Enumerates over all ports
	 * Format:  bool *blocked, u_int8_t *protocol, u_int16_t *port
	 *
	 * @return				enumerator
	 */
	enumerator_t* (*create_port_enumerator)(ietf_attr_port_filter_t *this);

};

/**
 * Creates an ietf_attr_port_filter_t object
 *
 */
pa_tnc_attr_t* ietf_attr_port_filter_create(void);

/**
 * Creates an ietf_attr_port_filter_t object from received data
 *
 * @param length			Total length of attribute value
 * @param value				Unparsed attribute value (might be a segment)
 */
pa_tnc_attr_t* ietf_attr_port_filter_create_from_data(size_t length,
													  chunk_t value);

#endif /** IETF_ATTR_PORT_FILTER_H_ @}*/
