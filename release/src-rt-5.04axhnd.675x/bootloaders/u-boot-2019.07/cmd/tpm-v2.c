// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2018 Bootlin
 * Author: Miquel Raynal <miquel.raynal@bootlin.com>
 */

#include <common.h>
#include <dm.h>
#include <log.h>
#include <mapmem.h>
#include <tpm-common.h>
#include <tpm-v2.h>
#include "tpm-user-utils.h"

static int do_tpm2_startup(cmd_tbl_t *cmdtp, int flag, int argc,
			   char * const argv[])
{
	enum tpm2_startup_types mode;
	struct udevice *dev;
	int ret;

	ret = get_tpm(&dev);
	if (ret)
		return ret;
	if (argc != 2)
		return CMD_RET_USAGE;

	if (!strcasecmp("TPM2_SU_CLEAR", argv[1])) {
		mode = TPM2_SU_CLEAR;
	} else if (!strcasecmp("TPM2_SU_STATE", argv[1])) {
		mode = TPM2_SU_STATE;
	} else {
		printf("Couldn't recognize mode string: %s\n", argv[1]);
		return CMD_RET_FAILURE;
	}

	return report_return_code(tpm2_startup(dev, mode));
}

static int do_tpm2_self_test(cmd_tbl_t *cmdtp, int flag, int argc,
			     char * const argv[])
{
	enum tpm2_yes_no full_test;
	struct udevice *dev;
	int ret;

	ret = get_tpm(&dev);
	if (ret)
		return ret;
	if (argc != 2)
		return CMD_RET_USAGE;

	if (!strcasecmp("full", argv[1])) {
		full_test = TPMI_YES;
	} else if (!strcasecmp("continue", argv[1])) {
		full_test = TPMI_NO;
	} else {
		printf("Couldn't recognize test mode: %s\n", argv[1]);
		return CMD_RET_FAILURE;
	}

	return report_return_code(tpm2_self_test(dev, full_test));
}

static int do_tpm2_clear(cmd_tbl_t *cmdtp, int flag, int argc,
			 char * const argv[])
{
	u32 handle = 0;
	const char *pw = (argc < 3) ? NULL : argv[2];
	const ssize_t pw_sz = pw ? strlen(pw) : 0;
	struct udevice *dev;
	int ret;

	ret = get_tpm(&dev);
	if (ret)
		return ret;

	if (argc < 2 || argc > 3)
		return CMD_RET_USAGE;

	if (pw_sz > TPM2_DIGEST_LEN)
		return -EINVAL;

	if (!strcasecmp("TPM2_RH_LOCKOUT", argv[1]))
		handle = TPM2_RH_LOCKOUT;
	else if (!strcasecmp("TPM2_RH_PLATFORM", argv[1]))
		handle = TPM2_RH_PLATFORM;
	else
		return CMD_RET_USAGE;

	return report_return_code(tpm2_clear(dev, handle, pw, pw_sz));
}

static int do_tpm2_pcr_extend(cmd_tbl_t *cmdtp, int flag, int argc,
			      char * const argv[])
{
	struct udevice *dev;
	struct tpm_chip_priv *priv;
	u32 index = simple_strtoul(argv[1], NULL, 0);
	void *digest = map_sysmem(simple_strtoul(argv[2], NULL, 0), 0);
	int ret;
	u32 rc;

	if (argc != 3)
		return CMD_RET_USAGE;

	ret = get_tpm(&dev);
	if (ret)
		return ret;

	priv = dev_get_uclass_priv(dev);
	if (!priv)
		return -EINVAL;

	if (index >= priv->pcr_count)
		return -EINVAL;

	rc = tpm2_pcr_extend(dev, index, digest);

	unmap_sysmem(digest);

	return report_return_code(rc);
}

