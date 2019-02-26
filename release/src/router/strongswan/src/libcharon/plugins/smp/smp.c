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

#include <stdlib.h>

#include "smp.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <libxml/xmlreader.h>
#include <libxml/xmlwriter.h>

#include <library.h>
#include <daemon.h>
#include <threading/thread.h>
#include <processing/jobs/callback_job.h>


typedef struct private_smp_t private_smp_t;

/**
 * Private data of an smp_t object.
 */
struct private_smp_t {

	/**
	 * Public part of smp_t object.
	 */
	smp_t public;

	/**
	 * XML unix socket fd
	 */
	int socket;
};

ENUM(ike_sa_state_lower_names, IKE_CREATED, IKE_DELETING,
	"created",
	"connecting",
	"established",
	"rekeying",
	"deleting",
);

/**
 * write a bool into element
 */
static void write_bool(xmlTextWriterPtr writer, char *element, bool val)
{
	xmlTextWriterWriteElement(writer, element, val ? "true" : "false");
}

/**
 * write a identification_t into element
 */
static void write_id(xmlTextWriterPtr writer, char *element, identification_t *id)
{
	xmlTextWriterStartElement(writer, element);
	switch (id->get_type(id))
	{
		{
			char *type;

			while (TRUE)
			{
				case ID_ANY:
					type = "any";
					break;
				case ID_IPV4_ADDR:
					type = "ipv4";
					break;
				case ID_IPV6_ADDR:
					type = "ipv6";
					break;
				case ID_FQDN:
					type = "fqdn";
					break;
				case ID_RFC822_ADDR:
					type = "email";
					break;
				case ID_DER_ASN1_DN:
					type = "asn1dn";
					break;
				case ID_DER_ASN1_GN:
					type = "asn1gn";
					break;
			}
			xmlTextWriterWriteAttribute(writer, "type", type);
			xmlTextWriterWriteFormatString(writer, "%Y", id);
			break;
		}
		default:
			/* TODO: base64 keyid */
			xmlTextWriterWriteAttribute(writer, "type", "keyid");
			break;
	}
	xmlTextWriterEndElement(writer);
}

/**
 * write a host_t address into an element
 */
static void write_address(xmlTextWriterPtr writer, char *element, host_t *host)
{
	xmlTextWriterStartElement(writer, element);
	xmlTextWriterWriteAttribute(writer, "type",
						host->get_family(host) == AF_INET ? "ipv4" : "ipv6");
	if (host->is_anyaddr(host))
	{	/* do not use %any for XML */
		xmlTextWriterWriteFormatString(writer, "%s",
						host->get_family(host) == AF_INET ? "0.0.0.0" : "::");
	}
	else
	{
		xmlTextWriterWriteFormatString(writer, "%H", host);
	}
	xmlTextWriterEndElement(writer);
}

/**
 * write networks element
 */
static void write_networks(xmlTextWriterPtr writer, char *element,
						   linked_list_t *list)
{
	enumerator_t *enumerator;
	traffic_selector_t *ts;

	xmlTextWriterStartElement(writer, element);
	enumerator = list->create_enumerator(list);
	while (enumerator->enumerate(enumerator, (void**)&ts))
	{
		xmlTextWriterStartElement(writer, "network");
		xmlTextWriterWriteAttribute(writer, "type",
						ts->get_type(ts) == TS_IPV4_ADDR_RANGE ? "ipv4" : "ipv6");
		xmlTextWriterWriteFormatString(writer, "%R", ts);
		xmlTextWriterEndElement(writer);
	}
	enumerator->destroy(enumerator);
	xmlTextWriterEndElement(writer);
}

/**
 * write a childEnd
 */
static void write_childend(xmlTextWriterPtr writer, child_sa_t *child, bool local)
{
	linked_list_t *list;

	xmlTextWriterWriteFormatElement(writer, "spi", "%x",
									htonl(child->get_spi(child, local)));
	list = linked_list_create_from_enumerator(
									child->create_ts_enumerator(child, local));
	write_networks(writer, "networks", list);
	list->destroy(list);
}

