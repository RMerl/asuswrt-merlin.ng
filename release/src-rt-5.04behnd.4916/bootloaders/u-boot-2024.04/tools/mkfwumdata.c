// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2023, Linaro Limited
 */

#include <errno.h>
#include <getopt.h>
#include <limits.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <u-boot/crc.h>
#include <unistd.h>
#include <uuid/uuid.h>

/* This will dynamically allocate the fwu_mdata */
#define CONFIG_FWU_NUM_BANKS		0
#define CONFIG_FWU_NUM_IMAGES_PER_BANK	0

/* Since we can not include fwu.h, redefine version here. */
#define FWU_MDATA_VERSION		1

typedef uint8_t u8;
typedef int16_t s16;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

#include <fwu_mdata.h>

/* TODO: Endianness conversion may be required for some arch. */

static const char *opts_short = "b:i:a:p:gh";

static struct option options[] = {
	{"banks", required_argument, NULL, 'b'},
	{"images", required_argument, NULL, 'i'},
	{"guid", required_argument, NULL, 'g'},
	{"active-bank", required_argument, NULL, 'a'},
	{"previous-bank", required_argument, NULL, 'p'},
	{"help", no_argument, NULL, 'h'},
	{NULL, 0, NULL, 0},
};

static void print_usage(void)
{
	fprintf(stderr, "Usage: mkfwumdata [options] <UUIDs list> <output file>\n");
	fprintf(stderr, "Options:\n"
		"\t-i, --images <num>          Number of images (mandatory)\n"
		"\t-b, --banks  <num>          Number of banks (mandatory)\n"
		"\t-a, --active-bank  <num>    Active bank (default=0)\n"
		"\t-p, --previous-bank  <num>  Previous active bank (default=active_bank - 1)\n"
		"\t-g, --guid                  Use GUID instead of UUID\n"
		"\t-h, --help                  print a help message\n"
		);
	fprintf(stderr, "  UUIDs list syntax:\n"
		"\t  <location uuid>,<image type uuid>,<images uuid list>\n"
		"\t     images uuid list syntax:\n"
		"\t        img_uuid_00,img_uuid_01...img_uuid_0b,\n"
		"\t        img_uuid_10,img_uuid_11...img_uuid_1b,\n"
		"\t        ...,\n"
		"\t        img_uuid_i0,img_uuid_i1...img_uuid_ib,\n"
		"\t          where 'b' and 'i' are number of banks and number\n"
		"\t          of images in a bank respectively.\n"
	       );
}

struct fwu_mdata_object {
	size_t images;
	size_t banks;
	size_t size;
	struct fwu_mdata *mdata;
};

static int previous_bank, active_bank;
static bool __use_guid;

static struct fwu_mdata_object *fwu_alloc_mdata(size_t images, size_t banks)
{
	struct fwu_mdata_object *mobj;

	mobj = calloc(1, sizeof(*mobj));
	if (!mobj)
		return NULL;

	mobj->size = sizeof(struct fwu_mdata) +
		(sizeof(struct fwu_image_entry) +
		 sizeof(struct fwu_image_bank_info) * banks) * images;
	mobj->images = images;
	mobj->banks = banks;

	mobj->mdata = calloc(1, mobj->size);
	if (!mobj->mdata) {
		free(mobj);
		return NULL;
	}

	return mobj;
}

static struct fwu_image_entry *
fwu_get_image(struct fwu_mdata_object *mobj, size_t idx)
{
	size_t offset;

	offset = sizeof(struct fwu_mdata) +
		(sizeof(struct fwu_image_entry) +
		 sizeof(struct fwu_image_bank_info) * mobj->banks) * idx;

	return (struct fwu_image_entry *)((char *)mobj->mdata + offset);
}

static struct fwu_image_bank_info *
fwu_get_bank(struct fwu_mdata_object *mobj, size_t img_idx, size_t bnk_idx)
{
	size_t offset;

	offset = sizeof(struct fwu_mdata) +
		(sizeof(struct fwu_image_entry) +
		 sizeof(struct fwu_image_bank_info) * mobj->banks) * img_idx +
		sizeof(struct fwu_image_entry) +
		sizeof(struct fwu_image_bank_info) * bnk_idx;

	return (struct fwu_image_bank_info *)((char *)mobj->mdata + offset);
}

/**
 * convert_uuid_to_guid() - convert UUID to GUID
 * @buf:	UUID binary
 *
 * UUID and GUID have the same data structure, but their binary
 * formats are different due to the endianness. See lib/uuid.c.
 * Since uuid_parse() can handle only UUID, this function must
 * be called to get correct data for GUID when parsing a string.
 *
 * The correct data will be returned in @buf.
 */
static void convert_uuid_to_guid(unsigned char *buf)
{
	unsigned char c;

	c = buf[0];
	buf[0] = buf[3];
	buf[3] = c;
	c = buf[1];
	buf[1] = buf[2];
	buf[2] = c;

	c = buf[4];
	buf[4] = buf[5];
	buf[5] = c;

	c = buf[6];
	buf[6] = buf[7];
	buf[7] = c;
}

