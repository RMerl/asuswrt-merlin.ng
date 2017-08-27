/*
 * Copyright (C) 2010 Andreas Steffen
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
 * @defgroup imv_recommendations imv_recommendations
 * @{ @ingroup imv
 */

#ifndef IMV_RECOMMENDATIONS_H_
#define IMV_RECOMMENDATIONS_H_

#include <tncifimv.h>
#include <library.h>

typedef enum recommendation_policy_t recommendation_policy_t;

enum recommendation_policy_t {
	RECOMMENDATION_POLICY_DEFAULT,
	RECOMMENDATION_POLICY_ANY,
	RECOMMENDATION_POLICY_ALL
};

extern enum_name_t *recommendation_policy_names;


typedef struct recommendations_t recommendations_t;

/**
 * Collection of all IMV action recommendations and evaluation results
 */
struct recommendations_t {

	/**
	 * Deliver an IMV action recommendation and IMV evaluation result to the TNCS
	 *
	 * @param imv_id		ID of the IMV providing the recommendation
	 * @param rec			action recommendation
	 * @param eval			evaluation result
	 * @return				return code
	 */
	TNC_Result (*provide_recommendation)(recommendations_t *this,
										 TNC_IMVID imv_id,
										 TNC_IMV_Action_Recommendation rec,
										 TNC_IMV_Evaluation_Result eval);

	/**
	 * If all IMVs provided a recommendation, derive a consolidated action
	 * recommendation and evaluation result based on a configured policy
	 *
	 * @param rec			action recommendation
	 * @param eval			evaluation result
	 * @return				TRUE if all IMVs provided a recommendation
	 */
	bool (*have_recommendation)(recommendations_t *this,
								TNC_IMV_Action_Recommendation *rec,
								TNC_IMV_Evaluation_Result *eval);

	/**
	 * Clear all recommendation information
	 */
	void (*clear_recommendation)(recommendations_t *this);

	/**
	 * Get the preferred language for remediation messages
	 *
	 * @return				preferred language
	 */
	chunk_t (*get_preferred_language)(recommendations_t *this);

	/**
	 * Set the preferred language for remediation messages
	 *
	 * @param pref_lang		preferred language
	 */
	void (*set_preferred_language)(recommendations_t *this, chunk_t pref_lang);

	/**
	 * Set the reason string
	 *
	 * @param id			ID of IMV setting the reason string
	 * @param reason		reason string
	 * @result				return code
	 */
	TNC_Result (*set_reason_string)(recommendations_t *this, TNC_IMVID id,
									chunk_t reason);

	/**
	 * Set the language for reason strings
	 *
	 * @param id			ID of IMV setting the reason language
	 * @param reason_lang	reason language
	 * @result				return code
	 */
	TNC_Result (*set_reason_language)(recommendations_t *this, TNC_IMVID id,
									  chunk_t reason_lang);

	/**
	 * Enumerates over all IMVs sending a reason string.
	 * Format:  TNC_IMVID *id, chunk_t *reason, chunk_t *reason_language
	 *
	 * @return				enumerator
	 */
	enumerator_t* (*create_reason_enumerator)(recommendations_t *this);

	/**
	 * Destroys an imv_t object.
	 */
	void (*destroy)(recommendations_t *this);
};

#endif /** IMV_RECOMMENDATIONS_H_ @}*/
