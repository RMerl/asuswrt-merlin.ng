/*
 * Copyright (C) 2007 Martin Willi
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

#include "gateway.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <xml.h>

typedef struct private_gateway_t private_gateway_t;

/**
 * private data of gateway
 */
struct private_gateway_t {

	/**
	 * public functions
	 */
	gateway_t public;

	/**
	 * name of the gateway
	 */
	char *name;

	/**
	 * host to connect using tcp
	 */
	host_t *host;

	/**
	 * socket file descriptor, > 0 if connected
	 */
	int fd;

	/**
	 * unique id assigned to each xml message
	 */
	int xmlid;
};

struct sockaddr_un unix_addr = { AF_UNIX, IPSEC_PIDDIR "/charon.xml"};

/**
 * establish connection to gateway
 */
static bool connect_(private_gateway_t *this)
{
	int family, len;
	struct sockaddr *addr;

	if (this->fd >= 0)
	{
		close(this->fd);
	}
	if (this->host)
	{
		family = AF_INET;
		addr = this->host->get_sockaddr(this->host);
		len = *this->host->get_sockaddr_len(this->host);
	}
	else
	{
		family = AF_UNIX;
		addr = (struct sockaddr*)&unix_addr;
		len = sizeof(unix_addr);
	}

	this->fd = socket(family, SOCK_STREAM, 0);
	if (this->fd < 0)
	{
		return FALSE;
	}
	if (connect(this->fd, addr, len) != 0)
	{
		close(this->fd);
		this->fd = -1;
		return FALSE;
	}
	return TRUE;
}

METHOD(gateway_t, request, char*,
	private_gateway_t *this, char *xml, ...)
{
	if (this->fd < 0)
	{
		if (!connect_(this))
		{
			return NULL;
		}
	}
	while (TRUE)
	{
		char buf[8096];
		ssize_t len;
		va_list args;

		va_start(args, xml);
		len = vsnprintf(buf, sizeof(buf), xml, args);
		va_end(args);
		if (len < 0 || len >= sizeof(buf))
		{
			return NULL;
		}
		if (send(this->fd, buf, len, 0) != len)
		{
			if (!connect_(this))
			{
				return NULL;
			}
			continue;
		}
		len = recv(this->fd, buf, sizeof(buf) - 1, 0);
		if (len <= 0)
		{
			if (!connect_(this))
			{
				return NULL;
			}
			continue;
		}
		buf[len] = 0;
		return strdup(buf);
	}
}

METHOD(gateway_t, query_ikesalist, enumerator_t*,
	private_gateway_t *this)
{
	char *str, *name, *value;
	xml_t *xml;
	enumerator_t *e1, *e2, *e3, *e4 = NULL;

	str = request(this,	"<message type=\"request\" id=\"%d\">"
							"<query>"
								"<ikesalist/>"
							"</query>"
						"</message>", this->xmlid++);
	if (str == NULL)
	{
		return NULL;
	}
	xml = xml_create(str);
	if (xml == NULL)
	{
		return NULL;
	}

	e1 = xml->children(xml);
	free(str);
	while (e1->enumerate(e1, &xml, &name, &value))
	{
		if (streq(name, "message"))
		{
			e2 = xml->children(xml);
			while (e2->enumerate(e2, &xml, &name, &value))
			{
				if (streq(name, "query"))
				{
					e3 = xml->children(xml);
					while (e3->enumerate(e3, &xml, &name, &value))
					{
						if (streq(name, "ikesalist"))
						{
							e4 = xml->children(xml);
							e1->destroy(e1);
							e2->destroy(e2);
							e3->destroy(e3);
							return e4;
						}
					}
					e3->destroy(e3);
				}
			}
			e2->destroy(e2);
		}
	}
	e1->destroy(e1);
	return NULL;
}

