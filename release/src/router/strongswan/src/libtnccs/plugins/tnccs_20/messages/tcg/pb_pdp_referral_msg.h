/*
 * Copyright (C) 2013 Andreas Steffen
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
 * @defgroup pb_pdp_referral_msg pb_pdp_referral_msg
 * @{ @ingroup tnccs_20
 */

#ifndef PB_PDP_REFERRAL_MSG_H_
#define PB_PDP_REFERRAL_MSG_H_

typedef enum pb_pdp_identifier_type_t pb_pdp_identifier_type_t;
typedef struct pb_pdp_referral_msg_t pb_pdp_referral_msg_t;

#include "messages/pb_tnc_msg.h"

#include <pen/pen.h>

/**
 * PB-TNC PDP Identifier Types as defined in section 3.1.1.2 of the
 * TCG TNC PDP Discovery and Validation Specification 1.0
 */
enum pb_pdp_identifier_type_t {
	PB_PDP_ID_FQDN =	0,
	PB_PDP_ID_IPV4 =	1,
	PB_PDP_ID_IPV6 =	2
};

/**
 * enum name for pb_pdp_identifier_type_t.
 */
extern enum_name_t *pb_pdp_identifier_type_names;

/**
 * Class representing the PB-Remediation-Parameters message type.
 */
struct pb_pdp_referral_msg_t {

	/**
	 * PB-TNC Message interface
	 */
	pb_tnc_msg_t pb_interface;

	/**
	 * Get the PDP Identifier Type (Vendor ID and Type)
	 *
	 * @return				PDP Identifier Type
	 */
	pen_type_t (*get_identifier_type)(pb_pdp_referral_msg_t *this);

	/**
	 * Get the PDP Identifier Value
	 *
	 * @return				PDP Identifier Value
	 */
	chunk_t (*get_identifier)(pb_pdp_referral_msg_t *this);

	/**
	 * Get the PDP Identifier Value
	 *
	 * @param protocol		PT transport protocol
	 * @param port			PT port the PDP is listening on
	 * @return				Fully Qualified Domain Name of PDP
	 */
	chunk_t (*get_fqdn)(pb_pdp_referral_msg_t *this, uint8_t *protocol,
						uint16_t *port);

};

/**
 * Create a general PB-PDP-Referral message
 *
 * @param identifier_type	PDP Identifier Type
 * @param identifier		PDP Identifier
 */
pb_tnc_msg_t* pb_pdp_referral_msg_create(pen_type_t identifier_type,
										 chunk_t identifier);

/**
 * Create a PB-PDP-Referral message of TCG Type PDP FQDN Identifier
 *
 * @param fqdn				Fully Qualified Domain Name of PDP
 * @param port				PT-TLS port the PDP is listening on 
 */
pb_tnc_msg_t* pb_pdp_referral_msg_create_from_fqdn(chunk_t fqdn, uint16_t port);

/**
 * Create an unprocessed PB-PDP-Referral message from raw data
 *
  * @param data		PB-PDP-Referral message data
 */
pb_tnc_msg_t* pb_pdp_referral_msg_create_from_data(chunk_t data);

#endif /** PB_PA_MSG_H_ @}*/