/**
 * write a child_sa_t
 */
static void write_child(xmlTextWriterPtr writer, child_sa_t *child)
{
	child_cfg_t *config;

	config = child->get_config(child);

	xmlTextWriterStartElement(writer, "childsa");
	xmlTextWriterWriteFormatElement(writer, "reqid", "%d",
									child->get_reqid(child));
	xmlTextWriterWriteFormatElement(writer, "childconfig", "%s",
									config->get_name(config));
	xmlTextWriterStartElement(writer, "local");
	write_childend(writer, child, TRUE);
	xmlTextWriterEndElement(writer);
	xmlTextWriterStartElement(writer, "remote");
	write_childend(writer, child, FALSE);
	xmlTextWriterEndElement(writer);
	xmlTextWriterEndElement(writer);
}

/**
 * process a ikesalist query request message
 */
static void request_query_ikesa(xmlTextReaderPtr reader, xmlTextWriterPtr writer)
{
	enumerator_t *enumerator;
	ike_sa_t *ike_sa;

	/* <ikesalist> */
	xmlTextWriterStartElement(writer, "ikesalist");

	enumerator = charon->controller->create_ike_sa_enumerator(
													charon->controller, TRUE);
	while (enumerator->enumerate(enumerator, &ike_sa))
	{
		ike_sa_id_t *id;
		host_t *local, *remote;
		enumerator_t *children;
		child_sa_t *child_sa;

		id = ike_sa->get_id(ike_sa);

		xmlTextWriterStartElement(writer, "ikesa");
		xmlTextWriterWriteFormatElement(writer, "id", "%d",
							ike_sa->get_unique_id(ike_sa));
		xmlTextWriterWriteFormatElement(writer, "status", "%N",
							ike_sa_state_lower_names, ike_sa->get_state(ike_sa));
		xmlTextWriterWriteElement(writer, "role",
							id->is_initiator(id) ? "initiator" : "responder");
		xmlTextWriterWriteElement(writer, "peerconfig", ike_sa->get_name(ike_sa));

		/* <local> */
		local = ike_sa->get_my_host(ike_sa);
		xmlTextWriterStartElement(writer, "local");
		xmlTextWriterWriteFormatElement(writer, "spi", "%.16llx",
					be64toh(id->is_initiator(id) ? id->get_initiator_spi(id)
												 : id->get_responder_spi(id)));
		write_id(writer, "identification", ike_sa->get_my_id(ike_sa));
		write_address(writer, "address", local);
		xmlTextWriterWriteFormatElement(writer, "port", "%d",
							local->get_port(local));
		if (ike_sa->supports_extension(ike_sa, EXT_NATT))
		{
			write_bool(writer, "nat", ike_sa->has_condition(ike_sa, COND_NAT_HERE));
		}
		xmlTextWriterEndElement(writer);
		/* </local> */

		/* <remote> */
		remote = ike_sa->get_other_host(ike_sa);
		xmlTextWriterStartElement(writer, "remote");
		xmlTextWriterWriteFormatElement(writer, "spi", "%.16llx",
					be64toh(id->is_initiator(id) ? id->get_responder_spi(id)
												 : id->get_initiator_spi(id)));
		write_id(writer, "identification", ike_sa->get_other_id(ike_sa));
		write_address(writer, "address", remote);
		xmlTextWriterWriteFormatElement(writer, "port", "%d",
							remote->get_port(remote));
		if (ike_sa->supports_extension(ike_sa, EXT_NATT))
		{
			write_bool(writer, "nat", ike_sa->has_condition(ike_sa, COND_NAT_THERE));
		}
		xmlTextWriterEndElement(writer);
		/* </remote> */

		/* <childsalist> */
		xmlTextWriterStartElement(writer, "childsalist");
		children = ike_sa->create_child_sa_enumerator(ike_sa);
		while (children->enumerate(children, (void**)&child_sa))
		{
			write_child(writer, child_sa);
		}
		children->destroy(children);
		/* </childsalist> */
		xmlTextWriterEndElement(writer);

		/* </ikesa> */
		xmlTextWriterEndElement(writer);
	}
	enumerator->destroy(enumerator);

	/* </ikesalist> */
	xmlTextWriterEndElement(writer);
}

