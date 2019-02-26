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

/**
 * @defgroup libtncif libtncif
 *
 * @addtogroup libtncif
 * TNC interface definitions
 *
 * @defgroup tnc_policy tnc_policy
 * @{ @ingroup libtncif
 */

#ifndef TNCIF_POLICY_H_
#define TNCIF_POLICY_H_

#include "tncifimv.h"

/**
 * Create an empty TNC Identity object
 *
 * @param eval			Existing evaluation to be updated
 * @param eval_add		Partial evaluation to be added
 * @return				Updated evaluation
 */
TNC_IMV_Evaluation_Result tncif_policy_update_evaluation(
									TNC_IMV_Evaluation_Result eval,
									TNC_IMV_Evaluation_Result eval_add);

/**
 * Create an empty TNC Identity object
 *
 * @param rec			Existing recommendationto be updated
 * @param rec_add		Partial recommendation to be added
 * @return				Updated recommendation
 */
TNC_IMV_Action_Recommendation tncif_policy_update_recommendation(
									TNC_IMV_Action_Recommendation rec,
									TNC_IMV_Action_Recommendation rec_add);

#endif /** TNCIF_POLICY_H_ @}*/
