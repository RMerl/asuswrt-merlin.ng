/*
 * Copyright (C) 2010-2012 Andreas Steffen
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

#include <tncifimv.h>
#include <tncif_names.h>
#include <tncif_policy.h>

#include <tnc/tnc.h>
#include <tnc/imv/imv.h>
#include <tnc/imv/imv_manager.h>
#include <tnc/imv/imv_recommendations.h>

#include <utils/debug.h>
#include <collections/linked_list.h>

typedef struct private_tnc_imv_recommendations_t private_tnc_imv_recommendations_t;
typedef struct recommendation_entry_t recommendation_entry_t;

/**
 * Recommendation entry
 */
struct recommendation_entry_t {

	/**
	 * IMV ID
	 */
	TNC_IMVID id;

	/**
	 * Received a recommendation message from this IMV?
	 */
	bool have_recommendation;

	/**
	 * Action Recommendation provided by IMV instance
	 */
	TNC_IMV_Action_Recommendation rec;

	/**
	 * Evaluation Result provided by IMV instance
	 */
	TNC_IMV_Evaluation_Result eval;

	/**
	 * Reason string provided by IMV instance
	 */
	chunk_t reason;

	/**
	 * Reason language provided by IMV instance
	 */
	chunk_t reason_language;
};

/**
 * Private data of a recommendations_t object.
 */
struct private_tnc_imv_recommendations_t {

	/**
	 * Public members of recommendations_t.
	 */
	recommendations_t public;

	/**
	 * list of recommendations and evaluations provided by IMVs
	 */
	linked_list_t *recs;

	/**
	 * Preferred language for remediation messages
	 */
	chunk_t preferred_language;
};

METHOD(recommendations_t, provide_recommendation, TNC_Result,
	private_tnc_imv_recommendations_t* this, TNC_IMVID id,
											 TNC_IMV_Action_Recommendation rec,
											 TNC_IMV_Evaluation_Result eval)
{
	enumerator_t *enumerator;
	recommendation_entry_t *entry;
	bool found = FALSE;

	DBG2(DBG_TNC, "IMV %u provides recommendation '%N' and evaluation '%N'", id,
		 TNC_IMV_Action_Recommendation_names, rec,
		 TNC_IMV_Evaluation_Result_names, eval);

	enumerator = this->recs->create_enumerator(this->recs);
	while (enumerator->enumerate(enumerator, &entry))
	{
		if (entry->id == id)
		{
			found = TRUE;
			entry->have_recommendation = TRUE;
			entry->rec = rec;
			entry->eval = eval;
			break;
		}
	}
	enumerator->destroy(enumerator);
	return found ? TNC_RESULT_SUCCESS : TNC_RESULT_FATAL;
}

