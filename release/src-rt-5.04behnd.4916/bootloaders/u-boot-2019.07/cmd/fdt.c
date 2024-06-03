// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2007
 * Gerald Van Baren, Custom IDEAS, vanbaren@cideas.com
 * Based on code written by:
 *   Pantelis Antoniou <pantelis.antoniou@gmail.com> and
 *   Matthew McClintock <msm@freescale.com>
 */

#include <common.h>
#include <command.h>
#include <linux/ctype.h>
#include <linux/types.h>
#include <asm/global_data.h>
#include <linux/libfdt.h>
#include <fdt_support.h>
#include <mapmem.h>
#include <asm/io.h>

#define MAX_LEVEL	32		/* how deeply nested we will go */
#define SCRATCHPAD	1024		/* bytes of scratchpad memory */
#define CMD_FDT_MAX_DUMP 64

/*
 * Global data (for the gd->bd)
 */
DECLARE_GLOBAL_DATA_PTR;

static int fdt_valid(struct fdt_header **blobp);
static int fdt_parse_prop(char *const*newval, int count, char *data, int *len);
static int fdt_print(const char *pathp, char *prop, int depth);
static int is_printable_string(const void *data, int len);

/*
 * The working_fdt points to our working flattened device tree.
 */
struct fdt_header *working_fdt;

void set_working_fdt_addr(ulong addr)
{
	void *buf;

	buf = map_sysmem(addr, 0);
	working_fdt = buf;
	env_set_hex("fdtaddr", addr);
}

/*
 * Get a value from the fdt and format it to be set in the environment
 */
static int fdt_value_env_set(const void *nodep, int len, const char *var)
{
	if (is_printable_string(nodep, len))
		env_set(var, (void *)nodep);
	else if (len == 4) {
		char buf[11];

		sprintf(buf, "0x%08X", fdt32_to_cpu(*(fdt32_t *)nodep));
		env_set(var, buf);
	} else if (len%4 == 0 && len <= 20) {
		/* Needed to print things like sha1 hashes. */
		char buf[41];
		int i;

		for (i = 0; i < len; i += sizeof(unsigned int))
			sprintf(buf + (i * 2), "%08x",
				*(unsigned int *)(nodep + i));
		env_set(var, buf);
	} else {
		printf("error: unprintable value\n");
		return 1;
	}
	return 0;
}

static const char * const fdt_member_table[] = {
	"magic",
	"totalsize",
	"off_dt_struct",
	"off_dt_strings",
	"off_mem_rsvmap",
	"version",
	"last_comp_version",
	"boot_cpuid_phys",
	"size_dt_strings",
	"size_dt_struct",
};

static int fdt_get_header_value(int argc, char * const argv[])
{
	fdt32_t *fdtp = (fdt32_t *)working_fdt;
	ulong val;
	int i;

	if (argv[2][0] != 'g')
		return CMD_RET_FAILURE;

	for (i = 0; i < ARRAY_SIZE(fdt_member_table); i++) {
		if (strcmp(fdt_member_table[i], argv[4]))
			continue;

		val = fdt32_to_cpu(fdtp[i]);
		env_set_hex(argv[3], val);
		return CMD_RET_SUCCESS;
	}

	return CMD_RET_FAILURE;
}

/*
 * Flattened Device Tree command, see the help for parameter definitions.
 */
