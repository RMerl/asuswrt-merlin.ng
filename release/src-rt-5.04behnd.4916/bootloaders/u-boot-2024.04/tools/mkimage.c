// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2008 Semihalf
 *
 * (C) Copyright 2000-2009
 * DENX Software Engineering
 * Wolfgang Denk, wd@denx.de
 */

#include "imagetool.h"
#include "mkimage.h"
#include "imximage.h"
#include <fit_common.h>
#include <getopt.h>
#include <image.h>
#include <version.h>
#ifdef __linux__
#include <sys/ioctl.h>
#endif

static void copy_file(int, const char *, int);

/* parameters initialized by core will be used by the image type code */
static struct image_tool_params params = {
	.os = IH_OS_LINUX,
	.arch = IH_ARCH_PPC,
	.type = IH_TYPE_KERNEL,
	.comp = IH_COMP_GZIP,
	.dtc = MKIMAGE_DEFAULT_DTC_OPTIONS,
	.imagename = "",
	.imagename2 = "",
};

static enum ih_category cur_category;

static int h_compare_category_name(const void *vtype1, const void *vtype2)
{
	const int *type1 = vtype1;
	const int *type2 = vtype2;
	const char *name1 = genimg_get_cat_short_name(cur_category, *type1);
	const char *name2 = genimg_get_cat_short_name(cur_category, *type2);

	return strcmp(name1, name2);
}

static int show_valid_options(enum ih_category category)
{
	int *order;
	int count;
	int item;
	int i;

	count = genimg_get_cat_count(category);
	order = calloc(count, sizeof(*order));
	if (!order)
		return -ENOMEM;

	/* Sort the names in order of short name for easier reading */
	for (i = 0, item = 0; i < count; i++, item++) {
		while (!genimg_cat_has_id(category, item) && i < count) {
			item++;
			count--;
		}
		order[i] = item;
	}
	cur_category = category;
	qsort(order, count, sizeof(int), h_compare_category_name);

	fprintf(stderr, "\nInvalid %s, supported are:\n",
		genimg_get_cat_desc(category));
	for (i = 0; i < count; i++) {
		item = order[i];
		fprintf(stderr, "\t%-15s  %s\n",
			genimg_get_cat_short_name(category, item),
			genimg_get_cat_name(category, item));
	}
	fprintf(stderr, "\n");
	free(order);

	return 0;
}

