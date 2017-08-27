/*
 * Copyright (C) 2010 Tobias Brunner
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
 * @defgroup libhydra libhydra
 *
 * @defgroup attributes attributes
 * @ingroup libhydra
 *
 * @defgroup hkernel kernel
 * @ingroup libhydra
 *
 * @defgroup hplugins plugins
 * @ingroup libhydra
 *
 * @addtogroup libhydra
 * @{
 */

#ifndef HYDRA_H_
#define HYDRA_H_

typedef struct hydra_t hydra_t;

#include <attributes/attribute_manager.h>
#include <kernel/kernel_interface.h>

#include <library.h>

/**
 * IKE Daemon support object.
 */
struct hydra_t {

	/**
	 * manager for payload attributes
	 */
	attribute_manager_t *attributes;

	/**
	 * kernel interface to communicate with kernel
	 */
	kernel_interface_t *kernel_interface;
};

/**
 * The single instance of hydra_t.
 *
 * Set between calls to libhydra_init() and libhydra_deinit() calls.
 */
extern hydra_t *hydra;

/**
 * Initialize libhydra.
 *
 * libhydra_init() may be called multiple times in a single process, but each
 * caller must call libhydra_deinit() for each call to libhydra_init().
 *
 * @return				FALSE if integrity check failed
 */
bool libhydra_init();

/**
 * Deinitialize libhydra.
 */
void libhydra_deinit();

#endif /** HYDRA_H_ @}*/
