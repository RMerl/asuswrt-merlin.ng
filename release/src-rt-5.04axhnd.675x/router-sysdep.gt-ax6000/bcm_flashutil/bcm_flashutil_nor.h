/***********************************************************************
 *
 *  Copyright (c) 2011  Broadcom Corporation
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