static int do_fdt(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	if (argc < 2)
		return CMD_RET_USAGE;

	/*
	 * Set the address of the fdt
	 */
	if (strncmp(argv[1], "ad", 2) == 0) {
		unsigned long addr;
		int control = 0;
		struct fdt_header *blob;
		/*
		 * Set the address [and length] of the fdt.
		 */
		argc -= 2;
		argv += 2;
/* Temporary #ifdef - some archs don't have fdt_blob yet */
#ifdef CONFIG_OF_CONTROL
		if (argc && !strcmp(*argv, "-c")) {
			control = 1;
			argc--;
			argv++;
		}
#endif
		if (argc == 0) {
			if (control)
				blob = (struct fdt_header *)gd->fdt_blob;
			else
				blob = working_fdt;
			if (!blob || !fdt_valid(&blob))
				return 1;
			printf("The address of the fdt is %#08lx\n",
			       control ? (ulong)map_to_sysmem(blob) :
					env_get_hex("fdtaddr", 0));
			return 0;
		}

		addr = simple_strtoul(argv[0], NULL, 16);
		blob = map_sysmem(addr, 0);
		if (!fdt_valid(&blob))
			return 1;
		if (control)
			gd->fdt_blob = blob;
		else
			set_working_fdt_addr(addr);

		if (argc >= 2) {
			int  len;
			int  err;
			/*
			 * Optional new length
			 */
			len = simple_strtoul(argv[1], NULL, 16);
			if (len < fdt_totalsize(blob)) {
				printf ("New length %d < existing length %d, "
					"ignoring.\n",
					len, fdt_totalsize(blob));
			} else {
				/*
				 * Open in place with a new length.
				 */
				err = fdt_open_into(blob, blob, len);
				if (err != 0) {
					printf ("libfdt fdt_open_into(): %s\n",
						fdt_strerror(err));
				}
			}
		}

		return CMD_RET_SUCCESS;
	}

	if (!working_fdt) {
		puts(
			"No FDT memory address configured. Please configure\n"
			"the FDT address via \"fdt addr <address>\" command.\n"
			"Aborting!\n");
		return CMD_RET_FAILURE;
	}

	/*
	 * Move the working_fdt
	 */
	if (strncmp(argv[1], "mo", 2) == 0) {
		struct fdt_header *newaddr;
		int  len;
		int  err;

		if (argc < 4)
			return CMD_RET_USAGE;

		/*
		 * Set the address and length of the fdt.
		 */
		working_fdt = (struct fdt_header *)simple_strtoul(argv[2], NULL, 16);
		if (!fdt_valid(&working_fdt))
			return 1;

		newaddr = (struct fdt_header *)simple_strtoul(argv[3],NULL,16);

		/*
		 * If the user specifies a length, use that.  Otherwise use the
		 * current length.
		 */
		if (argc <= 4) {
			len = fdt_totalsize(working_fdt);
		} else {
			len = simple_strtoul(argv[4], NULL, 16);
			if (len < fdt_totalsize(working_fdt)) {
				printf ("New length 0x%X < existing length "
					"0x%X, aborting.\n",
					len, fdt_totalsize(working_fdt));
				return 1;
			}
		}

		/*
		 * Copy to the new location.
		 */
		err = fdt_open_into(working_fdt, newaddr, len);
		if (err != 0) {
			printf ("libfdt fdt_open_into(): %s\n",
				fdt_strerror(err));
			return 1;
		}
		set_working_fdt_addr((ulong)newaddr);
#ifdef CONFIG_OF_SYSTEM_SETUP
	/* Call the board-specific fixup routine */
	} else if (strncmp(argv[1], "sys", 3) == 0) {
		int err = ft_system_setup(working_fdt, gd->bd);

		if (err) {
			printf("Failed to add system information to FDT: %s\n",
			       fdt_strerror(err));
			return CMD_RET_FAILURE;
		}
#endif
	/*
	 * Make a new node
	 */
	} else if (strncmp(argv[1], "mk", 2) == 0) {
		char *pathp;		/* path */
		char *nodep;		/* new node to add */
		int  nodeoffset;	/* node offset from libfdt */
		int  err;

		/*
		 * Parameters: Node path, new node to be appended to the path.
		 */
		if (argc < 4)
			return CMD_RET_USAGE;

		pathp = argv[2];
		nodep = argv[3];

		nodeoffset = fdt_path_offset (working_fdt, pathp);
		if (nodeoffset < 0) {
			/*
			 * Not found or something else bad happened.
			 */
			printf ("libfdt fdt_path_offset() returned %s\n",
				fdt_strerror(nodeoffset));
			return 1;
		}
		err = fdt_add_subnode(working_fdt, nodeoffset, nodep);
		if (err < 0) {
			printf ("libfdt fdt_add_subnode(): %s\n",
				fdt_strerror(err));
			return 1;
		}

	/*
	 * Set the value of a property in the working_fdt.
	 */
	} else if (argv[1][0] == 's') {
		char *pathp;		/* path */
		char *prop;		/* property */
		int  nodeoffset;	/* node offset from libfdt */
		static char data[SCRATCHPAD] __aligned(4);/* property storage */
		const void *ptmp;
		int  len;		/* new length of the property */
		int  ret;		/* return value */

		/*
		 * Parameters: Node path, property, optional value.
		 */
		if (argc < 4)
			return CMD_RET_USAGE;

		pathp  = argv[2];
		prop   = argv[3];

		nodeoffset = fdt_path_offset (working_fdt, pathp);
		if (nodeoffset < 0) {
			/*
			 * Not found or something else bad happened.
			 */
			printf ("libfdt fdt_path_offset() returned %s\n",
				fdt_strerror(nodeoffset));
			return 1;
		}

		if (argc == 4) {
			len = 0;
		} else {
			ptmp = fdt_getprop(working_fdt, nodeoffset, prop, &len);
			if (len > SCRATCHPAD) {
				printf("prop (%d) doesn't fit in scratchpad!\n",
				       len);
				return 1;
			}
			if (ptmp != NULL)
				memcpy(data, ptmp, len);

			ret = fdt_parse_prop(&argv[4], argc - 4, data, &len);
			if (ret != 0)
				return ret;
		}

		ret = fdt_setprop(working_fdt, nodeoffset, prop, data, len);
		if (ret < 0) {
			printf ("libfdt fdt_setprop(): %s\n", fdt_strerror(ret));
			return 1;
		}

	/********************************************************************
	 * Get the value of a property in the working_fdt.
	 ********************************************************************/
	} else if (argv[1][0] == 'g') {
		char *subcmd;		/* sub-command */
		char *pathp;		/* path */
		char *prop;		/* property */
		char *var;		/* variable to store result */
		int  nodeoffset;	/* node offset from libfdt */
		const void *nodep;	/* property node pointer */
		int  len = 0;		/* new length of the property */

		/*
		 * Parameters: Node path, property, optional value.
		 */
		if (argc < 5)
			return CMD_RET_USAGE;

		subcmd = argv[2];

		if (argc < 6 && subcmd[0] != 's')
			return CMD_RET_USAGE;

		var    = argv[3];
		pathp  = argv[4];
		prop   = argv[5];

		nodeoffset = fdt_path_offset(working_fdt, pathp);
		if (nodeoffset < 0) {
			/*
			 * Not found or something else bad happened.
			 */
			printf("libfdt fdt_path_offset() returned %s\n",
				fdt_strerror(nodeoffset));
			return 1;
		}

		if (subcmd[0] == 'n' || (subcmd[0] == 's' && argc == 5)) {
			int reqIndex = -1;
			int startDepth = fdt_node_depth(
				working_fdt, nodeoffset);
			int curDepth = startDepth;
			int curIndex = -1;
			int nextNodeOffset = fdt_next_node(
				working_fdt, nodeoffset, &curDepth);

			if (subcmd[0] == 'n')
				reqIndex = simple_strtoul(argv[5], NULL, 16);

			while (curDepth > startDepth) {
				if (curDepth == startDepth + 1)
					curIndex++;
				if (subcmd[0] == 'n' && curIndex == reqIndex) {
					const char *node_name;

					node_name = fdt_get_name(working_fdt,
								 nextNodeOffset,
								 NULL);
					env_set(var, node_name);
					return 0;
				}
				nextNodeOffset = fdt_next_node(
					working_fdt, nextNodeOffset, &curDepth);
				if (nextNodeOffset < 0)
					break;
			}
			if (subcmd[0] == 's') {
				/* get the num nodes at this level */
				env_set_ulong(var, curIndex + 1);
			} else {
				/* node index not found */
				printf("libfdt node not found\n");
				return 1;
			}
		} else {
			nodep = fdt_getprop(
				working_fdt, nodeoffset, prop, &len);
			if (len == 0) {
				/* no property value */
				env_set(var, "");
				return 0;
			} else if (nodep && len > 0) {
				if (subcmd[0] == 'v') {
					int ret;

					ret = fdt_value_env_set(nodep, len,
								var);
					if (ret != 0)
						return ret;
				} else if (subcmd[0] == 'a') {
					/* Get address */
					char buf[11];

					sprintf(buf, "0x%p", nodep);
					env_set(var, buf);
				} else if (subcmd[0] == 's') {
					/* Get size */
					char buf[11];

					sprintf(buf, "0x%08X", len);
					env_set(var, buf);
				} else
					return CMD_RET_USAGE;
				return 0;
			} else {
				printf("libfdt fdt_getprop(): %s\n",
					fdt_strerror(len));
				return 1;
			}
		}

	/*
	 * Print (recursive) / List (single level)
	 */
	} else if ((argv[1][0] == 'p') || (argv[1][0] == 'l')) {
		int depth = MAX_LEVEL;	/* how deep to print */
		char *pathp;		/* path */
		char *prop;		/* property */
		int  ret;		/* return value */
		static char root[2] = "/";

		/*
		 * list is an alias for print, but limited to 1 level
		 */
		if (argv[1][0] == 'l') {
			depth = 1;
		}

		/*
		 * Get the starting path.  The root node is an oddball,
		 * the offset is zero and has no name.
		 */
		if (argc == 2)
			pathp = root;
		else
			pathp = argv[2];
		if (argc > 3)
			prop = argv[3];
		else
			prop = NULL;

		ret = fdt_print(pathp, prop, depth);
		if (ret != 0)
			return ret;

	/*
	 * Remove a property/node
	 */
	} else if (strncmp(argv[1], "rm", 2) == 0) {
		int  nodeoffset;	/* node offset from libfdt */
		int  err;

		/*
		 * Get the path.  The root node is an oddball, the offset
		 * is zero and has no name.
		 */
		nodeoffset = fdt_path_offset (working_fdt, argv[2]);
		if (nodeoffset < 0) {
			/*
			 * Not found or something else bad happened.
			 */
			printf ("libfdt fdt_path_offset() returned %s\n",
				fdt_strerror(nodeoffset));
			return 1;
		}
		/*
		 * Do the delete.  A fourth parameter means delete a property,
		 * otherwise delete the node.
		 */
		if (argc > 3) {
			err = fdt_delprop(working_fdt, nodeoffset, argv[3]);
			if (err < 0) {
				printf("libfdt fdt_delprop():  %s\n",
					fdt_strerror(err));
				return err;
			}
		} else {
			err = fdt_del_node(working_fdt, nodeoffset);
			if (err < 0) {
				printf("libfdt fdt_del_node():  %s\n",
					fdt_strerror(err));
				return err;
			}
		}

	/*
	 * Display header info
	 */
	} else if (argv[1][0] == 'h') {
		if (argc == 5)
			return fdt_get_header_value(argc, argv);

		u32 version = fdt_version(working_fdt);
		printf("magic:\t\t\t0x%x\n", fdt_magic(working_fdt));
		printf("totalsize:\t\t0x%x (%d)\n", fdt_totalsize(working_fdt),
		       fdt_totalsize(working_fdt));
		printf("off_dt_struct:\t\t0x%x\n",
		       fdt_off_dt_struct(working_fdt));
		printf("off_dt_strings:\t\t0x%x\n",
		       fdt_off_dt_strings(working_fdt));
		printf("off_mem_rsvmap:\t\t0x%x\n",
		       fdt_off_mem_rsvmap(working_fdt));
		printf("version:\t\t%d\n", version);
		printf("last_comp_version:\t%d\n",
		       fdt_last_comp_version(working_fdt));
		if (version >= 2)
			printf("boot_cpuid_phys:\t0x%x\n",
				fdt_boot_cpuid_phys(working_fdt));
		if (version >= 3)
			printf("size_dt_strings:\t0x%x\n",
				fdt_size_dt_strings(working_fdt));
		if (version >= 17)
			printf("size_dt_struct:\t\t0x%x\n",
				fdt_size_dt_struct(working_fdt));
		printf("number mem_rsv:\t\t0x%x\n",
		       fdt_num_mem_rsv(working_fdt));
		printf("\n");

	/*
	 * Set boot cpu id
	 */
	} else if (strncmp(argv[1], "boo", 3) == 0) {
		unsigned long tmp = simple_strtoul(argv[2], NULL, 16);
		fdt_set_boot_cpuid_phys(working_fdt, tmp);

	/*
	 * memory command
	 */
	} else if (strncmp(argv[1], "me", 2) == 0) {
		uint64_t addr, size;
		int err;
		addr = simple_strtoull(argv[2], NULL, 16);
		size = simple_strtoull(argv[3], NULL, 16);
		err = fdt_fixup_memory(working_fdt, addr, size);
		if (err < 0)
			return err;

	/*
	 * mem reserve commands
	 */
	} else if (strncmp(argv[1], "rs", 2) == 0) {
		if (argv[2][0] == 'p') {
			uint64_t addr, size;
			int total = fdt_num_mem_rsv(working_fdt);
			int j, err;
			printf("index\t\t   start\t\t    size\n");
			printf("-------------------------------"
				"-----------------\n");
			for (j = 0; j < total; j++) {
				err = fdt_get_mem_rsv(working_fdt, j, &addr, &size);
				if (err < 0) {
					printf("libfdt fdt_get_mem_rsv():  %s\n",
							fdt_strerror(err));
					return err;
				}
				printf("    %x\t%08x%08x\t%08x%08x\n", j,
					(u32)(addr >> 32),
					(u32)(addr & 0xffffffff),
					(u32)(size >> 32),
					(u32)(size & 0xffffffff));
			}
		} else if (argv[2][0] == 'a') {
			uint64_t addr, size;
			int err;
			addr = simple_strtoull(argv[3], NULL, 16);
			size = simple_strtoull(argv[4], NULL, 16);
			err = fdt_add_mem_rsv(working_fdt, addr, size);

			if (err < 0) {
				printf("libfdt fdt_add_mem_rsv():  %s\n",
					fdt_strerror(err));
				return err;
			}
		} else if (argv[2][0] == 'd') {
			unsigned long idx = simple_strtoul(argv[3], NULL, 16);
			int err = fdt_del_mem_rsv(working_fdt, idx);

			if (err < 0) {
				printf("libfdt fdt_del_mem_rsv():  %s\n",
					fdt_strerror(err));
				return err;
			}
		} else {
			/* Unrecognized command */
			return CMD_RET_USAGE;
		}
	}
#ifdef CONFIG_OF_BOARD_SETUP
	/* Call the board-specific fixup routine */
	else if (strncmp(argv[1], "boa", 3) == 0) {
		int err = ft_board_setup(working_fdt, gd->bd);

		if (err) {
			printf("Failed to update board information in FDT: %s\n",
			       fdt_strerror(err));
			return CMD_RET_FAILURE;
		}
#ifdef CONFIG_SOC_KEYSTONE
		ft_board_setup_ex(working_fdt, gd->bd);
#endif
	}
#endif
	/* Create a chosen node */
	else if (strncmp(argv[1], "cho", 3) == 0) {
		unsigned long initrd_start = 0, initrd_end = 0;

		if ((argc != 2) && (argc != 4))
			return CMD_RET_USAGE;

		if (argc == 4) {
			initrd_start = simple_strtoul(argv[2], NULL, 16);
			initrd_end = simple_strtoul(argv[3], NULL, 16);
		}

		fdt_chosen(working_fdt);
		fdt_initrd(working_fdt, initrd_start, initrd_end);

#if defined(CONFIG_FIT_SIGNATURE)
	} else if (strncmp(argv[1], "che", 3) == 0) {
		int cfg_noffset;
		int ret;
		unsigned long addr;
		struct fdt_header *blob;

		if (!working_fdt)
			return CMD_RET_FAILURE;

		if (argc > 2) {
			addr = simple_strtoul(argv[2], NULL, 16);
			blob = map_sysmem(addr, 0);
		} else {
			blob = (struct fdt_header *)gd->fdt_blob;
		}
		if (!fdt_valid(&blob))
			return 1;

		gd->fdt_blob = blob;
		cfg_noffset = fit_conf_get_node(working_fdt, NULL);
		if (!cfg_noffset) {
			printf("Could not find configuration node: %s\n",
			       fdt_strerror(cfg_noffset));
			return CMD_RET_FAILURE;
		}

		ret = fit_config_verify(working_fdt, cfg_noffset);
		if (ret == 0)
			return CMD_RET_SUCCESS;
		else
			return CMD_RET_FAILURE;
#endif

	}
#ifdef CONFIG_OF_LIBFDT_OVERLAY
	/* apply an overlay */
	else if (strncmp(argv[1], "ap", 2) == 0) {
		unsigned long addr;
		struct fdt_header *blob;
		int ret;

		if (argc != 3)
			return CMD_RET_USAGE;

		if (!working_fdt)
			return CMD_RET_FAILURE;

		addr = simple_strtoul(argv[2], NULL, 16);
		blob = map_sysmem(addr, 0);
		if (!fdt_valid(&blob))
			return CMD_RET_FAILURE;

		/* apply method prints messages on error */
		ret = fdt_overlay_apply_verbose(working_fdt, blob);
		if (ret)
			return CMD_RET_FAILURE;
	}
#endif
	/* resize the fdt */
	else if (strncmp(argv[1], "re", 2) == 0) {
		uint extrasize;
		if (argc > 2)
			extrasize = simple_strtoul(argv[2], NULL, 16);
		else
			extrasize = 0;
		fdt_shrink_to_minimum(working_fdt, extrasize);
	}
	else {
		/* Unrecognized command */
		return CMD_RET_USAGE;
	}

	return 0;
}

