/***********************************************************************
 *
 *  Copyright (c) 2007  Broadcom Corporation
 *  All Rights Reserved
 *
<:label-BRCM:2012:proprietary:standard

 This program is the proprietary software of Broadcom and/or its
 licensors, and may only be used, duplicated, modified or distributed pursuant
 to the terms and conditions of a separate, written license agreement executed
 between you and Broadcom (an "Authorized License").  Except as set forth in
 an Authorized License, Broadcom grants no license (express or implied), right
 to use, or waiver of any kind with respect to the Software, and Broadcom
 expressly reserves all rights in and to the Software and all intellectual
 property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
 NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
 BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.

 Except as expressly set forth in the Authorized License,

 1. This program, including its structure, sequence and organization,
    constitutes the valuable trade secrets of Broadcom, and you shall use
    all reasonable efforts to protect the confidentiality thereof, and to
    use this information only in connection with your use of Broadcom
    integrated circuit products.

 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
    RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
    ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
    FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
    COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
    TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
    PERFORMANCE OF THE SOFTWARE.

 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
    ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
    INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
    WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
    IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
    OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
    SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
    SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
    LIMITED REMEDY.
:>
 *
 ************************************************************************/
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <asm/byteorder.h>
#include <sys/errno.h>
#include <getopt.h>

#include "ethswctl.h"
#include "ethswctl_api.h"
#include "bcm/bcmswapistat.h"
#include "boardparms.h"

void outputJumboStatus_MIPS(unsigned int regVal)
{
  // Test if port accepts or rejects jumbo frames.
  if ((regVal & ETHSWCTL_JUMBO_PORT_MIPS_MASK) != 0) {
    printf("  MIPS port accepts jumbo frames.\n");
  }
  else {
    printf("  MIPS port rejects jumbo frames.\n");
  }
}


void outputJumboStatus_GPON(unsigned int regVal)
{
  // Test if port accepts or rejects jumbo frames.
  if ((regVal & ETHSWCTL_JUMBO_PORT_GPON_MASK) != 0) {
    printf("  GPON port accepts jumbo frames.\n");
  }
  else {
    printf("  GPON port rejects jumbo frames.\n");
  }
}


void outputJumboStatus_USB(unsigned int regVal)
{
  // Test if port accepts or rejects jumbo frames.
  if ((regVal & ETHSWCTL_JUMBO_PORT_USB_MASK) != 0) {
    printf("  USB port accepts jumbo frames.\n");
  }
  else {
    printf("  USB port rejects jumbo frames.\n");
  }
}


void outputJumboStatus_MOCA(unsigned int regVal)
{
  // Test if port accepts or rejects jumbo frames.
  if ((regVal & ETHSWCTL_JUMBO_PORT_MOCA_MASK) != 0) {
    printf("  MOCA port accepts jumbo frames.\n");
  }
  else {
    printf("  MOCA port rejects jumbo frames.\n");
  }
}


void outputJumboStatus_GPON_SERDES(unsigned int regVal)
{
  // Test if port accepts or rejects jumbo frames.
  if ((regVal & ETHSWCTL_JUMBO_PORT_GPON_SERDES_MASK) != 0) {
    printf("  GPON_SERDES port accepts jumbo frames.\n");
  }
  else {
    printf("  GPON_SERDES port rejects jumbo frames.\n");
  }
}


void outputJumboStatus_GMII_2(unsigned int regVal)
{
  // Test if port accepts or rejects jumbo frames.
  if ((regVal & ETHSWCTL_JUMBO_PORT_GMII_2_MASK) != 0) {
    printf("  GMII_2 port accepts jumbo frames.\n");
  }
  else {
    printf("  GMII_2 port rejects jumbo frames.\n");
  }
}


void outputJumboStatus_GMII_1(unsigned int regVal)
{
  // Test if port accepts or rejects jumbo frames.
  if ((regVal & ETHSWCTL_JUMBO_PORT_GMII_1_MASK) != 0) {
    printf("  GMII_1 port accepts jumbo frames.\n");
  }
  else {
    printf("  GMII_1 port rejects jumbo frames.\n");
  }
}


void outputJumboStatus_GPHY_1(unsigned int regVal)
{
  // Test if port accepts or rejects jumbo frames.
  if ((regVal & ETHSWCTL_JUMBO_PORT_GPHY_1_MASK) != 0) {
    printf("  GPHY_1 port accepts jumbo frames.\n");
  }
  else {
    printf("  GPHY_1 port rejects jumbo frames.\n");
  }
}


void outputJumboStatus_GPHY_0(unsigned int regVal)
{
  // Test if port accepts or rejects jumbo frames.
  if ((regVal & ETHSWCTL_JUMBO_PORT_GPHY_0_MASK) != 0) {
    printf("  GPHY_0 port accepts jumbo frames.\n");
  }
  else {
    printf("  GPHY_0 port rejects jumbo frames.\n");
  }
}


void outputJumboStatus(int portNum, unsigned int regVal)  // bill
{
    // Switch on specified port.
    switch (portNum & ETHSWCTL_JUMBO_PORT_MASK_VAL) {
        case ETHSWCTL_JUMBO_PORT_ALL:
            printf("JUMBO_PORT_MASK:0x%08X\n", regVal);
            outputJumboStatus_GPHY_0(regVal);
            outputJumboStatus_GPHY_1(regVal);
            outputJumboStatus_GMII_1(regVal);
            outputJumboStatus_GMII_2(regVal);
            outputJumboStatus_GPON_SERDES(regVal);
            outputJumboStatus_MOCA(regVal);
            outputJumboStatus_USB(regVal);
            outputJumboStatus_GPON(regVal);
            outputJumboStatus_MIPS(regVal);
            break;
        case ETHSWCTL_JUMBO_PORT_MIPS:
            outputJumboStatus_MIPS(regVal);
            break;
        case ETHSWCTL_JUMBO_PORT_GPON:
            outputJumboStatus_GPON(regVal);
            break;
        case ETHSWCTL_JUMBO_PORT_USB:
            outputJumboStatus_USB(regVal);
            break;
        case ETHSWCTL_JUMBO_PORT_MOCA:
            outputJumboStatus_MOCA(regVal);
            break;
        case ETHSWCTL_JUMBO_PORT_GPON_SERDES:
            outputJumboStatus_GPON_SERDES(regVal);
            break;
        case ETHSWCTL_JUMBO_PORT_GMII_2:
            outputJumboStatus_GMII_2(regVal);
            break;
        case ETHSWCTL_JUMBO_PORT_GMII_1:
            outputJumboStatus_GMII_1(regVal);
            break;
        case ETHSWCTL_JUMBO_PORT_GPHY_1:
            outputJumboStatus_GPHY_1(regVal);
            break;
        case ETHSWCTL_JUMBO_PORT_GPHY_0:
            outputJumboStatus_GPHY_0(regVal);
            break;
    }
}

