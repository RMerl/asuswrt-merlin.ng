// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2013 The Chromium OS Authors.
 */

#include <common.h>
#include <malloc.h>
#include <asm/unaligned.h>
#include <tpm-common.h>
#include <tpm-v1.h>
#include "tpm-user-utils.h"

static int do_tpm_startup(cmd_tbl_t *cmdtp, int flag, int argc,
			  char * const argv[])
{
	enum tpm_startup_type mode;
	struct udevice *dev;
	int rc;

	rc = get_tpm(&dev);
	if (rc)
		return rc;
	if (argc != 2)
		return CMD_RET_USAGE;
	if (!strcasecmp("TPM_ST_CLEAR", argv[1])) {
		mode = TPM_ST_CLEAR;
	} else if (!strcasecmp("TPM_ST_STATE", argv[1])) {
		mode = TPM_ST_STATE;
	} else if (!strcasecmp("TPM_ST_DEACTIVATED", argv[1])) {
		mode = TPM_ST_DEACTIVATED;
	} else {
		printf("Couldn't recognize mode string: %s\n", argv[1]);
		return CMD_RET_FAILURE;
	}

	return report_return_code(tpm_startup(dev, mode));
}

static int do_tpm_nv_define_space(cmd_tbl_t *cmdtp, int flag, int argc,
				  char * const argv[])
{
	u32 index, perm, size;
	struct udevice *dev;
	int rc;

	rc = get_tpm(&dev);
	if (rc)
		return rc;

	if (argc != 4)
		return CMD_RET_USAGE;
	index = simple_strtoul(argv[1], NULL, 0);
	perm = simple_strtoul(argv[2], NULL, 0);
	size = simple_strtoul(argv[3], NULL, 0);

	return report_return_code(tpm_nv_define_space(dev, index, perm, size));
}

static int do_tpm_nv_read_value(cmd_tbl_t *cmdtp, int flag, int argc,
				char * const argv[])
{
	u32 index, count, rc;
	struct udevice *dev;
	void *data;

	rc = get_tpm(&dev);
	if (rc)
		return rc;

	if (argc != 4)
		return CMD_RET_USAGE;
	index = simple_strtoul(argv[1], NULL, 0);
	data = (void *)simple_strtoul(argv[2], NULL, 0);
	count = simple_strtoul(argv[3], NULL, 0);

	rc = tpm_nv_read_value(dev, index, data, count);
	if (!rc) {
		puts("area content:\n");
		print_byte_string(data, count);
	}

	return report_return_code(rc);
}

static int do_tpm_nv_write_value(cmd_tbl_t *cmdtp, int flag, int argc,
				 char * const argv[])
{
	struct udevice *dev;
	u32 index, rc;
	size_t count;
	void *data;

	rc = get_tpm(&dev);
	if (rc)
		return rc;

	if (argc != 3)
		return CMD_RET_USAGE;
	index = simple_strtoul(argv[1], NULL, 0);
	data = parse_byte_string(argv[2], NULL, &count);
	if (!data) {
		printf("Couldn't parse byte string %s\n", argv[2]);
		return CMD_RET_FAILURE;
	}

	rc = tpm_nv_write_value(dev, index, data, count);
	free(data);

	return report_return_code(rc);
}

static int do_tpm_extend(cmd_tbl_t *cmdtp, int flag, int argc,
			 char * const argv[])
{
	u8 in_digest[20], out_digest[20];
	struct udevice *dev;
	u32 index, rc;

	rc = get_tpm(&dev);
	if (rc)
		return rc;

	if (argc != 3)
		return CMD_RET_USAGE;
	index = simple_strtoul(argv[1], NULL, 0);
	if (!parse_byte_string(argv[2], in_digest, NULL)) {
		printf("Couldn't parse byte string %s\n", argv[2]);
		return CMD_RET_FAILURE;
	}

	rc = tpm_extend(dev, index, in_digest, out_digest);
	if (!rc) {
		puts("PCR value after execution of the command:\n");
		print_byte_string(out_digest, sizeof(out_digest));
	}

	return report_return_code(rc);
}

