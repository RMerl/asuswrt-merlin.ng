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

#include "imv_attestation_build.h"
#include "imv_attestation_state.h"

#include <tcg/pts/tcg_pts_attr_dh_nonce_params_req.h>
#include <tcg/pts/tcg_pts_attr_dh_nonce_finish.h>
#include <tcg/pts/tcg_pts_attr_get_tpm_version_info.h>
#include <tcg/pts/tcg_pts_attr_get_aik.h>
#include <tcg/pts/tcg_pts_attr_req_func_comp_evid.h>
#include <tcg/pts/tcg_pts_attr_gen_attest_evid.h>

#include <utils/debug.h>

bool imv_attestation_build(imv_msg_t *out_msg, imv_state_t *state,
						   pts_dh_group_t supported_dh_groups,
						   pts_database_t *pts_db)
{
	imv_attestation_state_t *attestation_state;
	imv_attestation_handshake_state_t handshake_state;
	pts_t *pts;
	pa_tnc_attr_t *attr = NULL;

	attestation_state = (imv_attestation_state_t*)state;
	handshake_state = attestation_state->get_handshake_state(attestation_state);
	pts = attestation_state->get_pts(attestation_state);

	switch (handshake_state)
	{
		case IMV_ATTESTATION_STATE_NONCE_REQ:
		{
			int min_nonce_len;

			/* Send DH nonce parameters request attribute */
			min_nonce_len = lib->settings->get_int(lib->settings,
						"%s.plugins.imv-attestation.min_nonce_len", 0, lib->ns);
			attr = tcg_pts_attr_dh_nonce_params_req_create(min_nonce_len,
													 supported_dh_groups);
			attr->set_noskip_flag(attr, TRUE);
			out_msg->add_attribute(out_msg, attr);

			attestation_state->set_handshake_state(attestation_state,
										IMV_ATTESTATION_STATE_TPM_INIT);
			break;
		}
		case IMV_ATTESTATION_STATE_TPM_INIT:
		{
			pts_meas_algorithms_t selected_algorithm;
			chunk_t initiator_value, initiator_nonce;

			if (!(state->get_action_flags(state) & IMV_ATTESTATION_DH_NONCE))
			{
				break;
			}

			/* Send DH nonce finish attribute */
			selected_algorithm = pts->get_meas_algorithm(pts);
			pts->get_my_public_value(pts, &initiator_value, &initiator_nonce);
			attr = tcg_pts_attr_dh_nonce_finish_create(selected_algorithm,
											initiator_value, initiator_nonce);
			attr->set_noskip_flag(attr, TRUE);
			out_msg->add_attribute(out_msg, attr);

			/* Send Get TPM Version attribute */
			attr = tcg_pts_attr_get_tpm_version_info_create();
			attr->set_noskip_flag(attr, TRUE);
			out_msg->add_attribute(out_msg, attr);

			/* Send Get AIK attribute */
			attr = tcg_pts_attr_get_aik_create();
			attr->set_noskip_flag(attr, TRUE);
			out_msg->add_attribute(out_msg, attr);

			attestation_state->set_handshake_state(attestation_state,
										IMV_ATTESTATION_STATE_COMP_EVID);
			break;
		}
		case IMV_ATTESTATION_STATE_COMP_EVID:
		{
			tcg_pts_attr_req_func_comp_evid_t *attr_cast;
			enumerator_t *enumerator;
			pts_comp_func_name_t *name;
			uint8_t flags;
			uint32_t depth;
			bool first_component = TRUE;

			if (!(state->get_action_flags(state) & IMV_ATTESTATION_AIK))
			{
				break;
			}

			attestation_state->set_handshake_state(attestation_state,
										IMV_ATTESTATION_STATE_END);

			if (!pts->get_aik_id(pts))
			{
				attestation_state->set_measurement_error(attestation_state,
									IMV_ATTESTATION_ERROR_NO_TRUSTED_AIK);
				return FALSE;
			}

			enumerator = attestation_state->create_component_enumerator(
													attestation_state);
			while (enumerator->enumerate(enumerator, &flags, &depth, &name))
			{
				if (first_component)
				{
					attr = tcg_pts_attr_req_func_comp_evid_create();
					attr->set_noskip_flag(attr, TRUE);
					first_component = FALSE;
					DBG2(DBG_IMV, "evidence request by");
				}
				name->log(name, "  ");

				/* TODO check flags against negotiated_caps */
				attr_cast = (tcg_pts_attr_req_func_comp_evid_t *)attr;
				attr_cast->add_component(attr_cast, flags, depth, name);
			}
			enumerator->destroy(enumerator);

			if (attr)
			{
				/* Send Request Functional Component Evidence attribute */
				out_msg->add_attribute(out_msg, attr);

				/* Send Generate Attestation Evidence attribute */
				attr = tcg_pts_attr_gen_attest_evid_create();
				attr->set_noskip_flag(attr, TRUE);
				out_msg->add_attribute(out_msg, attr);

				attestation_state->set_handshake_state(attestation_state,
										IMV_ATTESTATION_STATE_EVID_FINAL);
			}
			break;
		}
		default:
			break;
	}

	return TRUE;
}
