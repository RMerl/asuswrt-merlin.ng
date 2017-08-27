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

#include "tcg_attr.h"
#include "tcg/pts/tcg_pts_attr_proto_caps.h"
#include "tcg/pts/tcg_pts_attr_dh_nonce_params_req.h"
#include "tcg/pts/tcg_pts_attr_dh_nonce_params_resp.h"
#include "tcg/pts/tcg_pts_attr_dh_nonce_finish.h"
#include "tcg/pts/tcg_pts_attr_meas_algo.h"
#include "tcg/pts/tcg_pts_attr_get_tpm_version_info.h"
#include "tcg/pts/tcg_pts_attr_tpm_version_info.h"
#include "tcg/pts/tcg_pts_attr_get_aik.h"
#include "tcg/pts/tcg_pts_attr_aik.h"
#include "tcg/pts/tcg_pts_attr_req_func_comp_evid.h"
#include "tcg/pts/tcg_pts_attr_gen_attest_evid.h"
#include "tcg/pts/tcg_pts_attr_simple_comp_evid.h"
#include "tcg/pts/tcg_pts_attr_simple_evid_final.h"
#include "tcg/pts/tcg_pts_attr_req_file_meas.h"
#include "tcg/pts/tcg_pts_attr_file_meas.h"
#include "tcg/pts/tcg_pts_attr_req_file_meta.h"
#include "tcg/pts/tcg_pts_attr_unix_file_meta.h"
#include "tcg/swid/tcg_swid_attr_req.h"
#include "tcg/swid/tcg_swid_attr_tag_id_inv.h"
#include "tcg/swid/tcg_swid_attr_tag_inv.h"
#include "tcg/seg/tcg_seg_attr_max_size.h"
#include "tcg/seg/tcg_seg_attr_seg_env.h"
#include "tcg/seg/tcg_seg_attr_next_seg.h"

ENUM_BEGIN(tcg_attr_names,	TCG_SCAP_REFERENCES,
							TCG_SCAP_SUMMARY_RESULTS,
	"SCAP References",
	"SCAP Capabilities and Inventory",
	"SCAP Content",
	"SCAP Assessment",
	"SCAP Results",
	"SCAP Summary Results");
ENUM_NEXT(tcg_attr_names,	TCG_SWID_REQUEST,
							TCG_SWID_TAG_EVENTS,
							TCG_SCAP_SUMMARY_RESULTS,
	"SWID Request",
	"SWID Tag Identifier Inventory",
	"SWID Tag Identifier Events",
	"SWID Tag Inventory",
	"SWID Tag Events");
ENUM_NEXT(tcg_attr_names,	TCG_SEG_MAX_ATTR_SIZE_REQ,
							TCG_SEG_CANCEL_SEG_EXCH,
							TCG_SWID_TAG_EVENTS,
	"Max Attribute Size Request",
	"Max Attribute Size Response",
	"Attribute Segment Envelope",
	"Next Segment Request",
	"Cancel Segment Exchange");
ENUM_NEXT(tcg_attr_names,	TCG_PTS_REQ_FUNC_COMP_EVID,
							TCG_PTS_REQ_FUNC_COMP_EVID,
							TCG_SEG_CANCEL_SEG_EXCH,
	"Request Functional Component Evidence");
ENUM_NEXT(tcg_attr_names,	TCG_PTS_GEN_ATTEST_EVID,
							TCG_PTS_GEN_ATTEST_EVID,
							TCG_PTS_REQ_FUNC_COMP_EVID,
	"Generate Attestation Evidence");
ENUM_NEXT(tcg_attr_names,	TCG_PTS_SIMPLE_COMP_EVID,
							TCG_PTS_SIMPLE_COMP_EVID,
							TCG_PTS_GEN_ATTEST_EVID,
	"Simple Component Evidence");
ENUM_NEXT(tcg_attr_names,	TCG_PTS_SIMPLE_EVID_FINAL,
							TCG_PTS_SIMPLE_EVID_FINAL,
							TCG_PTS_SIMPLE_COMP_EVID,
	"Simple Evidence Final");
ENUM_NEXT(tcg_attr_names,	TCG_PTS_VERIFICATION_RESULT,
							TCG_PTS_VERIFICATION_RESULT,
							TCG_PTS_SIMPLE_EVID_FINAL,
	"Verification Result");
ENUM_NEXT(tcg_attr_names,	TCG_PTS_INTEG_REPORT,
							TCG_PTS_INTEG_REPORT,
							TCG_PTS_VERIFICATION_RESULT,
	"Integrity Report");
ENUM_NEXT(tcg_attr_names,	TCG_PTS_REQ_FILE_META,
							TCG_PTS_REQ_FILE_META,
							TCG_PTS_INTEG_REPORT,
	"Request File Metadata");
ENUM_NEXT(tcg_attr_names,	TCG_PTS_WIN_FILE_META,
							TCG_PTS_WIN_FILE_META,
							TCG_PTS_REQ_FILE_META,
	"Windows-Style File Metadata");
ENUM_NEXT(tcg_attr_names,	TCG_PTS_UNIX_FILE_META,
							TCG_PTS_UNIX_FILE_META,
							TCG_PTS_WIN_FILE_META,
	"Unix-Style File Metadata");