static int do_tpm_pcr_read(cmd_tbl_t *cmdtp, int flag, int argc,
			   char * const argv[])
{
	u32 index, count, rc;
	struct udevice *dev;
	void *data;

	rc = get_tpm(&dev);
	if (rc)
		return rc;

	if (argc != 4)
		return CMD_RET_USAGE;
	index = simple_strtoul(argv[1], NULL, 0);
	data = (void *)simple_strtoul(argv[2], NULL, 0);
	count = simple_strtoul(argv[3], NULL, 0);

	rc = tpm_pcr_read(dev, index, data, count);
	if (!rc) {
		puts("Named PCR content:\n");
		print_byte_string(data, count);
	}

	return report_return_code(rc);
}

static int do_tpm_tsc_physical_presence(cmd_tbl_t *cmdtp, int flag, int argc,
					char * const argv[])
{
	struct udevice *dev;
	u16 presence;
	int rc;

	rc = get_tpm(&dev);
	if (rc)
		return rc;

	if (argc != 2)
		return CMD_RET_USAGE;
	presence = (u16)simple_strtoul(argv[1], NULL, 0);

	return report_return_code(tpm_tsc_physical_presence(dev, presence));
}

static int do_tpm_read_pubek(cmd_tbl_t *cmdtp, int flag, int argc,
			     char * const argv[])
{
	struct udevice *dev;
	u32 count, rc;
	void *data;

	rc = get_tpm(&dev);
	if (rc)
		return rc;

	if (argc != 3)
		return CMD_RET_USAGE;
	data = (void *)simple_strtoul(argv[1], NULL, 0);
	count = simple_strtoul(argv[2], NULL, 0);

	rc = tpm_read_pubek(dev, data, count);
	if (!rc) {
		puts("pubek value:\n");
		print_byte_string(data, count);
	}

	return report_return_code(rc);
}

static int do_tpm_physical_set_deactivated(cmd_tbl_t *cmdtp, int flag, int argc,
					   char * const argv[])
{
	struct udevice *dev;
	u8 state;
	int rc;

	rc = get_tpm(&dev);
	if (rc)
		return rc;

	if (argc != 2)
		return CMD_RET_USAGE;
	state = (u8)simple_strtoul(argv[1], NULL, 0);

	return report_return_code(tpm_physical_set_deactivated(dev, state));
}

static int do_tpm_get_capability(cmd_tbl_t *cmdtp, int flag, int argc,
				 char * const argv[])
{
	u32 cap_area, sub_cap, rc;
	void *cap;
	size_t count;
	struct udevice *dev;

	rc = get_tpm(&dev);
	if (rc)
		return rc;

	if (argc != 5)
		return CMD_RET_USAGE;
	cap_area = simple_strtoul(argv[1], NULL, 0);
	sub_cap = simple_strtoul(argv[2], NULL, 0);
	cap = (void *)simple_strtoul(argv[3], NULL, 0);
	count = simple_strtoul(argv[4], NULL, 0);

	rc = tpm_get_capability(dev, cap_area, sub_cap, cap, count);
	if (!rc) {
		puts("capability information:\n");
		print_byte_string(cap, count);
	}

	return report_return_code(rc);
}

static int do_tpm_raw_transfer(cmd_tbl_t *cmdtp, int flag, int argc,
			       char * const argv[])
{
	struct udevice *dev;
	void *command;
	u8 response[1024];
	size_t count, response_length = sizeof(response);
	u32 rc;

	command = parse_byte_string(argv[1], NULL, &count);
	if (!command) {
		printf("Couldn't parse byte string %s\n", argv[1]);
		return CMD_RET_FAILURE;
	}

	rc = get_tpm(&dev);
	if (rc)
		return rc;

	rc = tpm_xfer(dev, command, count, response, &response_length);
	free(command);
	if (!rc) {
		puts("tpm response:\n");
		print_byte_string(response, response_length);
	}

	return report_return_code(rc);
}

static int do_tpm_nv_define(cmd_tbl_t *cmdtp, int flag, int argc,
			    char * const argv[])
{
	u32 index, perm, size;
	struct udevice *dev;
	int rc;

	rc = get_tpm(&dev);
	if (rc)
		return rc;

	if (argc != 4)
		return CMD_RET_USAGE;
	size = type_string_get_space_size(argv[1]);
	if (!size) {
		printf("Couldn't parse arguments\n");
		return CMD_RET_USAGE;
	}
	index = simple_strtoul(argv[2], NULL, 0);
	perm = simple_strtoul(argv[3], NULL, 0);

	return report_return_code(tpm_nv_define_space(dev, index, perm, size));
}

