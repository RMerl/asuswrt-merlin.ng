/*
 * Copyright (C) 2008 Martin Willi
 * Hochschule fuer Technik Rapperswil
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

#include "simaka_manager.h"

#include <utils/debug.h>
#include <collections/linked_list.h>
#include <threading/rwlock.h>

typedef struct private_simaka_manager_t private_simaka_manager_t;

/**
 * Private data of an simaka_manager_t object.
 */
struct private_simaka_manager_t {

	/**
	 * Public simaka_manager_t interface.
	 */
	simaka_manager_t public;

	/**
	 * list of added cards
	 */
	linked_list_t *cards;

	/**
	 * list of added provider
	 */
	linked_list_t *providers;

	/**
	 * list of added hooks
	 */
	linked_list_t *hooks;

	/**
	 * lock for lists above
	 */
	rwlock_t *lock;
};

/**
 * Described in header.
 */
void libsimaka_init(void)
{
	/* empty */
}

METHOD(simaka_manager_t, add_card, void,
	private_simaka_manager_t *this, simaka_card_t *card)
{
	this->lock->write_lock(this->lock);
	this->cards->insert_last(this->cards, card);
	this->lock->unlock(this->lock);
}

METHOD(simaka_manager_t, remove_card, void,
	private_simaka_manager_t *this, simaka_card_t *card)
{
	this->lock->write_lock(this->lock);
	this->cards->remove(this->cards, card, NULL);
	this->lock->unlock(this->lock);
}

METHOD(simaka_manager_t, card_get_triplet, bool,
	private_simaka_manager_t *this, identification_t *id,
	char rand[SIM_RAND_LEN], char sres[SIM_SRES_LEN], char kc[SIM_KC_LEN])
{
	enumerator_t *enumerator;
	simaka_card_t *card;
	int tried = 0;

	this->lock->read_lock(this->lock);
	enumerator = this->cards->create_enumerator(this->cards);
	while (enumerator->enumerate(enumerator, &card))
	{
		if (card->get_triplet(card, id, rand, sres, kc))
		{
			enumerator->destroy(enumerator);
			this->lock->unlock(this->lock);
			return TRUE;
		}
		tried++;
	}
	enumerator->destroy(enumerator);
	this->lock->unlock(this->lock);
	DBG1(DBG_LIB, "tried %d SIM cards, but none has triplets for '%Y'",
		 tried, id);
	return FALSE;
}

METHOD(simaka_manager_t, card_get_quintuplet, status_t,
	private_simaka_manager_t *this, identification_t *id, char rand[AKA_RAND_LEN],
	char autn[AKA_AUTN_LEN], char ck[AKA_CK_LEN], char ik[AKA_IK_LEN],
	char res[AKA_RES_MAX], int *res_len)
{
	enumerator_t *enumerator;
	simaka_card_t *card;
	status_t status = NOT_FOUND;
	int tried = 0;

	this->lock->read_lock(this->lock);
	enumerator = this->cards->create_enumerator(this->cards);
	while (enumerator->enumerate(enumerator, &card))
	{
		status = card->get_quintuplet(card, id, rand, autn, ck, ik, res, res_len);
		switch (status)
		{	/* try next on error, but not on INVALID_STATE */
			case SUCCESS:
			case INVALID_STATE:
				enumerator->destroy(enumerator);
				this->lock->unlock(this->lock);
				return status;
			case NOT_SUPPORTED:
			case FAILED:
			default:
				tried++;
				continue;
		}
	}
	enumerator->destroy(enumerator);
	this->lock->unlock(this->lock);
	DBG1(DBG_LIB, "tried %d SIM cards, but none has quintuplets for '%Y'",
		 tried, id);
	return status;
}

METHOD(simaka_manager_t, card_resync, bool,
	private_simaka_manager_t *this, identification_t *id,
	char rand[AKA_RAND_LEN], char auts[AKA_AUTS_LEN])
{
	enumerator_t *enumerator;
	simaka_card_t *card;

	this->lock->read_lock(this->lock);
	enumerator = this->cards->create_enumerator(this->cards);
	while (enumerator->enumerate(enumerator, &card))
	{
		if (card->resync(card, id, rand, auts))
		{
			enumerator->destroy(enumerator);
			this->lock->unlock(this->lock);
			return TRUE;
		}
	}
	enumerator->destroy(enumerator);
	this->lock->unlock(this->lock);
	return FALSE;
}

METHOD(simaka_manager_t, card_set_pseudonym, void,
	private_simaka_manager_t *this, identification_t *id,
	identification_t *pseudonym)
{
	enumerator_t *enumerator;
	simaka_card_t *card;

	DBG1(DBG_LIB, "storing pseudonym '%Y' for '%Y'", pseudonym, id);

	this->lock->read_lock(this->lock);
	enumerator = this->cards->create_enumerator(this->cards);
	while (enumerator->enumerate(enumerator, &card))
	{
		card->set_pseudonym(card, id, pseudonym);
	}
	enumerator->destroy(enumerator);
	this->lock->unlock(this->lock);
}