static int uuid_guid_parse(char *uuidstr, unsigned char *uuid)
{
	int ret;

	ret = uuid_parse(uuidstr, uuid);
	if (ret < 0)
		return ret;

	if (__use_guid)
		convert_uuid_to_guid(uuid);

	return ret;
}

static int
fwu_parse_fill_image_uuid(struct fwu_mdata_object *mobj,
			  size_t idx, char *uuids)
{
	struct fwu_image_entry *image = fwu_get_image(mobj, idx);
	struct fwu_image_bank_info *bank;
	char *p = uuids, *uuid;
	int i;

	if (!image)
		return -ENOENT;

	/* Image location UUID */
	uuid = strsep(&p, ",");
	if (!uuid)
		return -EINVAL;

	if (strcmp(uuid, "0") &&
	    uuid_guid_parse(uuid, (unsigned char *)&image->location_uuid) < 0)
		return -EINVAL;

	/* Image type UUID */
	uuid = strsep(&p, ",");
	if (!uuid)
		return -EINVAL;

	if (uuid_guid_parse(uuid, (unsigned char *)&image->image_type_uuid) < 0)
		return -EINVAL;

	/* Fill bank image-UUID */
	for (i = 0; i < mobj->banks; i++) {
		bank = fwu_get_bank(mobj, idx, i);
		if (!bank)
			return -ENOENT;
		bank->accepted = 1;
		uuid = strsep(&p, ",");
		if (!uuid)
			return -EINVAL;

		if (strcmp(uuid, "0") &&
		    uuid_guid_parse(uuid, (unsigned char *)&bank->image_uuid) < 0)
			return -EINVAL;
	}
	return 0;
}

/* Caller must ensure that @uuids[] has @mobj->images entries. */
static int fwu_parse_fill_uuids(struct fwu_mdata_object *mobj, char *uuids[])
{
	struct fwu_mdata *mdata = mobj->mdata;
	int i, ret;

	mdata->version = FWU_MDATA_VERSION;
	mdata->active_index = active_bank;
	mdata->previous_active_index = previous_bank;

	for (i = 0; i < mobj->images; i++) {
		ret = fwu_parse_fill_image_uuid(mobj, i, uuids[i]);
		if (ret < 0)
			return ret;
	}

	mdata->crc32 = crc32(0, (const unsigned char *)&mdata->version,
			     mobj->size - sizeof(uint32_t));

	return 0;
}

static int
fwu_make_mdata(size_t images, size_t banks, char *uuids[], char *output)
{
	struct fwu_mdata_object *mobj;
	FILE *file;
	int ret;

	mobj = fwu_alloc_mdata(images, banks);
	if (!mobj)
		return -ENOMEM;

	ret = fwu_parse_fill_uuids(mobj, uuids);
	if (ret < 0)
		goto done_make;

	file = fopen(output, "w");
	if (!file) {
		ret = -errno;
		goto done_make;
	}

	ret = fwrite(mobj->mdata, mobj->size, 1, file);
	if (ret != mobj->size)
		ret = -errno;
	else
		ret = 0;

	fclose(file);

done_make:
	free(mobj->mdata);
	free(mobj);

	return ret;
}

int main(int argc, char *argv[])
{
	unsigned long banks = 0, images = 0;
	int c, ret;

	/* Explicitly initialize defaults */
	active_bank = 0;
	__use_guid = false;
	previous_bank = INT_MAX;

	do {
		c = getopt_long(argc, argv, opts_short, options, NULL);
		switch (c) {
		case 'h':
			print_usage();
			return 0;
		case 'b':
			banks = strtoul(optarg, NULL, 0);
			break;
		case 'i':
			images = strtoul(optarg, NULL, 0);
			break;
		case 'g':
			__use_guid = true;
			break;
		case 'p':
			previous_bank = strtoul(optarg, NULL, 0);
			break;
		case 'a':
			active_bank = strtoul(optarg, NULL, 0);
			break;
		}
	} while (c != -1);

	if (!banks || !images) {
		fprintf(stderr, "Error: The number of banks and images must not be 0.\n");
		return -EINVAL;
	}

	/* This command takes UUIDs * images and output file. */
	if (optind + images + 1 != argc) {
		fprintf(stderr, "Error: UUID list or output file is not specified or too much.\n");
		print_usage();
		return -ERANGE;
	}

	if (previous_bank == INT_MAX) {
		/* set to the earlier bank in round-robin scheme */
		previous_bank = active_bank > 0 ? active_bank - 1 : banks - 1;
	}

	ret = fwu_make_mdata(images, banks, argv + optind, argv[argc - 1]);
	if (ret < 0)
		fprintf(stderr, "Error: Failed to parse and write image: %s\n",
			strerror(-ret));

	return ret;
}
