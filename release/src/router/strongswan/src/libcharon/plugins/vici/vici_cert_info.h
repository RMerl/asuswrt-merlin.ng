/*
 * Copyright (C) 2015 Andreas Steffen
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
 * @defgroup vici_cert_info vici_cert_info
 * @{ @ingroup vici
 */

#ifndef VICI_CERT_INFO_H_
#define VICI_CERT_INFO_H_

typedef struct vici_cert_info_t vici_cert_info_t;

#include <credentials/certificates/certificate.h>
#include <credentials/certificates/x509.h>

bool vici_cert_info_from_str(char *type_str, certificate_type_t *type,
							 x509_flag_t *flag);

#endif /** VICI_CERT_INFO_H_ @}*/
