/*
 * Copyright (C) 2013 Tobias Brunner
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

#ifndef IV_GEN_SEQ_H_
#define IV_GEN_SEQ_H_

#include <crypto/iv/iv_gen.h>

/**
 * Create an IV generator that generates sequential IVs (counter).
 *
 * The passed external IV must be larger than the one passed to any previous
 * call.
 *
 * @return		IV generator
 */
iv_gen_t *iv_gen_seq_create();

#endif /** IV_GEN_SEQ_H_ @}*/
