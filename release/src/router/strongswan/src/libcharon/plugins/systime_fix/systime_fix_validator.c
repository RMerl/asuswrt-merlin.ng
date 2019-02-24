/*
 * Copyright (C) 2013 Martin Willi
 * Copyright (C) 2013 revosec AG
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

#include "systime_fix_validator.h"

#include <errno.h>
#include <time.h>

#include <daemon.h>

typedef struct private_systime_fix_validator_t private_systime_fix_validator_t;

/**
 * Private data of an systime_fix_validator_t object.
 */
struct private_systime_fix_validator_t {

	/**
	 * Public systime_fix_validator_t interface.
	 */
	systime_fix_validator_t public;

	/**
	 * Timestamp where we start to consider system time valid
	 */
	time_t threshold;
};

METHOD(cert_validator_t, check_lifetime, status_t,
	private_systime_fix_validator_t *this, certificate_t *cert,
	int pathlen, bool anchor, auth_cfg_t *auth)
{
	if (time(NULL) < this->threshold)
	{
		/* our system time seems to be invalid, accept certificate */
		if (pathlen)
		{	/* report only once per validated chain */
			DBG1(DBG_CFG, "system time out of sync, skipping certificate "
				 "lifetime check");
		}
		return SUCCESS;
	}
	/* validate this certificate normally */
	return NEED_MORE;
}

METHOD(systime_fix_validator_t, destroy, void,
	private_systime_fix_validator_t *this)
{
	free(this);
}

/**
 * See header
 */
systime_fix_validator_t *systime_fix_validator_create(time_t threshold)
{
	private_systime_fix_validator_t *this;

	INIT(this,
		.public = {
			.validator = {
				.check_lifetime = _check_lifetime,
			},
			.destroy = _destroy,
		},
		.threshold = threshold,
	);

	return &this->public;
}
