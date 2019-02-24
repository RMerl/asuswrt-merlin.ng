/*
 * Copyright (C) 2010 Martin Willi
 * Copyright (C) 2010 revosec AG
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

#include "fast_smtp.h"

#include <unistd.h>
#include <errno.h>

#include <utils/debug.h>

typedef struct private_fast_smtp_t private_fast_smtp_t;

/**
 * Private data of an fast_smtp_t object.
 */
struct private_fast_smtp_t {

	/**
	 * Public fast_smtp_t interface.
	 */
	fast_smtp_t public;

	/**
	 * file stream to SMTP server
	 */
	FILE *f;
};

/**
 * Read the response code from an SMTP server
 */
static int read_response(private_fast_smtp_t *this)
{
	char buf[256], *end;
	int res = 0;

	while (TRUE)
	{
		if (!fgets(buf, sizeof(buf), this->f))
		{
			return 0;
		}
		res = strtol(buf, &end, 10);
		switch (*end)
		{
			case '-':
				continue;
			case ' ':
			case '\0':
			case '\n':
				break;
			default:
				return 0;
		}
		break;
	}
	return res;
}

/**
 * write a SMTP command to the server, read response code
 */
static int write_cmd(private_fast_smtp_t *this, char *fmt, ...)
{
	char buf[256];
	va_list args;

	va_start(args, fmt);
	vsnprintf(buf, sizeof(buf), fmt, args);
	va_end(args);

	if (fprintf(this->f, "%s\n", buf) < 1)
	{
		DBG1(DBG_LIB, "sending SMTP command failed");
		return 0;
	}
	return read_response(this);
}

METHOD(fast_smtp_t, send_mail, bool,
	private_fast_smtp_t *this, char *from, char *to, char *subject, char *fmt, ...)
{
	va_list args;

	if (write_cmd(this, "MAIL FROM:<%s>", from) != 250)
	{
		DBG1(DBG_LIB, "SMTP MAIL FROM failed");
		return FALSE;
	}
	if (write_cmd(this, "RCPT TO:<%s>", to) != 250)
	{
		DBG1(DBG_LIB, "SMTP RCPT TO failed");
		return FALSE;
	}
	if (write_cmd(this, "DATA") != 354)
	{
		DBG1(DBG_LIB, "SMTP DATA failed");
		return FALSE;
	}

	fprintf(this->f, "From: %s\n", from);
	fprintf(this->f, "To: %s\n", to);
	fprintf(this->f, "Subject: %s\n", subject);
	fprintf(this->f, "\n");
	va_start(args, fmt);
	vfprintf(this->f, fmt, args);
	va_end(args);
	fprintf(this->f, "\n.\n");
	return read_response(this) == 250;
}


METHOD(fast_smtp_t, destroy, void,
	private_fast_smtp_t *this)
{
	write_cmd(this, "QUIT");
	fclose(this->f);
	free(this);
}

/**
 * See header
 */
fast_smtp_t *fast_smtp_create()
{
	private_fast_smtp_t *this;
	struct sockaddr_in addr = {
		.sin_family = AF_INET,
		.sin_port = htons(25),
		.sin_addr = {
			.s_addr = htonl(INADDR_LOOPBACK),
		},
	};
	int s;

	INIT(this,
		.public = {
			.send_mail = _send_mail,
			.destroy = _destroy,
		},
	);

	s = socket(AF_INET, SOCK_STREAM, 0);
	if (s < 0)
	{
		DBG1(DBG_LIB, "opening SMTP socket failed: %s", strerror(errno));
		free(this);
		return NULL;
	}
	if (connect(s, (struct sockaddr*)&addr, sizeof(addr)) < 0)
	{
		DBG1(DBG_LIB, "connecting to SMTP server failed: %s", strerror(errno));
		close(s);
		free(this);
		return NULL;
	}
	this->f = fdopen(s, "a+");
	if (!this->f)
	{
		DBG1(DBG_LIB, "opening stream to SMTP server failed: %s",
			 strerror(errno));
		close(s);
		free(this);
		return NULL;
	}
	if (read_response(this) != 220 ||
		write_cmd(this, "EHLO localhost") != 250)
	{
		DBG1(DBG_LIB, "SMTP EHLO failed");
		fclose(this->f);
		free(this);
		return NULL;
	}
	return &this->public;
}