static int do_tpm_pcr_read(cmd_tbl_t *cmdtp, int flag, int argc,
			   char * const argv[])
{
	struct udevice *dev;
	struct tpm_chip_priv *priv;
	u32 index, rc;
	unsigned int updates;
	void *data;
	int ret;

	if (argc != 3)
		return CMD_RET_USAGE;

	ret = get_tpm(&dev);
	if (ret)
		return ret;

	priv = dev_get_uclass_priv(dev);
	if (!priv)
		return -EINVAL;

	index = simple_strtoul(argv[1], NULL, 0);
	if (index >= priv->pcr_count)
		return -EINVAL;

	data = map_sysmem(simple_strtoul(argv[2], NULL, 0), 0);

	rc = tpm2_pcr_read(dev, index, priv->pcr_select_min, data, &updates);
	if (!rc) {
		printf("PCR #%u content (%u known updates):\n", index, updates);
		print_byte_string(data, TPM2_DIGEST_LEN);
	}

	unmap_sysmem(data);

	return report_return_code(rc);
}

static int do_tpm_get_capability(cmd_tbl_t *cmdtp, int flag, int argc,
				 char * const argv[])
{
	u32 capability, property, rc;
	u8 *data;
	size_t count;
	int i, j;
	struct udevice *dev;
	int ret;

	ret = get_tpm(&dev);
	if (ret)
		return ret;

	if (argc != 5)
		return CMD_RET_USAGE;

	capability = simple_strtoul(argv[1], NULL, 0);
	property = simple_strtoul(argv[2], NULL, 0);
	data = map_sysmem(simple_strtoul(argv[3], NULL, 0), 0);
	count = simple_strtoul(argv[4], NULL, 0);

	rc = tpm2_get_capability(dev, capability, property, data, count);
	if (rc)
		goto unmap_data;

	printf("Capabilities read from TPM:\n");
	for (i = 0; i < count; i++) {
		printf("Property 0x");
		for (j = 0; j < 4; j++)
			printf("%02x", data[(i * 8) + j]);
		printf(": 0x");
		for (j = 4; j < 8; j++)
			printf("%02x", data[(i * 8) + j]);
		printf("\n");
	}

unmap_data:
	unmap_sysmem(data);

	return report_return_code(rc);
}

static int do_tpm_dam_reset(cmd_tbl_t *cmdtp, int flag, int argc,
			    char *const argv[])
{
	const char *pw = (argc < 2) ? NULL : argv[1];
	const ssize_t pw_sz = pw ? strlen(pw) : 0;
	struct udevice *dev;
	int ret;

	ret = get_tpm(&dev);
	if (ret)
		return ret;

	if (argc > 2)
		return CMD_RET_USAGE;

	if (pw_sz > TPM2_DIGEST_LEN)
		return -EINVAL;

	return report_return_code(tpm2_dam_reset(dev, pw, pw_sz));
}

static int do_tpm_dam_parameters(cmd_tbl_t *cmdtp, int flag, int argc,
				 char *const argv[])
{
	const char *pw = (argc < 5) ? NULL : argv[4];
	const ssize_t pw_sz = pw ? strlen(pw) : 0;
	/*
	 * No Dictionary Attack Mitigation (DAM) means:
	 * maxtries = 0xFFFFFFFF, recovery_time = 1, lockout_recovery = 0
	 */
	unsigned long int max_tries;
	unsigned long int recovery_time;
	unsigned long int lockout_recovery;
	struct udevice *dev;
	int ret;

	ret = get_tpm(&dev);
	if (ret)
		return ret;

	if (argc < 4 || argc > 5)
		return CMD_RET_USAGE;

	if (pw_sz > TPM2_DIGEST_LEN)
		return -EINVAL;

	if (strict_strtoul(argv[1], 0, &max_tries))
		return CMD_RET_USAGE;

	if (strict_strtoul(argv[2], 0, &recovery_time))
		return CMD_RET_USAGE;

	if (strict_strtoul(argv[3], 0, &lockout_recovery))
		return CMD_RET_USAGE;

	log(LOGC_NONE, LOGL_INFO, "Changing dictionary attack parameters:\n");
	log(LOGC_NONE, LOGL_INFO, "- maxTries: %lu", max_tries);
	log(LOGC_NONE, LOGL_INFO, "- recoveryTime: %lu\n", recovery_time);
	log(LOGC_NONE, LOGL_INFO, "- lockoutRecovery: %lu\n", lockout_recovery);

	return report_return_code(tpm2_dam_parameters(dev, pw, pw_sz, max_tries,
						      recovery_time,
						      lockout_recovery));
}

