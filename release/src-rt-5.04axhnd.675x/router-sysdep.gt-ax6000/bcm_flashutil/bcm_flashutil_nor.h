/***********************************************************************
 *
 *  Copyright (c) 2011  Broadcom Corporation
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
 * :>
 ************************************************************************/


#ifndef _BCM_FLASHUTIL_NOR_H__
#define _BCM_FLASHUTIL_NOR_H_

#define SPI_NOR_FLASH_NAME "spi-nor.0"
#define SPI_NOR_ROOTFS_MTD_NAME "rootfs"
#define SPI_NOR_BOOTFS_MTD_NAME "bootfs"
//define spinor uboot mtd partition table
#define SPI_NOR_MTD "mtd0"
#define SPI_NOR_LOADER_MTD "mtd1"
#define SPI_NOR_BOOTFS_MTD "mtd2"
#define SPI_NOR_ROOTFS_MTD "mtd3"
#define SPI_NOR_DATAMTD "mtd4"

int norIsNewFlashLayout(void);
uint64_t spinorGetAvailSpace(const char* mtdpart);
int norWriteFileToMtdPar(const char* filename,const char* mtdpart);
#endif /* _BCM_FLASHUTIL_NAND_H_ */
