// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2018 Linaro Ltd.
 * Sam Protsenko <semen.protsenko@linaro.org>
 * Eugeniu Rosca <rosca.eugeniu@gmail.com>
 */

#include <command.h>
#include <env.h>
#include <image-android-dt.h>
#include <common.h>

#define OPT_INDEX	"--index"

/*
 * Current/working DTB/DTBO Android image address.
 * Similar to 'working_fdt' variable in 'fdt' command.
 */
static ulong working_img;

static int do_adtimg_addr(struct cmd_tbl *cmdtp, int flag, int argc,
			  char *const argv[])
{
	char *endp;
	ulong hdr_addr;

	if (argc != 2)
		return CMD_RET_USAGE;

	hdr_addr = hextoul(argv[1], &endp);
	if (*endp != '\0') {
		printf("Error: Wrong image address '%s'\n", argv[1]);
		return CMD_RET_FAILURE;
	}

	/*
	 * Allow users to set an address prior to copying the DTB/DTBO
	 * image to that same address, i.e. skip header verification.
	 */

	working_img = hdr_addr;
	return CMD_RET_SUCCESS;
}

static int adtimg_check_working_img(void)
{
	if (!working_img) {
		printf("Error: Please, call 'adtimg addr <addr>'. Aborting!\n");
		return CMD_RET_FAILURE;
	}

	if (!android_dt_check_header(working_img)) {
		printf("Error: Invalid image header at 0x%lx\n", working_img);
		return CMD_RET_FAILURE;
	}

	return CMD_RET_SUCCESS;
}

static int do_adtimg_dump(struct cmd_tbl *cmdtp, int flag, int argc,
			  char *const argv[])
{
	if (argc != 1)
		return CMD_RET_USAGE;

	if (adtimg_check_working_img() != CMD_RET_SUCCESS)
		return CMD_RET_FAILURE;

	android_dt_print_contents(working_img);

	return CMD_RET_SUCCESS;
}

static int adtimg_getopt_u32(char * const opt, char * const name, u32 *optval)
{
	char *endp, *str;
	u32 val;

	if (!opt || !name || !optval)
		return CMD_RET_FAILURE;

	str = strchr(opt, '=');
	if (!str) {
		printf("Error: Option '%s' not followed by '='\n", name);
		return CMD_RET_FAILURE;
	}

	if (*++str == '\0') {
		printf("Error: Option '%s=' not followed by value\n", name);
		return CMD_RET_FAILURE;
	}

	val = simple_strtoul(str, &endp, 0);
	if (*endp != '\0') {
		printf("Error: Wrong integer value '%s=%s'\n", name, str);
		return CMD_RET_FAILURE;
	}

	*optval = val;
	return CMD_RET_SUCCESS;
}

static int adtimg_getopt_index(int argc, char *const argv[], u32 *index,
			       char **avar, char **svar)
{
	int ret;

	if (!argv || !avar || !svar)
		return CMD_RET_FAILURE;

	if (argc > 3) {
		printf("Error: Unexpected argument '%s'\n", argv[3]);
		return CMD_RET_FAILURE;
	}

	ret = adtimg_getopt_u32(argv[0], OPT_INDEX, index);
	if (ret != CMD_RET_SUCCESS)
		return ret;

	if (argc > 1)
		*avar = argv[1];
	if (argc > 2)
		*svar = argv[2];

	return CMD_RET_SUCCESS;
}

