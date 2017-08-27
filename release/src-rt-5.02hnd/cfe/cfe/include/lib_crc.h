/*
   <:copyright-BRCM:2015:DUAL/GPL:standard
   
      Copyright (c) 2015 Broadcom 
      All Rights Reserved
   
   Unless you and Broadcom execute a separate written software license
   agreement governing use of this software, this software is licensed
   to you under the terms of the GNU General Public License version 2
   (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
   with the following added to such license:
   
      As a special exception, the copyright holders of this software give
      you permission to link this software with independent modules, and
      to copy and distribute the resulting executable under terms of your
      choice, provided that you also meet, for each linked independent
      module, the terms and conditions of the license of that module.
      An independent module is a module which is not derived from this
      software.  The special exception does not apply to any modifications
      of the software.
   
   Not withstanding the above, under no circumstances may you combine
   this software in any way with any other Broadcom software provided
   under a license other than the GPL, without Broadcom's express prior
   written consent.
   
   :> 
*/

/*  *********************************************************************
    *  Broadcom Common Firmware Environment (CFE)
    *
    *  crc and other chksum related routines	File: lib_crc.h
    *  
    *  Author:  Mitch Lichtenberg (mpl@broadcom.com)
    *  
    *  This module contains a very, very, very simple printf
    *  suitable for use in the boot ROM.
    *  
    *********************************************************************  */

#ifndef __LIB_CRC_H__
#define __LIB_CRC_H__

uint32_t lib_get_crc32(unsigned char *pdata, uint32_t size, uint32_t crc);

#define CRC32_INIT_VALUE	0xffffffff /* Initial CRC32 checksum value */
#define CRC_LEN 		4

/*
 * compatibility macros
 */
#define getCrc32 lib_get_crc32

#endif