/**
 * process a configlist query request message
 */
static void request_query_config(xmlTextReaderPtr reader, xmlTextWriterPtr writer)
{
	enumerator_t *enumerator;
	peer_cfg_t *peer_cfg;

	/* <configlist> */
	xmlTextWriterStartElement(writer, "configlist");

	enumerator = charon->backends->create_peer_cfg_enumerator(charon->backends,
											NULL, NULL, NULL, NULL, IKE_ANY);
	while (enumerator->enumerate(enumerator, &peer_cfg))
	{
		enumerator_t *children;
		child_cfg_t *child_cfg;
		ike_cfg_t *ike_cfg;
		linked_list_t *list;

		/* <peerconfig> */
		xmlTextWriterStartElement(writer, "peerconfig");
		xmlTextWriterWriteElement(writer, "name", peer_cfg->get_name(peer_cfg));

		/* TODO: write auth_cfgs */

		/* <ikeconfig> */
		ike_cfg = peer_cfg->get_ike_cfg(peer_cfg);
		xmlTextWriterStartElement(writer, "ikeconfig");
		xmlTextWriterWriteElement(writer, "local",
								  ike_cfg->get_my_addr(ike_cfg));
		xmlTextWriterWriteElement(writer, "remote",
								  ike_cfg->get_other_addr(ike_cfg));
		xmlTextWriterEndElement(writer);
		/* </ikeconfig> */

		/* <childconfiglist> */
		xmlTextWriterStartElement(writer, "childconfiglist");
		children = peer_cfg->create_child_cfg_enumerator(peer_cfg);
		while (children->enumerate(children, &child_cfg))
		{
			/* <childconfig> */
			xmlTextWriterStartElement(writer, "childconfig");
			xmlTextWriterWriteElement(writer, "name",
									  child_cfg->get_name(child_cfg));
			list = child_cfg->get_traffic_selectors(child_cfg, TRUE, NULL,
													NULL, FALSE);
			write_networks(writer, "local", list);
			list->destroy_offset(list, offsetof(traffic_selector_t, destroy));
			list = child_cfg->get_traffic_selectors(child_cfg, FALSE, NULL,
													NULL, FALSE);
			write_networks(writer, "remote", list);
			list->destroy_offset(list, offsetof(traffic_selector_t, destroy));
			xmlTextWriterEndElement(writer);
			/* </childconfig> */
		}
		children->destroy(children);
		/* </childconfiglist> */
		xmlTextWriterEndElement(writer);
		/* </peerconfig> */
		xmlTextWriterEndElement(writer);
	}
	enumerator->destroy(enumerator);
	/* </configlist> */
	xmlTextWriterEndElement(writer);
}

/**
 * callback which logs to a XML writer
 */
static bool xml_callback(xmlTextWriterPtr writer, debug_t group, level_t level,
						 ike_sa_t* ike_sa, char* message)
{
	if (level <= 1)
	{
		/* <item> */
		xmlTextWriterStartElement(writer, "item");
		xmlTextWriterWriteFormatAttribute(writer, "level", "%d", level);
		xmlTextWriterWriteFormatAttribute(writer, "source", "%N", debug_names, group);
		xmlTextWriterWriteFormatAttribute(writer, "thread", "%u", thread_current_id());
		xmlTextWriterWriteString(writer, message);
		xmlTextWriterEndElement(writer);
		/* </item> */
	}
	return TRUE;
}

/**
 * process a *terminate control request message
 */
