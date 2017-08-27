/***********************************************************************
 *
 *  Copyright (c) 2006-2010  Broadcom Corporation
 *  All Rights Reserved
 *
 * <:label-BRCM:2011:DUAL/GPL:standard
 *
 * Unless you and Broadcom execute a separate written software license
 * agreement governing use of this software, this software is licensed
 * to you under the terms of the GNU General Public License version 2
 * (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
 * with the following added to such license:
 *
 *    As a special exception, the copyright holders of this software give
 *    you permission to link this software with independent modules, and
 *    to copy and distribute the resulting executable under terms of your
 *    choice, provided that you also meet, for each linked independent
 *    module, the terms and conditions of the license of that module.
 *    An independent module is a module which is not derived from this
 *    software.  The special exception does not apply to any modifications
 *    of the software.
 *
 * Not withstanding the above, under no circumstances may you combine
 * this software in any way with any other Broadcom software provided
 * under a license other than the GPL, without Broadcom's express prior
 * written consent.
 *
:>
 *
************************************************************************/

#ifndef __BCM_CRC_H__
#define __BCM_CRC_H__

/*!\file cms_crc.h
 * \brief Header file for CRC calculation functions.
 *
 * Also includes some defines for the CRC in front of config files.
 */


/** fixed length header in front of crc checked config file */
#define CRC_CONFIG_HEADER_LENGTH 20

/** start of the CRC header */
#define CRC_CONFIG_HEADER        "<crc="

/** Initial CRC checksum value, for use in cmsCrc_getCrc32.
 *
 * From bcmTag.h.
 */
#define CRC_INITIAL_VALUE 0xffffffff

/** Return the CRC32 value for the given data buffer.
 *
 * @param pdata (IN) data to calculate CRC over.
 * @param len   (IN) length of the data.
 * @param crc   (IN) initial CRC value, see CRC32_INIT_VALUE
 *
 * @return CRC32 value.
 */
unsigned int crc_getCrc32(const unsigned char *pdata, unsigned int size, unsigned int crc);

void crc_getCrc32Staged(unsigned int stage, unsigned int *crcAccumP, unsigned char *pBuf,
  unsigned int size);


#endif