METHOD(gateway_t, query_configlist, enumerator_t*,
	private_gateway_t *this)
{
	char *str, *name, *value;
	xml_t *xml;
	enumerator_t *e1, *e2, *e3, *e4 = NULL;

	str = request(this,	"<message type=\"request\" id=\"%d\">"
							"<query>"
								"<configlist/>"
							"</query>"
						"</message>", this->xmlid++);
	if (str == NULL)
	{
		return NULL;
	}
	xml = xml_create(str);
	if (xml == NULL)
	{
		return NULL;
	}

	e1 = xml->children(xml);
	free(str);
	while (e1->enumerate(e1, &xml, &name, &value))
	{
		if (streq(name, "message"))
		{
			e2 = xml->children(xml);
			while (e2->enumerate(e2, &xml, &name, &value))
			{
				if (streq(name, "query"))
				{
					e3 = xml->children(xml);
					while (e3->enumerate(e3, &xml, &name, &value))
					{
						if (streq(name, "configlist"))
						{
							e4 = xml->children(xml);
							e1->destroy(e1);
							e2->destroy(e2);
							e3->destroy(e3);
							return e4;
						}
					}
					e3->destroy(e3);
				}
			}
			e2->destroy(e2);
		}
	}
	e1->destroy(e1);
	return NULL;
}

/**
 * create enumerator over control elements children of a control response
 */
static enumerator_t* read_result(private_gateway_t *this, char *res)
{
	char *name, *value;
	xml_t *xml;
	enumerator_t *e1, *e2, *e3;

	if (res == NULL)
	{
		return NULL;
	}
	xml = xml_create(res);
	if (xml == NULL)
	{
		return NULL;
	}
	e1 = xml->children(xml);
	free(res);
	while (e1->enumerate(e1, &xml, &name, &value))
	{
		if (streq(name, "message"))
		{
			e2 = xml->children(xml);
			while (e2->enumerate(e2, &xml, &name, &value))
			{
				if (streq(name, "control"))
				{
					e3 = xml->children(xml);
					e1->destroy(e1);
					e2->destroy(e2);
					return e3;
				}
			}
			e2->destroy(e2);
		}
	}
	e1->destroy(e1);
	return NULL;
}

METHOD(gateway_t, initiate, enumerator_t*,
	private_gateway_t *this, bool ike, char *name)
{
	char *str, *kind;

	if (ike)
	{
		kind = "ike";
	}
	else
	{
		kind = "child";
	}
	str = request(this,	"<message type=\"request\" id=\"%d\">"
							"<control>"
								"<%ssainitiate>%s</%ssainitiate>"
							"</control>"
						"</message>", this->xmlid++, kind, name, kind);
	return read_result(this, str);
}

METHOD(gateway_t, terminate, enumerator_t*,
	private_gateway_t *this, bool ike, u_int32_t id)
{
	char *str, *kind;

	if (ike)
	{
		kind = "ike";
	}
	else
	{
		kind = "child";
	}
	str = request(this,	"<message type=\"request\" id=\"%d\">"
							"<control>"
								"<%ssaterminate>%d</%ssaterminate>"
							"</control>"
						"</message>", this->xmlid++, kind, id, kind);
	return read_result(this, str);
}

METHOD(gateway_t, destroy, void,
	private_gateway_t *this)
{
	if (this->fd >= 0)
	{
		close(this->fd);
	}
	if (this->host) this->host->destroy(this->host);
	free(this->name);
	free(this);
}

/**
 * generic constructor
 */
static private_gateway_t *gateway_create(char *name)
{
	private_gateway_t *this;

	INIT(this,
		.public = {
			.request = _request,
			.query_ikesalist = _query_ikesalist,
			.query_configlist = _query_configlist,
			.initiate = _initiate,
			.terminate = _terminate,
			.destroy = _destroy,
		},
		.name = strdup(name),
		.fd = -1,
		.xmlid = 1,
	);

	return this;
}

/**
 * see header
 */
gateway_t *gateway_create_tcp(char *name, host_t *host)
{
	private_gateway_t *this = gateway_create(name);

	this->host = host;

	return &this->public;
}

/**
 * see header
 */
gateway_t *gateway_create_unix(char *name)
{
	private_gateway_t *this = gateway_create(name);

	return &this->public;
}

