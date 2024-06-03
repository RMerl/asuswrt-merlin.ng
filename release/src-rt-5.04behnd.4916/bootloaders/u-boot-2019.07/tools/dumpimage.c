// SPDX-License-Identifier: GPL-2.0+
/*
 * Based on mkimage.c.
 *
 * Written by Guilherme Maciel Ferreira <guilherme.maciel.ferreira@gmail.com>
 */

#include "dumpimage.h"
#include <image.h>
#include <version.h>

static void usage(void);

/* parameters initialized by core will be used by the image type code */
static struct image_tool_params params = {
	.type = IH_TYPE_KERNEL,
};

/*
 * dumpimage_extract_subimage -
 *
 * It scans all registered image types,
 * verifies image_header for each supported image type
 * if verification is successful, it extracts the desired file,
 * indexed by pflag, from the image
 *
 * returns negative if input image format does not match with any of
 * supported image types
 */
static int dumpimage_extract_subimage(struct image_type_params *tparams,
		void *ptr, struct stat *sbuf)
{
	int retval = -1;

	if (tparams->verify_header) {
		retval = tparams->verify_header((unsigned char *)ptr,
				sbuf->st_size, &params);
		if (retval != 0)
			return -1;
		/*
		 * Extract the file from the image
		 * if verify is successful
		 */
		if (tparams->extract_subimage) {
			retval = tparams->extract_subimage(ptr, &params);
		} else {
			fprintf(stderr,
				"%s: extract_subimage undefined for %s\n",
				params.cmdname, tparams->name);
			return -2;
		}
	}

	return retval;
}

int main(int argc, char **argv)
{
	int opt;
	int ifd = -1;
	struct stat sbuf;
	char *ptr;
	int retval = EXIT_SUCCESS;
	struct image_type_params *tparams = NULL;

	params.cmdname = *argv;

	while ((opt = getopt(argc, argv, "hlo:T:p:V")) != -1) {
		switch (opt) {
		case 'l':
			params.lflag = 1;
			break;
		case 'o':
			params.outfile = optarg;
			params.iflag = 1;
			break;
		case 'T':
			params.type = genimg_get_type_id(optarg);
			if (params.type < 0) {
				fprintf(stderr, "%s: Invalid type\n",
					params.cmdname);
				exit(EXIT_FAILURE);
			}
			break;
		case 'p':
			params.pflag = strtoul(optarg, &ptr, 10);
			if (*ptr) {
				fprintf(stderr,
					"%s: invalid file position %s\n",
					params.cmdname, *argv);
				exit(EXIT_FAILURE);
			}
			break;
		case 'V':
			printf("dumpimage version %s\n", PLAIN_VERSION);
			exit(EXIT_SUCCESS);
		case 'h':
			usage();
		default:
			usage();
			break;
		}
	}

	if (argc < 2)
		usage();

	if (optind >= argc) {
		fprintf(stderr, "%s: image file missing\n", params.cmdname);
		exit(EXIT_FAILURE);
	}

	params.imagefile = argv[optind];

	/* set tparams as per input type_id */
	tparams = imagetool_get_type(params.type);
	if (tparams == NULL) {
		fprintf(stderr, "%s: unsupported type: %s\n",
			params.cmdname, genimg_get_type_name(params.type));
		exit(EXIT_FAILURE);
	}

	/*
	 * check the passed arguments parameters meets the requirements
	 * as per image type to be generated/listed
	 */
	if (tparams->check_params) {
		if (tparams->check_params(&params)) {
			fprintf(stderr, "%s: Parameter check failed\n",
				params.cmdname);
			exit(EXIT_FAILURE);
		}
	}

	if (!params.lflag && !params.outfile) {
		fprintf(stderr, "%s: No output file provided\n",
			params.cmdname);
		exit(EXIT_FAILURE);
	}

	ifd = open(params.imagefile, O_RDONLY|O_BINARY);
	if (ifd < 0) {
		fprintf(stderr, "%s: Can't open \"%s\": %s\n", params.cmdname,
			params.imagefile, strerror(errno));
		exit(EXIT_FAILURE);
	}

	if (fstat(ifd, &sbuf) < 0) {
		fprintf(stderr, "%s: Can't stat \"%s\": %s\n", params.cmdname,
			params.imagefile, strerror(errno));
		exit(EXIT_FAILURE);
	}

	if ((uint32_t)sbuf.st_size < tparams->header_size) {
		fprintf(stderr, "%s: Bad size: \"%s\" is not valid image\n",
			params.cmdname, params.imagefile);
		exit(EXIT_FAILURE);
	}

	ptr = mmap(0, sbuf.st_size, PROT_READ, MAP_SHARED, ifd, 0);
	if (ptr == MAP_FAILED) {
		fprintf(stderr, "%s: Can't read \"%s\": %s\n", params.cmdname,
			params.imagefile, strerror(errno));
		exit(EXIT_FAILURE);
	}

	/*
	 * Both calls bellow scan through dumpimage registry for all
	 * supported image types and verify the input image file
	 * header for match
	 */
	if (params.iflag) {
		/*
		 * Extract the data files from within the matched
		 * image type. Returns the error code if not matched
		 */
		retval = dumpimage_extract_subimage(tparams, ptr, &sbuf);
	} else {
		/*
		 * Print the image information for matched image type
		 * Returns the error code if not matched
		 */
		retval = imagetool_verify_print_header(ptr, &sbuf, tparams,
						       &params);
	}

	(void)munmap((void *)ptr, sbuf.st_size);
	(void)close(ifd);

	return retval;
}

static void usage(void)
{
	fprintf(stderr, "Usage: %s -l image\n"
		"          -l ==> list image header information\n",
		params.cmdname);
	fprintf(stderr,
		"       %s [-T type] [-p position] [-o outfile] image\n"
		"          -T ==> declare image type as 'type'\n"
		"          -p ==> 'position' (starting at 0) of the component to extract from image\n"
		"          -o ==> extract component to file 'outfile'\n",
		params.cmdname);
	fprintf(stderr,
		"       %s -h ==> print usage information and exit\n",
		params.cmdname);
	fprintf(stderr,
		"       %s -V ==> print version information and exit\n",
		params.cmdname);

	exit(EXIT_SUCCESS);
}
