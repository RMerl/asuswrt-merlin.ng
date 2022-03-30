
/*
 * (C) Copyright 2018, Linaro Limited
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <avb_verify.h>
#include <command.h>
#include <image.h>
#include <malloc.h>
#include <mmc.h>

#define AVB_BOOTARGS	"avb_bootargs"
static struct AvbOps *avb_ops;

static const char * const requested_partitions[] = {"boot",
					     "system",
					     "vendor",
					     NULL};

int do_avb_init(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	unsigned long mmc_dev;

	if (argc != 2)
		return CMD_RET_USAGE;

	mmc_dev = simple_strtoul(argv[1], NULL, 16);

	if (avb_ops)
		avb_ops_free(avb_ops);

	avb_ops = avb_ops_alloc(mmc_dev);
	if (avb_ops)
		return CMD_RET_SUCCESS;

	printf("Failed to initialize avb2\n");

	return CMD_RET_FAILURE;
}

int do_avb_read_part(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	const char *part;
	s64 offset;
	size_t bytes, bytes_read = 0;
	void *buffer;

	if (!avb_ops) {
		printf("AVB 2.0 is not initialized, please run 'avb init'\n");
		return CMD_RET_USAGE;
	}

	if (argc != 5)
		return CMD_RET_USAGE;

	part = argv[1];
	offset = simple_strtoul(argv[2], NULL, 16);
	bytes = simple_strtoul(argv[3], NULL, 16);
	buffer = (void *)simple_strtoul(argv[4], NULL, 16);

	if (avb_ops->read_from_partition(avb_ops, part, offset, bytes,
					 buffer, &bytes_read) ==
					 AVB_IO_RESULT_OK) {
		printf("Read %zu bytes\n", bytes_read);
		return CMD_RET_SUCCESS;
	}

	printf("Failed to read from partition\n");

	return CMD_RET_FAILURE;
}

int do_avb_read_part_hex(cmd_tbl_t *cmdtp, int flag, int argc,
			 char *const argv[])
{
	const char *part;
	s64 offset;
	size_t bytes, bytes_read = 0;
	char *buffer;

	if (!avb_ops) {
		printf("AVB 2.0 is not initialized, please run 'avb init'\n");
		return CMD_RET_USAGE;
	}

	if (argc != 4)
		return CMD_RET_USAGE;

	part = argv[1];
	offset = simple_strtoul(argv[2], NULL, 16);
	bytes = simple_strtoul(argv[3], NULL, 16);

	buffer = malloc(bytes);
	if (!buffer) {
		printf("Failed to tlb_allocate buffer for data\n");
		return CMD_RET_FAILURE;
	}
	memset(buffer, 0, bytes);

	if (avb_ops->read_from_partition(avb_ops, part, offset, bytes, buffer,
					 &bytes_read) == AVB_IO_RESULT_OK) {
		printf("Requested %zu, read %zu bytes\n", bytes, bytes_read);
		printf("Data: ");
		for (int i = 0; i < bytes_read; i++)
			printf("%02X", buffer[i]);

		printf("\n");

		free(buffer);
		return CMD_RET_SUCCESS;
	}

	printf("Failed to read from partition\n");

	free(buffer);
	return CMD_RET_FAILURE;
}

int do_avb_write_part(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	const char *part;
	s64 offset;
	size_t bytes;
	void *buffer;

	if (!avb_ops) {
		printf("AVB 2.0 is not initialized, run 'avb init' first\n");
		return CMD_RET_FAILURE;
	}

	if (argc != 5)
		return CMD_RET_USAGE;

	part = argv[1];
	offset = simple_strtoul(argv[2], NULL, 16);
	bytes = simple_strtoul(argv[3], NULL, 16);
	buffer = (void *)simple_strtoul(argv[4], NULL, 16);

	if (avb_ops->write_to_partition(avb_ops, part, offset, bytes, buffer) ==
	    AVB_IO_RESULT_OK) {
		printf("Wrote %zu bytes\n", bytes);
		return CMD_RET_SUCCESS;
	}

	printf("Failed to write in partition\n");

	return CMD_RET_FAILURE;
}

int do_avb_read_rb(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	size_t index;
	u64 rb_idx;

	if (!avb_ops) {
		printf("AVB 2.0 is not initialized, run 'avb init' first\n");
		return CMD_RET_FAILURE;
	}

	if (argc != 2)
		return CMD_RET_USAGE;

	index = (size_t)simple_strtoul(argv[1], NULL, 16);

	if (avb_ops->read_rollback_index(avb_ops, index, &rb_idx) ==
	    AVB_IO_RESULT_OK) {
		printf("Rollback index: %llx\n", rb_idx);
		return CMD_RET_SUCCESS;
	}

	printf("Failed to read rollback index\n");

	return CMD_RET_FAILURE;
}

int do_avb_write_rb(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	size_t index;
	u64 rb_idx;

	if (!avb_ops) {
		printf("AVB 2.0 is not initialized, run 'avb init' first\n");
		return CMD_RET_FAILURE;
	}

	if (argc != 3)
		return CMD_RET_USAGE;

	index = (size_t)simple_strtoul(argv[1], NULL, 16);
	rb_idx = simple_strtoul(argv[2], NULL, 16);

	if (avb_ops->write_rollback_index(avb_ops, index, rb_idx) ==
	    AVB_IO_RESULT_OK)
		return CMD_RET_SUCCESS;

	printf("Failed to write rollback index\n");

	return CMD_RET_FAILURE;
}

int do_avb_get_uuid(cmd_tbl_t *cmdtp, int flag,
		    int argc, char * const argv[])
{
	const char *part;
	char buffer[UUID_STR_LEN + 1];

	if (!avb_ops) {
		printf("AVB 2.0 is not initialized, run 'avb init' first\n");
		return CMD_RET_FAILURE;
	}

	if (argc != 2)
		return CMD_RET_USAGE;

	part = argv[1];

	if (avb_ops->get_unique_guid_for_partition(avb_ops, part, buffer,
						   UUID_STR_LEN + 1) ==
						   AVB_IO_RESULT_OK) {
		printf("'%s' UUID: %s\n", part, buffer);
		return CMD_RET_SUCCESS;
	}

	printf("Failed to read UUID\n");

	return CMD_RET_FAILURE;
}

int do_avb_verify_part(cmd_tbl_t *cmdtp, int flag,
		       int argc, char *const argv[])
{
	AvbSlotVerifyResult slot_result;
	AvbSlotVerifyData *out_data;
	char *cmdline;
	char *extra_args;

	bool unlocked = false;
	int res = CMD_RET_FAILURE;

	if (!avb_ops) {
		printf("AVB 2.0 is not initialized, run 'avb init' first\n");
		return CMD_RET_FAILURE;
	}

	if (argc != 1)
		return CMD_RET_USAGE;

	printf("## Android Verified Boot 2.0 version %s\n",
	       avb_version_string());

	if (avb_ops->read_is_device_unlocked(avb_ops, &unlocked) !=
	    AVB_IO_RESULT_OK) {
		printf("Can't determine device lock state.\n");
		return CMD_RET_FAILURE;
	}

	slot_result =
		avb_slot_verify(avb_ops,
				requested_partitions,
				"",
				unlocked,
				AVB_HASHTREE_ERROR_MODE_RESTART_AND_INVALIDATE,
				&out_data);

	switch (slot_result) {
	case AVB_SLOT_VERIFY_RESULT_OK:
		/* Until we don't have support of changing unlock states, we
		 * assume that we are by default in locked state.
		 * So in this case we can boot only when verification is
		 * successful; we also supply in cmdline GREEN boot state
		 */
		printf("Verification passed successfully\n");

		/* export additional bootargs to AVB_BOOTARGS env var */

		extra_args = avb_set_state(avb_ops, AVB_GREEN);
		if (extra_args)
			cmdline = append_cmd_line(out_data->cmdline,
						  extra_args);
		else
			cmdline = out_data->cmdline;

		env_set(AVB_BOOTARGS, cmdline);

		res = CMD_RET_SUCCESS;
		break;
	case AVB_SLOT_VERIFY_RESULT_ERROR_VERIFICATION:
		printf("Verification failed\n");
		break;
	case AVB_SLOT_VERIFY_RESULT_ERROR_IO:
		printf("I/O error occurred during verification\n");
		break;
	case AVB_SLOT_VERIFY_RESULT_ERROR_OOM:
		printf("OOM error occurred during verification\n");
		break;
	case AVB_SLOT_VERIFY_RESULT_ERROR_INVALID_METADATA:
		printf("Corrupted dm-verity metadata detected\n");
		break;
	case AVB_SLOT_VERIFY_RESULT_ERROR_UNSUPPORTED_VERSION:
		printf("Unsupported version avbtool was used\n");
		break;
	case AVB_SLOT_VERIFY_RESULT_ERROR_ROLLBACK_INDEX:
		printf("Checking rollback index failed\n");
		break;
	case AVB_SLOT_VERIFY_RESULT_ERROR_PUBLIC_KEY_REJECTED:
		printf("Public key was rejected\n");
		break;
	default:
		printf("Unknown error occurred\n");
	}

	return res;
}

