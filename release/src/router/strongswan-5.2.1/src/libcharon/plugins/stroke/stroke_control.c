/*
 * Copyright (C) 2013 Tobias Brunner
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

#include "stroke_control.h"

#include <hydra.h>
#include <daemon.h>

#include <processing/jobs/delete_ike_sa_job.h>
#include <processing/jobs/rekey_ike_sa_job.h>
#include <processing/jobs/rekey_child_sa_job.h>

typedef struct private_stroke_control_t private_stroke_control_t;

/**
 * private data of stroke_control
 */
struct private_stroke_control_t {

	/**
	 * public functions
	 */
	stroke_control_t public;

	/**
	 * Timeout for stroke commands, im ms
	 */
	u_int timeout;
};


typedef struct stroke_log_info_t stroke_log_info_t;

/**
 * helper struct to say what and where to log when using controller callback
 */
struct stroke_log_info_t {

	/**
	 * level to log up to
	 */
	level_t level;

	/**
	 * where to write log
	 */
	FILE* out;
};

/**
 * logging to the stroke interface
 */
static bool stroke_log(stroke_log_info_t *info, debug_t group, level_t level,
					   ike_sa_t *ike_sa, char *message)
{
	if (level <= info->level)
	{
		if (fprintf(info->out, "%s", message) < 0 ||
			fprintf(info->out, "\n") < 0 ||
			fflush(info->out) != 0)
		{
			return FALSE;
		}
	}
	return TRUE;
}

/**
 * get the child_cfg with the same name as the peer cfg
 */
static child_cfg_t* get_child_from_peer(peer_cfg_t *peer_cfg, char *name)
{
	child_cfg_t *current, *found = NULL;
	enumerator_t *enumerator;

	enumerator = peer_cfg->create_child_cfg_enumerator(peer_cfg);
	while (enumerator->enumerate(enumerator, &current))
	{
		if (streq(current->get_name(current), name))
		{
			found = current;
			found->get_ref(found);
			break;
		}
	}
	enumerator->destroy(enumerator);
	return found;
}

/**
 * call the charon controller to initiate the connection
 */
static void charon_initiate(private_stroke_control_t *this, peer_cfg_t *peer_cfg,
							child_cfg_t *child_cfg, stroke_msg_t *msg, FILE *out)
{
	if (msg->output_verbosity < 0)
	{
		charon->controller->initiate(charon->controller, peer_cfg, child_cfg,
									 NULL, NULL, 0);
	}
	else
	{
		stroke_log_info_t info = { msg->output_verbosity, out };
		status_t status;

		status = charon->controller->initiate(charon->controller,
							peer_cfg, child_cfg, (controller_cb_t)stroke_log,
							&info, this->timeout);
		switch (status)
		{
			case SUCCESS:
				fprintf(out, "connection '%s' established successfully\n",
						msg->initiate.name);
				break;
			case OUT_OF_RES:
				fprintf(out, "connection '%s' not established after %dms, "
						"detaching\n", msg->initiate.name, this->timeout);
				break;
			default:
			case FAILED:
				fprintf(out, "establishing connection '%s' failed\n",
						msg->initiate.name);
				break;
		}
	}
}

METHOD(stroke_control_t, initiate, void,
	private_stroke_control_t *this, stroke_msg_t *msg, FILE *out)
{
	child_cfg_t *child_cfg = NULL;
	peer_cfg_t *peer_cfg;
	enumerator_t *enumerator;
	bool empty = TRUE;

	peer_cfg = charon->backends->get_peer_cfg_by_name(charon->backends,
													  msg->initiate.name);
	if (peer_cfg)
	{
		child_cfg = get_child_from_peer(peer_cfg, msg->initiate.name);
		if (child_cfg == NULL)
		{
			enumerator = peer_cfg->create_child_cfg_enumerator(peer_cfg);
			while (enumerator->enumerate(enumerator, &child_cfg))
			{
				empty = FALSE;
				charon_initiate(this, peer_cfg->get_ref(peer_cfg),
								child_cfg->get_ref(child_cfg), msg, out);
			}
			enumerator->destroy(enumerator);

			if (empty)
			{
				DBG1(DBG_CFG, "no child config named '%s'", msg->initiate.name);
				fprintf(out, "no child config named '%s'\n", msg->initiate.name);
			}
			peer_cfg->destroy(peer_cfg);
			return;
		}
	}
	else
	{
		enumerator = charon->backends->create_peer_cfg_enumerator(
							charon->backends, NULL, NULL, NULL, NULL, IKE_ANY);
		while (enumerator->enumerate(enumerator, &peer_cfg))
		{
			child_cfg = get_child_from_peer(peer_cfg, msg->initiate.name);
			if (child_cfg)
			{
				peer_cfg->get_ref(peer_cfg);
				break;
			}
		}
		enumerator->destroy(enumerator);

		if (child_cfg == NULL)
		{
			DBG1(DBG_CFG, "no config named '%s'", msg->initiate.name);
			fprintf(out, "no config named '%s'\n", msg->initiate.name);
			return;
		}
	}
	charon_initiate(this, peer_cfg, child_cfg, msg, out);
}

