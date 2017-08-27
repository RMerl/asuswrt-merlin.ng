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
 * @defgroup pts_simple_evid_final pts_rsimple_evid_final
 * @{ @ingroup pts
 */

#ifndef PTS_SIMPLE_EVID_FINAL_H_
#define PTS_SIMPLE_EVID_FINAL_H_

typedef enum pts_simple_evid_final_flag_t pts_simple_evid_final_flag_t;

#include <library.h>

/**
 * PTS Simple Evidence Final Flags
 */
enum pts_simple_evid_final_flag_t {
	/** TPM PCR Composite and TPM Quote Signature not included   */
	PTS_SIMPLE_EVID_FINAL_NO =						0x00,
	/** TPM PCR Composite and TPM Quote Signature included
	  * using TPM_QUOTE_INFO                                     */
	PTS_SIMPLE_EVID_FINAL_QUOTE_INFO =			 	0x40,
	/** TPM PCR Composite and TPM Quote Signature included
	  * using TPM_QUOTE_INFO2, TPM_CAP_VERSION_INFO not appended */
	PTS_SIMPLE_EVID_FINAL_QUOTE_INFO2 =				0x80,
	/** TPM PCR Composite and TPM Quote Signature included
	  * using TPM_QUOTE_INFO2, TPM_CAP_VERSION_INFO appended     */
	PTS_SIMPLE_EVID_FINAL_QUOTE_INFO2_CAP_VER =	 	0xC0,
    /** Evidence Signature included                              */
	PTS_SIMPLE_EVID_FINAL_EVID_SIG =				0x20,
};

#endif /** PTS_SIMPLE_EVID_FINAL_H_ @}*/