METHOD(simaka_manager_t, card_get_pseudonym, identification_t*,
	private_simaka_manager_t *this, identification_t *id)
{
	enumerator_t *enumerator;
	simaka_card_t *card;
	identification_t *pseudonym = NULL;

	this->lock->read_lock(this->lock);
	enumerator = this->cards->create_enumerator(this->cards);
	while (enumerator->enumerate(enumerator, &card))
	{
		pseudonym = card->get_pseudonym(card, id);
		if (pseudonym)
		{
			DBG1(DBG_LIB, "using stored pseudonym identity '%Y' "
				 "instead of '%Y'", pseudonym, id);
			break;
		}
	}
	enumerator->destroy(enumerator);
	this->lock->unlock(this->lock);
	return pseudonym;
}

METHOD(simaka_manager_t, card_set_reauth, void,
	private_simaka_manager_t *this, identification_t *id, identification_t *next,
	char mk[HASH_SIZE_SHA1], u_int16_t counter)
{
	enumerator_t *enumerator;
	simaka_card_t *card;

	DBG1(DBG_LIB, "storing next reauthentication identity '%Y' for '%Y'",
		 next, id);

	this->lock->read_lock(this->lock);
	enumerator = this->cards->create_enumerator(this->cards);
	while (enumerator->enumerate(enumerator, &card))
	{
		card->set_reauth(card, id, next, mk, counter);
	}
	enumerator->destroy(enumerator);
	this->lock->unlock(this->lock);
}

METHOD(simaka_manager_t, card_get_reauth, identification_t*,
	private_simaka_manager_t *this, identification_t *id, char mk[HASH_SIZE_SHA1],
	u_int16_t *counter)
{
	enumerator_t *enumerator;
	simaka_card_t *card;
	identification_t *reauth = NULL;

	this->lock->read_lock(this->lock);
	enumerator = this->cards->create_enumerator(this->cards);
	while (enumerator->enumerate(enumerator, &card))
	{
		reauth = card->get_reauth(card, id, mk, counter);
		if (reauth)
		{
			DBG1(DBG_LIB, "using stored reauthentication identity '%Y' "
				 "instead of '%Y'", reauth, id);
			break;
		}
	}
	enumerator->destroy(enumerator);
	this->lock->unlock(this->lock);
	return reauth;
}

METHOD(simaka_manager_t, add_provider, void,
	private_simaka_manager_t *this, simaka_provider_t *provider)
{
	this->lock->write_lock(this->lock);
	this->providers->insert_last(this->providers, provider);
	this->lock->unlock(this->lock);
}

METHOD(simaka_manager_t, remove_provider, void,
	private_simaka_manager_t *this, simaka_provider_t *provider)
{
	this->lock->write_lock(this->lock);
	this->providers->remove(this->providers, provider, NULL);
	this->lock->unlock(this->lock);
}

METHOD(simaka_manager_t, provider_get_triplet, bool,
	private_simaka_manager_t *this, identification_t *id,
	char rand[SIM_RAND_LEN], char sres[SIM_SRES_LEN], char kc[SIM_KC_LEN])
{
	enumerator_t *enumerator;
	simaka_provider_t *provider;
	int tried = 0;

	this->lock->read_lock(this->lock);
	enumerator = this->providers->create_enumerator(this->providers);
	while (enumerator->enumerate(enumerator, &provider))
	{
		if (provider->get_triplet(provider, id, rand, sres, kc))
		{
			enumerator->destroy(enumerator);
			this->lock->unlock(this->lock);
			return TRUE;
		}
		tried++;
	}
	enumerator->destroy(enumerator);
	this->lock->unlock(this->lock);
	DBG1(DBG_LIB, "tried %d SIM providers, but none had a triplet for '%Y'",
		 tried, id);
	return FALSE;
}

METHOD(simaka_manager_t, provider_get_quintuplet, bool,
	private_simaka_manager_t *this, identification_t *id,
	char rand[AKA_RAND_LEN], char xres[AKA_RES_MAX], int *xres_len,
	char ck[AKA_CK_LEN], char ik[AKA_IK_LEN], char autn[AKA_AUTN_LEN])
{
	enumerator_t *enumerator;
	simaka_provider_t *provider;
	int tried = 0;

	this->lock->read_lock(this->lock);
	enumerator = this->providers->create_enumerator(this->providers);
	while (enumerator->enumerate(enumerator, &provider))
	{
		if (provider->get_quintuplet(provider, id, rand, xres, xres_len,
									 ck, ik, autn))
		{
			enumerator->destroy(enumerator);
			this->lock->unlock(this->lock);
			return TRUE;
		}
	}
	enumerator->destroy(enumerator);
	this->lock->unlock(this->lock);
	DBG1(DBG_LIB, "tried %d SIM providers, but none had a quintuplet for '%Y'",
		 tried, id);
	return FALSE;
}