ENUM_NEXT(tcg_attr_names,	TCG_PTS_REQ_REGISTRY_VALUE,
							TCG_PTS_REQ_REGISTRY_VALUE,
							TCG_PTS_UNIX_FILE_META,
	"Request Registry Value");
ENUM_NEXT(tcg_attr_names,	TCG_PTS_REGISTRY_VALUE,
							TCG_PTS_REGISTRY_VALUE,
							TCG_PTS_REQ_REGISTRY_VALUE,
	"Registry Value");
ENUM_NEXT(tcg_attr_names,	TCG_PTS_REQ_FILE_MEAS,
							TCG_PTS_REQ_FILE_MEAS,
							TCG_PTS_REGISTRY_VALUE,
	"Request File Measurement");
ENUM_NEXT(tcg_attr_names,	TCG_PTS_FILE_MEAS,
							TCG_PTS_FILE_MEAS,
							TCG_PTS_REQ_FILE_MEAS,
	"File Measurement");
ENUM_NEXT(tcg_attr_names,	TCG_PTS_REQ_INTEG_MEAS_LOG,
							TCG_PTS_REQ_INTEG_MEAS_LOG,
							TCG_PTS_FILE_MEAS,
	"Request Integrity Measurement Log");
ENUM_NEXT(tcg_attr_names,	TCG_PTS_INTEG_MEAS_LOG,
							TCG_PTS_INTEG_MEAS_LOG,
							TCG_PTS_REQ_INTEG_MEAS_LOG,
	"Integrity Measurement Log");
ENUM_NEXT(tcg_attr_names,	TCG_PTS_REQ_PROTO_CAPS,
							TCG_PTS_REQ_PROTO_CAPS,
							TCG_PTS_INTEG_MEAS_LOG,
	"Request PTS Protocol Capabilities");
ENUM_NEXT(tcg_attr_names,	TCG_PTS_PROTO_CAPS,
							TCG_PTS_PROTO_CAPS,
							TCG_PTS_REQ_PROTO_CAPS,
	"PTS Protocol Capabilities");
ENUM_NEXT(tcg_attr_names,	TCG_PTS_DH_NONCE_PARAMS_REQ,
							TCG_PTS_DH_NONCE_PARAMS_REQ,
							TCG_PTS_PROTO_CAPS,
	"DH Nonce Parameters Request");
ENUM_NEXT(tcg_attr_names,	TCG_PTS_DH_NONCE_PARAMS_RESP,
							TCG_PTS_DH_NONCE_PARAMS_RESP,
							TCG_PTS_DH_NONCE_PARAMS_REQ,
	"DH Nonce Parameters Response");
ENUM_NEXT(tcg_attr_names,	TCG_PTS_DH_NONCE_FINISH,
							TCG_PTS_DH_NONCE_FINISH,
							TCG_PTS_DH_NONCE_PARAMS_RESP,
	"DH Nonce Finish");
ENUM_NEXT(tcg_attr_names,	TCG_PTS_MEAS_ALGO,
							TCG_PTS_MEAS_ALGO,
							TCG_PTS_DH_NONCE_FINISH,
	"PTS Measurement Algorithm Request");
ENUM_NEXT(tcg_attr_names,	TCG_PTS_MEAS_ALGO_SELECTION,
							TCG_PTS_MEAS_ALGO_SELECTION,
							TCG_PTS_MEAS_ALGO,
	"PTS Measurement Algorithm");
ENUM_NEXT(tcg_attr_names,	TCG_PTS_GET_TPM_VERSION_INFO,
							TCG_PTS_GET_TPM_VERSION_INFO,
							TCG_PTS_MEAS_ALGO_SELECTION,
	"Get TPM Version Information");
ENUM_NEXT(tcg_attr_names,	TCG_PTS_TPM_VERSION_INFO,
							TCG_PTS_TPM_VERSION_INFO,
							TCG_PTS_GET_TPM_VERSION_INFO,
	"TPM Version Information");
ENUM_NEXT(tcg_attr_names,	TCG_PTS_REQ_TEMPL_REF_MANI_SET_META,
							TCG_PTS_REQ_TEMPL_REF_MANI_SET_META,
							TCG_PTS_TPM_VERSION_INFO,
	"Request Template Reference Manifest Set Metadata");
ENUM_NEXT(tcg_attr_names,	TCG_PTS_TEMPL_REF_MANI_SET_META,
							TCG_PTS_TEMPL_REF_MANI_SET_META,
							TCG_PTS_REQ_TEMPL_REF_MANI_SET_META,
	"Template Reference Manifest Set Metadata");
ENUM_NEXT(tcg_attr_names,	TCG_PTS_UPDATE_TEMPL_REF_MANI,
							TCG_PTS_UPDATE_TEMPL_REF_MANI,
							TCG_PTS_TEMPL_REF_MANI_SET_META,
	"Update Template Reference Manifest");
ENUM_NEXT(tcg_attr_names,	TCG_PTS_GET_AIK,
							TCG_PTS_GET_AIK,
							TCG_PTS_UPDATE_TEMPL_REF_MANI,
	"Get Attestation Identity Key");