static int do_tpm_nv_read(cmd_tbl_t *cmdtp, int flag, int argc,
			  char * const argv[])
{
	u32 index, count, err;
	struct udevice *dev;
	void *data;
	int rc;

	rc = get_tpm(&dev);
	if (rc)
		return rc;

	if (argc < 3)
		return CMD_RET_USAGE;
	if (argc != 3 + type_string_get_num_values(argv[1]))
		return CMD_RET_USAGE;
	index = simple_strtoul(argv[2], NULL, 0);
	data = type_string_alloc(argv[1], &count);
	if (!data) {
		printf("Couldn't parse arguments\n");
		return CMD_RET_USAGE;
	}

	err = tpm_nv_read_value(dev, index, data, count);
	if (!err) {
		if (type_string_write_vars(argv[1], data, argv + 3)) {
			printf("Couldn't write to variables\n");
			err = ~0;
		}
	}
	free(data);

	return report_return_code(err);
}

static int do_tpm_nv_write(cmd_tbl_t *cmdtp, int flag, int argc,
			   char * const argv[])
{
	u32 index, count, err;
	struct udevice *dev;
	void *data;
	int rc;

	rc = get_tpm(&dev);
	if (rc)
		return rc;

	if (argc < 3)
		return CMD_RET_USAGE;
	if (argc != 3 + type_string_get_num_values(argv[1]))
		return CMD_RET_USAGE;
	index = simple_strtoul(argv[2], NULL, 0);
	data = type_string_alloc(argv[1], &count);
	if (!data) {
		printf("Couldn't parse arguments\n");
		return CMD_RET_USAGE;
	}
	if (type_string_pack(argv[1], argv + 3, data)) {
		printf("Couldn't parse arguments\n");
		free(data);
		return CMD_RET_USAGE;
	}

	err = tpm_nv_write_value(dev, index, data, count);
	free(data);

	return report_return_code(err);
}

#ifdef CONFIG_TPM_AUTH_SESSIONS

static int do_tpm_oiap(cmd_tbl_t *cmdtp, int flag, int argc,
		       char * const argv[])
{
	u32 auth_handle, err;
	struct udevice *dev;
	int rc;

	rc = get_tpm(&dev);
	if (rc)
		return rc;

	err = tpm_oiap(dev, &auth_handle);

	return report_return_code(err);
}

#ifdef CONFIG_TPM_LOAD_KEY_BY_SHA1
static int do_tpm_load_key_by_sha1(cmd_tbl_t *cmdtp, int flag, int argc, char *
				   const argv[])
{
	u32 parent_handle = 0;
	u32 key_len, key_handle, err;
	u8 usage_auth[DIGEST_LENGTH];
	u8 parent_hash[DIGEST_LENGTH];
	void *key;
	struct udevice *dev;

	rc = get_tpm(&dev);
	if (rc)
		return rc;

	if (argc < 5)
		return CMD_RET_USAGE;

	parse_byte_string(argv[1], parent_hash, NULL);
	key = (void *)simple_strtoul(argv[2], NULL, 0);
	key_len = simple_strtoul(argv[3], NULL, 0);
	if (strlen(argv[4]) != 2 * DIGEST_LENGTH)
		return CMD_RET_FAILURE;
	parse_byte_string(argv[4], usage_auth, NULL);

	err = tpm_find_key_sha1(usage_auth, parent_hash, &parent_handle);
	if (err) {
		printf("Could not find matching parent key (err = %d)\n", err);
		return CMD_RET_FAILURE;
	}

	printf("Found parent key %08x\n", parent_handle);

	err = tpm_load_key2_oiap(parent_handle, key, key_len, usage_auth,
				 &key_handle);
	if (!err) {
		printf("Key handle is 0x%x\n", key_handle);
		env_set_hex("key_handle", key_handle);
	}

	return report_return_code(err);
}
#endif /* CONFIG_TPM_LOAD_KEY_BY_SHA1 */

