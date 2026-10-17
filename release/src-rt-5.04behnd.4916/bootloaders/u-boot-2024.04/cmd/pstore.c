// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright © 2019 Collabora Ltd
 */

#include <config.h>
#include <command.h>
#include <fdtdec.h>
#include <fs.h>
#include <log.h>
#include <mapmem.h>
#include <memalign.h>
#include <part.h>
#include <fdt_support.h>

struct persistent_ram_buffer {
	u32    sig;
	u32    start;
	u32    size;
	u8     data[0];
};

#define PERSISTENT_RAM_SIG (0x43474244) /* DBGC */
#define RAMOOPS_KERNMSG_HDR "===="

#define PSTORE_TYPE_DMESG 0
#define PSTORE_TYPE_CONSOLE 2
#define PSTORE_TYPE_FTRACE 3
#define PSTORE_TYPE_PMSG 7
#define PSTORE_TYPE_ALL 255

static phys_addr_t pstore_addr = CONFIG_CMD_PSTORE_MEM_ADDR;
static phys_size_t pstore_length = CONFIG_CMD_PSTORE_MEM_SIZE;
static unsigned int pstore_record_size = CONFIG_CMD_PSTORE_RECORD_SIZE;
static unsigned int pstore_console_size = CONFIG_CMD_PSTORE_CONSOLE_SIZE;
static unsigned int pstore_ftrace_size = CONFIG_CMD_PSTORE_FTRACE_SIZE;
static unsigned int pstore_pmsg_size = CONFIG_CMD_PSTORE_PMSG_SIZE;
static unsigned int pstore_ecc_size = CONFIG_CMD_PSTORE_ECC_SIZE;
static unsigned int buffer_size;

 /**
  * pstore_read_kmsg_hdr() - Check kernel header and get compression flag if
  *                          available.
  * @buffer: Kernel messages buffer.
  * @compressed: Returns TRUE if kernel buffer is compressed, else FALSE.
  *
  * Check if buffer starts with a kernel header of the form:
  *   ====<secs>.<nsecs>[-<compression>]\n
  * If <compression> is equal to 'C' then the buffer is compressed, else iter
  * should be 'D'.
  *
  * Return: Length of kernel header.
  */
static int pstore_read_kmsg_hdr(char *buffer, bool *compressed)
{
	char *ptr = buffer;
	*compressed = false;

	if (strncmp(RAMOOPS_KERNMSG_HDR, ptr, strlen(RAMOOPS_KERNMSG_HDR)) != 0)
		return 0;

	ptr += strlen(RAMOOPS_KERNMSG_HDR);

	ptr = strchr(ptr, '\n');
	if (!ptr)
		return 0;

	if (ptr[-2] == '-' && ptr[-1] == 'C')
		*compressed = true;

	return ptr - buffer + 1;
}

/**
 * pstore_get_buffer() - Get unwrapped record buffer
 * @sig: Signature to check
 * @buffer: Buffer containing wrapped record
 * @size: wrapped record size
 * @dest: Buffer used to store unwrapped record
 *
 * The record starts with <signature><start><size> header.
 * The signature is 'DBGC' for all records except for Ftrace's record(s) wich
 * use LINUX_VERSION_CODE ^ 'DBGC'.
 * Use 0 for @sig to prevent checking signature.
 * Start and size are 4 bytes long.
 *
 * Return: record's length
 */
static u32 pstore_get_buffer(u32 sig, phys_addr_t buffer, u32 size, char *dest)
{
	struct persistent_ram_buffer *prb =
		(struct persistent_ram_buffer *)map_sysmem(buffer, size);
	u32 dest_size;

	if (sig == 0 || prb->sig == sig) {
		if (prb->size == 0) {
			log_debug("found existing empty buffer\n");
			return 0;
		}

		if (prb->size > size) {
			log_debug("found existing invalid buffer, size %u, start %u\n",
			          prb->size, prb->start);
			return 0;
		}
	} else {
		log_debug("no valid data in buffer (sig = 0x%08x)\n", prb->sig);
		return 0;
	}

	log_debug("found existing buffer, size %u, start %u\n",
	          prb->size, prb->start);

	memcpy(dest, &prb->data[prb->start], prb->size - prb->start);
	memcpy(dest + prb->size - prb->start, &prb->data[0], prb->start);

	dest_size = prb->size;
	unmap_sysmem(prb);

	return dest_size;
}

