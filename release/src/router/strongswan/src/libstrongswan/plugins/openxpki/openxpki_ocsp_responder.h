/*
 * Copyright (C) 2023 Andreas Steffen, strongSec GmbH
 *
 * Copyright (C) secunet Security Networks AG
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
 * @defgroup openxpki_ocsp_responder openxpki_ocsp_responder
 * @{ @ingroup openxpki_p
 */

#ifndef OPENXPKI_OCSP_RESPONDER_H_
#define OPENXPKI_OCSP_RESPONDER_H_

typedef struct openxpki_ocsp_responder_t openxpki_ocsp_responder_t;

#include <credentials/ocsp_responder.h>

/**
 * OCSP responder implementation using OpenXPKI.
 */
struct openxpki_ocsp_responder_t {

	/**
	 * Implements ocsp_responder interface
	 */
	ocsp_responder_t interface;
};

/**
 * Create a openxpki_ocsp_responder instance.
 */
ocsp_responder_t *openxpki_ocsp_responder_create();

#endif /** OPENXPKI_OCSP_RESPONDER_H_ @}*/