static int do_tpm_load_key2_oiap(cmd_tbl_t *cmdtp, int flag, int argc,
				 char * const argv[])
{
	u32 parent_handle, key_len, key_handle, err;
	u8 usage_auth[DIGEST_LENGTH];
	void *key;
	struct udevice *dev;
	int rc;

	rc = get_tpm(&dev);
	if (rc)
		return rc;

	if (argc < 5)
		return CMD_RET_USAGE;

	parent_handle = simple_strtoul(argv[1], NULL, 0);
	key = (void *)simple_strtoul(argv[2], NULL, 0);
	key_len = simple_strtoul(argv[3], NULL, 0);
	if (strlen(argv[4]) != 2 * DIGEST_LENGTH)
		return CMD_RET_FAILURE;
	parse_byte_string(argv[4], usage_auth, NULL);

	err = tpm_load_key2_oiap(dev, parent_handle, key, key_len, usage_auth,
				 &key_handle);
	if (!err)
		printf("Key handle is 0x%x\n", key_handle);

	return report_return_code(err);
}

static int do_tpm_get_pub_key_oiap(cmd_tbl_t *cmdtp, int flag, int argc,
				   char * const argv[])
{
	u32 key_handle, err;
	u8 usage_auth[DIGEST_LENGTH];
	u8 pub_key_buffer[TPM_PUBKEY_MAX_LENGTH];
	size_t pub_key_len = sizeof(pub_key_buffer);
	struct udevice *dev;
	int rc;

	rc = get_tpm(&dev);
	if (rc)
		return rc;

	if (argc < 3)
		return CMD_RET_USAGE;

	key_handle = simple_strtoul(argv[1], NULL, 0);
	if (strlen(argv[2]) != 2 * DIGEST_LENGTH)
		return CMD_RET_FAILURE;
	parse_byte_string(argv[2], usage_auth, NULL);

	err = tpm_get_pub_key_oiap(dev, key_handle, usage_auth, pub_key_buffer,
				   &pub_key_len);
	if (!err) {
		printf("dump of received pub key structure:\n");
		print_byte_string(pub_key_buffer, pub_key_len);
	}
	return report_return_code(err);
}

TPM_COMMAND_NO_ARG(tpm_end_oiap)

#endif /* CONFIG_TPM_AUTH_SESSIONS */

#ifdef CONFIG_TPM_FLUSH_RESOURCES
static int do_tpm_flush(cmd_tbl_t *cmdtp, int flag, int argc,
			char * const argv[])
{
	struct udevice *dev;
	int type = 0;
	int rc;

	rc = get_tpm(&dev);
	if (rc)
		return rc;

	if (argc != 3)
		return CMD_RET_USAGE;

	if (!strcasecmp(argv[1], "key"))
		type = TPM_RT_KEY;
	else if (!strcasecmp(argv[1], "auth"))
		type = TPM_RT_AUTH;
	else if (!strcasecmp(argv[1], "hash"))
		type = TPM_RT_HASH;
	else if (!strcasecmp(argv[1], "trans"))
		type = TPM_RT_TRANS;
	else if (!strcasecmp(argv[1], "context"))
		type = TPM_RT_CONTEXT;
	else if (!strcasecmp(argv[1], "counter"))
		type = TPM_RT_COUNTER;
	else if (!strcasecmp(argv[1], "delegate"))
		type = TPM_RT_DELEGATE;
	else if (!strcasecmp(argv[1], "daa_tpm"))
		type = TPM_RT_DAA_TPM;
	else if (!strcasecmp(argv[1], "daa_v0"))
		type = TPM_RT_DAA_V0;
	else if (!strcasecmp(argv[1], "daa_v1"))
		type = TPM_RT_DAA_V1;

	if (!type) {
		printf("Resource type %s unknown.\n", argv[1]);
		return -1;
	}

	if (!strcasecmp(argv[2], "all")) {
		u16 res_count;
		u8 buf[288];
		u8 *ptr;
		int err;
		uint i;

		/* fetch list of already loaded resources in the TPM */
		err = tpm_get_capability(dev, TPM_CAP_HANDLE, type, buf,
					 sizeof(buf));
		if (err) {
			printf("tpm_get_capability returned error %d.\n", err);
			return -1;
		}
		res_count = get_unaligned_be16(buf);
		ptr = buf + 2;
		for (i = 0; i < res_count; ++i, ptr += 4)
			tpm_flush_specific(dev, get_unaligned_be32(ptr), type);
	} else {
		u32 handle = simple_strtoul(argv[2], NULL, 0);

		if (!handle) {
			printf("Illegal resource handle %s\n", argv[2]);
			return -1;
		}
		tpm_flush_specific(dev, cpu_to_be32(handle), type);
	}

	return 0;
}
#endif /* CONFIG_TPM_FLUSH_RESOURCES */

