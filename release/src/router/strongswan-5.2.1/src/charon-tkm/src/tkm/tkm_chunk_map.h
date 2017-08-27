/*
 * Copyright (C) 2012 Reto Buerki
 * Copyright (C) 2012 Adrian-Ken Rueegsegger
 * Hochschule fuer Technik Rapperswil
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
 * @defgroup tkm-chunk-map chunk map
 * @{ @ingroup tkm
 */

#ifndef TKM_CHUNK_MAP_H_
#define TKM_CHUNK_MAP_H_

#include <stdint.h>
#include <utils/chunk.h>

typedef struct tkm_chunk_map_t tkm_chunk_map_t;

/**
 * The tkm chunk map handles mappings of chunks to ids.
 */
struct tkm_chunk_map_t {

	/**
	 * Store new mapping for given chunk and id.
	 *
	 * @param data	data associated with id
	 * @param id	id associated with data
	 */
	void (*insert)(tkm_chunk_map_t * const this, const chunk_t * const data,
				   const uint64_t id);

	/**
	 * Get id for given chunk.
	 *
	 * @param data	data specifying the mapping
	 * @return		id of given chunk, 0 if not found
	 */
	uint64_t (*get_id)(tkm_chunk_map_t * const this, chunk_t *data);

	/**
	 * Remove mapping for given chunk.
	 *
	 * @param data	data specifying the mapping to remove
	 * @return		TRUE if mapping was removed, FALSE otherwise
	 */
	bool (*remove)(tkm_chunk_map_t * const this, chunk_t *data);

	/**
	 * Destroy a tkm chunk map instance.
	 */
	void (*destroy)(tkm_chunk_map_t *this);

};

/**
 * Create a tkm chunk map instance.
 */
tkm_chunk_map_t *tkm_chunk_map_create();

#endif /** TKM_CHUNK_MAP_H_ @}*/
