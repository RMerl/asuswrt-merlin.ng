/*
 * Copyright (C) 2012 Martin Willi
 * Copyright (C) 2012 revosec AG
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

#include "container.h"

ENUM(container_type_names, CONTAINER_PKCS7, CONTAINER_PKCS12,
	"PKCS7",
	"PKCS7_DATA",
	"PKCS7_SIGNED_DATA",
	"PKCS7_ENVELOPED_DATA",
	"PKCS7_ENCRYPTED_DATA",
	"PKCS12",
);