/****************************************************************************/

/**
 * fdt_valid() - Check if an FDT is valid. If not, change it to NULL
 *
 * @blobp: Pointer to FDT pointer
 * @return 1 if OK, 0 if bad (in which case *blobp is set to NULL)
 */
static int fdt_valid(struct fdt_header **blobp)
{
	const void *blob = *blobp;
	int err;

	if (blob == NULL) {
		printf ("The address of the fdt is invalid (NULL).\n");
		return 0;
	}

	err = fdt_check_header(blob);
	if (err == 0)
		return 1;	/* valid */

	if (err < 0) {
		printf("libfdt fdt_check_header(): %s", fdt_strerror(err));
		/*
		 * Be more informative on bad version.
		 */
		if (err == -FDT_ERR_BADVERSION) {
			if (fdt_version(blob) <
			    FDT_FIRST_SUPPORTED_VERSION) {
				printf (" - too old, fdt %d < %d",
					fdt_version(blob),
					FDT_FIRST_SUPPORTED_VERSION);
			}
			if (fdt_last_comp_version(blob) >
			    FDT_LAST_SUPPORTED_VERSION) {
				printf (" - too new, fdt %d > %d",
					fdt_version(blob),
					FDT_LAST_SUPPORTED_VERSION);
			}
		}
		printf("\n");
		*blobp = NULL;
		return 0;
	}
	return 1;
}