static void request_control_terminate(xmlTextReaderPtr reader,
									  xmlTextWriterPtr writer, bool ike)
{
	if (xmlTextReaderRead(reader) &&
		xmlTextReaderNodeType(reader) == XML_READER_TYPE_TEXT)
	{
		const char *str;
		uint32_t id;
		status_t status;

		str = xmlTextReaderConstValue(reader);
		if (str == NULL)
		{
			DBG1(DBG_CFG, "error parsing XML id string");
			return;
		}
		id = atoi(str);
		if (!id)
		{
			enumerator_t *enumerator;
			ike_sa_t *ike_sa;

			enumerator = charon->controller->create_ike_sa_enumerator(
													charon->controller, TRUE);
			while (enumerator->enumerate(enumerator, &ike_sa))
			{
				if (streq(str, ike_sa->get_name(ike_sa)))
				{
					ike = TRUE;
					id = ike_sa->get_unique_id(ike_sa);
					break;
				}
			}
			enumerator->destroy(enumerator);
		}
		if (!id)
		{
			DBG1(DBG_CFG, "error parsing XML id string");
			return;
		}

		DBG1(DBG_CFG, "terminating %s_SA %d", ike ? "IKE" : "CHILD", id);

		/* <log> */
		xmlTextWriterStartElement(writer, "log");
		if (ike)
		{
			status = charon->controller->terminate_ike(
					charon->controller, id, FALSE,
					(controller_cb_t)xml_callback, writer, 0);
		}
		else
		{
			status = charon->controller->terminate_child(
					charon->controller, id,
					(controller_cb_t)xml_callback, writer, 0);
		}
		/* </log> */
		xmlTextWriterEndElement(writer);
		xmlTextWriterWriteFormatElement(writer, "status", "%d", status);
	}
}

/**
 * process a *initiate control request message
 */
static void request_control_initiate(xmlTextReaderPtr reader,
									  xmlTextWriterPtr writer, bool ike)
{
	if (xmlTextReaderRead(reader) &&
		xmlTextReaderNodeType(reader) == XML_READER_TYPE_TEXT)
	{
		const char *str;
		status_t status = FAILED;
		peer_cfg_t *peer;
		child_cfg_t *child = NULL;
		enumerator_t *enumerator;

		str = xmlTextReaderConstValue(reader);
		if (str == NULL)
		{
			DBG1(DBG_CFG, "error parsing XML config name string");
			return;
		}
		DBG1(DBG_CFG, "initiating %s_SA %s", ike ? "IKE" : "CHILD", str);

		/* <log> */
		xmlTextWriterStartElement(writer, "log");
		peer = charon->backends->get_peer_cfg_by_name(charon->backends,
													  (char*)str);
		if (peer)
		{
			enumerator = peer->create_child_cfg_enumerator(peer);
			if (ike)
			{
				if (enumerator->enumerate(enumerator, &child))
				{
					child->get_ref(child);
				}
				else
				{
					child = NULL;
				}
			}
			else
			{
				while (enumerator->enumerate(enumerator, &child))
				{
					if (streq(child->get_name(child), str))
					{
						child->get_ref(child);
						break;
					}
					child = NULL;
				}
			}
			enumerator->destroy(enumerator);
			if (child)
			{
				status = charon->controller->initiate(charon->controller,
							peer, child, (controller_cb_t)xml_callback,
							writer, 0, FALSE);
			}
			else
			{
				peer->destroy(peer);
			}
		}
		/* </log> */
		xmlTextWriterEndElement(writer);
		xmlTextWriterWriteFormatElement(writer, "status", "%d", status);
	}
}

/**
 * process a query request
 */
static void request_query(xmlTextReaderPtr reader, xmlTextWriterPtr writer)
{
	/* <query> */
	xmlTextWriterStartElement(writer, "query");
	while (xmlTextReaderRead(reader))
	{
		if (xmlTextReaderNodeType(reader) == XML_READER_TYPE_ELEMENT)
		{
			if (streq(xmlTextReaderConstName(reader), "ikesalist"))
			{
				request_query_ikesa(reader, writer);
				break;
			}
			if (streq(xmlTextReaderConstName(reader), "configlist"))
			{
				request_query_config(reader, writer);
				break;
			}
		}
	}
	/* </query> */
	xmlTextWriterEndElement(writer);
}