int do_avb_is_unlocked(cmd_tbl_t *cmdtp, int flag,
		       int argc, char * const argv[])
{
	bool unlock;

	if (!avb_ops) {
		printf("AVB not initialized, run 'avb init' first\n");
		return CMD_RET_FAILURE;
	}

	if (argc != 1) {
		printf("--%s(-1)\n", __func__);
		return CMD_RET_USAGE;
	}

	if (avb_ops->read_is_device_unlocked(avb_ops, &unlock) ==
	    AVB_IO_RESULT_OK) {
		printf("Unlocked = %d\n", unlock);
		return CMD_RET_SUCCESS;
	}

	printf("Can't determine device lock state.\n");

	return CMD_RET_FAILURE;
}

int do_avb_read_pvalue(cmd_tbl_t *cmdtp, int flag, int argc,
		       char * const argv[])
{
	const char *name;
	size_t bytes;
	size_t bytes_read;
	void *buffer;
	char *endp;

	if (!avb_ops) {
		printf("AVB 2.0 is not initialized, run 'avb init' first\n");
		return CMD_RET_FAILURE;
	}

	if (argc != 3)
		return CMD_RET_USAGE;

	name = argv[1];
	bytes = simple_strtoul(argv[2], &endp, 10);
	if (*endp && *endp != '\n')
		return CMD_RET_USAGE;

	buffer = malloc(bytes);
	if (!buffer)
		return CMD_RET_FAILURE;

	if (avb_ops->read_persistent_value(avb_ops, name, bytes, buffer,
					   &bytes_read) == AVB_IO_RESULT_OK) {
		printf("Read %ld bytes, value = %s\n", bytes_read,
		       (char *)buffer);
		free(buffer);
		return CMD_RET_SUCCESS;
	}

	printf("Failed to read persistent value\n");

	free(buffer);

	return CMD_RET_FAILURE;
}

