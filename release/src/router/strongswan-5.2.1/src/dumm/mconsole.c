/*
 * Copyright (C) 2007 Martin Willi
 * Hochschule fuer Technik Rapperswil
 * Copyright (C) 2001-2004 Jeff Dike
 *
 * Based on the "uml_mconsole" utility from Jeff Dike.
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

#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <errno.h>
#include <sys/un.h>

#include <utils/debug.h>

#include "mconsole.h"

#define MCONSOLE_MAGIC 0xcafebabe
#define MCONSOLE_VERSION 2
#define MCONSOLE_MAX_DATA 512

typedef struct private_mconsole_t private_mconsole_t;

struct private_mconsole_t {
	/** public interface */
	mconsole_t public;
	/** mconsole socket */
	int console;
	/** notify socket */
	int notify;
	/** address of uml socket */
	struct sockaddr_un uml;
	/** idle function */
	void (*idle)(void);
};

/**
 * mconsole message format from "arch/um/include/mconsole.h"
 */
typedef struct mconsole_request mconsole_request;
/** mconsole request message */
struct mconsole_request {
	u_int32_t magic;
	u_int32_t version;
	u_int32_t len;
	char data[MCONSOLE_MAX_DATA];
};


typedef struct mconsole_reply mconsole_reply;
/** mconsole reply message */
struct mconsole_reply {
	u_int32_t err;
	u_int32_t more;
	u_int32_t len;
	char data[MCONSOLE_MAX_DATA];
};

typedef struct mconsole_notify mconsole_notify;
/** mconsole notify message */
struct mconsole_notify {
	u_int32_t magic;
	u_int32_t version;
	enum {
		MCONSOLE_SOCKET,
		MCONSOLE_PANIC,
		MCONSOLE_HANG,
		MCONSOLE_USER_NOTIFY,
	} type;
	u_int32_t len;
	char data[MCONSOLE_MAX_DATA];
};

/**
 * send a request to UML using mconsole
 */
static int request(private_mconsole_t *this, void(*cb)(void*,char*,size_t),
				   void *data, char *command, ...)
{
	mconsole_request request;
	mconsole_reply reply;
	int len, flags = 0;
	va_list args;

	memset(&request, 0, sizeof(request));
	request.magic = MCONSOLE_MAGIC;
	request.version = MCONSOLE_VERSION;
	va_start(args, command);
	request.len = vsnprintf(request.data, sizeof(request.data), command, args);
	va_end(args);

	if (this->idle)
	{
		flags = MSG_DONTWAIT;
	}
	do
	{
		if (this->idle)
		{
			this->idle();
		}
		len = sendto(this->console, &request, sizeof(request), flags,
					 (struct sockaddr*)&this->uml, sizeof(this->uml));
	}
	while (len < 0 && (errno == EINTR || errno == EAGAIN));

	if (len < 0)
	{
		DBG1(DBG_LIB, "sending mconsole command to UML failed: %m");
		return -1;
	}
	do
	{
		len = recv(this->console, &reply, sizeof(reply), flags);
		if (len < 0 && (errno == EINTR || errno == EAGAIN))
		{
			if (this->idle)
			{
				this->idle();
			}
			continue;
		}
		if (len < 0)
		{
			DBG1(DBG_LIB, "receiving from mconsole failed: %m");
			return -1;
		}
		if (len > 0)
		{
			if (cb)
			{
				cb(data, reply.data, reply.len);
			}
			else if (reply.err)
			{
				if (reply.len && *reply.data)
				{
					DBG1(DBG_LIB, "received mconsole error %d: %.*s",
						 reply.err, (int)reply.len, reply.data);
				}
				break;
			}
		}
	}
	while (reply.more);

	return reply.err;
}

/**
 * ignore error message
 */
static void ignore(void *data, char *buf, size_t len)
{
}

METHOD(mconsole_t, add_iface, bool,
	private_mconsole_t *this, char *guest, char *host)
{
	int tries = 0;

	while (tries++ < 5)
	{
		if (request(this, ignore, NULL, "config %s=tuntap,%s", guest, host) == 0)
		{
			return TRUE;
		}
		usleep(10000 * tries * tries);
	}
	return FALSE;
}