static void usage(const char *msg)
{
	fprintf(stderr, "Error: %s\n", msg);
	fprintf(stderr, "Usage: %s [-T type] -l image\n"
			 "          -l ==> list image header information\n"
			 "          -T ==> parse image file as 'type'\n"
			 "          -q ==> quiet\n",
		params.cmdname);
	fprintf(stderr,
		"       %s [-x] -A arch -O os -T type -C comp -a addr -e ep -n name -d data_file[:data_file...] image\n"
		"          -A ==> set architecture to 'arch'\n"
		"          -O ==> set operating system to 'os'\n"
		"          -T ==> set image type to 'type'\n"
		"          -C ==> set compression type 'comp'\n"
		"          -a ==> set load address to 'addr' (hex)\n"
		"          -e ==> set entry point to 'ep' (hex)\n"
		"          -n ==> set image name to 'name'\n"
		"          -R ==> set second image name to 'name'\n"
		"          -d ==> use image data from 'datafile'\n"
		"          -x ==> set XIP (execute in place)\n"
		"          -s ==> create an image with no data\n"
		"          -v ==> verbose\n",
		params.cmdname);
	fprintf(stderr,
		"       %s [-D dtc_options] [-f fit-image.its|-f auto|-f auto-conf|-F] [-b <dtb> [-b <dtb>]] [-E] [-B size] [-i <ramdisk.cpio.gz>] fit-image\n"
		"           <dtb> file is used with -f auto, it may occur multiple times.\n",
		params.cmdname);
	fprintf(stderr,
		"          -D => set all options for device tree compiler\n"
		"          -f => input filename for FIT source\n"
		"          -i => input filename for ramdisk file\n"
		"          -E => place data outside of the FIT structure\n"
		"          -B => align size in hex for FIT structure and header\n"
		"          -b => append the device tree binary to the FIT\n"
		"          -t => update the timestamp in the FIT\n");
#if CONFIG_IS_ENABLED(FIT_SIGNATURE)
	fprintf(stderr,
		"Signing / verified boot options: [-k keydir] [-K dtb] [ -c <comment>] [-p addr] [-r] [-N engine]\n"
		"          -k => set directory containing private keys\n"
		"          -K => write public keys to this .dtb file\n"
		"          -g => set key name hint\n"
		"          -G => use this signing key (in lieu of -k)\n"
		"          -c => add comment in signature node\n"
		"          -F => re-sign existing FIT image\n"
		"          -p => place external data at a static position\n"
		"          -r => mark keys used as 'required' in dtb\n"
		"          -N => openssl engine to use for signing\n"
		"          -o => algorithm to use for signing\n");
#else
	fprintf(stderr,
		"Signing / verified boot not supported (CONFIG_TOOLS_FIT_SIGNATURE undefined)\n");
#endif

	fprintf(stderr, "       %s -V ==> print version information and exit\n",
		params.cmdname);
	fprintf(stderr, "Use '-T list' to see a list of available image types\n");
	fprintf(stderr, "Long options are available; read the man page for details\n");

	exit(EXIT_FAILURE);
}

static int add_content(int type, const char *fname)
{
	struct content_info *cont;

	cont = calloc(1, sizeof(*cont));
	if (!cont)
		return -1;
	cont->type = type;
	cont->fname = fname;
	if (params.content_tail)
		params.content_tail->next = cont;
	else
		params.content_head = cont;
	params.content_tail = cont;

	return 0;
}

static const char optstring[] =
	"a:A:b:B:c:C:d:D:e:Ef:Fg:G:i:k:K:ln:N:o:O:p:qrR:stT:vVx";

static const struct option longopts[] = {
	{ "load-address", required_argument, NULL, 'a' },
	{ "architecture", required_argument, NULL, 'A' },
	{ "device-tree", required_argument, NULL, 'b' },
	{ "alignment", required_argument, NULL, 'B' },
	{ "comment", required_argument, NULL, 'c' },
	{ "compression", required_argument, NULL, 'C' },
	{ "image", required_argument, NULL, 'd' },
	{ "dtcopts", required_argument, NULL, 'D' },
	{ "entry-point", required_argument, NULL, 'e' },
	{ "external", no_argument, NULL, 'E' },
	{ "fit", required_argument, NULL, 'f' },
	{ "update", no_argument, NULL, 'F' },
	{ "key-name-hint", required_argument, NULL, 'g' },
	{ "key-file", required_argument, NULL, 'G' },
	{ "help", no_argument, NULL, 'h' },
	{ "initramfs", required_argument, NULL, 'i' },
	{ "key-dir", required_argument, NULL, 'k' },
	{ "key-dest", required_argument, NULL, 'K' },
	{ "list", no_argument, NULL, 'l' },
	{ "config", required_argument, NULL, 'n' },
	{ "engine", required_argument, NULL, 'N' },
	{ "algo", required_argument, NULL, 'o' },
	{ "os", required_argument, NULL, 'O' },
	{ "position", required_argument, NULL, 'p' },
	{ "quiet", no_argument, NULL, 'q' },
	{ "key-required", no_argument, NULL, 'r' },
	{ "secondary-config", required_argument, NULL, 'R' },
	{ "no-copy", no_argument, NULL, 's' },
	{ "touch", no_argument, NULL, 't' },
	{ "type", required_argument, NULL, 'T' },
	{ "verbose", no_argument, NULL, 'v' },
	{ "version", no_argument, NULL, 'V' },
	{ "xip", no_argument, NULL, 'x' },
};

