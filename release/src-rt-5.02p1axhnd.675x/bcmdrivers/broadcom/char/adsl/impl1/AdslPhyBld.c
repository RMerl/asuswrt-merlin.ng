/* 
* <:copyright-BRCM:2002:proprietary:standard
* 
*    Copyright (c) 2002 Broadcom 
*    All Rights Reserved
* 
*  This program is the proprietary software of Broadcom and/or its
*  licensors, and may only be used, duplicated, modified or distributed pursuant
*  to the terms and conditions of a separate, written license agreement executed
*  between you and Broadcom (an "Authorized License").  Except as set forth in
*  an Authorized License, Broadcom grants no license (express or implied), right
*  to use, or waiver of any kind with respect to the Software, and Broadcom
*  expressly reserves all rights in and to the Software and all intellectual
*  property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
*  NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
*  BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
* 
*  Except as expressly set forth in the Authorized License,
* 
*  1. This program, including its structure, sequence and organization,
*     constitutes the valuable trade secrets of Broadcom, and you shall use
*     all reasonable efforts to protect the confidentiality thereof, and to
*     use this information only in connection with your use of Broadcom
*     integrated circuit products.
* 
*  2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*     AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*     WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
*     RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
*     ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
*     FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
*     COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
*     TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
*     PERFORMANCE OF THE SOFTWARE.
* 
*  3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
*     ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
*     INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
*     WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
*     IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
*     OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
*     SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
*     SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
*     LIMITED REMEDY.
* :>
*/

#include <stdio.h>

#include "../AdslFile.h"

#include "adsl_defs.h"
#include "adsl_lmem.h"
#include "adsl_sdram.h"

#if defined(CONFIG_BCM_DSL_GFASTCOMBO) && !defined(GFAST_SUPPORT)
#error Building GFASTCOMBO with non Gfast PHY
#endif

#if defined(CONFIG_BCM_DSL_GFAST) && !defined(CONFIG_BCM_DSL_GFASTCOMBO) && defined(GFAST_SUPPORT) && defined(VDSL_ADSL)
#error Building GFAST multiPHY with combo PHY
#endif

void AssignBigEndian (int *pLong, int val)
{
	char	*pMem = (char *) pLong;

	pMem[0] = (val >> 24) & 0xFF;
	pMem[1] = (val >> 16) & 0xFF;
	pMem[2] = (val >> 8)  & 0xFF;
	pMem[3] = val & 0xFF;
}

int main(int argc, char *argv[])
{
	FILE				*binFile;
	char				*fname;
	adslPhyImageHdr		phyHdr;
	int					i;

	fname = (argc < 2 ? "adslphy.bin" : argv[1]);
	binFile = fopen(fname, "wb");
	if (binFile == NULL) {
		printf("Failed to open input file %s\n", fname);
		return -1;
	}

#ifdef ADSL_ANNEXC
	phyHdr.imageId[0]	= 'C';
	phyHdr.imageId[1]	= ' ';
#elif defined(ADSL_ANNEXB)
	phyHdr.imageId[0]	= 'B';
	phyHdr.imageId[1]	= ' ';
#elif defined(ADSL_SADSL)
	phyHdr.imageId[0]	= 'S';
	phyHdr.imageId[1]	= 'A';
#else
	phyHdr.imageId[0]	= 'A';
	phyHdr.imageId[1]	= ' ';
#endif
	phyHdr.imageId[2]	= 0;
	phyHdr.imageId[3]	= 0;

	for (i = 0; i < sizeof(phyHdr.reserved); i++)
		phyHdr.reserved[i] = 0;

	AssignBigEndian (&phyHdr.lmemOffset, sizeof(phyHdr));
	AssignBigEndian (&phyHdr.lmemSize, sizeof(adsl_lmem));
	AssignBigEndian (&phyHdr.sdramOffset, sizeof(phyHdr) + sizeof(adsl_lmem));
	AssignBigEndian (&phyHdr.sdramSize, sizeof(adsl_sdram));
	fwrite(&phyHdr, 1, sizeof(phyHdr), binFile);
	fwrite(adsl_lmem, 1, sizeof(adsl_lmem), binFile);
	fwrite(adsl_sdram, 1, sizeof(adsl_sdram), binFile);
	fclose (binFile);
	
	return 0;
}
