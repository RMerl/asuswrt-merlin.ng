/*
 * Copyright (C) 2020 Andreas Steffen
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
 * @defgroup pts_symlinks pts_symlinks
 * @{ @ingroup pts
 */

#ifndef PTS_SYMLINKS_H_
#define PTS_SYMLINKS_H_

#include <library.h>

typedef struct pts_symlinks_t pts_symlinks_t;


/**
 * Class storing a list of symbolic links pointing to directories
 */
struct pts_symlinks_t {

	/**
	 * Get the number of symbolic link entries
	 *
	 * @return				Number of symbolic links
	 */
	int (*get_count)(pts_symlinks_t *this);

	/**
	 * Add a symbolic link pointing to a directory
	 *
	 * @param symlink		Pathname of symbolic link
	 * @param dir		    Pathname of directory the symlink points to
	 */
	void (*add)(pts_symlinks_t *this, chunk_t symlink, chunk_t dir);

	/**
	  * Create a symlink enumerator
	  *
	  * @return				Enumerator returning (chunk_t symlink, chunk_t dir)
	  */
	enumerator_t* (*create_enumerator)(pts_symlinks_t *this);

	/**
	 * Get a new reference to the list of symbolic links
	 *
	 * @return			this, with an increased refcount
	 */
	pts_symlinks_t* (*get_ref)(pts_symlinks_t *this);


	/**
	 * Destroys a pts_symlinks_t object.
	 */
	void (*destroy)(pts_symlinks_t *this);

};

/**
 * Creates a pts_symlinks_t object
 */
pts_symlinks_t* pts_symlinks_create();

#endif /** PTS_SYMLINKS_H_ @}*/
