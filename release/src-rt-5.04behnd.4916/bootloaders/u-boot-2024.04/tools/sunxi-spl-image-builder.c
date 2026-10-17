/*
 * Allwinner NAND randomizer and image builder implementation:
 *
 * Copyright © 2016 NextThing Co.
 * Copyright © 2016 Free Electrons
 *
 * Author: Boris Brezillon <boris.brezillon@free-electrons.com>
 *
 */

#include <linux/bch.h>

#include <getopt.h>
#include <version.h>

#define BCH_PRIMITIVE_POLY	0x5803

#define ARRAY_SIZE(arr)		(sizeof(arr) / sizeof((arr)[0]))
#define DIV_ROUND_UP(n,d)	(((n) + (d) - 1) / (d))

struct image_info {
	int ecc_strength;
	int ecc_step_size;
	int page_size;
	int oob_size;
	int usable_page_size;
	int eraseblock_size;
	int scramble;
	int boot0;
	off_t offset;
	const char *source;
	const char *dest;
};

static void swap_bits(uint8_t *buf, int len)
{
	int i, j;

	for (j = 0; j < len; j++) {
		uint8_t byte = buf[j];

		buf[j] = 0;
		for (i = 0; i < 8; i++) {
			if (byte & (1 << i))
				buf[j] |= (1 << (7 - i));
		}
	}
}

static uint16_t lfsr_step(uint16_t state, int count)
{
	state &= 0x7fff;
	while (count--)
		state = ((state >> 1) |
			 ((((state >> 0) ^ (state >> 1)) & 1) << 14)) & 0x7fff;

	return state;
}

static uint16_t default_scrambler_seeds[] = {
	0x2b75, 0x0bd0, 0x5ca3, 0x62d1, 0x1c93, 0x07e9, 0x2162, 0x3a72,
	0x0d67, 0x67f9, 0x1be7, 0x077d, 0x032f, 0x0dac, 0x2716, 0x2436,
	0x7922, 0x1510, 0x3860, 0x5287, 0x480f, 0x4252, 0x1789, 0x5a2d,
	0x2a49, 0x5e10, 0x437f, 0x4b4e, 0x2f45, 0x216e, 0x5cb7, 0x7130,
	0x2a3f, 0x60e4, 0x4dc9, 0x0ef0, 0x0f52, 0x1bb9, 0x6211, 0x7a56,
	0x226d, 0x4ea7, 0x6f36, 0x3692, 0x38bf, 0x0c62, 0x05eb, 0x4c55,
	0x60f4, 0x728c, 0x3b6f, 0x2037, 0x7f69, 0x0936, 0x651a, 0x4ceb,
	0x6218, 0x79f3, 0x383f, 0x18d9, 0x4f05, 0x5c82, 0x2912, 0x6f17,
	0x6856, 0x5938, 0x1007, 0x61ab, 0x3e7f, 0x57c2, 0x542f, 0x4f62,
	0x7454, 0x2eac, 0x7739, 0x42d4, 0x2f90, 0x435a, 0x2e52, 0x2064,
	0x637c, 0x66ad, 0x2c90, 0x0bad, 0x759c, 0x0029, 0x0986, 0x7126,
	0x1ca7, 0x1605, 0x386a, 0x27f5, 0x1380, 0x6d75, 0x24c3, 0x0f8e,
	0x2b7a, 0x1418, 0x1fd1, 0x7dc1, 0x2d8e, 0x43af, 0x2267, 0x7da3,
	0x4e3d, 0x1338, 0x50db, 0x454d, 0x764d, 0x40a3, 0x42e6, 0x262b,
	0x2d2e, 0x1aea, 0x2e17, 0x173d, 0x3a6e, 0x71bf, 0x25f9, 0x0a5d,
	0x7c57, 0x0fbe, 0x46ce, 0x4939, 0x6b17, 0x37bb, 0x3e91, 0x76db,
};

static uint16_t brom_scrambler_seeds[] = { 0x4a80 };