/**
 * process a control request
 */
static void request_control(xmlTextReaderPtr reader, xmlTextWriterPtr writer)
{
	/* <control> */
	xmlTextWriterStartElement(writer, "control");
	while (xmlTextReaderRead(reader))
	{
		if (xmlTextReaderNodeType(reader) == XML_READER_TYPE_ELEMENT)
		{
			if (streq(xmlTextReaderConstName(reader), "ikesaterminate"))
			{
				request_control_terminate(reader, writer, TRUE);
				break;
			}
			if (streq(xmlTextReaderConstName(reader), "childsaterminate"))
			{
				request_control_terminate(reader, writer, FALSE);
				break;
			}
			if (streq(xmlTextReaderConstName(reader), "ikesainitiate"))
			{
				request_control_initiate(reader, writer, TRUE);
				break;
			}
			if (streq(xmlTextReaderConstName(reader), "childsainitiate"))
			{
				request_control_initiate(reader, writer, FALSE);
				break;
			}
		}
	}
	/* </control> */
	xmlTextWriterEndElement(writer);
}

/**
 * process a request message
 */
static void request(xmlTextReaderPtr reader, char *id, int fd)
{
	xmlTextWriterPtr writer;

	writer = xmlNewTextWriter(xmlOutputBufferCreateFd(fd, NULL));
	if (writer == NULL)
	{
		DBG1(DBG_CFG, "opening SMP XML writer failed");
		return;
	}

	xmlTextWriterStartDocument(writer, NULL, NULL, NULL);
	/* <message xmlns="http://www.strongswan.org/smp/1.0"
		id="id" type="response"> */
	xmlTextWriterStartElement(writer, "message");
	xmlTextWriterWriteAttribute(writer, "xmlns",
								"http://www.strongswan.org/smp/1.0");
	xmlTextWriterWriteAttribute(writer, "id", id);
	xmlTextWriterWriteAttribute(writer, "type", "response");

	while (xmlTextReaderRead(reader))
	{
		if (xmlTextReaderNodeType(reader) == XML_READER_TYPE_ELEMENT)
		{
			if (streq(xmlTextReaderConstName(reader), "query"))
			{
				request_query(reader, writer);
				break;
			}
			if (streq(xmlTextReaderConstName(reader), "control"))
			{
				request_control(reader, writer);
				break;
			}
		}
	}
	/*   </message> and close document */
	xmlTextWriterEndDocument(writer);
	xmlFreeTextWriter(writer);
}

/**
 * cleanup helper function for open file descriptors
 */
static void closefdp(int *fd)
{
	close(*fd);
}

/**
 * read from a opened connection and process it
 */
static job_requeue_t process(int *fdp)
{
	int fd = *fdp;
	bool oldstate;
	char buffer[4096];
	ssize_t len;
	xmlTextReaderPtr reader;
	char *id = NULL, *type = NULL;

	thread_cleanup_push((thread_cleanup_t)closefdp, (void*)&fd);
	oldstate = thread_cancelability(TRUE);
	len = read(fd, buffer, sizeof(buffer));
	thread_cancelability(oldstate);
	thread_cleanup_pop(FALSE);
	if (len <= 0)
	{
		close(fd);
		DBG2(DBG_CFG, "SMP XML connection closed");
		return JOB_REQUEUE_NONE;
	}
	DBG3(DBG_CFG, "got XML request: %b", buffer, (u_int)len);

	reader = xmlReaderForMemory(buffer, len, NULL, NULL, 0);
	if (reader == NULL)
	{
		DBG1(DBG_CFG, "opening SMP XML reader failed");
		return JOB_REQUEUE_FAIR;;
	}

	/* read message type and id */
	while (xmlTextReaderRead(reader))
	{
		if (xmlTextReaderNodeType(reader) == XML_READER_TYPE_ELEMENT &&
			streq(xmlTextReaderConstName(reader), "message"))
		{
			id = xmlTextReaderGetAttribute(reader, "id");
			type = xmlTextReaderGetAttribute(reader, "type");
			break;
		}
	}

	/* process message */
	if (id && type)
	{
		if (streq(type, "request"))
		{
			request(reader, id, fd);
		}
		else
		{
			/* response(reader, id) */
		}
	}
	xmlFreeTextReader(reader);
	return JOB_REQUEUE_FAIR;;
}