/**
 * pstore_init_buffer_size() - Init buffer size to largest record size
 *
 * Records, console, FTrace and user logs can use different buffer sizes.
 * This function allows to retrieve the biggest one.
 */
static void pstore_init_buffer_size(void)
{
	if (pstore_record_size > buffer_size)
		buffer_size = pstore_record_size;

	if (pstore_console_size > buffer_size)
		buffer_size = pstore_console_size;

	if (pstore_ftrace_size > buffer_size)
		buffer_size = pstore_ftrace_size;

	if (pstore_pmsg_size > buffer_size)
		buffer_size = pstore_pmsg_size;
}

/**
 * pstore_set() - Initialize PStore settings from command line arguments
 * @cmdtp: Command data struct pointer
 * @flag: Command flag
 * @argc: Command-line argument count
 * @argv: Array of command-line arguments
 *
 * Set pstore reserved memory info, starting at 'addr' for 'len' bytes.
 * Default length for records is 4K.
 * Mandatory arguments:
 * - addr: ramoops starting address
 * - len: ramoops total length
 * Optional arguments:
 * - record-size: size of one panic or oops record ('dump' type)
 * - console-size: size of the kernel logs record
 * - ftrace-size: size of the ftrace record(s), this can be a single record or
 *                divided in parts based on number of CPUs
 * - pmsg-size: size of the user space logs record
 * - ecc-size: enables/disables ECC support and specifies ECC buffer size in
 *             bytes (0 disables it, 1 is a special value, means 16 bytes ECC)
 *
 * Return: zero on success, CMD_RET_USAGE in case of misuse and negative
 * on error.
 */
static int pstore_set(struct cmd_tbl *cmdtp, int flag,  int argc,
		      char * const argv[])
{
	if (argc < 3)
		return CMD_RET_USAGE;

	/* Address is specified since argc > 2
	 */
	pstore_addr = hextoul(argv[1], NULL);

	/* Length is specified since argc > 2
	 */
	pstore_length = hextoul(argv[2], NULL);

	if (argc > 3)
		pstore_record_size = hextoul(argv[3], NULL);

	if (argc > 4)
		pstore_console_size = hextoul(argv[4], NULL);

	if (argc > 5)
		pstore_ftrace_size = hextoul(argv[5], NULL);

	if (argc > 6)
		pstore_pmsg_size = hextoul(argv[6], NULL);

	if (argc > 7)
		pstore_ecc_size = hextoul(argv[7], NULL);

	if (pstore_length < (pstore_record_size + pstore_console_size
			     + pstore_ftrace_size + pstore_pmsg_size)) {
		printf("pstore <len> should be larger than the sum of all records sizes\n");
		pstore_length = 0;
	}

	log_debug("pstore set done: start 0x%08llx - length 0x%llx\n",
	          (unsigned long long)pstore_addr,
	          (unsigned long long)pstore_length);

	return 0;
}

/**
 * pstore_print_buffer() - Print buffer
 * @type: buffer type
 * @buffer: buffer to print
 * @size: buffer size
 *
 * Print buffer type and content
 */
static void pstore_print_buffer(char *type, char *buffer, u32 size)
{
	u32 i = 0;

	printf("**** %s\n", type);
	while (i < size && buffer[i] != 0) {
		putc(buffer[i]);
		i++;
	}
}

/**
 * pstore_display() - Display existing records in pstore reserved memory
 * @cmdtp: Command data struct pointer
 * @flag: Command flag
 * @argc: Command-line argument count
 * @argv: Array of command-line arguments
 *
 * A 'record-type' can be given to only display records of this kind.
 * If no 'record-type' is given, all valid records are dispayed.
 * 'record-type' can be one of 'dump', 'console', 'ftrace' or 'user'. For 'dump'
 * and 'ftrace' types, a 'nb' can be given to only display one record.
 *
 * Return: zero on success, CMD_RET_USAGE in case of misuse and negative
 * on error.
 */