static void scramble(const struct image_info *info,
		     int page, uint8_t *data, int datalen)
{
	uint16_t state;
	int i;

	/* Boot0 is always scrambled no matter the command line option. */
	if (info->boot0) {
		state = brom_scrambler_seeds[0];
	} else {
		unsigned seedmod = info->eraseblock_size / info->page_size;

		/* Bail out earlier if the user didn't ask for scrambling. */
		if (!info->scramble)
			return;

		if (seedmod > ARRAY_SIZE(default_scrambler_seeds))
			seedmod = ARRAY_SIZE(default_scrambler_seeds);

		state = default_scrambler_seeds[page % seedmod];
	}

	/* Prepare the initial state... */
	state = lfsr_step(state, 15);

	/* and start scrambling data. */
	for (i = 0; i < datalen; i++) {
		data[i] ^= state;
		state = lfsr_step(state, 8);
	}
}

static int write_page(const struct image_info *info, uint8_t *buffer,
		      FILE *src, FILE *rnd, FILE *dst,
		      struct bch_control *bch, int page)
{
	int steps = info->usable_page_size / info->ecc_step_size;
	int eccbytes = DIV_ROUND_UP(info->ecc_strength * 14, 8);
	off_t pos = ftell(dst);
	size_t pad, cnt;
	int i;

	if (eccbytes % 2)
		eccbytes++;

	memset(buffer, 0xff, info->page_size + info->oob_size);
	cnt = fread(buffer, 1, info->usable_page_size, src);
	if (!cnt) {
		if (!feof(src)) {
			fprintf(stderr,
				"Failed to read data from the source\n");
			return -1;
		} else {
			return 0;
		}
	}

	fwrite(buffer, info->page_size + info->oob_size, 1, dst);

	for (i = 0; i < info->usable_page_size; i++) {
		if (buffer[i] !=  0xff)
			break;
	}

	/* We leave empty pages at 0xff. */
	if (i == info->usable_page_size)
		return 0;

	/* Restore the source pointer to read it again. */
	fseek(src, -cnt, SEEK_CUR);

	/* Randomize unused space if scrambling is required. */
	if (info->scramble) {
		int offs;

		if (info->boot0) {
			size_t ret;

			offs = steps * (info->ecc_step_size + eccbytes + 4);
			cnt = info->page_size + info->oob_size - offs;
			ret = fread(buffer + offs, 1, cnt, rnd);
			if (!ret && !feof(rnd)) {
				fprintf(stderr,
					"Failed to read random data\n");
				return -1;
			}
		} else {
			offs = info->page_size + (steps * (eccbytes + 4));
			cnt = info->page_size + info->oob_size - offs;
			memset(buffer + offs, 0xff, cnt);
			scramble(info, page, buffer + offs, cnt);
		}
		fseek(dst, pos + offs, SEEK_SET);
		fwrite(buffer + offs, cnt, 1, dst);
	}

	for (i = 0; i < steps; i++) {
		int ecc_offs, data_offs;
		uint8_t *ecc;

		memset(buffer, 0xff, info->ecc_step_size + eccbytes + 4);
		ecc = buffer + info->ecc_step_size + 4;
		if (info->boot0) {
			data_offs = i * (info->ecc_step_size + eccbytes + 4);
			ecc_offs = data_offs + info->ecc_step_size + 4;
		} else {
			data_offs = i * info->ecc_step_size;
			ecc_offs = info->page_size + 4 + (i * (eccbytes + 4));
		}

		cnt = fread(buffer, 1, info->ecc_step_size, src);
		if (!cnt && !feof(src)) {
			fprintf(stderr,
				"Failed to read data from the source\n");
			return -1;
		}

		pad = info->ecc_step_size - cnt;
		if (pad) {
			if (info->scramble && info->boot0) {
				size_t ret;

				ret = fread(buffer + cnt, 1, pad, rnd);
				if (!ret && !feof(rnd)) {
					fprintf(stderr,
						"Failed to read random data\n");
					return -1;
				}
			} else {
				memset(buffer + cnt, 0xff, pad);
			}
		}

		memset(ecc, 0, eccbytes);
		swap_bits(buffer, info->ecc_step_size + 4);
		encode_bch(bch, buffer, info->ecc_step_size + 4, ecc);
		swap_bits(buffer, info->ecc_step_size + 4);
		swap_bits(ecc, eccbytes);
		scramble(info, page, buffer, info->ecc_step_size + 4 + eccbytes);

		fseek(dst, pos + data_offs, SEEK_SET);
		fwrite(buffer, info->ecc_step_size, 1, dst);
		fseek(dst, pos + ecc_offs - 4, SEEK_SET);
		fwrite(ecc - 4, eccbytes + 4, 1, dst);
	}