METHOD(simaka_manager_t, provider_resync, bool,
	private_simaka_manager_t *this, identification_t *id,
	char rand[AKA_RAND_LEN], char auts[AKA_AUTS_LEN])
{
	enumerator_t *enumerator;
	simaka_provider_t *provider;

	this->lock->read_lock(this->lock);
	enumerator = this->providers->create_enumerator(this->providers);
	while (enumerator->enumerate(enumerator, &provider))
	{
		if (provider->resync(provider, id, rand, auts))
		{
			enumerator->destroy(enumerator);
			this->lock->unlock(this->lock);
			return TRUE;
		}
	}
	enumerator->destroy(enumerator);
	this->lock->unlock(this->lock);
	return FALSE;
}

METHOD(simaka_manager_t, provider_is_pseudonym, identification_t*,
	private_simaka_manager_t *this, identification_t *id)
{
	enumerator_t *enumerator;
	simaka_provider_t *provider;
	identification_t *permanent = NULL;

	this->lock->read_lock(this->lock);
	enumerator = this->providers->create_enumerator(this->providers);
	while (enumerator->enumerate(enumerator, &provider))
	{
		permanent = provider->is_pseudonym(provider, id);
		if (permanent)
		{
			DBG1(DBG_LIB, "received pseudonym identity '%Y' "
				 "mapping to '%Y'", id, permanent);
			break;
		}
	}
	enumerator->destroy(enumerator);
	this->lock->unlock(this->lock);
	return permanent;
}

METHOD(simaka_manager_t, provider_gen_pseudonym, identification_t*,
	private_simaka_manager_t *this, identification_t *id)
{
	enumerator_t *enumerator;
	simaka_provider_t *provider;
	identification_t *pseudonym = NULL;

	this->lock->read_lock(this->lock);
	enumerator = this->providers->create_enumerator(this->providers);
	while (enumerator->enumerate(enumerator, &provider))
	{
		pseudonym = provider->gen_pseudonym(provider, id);
		if (pseudonym)
		{
			DBG1(DBG_LIB, "proposing new pseudonym '%Y'", pseudonym);
			break;
		}
	}
	enumerator->destroy(enumerator);
	this->lock->unlock(this->lock);
	return pseudonym;
}

METHOD(simaka_manager_t, provider_is_reauth, identification_t*,
	private_simaka_manager_t *this, identification_t *id, char mk[HASH_SIZE_SHA1],
	u_int16_t *counter)
{
	enumerator_t *enumerator;
	simaka_provider_t *provider;
	identification_t *permanent = NULL;

	this->lock->read_lock(this->lock);
	enumerator = this->providers->create_enumerator(this->providers);
	while (enumerator->enumerate(enumerator, &provider))
	{
		permanent = provider->is_reauth(provider, id, mk, counter);
		if (permanent)
		{
			DBG1(DBG_LIB, "received reauthentication identity '%Y' "
				 "mapping to '%Y'", id, permanent);
			break;
		}
	}
	enumerator->destroy(enumerator);
	this->lock->unlock(this->lock);
	return permanent;
}

METHOD(simaka_manager_t, provider_gen_reauth, identification_t*,
	private_simaka_manager_t *this, identification_t *id, char mk[HASH_SIZE_SHA1])
{
	enumerator_t *enumerator;
	simaka_provider_t *provider;
	identification_t *reauth = NULL;

	this->lock->read_lock(this->lock);
	enumerator = this->providers->create_enumerator(this->providers);
	while (enumerator->enumerate(enumerator, &provider))
	{
		reauth = provider->gen_reauth(provider, id, mk);
		if (reauth)
		{
			DBG1(DBG_LIB, "proposing new reauthentication identity '%Y'", reauth);
			break;
		}
	}
	enumerator->destroy(enumerator);
	this->lock->unlock(this->lock);
	return reauth;
}

METHOD(simaka_manager_t, add_hooks, void,
	private_simaka_manager_t *this, simaka_hooks_t *hooks)
{
	this->lock->write_lock(this->lock);
	this->hooks->insert_last(this->hooks, hooks);
	this->lock->unlock(this->lock);
}

METHOD(simaka_manager_t, remove_hooks, void,
	private_simaka_manager_t *this, simaka_hooks_t *hooks)
{
	this->lock->write_lock(this->lock);
	this->hooks->remove(this->hooks, hooks, NULL);
	this->lock->unlock(this->lock);
}