METHOD(recommendations_t, have_recommendation, bool,
	private_tnc_imv_recommendations_t *this, TNC_IMV_Action_Recommendation *rec,
											 TNC_IMV_Evaluation_Result *eval)
{
	enumerator_t *enumerator;
	recommendation_entry_t *entry;
	recommendation_policy_t policy;
	TNC_IMV_Action_Recommendation final_rec;
	TNC_IMV_Evaluation_Result final_eval;
	bool first = TRUE, incomplete = FALSE;

	final_rec  = TNC_IMV_ACTION_RECOMMENDATION_NO_RECOMMENDATION;
	final_eval = TNC_IMV_EVALUATION_RESULT_DONT_KNOW;
	if (rec && eval)
	{
		*rec  = final_rec;
		*eval = final_eval;
	}

	if (this->recs->get_count(this->recs) == 0)
	{
		DBG1(DBG_TNC, "there are no IMVs to make a recommendation");
		return TRUE;
	}
	policy = tnc->imvs->get_recommendation_policy(tnc->imvs);

	enumerator = this->recs->create_enumerator(this->recs);
	while (enumerator->enumerate(enumerator, &entry))
	{
		if (!entry->have_recommendation)
		{
			incomplete = TRUE;
			break;
		}
		if (first)
		{
			final_rec = entry->rec;
			final_eval = entry->eval;
			first = FALSE;
			continue;
		}
		switch (policy)
		{
			case RECOMMENDATION_POLICY_DEFAULT:
				final_rec = tncif_policy_update_recommendation(final_rec,
															   entry->rec);
				final_eval = tncif_policy_update_evaluation(final_eval,
															entry->eval);
				break;

			case RECOMMENDATION_POLICY_ALL:
				if (entry->rec != final_rec)
				{
					final_rec = TNC_IMV_ACTION_RECOMMENDATION_NO_RECOMMENDATION;
				}
				if (entry->eval != final_eval)
				{
					final_eval = TNC_IMV_EVALUATION_RESULT_DONT_KNOW;
				}
				break;

			case RECOMMENDATION_POLICY_ANY:
				switch (entry->rec)
				{
					case TNC_IMV_ACTION_RECOMMENDATION_ALLOW:
						final_rec = entry->rec;
						break;
					case TNC_IMV_ACTION_RECOMMENDATION_ISOLATE:
						if (final_rec != TNC_IMV_ACTION_RECOMMENDATION_ALLOW)
						{
							final_rec = entry->rec;
						};
						break;
					case TNC_IMV_ACTION_RECOMMENDATION_NO_ACCESS:
						if (final_rec == TNC_IMV_ACTION_RECOMMENDATION_NO_RECOMMENDATION)
						{
							final_rec = entry->rec;
						};
						break;
					case TNC_IMV_ACTION_RECOMMENDATION_NO_RECOMMENDATION:
						break;
				}
				switch (entry->eval)
				{
					case TNC_IMV_EVALUATION_RESULT_COMPLIANT:
						final_eval = entry->eval;
						break;
					case TNC_IMV_EVALUATION_RESULT_NONCOMPLIANT_MINOR:
						if (final_eval != TNC_IMV_EVALUATION_RESULT_COMPLIANT)
						{
							final_eval = entry->eval;
						}
						break;
					case TNC_IMV_EVALUATION_RESULT_NONCOMPLIANT_MAJOR:
						if (final_eval != TNC_IMV_EVALUATION_RESULT_COMPLIANT &&
							final_eval != TNC_IMV_EVALUATION_RESULT_NONCOMPLIANT_MINOR)
						{
							final_eval = entry->eval;
						}
						break;
					case TNC_IMV_EVALUATION_RESULT_ERROR:
						if (final_eval == TNC_IMV_EVALUATION_RESULT_DONT_KNOW)
						{
							final_eval = entry->eval;
						}
						break;
					case TNC_IMV_EVALUATION_RESULT_DONT_KNOW:
						break;
				}
		}
	}
	enumerator->destroy(enumerator);

	if (incomplete)
	{
		return FALSE;
	}
	if (rec && eval)
	{
		*rec  = final_rec;
		*eval = final_eval;
	}
	return TRUE;
}

METHOD(recommendations_t, clear_recommendation, void,
	private_tnc_imv_recommendations_t *this)
{
	enumerator_t *enumerator;
	recommendation_entry_t *entry;

	enumerator = this->recs->create_enumerator(this->recs);
	while (enumerator->enumerate(enumerator, &entry))
	{
		entry->have_recommendation = FALSE;
		entry->rec = TNC_IMV_ACTION_RECOMMENDATION_NO_RECOMMENDATION;
		entry->eval = TNC_IMV_EVALUATION_RESULT_DONT_KNOW;
		chunk_clear(&entry->reason);
		chunk_clear(&entry->reason_language);
	}
	enumerator->destroy(enumerator);
}

METHOD(recommendations_t, get_preferred_language, chunk_t,
	private_tnc_imv_recommendations_t *this)
{
	return this->preferred_language;
}

METHOD(recommendations_t, set_preferred_language, void,
	private_tnc_imv_recommendations_t *this, chunk_t pref_lang)
{
	free(this->preferred_language.ptr);
	this->preferred_language = chunk_clone(pref_lang);
}