/****************************************************************************/

/*
 * Parse the user's input, partially heuristic.  Valid formats:
 * <0x00112233 4 05>	- an array of cells.  Numbers follow standard
 *			C conventions.
 * [00 11 22 .. nn] - byte stream
 * "string"	- If the the value doesn't start with "<" or "[", it is
 *			treated as a string.  Note that the quotes are
 *			stripped by the parser before we get the string.
 * newval: An array of strings containing the new property as specified
 *	on the command line
 * count: The number of strings in the array
 * data: A bytestream to be placed in the property
 * len: The length of the resulting bytestream
 */
static int fdt_parse_prop(char * const *newval, int count, char *data, int *len)
{
	char *cp;		/* temporary char pointer */
	char *newp;		/* temporary newval char pointer */
	unsigned long tmp;	/* holds converted values */
	int stridx = 0;

	*len = 0;
	newp = newval[0];

	/* An array of cells */
	if (*newp == '<') {
		newp++;
		while ((*newp != '>') && (stridx < count)) {
			/*
			 * Keep searching until we find that last ">"
			 * That way users don't have to escape the spaces
			 */
			if (*newp == '\0') {
				newp = newval[++stridx];
				continue;
			}

			cp = newp;
			tmp = simple_strtoul(cp, &newp, 0);
			if (*cp != '?')
				*(fdt32_t *)data = cpu_to_fdt32(tmp);
			else
				newp++;

			data  += 4;
			*len += 4;

			/* If the ptr didn't advance, something went wrong */
			if ((newp - cp) <= 0) {
				printf("Sorry, I could not convert \"%s\"\n",
					cp);
				return 1;
			}

			while (*newp == ' ')
				newp++;
		}

		if (*newp != '>') {
			printf("Unexpected character '%c'\n", *newp);
			return 1;
		}
	} else if (*newp == '[') {
		/*
		 * Byte stream.  Convert the values.
		 */
		newp++;
		while ((stridx < count) && (*newp != ']')) {
			while (*newp == ' ')
				newp++;
			if (*newp == '\0') {
				newp = newval[++stridx];
				continue;
			}
			if (!isxdigit(*newp))
				break;
			tmp = simple_strtoul(newp, &newp, 16);
			*data++ = tmp & 0xFF;
			*len    = *len + 1;
		}
		if (*newp != ']') {
			printf("Unexpected character '%c'\n", *newp);
			return 1;
		}
	} else {
		/*
		 * Assume it is one or more strings.  Copy it into our
		 * data area for convenience (including the
		 * terminating '\0's).
		 */
		while (stridx < count) {
			size_t length = strlen(newp) + 1;
			strcpy(data, newp);
			data += length;
			*len += length;
			newp = newval[++stridx];
		}
	}
	return 0;
}