static void process_args(int argc, char **argv)
{
	char *ptr;
	int type = IH_TYPE_INVALID;
	char *datafile = NULL;
	int opt;

	while ((opt = getopt_long(argc, argv, optstring,
				  longopts, NULL)) != -1) {
		switch (opt) {
		case 'a':
			params.addr = strtoull(optarg, &ptr, 16);
			if (*ptr) {
				fprintf(stderr, "%s: invalid load address %s\n",
					params.cmdname, optarg);
				exit(EXIT_FAILURE);
			}
			break;
		case 'A':
			params.arch = genimg_get_arch_id(optarg);
			if (params.arch < 0) {
				show_valid_options(IH_ARCH);
				usage("Invalid architecture");
			}
			params.Aflag = 1;
			break;
		case 'b':
			if (add_content(IH_TYPE_FLATDT, optarg)) {
				fprintf(stderr,
					"%s: Out of memory adding content '%s'",
					params.cmdname, optarg);
				exit(EXIT_FAILURE);
			}
			break;
		case 'B':
			params.bl_len = strtoull(optarg, &ptr, 16);
			if (*ptr) {
				fprintf(stderr, "%s: invalid block length %s\n",
					params.cmdname, optarg);
				exit(EXIT_FAILURE);
			}

			break;
		case 'c':
			params.comment = optarg;
			break;
		case 'C':
			params.comp = genimg_get_comp_id(optarg);
			if (params.comp < 0) {
				show_valid_options(IH_COMP);
				usage("Invalid compression type");
			}
			break;
		case 'd':
			params.datafile = optarg;
			params.dflag = 1;
			break;
		case 'D':
			params.dtc = optarg;
			break;
		case 'e':
			params.ep = strtoull(optarg, &ptr, 16);
			if (*ptr) {
				fprintf(stderr, "%s: invalid entry point %s\n",
					params.cmdname, optarg);
				exit(EXIT_FAILURE);
			}
			params.eflag = 1;
			break;
		case 'E':
			params.external_data = true;
			break;
		case 'f':
			datafile = optarg;
			if (!strcmp(datafile, "auto"))
				params.auto_fit = AF_HASHED_IMG;
			else if (!strcmp(datafile, "auto-conf"))
				params.auto_fit = AF_SIGNED_CONF;
			/* fallthrough */
		case 'F':
			/*
			 * The flattened image tree (FIT) format
			 * requires a flattened device tree image type
			 */
			params.type = IH_TYPE_FLATDT;
			params.fflag = 1;
			break;
		case 'g':
			params.keyname = optarg;
			break;
		case 'G':
			params.keyfile = optarg;
			break;
		case 'i':
			params.fit_ramdisk = optarg;
			break;
		case 'k':
			params.keydir = optarg;
			break;
		case 'K':
			params.keydest = optarg;
			break;
		case 'l':
			params.lflag = 1;
			break;
		case 'n':
			params.imagename = optarg;
			break;
		case 'N':
			params.engine_id = optarg;
			break;
		case 'o':
			params.algo_name = optarg;
			break;
		case 'O':
			params.os = genimg_get_os_id(optarg);
			if (params.os < 0) {
				show_valid_options(IH_OS);
				usage("Invalid operating system");
			}
			break;
		case 'p':
			params.external_offset = strtoull(optarg, &ptr, 16);
			if (*ptr) {
				fprintf(stderr, "%s: invalid offset size %s\n",
					params.cmdname, optarg);
				exit(EXIT_FAILURE);
			}
			break;
		case 'q':
			params.quiet = 1;
			break;
		case 'r':
			params.require_keys = 1;
			break;
		case 'R':
			/*
			 * This entry is for the second configuration
			 * file, if only one is not enough.
			 */
			params.imagename2 = optarg;
			break;
		case 's':
			params.skipcpy = 1;
			break;
		case 't':
			params.reset_timestamp = 1;
			break;
		case 'T':
			if (strcmp(optarg, "list") == 0) {
				show_valid_options(IH_TYPE);
				exit(EXIT_SUCCESS);
			}
			type = genimg_get_type_id(optarg);
			if (type < 0) {
				show_valid_options(IH_TYPE);
				usage("Invalid image type");
			}
			break;
		case 'v':
			params.vflag++;
			break;
		case 'V':
			printf("mkimage version %s\n", PLAIN_VERSION);
			exit(EXIT_SUCCESS);
		case 'x':
			params.xflag++;
			break;
		default:
			usage("Invalid option");
		}
	}

	/* The last parameter is expected to be the imagefile */
	if (optind < argc)
		params.imagefile = argv[optind];

	if (params.auto_fit == AF_SIGNED_CONF) {
		if (!params.keyname || !params.algo_name)
			usage("Missing key/algo for auto-FIT with signed configs (use -g -o)");
	} else if (params.auto_fit == AF_HASHED_IMG && params.keyname) {
		params.auto_fit = AF_SIGNED_IMG;
		if (!params.algo_name)
			usage("Missing algorithm for auto-FIT with signed images (use -g)");
	}

	/*
	 * For auto-generated FIT images we need to know the image type to put
	 * in the FIT, which is separate from the file's image type (which
	 * will always be IH_TYPE_FLATDT in this case).
	 */
	if (params.type == IH_TYPE_FLATDT) {
		params.fit_image_type = type ? type : IH_TYPE_KERNEL;
		/* For auto-FIT, datafile has to be provided with -d */
		if (!params.auto_fit)
			params.datafile = datafile;
		else if (!params.datafile)
			usage("Missing data file for auto-FIT (use -d)");
	} else if (params.lflag || type != IH_TYPE_INVALID) {
		if (type == IH_TYPE_SCRIPT && !params.datafile)
			usage("Missing data file for script (use -d)");
		params.type = type;
	}

	if (!params.imagefile)
		usage("Missing output filename");
}

