/***********************************************************************
 *
 *  Copyright (c) 2006-2010  Broadcom Corporation
 *  All Rights Reserved
 *
 * <:label-BRCM:2011:DUAL/GPL:standard
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2, as published by
 * the Free Software Foundation (the "GPL").
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * 
 * A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
 * writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 * 
:>
 *
************************************************************************/

#ifndef __GENUTIL_CRC_H__
#define __GENUTIL_CRC_H__

/*!\file genutil_crc.h
 * \brief Header file for CRC calculation functions.
 *
 * Also includes some defines for the CRC in front of config files.
 */


/** fixed length header in front of crc checked config file */
#define CRC_CONFIG_HEADER_LENGTH 20

/** start of the CRC header */
#define CRC_CONFIG_HEADER        "<crc="

/** Initial CRC checksum value, for use in genUtl_getCrc32.
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
unsigned int genUtl_getCrc32(const unsigned char *pdata, unsigned int size, unsigned int crc);

void genUtl_getCrc32Staged(unsigned int stage, unsigned int *crcAccumP, unsigned char *pBuf,
  unsigned int size);


#endif /* __GENUTIL_CRC_H__ */