ENUM_NEXT(tcg_attr_names,	TCG_PTS_AIK,
							TCG_PTS_AIK,
							TCG_PTS_GET_AIK,
	"Attestation Identity Key");
ENUM_END(tcg_attr_names,	TCG_PTS_AIK);

/**
 * See header
 */
pa_tnc_attr_t* tcg_attr_create_from_data(u_int32_t type, size_t length, chunk_t value)
{
	switch (type)
	{
		case TCG_SWID_REQUEST:
			return tcg_swid_attr_req_create_from_data(length, value);
		case TCG_SWID_TAG_ID_INVENTORY:
			return tcg_swid_attr_tag_id_inv_create_from_data(length, value);
		case TCG_SWID_TAG_INVENTORY:
			return tcg_swid_attr_tag_inv_create_from_data(length, value);
		case TCG_SEG_MAX_ATTR_SIZE_REQ:
			return tcg_seg_attr_max_size_create_from_data(length, value, TRUE);
		case TCG_SEG_MAX_ATTR_SIZE_RESP:
			return tcg_seg_attr_max_size_create_from_data(length, value, FALSE);
		case TCG_SEG_ATTR_SEG_ENV:
			return tcg_seg_attr_seg_env_create_from_data(length, value);
		case TCG_SEG_NEXT_SEG_REQ:
			return tcg_seg_attr_next_seg_create_from_data(length, value);
		case TCG_PTS_REQ_PROTO_CAPS:
			return tcg_pts_attr_proto_caps_create_from_data(length, value,
															TRUE);
		case TCG_PTS_PROTO_CAPS:
			return tcg_pts_attr_proto_caps_create_from_data(length, value,
															FALSE);
		case TCG_PTS_DH_NONCE_PARAMS_REQ:
			return tcg_pts_attr_dh_nonce_params_req_create_from_data(length,
																	 value);
		case TCG_PTS_DH_NONCE_PARAMS_RESP:
			return tcg_pts_attr_dh_nonce_params_resp_create_from_data(length,
																	  value);
		case TCG_PTS_DH_NONCE_FINISH:
			return tcg_pts_attr_dh_nonce_finish_create_from_data(length, value);
		case TCG_PTS_MEAS_ALGO:
			return tcg_pts_attr_meas_algo_create_from_data(length, value,
														   FALSE);
		case TCG_PTS_MEAS_ALGO_SELECTION:
			return tcg_pts_attr_meas_algo_create_from_data(length, value,
														   TRUE);
		case TCG_PTS_GET_TPM_VERSION_INFO:
			return tcg_pts_attr_get_tpm_version_info_create_from_data(length,
																	  value);
		case TCG_PTS_TPM_VERSION_INFO:
			return tcg_pts_attr_tpm_version_info_create_from_data(length,
																  value);
		case TCG_PTS_GET_AIK:
			return tcg_pts_attr_get_aik_create_from_data(length, value);
		case TCG_PTS_AIK:
			return tcg_pts_attr_aik_create_from_data(length, value);
		case TCG_PTS_REQ_FUNC_COMP_EVID:
			return tcg_pts_attr_req_func_comp_evid_create_from_data(length,
																	value);
		case TCG_PTS_GEN_ATTEST_EVID:
			return tcg_pts_attr_gen_attest_evid_create_from_data(length, value);
		case TCG_PTS_SIMPLE_COMP_EVID:
			return tcg_pts_attr_simple_comp_evid_create_from_data(length,
																  value);
		case TCG_PTS_SIMPLE_EVID_FINAL:
			return tcg_pts_attr_simple_evid_final_create_from_data(length,
																   value);
		case TCG_PTS_REQ_FILE_MEAS:
			return tcg_pts_attr_req_file_meas_create_from_data(length, value);
		case TCG_PTS_FILE_MEAS:
			return tcg_pts_attr_file_meas_create_from_data(length, value);
		case TCG_PTS_REQ_FILE_META:
			return tcg_pts_attr_req_file_meta_create_from_data(length, value);
		case TCG_PTS_UNIX_FILE_META:
			return tcg_pts_attr_unix_file_meta_create_from_data(length, value);
		/* unsupported TCG/SWID attributes */
		case TCG_SWID_TAG_ID_EVENTS:
		case TCG_SWID_TAG_EVENTS:
		/* unsupported TCG/PTS attributes */
		case TCG_PTS_REQ_TEMPL_REF_MANI_SET_META:
		case TCG_PTS_TEMPL_REF_MANI_SET_META:
		case TCG_PTS_UPDATE_TEMPL_REF_MANI:
		case TCG_PTS_VERIFICATION_RESULT:
		case TCG_PTS_INTEG_REPORT:
		case TCG_PTS_WIN_FILE_META:
		case TCG_PTS_REQ_REGISTRY_VALUE:
		case TCG_PTS_REGISTRY_VALUE:
		case TCG_PTS_REQ_INTEG_MEAS_LOG:
		case TCG_PTS_INTEG_MEAS_LOG:
		default:
			return NULL;
	}
}