static int pstore_display(struct cmd_tbl *cmdtp, int flag,  int argc,
			  char * const argv[])
{
	int type = PSTORE_TYPE_ALL;
	phys_addr_t ptr;
	char *buffer;
	u32 size;
	int header_len = 0;
	bool compressed;

	if (argc > 1) {
		if (!strcmp(argv[1], "dump"))
			type = PSTORE_TYPE_DMESG;
		else if (!strcmp(argv[1], "console"))
			type = PSTORE_TYPE_CONSOLE;
		else if (!strcmp(argv[1], "ftrace"))
			type = PSTORE_TYPE_FTRACE;
		else if (!strcmp(argv[1], "user"))
			type = PSTORE_TYPE_PMSG;
		else
			return CMD_RET_USAGE;
	}

	if (pstore_length == 0) {
		printf("Please set PStore configuration\n");
		return CMD_RET_USAGE;
	}

	if (buffer_size == 0)
		pstore_init_buffer_size();

	buffer = malloc_cache_aligned(buffer_size);

	if (type == PSTORE_TYPE_DMESG || type == PSTORE_TYPE_ALL) {
		ptr = pstore_addr;
		phys_addr_t ptr_end = ptr + pstore_length - pstore_pmsg_size
				- pstore_ftrace_size - pstore_console_size;

		if (argc > 2) {
			ptr += dectoul(argv[2], NULL)
				* pstore_record_size;
			ptr_end = ptr + pstore_record_size;
		}

		while (ptr < ptr_end) {
			size = pstore_get_buffer(PERSISTENT_RAM_SIG, ptr,
						 pstore_record_size, buffer);
			ptr += pstore_record_size;

			if (size == 0)
				continue;

			header_len = pstore_read_kmsg_hdr(buffer, &compressed);
			if (header_len == 0) {
				log_debug("no valid kernel header\n");
				continue;
			}

			if (compressed) {
				printf("Compressed buffer, display not available\n");
				continue;
			}

			pstore_print_buffer("Dump", buffer + header_len,
					    size - header_len);
		}
	}

	if (type == PSTORE_TYPE_CONSOLE || type == PSTORE_TYPE_ALL) {
		ptr = pstore_addr + pstore_length - pstore_pmsg_size
			- pstore_ftrace_size - pstore_console_size;
		size = pstore_get_buffer(PERSISTENT_RAM_SIG, ptr,
					 pstore_console_size, buffer);
		if (size != 0)
			pstore_print_buffer("Console", buffer, size);
	}

	if (type == PSTORE_TYPE_FTRACE || type == PSTORE_TYPE_ALL) {
		ptr = pstore_addr + pstore_length - pstore_pmsg_size
		- pstore_ftrace_size;
		/* The FTrace record(s) uses LINUX_VERSION_CODE ^ 'DBGC'
		 * signature, pass 0 to pstore_get_buffer to prevent
		 * checking it
		 */
		size = pstore_get_buffer(0, ptr, pstore_ftrace_size, buffer);
		if (size != 0)
			pstore_print_buffer("FTrace", buffer, size);
	}

	if (type == PSTORE_TYPE_PMSG || type == PSTORE_TYPE_ALL) {
		ptr = pstore_addr + pstore_length - pstore_pmsg_size;
		size = pstore_get_buffer(PERSISTENT_RAM_SIG, ptr,
					 pstore_pmsg_size, buffer);
		if (size != 0)
			pstore_print_buffer("User", buffer, size);
	}

	free(buffer);

	return 0;
}

/**
 * pstore_save() - Save existing records from pstore reserved memory
 * @cmdtp: Command data struct pointer
 * @flag: Command flag
 * @argc: Command-line argument count
 * @argv: Array of command-line arguments
 *
 * the records are saved under 'directory path', which should already exist,
 * to partition 'part' on device type 'interface' instance 'dev'
 * Filenames are automatically generated, depending on record type, like in
 * /sys/fs/pstore under Linux
 *
 * Return: zero on success, CMD_RET_USAGE in case of misuse and negative
 * on error.
 */
