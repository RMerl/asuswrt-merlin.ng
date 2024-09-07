/******************************************************************************

   Copyright 2023-2024 MaxLinear, Inc.

   For licensing information, see the file 'LICENSE' in the root folder of
   this software module.

******************************************************************************/

#ifndef __HOST_SMDIO_SSB_H__
#define __HOST_SMDIO_SSB_H__
#include <stdint.h>

/**
 * Initialize host_smdio_ssb_ops operation
 */
void host_smdio_ssb_ops_init(const void *pdev);

/**
 * Uninitialize host_smdio_ssb_ops operation
 */
void host_smdio_ssb_ops_uninit();

/**
 * Write image data to target which in rescue mode
 * Image start with 4 bytes of image type
 * followed by 4 bytes of image size and 4 bytes
 * of checksum
 *
 * pdata - data pointer to be writen to SB
 *
 * return size of data writen to target if successful
 */
int host_smdio_ssb_rescue_download(uint8_t *pdata, uint32_t timeout_ms);

/**
 * Read FW file
 */
int ssb_load(char* fw_path);


/**
 * Check Register Status
 */
int check_registers();

#endif
