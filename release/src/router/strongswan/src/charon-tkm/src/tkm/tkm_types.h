/*
 * Copyright (C) 2012 Reto Buerki
 * Copyright (C) 2012 Adrian-Ken Rueegsegger
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
 * @defgroup tkm-types types
 * @{ @ingroup tkm
 */

#ifndef TKM_TYPES_H_
#define TKM_TYPES_H_

#include <tkm/types.h>
#include <utils/chunk.h>

typedef struct esa_info_t esa_info_t;

/**
 * ESP SA info data structure.
 *
 * This type is used to transfer ESA information from the keymat
 * derive_child_keys to the kernel IPsec interface add_sa operation. This is
 * necessary because the CHILD SA key derivation and installation is handled
 * by a single exchange with the TKM (esa_create*) in add_sa.
 * For this purpose the out parameters encr_i and encr_r of the
 * derive_child_keys function are (ab)used and the data is stored in these
 * data chunks. This is possible since the child SA keys are treated as opaque
 * values and handed to the add_sa procedure of the kernel interface as-is
 * without any processing.
 */
struct esa_info_t {

	/**
	 * ISA context id.
	 */
	isa_id_type isa_id;

	/**
	 * Responder SPI of child SA.
	 */
	esp_spi_type spi_r;

	/**
	 * Initiator nonce.
	 */
	chunk_t nonce_i;

	/**
	 * Responder nonce.
	 */
	chunk_t nonce_r;

	/**
	 * Flag specifying if this esa info struct is contained in encr_r.
	 * It is set to TRUE for encr_r and FALSE for encr_i.
	 */
	bool is_encr_r;

	/**
	 * Diffie-Hellman context id.
	 */
	dh_id_type dh_id;

};

typedef struct isa_info_t isa_info_t;

/**
 * IKE SA info data structure.
 *
 * This type is used to transfer ISA information from the keymat of the parent
 * SA to the keymat of the new IKE SA. For this purpose the skd data chunk is
 * (ab)used. This is possible since the sk_d chunk is treated as an opaque value
 * and handed to the derive_ike_keys procedure of the new keymat as-is without
 * any processing.
 */
struct isa_info_t {

	/**
	 * Parent isa context id.
	 */
	isa_id_type parent_isa_id;

	/**
	 * Authenticated endpoint context id.
	 */
	ae_id_type ae_id;

};

typedef struct sign_info_t sign_info_t;

/**
 * AUTH signature info data structure.
 *
 * This type is used to transfer an ISA context id and the initial message
 * from the keymat to the TKM private key sign operation. For this purpose the
 * auth octets chunk is (ab)used and the data is stored in this chunk.
 * This is possible since the auth octets are treated as opaque value and handed
 * to the private key sign function as-is without any processing.
 */
struct sign_info_t {

	/**
	 * ISA context id.
	 */
	isa_id_type isa_id;

	/**
	 * Init message.
	 */
	chunk_t init_message;

};

#endif /** TKM_TYPES_H_ @}*/
