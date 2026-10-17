// SPDX-License-Identifier: GPL-2.0+
#include <image.h>
#include "fit_common.h"

static const char *cmdname;

static const char *algo_name = "sha1,rsa2048"; /* -a <algo> */
static const char *keydir = "."; /* -k <keydir> */
static const char *keyname = "key"; /* -n <keyname> */
static const char *require_keys; /* -r <conf|image> */
static const char *keydest; /* argv[n] */

static void __attribute__((__noreturn__)) print_usage(const char *msg)
{
	fprintf(stderr, "Error: %s\n", msg);
	fprintf(stderr, "Usage: %s [-a <algo>] [-k <keydir>] [-n <keyname>] [-r <conf|image>]"
			" <fdt blob>\n", cmdname);
	fprintf(stderr, "Help information: %s [-h]\n", cmdname);
	exit(EXIT_FAILURE);
}

static void __attribute__((__noreturn__)) print_help(void)
{
	fprintf(stderr, "Options:\n"
		"\t-a <algo>       Cryptographic algorithm. Optional parameter, default value: sha1,rsa2048\n"
		"\t-k <keydir>     Directory with public key. Optional parameter, default value: .\n"
		"\t-n <keyname>    Public key name. Optional parameter, default value: key\n"
		"\t-r <conf|image> Required: If present this indicates that the key must be verified for the image / configuration to be considered valid.\n"
		"\t<fdt blob>      FDT blob file for adding of the public key. Required parameter.\n");
	exit(EXIT_FAILURE);
}

static void process_args(int argc, char *argv[])
{
	int opt;

	while ((opt = getopt(argc, argv, "a:k:n:r:h")) != -1) {
		switch (opt) {
		case 'k':
			keydir = optarg;
			break;
		case 'a':
			algo_name = optarg;
			break;
		case 'n':
			keyname = optarg;
			break;
		case 'r':
			require_keys = optarg;
			break;
		case 'h':
			print_help();
		default:
			print_usage("Invalid option");
		}
	}
	/* The last parameter is expected to be the .dtb to add the public key to */
	if (optind < argc)
		keydest = argv[optind];

	if (!keydest)
		print_usage("Missing dtb file to update");
}

static void reset_info(struct image_sign_info *info)
{
	if (!info)
		fprintf(stderr, "Error: info is NULL in %s\n", __func__);

	memset(info, 0, sizeof(struct image_sign_info));

	info->keydir = keydir;
	info->keyname = keyname;
	info->name = algo_name;
	info->require_keys = require_keys;
	info->crypto = image_get_crypto_algo(algo_name);

	if (!info->crypto) {
		fprintf(stderr, "Unsupported signature algorithm '%s'\n",
			algo_name);
		exit(EXIT_FAILURE);
	}
}

static int add_pubkey(struct image_sign_info *info)
{
	int destfd = -1, ret;
	void *dest_blob = NULL;
	struct stat dest_sbuf;
	size_t size_inc = 0;

	if (!info)
		fprintf(stderr, "Error: info is NULL in %s\n", __func__);

	do {
		if (destfd >= 0) {
			munmap(dest_blob, dest_sbuf.st_size);
			close(destfd);

			fprintf(stderr, ".dtb too small, increasing size by 1024 bytes\n");
			size_inc = 1024;
		}

		destfd = mmap_fdt(cmdname, keydest, size_inc, &dest_blob,
				  &dest_sbuf, false, false);
		if (destfd < 0)
			exit(EXIT_FAILURE);

		ret = info->crypto->add_verify_data(info, dest_blob);
		if (ret == -ENOSPC)
			continue;
		else if (ret < 0)
			break;
	} while (ret == -ENOSPC);

	return ret;
}

int main(int argc, char *argv[])
{
	struct image_sign_info info;
	int ret;

	cmdname = argv[0];

	process_args(argc, argv);
	reset_info(&info);
	ret = add_pubkey(&info);

	if (ret < 0) {
		fprintf(stderr, "%s: Cannot add public key to FIT blob: %s\n",
			cmdname, strerror(ret));
		exit(EXIT_FAILURE);
	}

	exit(EXIT_SUCCESS);
}

