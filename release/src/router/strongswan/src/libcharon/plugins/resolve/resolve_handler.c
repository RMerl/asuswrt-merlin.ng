/*
 * Copyright (C) 2012-2016 Tobias Brunner
 * Copyright (C) 2009 Martin Willi
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

#include "resolve_handler.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <utils/debug.h>
#include <utils/process.h>
#include <collections/array.h>
#include <threading/mutex.h>

/* path to resolvconf executable */
#define RESOLVCONF_EXEC "/sbin/resolvconf"

/* default prefix used for resolvconf interfaces (should have high prio) */
#define RESOLVCONF_PREFIX "lo.inet.ipsec."

typedef struct private_resolve_handler_t private_resolve_handler_t;

/**
 * Private data of an resolve_handler_t object.
 */
struct private_resolve_handler_t {

	/**
	 * Public resolve_handler_t interface.
	 */
	resolve_handler_t public;

	/**
	 * resolv.conf file to use
	 */
	char *file;

	/**
	 * Use resolvconf instead of writing directly to resolv.conf
	 */
	bool use_resolvconf;

	/**
	 * Prefix to be used for interface names sent to resolvconf
	 */
	char *iface_prefix;

	/**
	 * Mutex to access file exclusively
	 */
	mutex_t *mutex;

	/**
	 * Reference counting for DNS servers dns_server_t
	 */
	array_t *servers;
};

/**
 * Reference counting for DNS servers
 */
typedef struct {

	/**
	 * DNS server address
	 */
	host_t *server;

	/**
	 * Reference count
	 */
	u_int refcount;

} dns_server_t;

/**
 * Compare a server and a stored reference
 */
static int dns_server_find(const void *a, const void *b)
{
	host_t *server = (host_t*)a;
	dns_server_t *item = (dns_server_t*)b;
	return chunk_compare(server->get_address(server),
						 item->server->get_address(item->server));
}

/**
 * Sort references by DNS server
 */
static int dns_server_sort(const void *a, const void *b, void *user)
{
	const dns_server_t *da = a, *db = b;
	return chunk_compare(da->server->get_address(da->server),
						 db->server->get_address(db->server));
}

/**
 * Writes the given nameserver to resolv.conf
 */
static bool write_nameserver(private_resolve_handler_t *this, host_t *addr)
{
	FILE *in, *out;
	char buf[1024];
	size_t len;
	bool handled = FALSE;

	in = fopen(this->file, "r");
	/* allows us to stream from in to out */
	unlink(this->file);
	out = fopen(this->file, "w");
	if (out)
	{
		fprintf(out, "nameserver %H   # by strongSwan\n", addr);
		DBG1(DBG_IKE, "installing DNS server %H to %s", addr, this->file);
		handled = TRUE;

		/* copy rest of the file */
		if (in)
		{
			while ((len = fread(buf, 1, sizeof(buf), in)))
			{
				ignore_result(fwrite(buf, 1, len, out));
			}
		}
		fclose(out);
	}
	if (in)
	{
		fclose(in);
	}
	return handled;
}

/**
 * Removes the given nameserver from resolv.conf
 */
static void remove_nameserver(private_resolve_handler_t *this, host_t *addr)
{
	FILE *in, *out;
	char line[1024], matcher[512];

	in = fopen(this->file, "r");
	if (in)
	{
		/* allows us to stream from in to out */
		unlink(this->file);
		out = fopen(this->file, "w");
		if (out)
		{
			snprintf(matcher, sizeof(matcher),
					 "nameserver %H   # by strongSwan\n", addr);

			/* copy all, but matching line */
			while (fgets(line, sizeof(line), in))
			{
				if (strpfx(line, matcher))
				{
					DBG1(DBG_IKE, "removing DNS server %H from %s",
						 addr, this->file);
				}
				else
				{
					fputs(line, out);
				}
			}
			fclose(out);
		}
		fclose(in);
	}
}