/****************************************************************************/

/*
 * Heuristic to guess if this is a string or concatenated strings.
 */

static int is_printable_string(const void *data, int len)
{
	const char *s = data;

	/* zero length is not */
	if (len == 0)
		return 0;

	/* must terminate with zero or '\n' */
	if (s[len - 1] != '\0' && s[len - 1] != '\n')
		return 0;

	/* printable or a null byte (concatenated strings) */
	while (((*s == '\0') || isprint(*s) || isspace(*s)) && (len > 0)) {
		/*
		 * If we see a null, there are three possibilities:
		 * 1) If len == 1, it is the end of the string, printable
		 * 2) Next character also a null, not printable.
		 * 3) Next character not a null, continue to check.
		 */
		if (s[0] == '\0') {
			if (len == 1)
				return 1;
			if (s[1] == '\0')
				return 0;
		}
		s++;
		len--;
	}

	/* Not the null termination, or not done yet: not printable */
	if (*s != '\0' || (len != 0))
		return 0;

	return 1;
}


/*
 * Print the property in the best format, a heuristic guess.  Print as
 * a string, concatenated strings, a byte, word, double word, or (if all
 * else fails) it is printed as a stream of bytes.
 */
static void print_data(const void *data, int len)
{
	int j;

	/* no data, don't print */
	if (len == 0)
		return;

	/*
	 * It is a string, but it may have multiple strings (embedded '\0's).
	 */
	if (is_printable_string(data, len)) {
		puts("\"");
		j = 0;
		while (j < len) {
			if (j > 0)
				puts("\", \"");
			puts(data);
			j    += strlen(data) + 1;
			data += strlen(data) + 1;
		}
		puts("\"");
		return;
	}

	if ((len %4) == 0) {
		if (len > CMD_FDT_MAX_DUMP)
			printf("* 0x%p [0x%08x]", data, len);
		else {
			const __be32 *p;

			printf("<");
			for (j = 0, p = data; j < len/4; j++)
				printf("0x%08x%s", fdt32_to_cpu(p[j]),
					j < (len/4 - 1) ? " " : "");
			printf(">");
		}
	} else { /* anything else... hexdump */
		if (len > CMD_FDT_MAX_DUMP)
			printf("* 0x%p [0x%08x]", data, len);
		else {
			const u8 *s;

			printf("[");
			for (j = 0, s = data; j < len; j++)
				printf("%02x%s", s[j], j < len - 1 ? " " : "");
			printf("]");
		}
	}
}