#ifdef CONFIG_TPM_LIST_RESOURCES
static int do_tpm_list(cmd_tbl_t *cmdtp, int flag, int argc,
		       char * const argv[])
{
	int type = 0;
	u16 res_count;
	u8 buf[288];
	u8 *ptr;
	int err;
	uint i;

	if (argc != 2)
		return CMD_RET_USAGE;

	if (!strcasecmp(argv[1], "key"))
		type = TPM_RT_KEY;
	else if (!strcasecmp(argv[1], "auth"))
		type = TPM_RT_AUTH;
	else if (!strcasecmp(argv[1], "hash"))
		type = TPM_RT_HASH;
	else if (!strcasecmp(argv[1], "trans"))
		type = TPM_RT_TRANS;
	else if (!strcasecmp(argv[1], "context"))
		type = TPM_RT_CONTEXT;
	else if (!strcasecmp(argv[1], "counter"))
		type = TPM_RT_COUNTER;
	else if (!strcasecmp(argv[1], "delegate"))
		type = TPM_RT_DELEGATE;
	else if (!strcasecmp(argv[1], "daa_tpm"))
		type = TPM_RT_DAA_TPM;
	else if (!strcasecmp(argv[1], "daa_v0"))
		type = TPM_RT_DAA_V0;
	else if (!strcasecmp(argv[1], "daa_v1"))
		type = TPM_RT_DAA_V1;

	if (!type) {
		printf("Resource type %s unknown.\n", argv[1]);
		return -1;
	}

	/* fetch list of already loaded resources in the TPM */
	err = tpm_get_capability(TPM_CAP_HANDLE, type, buf,
				 sizeof(buf));
	if (err) {
		printf("tpm_get_capability returned error %d.\n", err);
		return -1;
	}
	res_count = get_unaligned_be16(buf);
	ptr = buf + 2;

	printf("Resources of type %s (%02x):\n", argv[1], type);
	if (!res_count) {
		puts("None\n");
	} else {
		for (i = 0; i < res_count; ++i, ptr += 4)
			printf("Index %d: %08x\n", i, get_unaligned_be32(ptr));
	}

	return 0;
}
#endif /* CONFIG_TPM_LIST_RESOURCES */

TPM_COMMAND_NO_ARG(tpm_self_test_full)
TPM_COMMAND_NO_ARG(tpm_continue_self_test)
TPM_COMMAND_NO_ARG(tpm_force_clear)
TPM_COMMAND_NO_ARG(tpm_physical_enable)
TPM_COMMAND_NO_ARG(tpm_physical_disable)