static int do_tpm_change_auth(cmd_tbl_t *cmdtp, int flag, int argc,
			      char *const argv[])
{
	u32 handle;
	const char *newpw = argv[2];
	const char *oldpw = (argc == 3) ? NULL : argv[3];
	const ssize_t newpw_sz = strlen(newpw);
	const ssize_t oldpw_sz = oldpw ? strlen(oldpw) : 0;
	struct udevice *dev;
	int ret;

	ret = get_tpm(&dev);
	if (ret)
		return ret;

	if (argc < 3 || argc > 4)
		return CMD_RET_USAGE;

	if (newpw_sz > TPM2_DIGEST_LEN || oldpw_sz > TPM2_DIGEST_LEN)
		return -EINVAL;

	if (!strcasecmp("TPM2_RH_LOCKOUT", argv[1]))
		handle = TPM2_RH_LOCKOUT;
	else if (!strcasecmp("TPM2_RH_ENDORSEMENT", argv[1]))
		handle = TPM2_RH_ENDORSEMENT;
	else if (!strcasecmp("TPM2_RH_OWNER", argv[1]))
		handle = TPM2_RH_OWNER;
	else if (!strcasecmp("TPM2_RH_PLATFORM", argv[1]))
		handle = TPM2_RH_PLATFORM;
	else
		return CMD_RET_USAGE;

	return report_return_code(tpm2_change_auth(dev, handle, newpw, newpw_sz,
						   oldpw, oldpw_sz));
}

static int do_tpm_pcr_setauthpolicy(cmd_tbl_t *cmdtp, int flag, int argc,
				    char * const argv[])
{
	u32 index = simple_strtoul(argv[1], NULL, 0);
	char *key = argv[2];
	const char *pw = (argc < 4) ? NULL : argv[3];
	const ssize_t pw_sz = pw ? strlen(pw) : 0;
	struct udevice *dev;
	int ret;

	ret = get_tpm(&dev);
	if (ret)
		return ret;

	if (strlen(key) != TPM2_DIGEST_LEN)
		return -EINVAL;

	if (argc < 3 || argc > 4)
		return CMD_RET_USAGE;

	return report_return_code(tpm2_pcr_setauthpolicy(dev, pw, pw_sz, index,
							 key));
}

static int do_tpm_pcr_setauthvalue(cmd_tbl_t *cmdtp, int flag,
				   int argc, char * const argv[])
{
	u32 index = simple_strtoul(argv[1], NULL, 0);
	char *key = argv[2];
	const ssize_t key_sz = strlen(key);
	const char *pw = (argc < 4) ? NULL : argv[3];
	const ssize_t pw_sz = pw ? strlen(pw) : 0;
	struct udevice *dev;
	int ret;

	ret = get_tpm(&dev);
	if (ret)
		return ret;

	if (strlen(key) != TPM2_DIGEST_LEN)
		return -EINVAL;

	if (argc < 3 || argc > 4)
		return CMD_RET_USAGE;

	return report_return_code(tpm2_pcr_setauthvalue(dev, pw, pw_sz, index,
							key, key_sz));
}

static cmd_tbl_t tpm2_commands[] = {
	U_BOOT_CMD_MKENT(info, 0, 1, do_tpm_info, "", ""),
	U_BOOT_CMD_MKENT(init, 0, 1, do_tpm_init, "", ""),
	U_BOOT_CMD_MKENT(startup, 0, 1, do_tpm2_startup, "", ""),
	U_BOOT_CMD_MKENT(self_test, 0, 1, do_tpm2_self_test, "", ""),
	U_BOOT_CMD_MKENT(clear, 0, 1, do_tpm2_clear, "", ""),
	U_BOOT_CMD_MKENT(pcr_extend, 0, 1, do_tpm2_pcr_extend, "", ""),
	U_BOOT_CMD_MKENT(pcr_read, 0, 1, do_tpm_pcr_read, "", ""),
	U_BOOT_CMD_MKENT(get_capability, 0, 1, do_tpm_get_capability, "", ""),
	U_BOOT_CMD_MKENT(dam_reset, 0, 1, do_tpm_dam_reset, "", ""),
	U_BOOT_CMD_MKENT(dam_parameters, 0, 1, do_tpm_dam_parameters, "", ""),
	U_BOOT_CMD_MKENT(change_auth, 0, 1, do_tpm_change_auth, "", ""),
	U_BOOT_CMD_MKENT(pcr_setauthpolicy, 0, 1,
			 do_tpm_pcr_setauthpolicy, "", ""),
	U_BOOT_CMD_MKENT(pcr_setauthvalue, 0, 1,
			 do_tpm_pcr_setauthvalue, "", ""),
};

