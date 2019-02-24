/*
 * Copyright (C) 2014 Andreas Steffen
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

#include "bliss_huffman_code.h"

extern bliss_huffman_code_t bliss_huffman_code_1;
extern bliss_huffman_code_t bliss_huffman_code_3;
extern bliss_huffman_code_t bliss_huffman_code_4;

/**
 * See header.
 */
bliss_huffman_code_t* bliss_huffman_code_get_by_id(bliss_param_set_id_t id)
{
	switch (id)
	{
		case BLISS_I:
		case BLISS_B_I:
			return &bliss_huffman_code_1;
		case BLISS_III:
		case BLISS_B_III:
			return &bliss_huffman_code_3;
		case BLISS_IV:
		case BLISS_B_IV:
			return &bliss_huffman_code_4;
		default:
			return NULL;
	}
}