/**
 * accept from XML socket and create jobs to process connections
 */
static job_requeue_t dispatch(private_smp_t *this)
{
	struct sockaddr_un strokeaddr;
	int fd, *fdp, strokeaddrlen = sizeof(strokeaddr);
	callback_job_t *job;
	bool oldstate;

	/* wait for connections, but allow thread to terminate */
	oldstate = thread_cancelability(TRUE);
	fd = accept(this->socket, (struct sockaddr *)&strokeaddr, &strokeaddrlen);
	thread_cancelability(oldstate);

	if (fd < 0)
	{
		DBG1(DBG_CFG, "accepting SMP XML socket failed: %s", strerror(errno));
		sleep(1);
		return JOB_REQUEUE_FAIR;;
	}

	fdp = malloc_thing(int);
	*fdp = fd;
	job = callback_job_create((callback_job_cb_t)process, fdp, free,
							  (callback_job_cancel_t)return_false);
	lib->processor->queue_job(lib->processor, (job_t*)job);

	return JOB_REQUEUE_DIRECT;
}

METHOD(plugin_t, get_name, char*,
	private_smp_t *this)
{
	return "smp";
}

METHOD(plugin_t, get_features, int,
	private_smp_t *this, plugin_feature_t *features[])
{
	static plugin_feature_t f[] = {
		PLUGIN_NOOP,
			PLUGIN_PROVIDE(CUSTOM, "smp"),
	};
	*features = f;
	return countof(f);
}

METHOD(plugin_t, destroy, void,
	private_smp_t *this)
{
	close(this->socket);
	free(this);
}

/*
 * Described in header file
 */
plugin_t *smp_plugin_create()
{
	struct sockaddr_un unix_addr = { AF_UNIX, IPSEC_PIDDIR "/charon.xml"};
	private_smp_t *this;
	mode_t old;

	if (!lib->caps->check(lib->caps, CAP_CHOWN))
	{	/* required to chown(2) control socket */
		DBG1(DBG_CFG, "smp plugin requires CAP_CHOWN capability");
		return NULL;
	}

	INIT(this,
		.public = {
			.plugin = {
				.get_name = _get_name,
				.get_features = _get_features,
				.destroy = _destroy,
			},
		},
	);

	/* set up unix socket */
	this->socket = socket(AF_UNIX, SOCK_STREAM, 0);
	if (this->socket == -1)
	{
		DBG1(DBG_CFG, "could not create XML socket");
		free(this);
		return NULL;
	}

	unlink(unix_addr.sun_path);
	old = umask(S_IRWXO);
	if (bind(this->socket, (struct sockaddr *)&unix_addr, sizeof(unix_addr)) < 0)
	{
		DBG1(DBG_CFG, "could not bind XML socket: %s", strerror(errno));
		close(this->socket);
		free(this);
		return NULL;
	}
	umask(old);
	if (chown(unix_addr.sun_path, lib->caps->get_uid(lib->caps),
			  lib->caps->get_gid(lib->caps)) != 0)
	{
		DBG1(DBG_CFG, "changing XML socket permissions failed: %s", strerror(errno));
	}

	if (listen(this->socket, 5) < 0)
	{
		DBG1(DBG_CFG, "could not listen on XML socket: %s", strerror(errno));
		close(this->socket);
		free(this);
		return NULL;
	}

	lib->processor->queue_job(lib->processor,
		(job_t*)callback_job_create_with_prio((callback_job_cb_t)dispatch, this,
				NULL, (callback_job_cancel_t)return_false, JOB_PRIO_CRITICAL));

	return &this->public.plugin;
}