cmd_tbl_t *get_tpm2_commands(unsigned int *size)
{
	*size = ARRAY_SIZE(tpm2_commands);

	return tpm2_commands;
}

U_BOOT_CMD(tpm2, CONFIG_SYS_MAXARGS, 1, do_tpm, "Issue a TPMv2.x command",
"<command> [<arguments>]\n"
"\n"
"info\n"
"    Show information about the TPM.\n"
"init\n"
"    Initialize the software stack. Always the first command to issue.\n"
"startup <mode>\n"
"    Issue a TPM2_Startup command.\n"
"    <mode> is one of:\n"
"        * TPM2_SU_CLEAR (reset state)\n"
"        * TPM2_SU_STATE (preserved state)\n"
"self_test <type>\n"
"    Test the TPM capabilities.\n"
"    <type> is one of:\n"
"        * full (perform all tests)\n"
"        * continue (only check untested tests)\n"
"clear <hierarchy>\n"
"    Issue a TPM2_Clear command.\n"
"    <hierarchy> is one of:\n"
"        * TPM2_RH_LOCKOUT\n"
"        * TPM2_RH_PLATFORM\n"
"pcr_extend <pcr> <digest_addr>\n"
"    Extend PCR #<pcr> with digest at <digest_addr>.\n"
"    <pcr>: index of the PCR\n"
"    <digest_addr>: address of a 32-byte SHA256 digest\n"
"pcr_read <pcr> <digest_addr>\n"
"    Read PCR #<pcr> to memory address <digest_addr>.\n"
"    <pcr>: index of the PCR\n"
"    <digest_addr>: address to store the a 32-byte SHA256 digest\n"
"get_capability <capability> <property> <addr> <count>\n"
"    Read and display <count> entries indexed by <capability>/<property>.\n"
"    Values are 4 bytes long and are written at <addr>.\n"
"    <capability>: capability\n"
"    <property>: property\n"
"    <addr>: address to store <count> entries of 4 bytes\n"
"    <count>: number of entries to retrieve\n"
"dam_reset [<password>]\n"
"    If the TPM is not in a LOCKOUT state, reset the internal error counter.\n"
"    <password>: optional password\n"
"dam_parameters <max_tries> <recovery_time> <lockout_recovery> [<password>]\n"
"    If the TPM is not in a LOCKOUT state, set the DAM parameters\n"
"    <maxTries>: maximum number of failures before lockout,\n"
"                0 means always locking\n"
"    <recoveryTime>: time before decrement of the error counter,\n"
"                    0 means no lockout\n"
"    <lockoutRecovery>: time of a lockout (before the next try),\n"
"                       0 means a reboot is needed\n"
"    <password>: optional password of the LOCKOUT hierarchy\n"
"change_auth <hierarchy> <new_pw> [<old_pw>]\n"
"    <hierarchy>: the hierarchy\n"
"    <new_pw>: new password for <hierarchy>\n"
"    <old_pw>: optional previous password of <hierarchy>\n"
"pcr_setauthpolicy|pcr_setauthvalue <pcr> <key> [<password>]\n"
"    Change the <key> to access PCR #<pcr>.\n"
"    hierarchy and may be empty.\n"
"    /!\\WARNING: untested function, use at your own risks !\n"
"    <pcr>: index of the PCR\n"
"    <key>: secret to protect the access of PCR #<pcr>\n"
"    <password>: optional password of the PLATFORM hierarchy\n"
);
