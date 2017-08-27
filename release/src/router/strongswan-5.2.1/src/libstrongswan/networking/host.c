/*
 * Copyright (C) 2006-2012 Tobias Brunner
 * Copyright (C) 2006 Daniel Roethlisberger
 * Copyright (C) 2005-2006 Martin Willi
 * Copyright (C) 2005 Jan Hutter
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

#include "host.h"

#include <utils/debug.h>
#include <library.h>

#define IPV4_LEN	 4
#define IPV6_LEN	16

typedef struct private_host_t private_host_t;

/**
 * Private Data of a host object.
 */
struct private_host_t {
	/**
	 * Public data
	 */
	host_t public;

	/**
	 * low-lewel structure, which stores the address
	 */
	union {
		/** generic type */
		struct sockaddr address;
		/** maximum sockaddr size */
		struct sockaddr_storage address_max;
		/** IPv4 address */
		struct sockaddr_in address4;
		/** IPv6 address */
		struct sockaddr_in6 address6;
	};
	/**
	 * length of address structure
	 */
	socklen_t socklen;
};

/**
 * Update the sockaddr internal sa_len option, if available
 */
static inline void update_sa_len(private_host_t *this)
{
#ifdef HAVE_STRUCT_SOCKADDR_SA_LEN
	this->address.sa_len = this->socklen;
#endif /* HAVE_STRUCT_SOCKADDR_SA_LEN */
}

METHOD(host_t, get_sockaddr, sockaddr_t*,
	private_host_t *this)
{
	return &(this->address);
}

METHOD(host_t, get_sockaddr_len, socklen_t*,
	private_host_t *this)
{
	return &(this->socklen);
}

METHOD(host_t, is_anyaddr, bool,
	private_host_t *this)
{
	static const u_int8_t zeroes[IPV6_LEN];

	switch (this->address.sa_family)
	{
		case AF_INET:
		{
			return memeq(zeroes, &(this->address4.sin_addr.s_addr), IPV4_LEN);
		}
		case AF_INET6:
		{
			return memeq(zeroes, &(this->address6.sin6_addr.s6_addr), IPV6_LEN);
		}
		default:
		{
			return FALSE;
		}
	}
}

/**
 * Described in header.
 */
int host_printf_hook(printf_hook_data_t *data, printf_hook_spec_t *spec,
					 const void *const *args)
{
	private_host_t *this = *((private_host_t**)(args[0]));
	char buffer[INET6_ADDRSTRLEN + 16];

	if (this == NULL)
	{
		snprintf(buffer, sizeof(buffer), "(null)");
	}
	else if (is_anyaddr(this) && !spec->plus && !spec->hash)
	{
		snprintf(buffer, sizeof(buffer), "%%any%s",
				 this->address.sa_family == AF_INET6 ? "6" : "");
	}
	else
	{
		void *address;
		u_int16_t port;
		int len;

		address = &this->address6.sin6_addr;
		port = this->address6.sin6_port;

		switch (this->address.sa_family)
		{
			case AF_INET:
				address = &this->address4.sin_addr;
				port = this->address4.sin_port;
				/* fall */
			case AF_INET6:

				if (inet_ntop(this->address.sa_family, address,
							  buffer, sizeof(buffer)) == NULL)
				{
					snprintf(buffer, sizeof(buffer),
							 "(address conversion failed)");
				}
				else if (spec->hash)
				{
					len = strlen(buffer);
					snprintf(buffer + len, sizeof(buffer) - len,
							 "[%d]", ntohs(port));
				}
				break;
			default:
				snprintf(buffer, sizeof(buffer), "(family not supported)");
				break;
		}
	}
	if (spec->minus)
	{
		return print_in_hook(data, "%-*s", spec->width, buffer);
	}
	return print_in_hook(data, "%*s", spec->width, buffer);
}

METHOD(host_t, get_address, chunk_t,
	private_host_t *this)
{
	chunk_t address = chunk_empty;

	switch (this->address.sa_family)
	{
		case AF_INET:
		{
			address.ptr = (char*)&(this->address4.sin_addr.s_addr);
			address.len = IPV4_LEN;
			return address;
		}
		case AF_INET6:
		{
			address.ptr = (char*)&(this->address6.sin6_addr.s6_addr);
			address.len = IPV6_LEN;
			return address;
		}
		default:
		{
			/* return empty chunk */
			return address;
		}
	}
}

METHOD(host_t, get_family, int,
	private_host_t *this)
{
	return this->address.sa_family;
}

METHOD(host_t, get_port, u_int16_t,
	private_host_t *this)
{
	switch (this->address.sa_family)
	{
		case AF_INET:
		{
			return ntohs(this->address4.sin_port);
		}
		case AF_INET6:
		{
			return ntohs(this->address6.sin6_port);
		}
		default:
		{
			return 0;
		}
	}
}

