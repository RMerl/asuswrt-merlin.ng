// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2021, Foundries.IO
 *
 */

#include <common.h>
#include <scp03.h>
#include <tee.h>
#include <tee/optee_ta_scp03.h>

static int scp03_enable(bool provision)
{
	const struct tee_optee_ta_uuid uuid = PTA_SCP03_UUID;
	struct tee_open_session_arg session;
	struct tee_invoke_arg invoke;
	struct tee_param param;
	struct udevice *tee = NULL;

	tee = tee_find_device(tee, NULL, NULL, NULL);
	if (!tee)
		return -ENODEV;

	memset(&session, 0, sizeof(session));
	tee_optee_ta_uuid_to_octets(session.uuid, &uuid);
	if (tee_open_session(tee, &session, 0, NULL))
		return -ENXIO;

	memset(&param, 0, sizeof(param));
	param.attr = TEE_PARAM_ATTR_TYPE_VALUE_INPUT;
	param.u.value.a = provision;

	memset(&invoke, 0, sizeof(invoke));
	invoke.func = PTA_CMD_ENABLE_SCP03;
	invoke.session = session.session;

	if (tee_invoke_func(tee, &invoke, 1, &param))
		return -EIO;

	tee_close_session(tee, session.session);

	return 0;
}

int tee_enable_scp03(void)
{
	return scp03_enable(false);
}

int tee_provision_scp03(void)
{
	return scp03_enable(true);
}