/**
 * Parse a terminate/rekey specifier
 */
static bool parse_specifier(char *string, u_int32_t *id,
							char **name, bool *child, bool *all)
{
	int len;
	char *pos = NULL;

	*id = 0;
	*name = NULL;
	*all = FALSE;

	len = strlen(string);
	if (len < 1)
	{
		return FALSE;
	}
	switch (string[len-1])
	{
		case '}':
			*child = TRUE;
			pos = strchr(string, '{');
			break;
		case ']':
			*child = FALSE;
			pos = strchr(string, '[');
			break;
		default:
			*name = string;
			*child = FALSE;
			break;
	}

	if (*name)
	{
		/* is a single name */
	}
	else if (pos == string + len - 2)
	{	/* is name[] or name{} */
		string[len-2] = '\0';
		*name = string;
	}
	else
	{
		if (!pos)
		{
			return FALSE;
		}
		if (*(pos + 1) == '*')
		{	/* is name[*] */
			*all = TRUE;
			*pos = '\0';
			*name = string;
		}
		else
		{	/* is name[123] or name{23} */
			*id = atoi(pos + 1);
			if (*id == 0)
			{
				return FALSE;
			}
		}
	}
	return TRUE;
}

/**
 * Report the result of a terminate() call to console
 */
static void report_terminate_status(private_stroke_control_t *this,
						status_t status, FILE *out, u_int32_t id, bool child)
{
	char *prefix, *postfix;

	if (child)
	{
		prefix = "CHILD_SA {";
		postfix = "}";
	}
	else
	{
		prefix = "IKE_SA [";
		postfix = "]";
	}

	switch (status)
	{
		case SUCCESS:
			fprintf(out, "%s%d%s closed successfully\n", prefix, id, postfix);
			break;
		case OUT_OF_RES:
			fprintf(out, "%s%d%s not closed after %dms, detaching\n",
					prefix, id, postfix, this->timeout);
			break;
		default:
		case FAILED:
			fprintf(out, "closing %s%d%s failed\n", prefix, id, postfix);
			break;
	}
}