static cmd_tbl_t tpm1_commands[] = {
	U_BOOT_CMD_MKENT(info, 0, 1, do_tpm_info, "", ""),
	U_BOOT_CMD_MKENT(init, 0, 1, do_tpm_init, "", ""),
	U_BOOT_CMD_MKENT(startup, 0, 1,
			 do_tpm_startup, "", ""),
	U_BOOT_CMD_MKENT(self_test_full, 0, 1,
			 do_tpm_self_test_full, "", ""),
	U_BOOT_CMD_MKENT(continue_self_test, 0, 1,
			 do_tpm_continue_self_test, "", ""),
	U_BOOT_CMD_MKENT(force_clear, 0, 1,
			 do_tpm_force_clear, "", ""),
	U_BOOT_CMD_MKENT(physical_enable, 0, 1,
			 do_tpm_physical_enable, "", ""),
	U_BOOT_CMD_MKENT(physical_disable, 0, 1,
			 do_tpm_physical_disable, "", ""),
	U_BOOT_CMD_MKENT(nv_define_space, 0, 1,
			 do_tpm_nv_define_space, "", ""),
	U_BOOT_CMD_MKENT(nv_read_value, 0, 1,
			 do_tpm_nv_read_value, "", ""),
	U_BOOT_CMD_MKENT(nv_write_value, 0, 1,
			 do_tpm_nv_write_value, "", ""),
	U_BOOT_CMD_MKENT(extend, 0, 1,
			 do_tpm_extend, "", ""),
	U_BOOT_CMD_MKENT(pcr_read, 0, 1,
			 do_tpm_pcr_read, "", ""),
	U_BOOT_CMD_MKENT(tsc_physical_presence, 0, 1,
			 do_tpm_tsc_physical_presence, "", ""),
	U_BOOT_CMD_MKENT(read_pubek, 0, 1,
			 do_tpm_read_pubek, "", ""),
	U_BOOT_CMD_MKENT(physical_set_deactivated, 0, 1,
			 do_tpm_physical_set_deactivated, "", ""),
	U_BOOT_CMD_MKENT(get_capability, 0, 1,
			 do_tpm_get_capability, "", ""),
	U_BOOT_CMD_MKENT(raw_transfer, 0, 1,
			 do_tpm_raw_transfer, "", ""),
	U_BOOT_CMD_MKENT(nv_define, 0, 1,
			 do_tpm_nv_define, "", ""),
	U_BOOT_CMD_MKENT(nv_read, 0, 1,
			 do_tpm_nv_read, "", ""),
	U_BOOT_CMD_MKENT(nv_write, 0, 1,
			 do_tpm_nv_write, "", ""),
#ifdef CONFIG_TPM_AUTH_SESSIONS
	U_BOOT_CMD_MKENT(oiap, 0, 1,
			 do_tpm_oiap, "", ""),
	U_BOOT_CMD_MKENT(end_oiap, 0, 1,
			 do_tpm_end_oiap, "", ""),
	U_BOOT_CMD_MKENT(load_key2_oiap, 0, 1,
			 do_tpm_load_key2_oiap, "", ""),
#ifdef CONFIG_TPM_LOAD_KEY_BY_SHA1
	U_BOOT_CMD_MKENT(load_key_by_sha1, 0, 1,
			 do_tpm_load_key_by_sha1, "", ""),
#endif /* CONFIG_TPM_LOAD_KEY_BY_SHA1 */
	U_BOOT_CMD_MKENT(get_pub_key_oiap, 0, 1,
			 do_tpm_get_pub_key_oiap, "", ""),
#endif /* CONFIG_TPM_AUTH_SESSIONS */
#ifdef CONFIG_TPM_FLUSH_RESOURCES
	U_BOOT_CMD_MKENT(flush, 0, 1,
			 do_tpm_flush, "", ""),
#endif /* CONFIG_TPM_FLUSH_RESOURCES */
#ifdef CONFIG_TPM_LIST_RESOURCES
	U_BOOT_CMD_MKENT(list, 0, 1,
			 do_tpm_list, "", ""),
#endif /* CONFIG_TPM_LIST_RESOURCES */
};

cmd_tbl_t *get_tpm1_commands(unsigned int *size)
{
	*size = ARRAY_SIZE(tpm1_commands);

	return tpm1_commands;
}

