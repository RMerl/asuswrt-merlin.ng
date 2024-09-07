/******************************************************************************

   Copyright 2023 MaxLinear, Inc.

   For licensing information, see the file 'LICENSE' in the root folder of
   this software module.

******************************************************************************/

#ifndef _GSW_SS_H_
#define _GSW_SS_H_

#pragma pack(push, 1)
#pragma scalar_storage_order little-endian

struct gsw_ss_sptag {
	/* port ID (1~16) */
	uint8_t pid;
	/* bit value 1 to indicate valid field
	 *   bit 0 - rx
	 *       1 - tx
	 *       2 - rx_pen
	 *       3 - tx_pen
	 */
	uint8_t mask;
	/* RX special tag mode
	 *   value 0 - packet does NOT have special tag and special tag is
	 *             NOT inserted
	 *         1 - packet does NOT have special tag and special tag is
	 *             inserted
	 *         2 - packet has special tag and special tag is NOT inserted
	 */
	uint8_t rx;
	/* TX special tag mode
	 *   value 0 - packet does NOT have special tag and special tag is
	 *             NOT removed
	 *         1 - packet has special tag and special tag is replaced
	 *         2 - packet has special tag and special tag is NOT removed
	 *         3 - packet has special tag and special tag is removed
	 */
	uint8_t tx;
	/* RX special tag info over preamble
	 *   value 0 - special tag info inserted from byte 2 to 7 are all 0
	 *         1 - special tag byte 5 is 16, other bytes from 2 to 7 are 0
	 *         2 - special tag byte 5 is from preamble field, others are 0
	 *         3 - special tag byte 2 to 7 are from preabmle field
	 */
	uint8_t rx_pen;
	/* TX special tag info over preamble
	 *   value 0 - disabled
	 *         1 - enabled
	 */
	uint8_t tx_pen;
};

#pragma scalar_storage_order default
#pragma pack(pop)

int gsw_ss_sptag_get(const GSW_Device_t *dummy, struct gsw_ss_sptag *pdata);
int gsw_ss_sptag_set(const GSW_Device_t *dummy, struct gsw_ss_sptag *pdata);

#endif /*  _GSW_SS_H_ */
