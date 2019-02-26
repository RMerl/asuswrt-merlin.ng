/*
 * Copyright (C) 2012 Tobias Brunner
 * Copyright (C) 2010 Martin Willi
 * Copyright (C) 2010 revosec AG
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
 * @defgroup leap eap
 * @{ @ingroup libstrongswan
 */

#ifndef EAP_H_
#define EAP_H_

typedef enum eap_code_t eap_code_t;
typedef enum eap_type_t eap_type_t;
typedef struct eap_vendor_type_t eap_vendor_type_t;

#include <library.h>

/**
 * EAP code, type of an EAP message
 */
enum eap_code_t {
	EAP_REQUEST = 1,
	EAP_RESPONSE = 2,
	EAP_SUCCESS = 3,
	EAP_FAILURE = 4,
};

/**
 * enum names for eap_code_t.
 */
extern enum_name_t *eap_code_names;

/**
 * short string enum names for eap_code_t.
 */
extern enum_name_t *eap_code_short_names;

/**
 * EAP types, defines the EAP method implementation
 */
enum eap_type_t {
	EAP_IDENTITY = 1,
	EAP_NOTIFICATION = 2,
	EAP_NAK = 3,
	EAP_MD5 = 4,
	EAP_OTP = 5,
	EAP_GTC = 6,
	EAP_TLS = 13,
	EAP_SIM = 18,
	EAP_TTLS = 21,
	EAP_AKA = 23,
	EAP_PEAP = 25,
	EAP_MSCHAPV2 = 26,
	EAP_MSTLV = 33,
	EAP_TNC = 38,
	EAP_PT_EAP = 54,
	EAP_EXPANDED = 254,
	EAP_EXPERIMENTAL = 255,
	/** not a method, but an implementation providing different methods */
	EAP_RADIUS = 256,
	/** not a method, select method dynamically based on client selection */
	EAP_DYNAMIC = 257,
};

/**
 * enum names for eap_type_t.
 */
extern enum_name_t *eap_type_names;

/**
 * short string enum names for eap_type_t.
 */
extern enum_name_t *eap_type_short_names;

/**
 * Struct that stores EAP type and vendor ID
 */
struct eap_vendor_type_t {

	/**
	 * EAP type
	 */
	eap_type_t type;

	/**
	 * Vendor Id
	 */
	uint32_t vendor;
};

/**
 * EAP packet format
 */
typedef struct __attribute__((packed)) {
	uint8_t code;
	uint8_t identifier;
	uint16_t length;
	uint8_t type;
	uint8_t data;
} eap_packet_t;

/**
 * Lookup the EAP method type from a string.
 *
 * @param name		EAP method name (such as "md5", "aka")
 * @return			method type, 0 if unknown
 */
eap_type_t eap_type_from_string(char *name);

/**
 * Parse a string of the form [eap-]type[-vendor].
 *
 * @param str		EAP method string
 * @return			parsed type (gets allocated), NULL if unknown or failed
 */
eap_vendor_type_t *eap_vendor_type_from_string(char *str);

#endif /** EAP_H_ @}*/