METHOD(mconsole_t, del_iface, bool,
	private_mconsole_t *this, char *guest)
{
	if (request(this, NULL, NULL, "remove %s", guest) != 0)
	{
		return FALSE;
	}
	return TRUE;
}

METHOD(mconsole_t, exec, int,
	private_mconsole_t *this, void(*cb)(void*,char*,size_t), void *data,
	char *cmd)
{
	return request(this, cb, data, "%s", cmd);
}

/**
 * Poll until guest is ready
 */
static void wait_bootup(private_mconsole_t *this)
{
	/* wait for init process to appear */
	while (request(this, ignore, NULL, "exec ps -p 1 > /dev/null"))
	{
		if (this->idle)
		{
			this->idle();
		}
		usleep(100000);
	}
}

METHOD(mconsole_t, destroy, void,
	private_mconsole_t *this)
{
	close(this->console);
	close(this->notify);
	free(this);
}

/**
 * setup the mconsole notify connection and wait for its readiness
 */
static bool wait_for_notify(private_mconsole_t *this, char *nsock)
{
	struct sockaddr_un addr;
	mconsole_notify notify;
	int len, flags = 0;

	this->notify = socket(AF_UNIX, SOCK_DGRAM, 0);
	if (this->notify < 0)
	{
		DBG1(DBG_LIB, "opening mconsole notify socket failed: %m");
		return FALSE;
	}
	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, nsock, sizeof(addr.sun_path));
	if (bind(this->notify, (struct sockaddr*)&addr, sizeof(addr)) < 0)
	{
		DBG1(DBG_LIB, "binding mconsole notify socket to '%s' failed: %m",
			 nsock);
		close(this->notify);
		return FALSE;
	}
	if (this->idle)
	{
		flags = MSG_DONTWAIT;
	}
	do
	{
		if (this->idle)
		{
			this->idle();
		}
		len = recvfrom(this->notify, &notify, sizeof(notify), flags, NULL, 0);
	}
	while (len < 0 && (errno == EINTR || errno == EAGAIN));

	if (len < 0 || len >= sizeof(notify))
	{
		DBG1(DBG_LIB, "reading from mconsole notify socket failed: %m");
		close(this->notify);
		unlink(nsock);
		return FALSE;
	}
	if (notify.magic != MCONSOLE_MAGIC ||
		notify.version != MCONSOLE_VERSION ||
		notify.type != MCONSOLE_SOCKET)
	{
		DBG1(DBG_LIB, "received unexpected message from mconsole notify"
			 " socket: %b", &notify, sizeof(notify));
		close(this->notify);
		unlink(nsock);
		return FALSE;
	}
	memset(&this->uml, 0, sizeof(this->uml));
	this->uml.sun_family = AF_UNIX;
	strncpy(this->uml.sun_path, (char*)&notify.data, sizeof(this->uml.sun_path));
	return TRUE;
}

/**
 * setup the mconsole console connection
 */
static bool setup_console(private_mconsole_t *this)
{
	struct sockaddr_un addr;

	this->console = socket(AF_UNIX, SOCK_DGRAM, 0);
	if (this->console < 0)
	{
		DBG1(DBG_LIB, "opening mconsole socket failed: %m");
		return FALSE;
	}
	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	snprintf(&addr.sun_path[1], sizeof(addr.sun_path)-1, "%5d-%d",
			 getpid(), this->console);
	if (bind(this->console, (struct sockaddr*)&addr, sizeof(addr)) < 0)
	{
		DBG1(DBG_LIB, "binding mconsole socket to '%s' failed: %m",
			 &addr.sun_path[1]);
		close(this->console);
		return FALSE;
	}
	return TRUE;
}

/**
 * create the mconsole instance
 */
mconsole_t *mconsole_create(char *notify, void(*idle)(void))
{
	private_mconsole_t *this;

	INIT(this,
		.public = {
			.add_iface = _add_iface,
			.del_iface = _del_iface,
			.exec = _exec,
			.destroy = _destroy,
		},
		.idle = idle,
	);

	if (!wait_for_notify(this, notify))
	{
		free(this);
		return NULL;
	}

	if (!setup_console(this))
	{
		close(this->notify);
		unlink(notify);
		free(this);
		return NULL;
	}
	unlink(notify);

	wait_bootup(this);

	return &this->public;
}

