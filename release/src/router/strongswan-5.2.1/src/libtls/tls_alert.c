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

#include "tls_alert.h"

#include <utils/debug.h>
#include <collections/linked_list.h>

ENUM_BEGIN(tls_alert_desc_names, TLS_CLOSE_NOTIFY, TLS_CLOSE_NOTIFY,
	"close notify",
);
ENUM_NEXT(tls_alert_desc_names, TLS_UNEXPECTED_MESSAGE, TLS_UNEXPECTED_MESSAGE,
		TLS_CLOSE_NOTIFY,
	"unexpected message",
);
ENUM_NEXT(tls_alert_desc_names, TLS_BAD_RECORD_MAC, TLS_RECORD_OVERFLOW,
		TLS_UNEXPECTED_MESSAGE,
	"bad record mac",
	"decryption failed",
	"record overflow",
);
ENUM_NEXT(tls_alert_desc_names, TLS_DECOMPRESSION_FAILURE, TLS_DECOMPRESSION_FAILURE,
		TLS_RECORD_OVERFLOW,
	"decompression_failure",
);
ENUM_NEXT(tls_alert_desc_names, TLS_HANDSHAKE_FAILURE, TLS_DECRYPT_ERROR,
		TLS_DECOMPRESSION_FAILURE,
	"handshake failure",
	"no certificate",
	"bad certificate",
	"unsupported certificate",
	"certificate revoked",
	"certificate expired",
	"certificate unknown",
	"illegal parameter",
	"unknown ca",
	"access denied",
	"decode error",
	"decrypt error",
);
ENUM_NEXT(tls_alert_desc_names, TLS_EXPORT_RESTRICTION, TLS_EXPORT_RESTRICTION,
		TLS_DECRYPT_ERROR,
	"export restriction",
);
ENUM_NEXT(tls_alert_desc_names, TLS_PROTOCOL_VERSION, TLS_INSUFFICIENT_SECURITY,
		TLS_EXPORT_RESTRICTION,
	"protocol version",
	"insufficient security",
);
ENUM_NEXT(tls_alert_desc_names, TLS_INTERNAL_ERROR, TLS_INTERNAL_ERROR,
		TLS_INSUFFICIENT_SECURITY,
	"internal error",
);
ENUM_NEXT(tls_alert_desc_names, TLS_USER_CANCELED, TLS_USER_CANCELED,
		TLS_INTERNAL_ERROR,
	"user canceled",
);
ENUM_NEXT(tls_alert_desc_names, TLS_NO_RENEGOTIATION, TLS_NO_RENEGOTIATION,
		TLS_USER_CANCELED,
	"no renegotiation",
);
ENUM_NEXT(tls_alert_desc_names, TLS_UNSUPPORTED_EXTENSION, TLS_UNSUPPORTED_EXTENSION,
		TLS_NO_RENEGOTIATION,
	"unsupported extension",
);
ENUM_END(tls_alert_desc_names, TLS_UNSUPPORTED_EXTENSION);


typedef struct private_tls_alert_t private_tls_alert_t;

/**
 * Private data of an tls_alert_t object.
 */
struct private_tls_alert_t {

	/**
	 * Public tls_alert_t interface.
	 */
	tls_alert_t public;

	/**
	 * Warning queue
	 */
	linked_list_t *warnings;

	/**
	 * Do we have a fatal alert?
	 */
	bool fatal;

	/**
	 * Has the fatal alert been consumed?
	 */
	bool consumed;

	/**
	 * Fatal alert discription
	 */
	tls_alert_desc_t desc;
};

METHOD(tls_alert_t, add, void,
	private_tls_alert_t *this, tls_alert_level_t level,
	tls_alert_desc_t desc)
{
	if (level == TLS_FATAL)
	{
		if (!this->fatal)
		{
			this->desc = desc;
			this->fatal = TRUE;
		}
	}
	else
	{
		this->warnings->insert_last(this->warnings, (void*)(uintptr_t)desc);
	}
}

METHOD(tls_alert_t, get, bool,
	private_tls_alert_t *this, tls_alert_level_t *level,
	tls_alert_desc_t *desc)
{
	if (this->fatal && !this->consumed)
	{
		this->consumed = TRUE;
		*level = TLS_FATAL;
		*desc = this->desc;
		if (this->desc == TLS_CLOSE_NOTIFY)
		{
			DBG1(DBG_TLS, "sending TLS close notify");
		}
		else
		{
			DBG1(DBG_TLS, "sending fatal TLS alert '%N'",
				 tls_alert_desc_names, this->desc);
		}
		return TRUE;
	}
	else
	{
		uintptr_t warning;

		if (this->warnings->remove_first(this->warnings,
										 (void**)&warning) == SUCCESS)
		{
			*level = TLS_WARNING;
			*desc = warning;
			DBG1(DBG_TLS, "sending TLS alert warning '%N'",
				 tls_alert_desc_names, warning);
			return TRUE;
		}
	}
	return FALSE;
}

METHOD(tls_alert_t, fatal, bool,
	private_tls_alert_t *this)
{
	return this->fatal;
}

METHOD(tls_alert_t, process, status_t,
	private_tls_alert_t *this, tls_alert_level_t level,
	tls_alert_desc_t desc)
{
	if (desc == TLS_CLOSE_NOTIFY)
	{
		DBG1(DBG_TLS, "received TLS close notify");
		add(this, TLS_FATAL, TLS_CLOSE_NOTIFY);
		return NEED_MORE;
	}
	switch (level)
	{
		case TLS_WARNING:
			DBG1(DBG_TLS, "received TLS alert warning '%N'",
				 tls_alert_desc_names, desc);
			return NEED_MORE;
		case TLS_FATAL:
			DBG1(DBG_TLS, "received fatal TLS alert '%N'",
				 tls_alert_desc_names, desc);
			return FAILED;
		default:
			DBG1(DBG_TLS, "received unknown TLS alert '%N'",
				 tls_alert_desc_names, desc);
			return FAILED;
	}
}

METHOD(tls_alert_t, destroy, void,
	private_tls_alert_t *this)
{
	this->warnings->destroy(this->warnings);
	free(this);
}

/**
 * See header
 */
tls_alert_t *tls_alert_create()
{
	private_tls_alert_t *this;

	INIT(this,
		.public = {
			.add = _add,
			.get = _get,
			.fatal = _fatal,
			.process = _process,
			.destroy = _destroy,
		},
		.warnings = linked_list_create(),
	);

	return &this->public;
}