U_BOOT_CMD(tpm, CONFIG_SYS_MAXARGS, 1, do_tpm,
"Issue a TPMv1.x command",
"cmd args...\n"
"    - Issue TPM command <cmd> with arguments <args...>.\n"
"Admin Startup and State Commands:\n"
"  info - Show information about the TPM\n"
"  init\n"
"    - Put TPM into a state where it waits for 'startup' command.\n"
"  startup mode\n"
"    - Issue TPM_Starup command.  <mode> is one of TPM_ST_CLEAR,\n"
"      TPM_ST_STATE, and TPM_ST_DEACTIVATED.\n"
"Admin Testing Commands:\n"
"  self_test_full\n"
"    - Test all of the TPM capabilities.\n"
"  continue_self_test\n"
"    - Inform TPM that it should complete the self-test.\n"
"Admin Opt-in Commands:\n"
"  physical_enable\n"
"    - Set the PERMANENT disable flag to FALSE using physical presence as\n"
"      authorization.\n"
"  physical_disable\n"
"    - Set the PERMANENT disable flag to TRUE using physical presence as\n"
"      authorization.\n"
"  physical_set_deactivated 0|1\n"
"    - Set deactivated flag.\n"
"Admin Ownership Commands:\n"
"  force_clear\n"
"    - Issue TPM_ForceClear command.\n"
"  tsc_physical_presence flags\n"
"    - Set TPM device's Physical Presence flags to <flags>.\n"
"The Capability Commands:\n"
"  get_capability cap_area sub_cap addr count\n"
"    - Read <count> bytes of TPM capability indexed by <cap_area> and\n"
"      <sub_cap> to memory address <addr>.\n"
#if defined(CONFIG_TPM_FLUSH_RESOURCES) || defined(CONFIG_TPM_LIST_RESOURCES)
"Resource management functions\n"
#endif
#ifdef CONFIG_TPM_FLUSH_RESOURCES
"  flush resource_type id\n"
"    - flushes a resource of type <resource_type> (may be one of key, auth,\n"
"      hash, trans, context, counter, delegate, daa_tpm, daa_v0, daa_v1),\n"
"      and id <id> from the TPM. Use an <id> of \"all\" to flush all\n"
"      resources of that type.\n"
#endif /* CONFIG_TPM_FLUSH_RESOURCES */
#ifdef CONFIG_TPM_LIST_RESOURCES
"  list resource_type\n"
"    - lists resources of type <resource_type> (may be one of key, auth,\n"
"      hash, trans, context, counter, delegate, daa_tpm, daa_v0, daa_v1),\n"
"      contained in the TPM.\n"
#endif /* CONFIG_TPM_LIST_RESOURCES */
#ifdef CONFIG_TPM_AUTH_SESSIONS
"Storage functions\n"
"  loadkey2_oiap parent_handle key_addr key_len usage_auth\n"
"    - loads a key data from memory address <key_addr>, <key_len> bytes\n"
"      into TPM using the parent key <parent_handle> with authorization\n"
"      <usage_auth> (20 bytes hex string).\n"
#ifdef CONFIG_TPM_LOAD_KEY_BY_SHA1
"  load_key_by_sha1 parent_hash key_addr key_len usage_auth\n"
"    - loads a key data from memory address <key_addr>, <key_len> bytes\n"
"      into TPM using the parent hash <parent_hash> (20 bytes hex string)\n"
"      with authorization <usage_auth> (20 bytes hex string).\n"
#endif /* CONFIG_TPM_LOAD_KEY_BY_SHA1 */
"  get_pub_key_oiap key_handle usage_auth\n"
"    - get the public key portion of a loaded key <key_handle> using\n"
"      authorization <usage auth> (20 bytes hex string)\n"
#endif /* CONFIG_TPM_AUTH_SESSIONS */
"Endorsement Key Handling Commands:\n"
"  read_pubek addr count\n"
"    - Read <count> bytes of the public endorsement key to memory\n"
"      address <addr>\n"
"Integrity Collection and Reporting Commands:\n"
"  extend index digest_hex_string\n"
"    - Add a new measurement to a PCR.  Update PCR <index> with the 20-bytes\n"
"      <digest_hex_string>\n"
"  pcr_read index addr count\n"
"    - Read <count> bytes from PCR <index> to memory address <addr>.\n"
#ifdef CONFIG_TPM_AUTH_SESSIONS
"Authorization Sessions\n"
"  oiap\n"
"    - setup an OIAP session\n"
"  end_oiap\n"
"    - terminates an active OIAP session\n"
#endif /* CONFIG_TPM_AUTH_SESSIONS */
"Non-volatile Storage Commands:\n"
"  nv_define_space index permission size\n"
"    - Establish a space at index <index> with <permission> of <size> bytes.\n"
"  nv_read_value index addr count\n"
"    - Read <count> bytes from space <index> to memory address <addr>.\n"
"  nv_write_value index addr count\n"
"    - Write <count> bytes from memory address <addr> to space <index>.\n"
"Miscellaneous helper functions:\n"
"  raw_transfer byte_string\n"
"    - Send a byte string <byte_string> to TPM and print the response.\n"
" Non-volatile storage helper functions:\n"
"    These helper functions treat a non-volatile space as a non-padded\n"
"    sequence of integer values.  These integer values are defined by a type\n"
"    string, which is a text string of 'bwd' characters: 'b' means a 8-bit\n"
"    value, 'w' 16-bit value, 'd' 32-bit value.  All helper functions take\n"
"    a type string as their first argument.\n"
"  nv_define type_string index perm\n"
"    - Define a space <index> with permission <perm>.\n"
"  nv_read types_string index vars...\n"
"    - Read from space <index> to environment variables <vars...>.\n"
"  nv_write types_string index values...\n"
"    - Write to space <index> from values <values...>.\n"
);