METHOD(stroke_control_t, terminate, void,
	private_stroke_control_t *this, stroke_msg_t *msg, FILE *out)
{
	char *name;
	u_int32_t id;
	bool child, all;
	ike_sa_t *ike_sa;
	enumerator_t *enumerator;
	linked_list_t *ike_list, *child_list;
	stroke_log_info_t info;
	uintptr_t del;
	status_t status;

	if (!parse_specifier(msg->terminate.name, &id, &name, &child, &all))
	{
		DBG1(DBG_CFG, "error parsing specifier string");
		return;
	}

	info.out = out;
	info.level = msg->output_verbosity;

	if (id)
	{
		if (child)
		{
			status = charon->controller->terminate_child(charon->controller, id,
							(controller_cb_t)stroke_log, &info, this->timeout);
		}
		else
		{
			status = charon->controller->terminate_ike(charon->controller, id,
							(controller_cb_t)stroke_log, &info, this->timeout);
		}
		return report_terminate_status(this, status, out, id, child);
	}

	ike_list = linked_list_create();
	child_list = linked_list_create();
	enumerator = charon->controller->create_ike_sa_enumerator(
													charon->controller, TRUE);
	while (enumerator->enumerate(enumerator, &ike_sa))
	{
		child_sa_t *child_sa;
		enumerator_t *children;

		if (child)
		{
			children = ike_sa->create_child_sa_enumerator(ike_sa);
			while (children->enumerate(children, (void**)&child_sa))
			{
				if (streq(name, child_sa->get_name(child_sa)))
				{
					child_list->insert_last(child_list,
							(void*)(uintptr_t)child_sa->get_reqid(child_sa));
					if (!all)
					{
						break;
					}
				}
			}
			children->destroy(children);
			if (child_list->get_count(child_list) && !all)
			{
				break;
			}
		}
		else if (streq(name, ike_sa->get_name(ike_sa)))
		{
			ike_list->insert_last(ike_list,
						(void*)(uintptr_t)ike_sa->get_unique_id(ike_sa));
			if (!all)
			{
				break;
			}
		}
	}
	enumerator->destroy(enumerator);

	enumerator = child_list->create_enumerator(child_list);
	while (enumerator->enumerate(enumerator, &del))
	{
		status = charon->controller->terminate_child(charon->controller, del,
							(controller_cb_t)stroke_log, &info, this->timeout);
		report_terminate_status(this, status, out, del, TRUE);
	}
	enumerator->destroy(enumerator);

	enumerator = ike_list->create_enumerator(ike_list);
	while (enumerator->enumerate(enumerator, &del))
	{
		status = charon->controller->terminate_ike(charon->controller, del,
							(controller_cb_t)stroke_log, &info, this->timeout);
		report_terminate_status(this, status, out, del, FALSE);
	}
	enumerator->destroy(enumerator);

	if (child_list->get_count(child_list) == 0 &&
		ike_list->get_count(ike_list) == 0)
	{
		DBG1(DBG_CFG, "no %s_SA named '%s' found",
			 child ? "CHILD" : "IKE", name);
	}
	ike_list->destroy(ike_list);
	child_list->destroy(child_list);
}

METHOD(stroke_control_t, rekey, void,
	private_stroke_control_t *this, stroke_msg_t *msg, FILE *out)
{
	char *name;
	u_int32_t id;
	bool child, all, finished = FALSE;
	ike_sa_t *ike_sa;
	enumerator_t *enumerator;

	if (!parse_specifier(msg->terminate.name, &id, &name, &child, &all))
	{
		DBG1(DBG_CFG, "error parsing specifier string");
		return;
	}
	enumerator = charon->controller->create_ike_sa_enumerator(
													charon->controller, TRUE);
	while (enumerator->enumerate(enumerator, &ike_sa))
	{
		child_sa_t *child_sa;
		enumerator_t *children;

		if (child)
		{
			children = ike_sa->create_child_sa_enumerator(ike_sa);
			while (children->enumerate(children, (void**)&child_sa))
			{
				if ((name && streq(name, child_sa->get_name(child_sa))) ||
					(id && id == child_sa->get_reqid(child_sa)))
				{
					lib->processor->queue_job(lib->processor,
						(job_t*)rekey_child_sa_job_create(
								child_sa->get_reqid(child_sa),
								child_sa->get_protocol(child_sa),
								child_sa->get_spi(child_sa, TRUE)));
					if (!all)
					{
						finished = TRUE;
						break;
					}
				}
			}
			children->destroy(children);
		}
		else if ((name && streq(name, ike_sa->get_name(ike_sa))) ||
				 (id && id == ike_sa->get_unique_id(ike_sa)))
		{
			lib->processor->queue_job(lib->processor,
				(job_t*)rekey_ike_sa_job_create(ike_sa->get_id(ike_sa), FALSE));
			if (!all)
			{
				finished = TRUE;
			}
		}
		if (finished)
		{
			break;
		}
	}
	enumerator->destroy(enumerator);
}