	/* Fix BBM. */
	fseek(dst, pos + info->page_size, SEEK_SET);
	memset(buffer, 0xff, 2);
	fwrite(buffer, 2, 1, dst);

	/* Make dst pointer point to the next page. */
	fseek(dst, pos + info->page_size + info->oob_size, SEEK_SET);

	return 0;
}

static int create_image(const struct image_info *info)
{
	off_t page = info->offset / info->page_size;
	struct bch_control *bch;
	FILE *src, *dst, *rnd;
	uint8_t *buffer;

	bch = init_bch(14, info->ecc_strength, BCH_PRIMITIVE_POLY);
	if (!bch) {
		fprintf(stderr, "Failed to init the BCH engine\n");
		return -1;
	}

	buffer = malloc(info->page_size + info->oob_size);
	if (!buffer) {
		fprintf(stderr, "Failed to allocate the NAND page buffer\n");
		return -1;
	}

	memset(buffer, 0xff, info->page_size + info->oob_size);

	src = fopen(info->source, "r");
	if (!src) {
		fprintf(stderr, "Failed to open source file (%s)\n",
			info->source);
		return -1;
	}

	dst = fopen(info->dest, "w");
	if (!dst) {
		fprintf(stderr, "Failed to open dest file (%s)\n", info->dest);
		return -1;
	}

	rnd = fopen("/dev/urandom", "r");
	if (!rnd) {
		fprintf(stderr, "Failed to open /dev/urandom\n");
		return -1;
	}

	while (!feof(src)) {
		int ret;

		ret = write_page(info, buffer, src, rnd, dst, bch, page++);
		if (ret)
			return ret;
	}

	return 0;
}

static void display_help(int status)
{
	fprintf(status == EXIT_SUCCESS ? stdout : stderr,
		"sunxi-nand-image-builder %s\n"
		"\n"
		"Usage: sunxi-nand-image-builder [OPTIONS] source-image output-image\n"
		"\n"
		"Creates a raw NAND image that can be read by the sunxi NAND controller.\n"
		"\n"
		"-h               --help               Display this help and exit\n"
		"-c <str>/<step>  --ecc=<str>/<step>   ECC config (strength/step-size)\n"
		"-p <size>        --page=<size>        Page size\n"
		"-o <size>        --oob=<size>         OOB size\n"
		"-u <size>        --usable=<size>      Usable page size\n"
		"-e <size>        --eraseblock=<size>  Erase block size\n"
		"-b               --boot0              Build a boot0 image.\n"
		"-s               --scramble           Scramble data\n"
		"-a <offset>      --address=<offset>   Where the image will be programmed.\n"
		"\n"
		"Notes:\n"
		"All the information you need to pass to this tool should be part of\n"
		"the NAND datasheet.\n"
		"\n"
		"The NAND controller only supports the following ECC configs\n"
		"  Valid ECC strengths: 16, 24, 28, 32, 40, 48, 56, 60 and 64\n"
		"  Valid ECC step size: 512 and 1024\n"
		"\n"
		"If you are building a boot0 image, you'll have specify extra options.\n"
		"These options should be chosen based on the layouts described here:\n"
		"  http://linux-sunxi.org/NAND#More_information_on_BROM_NAND\n"
		"\n"
		"  --usable should be assigned the 'Hardware page' value\n"
		"  --ecc should be assigned the 'ECC capacity'/'ECC page' values\n"
		"  --usable should be smaller than --page\n"
		"\n"
		"The --address option is only required for non-boot0 images that are \n"
		"meant to be programmed at a non eraseblock aligned offset.\n"
		"\n"
		"Examples:\n"
		"  The H27UCG8T2BTR-BC NAND exposes\n"
		"  * 16k pages\n"
		"  * 1280 OOB bytes per page\n"
		"  * 4M eraseblocks\n"
		"  * requires data scrambling\n"
		"  * expects a minimum ECC of 40bits/1024bytes\n"
		"\n"
		"  A normal image can be generated with\n"
		"    sunxi-nand-image-builder -p 16384 -o 1280 -e 0x400000 -s -c 40/1024\n"
		"  A boot0 image can be generated with\n"
		"    sunxi-nand-image-builder -p 16384 -o 1280 -e 0x400000 -s -b -u 4096 -c 64/1024\n",
		PLAIN_VERSION);
	exit(status);
}