/**
 * Add or remove the given nameserver by invoking resolvconf.
 */
static bool invoke_resolvconf(private_resolve_handler_t *this, host_t *addr,
							  bool install)
{
	process_t *process;
	FILE *shell;
	int in, out, retval;

	/* we use the nameserver's IP address as part of the interface name to
	 * make them unique */
	process = process_start_shell(NULL, install ? &in : NULL, &out, NULL,
							"2>&1 %s %s %s%H", RESOLVCONF_EXEC,
							install ? "-a" : "-d", this->iface_prefix, addr);

	if (!process)
	{
		return FALSE;
	}
	if (install)
	{
		shell = fdopen(in, "w");
		if (shell)
		{
			DBG1(DBG_IKE, "installing DNS server %H via resolvconf", addr);
			fprintf(shell, "nameserver %H\n", addr);
			fclose(shell);
		}
		else
		{
			close(in);
			close(out);
			process->wait(process, NULL);
			return FALSE;
		}
	}
	else
	{
		DBG1(DBG_IKE, "removing DNS server %H via resolvconf", addr);
	}
	shell = fdopen(out, "r");
	if (shell)
	{
		while (TRUE)
		{
			char resp[128], *e;

			if (fgets(resp, sizeof(resp), shell) == NULL)
			{
				if (ferror(shell))
				{
					DBG1(DBG_IKE, "error reading from resolvconf");
				}
				break;
			}
			else
			{
				e = resp + strlen(resp);
				if (e > resp && e[-1] == '\n')
				{
					e[-1] = '\0';
				}
				DBG1(DBG_IKE, "resolvconf: %s", resp);
			}
		}
		fclose(shell);
	}
	else
	{
		close(out);
	}
	if (!process->wait(process, &retval) || retval != EXIT_SUCCESS)
	{
		if (install)
		{	/* revert changes when installing fails */
			invoke_resolvconf(this, addr, FALSE);
			return FALSE;
		}
	}
	return TRUE;
}

METHOD(attribute_handler_t, handle, bool,
	private_resolve_handler_t *this, ike_sa_t *ike_sa,
	configuration_attribute_type_t type, chunk_t data)
{
	dns_server_t *found = NULL;
	host_t *addr;
	bool handled;

	switch (type)
	{
		case INTERNAL_IP4_DNS:
			addr = host_create_from_chunk(AF_INET, data, 0);
			break;
		case INTERNAL_IP6_DNS:
			addr = host_create_from_chunk(AF_INET6, data, 0);
			break;
		default:
			return FALSE;
	}

	if (!addr || addr->is_anyaddr(addr))
	{
		DESTROY_IF(addr);
		return FALSE;
	}

	this->mutex->lock(this->mutex);
	if (array_bsearch(this->servers, addr, dns_server_find, &found) == -1)
	{
		if (this->use_resolvconf)
		{
			handled = invoke_resolvconf(this, addr, TRUE);
		}
		else
		{
			handled = write_nameserver(this, addr);
		}
		if (handled)
		{
			INIT(found,
				.server = addr->clone(addr),
				.refcount = 1,
			);
			array_insert_create(&this->servers, ARRAY_TAIL, found);
			array_sort(this->servers, dns_server_sort, NULL);
		}
	}
	else
	{
		DBG1(DBG_IKE, "DNS server %H already installed, increasing refcount",
			 addr);
		found->refcount++;
		handled = TRUE;
	}
	this->mutex->unlock(this->mutex);
	addr->destroy(addr);

	if (!handled)
	{
		DBG1(DBG_IKE, "adding DNS server failed");
	}
	return handled;
}

