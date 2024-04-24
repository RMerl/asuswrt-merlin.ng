/*
 * Copyright (C) 2023 Tobias Brunner
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
 * @defgroup ocsp ocsp
 * @{ @ingroup pki
 */

#ifndef INDEX_RESPONDER_H_
#define INDEX_RESPONDER_H_

#include <credentials/ocsp_responder.h>

/**
 * Create an index.txt-based OCSP responder for the given CA and file.
 *
 * On success, the responder is automatically registered until destroyed.
 *
 * @param ca			CA certificate (referenced)
 * @param path			path to index.txt
 * @return				OCSP responder, NULL if file is invalid
 */
ocsp_responder_t *index_responder_create(certificate_t *ca, char *path);

#endif /** INDEX_RESPONDER_H_ @} */
