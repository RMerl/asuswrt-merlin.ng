/*
 * Copyright (C) 2007 Martin Willi
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

#include "config_controller.h"
#include "../manager.h"
#include "../gateway.h"

#include <xml.h>

#include <library.h>


typedef struct private_config_controller_t private_config_controller_t;

/**
 * private data of the task manager
 */
struct private_config_controller_t {

	/**
	 * public functions
	 */
	config_controller_t public;

	/**
	 * manager instance
	 */
	manager_t *manager;
};

/**
 * read XML of a peerconfig element and fill template
 */
static void process_peerconfig(private_config_controller_t *this,
							   enumerator_t *e, fast_request_t *r)
{
	xml_t *xml;
	enumerator_t *e1, *e2, *e3;
	char *name, *value, *config = "", *child = "", *section = "";

	while (e->enumerate(e, &xml, &name, &value))
	{
		if (streq(name, "name"))
		{
			config = value;
		}
		else if (streq(name, "ikeconfig"))
		{
			e1 = xml->children(xml);
			while (e1->enumerate(e1, &xml, &name, &value))
			{
				if (streq(name, "local") || streq(name, "remote"))
				{
					if (streq(value, "0.0.0.0") || streq(value, "::"))
					{
						value = "%any";
					}
					r->setf(r, "peercfgs.%s.ikecfg.%s=%s", config, name, value);
				}
			}
			e1->destroy(e1);
		}
		else if (streq(name, "childconfiglist"))
		{
			e1 = xml->children(xml);
			while (e1->enumerate(e1, &xml, &name, &value))
			{
				if (streq(name, "childconfig"))
				{
					int num = 0;

					e2 = xml->children(xml);
					while (e2->enumerate(e2, &xml, &name, &value))
					{
						if (streq(name, "name"))
						{
							child = value;
						}
						else if (streq(name, "local") || streq(name, "remote"))
						{
							section = name;
							e3 = xml->children(xml);
							while (e3->enumerate(e3, &xml, &name, &value))
							{
								if (streq(name, "network"))
								{
									r->setf(r, "peercfgs.%s.childcfgs.%s.%s.networks.%d=%s",
											config, child, section, ++num, value);
								}
							}
							e3->destroy(e3);
						}
					}
					e2->destroy(e2);
				}
			}
			e1->destroy(e1);
		}
		else
		{
			r->setf(r, "peercfgs.%s.%s=%s", config, name, value);
		}
	}
}

static void list(private_config_controller_t *this, fast_request_t *r)
{
	gateway_t *gateway;
	xml_t *xml;
	enumerator_t *e1, *e2;
	char *name, *value;

	gateway = this->manager->select_gateway(this->manager, 0);
	e1 = gateway->query_configlist(gateway);
	if (e1 == NULL)
	{
		r->set(r, "title", "Error");
		r->set(r, "error", "querying the gateway failed");
		r->render(r, "templates/error.cs");
	}
	else
	{
		r->set(r, "title", "Configuration overview");

		while (e1->enumerate(e1, &xml, &name, &value))
		{
			if (streq(name, "peerconfig"))
			{
				e2 = xml->children(xml);
				process_peerconfig(this, e2, r);
				e2->destroy(e2);
			}
		}
		e1->destroy(e1);

		r->render(r, "templates/config/list.cs");
	}
}

METHOD(fast_controller_t, get_name, char*,
	private_config_controller_t *this)
{
	return "config";
}

METHOD(fast_controller_t, handle, void,
	private_config_controller_t *this, fast_request_t *request, char *action,
	char *p2, char *p3, char *p4, char *p5)
{
	if (!this->manager->logged_in(this->manager))
	{
		return request->redirect(request, "auth/login");
	}
	if (this->manager->select_gateway(this->manager, 0) == NULL)
	{
		return request->redirect(request, "gateway/list");
	}
	if (action)
	{
		if (streq(action, "list"))
		{
			return list(this, request);
		}
	}
	return request->redirect(request, "config/list");
}

METHOD(fast_controller_t, destroy, void,
	private_config_controller_t *this)
{
	free(this);
}

/*
 * see header file
 */
fast_controller_t *config_controller_create(fast_context_t *context,
											void *param)
{
	private_config_controller_t *this;

	INIT(this,
		.public = {
			.controller = {
				.get_name = _get_name,
				.handle = _handle,
				.destroy = _destroy,
			},
		},
		.manager = (manager_t*)context,
	);

	return &this->public.controller;
}
