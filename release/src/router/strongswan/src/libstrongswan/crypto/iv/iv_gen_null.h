/*
 * Copyright (C) 2015 Tobias Brunner
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
 * @{ @ingroup iv
 */

#ifndef IV_GEN_NULL_H_
#define IV_GEN_NULL_H_

#include <crypto/iv/iv_gen.h>

/**
 * Create an IV generator that does not actually generate an IV.
 *
 * @return		IV generator
 */
iv_gen_t *iv_gen_null_create();

#endif /** IV_GEN_NULL_H_ @}*/