static int adtimg_get_dt_by_index(int argc, char *const argv[])
{
	ulong addr;
	u32 index, size;
	int ret;
	char *avar = NULL, *svar = NULL;

	ret = adtimg_getopt_index(argc, argv, &index, &avar, &svar);
	if (ret != CMD_RET_SUCCESS)
		return ret;

	if (!android_dt_get_fdt_by_index(working_img, index, &addr, &size))
		return CMD_RET_FAILURE;

	if (avar && svar) {
		ret = env_set_hex(avar, addr);
		if (ret) {
			printf("Error: Can't set '%s' to 0x%lx\n", avar, addr);
			return CMD_RET_FAILURE;
		}
		ret = env_set_hex(svar, size);
		if (ret) {
			printf("Error: Can't set '%s' to 0x%x\n", svar, size);
			return CMD_RET_FAILURE;
		}
	} else if (avar) {
		ret = env_set_hex(avar, addr);
		if (ret) {
			printf("Error: Can't set '%s' to 0x%lx\n", avar, addr);
			return CMD_RET_FAILURE;
		}
		printf("0x%x (%d)\n", size, size);
	} else {
		printf("0x%lx, 0x%x (%d)\n", addr, size, size);
	}

	return CMD_RET_SUCCESS;
}

static int adtimg_get_dt(int argc, char *const argv[])
{
	if (argc < 2) {
		printf("Error: No options passed to '%s'\n", argv[0]);
		return CMD_RET_FAILURE;
	}

	/* Strip off leading 'dt' command argument */
	argc--;
	argv++;

	if (!strncmp(argv[0], OPT_INDEX, sizeof(OPT_INDEX) - 1))
		return adtimg_get_dt_by_index(argc, argv);

	printf("Error: Option '%s' not supported\n", argv[0]);
	return CMD_RET_FAILURE;
}

static int do_adtimg_get(struct cmd_tbl *cmdtp, int flag, int argc,
			 char *const argv[])
{
	if (argc < 2) {
		printf("Error: No arguments passed to '%s'\n", argv[0]);
		return CMD_RET_FAILURE;
	}

	if (adtimg_check_working_img() != CMD_RET_SUCCESS)
		return CMD_RET_FAILURE;

	/* Strip off leading 'get' command argument */
	argc--;
	argv++;

	if (!strcmp(argv[0], "dt"))
		return adtimg_get_dt(argc, argv);

	printf("Error: Wrong argument '%s'\n", argv[0]);
	return CMD_RET_FAILURE;
}

static struct cmd_tbl cmd_adtimg_sub[] = {
	U_BOOT_CMD_MKENT(addr, CONFIG_SYS_MAXARGS, 1, do_adtimg_addr, "", ""),
	U_BOOT_CMD_MKENT(dump, CONFIG_SYS_MAXARGS, 1, do_adtimg_dump, "", ""),
	U_BOOT_CMD_MKENT(get, CONFIG_SYS_MAXARGS, 1, do_adtimg_get, "", ""),
};

static int do_adtimg(struct cmd_tbl *cmdtp, int flag, int argc,
		     char *const argv[])
{
	struct cmd_tbl *cp;

	cp = find_cmd_tbl(argv[1], cmd_adtimg_sub, ARRAY_SIZE(cmd_adtimg_sub));

	/* Strip off leading 'adtimg' command argument */
	argc--;
	argv++;

	if (!cp || argc > cp->maxargs)
		return CMD_RET_USAGE;
	if (flag == CMD_FLAG_REPEAT && !cmd_is_repeatable(cp))
		return CMD_RET_SUCCESS;

	return cp->cmd(cmdtp, flag, argc, argv);
}

U_BOOT_CMD(
	adtimg, CONFIG_SYS_MAXARGS, 0, do_adtimg,
	"manipulate dtb/dtbo Android image",
	"addr <addr> - Set image location to <addr>\n"
	"adtimg dump        - Print out image contents\n"
	"adtimg get dt --index=<index> [avar [svar]]         - Get DT address/size by index\n"
	"\n"
	"Legend:\n"
	"  - <addr>: DTB/DTBO image address (hex) in RAM\n"
	"  - <index>: index (hex/dec) of desired DT in the image\n"
	"  - <avar>: variable name to contain DT address (hex)\n"
	"  - <svar>: variable name to contain DT size (hex)"
);