METHOD(simaka_manager_t, message_hook, void,
	private_simaka_manager_t *this, simaka_message_t *message,
	bool inbound, bool decrypted)
{
	enumerator_t *enumerator;
	simaka_hooks_t *hooks;

	this->lock->read_lock(this->lock);
	enumerator = this->hooks->create_enumerator(this->hooks);
	while (enumerator->enumerate(enumerator, &hooks))
	{
		hooks->message(hooks, message, inbound, decrypted);
	}
	enumerator->destroy(enumerator);
	this->lock->unlock(this->lock);
}

METHOD(simaka_manager_t, key_hook, void,
	private_simaka_manager_t *this, chunk_t k_encr, chunk_t k_auth)
{
	enumerator_t *enumerator;
	simaka_hooks_t *hooks;

	this->lock->read_lock(this->lock);
	enumerator = this->hooks->create_enumerator(this->hooks);
	while (enumerator->enumerate(enumerator, &hooks))
	{
		hooks->keys(hooks, k_encr, k_auth);
	}
	enumerator->destroy(enumerator);
	this->lock->unlock(this->lock);
}

METHOD(simaka_manager_t, destroy, void,
	private_simaka_manager_t *this)
{
	this->cards->destroy(this->cards);
	this->providers->destroy(this->providers);
	this->hooks->destroy(this->hooks);
	this->lock->destroy(this->lock);
	free(this);
}

/**
 * See header
 */
simaka_manager_t *simaka_manager_create()
{
	private_simaka_manager_t *this;

	INIT(this,
		.public = {
			.add_card = _add_card,
			.remove_card = _remove_card,
			.card_get_triplet = _card_get_triplet,
			.card_get_quintuplet = _card_get_quintuplet,
			.card_resync = _card_resync,
			.card_set_pseudonym = _card_set_pseudonym,
			.card_get_pseudonym = _card_get_pseudonym,
			.card_set_reauth = _card_set_reauth,
			.card_get_reauth = _card_get_reauth,
			.add_provider = _add_provider,
			.remove_provider = _remove_provider,
			.provider_get_triplet = _provider_get_triplet,
			.provider_get_quintuplet = _provider_get_quintuplet,
			.provider_resync = _provider_resync,
			.provider_is_pseudonym = _provider_is_pseudonym,
			.provider_gen_pseudonym = _provider_gen_pseudonym,
			.provider_is_reauth = _provider_is_reauth,
			.provider_gen_reauth = _provider_gen_reauth,
			.add_hooks = _add_hooks,
			.remove_hooks = _remove_hooks,
			.message_hook = _message_hook,
			.key_hook = _key_hook,
			.destroy = _destroy,
		},
		.cards = linked_list_create(),
		.providers = linked_list_create(),
		.hooks = linked_list_create(),
		.lock = rwlock_create(RWLOCK_TYPE_DEFAULT),
	);

	return &this->public;
}

/**
 * (Un-)register a provider to a simaka manager
 */
static bool register_card(char *mgr_name, bool reg, simaka_card_t *card)
{
	simaka_manager_t *mgr;

	if (!card)
	{
		return FALSE;
	}
	mgr = lib->get(lib, mgr_name);
	if (mgr)
	{
		if (reg)
		{
			mgr->add_card(mgr, card);
		}
		else
		{
			mgr->remove_card(mgr, card);
		}
		return TRUE;
	}
	return FALSE;
}

/**
 * (Un-)register a provider to a simaka manager
 */
static bool register_provider(char *mgr_name, bool reg,
							  simaka_provider_t *provider)
{
	simaka_manager_t *mgr;

	if (!provider)
	{
		return FALSE;
	}
	mgr = lib->get(lib, mgr_name);
	if (mgr)
	{
		if (reg)
		{
			mgr->add_provider(mgr, provider);
		}
		else
		{
			mgr->remove_provider(mgr, provider);
		}
		return TRUE;
	}
	return FALSE;
}

/**
 * See header
 */
bool simaka_manager_register(plugin_t *plugin, plugin_feature_t *feature,
							 bool reg, void *data)
{
	simaka_manager_register_cb_t get = (simaka_manager_register_cb_t)data;

	if (feature->type == FEATURE_CUSTOM)
	{
		if (streq(feature->arg.custom, "aka-card"))
		{
			return register_card("aka-manager", reg, get(plugin));
		}
		else if (streq(feature->arg.custom, "aka-provider"))
		{
			return register_provider("aka-manager", reg, get(plugin));
		}
		else if (streq(feature->arg.custom, "sim-card"))
		{
			return register_card("sim-manager", reg, get(plugin));
		}
		else if (streq(feature->arg.custom, "sim-provider"))
		{
			return register_provider("sim-manager", reg, get(plugin));
		}
	}
	return FALSE;
}
