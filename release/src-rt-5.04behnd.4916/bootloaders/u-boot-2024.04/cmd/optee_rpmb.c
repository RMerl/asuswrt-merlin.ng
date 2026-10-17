// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2020 NXP
 */

#include <command.h>
#include <common.h>
#include <env.h>
#include <errno.h>
#include <image.h>
#include <malloc.h>
#include <mmc.h>
#include <tee.h>
#include <tee/optee_ta_avb.h>

static struct udevice *tee;
static u32 session;

static int avb_ta_open_session(void)
{
	const struct tee_optee_ta_uuid uuid = TA_AVB_UUID;
	struct tee_open_session_arg arg;
	int rc;

	tee = tee_find_device(tee, NULL, NULL, NULL);
	if (!tee)
		return -ENODEV;

	memset(&arg, 0, sizeof(arg));
	tee_optee_ta_uuid_to_octets(arg.uuid, &uuid);
	rc = tee_open_session(tee, &arg, 0, NULL);
	if (!rc)
		session = arg.session;

	return 0;
}

static int invoke_func(u32 func, ulong num_param, struct tee_param *param)
{
	struct tee_invoke_arg arg;

	if (!tee)
		if (avb_ta_open_session())
			return -ENODEV;

	memset(&arg, 0, sizeof(arg));
	arg.func = func;
	arg.session = session;

	if (tee_invoke_func(tee, &arg, num_param, param))
		return -EFAULT;
	switch (arg.ret) {
	case TEE_SUCCESS:
		return 0;
	case TEE_ERROR_OUT_OF_MEMORY:
	case TEE_ERROR_STORAGE_NO_SPACE:
		return -ENOSPC;
	case TEE_ERROR_ITEM_NOT_FOUND:
		return -EIO;
	case TEE_ERROR_TARGET_DEAD:
		/*
		 * The TA has paniced, close the session to reload the TA
		 * for the next request.
		 */
		tee_close_session(tee, session);
		tee = NULL;
		return -EIO;
	default:
		return -EIO;
	}
}

static int read_persistent_value(const char *name,
				 size_t buffer_size,
				 u8 *out_buffer,
				 size_t *out_num_bytes_read)
{
	int rc = 0;
	struct tee_shm *shm_name;
	struct tee_shm *shm_buf;
	struct tee_param param[2];
	size_t name_size = strlen(name) + 1;

	if (!tee)
		if (avb_ta_open_session())
			return -ENODEV;

	rc = tee_shm_alloc(tee, name_size,
			   TEE_SHM_ALLOC, &shm_name);
	if (rc)
		return -ENOMEM;

	rc = tee_shm_alloc(tee, buffer_size,
			   TEE_SHM_ALLOC, &shm_buf);
	if (rc) {
		rc = -ENOMEM;
		goto free_name;
	}

	memcpy(shm_name->addr, name, name_size);

	memset(param, 0, sizeof(param));
	param[0].attr = TEE_PARAM_ATTR_TYPE_MEMREF_INPUT;
	param[0].u.memref.shm = shm_name;
	param[0].u.memref.size = name_size;
	param[1].attr = TEE_PARAM_ATTR_TYPE_MEMREF_INOUT;
	param[1].u.memref.shm = shm_buf;
	param[1].u.memref.size = buffer_size;

	rc = invoke_func(TA_AVB_CMD_READ_PERSIST_VALUE,
			 2, param);
	if (rc)
		goto out;

	if (param[1].u.memref.size > buffer_size) {
		rc = -EINVAL;
		goto out;
	}

	*out_num_bytes_read = param[1].u.memref.size;

	memcpy(out_buffer, shm_buf->addr, *out_num_bytes_read);

out:
	tee_shm_free(shm_buf);
free_name:
	tee_shm_free(shm_name);

	return rc;
}