METHOD(host_t, set_port, void,
	private_host_t *this, u_int16_t port)
{
	switch (this->address.sa_family)
	{
		case AF_INET:
		{
			this->address4.sin_port = htons(port);
			break;
		}
		case AF_INET6:
		{
			this->address6.sin6_port = htons(port);
			break;
		}
		default:
		{
			break;
		}
	}
}

METHOD(host_t, clone_, host_t*,
	private_host_t *this)
{
	private_host_t *new;

	new = malloc_thing(private_host_t);
	memcpy(new, this, sizeof(private_host_t));

	return &new->public;
}

/**
 * Implements host_t.ip_equals
 */
static bool ip_equals(private_host_t *this, private_host_t *other)
{
	if (this->address.sa_family != other->address.sa_family)
	{
		/* 0.0.0.0 and 0::0 are equal */
		return (is_anyaddr(this) && is_anyaddr(other));
	}

	switch (this->address.sa_family)
	{
		case AF_INET:
		{
			return memeq(&this->address4.sin_addr, &other->address4.sin_addr,
						 sizeof(this->address4.sin_addr));
		}
		case AF_INET6:
		{
			return memeq(&this->address6.sin6_addr, &other->address6.sin6_addr,
						 sizeof(this->address6.sin6_addr));
		}
		default:
			break;
	}
	return FALSE;
}

/**
 * Implements host_t.equals
 */
static bool equals(private_host_t *this, private_host_t *other)
{
	if (!ip_equals(this, other))
	{
		return FALSE;
	}

	switch (this->address.sa_family)
	{
		case AF_INET:
		{
			return (this->address4.sin_port == other->address4.sin_port);
		}
		case AF_INET6:
		{
			return (this->address6.sin6_port == other->address6.sin6_port);
		}
		default:
			break;
	}
	return FALSE;
}

METHOD(host_t, destroy, void,
	private_host_t *this)
{
	free(this);
}

/**
 * Creates an empty host_t object
 */
static private_host_t *host_create_empty(void)
{
	private_host_t *this;

	INIT(this,
		.public = {
			.get_sockaddr = _get_sockaddr,
			.get_sockaddr_len = _get_sockaddr_len,
			.clone = _clone_,
			.get_family = _get_family,
			.get_address = _get_address,
			.get_port = _get_port,
			.set_port = _set_port,
			.ip_equals = (bool (*)(host_t *,host_t *))ip_equals,
			.equals = (bool (*)(host_t *,host_t *)) equals,
			.is_anyaddr = _is_anyaddr,
			.destroy = _destroy,
		},
	);

	return this;
}

/*
 * Create a %any host with port
 */
static host_t *host_create_any_port(int family, u_int16_t port)
{
	host_t *this;

	this = host_create_any(family);
	this->set_port(this, port);
	return this;
}

/*
 * Described in header.
 */
host_t *host_create_from_string_and_family(char *string, int family,
										   u_int16_t port)
{
	union {
		struct sockaddr_in v4;
		struct sockaddr_in6 v6;
	} addr;

	if (streq(string, "%any"))
	{
		return host_create_any_port(family ? family : AF_INET, port);
	}
	if (family == AF_UNSPEC || family == AF_INET)
	{
		if (streq(string, "%any4") || streq(string, "0.0.0.0"))
		{
			return host_create_any_port(AF_INET, port);
		}
	}
	if (family == AF_UNSPEC || family == AF_INET6)
	{
		if (streq(string, "%any6") || streq(string, "::"))
		{
			return host_create_any_port(AF_INET6, port);
		}
	}
	switch (family)
	{
		case AF_UNSPEC:
			if (strchr(string, '.'))
			{
				goto af_inet;
			}
			/* FALL */
		case AF_INET6:
			memset(&addr.v6, 0, sizeof(addr.v6));
			if (inet_pton(AF_INET6, string, &addr.v6.sin6_addr) != 1)
			{
				return NULL;
			}
			addr.v6.sin6_port = htons(port);
			addr.v6.sin6_family = AF_INET6;
			return host_create_from_sockaddr((sockaddr_t*)&addr);
		case AF_INET:
			if (strchr(string, ':'))
			{	/* do not try to convert v6 addresses for v4 family */
				return NULL;
			}
		af_inet:
			memset(&addr.v4, 0, sizeof(addr.v4));
			if (inet_pton(AF_INET, string, &addr.v4.sin_addr) != 1)
			{
				return NULL;
			}
			addr.v4.sin_port = htons(port);
			addr.v4.sin_family = AF_INET;
			return host_create_from_sockaddr((sockaddr_t*)&addr);
		default:
			return NULL;
	}
}

/*
 * Described in header.
 */
host_t *host_create_from_string(char *string, u_int16_t port)
{
	return host_create_from_string_and_family(string, AF_UNSPEC, port);
}

/*
 * Described in header.
 */
