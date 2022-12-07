/*
 * Copyright (C) 2017 Andreas Steffen
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
 * @defgroup imv_swima_agent_t imv_swima_agent
 * @{ @ingroup imv_swima
 */

#ifndef IMV_SWIMA_AGENT_H_
#define IMV_SWIMA_AGENT_H_

#include <imv/imv_agent_if.h>

/**
 * Creates an SWID IMV agent
 *
 * @param name					Name of the IMV
 * @param id					ID of the IMV
 * @param actual_version		TNC IF-IMV version
 */
imv_agent_if_t* imv_swima_agent_create(const char* name, TNC_IMVID id,
									   TNC_Version *actual_version);

#endif /** IMV_SWIMA_AGENT_H_ @}*/