int do_avb_write_pvalue(cmd_tbl_t *cmdtp, int flag, int argc,
			char * const argv[])
{
	const char *name;
	const char *value;

	if (!avb_ops) {
		printf("AVB 2.0 is not initialized, run 'avb init' first\n");
		return CMD_RET_FAILURE;
	}

	if (argc != 3)
		return CMD_RET_USAGE;

	name = argv[1];
	value = argv[2];

	if (avb_ops->write_persistent_value(avb_ops, name, strlen(value) + 1,
					    (const uint8_t *)value) ==
	    AVB_IO_RESULT_OK) {
		printf("Wrote %ld bytes\n", strlen(value) + 1);
		return CMD_RET_SUCCESS;
	}

	printf("Failed to write persistent value\n");

	return CMD_RET_FAILURE;
}

static cmd_tbl_t cmd_avb[] = {
	U_BOOT_CMD_MKENT(init, 2, 0, do_avb_init, "", ""),
	U_BOOT_CMD_MKENT(read_rb, 2, 0, do_avb_read_rb, "", ""),
	U_BOOT_CMD_MKENT(write_rb, 3, 0, do_avb_write_rb, "", ""),
	U_BOOT_CMD_MKENT(is_unlocked, 1, 0, do_avb_is_unlocked, "", ""),
	U_BOOT_CMD_MKENT(get_uuid, 2, 0, do_avb_get_uuid, "", ""),
	U_BOOT_CMD_MKENT(read_part, 5, 0, do_avb_read_part, "", ""),
	U_BOOT_CMD_MKENT(read_part_hex, 4, 0, do_avb_read_part_hex, "", ""),
	U_BOOT_CMD_MKENT(write_part, 5, 0, do_avb_write_part, "", ""),
	U_BOOT_CMD_MKENT(verify, 1, 0, do_avb_verify_part, "", ""),
#ifdef CONFIG_OPTEE_TA_AVB
	U_BOOT_CMD_MKENT(read_pvalue, 3, 0, do_avb_read_pvalue, "", ""),
	U_BOOT_CMD_MKENT(write_pvalue, 3, 0, do_avb_write_pvalue, "", ""),
#endif
};

static int do_avb(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	cmd_tbl_t *cp;

	cp = find_cmd_tbl(argv[1], cmd_avb, ARRAY_SIZE(cmd_avb));

	argc--;
	argv++;

	if (!cp || argc > cp->maxargs)
		return CMD_RET_USAGE;

	if (flag == CMD_FLAG_REPEAT)
		return CMD_RET_FAILURE;

	return cp->cmd(cmdtp, flag, argc, argv);
}

U_BOOT_CMD(
	avb, 29, 0, do_avb,
	"Provides commands for testing Android Verified Boot 2.0 functionality",
	"init <dev> - initialize avb2 for <dev>\n"
	"avb read_rb <num> - read rollback index at location <num>\n"
	"avb write_rb <num> <rb> - write rollback index <rb> to <num>\n"
	"avb is_unlocked - returns unlock status of the device\n"
	"avb get_uuid <partname> - read and print uuid of partition <part>\n"
	"avb read_part <partname> <offset> <num> <addr> - read <num> bytes from\n"
	"    partition <partname> to buffer <addr>\n"
	"avb read_part_hex <partname> <offset> <num> - read <num> bytes from\n"
	"    partition <partname> and print to stdout\n"
	"avb write_part <partname> <offset> <num> <addr> - write <num> bytes to\n"
	"    <partname> by <offset> using data from <addr>\n"
#ifdef CONFIG_OPTEE_TA_AVB
	"avb read_pvalue <name> <bytes> - read a persistent value <name>\n"
	"avb write_pvalue <name> <value> - write a persistent value <name>\n"
#endif
	"avb verify - run verification process using hash data\n"
	"    from vbmeta structure\n"
	);
