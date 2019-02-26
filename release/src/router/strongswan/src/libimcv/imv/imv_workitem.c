/*
 * Copyright (C) 2013 Andreas Steffen
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

#include "imv_workitem.h"

#include <utils/debug.h>
#include <tncif_names.h>

typedef struct private_imv_workitem_t private_imv_workitem_t;

ENUM(imv_workitem_type_names, IMV_WORKITEM_PACKAGES, IMV_WORKITEM_TPM_ATTEST,
	"PCKGS",
	"UNSRC",
	"FWDEN",
	"PWDEN",
	"FREFM",
	"FMEAS",
	"FMETA",
	"DREFM",
	"DMEAS",
	"DMETA",
	"TCPOP",
	"TCPBL",
	"UDPOP",
	"UDPBL",
	"SWIDT",
	"TPMRA"
);

/**
 * Private data of a imv_workitem_t object.
 *
 */
struct private_imv_workitem_t {

	/**
	 * Public imv_workitem_t interface.
	 */
	imv_workitem_t public;

	/**
	 * Primary workitem key
	 */
	int id;

	/**
	 * IMV ID
	 */
	TNC_IMVID imv_id;

	/**
	 * Workitem type
	 */
	imv_workitem_type_t type;

	/**
	 * Argument string
	 */
	char *arg_str;

	/**
	 * Argument integer
	 */
	int arg_int;

	/**
	 * Result string
	 */
	char *result;

	/**
	 * IMV action recommendation
	 */
	TNC_IMV_Action_Recommendation rec_fail;

	/**
	 * IMV action recommendation
	 */
	TNC_IMV_Action_Recommendation rec_noresult;

	/**
	 * IMV action recommendation
	 */
	TNC_IMV_Action_Recommendation rec_final;

};

METHOD(imv_workitem_t, get_id, int,
	private_imv_workitem_t *this)
{
	return this->id;
}

METHOD(imv_workitem_t, set_imv_id, void,
	private_imv_workitem_t *this, TNC_IMVID imv_id)
{
	this->imv_id = imv_id;

	DBG2(DBG_IMV, "IMV %d handles %N workitem %d", imv_id,
		 imv_workitem_type_names, this->type, this->id);
}

METHOD(imv_workitem_t, get_imv_id, TNC_IMVID,
	private_imv_workitem_t *this)
{
	return this->imv_id;
}

METHOD(imv_workitem_t, get_type, imv_workitem_type_t,
	private_imv_workitem_t *this)
{
	return this->type;
}

METHOD(imv_workitem_t, get_arg_str, char*,
	private_imv_workitem_t *this)
{
	return this->arg_str;
}

METHOD(imv_workitem_t, get_arg_int, int,
	private_imv_workitem_t *this)
{
	return this->arg_int;
}

METHOD(imv_workitem_t, set_result, TNC_IMV_Action_Recommendation,
	private_imv_workitem_t *this, char *result, TNC_IMV_Evaluation_Result eval)
{
	this->result = strdup(result);
	switch (eval)
	{
		case TNC_IMV_EVALUATION_RESULT_COMPLIANT:
			this->rec_final = TNC_IMV_ACTION_RECOMMENDATION_ALLOW;
			break;
		case TNC_IMV_EVALUATION_RESULT_NONCOMPLIANT_MINOR:
		case TNC_IMV_EVALUATION_RESULT_NONCOMPLIANT_MAJOR:
			this->rec_final = this->rec_fail;
			break;
		case TNC_IMV_EVALUATION_RESULT_ERROR:
		case TNC_IMV_EVALUATION_RESULT_DONT_KNOW:
		default:
			this->rec_final = this->rec_noresult;
			break;
	}
	DBG2(DBG_IMV, "IMV %d handled %N workitem %d: %N%s%s", this->imv_id,
		 imv_workitem_type_names, this->type, this->id,
		 TNC_IMV_Action_Recommendation_names, this->rec_final,
		 strlen(result) ? " - " : "", result);

	return this->rec_final;	
}

METHOD(imv_workitem_t, get_result, TNC_IMV_Action_Recommendation,
	private_imv_workitem_t *this, char **result)
{
	if (result)
	{
		*result = this->result;
	}
	return this->rec_final;
}

METHOD(imv_workitem_t, destroy, void,
	private_imv_workitem_t *this)
{
	free(this->arg_str);
	free(this->result);
	free(this);
}

/**
 * See header
 */
imv_workitem_t *imv_workitem_create(int id, imv_workitem_type_t type,
									char *arg_str, int arg_int,
									TNC_IMV_Action_Recommendation rec_fail,
									TNC_IMV_Action_Recommendation rec_noresult)
{
	private_imv_workitem_t *this;

	INIT(this,
		.public = {
			.get_id = _get_id,
			.set_imv_id = _set_imv_id,
			.get_imv_id = _get_imv_id,
			.get_type = _get_type,
			.get_arg_str = _get_arg_str,
			.get_arg_int = _get_arg_int,
			.set_result = _set_result,
			.get_result = _get_result,
			.destroy = _destroy,
		},
		.id = id,
		.imv_id = TNC_IMVID_ANY,
		.type = type,
		.arg_str = arg_str ? strdup(arg_str) : NULL,
		.arg_int = arg_int,
		.rec_fail = rec_fail,
		.rec_noresult = rec_noresult,
		.rec_final = TNC_IMV_ACTION_RECOMMENDATION_NO_RECOMMENDATION,
	);

	return &this->public;
}

