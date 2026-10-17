/*
   Copyright (c) 2024 Broadcom Corporation
   All Rights Reserved

   <:label-BRCM:2024:DUAL/GPL:standard

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


#define MERLIN8_PHY1_UCODE_IMAGE_VERSION "1_0_0_rc1"  /* matches the version number from microcode */
#define MERLIN8_PHY1_UCODE_IMAGE_SIZE    31744
#define MERLIN8_PHY1_UCODE_STACK_SIZE    0x00
#define MERLIN8_PHY1_UCODE_IMAGE_CRC     0x4643

extern
unsigned char merlin8_phy1_ucode_image[MERLIN8_PHY1_UCODE_IMAGE_SIZE];