/****************************************************************************/

/*
 * Recursively print (a portion of) the working_fdt.  The depth parameter
 * determines how deeply nested the fdt is printed.
 */
static int fdt_print(const char *pathp, char *prop, int depth)
{
	static char tabs[MAX_LEVEL+1] =
		"\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t"
		"\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t";
	const void *nodep;	/* property node pointer */
	int  nodeoffset;	/* node offset from libfdt */
	int  nextoffset;	/* next node offset from libfdt */
	uint32_t tag;		/* tag */
	int  len;		/* length of the property */
	int  level = 0;		/* keep track of nesting level */
	const struct fdt_property *fdt_prop;

	nodeoffset = fdt_path_offset (working_fdt, pathp);
	if (nodeoffset < 0) {
		/*
		 * Not found or something else bad happened.
		 */
		printf ("libfdt fdt_path_offset() returned %s\n",
			fdt_strerror(nodeoffset));
		return 1;
	}
	/*
	 * The user passed in a property as well as node path.
	 * Print only the given property and then return.
	 */
	if (prop) {
		nodep = fdt_getprop (working_fdt, nodeoffset, prop, &len);
		if (len == 0) {
			/* no property value */
			printf("%s %s\n", pathp, prop);
			return 0;
		} else if (nodep && len > 0) {
			printf("%s = ", prop);
			print_data (nodep, len);
			printf("\n");
			return 0;
		} else {
			printf ("libfdt fdt_getprop(): %s\n",
				fdt_strerror(len));
			return 1;
		}
	}

	/*
	 * The user passed in a node path and no property,
	 * print the node and all subnodes.
	 */
	while(level >= 0) {
		tag = fdt_next_tag(working_fdt, nodeoffset, &nextoffset);
		switch(tag) {
		case FDT_BEGIN_NODE:
			pathp = fdt_get_name(working_fdt, nodeoffset, NULL);
			if (level <= depth) {
				if (pathp == NULL)
					pathp = "/* NULL pointer error */";
				if (*pathp == '\0')
					pathp = "/";	/* root is nameless */
				printf("%s%s {\n",
					&tabs[MAX_LEVEL - level], pathp);
			}
			level++;
			if (level >= MAX_LEVEL) {
				printf("Nested too deep, aborting.\n");
				return 1;
			}
			break;
		case FDT_END_NODE:
			level--;
			if (level <= depth)
				printf("%s};\n", &tabs[MAX_LEVEL - level]);
			if (level == 0) {
				level = -1;		/* exit the loop */
			}
			break;
		case FDT_PROP:
			fdt_prop = fdt_offset_ptr(working_fdt, nodeoffset,
					sizeof(*fdt_prop));
			pathp    = fdt_string(working_fdt,
					fdt32_to_cpu(fdt_prop->nameoff));
			len      = fdt32_to_cpu(fdt_prop->len);
			nodep    = fdt_prop->data;
			if (len < 0) {
				printf ("libfdt fdt_getprop(): %s\n",
					fdt_strerror(len));
				return 1;
			} else if (len == 0) {
				/* the property has no value */
				if (level <= depth)
					printf("%s%s;\n",
						&tabs[MAX_LEVEL - level],
						pathp);
			} else {
				if (level <= depth) {
					printf("%s%s = ",
						&tabs[MAX_LEVEL - level],
						pathp);
					print_data (nodep, len);
					printf(";\n");
				}
			}
			break;
		case FDT_NOP:
			printf("%s/* NOP */\n", &tabs[MAX_LEVEL - level]);
			break;
		case FDT_END:
			return 1;
		default:
			if (level <= depth)
				printf("Unknown tag 0x%08X\n", tag);
			return 1;
		}
		nodeoffset = nextoffset;
	}
	return 0;
}