static int pstore_save(struct cmd_tbl *cmdtp, int flag,  int argc,
		       char * const argv[])
{
	phys_addr_t ptr, ptr_end;
	char *buffer;
	char *save_argv[6];
	char addr[19], length[19];
	char path[256];
	u32 size;
	unsigned int index;
	int header_len = 0;
	bool compressed;

	if (argc < 4)
		return CMD_RET_USAGE;

	if (pstore_length == 0) {
		printf("Please set PStore configuration\n");
		return CMD_RET_USAGE;
	}

	if (buffer_size == 0)
		pstore_init_buffer_size();

	buffer = malloc_cache_aligned(buffer_size);
	sprintf(addr, "0x%p", buffer);

	save_argv[0] = argv[0];
	save_argv[1] = argv[1];
	save_argv[2] = argv[2];
	save_argv[3] = addr;
	save_argv[4] = path;
	save_argv[5] = length;

	/* Save all Dump records */
	ptr = pstore_addr;
	ptr_end = ptr + pstore_length - pstore_pmsg_size - pstore_ftrace_size
				- pstore_console_size;
	index = 0;
	while (ptr < ptr_end) {
		size = pstore_get_buffer(PERSISTENT_RAM_SIG, ptr,
					 pstore_record_size, buffer);
		ptr += pstore_record_size;

		if (size == 0)
			continue;

		header_len = pstore_read_kmsg_hdr(buffer, &compressed);
		if (header_len == 0) {
			log_debug("no valid kernel header\n");
			continue;
		}

		sprintf(addr, "0x%08lx", (ulong)map_to_sysmem(buffer + header_len));
		sprintf(length, "0x%X", size - header_len);
		sprintf(path, "%s/dmesg-ramoops-%u%s", argv[3], index,
			compressed ? ".enc.z" : "");
		do_save(cmdtp, flag, 6, save_argv, FS_TYPE_ANY);
		index++;
	}

	sprintf(addr, "0x%08lx", (ulong)map_to_sysmem(buffer));

	/* Save Console record */
	size = pstore_get_buffer(PERSISTENT_RAM_SIG, ptr, pstore_console_size,
				 buffer);
	if (size != 0) {
		sprintf(length, "0x%X", size);
		sprintf(path, "%s/console-ramoops-0", argv[3]);
		do_save(cmdtp, flag, 6, save_argv, FS_TYPE_ANY);
	}
	ptr += pstore_console_size;

	/* Save FTrace record(s)
	 * The FTrace record(s) uses LINUX_VERSION_CODE ^ 'DBGC' signature,
	 * pass 0 to pstore_get_buffer to prevent checking it
	 */
	size = pstore_get_buffer(0, ptr, pstore_ftrace_size, buffer);
	if (size != 0) {
		sprintf(length, "0x%X", size);
		sprintf(path, "%s/ftrace-ramoops-0", argv[3]);
		do_save(cmdtp, flag, 6, save_argv, FS_TYPE_ANY);
	}
	ptr += pstore_ftrace_size;

	/* Save Console record */
	size = pstore_get_buffer(PERSISTENT_RAM_SIG, ptr, pstore_pmsg_size,
				 buffer);
	if (size != 0) {
		sprintf(length, "0x%X", size);
		sprintf(path, "%s/pmsg-ramoops-0", argv[3]);
		do_save(cmdtp, flag, 6, save_argv, FS_TYPE_ANY);
	}

	free(buffer);

	return 0;
}

static struct cmd_tbl cmd_pstore_sub[] = {
	U_BOOT_CMD_MKENT(set, 8, 0, pstore_set, "", ""),
	U_BOOT_CMD_MKENT(display, 3, 0, pstore_display, "", ""),
	U_BOOT_CMD_MKENT(save, 4, 0, pstore_save, "", ""),
};

static int do_pstore(struct cmd_tbl *cmdtp, int flag, int argc, char * const argv[])
{
	struct cmd_tbl *c;

	if (argc < 2)
		return CMD_RET_USAGE;

	/* Strip off leading argument */
	argc--;
	argv++;

	c = find_cmd_tbl(argv[0], cmd_pstore_sub, ARRAY_SIZE(cmd_pstore_sub));

	if (!c)
		return CMD_RET_USAGE;

	return c->cmd(cmdtp, flag, argc, argv);
}

