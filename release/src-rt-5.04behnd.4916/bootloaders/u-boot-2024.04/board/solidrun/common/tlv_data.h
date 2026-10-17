/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2020 SolidRun
 */

#ifndef __BOARD_SR_COMMON_H_
#define __BOARD_SR_COMMON_H_

struct tlv_data {
	/* Store product name of both SOM and carrier */
	char tlv_product_name[2][32];
	unsigned int ram_size;
	uint8_t ram_channels;
};

void read_tlv_data(struct tlv_data *td);
bool sr_product_is(const struct tlv_data *td, const char *product);

#endif /* __BOARD_SR_COMMON_H_ */
