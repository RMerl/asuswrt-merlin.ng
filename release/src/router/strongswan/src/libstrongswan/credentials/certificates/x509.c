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

#include "x509.h"

ENUM_BEGIN(x509_flag_names, X509_NONE, X509_AA,
	"NONE",
	"CA",
	"AA");
ENUM_NEXT(x509_flag_names, X509_OCSP_SIGNER, X509_OCSP_SIGNER, X509_AA,
	"OCSP");
ENUM_NEXT(x509_flag_names, X509_ANY, X509_ANY, X509_OCSP_SIGNER,
	"ANY");
ENUM_END(x509_flag_names, X509_ANY);

/*
 * Described in header
 */
void x509_cdp_destroy(x509_cdp_t *this)
{
	free(this->uri);
	DESTROY_IF(this->issuer);
	free(this);
}
