/*
 * Copyright (C) 2011 Sansar Choinyambuu
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
 * @defgroup pts_req_func_comp_evid pts_req_func_comp_evid
 * @{ @ingroup pts
 */

#ifndef PTS_REQ_FUNC_COMP_EVID_H_
#define PTS_REQ_FUNC_COMP_EVID_H_

typedef enum pts_req_func_comp_evid_t pts_req_func_comp_evid_t;

#include <library.h>

/**
 * PTS Request Functional Component Evidence Flags
 */
enum pts_req_func_comp_evid_t {
	/** Transitive Trust Chain flag */
	PTS_REQ_FUNC_COMP_EVID_TTC =			(1<<7),
	/** Verify Component flag */
	PTS_REQ_FUNC_COMP_EVID_VER =			(1<<6),
	/** Current Evidence flag */
	PTS_REQ_FUNC_COMP_EVID_CURR =			(1<<5),
	/** PCR Information flag */
	PTS_REQ_FUNC_COMP_EVID_PCR =			(1<<4),
};

#endif /** PTS_FUNCT_COMP_EVID_REQ_H_ @}*/
