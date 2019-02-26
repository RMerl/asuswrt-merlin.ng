/*
 * Copyright (C) 2008 Thomas Kallenberg
 * Copyright (C) 2008 Martin Willi
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

#define _GNU_SOURCE
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#include "uci_control.h"

#include <daemon.h>
#include <threading/thread.h>
#include <processing/jobs/callback_job.h>

#define FIFO_FILE "/var/run/charon.fifo"


typedef struct private_uci_control_t private_uci_control_t;

/**
 * private data of uci_control_t
 */
struct private_uci_control_t {

	/**
	 * Public part
	 */
	uci_control_t public;
};

/**
 * write answer to fifo
 */
static void write_fifo(private_uci_control_t *this, char *format, ...)
{
	va_list args;
	FILE *out;

	out = fopen(FIFO_FILE, "w");
	if (out)
	{
		va_start(args, format);
		vfprintf(out, format, args);
		va_end(args);
		fclose(out);
	}
	else
	{
		DBG1(DBG_CFG, "writing to UCI fifo failed: %s", strerror(errno));
	}
}

/**
 * print IKE_SA status information
 */
static void status(private_uci_control_t *this, char *name)
{
	enumerator_t *configs, *sas, *children;
	linked_list_t *list;
	ike_sa_t *ike_sa;
	child_sa_t *child_sa;
	peer_cfg_t *peer_cfg;
	char buf[2048];
	FILE *out = NULL;

	configs = charon->backends->create_peer_cfg_enumerator(charon->backends,
											NULL, NULL, NULL, NULL, IKE_ANY);
	while (configs->enumerate(configs, &peer_cfg))
	{
		if (name && !streq(name, peer_cfg->get_name(peer_cfg)))
		{
			continue;
		}
		sas = charon->controller->create_ike_sa_enumerator(
													charon->controller, TRUE);
		while (sas->enumerate(sas, &ike_sa))
		{
			if (!streq(ike_sa->get_name(ike_sa), peer_cfg->get_name(peer_cfg)))
			{
				continue;
			}
			if (!out)
			{
				out = fmemopen(buf, sizeof(buf), "w");
				if (!out)
				{
					continue;
				}
			}
			fprintf(out, "%-8s %-20D %-16H ", ike_sa->get_name(ike_sa),
				ike_sa->get_other_id(ike_sa), ike_sa->get_other_host(ike_sa));

			children = ike_sa->create_child_sa_enumerator(ike_sa);
			while (children->enumerate(children, (void**)&child_sa))
			{
				list = linked_list_create_from_enumerator(
							child_sa->create_ts_enumerator(child_sa, FALSE));
				fprintf(out, "%#R", list);
				list->destroy(list);
			}
			children->destroy(children);
			fprintf(out, "\n");
		}
		sas->destroy(sas);
	}
	configs->destroy(configs);
	if (out)
	{
		fclose(out);
		write_fifo(this, "%s", buf);
	}
	else
	{
		write_fifo(this, "");
	}
}

/**
 * Initiate an IKE_SA
 */
static void initiate(private_uci_control_t *this, char *name)
{
	peer_cfg_t *peer_cfg;
	child_cfg_t *child_cfg;
	enumerator_t *enumerator;

	peer_cfg = charon->backends->get_peer_cfg_by_name(charon->backends, name);
	if (peer_cfg)
	{
		enumerator = peer_cfg->create_child_cfg_enumerator(peer_cfg);
		if (enumerator->enumerate(enumerator, &child_cfg) &&
			charon->controller->initiate(charon->controller, peer_cfg,
								child_cfg->get_ref(child_cfg),
								controller_cb_empty, NULL, 0, FALSE) == SUCCESS)
		{
			write_fifo(this, "connection '%s' established\n", name);
		}
		else
		{
			write_fifo(this, "establishing connection '%s' failed\n", name);
		}
		enumerator->destroy(enumerator);
	}
	else
	{
		write_fifo(this, "no connection named '%s' found\n", name);
	}
}

/**
 * terminate an IKE_SA
 */
static void terminate(private_uci_control_t *this, char *name)
{
	enumerator_t *enumerator;
	ike_sa_t *ike_sa;
	u_int id;

	enumerator = charon->controller->create_ike_sa_enumerator(
													charon->controller, TRUE);
	while (enumerator->enumerate(enumerator, &ike_sa))
	{
		if (streq(name, ike_sa->get_name(ike_sa)))
		{
			id = ike_sa->get_unique_id(ike_sa);
			enumerator->destroy(enumerator);
			charon->controller->terminate_ike(charon->controller, id, FALSE,
											  controller_cb_empty, NULL, 0);
			write_fifo(this, "connection '%s' terminated\n", name);
			return;
		}
	}
	enumerator->destroy(enumerator);
	write_fifo(this, "no active connection named '%s'\n", name);
}

/**
 * dispatch control request
 */
static void process(private_uci_control_t *this, char *message)
{
	enumerator_t* enumerator;

	enumerator = enumerator_create_token(message, " \n", "");
	if (enumerator->enumerate(enumerator, &message))
	{
		if (streq(message, "status"))
		{
			if (enumerator->enumerate(enumerator, &message))
			{
				status(this, message);
			}
			else
			{
				status(this, NULL);
			}
		}
		else if (streq(message, "up") &&
				 enumerator->enumerate(enumerator, &message))
		{
			initiate(this, message);
		}
		else if (streq(message, "down") &&
				 enumerator->enumerate(enumerator, &message))
		{
			terminate(this, message);
		}
		else
		{
			write_fifo(this, "usage: status [<name>] | up <name> | down <name>\n"
					   "  status format: name peer-id peer-addr tunnel(s)\n");
		}
	}
	enumerator->destroy(enumerator);
}

/**
 * read from fifo
 */
static job_requeue_t receive(private_uci_control_t *this)
{
	char message[128];
	int len;
	bool oldstate;
	FILE *in;

	memset(message, 0, sizeof(message));
	oldstate = thread_cancelability(TRUE);
	in = fopen(FIFO_FILE, "r");
	thread_cancelability(oldstate);
	if (in)
	{
		len = fread(message, 1, sizeof(message) - 1, in);
		fclose(in);
		if (len > 0)
		{
			process(this, message);
		}
		else
		{
			DBG1(DBG_DMN, "reading from UCI fifo failed: %s", strerror(errno));
		}
	}
	else
	{
		DBG1(DBG_DMN, "opening UCI fifo failed: %s", strerror(errno));
	}
	return JOB_REQUEUE_FAIR;
}

METHOD(uci_control_t, destroy, void,
	private_uci_control_t *this)
{
	unlink(FIFO_FILE);
	free(this);
}

/**
 * Described in header.
 */
uci_control_t *uci_control_create()
{
	private_uci_control_t *this;

	INIT(this,
		.public = {
			.destroy = _destroy,
		},
	);

	unlink(FIFO_FILE);
	if (mkfifo(FIFO_FILE, S_IRUSR|S_IWUSR) != 0)
	{
		DBG1(DBG_CFG, "creating UCI control fifo '%s' failed: %s",
			 FIFO_FILE, strerror(errno));
	}
	else
	{
		lib->processor->queue_job(lib->processor,
			(job_t*)callback_job_create_with_prio((callback_job_cb_t)receive,
							this, NULL, (callback_job_cancel_t)return_false,
							JOB_PRIO_CRITICAL));
	}
	return &this->public;
}
