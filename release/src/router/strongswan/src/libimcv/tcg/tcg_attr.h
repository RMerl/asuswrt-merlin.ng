/*
 * Copyright (C) 2011-2014 Andreas Steffen
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
 * @defgroup tcg_attr tcg_attr
 * @{ @ingroup libimcv
 */

#ifndef TCG_ATTR_H_
#define TCG_ATTR_H_

#include <pa_tnc/pa_tnc_attr.h>
#include <library.h>

typedef enum tcg_attr_t tcg_attr_t;

/**
 * TCG PTS IF-M Attributes (section 4 of PTS PROTO: Binding to TNC IF-M)
 */
enum tcg_attr_t {

	/* SCAP Attributes */
	TCG_SCAP_REFERENCES =                 0x00000001,
	TCG_SCAP_CAPS_AND_INVENTORY =         0x00000002,
	TCG_SCAP_CONTENT =                    0x00000003,
	TCG_SCAP_ASSESSMENT =                 0x00000004,
	TCG_SCAP_RESULTS =                    0x00000005,
	TCG_SCAP_SUMMARY_RESULTS =            0x00000006,

	/* SWID Attributes */
	TCG_SWID_REQUEST =                    0x00000011,
	TCG_SWID_TAG_ID_INVENTORY =           0x00000012,
	TCG_SWID_TAG_ID_EVENTS =              0x00000013,
	TCG_SWID_TAG_INVENTORY =              0x00000014,
	TCG_SWID_TAG_EVENTS =                 0x00000015,
	TCG_SWID_SUBSCRIPTION_STATUS_REQ =    0x00000016,
	TCG_SWID_SUBSCRIPTION_STATUS_RESP =   0x00000017,

	/* IF-M Attribute Segmentation */
	TCG_SEG_MAX_ATTR_SIZE_REQ =           0x00000021,
	TCG_SEG_MAX_ATTR_SIZE_RESP =          0x00000022,
	TCG_SEG_ATTR_SEG_ENV =                0x00000023,
	TCG_SEG_NEXT_SEG_REQ =                0x00000024,
	TCG_SEG_CANCEL_SEG_EXCH =             0x00000025,

	/* PTS Protocol Negotiations */
	TCG_PTS_REQ_PROTO_CAPS =              0x01000000,
	TCG_PTS_PROTO_CAPS =                  0x02000000,
	TCG_PTS_DH_NONCE_PARAMS_REQ =         0x03000000,
	TCG_PTS_DH_NONCE_PARAMS_RESP =        0x04000000,
	TCG_PTS_DH_NONCE_FINISH =             0x05000000,
	TCG_PTS_MEAS_ALGO =                   0x06000000,
	TCG_PTS_MEAS_ALGO_SELECTION =         0x07000000,
	TCG_PTS_GET_TPM_VERSION_INFO =        0x08000000,
	TCG_PTS_TPM_VERSION_INFO =            0x09000000,
	TCG_PTS_REQ_TEMPL_REF_MANI_SET_META = 0x0A000000,
	TCG_PTS_TEMPL_REF_MANI_SET_META =     0x0B000000,
	TCG_PTS_UPDATE_TEMPL_REF_MANI =       0x0C000000,
	TCG_PTS_GET_AIK =                     0x0D000000,
	TCG_PTS_AIK =                         0x0E000000,

	/* PTS-based Attestation Evidence */
	TCG_PTS_REQ_FUNC_COMP_EVID =          0x00100000,
	TCG_PTS_GEN_ATTEST_EVID =             0x00200000,
	TCG_PTS_SIMPLE_COMP_EVID =            0x00300000,
	TCG_PTS_SIMPLE_EVID_FINAL =           0x00400000,
	TCG_PTS_VERIFICATION_RESULT =         0x00500000,
	TCG_PTS_INTEG_REPORT =                0x00600000,
	TCG_PTS_REQ_FILE_META =               0x00700000,
	TCG_PTS_WIN_FILE_META =               0x00800000,
	TCG_PTS_UNIX_FILE_META =              0x00900000,
	TCG_PTS_REQ_REGISTRY_VALUE =          0x00A00000,
	TCG_PTS_REGISTRY_VALUE =              0x00B00000,
	TCG_PTS_REQ_FILE_MEAS =               0x00C00000,
	TCG_PTS_FILE_MEAS =                   0x00D00000,
	TCG_PTS_REQ_INTEG_MEAS_LOG =          0x00E00000,
	TCG_PTS_INTEG_MEAS_LOG =              0x00F00000,
};

/**
 * enum name for tcg_attr_t.
 */
extern enum_name_t *tcg_attr_names;

/**
 * Create a TCG PA-TNC attribute from data
 *
 * @param type				attribute type
 * @param length			attribute length
 * @param value				attribute value or segment
 */
pa_tnc_attr_t* tcg_attr_create_from_data(uint32_t type, size_t length,
										 chunk_t value);

#endif /** TCG_ATTR_H_ @}*/
