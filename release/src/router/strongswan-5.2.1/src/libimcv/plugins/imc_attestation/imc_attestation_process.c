/*
 * Copyright (C) 2011-2012 Sansar Choinyambuu, Andreas Steffen
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

#define _GNU_SOURCE

#include <stdio.h>
/* for isdigit */
#include <ctype.h>

#include "imc_attestation_process.h"

#include <ietf/ietf_attr_pa_tnc_error.h>

#include <pts/pts.h>

#include <tcg/pts/tcg_pts_attr_proto_caps.h>
#include <tcg/pts/tcg_pts_attr_meas_algo.h>
#include <tcg/pts/tcg_pts_attr_dh_nonce_params_req.h>
#include <tcg/pts/tcg_pts_attr_dh_nonce_params_resp.h>
#include <tcg/pts/tcg_pts_attr_dh_nonce_finish.h>
#include <tcg/pts/tcg_pts_attr_get_tpm_version_info.h>
#include <tcg/pts/tcg_pts_attr_tpm_version_info.h>
#include <tcg/pts/tcg_pts_attr_get_aik.h>
#include <tcg/pts/tcg_pts_attr_aik.h>
#include <tcg/pts/tcg_pts_attr_req_func_comp_evid.h>
#include <tcg/pts/tcg_pts_attr_gen_attest_evid.h>
#include <tcg/pts/tcg_pts_attr_simple_comp_evid.h>
#include <tcg/pts/tcg_pts_attr_simple_evid_final.h>
#include <tcg/pts/tcg_pts_attr_req_file_meas.h>
#include <tcg/pts/tcg_pts_attr_file_meas.h>
#include <tcg/pts/tcg_pts_attr_req_file_meta.h>
#include <tcg/pts/tcg_pts_attr_unix_file_meta.h>

#include <utils/debug.h>
#include <utils/lexparser.h>

#define DEFAULT_NONCE_LEN		20