METHOD(stroke_control_t, terminate_srcip, void,
	private_stroke_control_t *this, stroke_msg_t *msg, FILE *out)
{
	enumerator_t *enumerator, *vips;
	ike_sa_t *ike_sa;
	host_t *start = NULL, *end = NULL, *vip;
	chunk_t chunk_start, chunk_end = chunk_empty, chunk;

	if (msg->terminate_srcip.start)
	{
		start = host_create_from_string(msg->terminate_srcip.start, 0);
	}
	if (!start)
	{
		DBG1(DBG_CFG, "invalid start address: %s", msg->terminate_srcip.start);
		return;
	}
	chunk_start = start->get_address(start);
	if (msg->terminate_srcip.end)
	{
		end = host_create_from_string(msg->terminate_srcip.end, 0);
		if (!end)
		{
			DBG1(DBG_CFG, "invalid end address: %s", msg->terminate_srcip.end);
			start->destroy(start);
			return;
		}
		chunk_end = end->get_address(end);
	}

	enumerator = charon->controller->create_ike_sa_enumerator(
													charon->controller, TRUE);
	while (enumerator->enumerate(enumerator, &ike_sa))
	{
		bool match = FALSE;

		vips = ike_sa->create_virtual_ip_enumerator(ike_sa, FALSE);
		while (vips->enumerate(vips, &vip))
		{
			if (!end)
			{
				if (vip->ip_equals(vip, start))
				{
					match = TRUE;
					break;
				}
			}
			else
			{
				chunk = vip->get_address(vip);
				if (chunk.len == chunk_start.len &&
					chunk.len == chunk_end.len &&
					memcmp(chunk.ptr, chunk_start.ptr, chunk.len) >= 0 &&
					memcmp(chunk.ptr, chunk_end.ptr, chunk.len) <= 0)
				{
					match = TRUE;
					break;
				}
			}
		}
		vips->destroy(vips);

		if (match)
		{
			/* schedule delete asynchronously */
			lib->processor->queue_job(lib->processor, (job_t*)
						delete_ike_sa_job_create(ike_sa->get_id(ike_sa), TRUE));
		}
	}
	enumerator->destroy(enumerator);
	start->destroy(start);
	DESTROY_IF(end);
}

METHOD(stroke_control_t, purge_ike, void,
	private_stroke_control_t *this, stroke_msg_t *msg, FILE *out)
{
	enumerator_t *enumerator, *children;
	ike_sa_t *ike_sa;
	child_sa_t *child_sa;
	linked_list_t *list;
	uintptr_t del;
	stroke_log_info_t info;
	status_t status;

	info.out = out;
	info.level = msg->output_verbosity;

	list = linked_list_create();
	enumerator = charon->controller->create_ike_sa_enumerator(
													charon->controller, TRUE);
	while (enumerator->enumerate(enumerator, &ike_sa))
	{
		children = ike_sa->create_child_sa_enumerator(ike_sa);
		if (!children->enumerate(children, (void**)&child_sa))
		{
			list->insert_last(list,
						(void*)(uintptr_t)ike_sa->get_unique_id(ike_sa));
		}
		children->destroy(children);
	}
	enumerator->destroy(enumerator);

	enumerator = list->create_enumerator(list);
	while (enumerator->enumerate(enumerator, &del))
	{
		status = charon->controller->terminate_ike(charon->controller, del,
							(controller_cb_t)stroke_log, &info, this->timeout);
		report_terminate_status(this, status, out, del, TRUE);
	}
	enumerator->destroy(enumerator);
	list->destroy(list);
}

/**
 * Find an existing CHILD_SA/reqid
 */
static u_int32_t find_reqid(child_cfg_t *child_cfg)
{
	enumerator_t *enumerator, *children;
	child_sa_t *child_sa;
	ike_sa_t *ike_sa;
	char *name;
	u_int32_t reqid;

	reqid = charon->traps->find_reqid(charon->traps, child_cfg);
	if (reqid)
	{	/* already trapped */
		return reqid;
	}

	name = child_cfg->get_name(child_cfg);
	enumerator = charon->controller->create_ike_sa_enumerator(
													charon->controller, TRUE);
	while (enumerator->enumerate(enumerator, &ike_sa))
	{
		children = ike_sa->create_child_sa_enumerator(ike_sa);
		while (children->enumerate(children, (void**)&child_sa))
		{
			if (streq(name, child_sa->get_name(child_sa)))
			{
				reqid = child_sa->get_reqid(child_sa);
				break;
			}
		}
		children->destroy(children);
		if (reqid)
		{
			break;
		}
	}
	enumerator->destroy(enumerator);
	return reqid;
}

/**
 * call charon to install a shunt or trap
 */
