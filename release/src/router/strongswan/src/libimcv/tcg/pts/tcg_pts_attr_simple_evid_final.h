/*
 * Copyright (C) 2011 Sansar Choinyambuu
 * Copyright (C) 2014-2016 Andreas Steffen
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
 * @defgroup tcg_pts_attr_simple_evid_final tcg_pts_attr_simple_evid_final
 * @{ @ingroup tcg_attr
 */

#ifndef TCG_PTS_ATTR_SIMPLE_EVID_FINAL_H_
#define TCG_PTS_ATTR_SIMPLE_EVID_FINAL_H_

typedef struct tcg_pts_attr_simple_evid_final_t tcg_pts_attr_simple_evid_final_t;

#include "tcg/tcg_attr.h"
#include "tcg_pts_attr_meas_algo.h"
#include "pa_tnc/pa_tnc_attr.h"

#include <tpm_tss_quote_info.h>

/**
 * Class implementing the TCG PTS Simple Evidence Final attribute
 *
 */
struct tcg_pts_attr_simple_evid_final_t {

	/**
	 * Public PA-TNC attribute interface
	 */
	pa_tnc_attr_t pa_tnc_attribute;

	/**
	 * Get Optional TPM Quote Info and TPM Quote Signature
	 *
	 * @param quote_info		Optional TPM Quote Info
	 * @param quote sig			Optional TPM Quote Signature
	 */
	void (*get_quote_info)(tcg_pts_attr_simple_evid_final_t *this,
						   tpm_tss_quote_info_t **quote_info,
						   chunk_t *quote_sig);

	/**
	 * Get Optional Evidence Signature
	 *
	 * @param evid_sig			Optional Evidence Signature
	 * @return					TRUE if Evidence Signature is available
	 */
	bool (*get_evid_sig)(tcg_pts_attr_simple_evid_final_t *this,
						 chunk_t *evid_sig);

	/**
	 * Set Optional Evidence Signature
	 *
	 * @param vid_sig			Optional Evidence Signature
	 */
	void (*set_evid_sig)(tcg_pts_attr_simple_evid_final_t *this,
						 chunk_t evid_sig);

};

/**
 * Creates an tcg_pts_attr_simple_evid_final_t object
 *
 * @param quote_info			Optional TPM Quote Info
 * @param quote_sig				Optional TPM Quote Signature
 */
pa_tnc_attr_t* tcg_pts_attr_simple_evid_final_create(
						tpm_tss_quote_info_t *quote_info, chunk_t quote_sig);

/**
 * Creates an tcg_pts_attr_simple_evid_final_t object from received data
 *
 * @param length			Total length of attribute value
 * @param value				Unparsed attribute value (might be a segment)
 */
pa_tnc_attr_t* tcg_pts_attr_simple_evid_final_create_from_data(size_t length,
															   chunk_t value);

#endif /** TCG_PTS_ATTR_SIMPLE_EVID_FINAL_H_ @}*/