host_t *host_create_from_sockaddr(sockaddr_t *sockaddr)
{
	private_host_t *this = host_create_empty();

	switch (sockaddr->sa_family)
	{
		case AF_INET:
		{
			memcpy(&this->address4, (struct sockaddr_in*)sockaddr,
				   sizeof(struct sockaddr_in));
			this->socklen = sizeof(struct sockaddr_in);
			update_sa_len(this);
			return &this->public;
		}
		case AF_INET6:
		{
			memcpy(&this->address6, (struct sockaddr_in6*)sockaddr,
				   sizeof(struct sockaddr_in6));
			this->socklen = sizeof(struct sockaddr_in6);
			update_sa_len(this);
			return &this->public;
		}
		default:
			break;
	}
	free(this);
	return NULL;
}

/*
 * Described in header.
 */
host_t *host_create_from_dns(char *string, int af, u_int16_t port)
{
	host_t *this;

	this = host_create_from_string_and_family(string, af, port);
	if (!this)
	{
		this = lib->hosts->resolve(lib->hosts, string, af);
	}
	if (this)
	{
		this->set_port(this, port);
	}
	return this;
}

/*
 * Described in header.
 */
host_t *host_create_from_chunk(int family, chunk_t address, u_int16_t port)
{
	private_host_t *this;

	switch (family)
	{
		case AF_INET:
			if (address.len < IPV4_LEN)
			{
				return NULL;
			}
			address.len = IPV4_LEN;
			break;
		case AF_INET6:
			if (address.len < IPV6_LEN)
			{
				return NULL;
			}
			address.len = IPV6_LEN;
			break;
		case AF_UNSPEC:
			switch (address.len)
			{
				case IPV4_LEN:
					family = AF_INET;
					break;
				case IPV6_LEN:
					family = AF_INET6;
					break;
				default:
					return NULL;
			}
			break;
		default:
			return NULL;
	}
	this = host_create_empty();
	this->address.sa_family = family;
	switch (family)
	{
		case AF_INET:
			memcpy(&this->address4.sin_addr.s_addr, address.ptr, address.len);
			this->address4.sin_port = htons(port);
			this->socklen = sizeof(struct sockaddr_in);
			break;
		case AF_INET6:
			memcpy(&this->address6.sin6_addr.s6_addr, address.ptr, address.len);
			this->address6.sin6_port = htons(port);
			this->socklen = sizeof(struct sockaddr_in6);
			break;
	}
	update_sa_len(this);
	return &this->public;
}

/*
 * Described in header.
 */
host_t *host_create_from_subnet(char *string, int *bits)
{
	char *pos, buf[64];
	host_t *net;

	pos = strchr(string, '/');
	if (pos)
	{
		if (pos - string >= sizeof(buf))
		{
			return NULL;
		}
		strncpy(buf, string, pos - string);
		buf[pos - string] = '\0';
		*bits = atoi(pos + 1);
		return host_create_from_string(buf, 0);
	}
	net = host_create_from_string(string, 0);
	if (net)
	{
		if (net->get_family(net) == AF_INET)
		{
			*bits = 32;
		}
		else
		{
			*bits = 128;
		}
	}
	return net;
}

/*
 * See header.
 */
host_t *host_create_netmask(int family, int netbits)
{
	private_host_t *this;
	int bits, bytes, len = 0;
	char *target;

	switch (family)
	{
		case AF_INET:
			if (netbits < 0 || netbits > 32)
			{
				return NULL;
			}
			this = host_create_empty();
			this->socklen = sizeof(struct sockaddr_in);
			target = (char*)&this->address4.sin_addr;
			len = 4;
			break;
		case AF_INET6:
			if (netbits < 0 || netbits > 128)
			{
				return NULL;
			}
			this = host_create_empty();
			this->socklen = sizeof(struct sockaddr_in6);
			target = (char*)&this->address6.sin6_addr;
			len = 16;
			break;
		default:
			return NULL;
	}

	memset(&this->address_max, 0, sizeof(struct sockaddr_storage));
	this->address.sa_family = family;
	update_sa_len(this);

	bytes = netbits / 8;
	bits = 8 - (netbits & 0x07);

	memset(target, 0xff, bytes);
	if (bytes < len)
	{
		memset(target + bytes, 0x00, len - bytes);
		target[bytes] = (u_int8_t)(0xff << bits);
	}
	return &this->public;
}

/*
 * Described in header.
 */
host_t *host_create_any(int family)
{
	private_host_t *this = host_create_empty();

	memset(&this->address_max, 0, sizeof(struct sockaddr_storage));
	this->address.sa_family = family;

	switch (family)
	{
		case AF_INET:
		{
			this->socklen = sizeof(struct sockaddr_in);
			update_sa_len(this);
			return &(this->public);
		}
		case AF_INET6:
		{
			this->socklen = sizeof(struct sockaddr_in6);
			update_sa_len(this);
			return &this->public;
		}
		default:
			break;
	}
	free(this);
	return NULL;
}