METHOD(attribute_handler_t, release, void,
	private_resolve_handler_t *this, ike_sa_t *ike_sa,
	configuration_attribute_type_t type, chunk_t data)
{
	dns_server_t *found = NULL;
	host_t *addr;
	int family, idx;

	switch (type)
	{
		case INTERNAL_IP4_DNS:
			family = AF_INET;
			break;
		case INTERNAL_IP6_DNS:
			family = AF_INET6;
			break;
		default:
			return;
	}
	addr = host_create_from_chunk(family, data, 0);

	this->mutex->lock(this->mutex);
	idx = array_bsearch(this->servers, addr, dns_server_find, &found);
	if (idx != -1)
	{
		if (--found->refcount > 0)
		{
			DBG1(DBG_IKE, "DNS server %H still used, decreasing refcount",
				 addr);
		}
		else
		{
			if (this->use_resolvconf)
			{
				invoke_resolvconf(this, addr, FALSE);
			}
			else
			{
				remove_nameserver(this, addr);
			}
			array_remove(this->servers, idx, NULL);
			found->server->destroy(found->server);
			free(found);
		}
	}
	this->mutex->unlock(this->mutex);

	addr->destroy(addr);
}

/**
 * Attribute enumerator implementation
 */
typedef struct {
	/** implements enumerator_t interface */
	enumerator_t public;
	/** request IPv4 DNS? */
	bool v4;
	/** request IPv6 DNS? */
	bool v6;
} attribute_enumerator_t;

METHOD(enumerator_t, attribute_enumerate, bool,
	attribute_enumerator_t *this, va_list args)
{
	configuration_attribute_type_t *type;
	chunk_t *data;

	VA_ARGS_VGET(args, type, data);
	if (this->v4)
	{
		*type = INTERNAL_IP4_DNS;
		*data = chunk_empty;
		this->v4 = FALSE;
		return TRUE;
	}
	if (this->v6)
	{
		*type = INTERNAL_IP6_DNS;
		*data = chunk_empty;
		this->v6 = FALSE;
		return TRUE;
	}
	return FALSE;
}

/**
 * Check if a list has a host of given family
 */
static bool has_host_family(linked_list_t *list, int family)
{
	enumerator_t *enumerator;
	host_t *host;
	bool found = FALSE;

	enumerator = list->create_enumerator(list);
	while (enumerator->enumerate(enumerator, &host))
	{
		if (host->get_family(host) == family)
		{
			found = TRUE;
			break;
		}
	}
	enumerator->destroy(enumerator);

	return found;
}

METHOD(attribute_handler_t, create_attribute_enumerator, enumerator_t*,
	private_resolve_handler_t *this, ike_sa_t *ike_sa,
	linked_list_t *vips)
{
	attribute_enumerator_t *enumerator;

	INIT(enumerator,
		.public = {
			.enumerate = enumerator_enumerate_default,
			.venumerate = _attribute_enumerate,
			.destroy = (void*)free,
		},
		.v4 = has_host_family(vips, AF_INET),
		.v6 = has_host_family(vips, AF_INET6),
	);
	return &enumerator->public;
}

METHOD(resolve_handler_t, destroy, void,
	private_resolve_handler_t *this)
{
	array_destroy(this->servers);
	this->mutex->destroy(this->mutex);
	free(this);
}

/**
 * See header
 */
resolve_handler_t *resolve_handler_create()
{
	private_resolve_handler_t *this;
	struct stat st;

	INIT(this,
		.public = {
			.handler = {
				.handle = _handle,
				.release = _release,
				.create_attribute_enumerator = _create_attribute_enumerator,
			},
			.destroy = _destroy,
		},
		.mutex = mutex_create(MUTEX_TYPE_DEFAULT),
		.file = lib->settings->get_str(lib->settings, "%s.plugins.resolve.file",
									   RESOLV_CONF, lib->ns),
	);

	if (stat(RESOLVCONF_EXEC, &st) == 0)
	{
		this->use_resolvconf = TRUE;
		this->iface_prefix = lib->settings->get_str(lib->settings,
								"%s.plugins.resolve.resolvconf.iface_prefix",
								RESOLVCONF_PREFIX, lib->ns);
	}

	return &this->public;
}