static void verify_image(const struct image_type_params *tparams)
{
	struct stat sbuf;
	void *ptr;
	int ifd;

	ifd = open(params.imagefile, O_RDONLY | O_BINARY);
	if (ifd < 0) {
		fprintf(stderr, "%s: Can't open %s: %s\n",
			params.cmdname, params.imagefile,
			strerror(errno));
		exit(EXIT_FAILURE);
	}

	if (fstat(ifd, &sbuf) < 0) {
		fprintf(stderr, "%s: Can't stat %s: %s\n",
			params.cmdname, params.imagefile, strerror(errno));
		exit(EXIT_FAILURE);
	}
	params.file_size = sbuf.st_size;

	ptr = mmap(0, params.file_size, PROT_READ, MAP_SHARED, ifd, 0);
	if (ptr == MAP_FAILED) {
		fprintf(stderr, "%s: Can't map %s: %s\n",
			params.cmdname, params.imagefile, strerror(errno));
		exit(EXIT_FAILURE);
	}

	if (tparams->verify_header((unsigned char *)ptr, params.file_size, &params) != 0) {
		fprintf(stderr, "%s: Failed to verify header of %s\n",
			params.cmdname, params.imagefile);
		exit(EXIT_FAILURE);
	}

	(void)munmap(ptr, params.file_size);
	(void)close(ifd);
}

void copy_datafile(int ifd, char *file)
{
	if (!file)
		return;
	for (;;) {
		char *sep = strchr(file, ':');

		if (sep) {
			*sep = '\0';
			copy_file(ifd, file, 1);
			*sep++ = ':';
			file = sep;
		} else {
			copy_file(ifd, file, 0);
			break;
		}
	}
}