static int check_image_info(struct image_info *info)
{
	static int valid_ecc_strengths[] = { 16, 24, 28, 32, 40, 48, 56, 60, 64 };
	int eccbytes, eccsteps;
	unsigned i;

	if (!info->page_size) {
		fprintf(stderr, "--page is missing\n");
		return -EINVAL;
	}

	if (!info->page_size) {
		fprintf(stderr, "--oob is missing\n");
		return -EINVAL;
	}

	if (!info->eraseblock_size) {
		fprintf(stderr, "--eraseblock is missing\n");
		return -EINVAL;
	}

	if (info->ecc_step_size != 512 && info->ecc_step_size != 1024) {
		fprintf(stderr, "Invalid ECC step argument: %d\n",
			info->ecc_step_size);
		return -EINVAL;
	}

	for (i = 0; i < ARRAY_SIZE(valid_ecc_strengths); i++) {
		if (valid_ecc_strengths[i] == info->ecc_strength)
			break;
	}

	if (i == ARRAY_SIZE(valid_ecc_strengths)) {
		fprintf(stderr, "Invalid ECC strength argument: %d\n",
			info->ecc_strength);
		return -EINVAL;
	}

	eccbytes = DIV_ROUND_UP(info->ecc_strength * 14, 8);
	if (eccbytes % 2)
		eccbytes++;
	eccbytes += 4;

	eccsteps = info->usable_page_size / info->ecc_step_size;

	if (info->page_size + info->oob_size <
	    info->usable_page_size + (eccsteps * eccbytes)) {
		fprintf(stderr,
			"ECC bytes do not fit in the NAND page, choose a weaker ECC\n");
		return -EINVAL;
	}

	return 0;
}

int main(int argc, char **argv)
{
	struct image_info info;

	memset(&info, 0, sizeof(info));
	/*
	 * Process user arguments
	 */
	for (;;) {
		int option_index = 0;
		char *endptr = NULL;
		static const struct option long_options[] = {
			{"help", no_argument, 0, 'h'},
			{"ecc", required_argument, 0, 'c'},
			{"page", required_argument, 0, 'p'},
			{"oob", required_argument, 0, 'o'},
			{"usable", required_argument, 0, 'u'},
			{"eraseblock", required_argument, 0, 'e'},
			{"boot0", no_argument, 0, 'b'},
			{"scramble", no_argument, 0, 's'},
			{"address", required_argument, 0, 'a'},
			{0, 0, 0, 0},
		};

		int c = getopt_long(argc, argv, "c:p:o:u:e:ba:sh",
				long_options, &option_index);
		if (c == EOF)
			break;

		switch (c) {
		case 'h':
			display_help(0);
			break;
		case 's':
			info.scramble = 1;
			break;
		case 'c':
			info.ecc_strength = strtol(optarg, &endptr, 0);
			if (*endptr == '/')
				info.ecc_step_size = strtol(endptr + 1, NULL, 0);
			break;
		case 'p':
			info.page_size = strtol(optarg, NULL, 0);
			break;
		case 'o':
			info.oob_size = strtol(optarg, NULL, 0);
			break;
		case 'u':
			info.usable_page_size = strtol(optarg, NULL, 0);
			break;
		case 'e':
			info.eraseblock_size = strtol(optarg, NULL, 0);
			break;
		case 'b':
			info.boot0 = 1;
			break;
		case 'a':
			info.offset = strtoull(optarg, NULL, 0);
			break;
		case '?':
			display_help(-1);
			break;
		}
	}

	if ((argc - optind) != 2)
		display_help(-1);

	info.source = argv[optind];
	info.dest = argv[optind + 1];

	if (!info.boot0) {
		info.usable_page_size = info.page_size;
	} else if (!info.usable_page_size) {
		if (info.page_size > 8192)
			info.usable_page_size = 8192;
		else if (info.page_size > 4096)
			info.usable_page_size = 4096;
		else
			info.usable_page_size = 1024;
	}

	if (check_image_info(&info))
		display_help(-1);

	return create_image(&info);
}
