/*
 * Copyright (C) 2021 Tobias Brunner
 * Copyright (C) 2021 Thomas Egerer
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
 * @defgroup metadata_int metadata_int
 * @{ @ingroup metadata
 */

#ifndef METADATA_INT_H_
#define METADATA_INT_H_

#include "metadata.h"

/**
 * Create a metadata object of an integer type.
 *
 * @param type      type name
 * @param args      integer of the specified type
 * @return          successfully created object, NULL on failure
 */
metadata_t *metadata_create_int(const char *type, va_list args);

#endif /** METADATA_INT_H_ @}*/
