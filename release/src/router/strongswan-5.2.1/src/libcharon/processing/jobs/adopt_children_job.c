/*
 * Copyright (C) 2012 Martin Willi
 * Copyright (C) 2012 revosec AG
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

#include "adopt_children_job.h"

#include <daemon.h>
#include <hydra.h>
#include <collections/array.h>

typedef struct private_adopt_children_job_t private_adopt_children_job_t;

/**
 * Private data of an adopt_children_job_t object.
 */
struct private_adopt_children_job_t {

	/**
	 * Public adopt_children_job_t interface.
	 */
	adopt_children_job_t public;

	/**
	 * IKE_SA id to adopt children from
	 */
	ike_sa_id_t *id;

	/**
	 * Tasks queued for execution
	 */
	array_t *tasks;
};

METHOD(job_t, destroy, void,
	private_adopt_children_job_t *this)
{
	array_destroy_offset(this->tasks, offsetof(task_t, destroy));
	this->id->destroy(this->id);
	free(this);
}

METHOD(job_t, execute, job_requeue_t,
	private_adopt_children_job_t *this)
{
	identification_t *my_id, *other_id, *xauth;
	host_t *me, *other;
	peer_cfg_t *cfg;
	linked_list_t *children;
	enumerator_t *enumerator, *childenum;
	ike_sa_id_t *id;
	ike_sa_t *ike_sa;
	child_sa_t *child_sa;

	ike_sa = charon->ike_sa_manager->checkout(charon->ike_sa_manager, this->id);
	if (ike_sa)
	{
		/* get what we need from new SA */
		me = ike_sa->get_my_host(ike_sa);
		me = me->clone(me);
		other = ike_sa->get_other_host(ike_sa);
		other = other->clone(other);
		my_id = ike_sa->get_my_id(ike_sa);
		my_id = my_id->clone(my_id);
		other_id = ike_sa->get_other_id(ike_sa);
		other_id = other_id->clone(other_id);
		xauth = ike_sa->get_other_eap_id(ike_sa);
		xauth = xauth->clone(xauth);
		cfg = ike_sa->get_peer_cfg(ike_sa);
		cfg->get_ref(cfg);

		charon->ike_sa_manager->checkin(charon->ike_sa_manager, ike_sa);

		/* find old SA to adopt children from */
		children = linked_list_create();
		enumerator = charon->ike_sa_manager->create_id_enumerator(
									charon->ike_sa_manager, my_id, xauth,
									other->get_family(other));
		while (enumerator->enumerate(enumerator, &id))
		{
			if (id->equals(id, this->id))
			{	/* not from self */
				continue;
			}
			ike_sa = charon->ike_sa_manager->checkout(charon->ike_sa_manager, id);
			if (ike_sa)
			{
				if ((ike_sa->get_state(ike_sa) == IKE_ESTABLISHED ||
					 ike_sa->get_state(ike_sa) == IKE_PASSIVE) &&
					me->equals(me, ike_sa->get_my_host(ike_sa)) &&
					other->equals(other, ike_sa->get_other_host(ike_sa)) &&
					other_id->equals(other_id, ike_sa->get_other_id(ike_sa)) &&
					cfg->equals(cfg, ike_sa->get_peer_cfg(ike_sa)))
				{
					childenum = ike_sa->create_child_sa_enumerator(ike_sa);
					while (childenum->enumerate(childenum, &child_sa))
					{
						ike_sa->remove_child_sa(ike_sa, childenum);
						children->insert_last(children, child_sa);
					}
					childenum->destroy(childenum);
					if (children->get_count(children))
					{
						DBG1(DBG_IKE, "detected reauth of existing IKE_SA, "
							 "adopting %d children",
							 children->get_count(children));
					}
					ike_sa->set_state(ike_sa, IKE_DELETING);
					charon->bus->ike_updown(charon->bus, ike_sa, FALSE);
					charon->ike_sa_manager->checkin_and_destroy(
											charon->ike_sa_manager, ike_sa);
				}
				else
				{
					charon->ike_sa_manager->checkin(
											charon->ike_sa_manager, ike_sa);
				}
				if (children->get_count(children))
				{
					break;
				}
			}
		}
		enumerator->destroy(enumerator);

		me->destroy(me);
		other->destroy(other);
		my_id->destroy(my_id);
		other_id->destroy(other_id);
		xauth->destroy(xauth);
		cfg->destroy(cfg);

		if (children->get_count(children))
		{
			/* adopt children by new SA */
			ike_sa = charon->ike_sa_manager->checkout(charon->ike_sa_manager,
													  this->id);
			if (ike_sa)
			{
				while (children->remove_last(children,
											 (void**)&child_sa) == SUCCESS)
				{
					ike_sa->add_child_sa(ike_sa, child_sa);
				}
				charon->ike_sa_manager->checkin(charon->ike_sa_manager, ike_sa);
			}
		}
		children->destroy_offset(children, offsetof(child_sa_t, destroy));

		if (array_count(this->tasks))
		{
			ike_sa = charon->ike_sa_manager->checkout(charon->ike_sa_manager,
													  this->id);
			if (ike_sa)
			{
				task_t *task;

				while (array_remove(this->tasks, ARRAY_HEAD, &task))
				{
					task->migrate(task, ike_sa);
					ike_sa->queue_task(ike_sa, task);
				}
				if (ike_sa->initiate(ike_sa, NULL, 0, NULL, NULL) == DESTROY_ME)
				{
					charon->ike_sa_manager->checkin_and_destroy(
											charon->ike_sa_manager, ike_sa);
				}
				else
				{
					charon->ike_sa_manager->checkin(charon->ike_sa_manager,
													ike_sa);
				}
			}
		}
	}
	return JOB_REQUEUE_NONE;
}

METHOD(job_t, get_priority, job_priority_t,
	private_adopt_children_job_t *this)
{
	return JOB_PRIO_HIGH;
}

METHOD(adopt_children_job_t, queue_task, void,
	private_adopt_children_job_t *this, task_t *task)
{
	array_insert_create(&this->tasks, ARRAY_TAIL, task);
}

/**
 * See header
 */
adopt_children_job_t *adopt_children_job_create(ike_sa_id_t *id)
{
	private_adopt_children_job_t *this;

	INIT(this,
		.public = {
			.job_interface = {
				.execute = _execute,
				.get_priority = _get_priority,
				.destroy = _destroy,
			},
			.queue_task = _queue_task,
		},
		.id = id->clone(id),
	);

	return &this->public;
}