METHOD(recommendations_t, set_reason_string, TNC_Result,
	private_tnc_imv_recommendations_t *this, TNC_IMVID id, chunk_t reason)
{
	enumerator_t *enumerator;
	recommendation_entry_t *entry;
	bool found = FALSE;

	DBG2(DBG_TNC, "IMV %u is setting reason string to '%.*s'",
		 id, (int)reason.len, reason.ptr);

	enumerator = this->recs->create_enumerator(this->recs);
	while (enumerator->enumerate(enumerator, &entry))
	{
		if (entry->id == id)
		{
			found = TRUE;
			free(entry->reason.ptr);
			entry->reason = chunk_clone(reason);
			break;
		}
	}
	enumerator->destroy(enumerator);
	return found ? TNC_RESULT_SUCCESS : TNC_RESULT_INVALID_PARAMETER;
}

METHOD(recommendations_t, set_reason_language, TNC_Result,
	private_tnc_imv_recommendations_t *this, TNC_IMVID id, chunk_t reason_lang)
{
	enumerator_t *enumerator;
	recommendation_entry_t *entry;
	bool found = FALSE;

	DBG2(DBG_TNC, "IMV %u is setting reason language to '%.*s'",
		 id, (int)reason_lang.len, reason_lang.ptr);

	enumerator = this->recs->create_enumerator(this->recs);
	while (enumerator->enumerate(enumerator, &entry))
	{
		if (entry->id == id)
		{
			found = TRUE;
			free(entry->reason_language.ptr);
			entry->reason_language = chunk_clone(reason_lang);
			break;
		}
	}
	enumerator->destroy(enumerator);
	return found ? TNC_RESULT_SUCCESS : TNC_RESULT_INVALID_PARAMETER;
}

CALLBACK(reason_filter, bool,
	void *null, enumerator_t *orig, va_list args)
{
	recommendation_entry_t *entry;
	TNC_IMVID *id;
	chunk_t *reason, *reason_language;

	VA_ARGS_VGET(args, id, reason, reason_language);

	while (orig->enumerate(orig, &entry))
	{
		if (entry->reason.len)
		{
			*id = entry->id;
			*reason = entry->reason;
			*reason_language = entry->reason_language;
			return TRUE;
		}
	}
	return FALSE;
}

METHOD(recommendations_t, create_reason_enumerator, enumerator_t*,
	private_tnc_imv_recommendations_t *this)
{
	return enumerator_create_filter(this->recs->create_enumerator(this->recs),
									reason_filter, NULL, NULL);
}

METHOD(recommendations_t, destroy, void,
	private_tnc_imv_recommendations_t *this)
{
	recommendation_entry_t *entry;

	while (this->recs->remove_last(this->recs, (void**)&entry) == SUCCESS)
	{
		free(entry->reason.ptr);
		free(entry->reason_language.ptr);
		free(entry);
	}
	this->recs->destroy(this->recs);
	free(this->preferred_language.ptr);
	free(this);
}

/**
 * Described in header.
 */
recommendations_t* tnc_imv_recommendations_create(linked_list_t *imv_list)
{
	private_tnc_imv_recommendations_t *this;
	recommendation_entry_t *entry;
	enumerator_t *enumerator;
	imv_t *imv;

	INIT(this,
		.public = {
			.provide_recommendation = _provide_recommendation,
			.have_recommendation = _have_recommendation,
			.clear_recommendation = _clear_recommendation,
			.get_preferred_language = _get_preferred_language,
			.set_preferred_language = _set_preferred_language,
			.set_reason_string = _set_reason_string,
			.set_reason_language = _set_reason_language,
			.create_reason_enumerator = _create_reason_enumerator,
			.destroy = _destroy,
		},
		.recs = linked_list_create(),
	);

	enumerator = imv_list->create_enumerator(imv_list);
	while (enumerator->enumerate(enumerator, &imv))
	{
		entry = malloc_thing(recommendation_entry_t);
		entry->id = imv->get_id(imv);
		entry->have_recommendation = FALSE;
		entry->rec = TNC_IMV_ACTION_RECOMMENDATION_NO_RECOMMENDATION;
		entry->eval = TNC_IMV_EVALUATION_RESULT_DONT_KNOW;
		entry->reason = chunk_empty;
		entry->reason_language = chunk_empty;
		this->recs->insert_last(this->recs, entry);
	}
	enumerator->destroy(enumerator);

	return &this->public;
}