int main(int argc, char **argv)
{
	int ifd = -1;
	struct stat sbuf;
	char *ptr;
	int retval = 0;
	struct image_type_params *tparams = NULL;
	int pad_len = 0;
	int dfd;
	size_t map_len;

	params.cmdname = *argv;
	params.addr = 0;
	params.ep = 0;

	process_args(argc, argv);

	/* set tparams as per input type_id */
	tparams = imagetool_get_type(params.type);
	if (tparams == NULL && !params.lflag) {
		fprintf (stderr, "%s: unsupported type %s\n",
			params.cmdname, genimg_get_type_name(params.type));
		exit (EXIT_FAILURE);
	}

	/*
	 * check the passed arguments parameters meets the requirements
	 * as per image type to be generated/listed
	 */
	if (tparams && tparams->check_params)
		if (tparams->check_params (&params))
			usage("Bad parameters for image type");

	if (!params.eflag) {
		params.ep = params.addr;
		/* If XIP, entry point must be after the U-Boot header */
		if (params.xflag && tparams)
			params.ep += tparams->header_size;
	}

	if (params.fflag){
		if (!tparams) {
			fprintf(stderr, "%s: Missing FIT support\n",
				params.cmdname);
			exit (EXIT_FAILURE);
		}
		if (tparams->fflag_handle)
			/*
			 * in some cases, some additional processing needs
			 * to be done if fflag is defined
			 *
			 * For ex. fit_handle_file for Fit file support
			 */
			retval = tparams->fflag_handle(&params);

		if (retval != EXIT_SUCCESS)
			usage("Bad parameters for FIT image type");
	}

	if (params.lflag || params.fflag) {
		ifd = open (params.imagefile, O_RDONLY|O_BINARY);
	} else {
		ifd = open (params.imagefile,
			O_RDWR|O_CREAT|O_TRUNC|O_BINARY, 0666);
	}

	if (ifd < 0) {
		fprintf (stderr, "%s: Can't open %s: %s\n",
			params.cmdname, params.imagefile,
			strerror(errno));
		exit (EXIT_FAILURE);
	}

	if (params.lflag || params.fflag) {
		uint64_t size;
		/*
		 * list header information of existing image
		 */
		if (fstat(ifd, &sbuf) < 0) {
			fprintf (stderr, "%s: Can't stat %s: %s\n",
				params.cmdname, params.imagefile,
				strerror(errno));
			exit (EXIT_FAILURE);
		}

		if ((sbuf.st_mode & S_IFMT) == S_IFBLK) {
#ifdef __linux__
#if defined(__linux__) && defined(_IOR) && !defined(BLKGETSIZE64)
#define BLKGETSIZE64 _IOR(0x12,114,size_t)	/* return device size in bytes (u64 *arg) */
#endif
			if (ioctl(ifd, BLKGETSIZE64, &size) < 0) {
				fprintf (stderr,
					"%s: failed to get size of block device \"%s\"\n",
					params.cmdname, params.imagefile);
				exit (EXIT_FAILURE);
			}
#else
			fprintf (stderr,
				"%s: \"%s\" is block device, don't know how to get its size\n",
				params.cmdname, params.imagefile);
			exit (EXIT_FAILURE);
#endif
		} else if (tparams && sbuf.st_size < (off_t)tparams->header_size) {
			fprintf (stderr,
				"%s: Bad size: \"%s\" is not valid image: size %llu < %u\n",
				params.cmdname, params.imagefile,
				(unsigned long long) sbuf.st_size,
				tparams->header_size);
			exit (EXIT_FAILURE);
		} else {
			size = sbuf.st_size;
		}

		ptr = mmap(0, size, PROT_READ, MAP_SHARED, ifd, 0);
		if (ptr == MAP_FAILED) {
			fprintf (stderr, "%s: Can't read %s: %s\n",
				params.cmdname, params.imagefile,
				strerror(errno));
			exit (EXIT_FAILURE);
		}

		/*
		 * Verifies the header format based on the expected header for image
		 * type in tparams. If tparams is NULL simply check all image types
		 * to find one that matches our header.
		 */
		retval = imagetool_verify_print_header(ptr, &sbuf, tparams, &params);

		(void) munmap((void *)ptr, sbuf.st_size);
		(void) close (ifd);
		if (!retval)
			summary_show(&params.summary, params.imagefile,
				     params.keydest);

		exit (retval);
	}

	if (!params.skipcpy && params.type != IH_TYPE_MULTI && params.type != IH_TYPE_SCRIPT) {
		if (!params.datafile) {
			fprintf(stderr, "%s: Option -d with image data file was not specified\n",
				params.cmdname);
			exit(EXIT_FAILURE);
		}
		dfd = open(params.datafile, O_RDONLY | O_BINARY);
		if (dfd < 0) {
			fprintf(stderr, "%s: Can't open %s: %s\n",
				params.cmdname, params.datafile,
				strerror(errno));
			exit(EXIT_FAILURE);
		}

		if (fstat(dfd, &sbuf) < 0) {
			fprintf(stderr, "%s: Can't stat %s: %s\n",
				params.cmdname, params.datafile,
				strerror(errno));
			exit(EXIT_FAILURE);
		}

		params.file_size = sbuf.st_size + tparams->header_size;
		close(dfd);
	}

	/*
	 * In case there an header with a variable
	 * length will be added, the corresponding
	 * function is called. This is responsible to
	 * allocate memory for the header itself.
	 */
	if (tparams->vrec_header)
		pad_len = tparams->vrec_header(&params, tparams);
	else
		memset(tparams->hdr, 0, tparams->header_size);

	if (write(ifd, tparams->hdr, tparams->header_size)
					!= tparams->header_size) {
		fprintf (stderr, "%s: Write error on %s: %s\n",
			params.cmdname, params.imagefile, strerror(errno));
		exit (EXIT_FAILURE);
	}

	if (!params.skipcpy) {
		if (params.type == IH_TYPE_MULTI ||
		    params.type == IH_TYPE_SCRIPT) {
			char *file = params.datafile;
			uint32_t size;

			for (;;) {
				char *sep = NULL;

				if (file) {
					if ((sep = strchr(file, ':')) != NULL) {
						*sep = '\0';
					}

					if (stat (file, &sbuf) < 0) {
						fprintf (stderr, "%s: Can't stat %s: %s\n",
							 params.cmdname, file, strerror(errno));
						exit (EXIT_FAILURE);
					}
					size = cpu_to_uimage (sbuf.st_size);
				} else {
					size = 0;
				}

				if (write(ifd, (char *)&size, sizeof(size)) != sizeof(size)) {
					fprintf (stderr, "%s: Write error on %s: %s\n",
						 params.cmdname, params.imagefile,
						 strerror(errno));
					exit (EXIT_FAILURE);
				}

				if (!file) {
					break;
				}

				if (sep) {
					*sep = ':';
					file = sep + 1;
				} else {
					file = NULL;
				}
			}
			copy_datafile(ifd, params.datafile);
		} else if (params.type == IH_TYPE_PBLIMAGE) {
			/* PBL has special Image format, implements its' own */
			pbl_load_uboot(ifd, &params);
		} else if (params.type == IH_TYPE_ZYNQMPBIF) {
			/* Image file is meta, walk through actual targets */
			int ret;

			ret = zynqmpbif_copy_image(ifd, &params);
			if (ret)
				return ret;
		} else if (params.type == IH_TYPE_IMX8IMAGE) {
			/* i.MX8/8X has special Image format */
			int ret;

			ret = imx8image_copy_image(ifd, &params);
			if (ret)
				return ret;
		} else if (params.type == IH_TYPE_IMX8MIMAGE) {
			/* i.MX8M has special Image format */
			int ret;

			ret = imx8mimage_copy_image(ifd, &params);
			if (ret)
				return ret;
		} else if ((params.type == IH_TYPE_RKSD) ||
				(params.type == IH_TYPE_RKSPI)) {
			/* Rockchip has special Image format */
			int ret;

			ret = rockchip_copy_image(ifd, &params);
			if (ret)
				return ret;
		} else {
			copy_file(ifd, params.datafile, pad_len);
		}
		if (params.type == IH_TYPE_FIRMWARE_IVT) {
			/* Add alignment and IVT */
			uint32_t aligned_filesize = ALIGN(params.file_size,
							  0x1000);
			flash_header_v2_t ivt_header = { { 0xd1, 0x2000, 0x40 },
					params.addr, 0, 0, 0, params.addr
							+ aligned_filesize
							- tparams->header_size,
					params.addr + aligned_filesize
							- tparams->header_size
							+ 0x20, 0 };
			int i = params.file_size;
			for (; i < aligned_filesize; i++) {
				if (write(ifd, (char *) &i, 1) != 1) {
					fprintf(stderr,
							"%s: Write error on %s: %s\n",
							params.cmdname,
							params.imagefile,
							strerror(errno));
					exit(EXIT_FAILURE);
				}
			}
			if (write(ifd, &ivt_header, sizeof(flash_header_v2_t))
					!= sizeof(flash_header_v2_t)) {
				fprintf(stderr, "%s: Write error on %s: %s\n",
						params.cmdname,
						params.imagefile,
						strerror(errno));
				exit(EXIT_FAILURE);
			}
		}
	}

	/* We're a bit of paranoid */
#if defined(_POSIX_SYNCHRONIZED_IO) && \
   !defined(__sun__) && \
   !defined(__FreeBSD__) && \
   !defined(__OpenBSD__) && \
   !defined(__APPLE__)
	(void) fdatasync (ifd);
#else
	(void) fsync (ifd);
#endif

	if (fstat(ifd, &sbuf) < 0) {
		fprintf (stderr, "%s: Can't stat %s: %s\n",
			params.cmdname, params.imagefile, strerror(errno));
		exit (EXIT_FAILURE);
	}
	params.file_size = sbuf.st_size;

	map_len = sbuf.st_size;
	ptr = mmap(0, map_len, PROT_READ | PROT_WRITE, MAP_SHARED, ifd, 0);
	if (ptr == MAP_FAILED) {
		fprintf (stderr, "%s: Can't map %s: %s\n",
			params.cmdname, params.imagefile, strerror(errno));
		exit (EXIT_FAILURE);
	}

	/* Setup the image header as per input image type*/
	if (tparams->set_header)
		tparams->set_header (ptr, &sbuf, ifd, &params);
	else {
		fprintf (stderr, "%s: Can't set header for %s\n",
			params.cmdname, tparams->name);
		exit (EXIT_FAILURE);
	}

	/* Print the image information by processing image header */
	if (tparams->print_header)
		tparams->print_header (ptr, &params);
	else {
		fprintf (stderr, "%s: Can't print header for %s\n",
			params.cmdname, tparams->name);
	}

	(void)munmap((void *)ptr, map_len);

	/* We're a bit of paranoid */
#if defined(_POSIX_SYNCHRONIZED_IO) && \
   !defined(__sun__) && \
   !defined(__FreeBSD__) && \
   !defined(__OpenBSD__) && \
   !defined(__APPLE__)
	(void) fdatasync (ifd);
#else
	(void) fsync (ifd);
#endif

	if (close(ifd)) {
		fprintf (stderr, "%s: Write error on %s: %s\n",
			params.cmdname, params.imagefile, strerror(errno));
		exit (EXIT_FAILURE);
	}

	if (tparams->verify_header)
		verify_image(tparams);

	exit (EXIT_SUCCESS);
}

