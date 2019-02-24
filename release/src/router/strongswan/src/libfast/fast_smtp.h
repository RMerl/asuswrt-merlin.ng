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

/**
 * @defgroup fast_smtp fast_smtp
 * @{ @ingroup libfast
 */

#ifndef FAST_SMTP_H_
#define FAST_SMTP_H_

typedef struct fast_smtp_t fast_smtp_t;

#include <library.h>

/**
 * Ultra-minimalistic SMTP client. Works at most with Exim on localhost.
 */
struct fast_smtp_t {

	/**
	 * Send an e-mail message.
	 *
	 * @param from		sender address
	 * @param to		recipient address
	 * @param subject	mail subject
	 * @param fmt		mail body format string
	 * @param ...		arguments for body format string
	 */
	bool (*send_mail)(fast_smtp_t *this, char *from, char *to,
					  char *subject, char *fmt, ...);

	/**
	 * Destroy a fast_smtp_t.
	 */
	void (*destroy)(fast_smtp_t *this);
};

/**
 * Create a smtp instance.
 */
fast_smtp_t *fast_smtp_create();

#endif /** FAST_SMTP_H_ @}*/
