// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright 2020 Broadcom.
 */

#include <common.h>
#include <tee.h>
#include <broadcom/chimp.h>

#ifdef CONFIG_CHIMP_OPTEE

#define CHMIP_BOOT_UUID { 0x6272636D, 0x2019, 0x0716, \
		   { 0x42, 0x43, 0x4D, 0x5F, 0x53, 0x43, 0x48, 0x49 } }

enum {
	TEE_CHIMP_FASTBOOT = 0,
	TEE_CHIMP_HEALTH_STATUS,
	TEE_CHIMP_HANDSHAKE_STATUS,
} tee_chmip_cmd;

struct bcm_chimp_data {
	struct udevice *tee;
	u32 session;
} chimp_data;

static int get_open_session(struct bcm_chimp_data *b_data)
{
	const struct tee_optee_ta_uuid uuid = CHMIP_BOOT_UUID;
	struct tee_open_session_arg arg;
	struct udevice *tee = NULL;
	int rc;

	tee = tee_find_device(NULL, NULL, NULL, NULL);
	if (!tee)
		return -ENODEV;

	memset(&arg, 0, sizeof(arg));
	tee_optee_ta_uuid_to_octets(arg.uuid, &uuid);
	rc = tee_open_session(tee, &arg, 0, NULL);
	if (rc < 0)
		return -ENODEV;

	b_data->tee = tee;
	b_data->session = arg.session;

	return 0;
}

static int init_arg(struct tee_invoke_arg *arg, u32 func)
{
	if (get_open_session(&chimp_data))
		return -EINVAL;

	memset(arg, 0, sizeof(struct tee_invoke_arg));
	arg->func = func;
	arg->session = chimp_data.session;

	return 0;
}

int chimp_handshake_status_optee(u32 timeout, u32 *hs)
{
	struct tee_invoke_arg arg;
	struct tee_param param[1];
	int ret;

	ret = init_arg(&arg, TEE_CHIMP_HANDSHAKE_STATUS);
	if (ret < 0)
		return ret;

	param[0].attr = TEE_PARAM_ATTR_TYPE_VALUE_INOUT;
	param[0].u.value.a = timeout;

	ret = tee_invoke_func(chimp_data.tee, &arg, ARRAY_SIZE(param), param);
	if (ret < 0) {
		printf("Handshake status command failed\n");
		goto out;
	}

	switch (arg.ret) {
	case TEE_SUCCESS:
		*hs = param[0].u.value.a;
		ret =  0;
		break;
	default:
		ret = -EINVAL;
		break;
	}

out:
	tee_close_session(chimp_data.tee, chimp_data.session);
	chimp_data.tee = NULL;

	return ret;
}

int chimp_health_status_optee(u32 *health)
{
	struct tee_invoke_arg arg;
	struct tee_param param[1];
	int ret;

	ret = init_arg(&arg, TEE_CHIMP_HEALTH_STATUS);
	if (ret < 0)
		return ret;

	param[0].attr = TEE_PARAM_ATTR_TYPE_VALUE_OUTPUT;

	ret = tee_invoke_func(chimp_data.tee, &arg, ARRAY_SIZE(param), param);
	if (ret < 0) {
		printf("Helath status command failed\n");
		goto out;
	}

	switch (arg.ret) {
	case TEE_SUCCESS:
		*health = param[0].u.value.a;
		ret =  0;
		break;
	default:
		ret = -EINVAL;
		break;
	}

out:
	tee_close_session(chimp_data.tee, chimp_data.session);
	chimp_data.tee = NULL;

	return ret;
}

int chimp_fastboot_optee(void)
{
	struct tee_invoke_arg arg;
	int ret;

	ret = init_arg(&arg, TEE_CHIMP_FASTBOOT);
	if (ret < 0)
		return ret;

	ret = tee_invoke_func(chimp_data.tee, &arg, 0, NULL);
	if (ret < 0) {
		printf("Chimp boot_fail\n");
		goto out;
	}

	switch (arg.ret) {
	case TEE_SUCCESS:
		ret = 0;
		break;
	default:
		ret = -EINVAL;
		break;
	}

out:
	tee_close_session(chimp_data.tee, chimp_data.session);
	chimp_data.tee = NULL;

	return ret;
}
#else
int chimp_handshake_status_optee(u32 timeout, u32 *status)
{
	printf("ChiMP handshake status fail (OPTEE not enabled)\n");

	return -EINVAL;
}

int chimp_health_status_optee(u32 *status)
{
	printf("ChiMP health status fail (OPTEE not enabled)\n");

	return -EINVAL;
}

int chimp_fastboot_optee(void)
{
	printf("ChiMP secure boot fail (OPTEE not enabled)\n");

	return -EINVAL;
}
#endif /* CONFIG_CHIMP_OPTEE */
