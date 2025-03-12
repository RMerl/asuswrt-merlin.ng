
/******************************************************************************

    Copyright 2022 Maxlinear

    SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0-only)

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.
******************************************************************************/
/**
   \file smdio_ssb.h
    GPHY mode flashless load FW example.

*/

#ifndef __SMDIO_SSB_H__
#define __SMDIO_SSB_H__
#include <stdint.h>

/**
 * Read FW file
 */
int ssb_flashless_load(uint8_t lid, char *fw_path);

/**
 * Check Register Status
 */
int check_registers(uint8_t lid);

/**
 * Write data to SSB from offset 0x0
 *
 * pdata - data pointer to be writen to SSB
 * len   - size of data pointer
 *
 * return size of data writen to SB if successful
 */
int smdio_ssb_write(uint8_t lid, uint8_t phy_id, uint8_t *pdata, uint32_t len);

#ifdef SMDIO_TEST_TARGET
/**
 * Read data from SSB offset 0x0
 * data patter is increased by 1 for one uint32_t
 * Maximum is one slice SSB size 64KB
 * It is used for only for testing purpose
 *
 * len - number of uint32_t value to be read
 *
 * return size of data read from SB if successful
 */
int smdio_ssb_read_verify(uint8_t lid, uint8_t phy_id, uint32_t len);
#endif

/**
 * Write data to SSB from offset 0x0
 * data patter is increased by 1 for one uint32_t
 * It is used for only for testing purpose
 *
 * len - number of uint32_t value to be writen
 *
 * return size of data writen to SSB if successful
 */
int smdio_ssb_write_verify(uint8_t lid, uint8_t phy_id, uint32_t len);

#endif /* #define __SMDIO_SSB_H__ */