bool imc_attestation_process(pa_tnc_attr_t *attr, imc_msg_t *msg,
							 imc_attestation_state_t *attestation_state,
							 pts_meas_algorithms_t supported_algorithms,
							 pts_dh_group_t supported_dh_groups)
{
	chunk_t attr_info;
	pts_t *pts;
	pts_error_code_t pts_error;
	pen_type_t attr_type;
	bool valid_path;

	pts = attestation_state->get_pts(attestation_state);
	attr_type = attr->get_type(attr);

	switch (attr_type.type)
	{
		case TCG_PTS_REQ_PROTO_CAPS:
		{
			tcg_pts_attr_proto_caps_t *attr_cast;
			pts_proto_caps_flag_t imc_caps, imv_caps;

			attr_cast = (tcg_pts_attr_proto_caps_t*)attr;
			imv_caps = attr_cast->get_flags(attr_cast);
			imc_caps = pts->get_proto_caps(pts);
			pts->set_proto_caps(pts, imc_caps & imv_caps);

			/* Send PTS Protocol Capabilities attribute */
			attr = tcg_pts_attr_proto_caps_create(imc_caps & imv_caps, FALSE);
			msg->add_attribute(msg, attr);
			break;
		}
		case TCG_PTS_MEAS_ALGO:
		{
			tcg_pts_attr_meas_algo_t *attr_cast;
			pts_meas_algorithms_t offered_algorithms, selected_algorithm;

			attr_cast = (tcg_pts_attr_meas_algo_t*)attr;
			offered_algorithms = attr_cast->get_algorithms(attr_cast);
			selected_algorithm = pts_meas_algo_select(supported_algorithms,
													  offered_algorithms);
			if (selected_algorithm == PTS_MEAS_ALGO_NONE)
			{
				attr = pts_hash_alg_error_create(supported_algorithms);
				msg->add_attribute(msg, attr);
				break;
			}

			/* Send Measurement Algorithm Selection attribute */
			pts->set_meas_algorithm(pts, selected_algorithm);
			attr = tcg_pts_attr_meas_algo_create(selected_algorithm, TRUE);
			msg->add_attribute(msg, attr);
			break;
		}
		case TCG_PTS_DH_NONCE_PARAMS_REQ:
		{
			tcg_pts_attr_dh_nonce_params_req_t *attr_cast;
			pts_dh_group_t offered_dh_groups, selected_dh_group;
			chunk_t responder_value, responder_nonce;
			int nonce_len, min_nonce_len;

			nonce_len = lib->settings->get_int(lib->settings,
								"%s.plugins.imc-attestation.nonce_len",
								 DEFAULT_NONCE_LEN, lib->ns);

			attr_cast = (tcg_pts_attr_dh_nonce_params_req_t*)attr;
			min_nonce_len = attr_cast->get_min_nonce_len(attr_cast);
			if (nonce_len < PTS_MIN_NONCE_LEN ||
				(min_nonce_len > 0 && nonce_len < min_nonce_len))
			{
				attr = pts_dh_nonce_error_create(nonce_len, PTS_MAX_NONCE_LEN);
				msg->add_attribute(msg, attr);
				break;
			}

			offered_dh_groups = attr_cast->get_dh_groups(attr_cast);
			selected_dh_group = pts_dh_group_select(supported_dh_groups,
													offered_dh_groups);
			if (selected_dh_group == PTS_DH_GROUP_NONE)
			{
				attr = pts_dh_group_error_create(supported_dh_groups);
				msg->add_attribute(msg, attr);
				break;
			}

			/* Create own DH factor and nonce */
			if (!pts->create_dh_nonce(pts, selected_dh_group, nonce_len))
			{
				return FALSE;
			}
			pts->get_my_public_value(pts, &responder_value, &responder_nonce);

			/* Send DH Nonce Parameters Response attribute */
			attr = tcg_pts_attr_dh_nonce_params_resp_create(selected_dh_group,
					 supported_algorithms, responder_nonce, responder_value);
			msg->add_attribute(msg, attr);
			break;
		}
		case TCG_PTS_DH_NONCE_FINISH:
		{
			tcg_pts_attr_dh_nonce_finish_t *attr_cast;
			pts_meas_algorithms_t selected_algorithm;
			chunk_t initiator_nonce, initiator_value;
			int nonce_len;

			attr_cast = (tcg_pts_attr_dh_nonce_finish_t*)attr;
			selected_algorithm = attr_cast->get_hash_algo(attr_cast);
			if (!(selected_algorithm & supported_algorithms))
			{
				DBG1(DBG_IMC, "PTS-IMV selected unsupported DH hash algorithm");
				return FALSE;
			}
			pts->set_dh_hash_algorithm(pts, selected_algorithm);

			initiator_value = attr_cast->get_initiator_value(attr_cast);
			initiator_nonce = attr_cast->get_initiator_nonce(attr_cast);

			nonce_len = lib->settings->get_int(lib->settings,
								"%s.plugins.imc-attestation.nonce_len",
								 DEFAULT_NONCE_LEN, lib->ns);
			if (nonce_len != initiator_nonce.len)
			{
				DBG1(DBG_IMC, "initiator and responder DH nonces "
							  "have differing lengths");
				return FALSE;
			}

			pts->set_peer_public_value(pts, initiator_value, initiator_nonce);
			if (!pts->calculate_secret(pts))
			{
				return FALSE;
			}
			break;
		}
		case TCG_PTS_GET_TPM_VERSION_INFO:
		{
			chunk_t tpm_version_info, attr_info;
			pen_type_t error_code = { PEN_TCG, TCG_PTS_TPM_VERS_NOT_SUPPORTED };

			if (!pts->get_tpm_version_info(pts, &tpm_version_info))
			{
				attr_info = attr->get_value(attr);
				attr = ietf_attr_pa_tnc_error_create(error_code, attr_info);
				msg->add_attribute(msg, attr);
				break;
			}

			/* Send TPM Version Info attribute */
			attr = tcg_pts_attr_tpm_version_info_create(tpm_version_info);
			msg->add_attribute(msg, attr);
			break;
		}
		case TCG_PTS_GET_AIK:
		{
			certificate_t *aik;

			aik = pts->get_aik(pts);
			if (!aik)
			{
				DBG1(DBG_IMC, "no AIK certificate or public key available");
				break;
			}

			/* Send AIK attribute */
			attr = tcg_pts_attr_aik_create(aik);
			msg->add_attribute(msg, attr);
			break;
		}
		case TCG_PTS_REQ_FILE_MEAS:
		{
			tcg_pts_attr_req_file_meas_t *attr_cast;
			char *pathname;
			u_int16_t request_id;
			bool is_directory;
			u_int32_t delimiter;
			pts_file_meas_t *measurements;
			pen_type_t error_code;

			attr_info = attr->get_value(attr);
			attr_cast = (tcg_pts_attr_req_file_meas_t*)attr;
			is_directory = attr_cast->get_directory_flag(attr_cast);
			request_id = attr_cast->get_request_id(attr_cast);
			delimiter = attr_cast->get_delimiter(attr_cast);
			pathname = attr_cast->get_pathname(attr_cast);
			valid_path = pts->is_path_valid(pts, pathname, &pts_error);

			if (valid_path && pts_error)
			{
				error_code = pen_type_create(PEN_TCG, pts_error);
				attr = ietf_attr_pa_tnc_error_create(error_code, attr_info);
				msg->add_attribute(msg, attr);
				break;
			}
			else if (!valid_path)
			{
				break;
			}

			if (delimiter != SOLIDUS_UTF && delimiter != REVERSE_SOLIDUS_UTF)
			{
				error_code = pen_type_create(PEN_TCG,
											 TCG_PTS_INVALID_DELIMITER);
				attr = ietf_attr_pa_tnc_error_create(error_code, attr_info);
				msg->add_attribute(msg, attr);
				break;
			}

			/* Do PTS File Measurements and send them to PTS-IMV */
			DBG2(DBG_IMC, "measurement request %d for %s '%s'",
				 request_id, is_directory ? "directory" : "file",
				 pathname);
			measurements = pts_file_meas_create_from_path(request_id,
										pathname, is_directory, TRUE,
										pts->get_meas_algorithm(pts));
			if (!measurements)
			{
				/* TODO handle error codes from measurements */
				return FALSE;
			}
			attr = tcg_pts_attr_file_meas_create(measurements);
			attr->set_noskip_flag(attr, TRUE);
			msg->add_attribute(msg, attr);
			break;
		}
		case TCG_PTS_REQ_FILE_META:
		{
			tcg_pts_attr_req_file_meta_t *attr_cast;
			char *pathname;
			bool is_directory;
			u_int8_t delimiter;
			pts_file_meta_t *metadata;
			pen_type_t error_code;

			attr_info = attr->get_value(attr);
			attr_cast = (tcg_pts_attr_req_file_meta_t*)attr;
			is_directory = attr_cast->get_directory_flag(attr_cast);
			delimiter = attr_cast->get_delimiter(attr_cast);
			pathname = attr_cast->get_pathname(attr_cast);

			valid_path = pts->is_path_valid(pts, pathname, &pts_error);
			if (valid_path && pts_error)
			{
				error_code = pen_type_create(PEN_TCG, pts_error);
				attr = ietf_attr_pa_tnc_error_create(error_code, attr_info);
				msg->add_attribute(msg, attr);
				break;
			}
			else if (!valid_path)
			{
				break;
			}
			if (delimiter != SOLIDUS_UTF && delimiter != REVERSE_SOLIDUS_UTF)
			{
				error_code = pen_type_create(PEN_TCG,
											 TCG_PTS_INVALID_DELIMITER);
				attr = ietf_attr_pa_tnc_error_create(error_code, attr_info);
				msg->add_attribute(msg, attr);
				break;
			}
			/* Get File Metadata and send them to PTS-IMV */
			DBG2(DBG_IMC, "metadata request for %s '%s'",
					is_directory ? "directory" : "file",
					pathname);
			metadata = pts->get_metadata(pts, pathname, is_directory);

			if (!metadata)
			{
				/* TODO handle error codes from measurements */
				return FALSE;
			}
			attr = tcg_pts_attr_unix_file_meta_create(metadata);
			attr->set_noskip_flag(attr, TRUE);
			msg->add_attribute(msg, attr);
			break;
		}
		case TCG_PTS_REQ_FUNC_COMP_EVID:
		{
			tcg_pts_attr_req_func_comp_evid_t *attr_cast;
			pts_proto_caps_flag_t negotiated_caps;
			pts_comp_func_name_t *name;
			pts_comp_evidence_t *evid;
			pts_component_t *comp;
			pen_type_t error_code;
			u_int32_t depth;
			u_int8_t flags;
			status_t status;
			enumerator_t *e;

			attr_info = attr->get_value(attr);
			attr_cast = (tcg_pts_attr_req_func_comp_evid_t*)attr;

			DBG1(DBG_IMC, "evidence requested for %d functional components",
						   attr_cast->get_count(attr_cast));

			e = attr_cast->create_enumerator(attr_cast);
			while (e->enumerate(e, &flags, &depth, &name))
			{
				name->log(name, "* ");
				negotiated_caps = pts->get_proto_caps(pts);

				if (flags & PTS_REQ_FUNC_COMP_EVID_TTC)
				{
					error_code = pen_type_create(PEN_TCG,
												 TCG_PTS_UNABLE_DET_TTC);
					attr = ietf_attr_pa_tnc_error_create(error_code, attr_info);
					msg->add_attribute(msg, attr);
					break;
				}
				if (flags & PTS_REQ_FUNC_COMP_EVID_VER &&
					!(negotiated_caps & PTS_PROTO_CAPS_V))
				{
					error_code = pen_type_create(PEN_TCG,
												 TCG_PTS_UNABLE_LOCAL_VAL);
					attr = ietf_attr_pa_tnc_error_create(error_code, attr_info);
					msg->add_attribute(msg, attr);
					break;
				}
				if (flags & PTS_REQ_FUNC_COMP_EVID_CURR &&
					!(negotiated_caps & PTS_PROTO_CAPS_C))
				{
					error_code = pen_type_create(PEN_TCG,
												 TCG_PTS_UNABLE_CUR_EVID);
					attr = ietf_attr_pa_tnc_error_create(error_code, attr_info);
					msg->add_attribute(msg, attr);
					break;
				}
				if (flags & PTS_REQ_FUNC_COMP_EVID_PCR &&
					!(negotiated_caps & PTS_PROTO_CAPS_T))
				{
					error_code = pen_type_create(PEN_TCG,
												 TCG_PTS_UNABLE_DET_PCR);
					attr = ietf_attr_pa_tnc_error_create(error_code, attr_info);
					msg->add_attribute(msg, attr);
					break;
				}
				if (depth > 0)
				{
					DBG1(DBG_IMC, "the Attestation IMC currently does not "
								  "support sub component measurements");
					return FALSE;
				}
				comp = attestation_state->create_component(attestation_state,
														   name, depth);
				if (!comp)
				{
					DBG2(DBG_IMC, "    not registered: no evidence provided");
					continue;
				}

				/* do the component evidence measurement[s] and cache them */
				do
				{
					status = comp->measure(comp, name->get_qualifier(name),
										   pts, &evid);
					if (status == FAILED)
					{
						break;
					}
					attestation_state->add_evidence(attestation_state, evid);
				}
				while (status == NEED_MORE);
			}
			e->destroy(e);
			break;
		}
		case TCG_PTS_GEN_ATTEST_EVID:
		{
			pts_simple_evid_final_flag_t flags;
			pts_meas_algorithms_t comp_hash_algorithm;
			pts_comp_evidence_t *evid;
			chunk_t pcr_composite, quote_sig;
			bool use_quote2;

			/* Send cached Component Evidence entries */
			while (attestation_state->next_evidence(attestation_state, &evid))
			{
				attr = tcg_pts_attr_simple_comp_evid_create(evid);
				msg->add_attribute(msg, attr);
			}

			use_quote2 = lib->settings->get_bool(lib->settings,
							"%s.plugins.imc-attestation.use_quote2", TRUE,
							lib->ns);
			if (!pts->quote_tpm(pts, use_quote2, &pcr_composite, &quote_sig))
			{
				DBG1(DBG_IMC, "error occurred during TPM quote operation");
				return FALSE;
			}

			/* Send Simple Evidence Final attribute */
			flags = use_quote2 ? PTS_SIMPLE_EVID_FINAL_QUOTE_INFO2 :
								 PTS_SIMPLE_EVID_FINAL_QUOTE_INFO;
			comp_hash_algorithm = PTS_MEAS_ALGO_SHA1;

			attr = tcg_pts_attr_simple_evid_final_create(flags,
								comp_hash_algorithm, pcr_composite, quote_sig);
			msg->add_attribute(msg, attr);
			break;
		}
		case TCG_SEG_MAX_ATTR_SIZE_REQ:
		case TCG_SEG_NEXT_SEG_REQ:
			break;

		/* TODO: Not implemented yet */
		case TCG_PTS_REQ_INTEG_MEAS_LOG:
		/* Attributes using XML */
		case TCG_PTS_REQ_TEMPL_REF_MANI_SET_META:
		case TCG_PTS_UPDATE_TEMPL_REF_MANI:
		/* On Windows only*/
		case TCG_PTS_REQ_REGISTRY_VALUE:
		/* Received on IMV side only*/
		case TCG_PTS_PROTO_CAPS:
		case TCG_PTS_DH_NONCE_PARAMS_RESP:
		case TCG_PTS_MEAS_ALGO_SELECTION:
		case TCG_PTS_TPM_VERSION_INFO:
		case TCG_PTS_TEMPL_REF_MANI_SET_META:
		case TCG_PTS_AIK:
		case TCG_PTS_SIMPLE_COMP_EVID:
		case TCG_PTS_SIMPLE_EVID_FINAL:
		case TCG_PTS_VERIFICATION_RESULT:
		case TCG_PTS_INTEG_REPORT:
		case TCG_PTS_UNIX_FILE_META:
		case TCG_PTS_FILE_MEAS:
		case TCG_PTS_INTEG_MEAS_LOG:
		default:
			DBG1(DBG_IMC, "received unsupported attribute '%N/%N'",
				 pen_names, PEN_TCG, tcg_attr_names, attr_type.type);
			break;
	}
	return TRUE;
}