/********************************************************************/
#ifdef CONFIG_SYS_LONGHELP
static char fdt_help_text[] =
	"addr [-c]  <addr> [<length>]   - Set the [control] fdt location to <addr>\n"
#ifdef CONFIG_OF_LIBFDT_OVERLAY
	"fdt apply <addr>                    - Apply overlay to the DT\n"
#endif
#ifdef CONFIG_OF_BOARD_SETUP
	"fdt boardsetup                      - Do board-specific set up\n"
#endif
#ifdef CONFIG_OF_SYSTEM_SETUP
	"fdt systemsetup                     - Do system-specific set up\n"
#endif
	"fdt move   <fdt> <newaddr> <length> - Copy the fdt to <addr> and make it active\n"
	"fdt resize [<extrasize>]            - Resize fdt to size + padding to 4k addr + some optional <extrasize> if needed\n"
	"fdt print  <path> [<prop>]          - Recursive print starting at <path>\n"
	"fdt list   <path> [<prop>]          - Print one level starting at <path>\n"
	"fdt get value <var> <path> <prop>   - Get <property> and store in <var>\n"
	"fdt get name <var> <path> <index>   - Get name of node <index> and store in <var>\n"
	"fdt get addr <var> <path> <prop>    - Get start address of <property> and store in <var>\n"
	"fdt get size <var> <path> [<prop>]  - Get size of [<property>] or num nodes and store in <var>\n"
	"fdt set    <path> <prop> [<val>]    - Set <property> [to <val>]\n"
	"fdt mknode <path> <node>            - Create a new node after <path>\n"
	"fdt rm     <path> [<prop>]          - Delete the node or <property>\n"
	"fdt header [get <var> <member>]     - Display header info\n"
	"                                      get - get header member <member> and store it in <var>\n"
	"fdt bootcpu <id>                    - Set boot cpuid\n"
	"fdt memory <addr> <size>            - Add/Update memory node\n"
	"fdt rsvmem print                    - Show current mem reserves\n"
	"fdt rsvmem add <addr> <size>        - Add a mem reserve\n"
	"fdt rsvmem delete <index>           - Delete a mem reserves\n"
	"fdt chosen [<start> <end>]          - Add/update the /chosen branch in the tree\n"
	"                                        <start>/<end> - initrd start/end addr\n"
#if defined(CONFIG_FIT_SIGNATURE)
	"fdt checksign [<addr>]              - check FIT signature\n"
	"                                        <start> - addr of key blob\n"
	"                                                  default gd->fdt_blob\n"
#endif
	"NOTE: Dereference aliases by omitting the leading '/', "
		"e.g. fdt print ethernet0.";
#endif

U_BOOT_CMD(
	fdt,	255,	0,	do_fdt,
	"flattened device tree utility commands", fdt_help_text
);
