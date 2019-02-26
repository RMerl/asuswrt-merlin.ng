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

#include "tncif_policy.h"

/**
 * See header
 */
TNC_IMV_Evaluation_Result tncif_policy_update_evaluation(
									TNC_IMV_Evaluation_Result eval,
									TNC_IMV_Evaluation_Result eval_add)
{
	switch (eval)
	{
		case TNC_IMV_EVALUATION_RESULT_COMPLIANT:
			switch (eval_add)
			{
				case TNC_IMV_EVALUATION_RESULT_NONCOMPLIANT_MINOR:
				case TNC_IMV_EVALUATION_RESULT_NONCOMPLIANT_MAJOR:
				case TNC_IMV_EVALUATION_RESULT_ERROR:
					eval = eval_add;
					break;
				default:
					break;
			}
			break;
		case TNC_IMV_EVALUATION_RESULT_NONCOMPLIANT_MINOR:
			switch (eval_add)
			{
				case TNC_IMV_EVALUATION_RESULT_NONCOMPLIANT_MAJOR:
				case TNC_IMV_EVALUATION_RESULT_ERROR:
					eval = eval_add;
					break;
				default:
					break;
			}
			break;
		case TNC_IMV_EVALUATION_RESULT_NONCOMPLIANT_MAJOR:
			switch (eval_add)
			{
				case TNC_IMV_EVALUATION_RESULT_ERROR:
					eval = eval_add;
					break;
				default:
					break;
			}
			break;
		case TNC_IMV_EVALUATION_RESULT_DONT_KNOW:
			eval = eval_add;
			break;
		default:
			break;
	}
	return eval;
}

/**
 * See header
 */
TNC_IMV_Action_Recommendation tncif_policy_update_recommendation(
									TNC_IMV_Action_Recommendation rec,
									TNC_IMV_Action_Recommendation rec_add)
{
	switch (rec)
	{
		case TNC_IMV_ACTION_RECOMMENDATION_ALLOW:
			switch (rec_add)
			{
				case TNC_IMV_ACTION_RECOMMENDATION_NO_ACCESS:
				case TNC_IMV_ACTION_RECOMMENDATION_ISOLATE:
					rec = rec_add;
					break;
				default:
					break;
			}
			break;
		case TNC_IMV_ACTION_RECOMMENDATION_ISOLATE:
			switch (rec_add)
			{
				case TNC_IMV_ACTION_RECOMMENDATION_NO_ACCESS:
					rec = rec_add;
					break;
				default:
					break;
			}
			break;
		case TNC_IMV_ACTION_RECOMMENDATION_NO_RECOMMENDATION:
			rec = rec_add;
			break;
		default:
			break;
	}
	return rec;
}