static int write_persistent_value(const char *name,
				  size_t value_size,
				  const u8 *value)
{
	int rc = 0;
	struct tee_shm *shm_name;
	struct tee_shm *shm_buf;
	struct tee_param param[2];
	size_t name_size = strlen(name) + 1;

	if (!tee) {
		if (avb_ta_open_session())
			return -ENODEV;
	}
	if (!value_size)
		return -EINVAL;

	rc = tee_shm_alloc(tee, name_size,
			   TEE_SHM_ALLOC, &shm_name);
	if (rc)
		return -ENOMEM;

	rc = tee_shm_alloc(tee, value_size,
			   TEE_SHM_ALLOC, &shm_buf);
	if (rc) {
		rc = -ENOMEM;
		goto free_name;
	}

	memcpy(shm_name->addr, name, name_size);
	memcpy(shm_buf->addr, value, value_size);

	memset(param, 0, sizeof(param));
	param[0].attr = TEE_PARAM_ATTR_TYPE_MEMREF_INPUT;
	param[0].u.memref.shm = shm_name;
	param[0].u.memref.size = name_size;
	param[1].attr = TEE_PARAM_ATTR_TYPE_MEMREF_INPUT;
	param[1].u.memref.shm = shm_buf;
	param[1].u.memref.size = value_size;

	rc = invoke_func(TA_AVB_CMD_WRITE_PERSIST_VALUE,
			 2, param);
	if (rc)
		goto out;

out:
	tee_shm_free(shm_buf);
free_name:
	tee_shm_free(shm_name);

	return rc;
}

int do_optee_rpmb_read(struct cmd_tbl *cmdtp, int flag, int argc,
		       char * const argv[])
{
	const char *name;
	size_t bytes;
	size_t bytes_read;
	void *buffer;
	char *endp;

	if (argc != 3)
		return CMD_RET_USAGE;

	name = argv[1];
	bytes = dectoul(argv[2], &endp);
	if (*endp && *endp != '\n')
		return CMD_RET_USAGE;

	buffer = malloc(bytes);
	if (!buffer)
		return CMD_RET_FAILURE;

	if (read_persistent_value(name, bytes, buffer, &bytes_read) == 0) {
		printf("Read %zu bytes, value = %s\n", bytes_read,
		       (char *)buffer);
		free(buffer);
		return CMD_RET_SUCCESS;
	}

	printf("Failed to read persistent value\n");

	free(buffer);

	return CMD_RET_FAILURE;
}

int do_optee_rpmb_write(struct cmd_tbl *cmdtp, int flag, int argc,
			char * const argv[])
{
	const char *name;
	const char *value;

	if (argc != 3)
		return CMD_RET_USAGE;

	name = argv[1];
	value = argv[2];

	if (write_persistent_value(name, strlen(value) + 1,
				   (const uint8_t *)value) == 0) {
		printf("Wrote %zu bytes\n", strlen(value) + 1);
		return CMD_RET_SUCCESS;
	}

	printf("Failed to write persistent value\n");

	return CMD_RET_FAILURE;
}

static struct cmd_tbl cmd_optee_rpmb[] = {
	U_BOOT_CMD_MKENT(read_pvalue, 3, 0, do_optee_rpmb_read, "", ""),
	U_BOOT_CMD_MKENT(write_pvalue, 3, 0, do_optee_rpmb_write, "", ""),
};

static int do_optee_rpmb(struct cmd_tbl *cmdtp, int flag, int argc,
			 char * const argv[])
{
	struct cmd_tbl *cp;

	cp = find_cmd_tbl(argv[1], cmd_optee_rpmb, ARRAY_SIZE(cmd_optee_rpmb));

	argc--;
	argv++;

	if (!cp || argc > cp->maxargs)
		return CMD_RET_USAGE;

	if (flag == CMD_FLAG_REPEAT)
		return CMD_RET_FAILURE;

	return cp->cmd(cmdtp, flag, argc, argv);
}

U_BOOT_CMD (
	optee_rpmb, 29, 0, do_optee_rpmb,
	"Provides commands for testing secure storage on RPMB on OPTEE",
	"read_pvalue <name> <bytes> - read a persistent value <name>\n"
	"optee_rpmb write_pvalue <name> <value> - write a persistent value <name>\n"
	);