void fdt_fixup_pstore(void *blob)
{
	char node[32];
	int  nodeoffset;	/* node offset from libfdt */
	u32 addr_cells_root;
	u32 size_cells_root;
	u32 addr_cells;
	u32 size_cells;

	nodeoffset = fdt_path_offset(blob, "/");
	if (nodeoffset < 0) {
		/* Not found or something else bad happened. */
		log_err("fdt_path_offset() returned %s\n", fdt_strerror(nodeoffset));
		return;
	}
	addr_cells_root = fdt_getprop_u32_default_node(blob, nodeoffset, 0, "#address-cells", 2);
	size_cells_root = fdt_getprop_u32_default_node(blob, nodeoffset, 0, "#size-cells", 2);

	nodeoffset = fdt_find_or_add_subnode(blob, nodeoffset, "reserved-memory");
	if (nodeoffset < 0) {
		log_err("Add 'reserved-memory' node failed: %s\n",
				fdt_strerror(nodeoffset));
		return;
	}

	addr_cells = fdt_getprop_u32_default_node(blob, nodeoffset, 0,
						  "#address-cells", addr_cells_root);
	size_cells = fdt_getprop_u32_default_node(blob, nodeoffset, 0,
						  "#size-cells", size_cells_root);
	fdt_setprop_u32(blob, nodeoffset, "#address-cells", addr_cells);
	fdt_setprop_u32(blob, nodeoffset, "#size-cells", size_cells);

	fdt_setprop_empty(blob, nodeoffset, "ranges");

	sprintf(node, "ramoops@%llx", (unsigned long long)pstore_addr);
	nodeoffset = fdt_add_subnode(blob, nodeoffset, node);
	if (nodeoffset < 0) {
		log_err("Add '%s' node failed: %s\n", node, fdt_strerror(nodeoffset));
		return;
	}

	fdt_setprop_string(blob, nodeoffset, "compatible", "ramoops");

	if (addr_cells == 1) {
		fdt_setprop_u32(blob, nodeoffset, "reg", pstore_addr);
	} else if (addr_cells == 2) {
		fdt_setprop_u64(blob, nodeoffset, "reg", pstore_addr);
	} else {
		log_err("Unsupported #address-cells: %u\n", addr_cells);
		goto clean_ramoops;
	}

	if (size_cells == 1) {
		// Let's consider that the pstore_length fits in a 32 bits value
		fdt_appendprop_u32(blob, nodeoffset, "reg", pstore_length);
	} else if (size_cells == 2) {
		fdt_appendprop_u64(blob, nodeoffset, "reg", pstore_length);
	} else {
		log_err("Unsupported #size-cells: %u\n", addr_cells);
		goto clean_ramoops;
	}

	fdt_setprop_u32(blob, nodeoffset, "record-size", pstore_record_size);
	fdt_setprop_u32(blob, nodeoffset, "console-size", pstore_console_size);
	fdt_setprop_u32(blob, nodeoffset, "ftrace-size", pstore_ftrace_size);
	fdt_setprop_u32(blob, nodeoffset, "pmsg-size", pstore_pmsg_size);
	fdt_setprop_u32(blob, nodeoffset, "ecc-size", pstore_ecc_size);

clean_ramoops:
	fdt_del_node_and_alias(blob, node);
}

U_BOOT_CMD(pstore, 10, 0, do_pstore,
	   "Manage Linux Persistent Storage",
	   "set <addr> <len> [record-size] [console-size] [ftrace-size] [pmsg_size] [ecc-size]\n"
	   "- Set pstore reserved memory info, starting at 'addr' for 'len' bytes.\n"
	   "  Default length for records is 4K.\n"
	   "  'record-size' is the size of one panic or oops record ('dump' type).\n"
	   "  'console-size' is the size of the kernel logs record.\n"
	   "  'ftrace-size' is the size of the ftrace record(s), this can be a single\n"
	   "  record or divided in parts based on number of CPUs.\n"
	   "  'pmsg-size' is the size of the user space logs record.\n"
	   "  'ecc-size' enables/disables ECC support and specifies ECC buffer size in\n"
	   "  bytes (0 disables it, 1 is a special value, means 16 bytes ECC).\n"
	   "pstore display [record-type] [nb]\n"
	   "- Display existing records in pstore reserved memory. A 'record-type' can\n"
	   "  be given to only display records of this kind. 'record-type' can be one\n"
	   "  of 'dump', 'console', 'ftrace' or 'user'. For 'dump' and 'ftrace' types,\n"
	   "  a 'nb' can be given to only display one record.\n"
	   "pstore save <interface> <dev[:part]> <directory-path>\n"
	   "- Save existing records in pstore reserved memory under 'directory path'\n"
	   "  to partition 'part' on device type 'interface' instance 'dev'.\n"
	   "  Filenames are automatically generated, depending on record type, like\n"
	   "  in /sys/fs/pstore under Linux.\n"
	   "  The 'directory-path' should already exist.\n"
);
