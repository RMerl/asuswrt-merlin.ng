/*
 * Copyright (C) 2011-2012 Sansar Choinyambuu
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

#define _GNU_SOURCE /* for stdndup() */
#include <string.h>

#include "imv_attestation_process.h"

#include <imcv.h>
#include <ietf/ietf_attr_pa_tnc_error.h>

#include <pts/pts.h>

#include <tcg/pts/tcg_pts_attr_aik.h>
#include <tcg/pts/tcg_pts_attr_dh_nonce_params_resp.h>
#include <tcg/pts/tcg_pts_attr_file_meas.h>
#include <tcg/pts/tcg_pts_attr_meas_algo.h>
#include <tcg/pts/tcg_pts_attr_proto_caps.h>
#include <tcg/pts/tcg_pts_attr_simple_comp_evid.h>
#include <tcg/pts/tcg_pts_attr_simple_evid_final.h>
#include <tcg/pts/tcg_pts_attr_tpm_version_info.h>
#include <tcg/pts/tcg_pts_attr_unix_file_meta.h>

#include <utils/debug.h>
#include <crypto/hashers/hasher.h>

#include <inttypes.h>

bool imv_attestation_process(pa_tnc_attr_t *attr, imv_msg_t *out_msg,
							 imv_state_t *state,
							 pts_meas_algorithms_t supported_algorithms,
							 pts_dh_group_t supported_dh_groups,
							 pts_database_t *pts_db,
							 credential_manager_t *pts_credmgr)
{
	imv_session_t *session;
	imv_attestation_state_t *attestation_state;
	pen_type_t attr_type;
	pts_t *pts;

	session = state->get_session(state);
	attestation_state = (imv_attestation_state_t*)state;
	pts = attestation_state->get_pts(attestation_state);
	attr_type = attr->get_type(attr);

	switch (attr_type.type)
	{
		case TCG_PTS_PROTO_CAPS:
		{
			tcg_pts_attr_proto_caps_t *attr_cast;
			pts_proto_caps_flag_t flags;

			attr_cast = (tcg_pts_attr_proto_caps_t*)attr;
			flags = attr_cast->get_flags(attr_cast);
			pts->set_proto_caps(pts, flags);
			break;
		}
		case TCG_PTS_MEAS_ALGO_SELECTION:
		{
			tcg_pts_attr_meas_algo_t *attr_cast;
			pts_meas_algorithms_t selected_algorithm;

			attr_cast = (tcg_pts_attr_meas_algo_t*)attr;
			selected_algorithm = attr_cast->get_algorithms(attr_cast);
			if (!(selected_algorithm & supported_algorithms))
			{
				DBG1(DBG_IMV, "PTS-IMC selected unsupported"
							  " measurement algorithm");
				return FALSE;
			}
			pts->set_meas_algorithm(pts, selected_algorithm);
			state->set_action_flags(state, IMV_ATTESTATION_ALGO);
			break;
		}
		case TCG_PTS_DH_NONCE_PARAMS_RESP:
		{
			tcg_pts_attr_dh_nonce_params_resp_t *attr_cast;
			int nonce_len, min_nonce_len;
			pts_dh_group_t dh_group;
			pts_meas_algorithms_t offered_algorithms, selected_algorithm;
			chunk_t responder_value, responder_nonce;

			attr_cast = (tcg_pts_attr_dh_nonce_params_resp_t*)attr;
			responder_nonce = attr_cast->get_responder_nonce(attr_cast);

			/* check compliance of responder nonce length */
			min_nonce_len = lib->settings->get_int(lib->settings,
						"%s.plugins.imv-attestation.min_nonce_len", 0, lib->ns);
			nonce_len = responder_nonce.len;
			if (nonce_len < PTS_MIN_NONCE_LEN ||
			   (min_nonce_len > 0 && nonce_len < min_nonce_len))
			{
				attr = pts_dh_nonce_error_create(
									max(PTS_MIN_NONCE_LEN, min_nonce_len),
										PTS_MAX_NONCE_LEN);
				out_msg->add_attribute(out_msg, attr);
				break;
			}

			dh_group = attr_cast->get_dh_group(attr_cast);
			if (!(dh_group & supported_dh_groups))
			{
				DBG1(DBG_IMV, "PTS-IMC selected unsupported DH group");
				return FALSE;
			}

			offered_algorithms = attr_cast->get_hash_algo_set(attr_cast);
			selected_algorithm = pts_meas_algo_select(supported_algorithms,
													  offered_algorithms);
			if (selected_algorithm == PTS_MEAS_ALGO_NONE)
			{
				attr = pts_hash_alg_error_create(supported_algorithms);
				out_msg->add_attribute(out_msg, attr);
				break;
			}
			pts->set_dh_hash_algorithm(pts, selected_algorithm);

			if (!pts->create_dh_nonce(pts, dh_group, nonce_len))
			{
				return FALSE;
			}

			responder_value = attr_cast->get_responder_value(attr_cast);

			/* Calculate secret assessment value */
			if (!pts->set_peer_public_value(pts, responder_value,
											responder_nonce) ||
				!pts->calculate_secret(pts))
			{
				return FALSE;
			}
			state->set_action_flags(state, IMV_ATTESTATION_DH_NONCE);
			break;
		}
		case TCG_PTS_TPM_VERSION_INFO:
		{
			tcg_pts_attr_tpm_version_info_t *attr_cast;
			chunk_t tpm_version_info;

			attr_cast = (tcg_pts_attr_tpm_version_info_t*)attr;
			tpm_version_info = attr_cast->get_tpm_version_info(attr_cast);
			pts->set_tpm_version_info(pts, tpm_version_info);
			break;
		}
		case TCG_PTS_AIK:
		{
			tcg_pts_attr_aik_t *attr_cast;
			certificate_t *aik, *issuer;
			public_key_t *public;
			chunk_t keyid, keyid_hex, device_id;
			int aik_id;
			enumerator_t *e;
			bool trusted = FALSE, trusted_chain = FALSE;

			attr_cast = (tcg_pts_attr_aik_t*)attr;
			aik = attr_cast->get_aik(attr_cast);
			if (!aik)
			{
				DBG1(DBG_IMV, "AIK unavailable");
				attestation_state->set_measurement_error(attestation_state,
									IMV_ATTESTATION_ERROR_NO_TRUSTED_AIK);
				break;
			}

			/* check trust into public key as stored in the database */
			public = aik->get_public_key(aik);
			public->get_fingerprint(public, KEYID_PUBKEY_INFO_SHA1, &keyid);
			DBG1(DBG_IMV, "verifying AIK with keyid %#B", &keyid);
			keyid_hex = chunk_to_hex(keyid, NULL, FALSE);
			if (session->get_device_id(session, &device_id) &&
				chunk_equals_const(keyid_hex, device_id))
			{
				trusted = session->get_device_trust(session);
			}
			else
			{
				DBG1(DBG_IMV, "device ID unknown or different from AIK keyid");
			}
			DBG1(DBG_IMV, "AIK public key is %strusted", trusted ? "" : "not ");
			public->destroy(public);
			chunk_free(&keyid_hex);

			if (aik->get_type(aik) == CERT_X509)
			{

				e = pts_credmgr->create_trusted_enumerator(pts_credmgr,
							KEY_ANY, aik->get_issuer(aik), FALSE);
				while (e->enumerate(e, &issuer, NULL))
				{
					if (aik->issued_by(aik, issuer, NULL))
					{
						trusted_chain = TRUE;
						break;
					}
				}
				e->destroy(e);
				DBG1(DBG_IMV, "AIK certificate is %strusted",
							   trusted_chain ? "" : "not ");
				if (!trusted || !trusted_chain)
				{
					attestation_state->set_measurement_error(attestation_state,
										IMV_ATTESTATION_ERROR_NO_TRUSTED_AIK);
					break;
				}
			}
			session->get_session_id(session, NULL, &aik_id);
			pts->set_aik(pts, aik, aik_id);
			state->set_action_flags(state, IMV_ATTESTATION_AIK);
			break;
		}
		case TCG_PTS_FILE_MEAS:
		{
			TNC_IMV_Evaluation_Result eval;
			TNC_IMV_Action_Recommendation rec;
			tcg_pts_attr_file_meas_t *attr_cast;
			uint16_t request_id;
			int arg_int, file_count;
			pts_meas_algorithms_t algo;
			pts_file_meas_t *measurements;
			imv_workitem_t *workitem, *found = NULL;
			imv_workitem_type_t type;
			char result_str[BUF_LEN];
			bool is_dir, correct;
			enumerator_t *enumerator;

			eval = TNC_IMV_EVALUATION_RESULT_COMPLIANT;
			algo = pts->get_meas_algorithm(pts);
			attr_cast = (tcg_pts_attr_file_meas_t*)attr;
			measurements = attr_cast->get_measurements(attr_cast);
			request_id = measurements->get_request_id(measurements);
			file_count = measurements->get_file_count(measurements);

			DBG1(DBG_IMV, "measurement request %d returned %d file%s:",
				 request_id, file_count, (file_count == 1) ? "":"s");

			if (request_id)
			{
				enumerator = session->create_workitem_enumerator(session);
				while (enumerator->enumerate(enumerator, &workitem))
				{
					/* request ID consist of lower 16 bits of workitem ID */
					if ((workitem->get_id(workitem) & 0xffff) == request_id)
					{
						found = workitem;
						break;
					}
				}

				if (!found)
				{
					DBG1(DBG_IMV, "  no entry found for file measurement "
								  "request %d", request_id);
					enumerator->destroy(enumerator);
					break;
				}
				type =    found->get_type(found);
				arg_int = found->get_arg_int(found);

				switch (type)
				{
					default:
					case IMV_WORKITEM_FILE_REF_MEAS:
					case IMV_WORKITEM_FILE_MEAS:
						is_dir = FALSE;
						break;
					case IMV_WORKITEM_DIR_REF_MEAS:
					case IMV_WORKITEM_DIR_MEAS:
						is_dir = TRUE;
				}

				switch (type)
				{
					case IMV_WORKITEM_FILE_MEAS:
					case IMV_WORKITEM_DIR_MEAS:
					{
						enumerator_t *e;

						/* check hashes from database against measurements */
						e = pts_db->create_file_hash_enumerator(pts_db,
											pts->get_platform_id(pts),
											algo, is_dir, arg_int);
						if (!e)
						{
							eval = TNC_IMV_EVALUATION_RESULT_ERROR;
							break;
						}
						correct = measurements->verify(measurements, e, is_dir);
						if (!correct)
						{
							attestation_state->set_measurement_error(
										attestation_state,
										IMV_ATTESTATION_ERROR_FILE_MEAS_FAIL);
							eval = TNC_IMV_EVALUATION_RESULT_NONCOMPLIANT_MINOR;
						}
						e->destroy(e);

						snprintf(result_str, BUF_LEN, "%s measurement%s correct",
								 is_dir ? "directory" : "file",
								 correct ? "" : " not");
						break;
					}
					case IMV_WORKITEM_FILE_REF_MEAS:
					case IMV_WORKITEM_DIR_REF_MEAS:
					{
						enumerator_t *e;
						char *filename;
						chunk_t measurement;
						int vid;

						if (!pts_db->get_product_version(pts_db,
											pts->get_platform_id(pts), &vid))
						{
							eval = TNC_IMV_EVALUATION_RESULT_ERROR;
							break;
						}

						e = measurements->create_enumerator(measurements);
						while (e->enumerate(e, &filename, &measurement))
						{
							if (!pts_db->add_file_measurement(pts_db, vid, algo,
										measurement, filename, is_dir, arg_int))
							{
								eval = TNC_IMV_EVALUATION_RESULT_ERROR;
								e->destroy(e);
								break;
							}
						}
						e->destroy(e);
						snprintf(result_str, BUF_LEN, "%s reference measurement "
								"successful", is_dir ? "directory" : "file");
						break;
					}
					default:
						break;
				}

				session->remove_workitem(session, enumerator);
				enumerator->destroy(enumerator);
				rec = found->set_result(found, result_str, eval);
				state->update_recommendation(state, rec, eval);
				imcv_db->finalize_workitem(imcv_db, found);
				found->destroy(found);
			}
			else
			{
				measurements->check(measurements, pts_db,
									pts->get_platform_id(pts), algo);
			}
			break;
		}
		case TCG_PTS_UNIX_FILE_META:
		{
			tcg_pts_attr_file_meta_t *attr_cast;
			int file_count;
			pts_file_meta_t *metadata;
			pts_file_metadata_t *entry;
			time_t created, modified, accessed;
			bool utc = FALSE;
			enumerator_t *e;

			attr_cast = (tcg_pts_attr_file_meta_t*)attr;
			metadata = attr_cast->get_metadata(attr_cast);
			file_count = metadata->get_file_count(metadata);

			DBG1(DBG_IMV, "metadata request returned %d file%s:",
				 file_count, (file_count == 1) ? "":"s");

			e = metadata->create_enumerator(metadata);
			while (e->enumerate(e, &entry))
			{
				DBG1(DBG_IMV, " '%s' (%"PRIu64" bytes)"
							  " owner %"PRIu64", group %"PRIu64", type %N",
					 entry->filename, entry->filesize, entry->owner,
					 entry->group, pts_file_type_names, entry->type);

				created = entry->created;
				modified = entry->modified;
				accessed = entry->accessed;

				DBG1(DBG_IMV, "    created %T, modified %T, accessed %T",
					 &created, utc, &modified, utc, &accessed, utc);
			}
			e->destroy(e);
			break;
		}
		case TCG_PTS_SIMPLE_COMP_EVID:
		{
			tcg_pts_attr_simple_comp_evid_t *attr_cast;
			pts_comp_func_name_t *name;
			pts_comp_evidence_t *evidence;
			pts_component_t *comp;
			uint32_t depth;
			status_t status;

			attr_cast = (tcg_pts_attr_simple_comp_evid_t*)attr;
			evidence = attr_cast->get_comp_evidence(attr_cast);
			name = evidence->get_comp_func_name(evidence, &depth);

			comp = attestation_state->get_component(attestation_state, name);
			if (!comp)
			{
				DBG1(DBG_IMV, "  no entry found for component evidence request");
				break;
			}
			status = comp->verify(comp, name->get_qualifier(name), pts, evidence);
			if (status == VERIFY_ERROR || status == FAILED)
			{
				attestation_state->set_measurement_error(attestation_state,
									IMV_ATTESTATION_ERROR_COMP_EVID_FAIL);
				name->log(name, "  measurement mismatch for ");
			}
			break;
		}
		case TCG_PTS_SIMPLE_EVID_FINAL:
		{
			tcg_pts_attr_simple_evid_final_t *attr_cast;
			tpm_tss_quote_info_t *quote_info;
			chunk_t quoted = chunk_empty, quote_sig, evid_sig, result_buf;
			imv_workitem_t *workitem;
			imv_reason_string_t *reason_string;
			hash_algorithm_t digest_alg;
			enumerator_t *enumerator;
			bio_writer_t *result;

			attr_cast = (tcg_pts_attr_simple_evid_final_t*)attr;
			attr_cast->get_quote_info(attr_cast, &quote_info, &quote_sig);

			if (quote_info->get_quote_mode(quote_info) != TPM_QUOTE_NONE)
			{
				/* Construct PCR Composite and TPM Quote Info structures */
				if (!pts->get_quote(pts, quote_info, &quoted))
				{
					DBG1(DBG_IMV, "unable to construct TPM Quote Info digest");
					attestation_state->set_measurement_error(attestation_state,
										IMV_ATTESTATION_ERROR_TPM_QUOTE_FAIL);
					goto quote_error;
				}
				digest_alg = quote_info->get_pcr_digest_alg(quote_info);

				if (!pts->verify_quote_signature(pts, digest_alg, quoted,
												 quote_sig))
				{
					attestation_state->set_measurement_error(attestation_state,
										IMV_ATTESTATION_ERROR_TPM_QUOTE_FAIL);
					goto quote_error;
				}
				DBG2(DBG_IMV, "TPM Quote Info signature verification successful");

quote_error:
				chunk_free(&quoted);

				/**
				 * Finalize any pending measurement registrations and check
				 * if all expected component measurements were received
				 */
				result = bio_writer_create(128);
				attestation_state->finalize_components(attestation_state,
													   result);

				enumerator = session->create_workitem_enumerator(session);
				while (enumerator->enumerate(enumerator, &workitem))
				{
					if (workitem->get_type(workitem) == IMV_WORKITEM_TPM_ATTEST)
					{
						TNC_IMV_Action_Recommendation rec;
						TNC_IMV_Evaluation_Result eval;
						uint32_t error;

						error = attestation_state->get_measurement_error(
														attestation_state);
						if (error & (IMV_ATTESTATION_ERROR_COMP_EVID_FAIL |
									 IMV_ATTESTATION_ERROR_COMP_EVID_PEND |
									 IMV_ATTESTATION_ERROR_TPM_QUOTE_FAIL))
						{
							reason_string = imv_reason_string_create("en", ", ");
							attestation_state->add_comp_evid_reasons(
											attestation_state, reason_string);
							result->write_data(result, chunk_from_str("; "));
							result->write_data(result,
									reason_string->get_encoding(reason_string));
							reason_string->destroy(reason_string);
							eval = TNC_IMV_EVALUATION_RESULT_NONCOMPLIANT_MINOR;
						}
						else
						{
							eval = TNC_IMV_EVALUATION_RESULT_COMPLIANT;
						}
						session->remove_workitem(session, enumerator);

						result->write_uint8(result, '\0');
						result_buf = result->get_buf(result);
						rec = workitem->set_result(workitem, result_buf.ptr,
															 eval);
						state->update_recommendation(state, rec, eval);
						imcv_db->finalize_workitem(imcv_db, workitem);
						workitem->destroy(workitem);
						attestation_state->set_handshake_state(attestation_state,
													IMV_ATTESTATION_STATE_END);
						break;
					}
				}
				enumerator->destroy(enumerator);
				result->destroy(result);
			}

			if (attr_cast->get_evid_sig(attr_cast, &evid_sig))
			{
				/** TODO: What to do with Evidence Signature */
				DBG1(DBG_IMV, "this version of the Attestation IMV can not "
							  "handle Evidence Signatures");
			}
			break;
		}
		case TCG_SEG_MAX_ATTR_SIZE_RESP:
		case TCG_SEG_ATTR_SEG_ENV:
			break;

		/* TODO: Not implemented yet */
		case TCG_PTS_INTEG_MEAS_LOG:
		/* Attributes using XML */
		case TCG_PTS_TEMPL_REF_MANI_SET_META:
		case TCG_PTS_VERIFICATION_RESULT:
		case TCG_PTS_INTEG_REPORT:
		/* On Windows only*/
		case TCG_PTS_WIN_FILE_META:
		case TCG_PTS_REGISTRY_VALUE:
		/* Received on IMC side only*/
		case TCG_PTS_REQ_PROTO_CAPS:
		case TCG_PTS_DH_NONCE_PARAMS_REQ:
		case TCG_PTS_DH_NONCE_FINISH:
		case TCG_PTS_MEAS_ALGO:
		case TCG_PTS_GET_TPM_VERSION_INFO:
		case TCG_PTS_REQ_TEMPL_REF_MANI_SET_META:
		case TCG_PTS_UPDATE_TEMPL_REF_MANI:
		case TCG_PTS_GET_AIK:
		case TCG_PTS_REQ_FUNC_COMP_EVID:
		case TCG_PTS_GEN_ATTEST_EVID:
		case TCG_PTS_REQ_FILE_META:
		case TCG_PTS_REQ_FILE_MEAS:
		case TCG_PTS_REQ_INTEG_MEAS_LOG:
		default:
			DBG1(DBG_IMV, "received unsupported attribute '%N/%N'",
				 pen_names, PEN_TCG, tcg_attr_names, attr_type.type);
			break;
	}
	return TRUE;
}
