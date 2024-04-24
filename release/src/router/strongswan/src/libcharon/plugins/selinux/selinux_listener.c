/*
 * Copyright (C) 2022 Tobias Brunner
 *
 * Copyright (C) secunet Security Networks AG
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

#include "selinux_listener.h"

#include <daemon.h>
#include <collections/array.h>
#include <collections/hashtable.h>

typedef struct private_selinux_listener_t private_selinux_listener_t;

/**
 * Private data.
 */
struct private_selinux_listener_t {

	/**
	 * Public interface.
	 */
	selinux_listener_t public;

	/**
	 * IKE_SAs with attached trap policies, ike_sa_id_t => entry_t.
	 */
	hashtable_t *sas;
};

/**
 * Entry to keep track of trap policies.
 */
typedef struct {

	/**
	 * IKE_SA ID.
	 */
	ike_sa_id_t *id;

	/**
	 * Installed trap policies.
	 */
	array_t *traps;

} entry_t;

/**
 * Destroy the given entry.
 */
static void destroy_entry(entry_t *entry)
{
	entry->id->destroy(entry->id);
	array_destroy(entry->traps);
	free(entry);
}

/**
 * Hashtable hash function
 */
static u_int hash(const void *key)
{
	ike_sa_id_t *id = (ike_sa_id_t*)key;
	uint64_t spi_i = id->get_initiator_spi(id),
			 spi_r = id->get_responder_spi(id);
	return chunk_hash_inc(chunk_from_thing(spi_i),
						  chunk_hash(chunk_from_thing(spi_r)));
}

/**
 * Hashtable equals function
 */
static bool equals(const void *a_pub, const void *b)
{
	ike_sa_id_t *a = (ike_sa_id_t*)a_pub;
	return a->equals(a, (ike_sa_id_t*)b);
}

/**
 * Install a trap policy for the generic SELinux label.
 */
static bool install_generic_trap(ike_sa_t *ike_sa, child_sa_t *child_sa)
{
	linked_list_t *local, *remote;
	bool success;

#if DEBUG_LEVEL >= 1
	sec_label_t *label = child_sa->get_label(child_sa);
	DBG1(DBG_IKE, "installing trap %s{%d} with generic security label '%s'",
		 child_sa->get_name(child_sa), child_sa->get_unique_id(child_sa),
		 label->get_string(label));
#endif

	local = ike_sa_get_dynamic_hosts(ike_sa, TRUE);
	remote = ike_sa_get_dynamic_hosts(ike_sa, FALSE);
	success = charon->traps->install_external(charon->traps,
											  ike_sa->get_peer_cfg(ike_sa),
											  child_sa, local, remote);
	local->destroy(local);
	remote->destroy(remote);
	return success;
}

METHOD(listener_t, ike_updown, bool,
	private_selinux_listener_t *this, ike_sa_t *ike_sa, bool up)
{
	enumerator_t *enumerator;
	peer_cfg_t *peer_cfg;
	child_cfg_t *child_cfg;
	child_sa_t *child_sa;
	entry_t *entry;

	if (up)
	{
		child_sa_create_t child = {
			.if_id_in_def = ike_sa->get_if_id(ike_sa, TRUE),
			.if_id_out_def = ike_sa->get_if_id(ike_sa, FALSE),
		};

		INIT(entry,
			.id = ike_sa->get_id(ike_sa),
		);
		entry->id = entry->id->clone(entry->id);

		peer_cfg = ike_sa->get_peer_cfg(ike_sa);
		enumerator = peer_cfg->create_child_cfg_enumerator(peer_cfg);
		while (enumerator->enumerate(enumerator, &child_cfg))
		{
			if (child_cfg->get_label(child_cfg) &&
				child_cfg->get_label_mode(child_cfg) == SEC_LABEL_MODE_SELINUX)
			{
				child_sa = child_sa_create(ike_sa->get_my_host(ike_sa),
										   ike_sa->get_other_host(ike_sa),
										   child_cfg, &child);
				if (install_generic_trap(ike_sa, child_sa))
				{
					array_insert_create(&entry->traps, ARRAY_TAIL, child_sa);
				}
				else
				{
					child_sa->destroy(child_sa);
				}
			}
		}
		enumerator->destroy(enumerator);

		if (array_count(entry->traps))
		{
			this->sas->put(this->sas, entry->id, entry);
		}
		else
		{
			destroy_entry(entry);
		}
	}
	else
	{
		entry = this->sas->remove(this->sas, ike_sa->get_id(ike_sa));
		if (entry)
		{
			while (array_remove(entry->traps, ARRAY_TAIL, &child_sa))
			{
#if DEBUG_LEVEL >= 1
				sec_label_t *label = child_sa->get_label(child_sa);
				DBG1(DBG_IKE, "uninstalling trap %s{%d} with generic security "
					 "label '%s'", child_sa->get_name(child_sa),
					 child_sa->get_unique_id(child_sa),
					 label->get_string(label));
#endif
				charon->traps->remove_external(charon->traps, child_sa);
				child_sa->destroy(child_sa);
			}
			destroy_entry(entry);
		}
	}
	return TRUE;
}

METHOD(listener_t, ike_rekey, bool,
	private_selinux_listener_t *this, ike_sa_t *old, ike_sa_t *new)
{
	entry_t *entry;

	entry = this->sas->remove(this->sas, old->get_id(old));
	if (entry)
	{
		entry->id->destroy(entry->id);
		entry->id = new->get_id(new);
		entry->id = entry->id->clone(entry->id);
		this->sas->put(this->sas, entry->id, entry);
	}
	return TRUE;
}

METHOD(listener_t, ike_update, bool,
	private_selinux_listener_t *this, ike_sa_t *ike_sa,
	host_t *local, host_t *remote)
{
	entry_t *entry;
	child_sa_t *child_sa;
	linked_list_t *vips;
	int i;

	entry = this->sas->get(this->sas, ike_sa->get_id(ike_sa));
	if (entry)
	{
		vips = linked_list_create_from_enumerator(
						ike_sa->create_virtual_ip_enumerator(ike_sa, local));
		for (i = 0; i < array_count(entry->traps); i++)
		{
			array_get(entry->traps, i, &child_sa);
			child_sa->update(child_sa, local, remote, vips,
							 ike_sa->has_condition(ike_sa, COND_NAT_ANY));
		}
		vips->destroy(vips);
	}
	return TRUE;
}

METHOD(selinux_listener_t, destroy, void,
	private_selinux_listener_t *this)
{
	this->sas->destroy(this->sas);
	free(this);
}

/*
 * Described in header
 */
selinux_listener_t *selinux_listener_create()
{
	private_selinux_listener_t *this;

	INIT(this,
		.public = {
			.listener = {
				.ike_updown = _ike_updown,
				.ike_rekey = _ike_rekey,
				.ike_update = _ike_update,
			},
			.destroy = _destroy,
		},
		.sas = hashtable_create(hash, equals, 32),
	);

	return &this->public;
}
