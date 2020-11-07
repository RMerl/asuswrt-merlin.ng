/*
 * Generic Broadcom Home Networking Division (HND) BME module.
 * This supports the following chips: BCM42xx, 44xx, 47xx .
 *
 * Copyright (C) 2020, Broadcom. All Rights Reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: hndbme.h 999999 2017-01-04 16:02:31Z $
 */

#ifndef _HNDBME_H_
#define _HNDBME_H_

/**
 * Definitions for the flags parameter for function bme_config.
 */
#define BME_CONFIG_FLAG_SRC_COHERENT		(1 << 0)
#define BME_CONFIG_FLAG_DEST_COHERENT		(1 << 1)
#define BME_CONFIG_FLAG_SRC_HOSTMEM		(1 << 2)
#define BME_CONFIG_FLAG_DEST_HOSTMEM		(1 << 3)

/**
 * Channel enum to be used as paramater for bme_attach.
 */
typedef enum bme_channel_e {
	BME_CHANNEL_0 = 0,
	BME_CHANNEL_1 = 1  /** DMA channel 3, with rx/tx status accelerator for 63178/47622 */
} bme_channel_t;

struct bme_info_s;
typedef struct bme_info_s bme_info_t;

/**
 * bme_attach initializes the BME part of the DMA channels. This function is to be called at
 * initialization phase. There will be no checking in the rest of the functions on whether or
 * not this attach call was made. This function initializes some of the BME registers. This
 * function is of the BCMATTACHFN type. So it can only be called during the attach phase.
 *
 * @param[in]	sih
 *
 * @param[in]	channel
 *   Enum, of either BME_CHANNEL_0, or BME_CHANNEL_1. BME_CHANNEL_0 maps to DMA channel 2, while
 *   BME_CHANNEL_1 maps to DMA channel 3. Start from d11 rev 130, BME_CHANNEL_1 has tx/phyrx status
 *   offload support.
 *
 * @returns
 *   bme_info_t * (bme_info): which is either pointer to an opaque structure which caller should
 *   pass on to every bme function or a NULL pointer in case of an error.: BCME_OK for success,
 *   otherwise failed to get stats.
 */
extern bme_info_t *bme_attach(si_t *sih, bme_channel_t channel);

/**
* With bme_detach the BME module can be de-initialized. Should only be called during attach
* phase, to cleanup data. Function is of type BCMATTACHFN.
*
* @param[in]	bme_info
*   Pointer to bme info data as returned by bme_attach. If bme_attach failed and returned NULL
*   then this function should not be called.
*
* @returns
*   <none>
*/
extern void bme_detach(bme_info_t *bme_info);

/**
* bme_config; The BME can transfer data from a 64 bit source address to a 64 bit destination
* address. Most of the time the high part of the source and destination address will not change,
* as will the flags not change. To avoid any unnecessary register programming for each "copy"
* operation the high part of the source and destination address and flags can be configured
* using this function. The actual programming of the hi address registers and flags in HW will
* be performed when there is no copy operation ongoing. If the system is 64 bit then the hi_src
* and hi_dest as configured in this function will be ignored by bme_cpy. If the burst_len changes
* then this function will make sure that any pending copy operation gets completed first.
*
* @param[in]	bme_info
*   Pointer to bme info data as returned by bme_attach.
*
* @param[in]	hi_src
*   The high part of the source address.
*
* @param[in]	hi_src
*   The high part of the destination address.
*
* @param[in]	flags
*   Bit configuration to be used when performing cpy operation. Use bit defintions from header
*   file with name BME_CONFIG_FLAG...
*
* @param[in]	burst_len
*   This field encodes the burst length, in bytes, used by the DMA engines for data read and
*   writes from and to memory. When this field contains the value N, the burst length is
*   2**( N + 4) bytes. This field must not be set higher then 2 (64 bytes).
*
* @returns
*   <none>.
*/
extern void bme_config(bme_info_t *bme_info, uint32 hi_src, uint32 hi_dest, uint32 flags,
	uint8 burst_len);

/**
* bme_cpy; Copy data from source address to destination address. This function will setup a copy
* operation from source address to destination address for len number of bytes. On 64 bit systems
* the source address will be parmeter src. On 32 bit systems the source address will be the
* composed 64 bit address from the parameter src (low part) and the earlier configured hi_src
* (through the function bme_config). The same behvior is valid for the destination address. If
* this function is called while a "previous" copy (bme_cpy or bme_cpy64) operation did not
* complete yet then the function will block (poll) till the previous operation completes and
* then start the new copy operation. The function will return as soon as the hardware has started
* the copy. The copy does not have to (and will likely not) be competed by the time this function
* returns. If an error occurs then this error code will be returned otherwise BCME_OK will be
* returned.
*
* @param[in]	bme_info
*   Pointer to bme info data as returned by bme_attach.
*
* @param[in]	src
*   On a 32 bit system this is the low part of the source address, on a 64 bit system this is
*   the low and hi part of the source address.
*
* @param[in]	dest
*   On a 32 bit system this is the low part of the destination address, on a 64 bit system this
*   is the low and hi part of the destination address.
*
* @param[in]	len
*   The number of bytes to be copied
*
* @returns
*   int (error): Error code. BCME_OK (0) on success.
*/
extern int bme_cpy(bme_info_t *bme_info, const uint8 *src, uint8 *dest, uint16 len);

/**
* bme_cpy64; Copy data from source address to destination address. This function will setup a
* copy operation from source address to destination address for len number of bytes. The source
* and destination address have to be the full 64 bit address. If this function is called on a
* 64 bit system then it will call bme_cpy. On 32 bit systems this function provides a way to
* override the hi_addr as set by bme_config. If this function is called while a "previous" copy
* (bme_cpy or bme_cpy64) operation did not complete yet then the function will block (poll) till
* the previous operation completes and then start the new copy operation. The function will
* return as soon as the hardware has started the copy. The copy does not have to (and will likely
* not) be competed by the time this function returns. If an error occurs then this error code
* will be returned otherwise BCME_OK will be returned.
*
* @param[in]	bme_info
*   Pointer to bme info data as returned by bme_attach.
*
* @param[in]	src
*   The source address in 64 bit.
*
* @param[in]	dest
*   The destination address in 64 bit.
*
* @param[in]	len
*   The number of bytes to be copied
*
* @returns
*   int (error): Error code. BCME_OK (0) on success.
*/
extern int bme_cpy64(bme_info_t *bme_info, uint64 src, uint64 dest, uint16 len);

/**
* bme_completed; This function returns a boolean. When a transfer (copy) has been completed then
* this function returns true. If the copy operation did not complete yet then it will return false.
*
* @param[in]	bme_info
*   Pointer to bme info data as returned by bme_attach.
*
* @returns
*   bool (completed): Return whether or not the transfer completed.
*/
extern bool bme_completed(bme_info_t *bme_info);

/**
* bme_sync; Wait for the current transfer to complete. If the transfer already completed then
* this function returns immediately, otherwise the function will wait for the transfer to complete
* and then return.
*
* @param[in]	bme_info
*   Pointer to bme info data as returned by bme_attach.
*
* @returns
*   <none>.
*/
extern void bme_sync(bme_info_t *bme_info);

#endif /* _HNDBME_H_ */