static void charon_route(peer_cfg_t *peer_cfg, child_cfg_t *child_cfg,
						 char *name, FILE *out)
{
	ipsec_mode_t mode;
	u_int32_t reqid;

	mode = child_cfg->get_mode(child_cfg);
	if (mode == MODE_PASS || mode == MODE_DROP)
	{
		if (charon->shunts->install(charon->shunts, child_cfg))
		{
			fprintf(out, "'%s' shunt %N policy installed\n",
					name, ipsec_mode_names, mode);
		}
		else
		{
			fprintf(out, "'%s' shunt %N policy installation failed\n",
					name, ipsec_mode_names, mode);
		}
	}
	else
	{
		reqid = find_reqid(child_cfg);
		if (charon->traps->install(charon->traps, peer_cfg, child_cfg, reqid))
		{
			fprintf(out, "'%s' routed\n", name);
		}
		else
		{
			fprintf(out, "routing '%s' failed\n", name);
		}
	}
}

METHOD(stroke_control_t, route, void,
	private_stroke_control_t *this, stroke_msg_t *msg, FILE *out)
{
	child_cfg_t *child_cfg = NULL;
	peer_cfg_t *peer_cfg;
	enumerator_t *enumerator;
	bool empty = TRUE;

	peer_cfg = charon->backends->get_peer_cfg_by_name(charon->backends,
													  msg->route.name);
	if (peer_cfg)
	{
		child_cfg = get_child_from_peer(peer_cfg, msg->route.name);
		if (child_cfg == NULL)
		{
			enumerator = peer_cfg->create_child_cfg_enumerator(peer_cfg);
			while (enumerator->enumerate(enumerator, &child_cfg))
			{
				empty = FALSE;
				charon_route(peer_cfg, child_cfg, child_cfg->get_name(child_cfg),
							 out);
			}
			enumerator->destroy(enumerator);

			if (empty)
			{
				DBG1(DBG_CFG, "no child config named '%s'", msg->route.name);
				fprintf(out, "no child config named '%s'\n", msg->route.name);
			}
			peer_cfg->destroy(peer_cfg);
			return;
		}
	}
	else
	{
		enumerator = charon->backends->create_peer_cfg_enumerator(
							charon->backends, NULL, NULL, NULL, NULL, IKE_ANY);
		while (enumerator->enumerate(enumerator, &peer_cfg))
		{
			child_cfg = get_child_from_peer(peer_cfg, msg->route.name);
			if (child_cfg)
			{
				peer_cfg->get_ref(peer_cfg);
				break;
			}
		}
		enumerator->destroy(enumerator);

		if (child_cfg == NULL)
		{
			DBG1(DBG_CFG, "no config named '%s'", msg->route.name);
			fprintf(out, "no config named '%s'\n", msg->route.name);
			return;
		}
	}
	charon_route(peer_cfg, child_cfg, msg->route.name, out);
	peer_cfg->destroy(peer_cfg);
	child_cfg->destroy(child_cfg);
}

METHOD(stroke_control_t, unroute, void,
	private_stroke_control_t *this, stroke_msg_t *msg, FILE *out)
{
	child_sa_t *child_sa;
	enumerator_t *enumerator;
	u_int32_t id = 0;

	if (charon->shunts->uninstall(charon->shunts, msg->unroute.name))
	{
		fprintf(out, "shunt policy '%s' uninstalled\n", msg->unroute.name);
		return;
	}

	enumerator = charon->traps->create_enumerator(charon->traps);
	while (enumerator->enumerate(enumerator, NULL, &child_sa))
	{
		if (streq(msg->unroute.name, child_sa->get_name(child_sa)))
		{
			id = child_sa->get_reqid(child_sa);
			break;
		}
	}
	enumerator->destroy(enumerator);

	if (id)
	{
		charon->traps->uninstall(charon->traps, id);
		fprintf(out, "configuration '%s' unrouted\n", msg->unroute.name);
	}
	else
	{
		fprintf(out, "configuration '%s' not found\n", msg->unroute.name);
	}
}

METHOD(stroke_control_t, destroy, void,
	private_stroke_control_t *this)
{
	free(this);
}

/*
 * see header file
 */
stroke_control_t *stroke_control_create()
{
	private_stroke_control_t *this;

	INIT(this,
		.public = {
			.initiate = _initiate,
			.terminate = _terminate,
			.terminate_srcip = _terminate_srcip,
			.rekey = _rekey,
			.purge_ike = _purge_ike,
			.route = _route,
			.unroute = _unroute,
			.destroy = _destroy,
		},
		.timeout = lib->settings->get_int(lib->settings,
									"%s.plugins.stroke.timeout", 0, lib->ns),
	);

	return &this->public;
}
