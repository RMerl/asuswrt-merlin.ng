// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2016 General Electric Company
 */

#include "vpd_reader.h"
#include <malloc.h>

#include <i2c.h>
#include <linux/bch.h>
#include <stdlib.h>
#include <dm/uclass.h>
#include <i2c_eeprom.h>
#include <hexdump.h>

/* BCH configuration */

const struct {
	int header_ecc_capability_bits;
	int data_ecc_capability_bits;
	unsigned int prim_poly;
	struct {
		int min;
		int max;
	} galois_field_order;
} bch_configuration = {
	.header_ecc_capability_bits = 4,
	.data_ecc_capability_bits = 16,
	.prim_poly = 0,
	.galois_field_order = {
		.min = 5,
		.max = 15,
	},
};

static int calculate_galois_field_order(size_t source_length)
{
	int gfo = bch_configuration.galois_field_order.min;

	for (; gfo < bch_configuration.galois_field_order.max &&
	     ((((1 << gfo) - 1) - ((int)source_length * 8)) < 0);
	     gfo++) {
	}

	if (gfo == bch_configuration.galois_field_order.max)
		return -1;

	return gfo + 1;
}

static int verify_bch(int ecc_bits, unsigned int prim_poly, u8 *data,
		      size_t data_length, const u8 *ecc, size_t ecc_length)
{
	int gfo = calculate_galois_field_order(data_length);

	if (gfo < 0)
		return -1;

	struct bch_control *bch = init_bch(gfo, ecc_bits, prim_poly);

	if (!bch)
		return -1;

	if (bch->ecc_bytes != ecc_length) {
		free_bch(bch);
		return -1;
	}

	unsigned int *errloc = (unsigned int *)calloc(data_length,
						      sizeof(unsigned int));
	int errors = decode_bch(bch, data, data_length, ecc, NULL, NULL,
				errloc);

	free_bch(bch);
	if (errors < 0) {
		free(errloc);
		return -1;
	}

	if (errors > 0) {
		for (int n = 0; n < errors; n++) {
			if (errloc[n] >= 8 * data_length) {
				/*
				 * n-th error located in ecc (no need for data
				 * correction)
				 */
			} else {
				/* n-th error located in data */
				data[errloc[n] / 8] ^= 1 << (errloc[n] % 8);
			}
		}
	}

	free(errloc);
	return 0;
}

static const int ID;
static const int LEN = 1;
static const int VER = 2;
static const int TYP = 3;
static const int BLOCK_SIZE = 4;

static const u8 HEADER_BLOCK_ID;
static const u8 HEADER_BLOCK_LEN = 18;
static const u32 HEADER_BLOCK_MAGIC = 0xca53ca53;
static const size_t HEADER_BLOCK_VERIFY_LEN = 14;
static const size_t HEADER_BLOCK_ECC_OFF = 14;
static const size_t HEADER_BLOCK_ECC_LEN = 4;

static const u8 ECC_BLOCK_ID = 0xFF;

int vpd_reader(size_t size, u8 *data, struct vpd_cache *userdata,
	       int (*fn)(struct vpd_cache *, u8 id, u8 version, u8 type,
			 size_t size, u8 const *data))
{
	if (size < HEADER_BLOCK_LEN || !data || !fn)
		return -EINVAL;

	/*
	 * +--------------------+----------------+--//--+--------------------+
	 * | header block       | data block     | ...  | ecc block          |
	 * +--------------------+----------------+--//--+--------------------+
	 * :                    :                       :
	 * +------+-------+-----+                       +------+-------------+
	 * | id   | magic | ecc |                       | ...  | ecc         |
	 * | len  | off   |     |                       +------+-------------+
	 * | ver  | size  |     |                       :
	 * | type |       |     |                       :
	 * +------+-------+-----+                       :
	 * :              :     :                       :
	 * <----- [1] ---->     <--------- [2] --------->
	 *
	 * Repair (if necessary) the contents of header block [1] by using a
	 * 4 byte ECC located at the end of the header block.  A successful
	 * return value means that we can trust the header.
	 */
	int ret = verify_bch(bch_configuration.header_ecc_capability_bits,
			     bch_configuration.prim_poly, data,
			     HEADER_BLOCK_VERIFY_LEN,
			     &data[HEADER_BLOCK_ECC_OFF], HEADER_BLOCK_ECC_LEN);
	if (ret < 0)
		return ret;

	/* Validate header block { id, length, version, type }. */
	if (data[ID] != HEADER_BLOCK_ID || data[LEN] != HEADER_BLOCK_LEN ||
	    data[VER] != 0 || data[TYP] != 0 ||
	    ntohl(*(u32 *)(&data[4])) != HEADER_BLOCK_MAGIC)
		return -EINVAL;

	u32 offset = ntohl(*(u32 *)(&data[8]));
	u16 size_bits = ntohs(*(u16 *)(&data[12]));

	/* Check that ECC header fits. */
	if (offset + 3 >= size)
		return -EINVAL;

	/* Validate ECC block. */
	u8 *ecc = &data[offset];

	if (ecc[ID] != ECC_BLOCK_ID || ecc[LEN] < BLOCK_SIZE ||
	    ecc[LEN] + offset > size ||
	    ecc[LEN] - BLOCK_SIZE != size_bits / 8 || ecc[VER] != 1 ||
	    ecc[TYP] != 1)
		return -EINVAL;

	/*
	 * Use the header block to locate the ECC block and verify the data
	 * blocks [2] against the ecc block ECC.
	 */
	ret = verify_bch(bch_configuration.data_ecc_capability_bits,
			 bch_configuration.prim_poly, &data[data[LEN]],
			 offset - data[LEN], &data[offset + BLOCK_SIZE],
			 ecc[LEN] - BLOCK_SIZE);
	if (ret < 0)
		return ret;

	/* Stop after ECC.  Ignore possible zero padding. */
	size = offset;

	for (;;) {
		/* Move to next block. */
		size -= data[LEN];
		data += data[LEN];

		if (size == 0) {
			/* Finished iterating through blocks. */
			return 0;
		}

		if (size < BLOCK_SIZE || data[LEN] < BLOCK_SIZE) {
			/* Not enough data for a header, or short header. */
			return -EINVAL;
		}

		ret = fn(userdata, data[ID], data[VER], data[TYP],
			 data[LEN] - BLOCK_SIZE, &data[BLOCK_SIZE]);
		if (ret)
			return ret;
	}
}

int read_i2c_vpd(struct vpd_cache *cache,
		 int (*process_block)(struct vpd_cache *, u8 id, u8 version,
				      u8 type, size_t size, u8 const *data))
{
	struct udevice *dev;
	int ret;
	u8 *data;
	int size;

	ret = uclass_get_device_by_name(UCLASS_I2C_EEPROM, "vpd@0", &dev);
	if (ret)
		return ret;

	size = i2c_eeprom_size(dev);
	if (size < 0) {
		printf("Unable to get size of eeprom: %d\n", ret);
		return ret;
	}

	data = malloc(size);
	if (!data)
		return -ENOMEM;

	ret = i2c_eeprom_read(dev, 0, data, size);
	if (ret) {
		free(data);
		return ret;
	}

	ret = vpd_reader(size, data, cache, process_block);

	free(data);

	return ret;
}
