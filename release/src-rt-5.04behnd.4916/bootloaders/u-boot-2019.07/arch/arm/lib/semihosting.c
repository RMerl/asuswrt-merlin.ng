// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2014 Broadcom Corporation
 */

/*
 * Minimal semihosting implementation for reading files into memory. If more
 * features like writing files or console output are required they can be
 * added later. This code has been tested on arm64/aarch64 fastmodel only.
 * An untested placeholder exists for armv7 architectures, but since they
 * are commonly available in silicon now, fastmodel usage makes less sense
 * for them.
 */
#include <common.h>
#include <command.h>

#define SYSOPEN		0x01
#define SYSCLOSE	0x02
#define SYSREAD		0x06
#define SYSFLEN		0x0C

#define MODE_READ	0x0
#define MODE_READBIN	0x1

/*
 * Call the handler
 */
static noinline long smh_trap(unsigned int sysnum, void *addr)
{
	register long result asm("r0");
#if defined(CONFIG_ARM64)
	asm volatile ("hlt #0xf000" : "=r" (result) : "0"(sysnum), "r"(addr));
#elif defined(CONFIG_CPU_V7M)
	asm volatile ("bkpt #0xAB" : "=r" (result) : "0"(sysnum), "r"(addr));
#else
	/* Note - untested placeholder */
	asm volatile ("svc #0x123456" : "=r" (result) : "0"(sysnum), "r"(addr));
#endif
	return result;
}

/*
 * Open a file on the host. Mode is "r" or "rb" currently. Returns a file
 * descriptor or -1 on error.
 */
static long smh_open(const char *fname, char *modestr)
{
	long fd;
	unsigned long mode;
	struct smh_open_s {
		const char *fname;
		unsigned long mode;
		size_t len;
	} open;

	debug("%s: file \'%s\', mode \'%s\'\n", __func__, fname, modestr);

	/* Check the file mode */
	if (!(strcmp(modestr, "r"))) {
		mode = MODE_READ;
	} else if (!(strcmp(modestr, "rb"))) {
		mode = MODE_READBIN;
	} else {
		printf("%s: ERROR mode \'%s\' not supported\n", __func__,
		       modestr);
		return -1;
	}

	open.fname = fname;
	open.len = strlen(fname);
	open.mode = mode;

	/* Open the file on the host */
	fd = smh_trap(SYSOPEN, &open);
	if (fd == -1)
		printf("%s: ERROR fd %ld for file \'%s\'\n", __func__, fd,
		       fname);

	return fd;
}

/*
 * Read 'len' bytes of file into 'memp'. Returns 0 on success, else failure
 */
static long smh_read(long fd, void *memp, size_t len)
{
	long ret;
	struct smh_read_s {
		long fd;
		void *memp;
		size_t len;
	} read;

	debug("%s: fd %ld, memp %p, len %zu\n", __func__, fd, memp, len);

	read.fd = fd;
	read.memp = memp;
	read.len = len;

	ret = smh_trap(SYSREAD, &read);
	if (ret < 0) {
		/*
		 * The ARM handler allows for returning partial lengths,
		 * but in practice this never happens so rather than create
		 * hard to maintain partial read loops and such, just fail
		 * with an error message.
		 */
		printf("%s: ERROR ret %ld, fd %ld, len %zu memp %p\n",
		       __func__, ret, fd, len, memp);
		return -1;
	}

	return 0;
}

/*
 * Close the file using the file descriptor
 */
static long smh_close(long fd)
{
	long ret;

	debug("%s: fd %ld\n", __func__, fd);

	ret = smh_trap(SYSCLOSE, &fd);
	if (ret == -1)
		printf("%s: ERROR fd %ld\n", __func__, fd);

	return ret;
}

/*
 * Get the file length from the file descriptor
 */
static long smh_len_fd(long fd)
{
	long ret;

	debug("%s: fd %ld\n", __func__, fd);

	ret = smh_trap(SYSFLEN, &fd);
	if (ret == -1)
		printf("%s: ERROR ret %ld, fd %ld\n", __func__, ret, fd);

	return ret;
}

static int smh_load_file(const char * const name, ulong load_addr,
			 ulong *end_addr)
{
	long fd;
	long len;
	long ret;

	fd = smh_open(name, "rb");
	if (fd == -1)
		return -1;

	len = smh_len_fd(fd);
	if (len < 0) {
		smh_close(fd);
		return -1;
	}

	ret = smh_read(fd, (void *)load_addr, len);
	smh_close(fd);

	if (ret == 0) {
		*end_addr = load_addr + len - 1;
		printf("loaded file %s from %08lX to %08lX, %08lX bytes\n",
		       name,
		       load_addr,
		       *end_addr,
		       len);
	} else {
		printf("read failed\n");
		return 0;
	}

	return 0;
}

static int do_smhload(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	if (argc == 3 || argc == 4) {
		ulong load_addr;
		ulong end_addr = 0;
		int ret;
		char end_str[64];

		load_addr = simple_strtoul(argv[2], NULL, 16);
		if (!load_addr)
			return -1;

		ret = smh_load_file(argv[1], load_addr, &end_addr);
		if (ret < 0)
			return CMD_RET_FAILURE;

		/* Optionally save returned end to the environment */
		if (argc == 4) {
			sprintf(end_str, "0x%08lx", end_addr);
			env_set(argv[3], end_str);
		}
	} else {
		return CMD_RET_USAGE;
	}
	return 0;
}

U_BOOT_CMD(smhload, 4, 0, do_smhload, "load a file using semihosting",
	   "<file> 0x<address> [end var]\n"
	   "    - load a semihosted file to the address specified\n"
	   "      if the optional [end var] is specified, the end\n"
	   "      address of the file will be stored in this environment\n"
	   "      variable.\n");
