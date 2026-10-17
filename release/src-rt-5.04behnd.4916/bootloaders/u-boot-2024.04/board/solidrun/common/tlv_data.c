// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2020 SolidRun
 */

#include <common.h>
#include <compiler.h>
#include <tlv_eeprom.h>
#include "tlv_data.h"

#define SR_TLV_CODE_RAM_SIZE	0x81

static void store_product_name(struct tlvinfo_tlv *tlv_entry,
			       struct tlv_data *td)
{
	int len;
	char *dest;

	if (strlen(td->tlv_product_name[0]) == 0)
		dest = td->tlv_product_name[0];
	else if (strlen(td->tlv_product_name[1]) == 0)
		dest = td->tlv_product_name[1];
	else
		return;

	len = min_t(unsigned int, tlv_entry->length,
		    sizeof(td->tlv_product_name[0]) - 1);
	memcpy(dest, tlv_entry->value, len);
}

static void parse_tlv_vendor_ext(struct tlvinfo_tlv *tlv_entry,
				 struct tlv_data *td)
{
	u8 *val = tlv_entry->value;
	u32 pen; /* IANA Private Enterprise Numbers */

	if (tlv_entry->length < 5) /* 4 bytes PEN + at least 1 byte type */
		return;

	/* PEN is big endian */
	pen = (val[0] << 24) | (val[1] << 16) | (val[2] << 8) | val[3];
	/* Not a real PEN */
	if (pen != 0xffffffff)
		return;

	if (val[4] != SR_TLV_CODE_RAM_SIZE)
		return;
	if (tlv_entry->length < 6)
		return;
	td->ram_size = val[5];

	/* extension with additional data field for number of ddr channels */
	if (tlv_entry->length >= 7) {
		td->ram_channels = val[6];
	}
}

static void parse_tlv_data(u8 *eeprom, struct tlvinfo_header *hdr,
			   struct tlvinfo_tlv *entry, struct tlv_data *td)
{
	unsigned int tlv_offset, tlv_len;

	tlv_offset = sizeof(struct tlvinfo_header);
	tlv_len = sizeof(struct tlvinfo_header) + be16_to_cpu(hdr->totallen);
	while (tlv_offset < tlv_len) {
		entry = (struct tlvinfo_tlv *)&eeprom[tlv_offset];

		switch (entry->type) {
		case TLV_CODE_PRODUCT_NAME:
			store_product_name(entry, td);
			break;
		case TLV_CODE_VENDOR_EXT:
			parse_tlv_vendor_ext(entry, td);
			break;
		default:
			break;
		}

		tlv_offset += sizeof(struct tlvinfo_tlv) + entry->length;
	}
}

void read_tlv_data(struct tlv_data *td)
{
	u8 eeprom_data[TLV_TOTAL_LEN_MAX];
	struct tlvinfo_header *tlv_hdr;
	struct tlvinfo_tlv *tlv_entry;
	int ret, i;

	for (i = 0; i < 2; i++) {
		ret = read_tlvinfo_tlv_eeprom(eeprom_data, &tlv_hdr,
					      &tlv_entry, i);
		if (ret < 0)
			continue;
		parse_tlv_data(eeprom_data, tlv_hdr, tlv_entry, td);
	}
}

bool sr_product_is(const struct tlv_data *td, const char *product)
{
	/* Allow prefix sub-string match */
	if (strncmp(td->tlv_product_name[0], product, strlen(product)) == 0)
		return true;
	if (strncmp(td->tlv_product_name[1], product, strlen(product)) == 0)
		return true;

	return false;
}
