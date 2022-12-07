/*
 * Copyright (C) 2011-2022 Andreas Steffen
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
 * @defgroup libimcv libimcv
 *
 * @defgroup generic_attr generic_attr
 * @ingroup libimcv
 *
 * @defgroup libimcv_imc imc
 * @ingroup libimcv
 *
 * @defgroup libimcv_imv imv
 * @ingroup libimcv
 *
 * @defgroup pa_tnc pa_tnc
 * @ingroup libimcv
 *
 * @defgroup libimcv_plugins plugins
 * @ingroup libimcv
 *
 * @defgroup libimcv_seg seg
 * @ingroup libimcv
 *
 * @defgroup libimcv_swima swima
 * @ingroup libimcv
 *
 * @addtogroup libimcv
 * @{
 */

#ifndef IMCV_H_
#define IMCV_H_

#include "pa_tnc/pa_tnc_attr_manager.h"
#include "imv/imv_database.h"
#include "imv/imv_session_manager.h"
#include "pts/components/pts_component_manager.h"

#include <library.h>

/**
 * Initialize libimcv.
 *
 * @param is_imv		TRUE if called by IMV, FALSE if by IMC
 * @return				FALSE if initialization failed
 */
bool libimcv_init(bool is_imv);

/**
 * Deinitialize libimcv.
 */
void libimcv_deinit(void);

/**
 * List PA-TNC attribute type and namespace (vendor ID)
 *
 * @param label         label to be used (creating/processing)
 * @param vendor_id     namespace defined by Private Enterprise Number (PEN)
 * @param type          PA-TNC attribute type
 */
void imcv_list_pa_tnc_attribute_type(char *label, pen_t vendor_id, uint32_t type);

/**
 * PA-TNC attribute manager
 */
extern pa_tnc_attr_manager_t* imcv_pa_tnc_attributes;

/**
 * Global IMV database object
 */
extern imv_database_t* imcv_db;

/**
 * Global IMV session manager
 */
extern imv_session_manager_t* imcv_sessions;

/**
 * PTS Functional Component manager
 */
extern pts_component_manager_t* imcv_pts_components;

#endif /** IMCV_H_ @}*/