static void
copy_file (int ifd, const char *datafile, int pad)
{
	int dfd;
	struct stat sbuf;
	unsigned char *ptr;
	int tail;
	int zero = 0;
	uint8_t zeros[4096];
	int offset = 0;
	int size, ret;
	struct image_type_params *tparams = imagetool_get_type(params.type);

	memset(zeros, 0, sizeof(zeros));

	if (params.vflag) {
		fprintf (stderr, "Adding Image %s\n", datafile);
	}

	if ((dfd = open(datafile, O_RDONLY|O_BINARY)) < 0) {
		fprintf (stderr, "%s: Can't open %s: %s\n",
			params.cmdname, datafile, strerror(errno));
		exit (EXIT_FAILURE);
	}

	if (fstat(dfd, &sbuf) < 0) {
		fprintf (stderr, "%s: Can't stat %s: %s\n",
			params.cmdname, datafile, strerror(errno));
		exit (EXIT_FAILURE);
	}

	if (sbuf.st_size == 0) {
		fprintf (stderr, "%s: Input file %s is empty, bailing out\n",
			params.cmdname, datafile);
		exit (EXIT_FAILURE);
	}

	ptr = mmap(0, sbuf.st_size, PROT_READ, MAP_SHARED, dfd, 0);
	if (ptr == MAP_FAILED) {
		fprintf (stderr, "%s: Can't read %s: %s\n",
			params.cmdname, datafile, strerror(errno));
		exit (EXIT_FAILURE);
	}

	if (params.xflag &&
	    (((params.type > IH_TYPE_INVALID) && (params.type < IH_TYPE_FLATDT)) ||
	     (params.type == IH_TYPE_KERNEL_NOLOAD) || (params.type == IH_TYPE_FIRMWARE_IVT))) {
		unsigned char *p = NULL;
		/*
		 * XIP: do not append the struct legacy_img_hdr at the
		 * beginning of the file, but consume the space
		 * reserved for it.
		 */

		if ((unsigned)sbuf.st_size < tparams->header_size) {
			fprintf (stderr,
				"%s: Bad size: \"%s\" is too small for XIP\n",
				params.cmdname, datafile);
			exit (EXIT_FAILURE);
		}

		for (p = ptr; p < ptr + tparams->header_size; p++) {
			if ( *p != 0xff ) {
				fprintf (stderr,
					"%s: Bad file: \"%s\" has invalid buffer for XIP\n",
					params.cmdname, datafile);
				exit (EXIT_FAILURE);
			}
		}

		offset = tparams->header_size;
	}

	size = sbuf.st_size - offset;

	ret = write(ifd, ptr + offset, size);
	if (ret != size) {
		if (ret < 0)
			fprintf (stderr, "%s: Write error on %s: %s\n",
				 params.cmdname, params.imagefile, strerror(errno));
		else if (ret < size)
			fprintf (stderr, "%s: Write only %d/%d bytes, "\
				 "probably no space left on the device\n",
				 params.cmdname, ret, size);
		exit (EXIT_FAILURE);
	}

	tail = size % 4;
	if ((pad == 1) && (tail != 0)) {

		if (write(ifd, (char *)&zero, 4-tail) != 4-tail) {
			fprintf (stderr, "%s: Write error on %s: %s\n",
				params.cmdname, params.imagefile,
				strerror(errno));
			exit (EXIT_FAILURE);
		}
	} else if (pad > 1) {
		while (pad > 0) {
			int todo = sizeof(zeros);

			if (todo > pad)
				todo = pad;
			if (write(ifd, (char *)&zeros, todo) != todo) {
				fprintf(stderr, "%s: Write error on %s: %s\n",
					params.cmdname, params.imagefile,
					strerror(errno));
				exit(EXIT_FAILURE);
			}
			pad -= todo;
		}
	}

	(void) munmap((void *)ptr, sbuf.st_size);
	(void) close (dfd);
}
