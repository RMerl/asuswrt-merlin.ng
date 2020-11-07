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

/****************************************************************************
 *
 * AdslFile.c -- Adsl driver file I/O functions
 *
 * Authors: Ilya Stomakhin
 *
 * $Revision: 1.1 $
 *
 * $Id: AdslFile.c,v 1.1 2004/04/14 21:11:59 ilyas Exp $
 *
 * $Log: AdslFile.c,v $
 * Revision 1.1  2004/04/14 21:11:59  ilyas
 * Inial CVS checkin
 *
 ****************************************************************************/

#include <linux/version.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0))
#include <linux/kernel.h>
#include <linux/fcntl.h>
#include <linux/syscalls.h>
#else
#define __KERNEL_SYSCALLS__
#endif
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 33)
#include <generated/autoconf.h>
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,21))
#include <linux/autoconf.h>
#else
#include <linux/config.h>
#endif
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/vmalloc.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/unistd.h>
#include <asm/uaccess.h>

#include "AdslFile.h"
#include "softdsl/EndianUtil.h"

extern void * AdslCoreSetSdramImageAddr(uint lmem2, uint pgSize, uint sdramSize);
extern void	AdslCoreSetXfaceOffset(void *lmemAddr, uint lmemSize);


#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,9))
#ifdef SUPPORT_PHY_BIN_FROM_SDRAM
extern char phyBinAddr[];
#endif

int AdslFileLoadImage(char * fname, void *pAdslLMem, void *pAdslSDRAM)
{
	adslPhyImageHdr		phyHdr;
	struct file  			* fp;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4,0,0))
	mm_segment_t		fs;
#endif
#if 1 && defined(LMEM_ACCESS_WORKAROUND)
	uint	lmemhdr[4];
#endif
	fp = filp_open(fname, O_RDONLY, 0);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,0,0))
	if(IS_ERR(fp)) {
#else
	if (!fp || !fp->f_op || !fp->f_op->read) {
#endif
		printk("Unable to load '%s'.\n", fname);
		return 0;
	}
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,19,0))
   if (kernel_read(fp, (void *)&phyHdr, sizeof(phyHdr), &fp->f_pos) != sizeof(phyHdr)) {
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(4,0,0))
	if (kernel_read(fp, fp->f_pos, (void *)&phyHdr, sizeof(phyHdr)) != sizeof(phyHdr)) {
#else
	fs= get_fs();
	set_fs(get_ds());

	fp->f_pos = 0;
	if (fp->f_op->read(fp, (void *)&phyHdr, sizeof(phyHdr), &fp->f_pos) != sizeof(phyHdr)) {
#endif
		printk("Failed to read image header from '%s'.\n", fname);
		filp_close(fp, NULL);
		return 0;
	}
#ifdef SUPPORT_PHY_BIN_FROM_SDRAM
	memcpy(&phyBinAddr[0], (void *)&phyHdr, sizeof(phyHdr));
#endif
#if defined(__arm) || defined(CONFIG_ARM) || defined(CONFIG_ARM64)
	phyHdr.lmemOffset = ADSL_SWAP_UINT32(phyHdr.lmemOffset);
	phyHdr.lmemSize = ADSL_SWAP_UINT32(phyHdr.lmemSize);
	phyHdr.sdramOffset = ADSL_SWAP_UINT32(phyHdr.sdramOffset);
	phyHdr.sdramSize = ADSL_SWAP_UINT32(phyHdr.sdramSize);
#endif
#if 1 && defined(LMEM_ACCESS_WORKAROUND)
	fp->f_pos = phyHdr.lmemOffset;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,19,0))
   if (kernel_read(fp, (void *)lmemhdr, sizeof(lmemhdr), &fp->f_pos) != sizeof(lmemhdr)) {
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(4,0,0))
	if (kernel_read(fp, fp->f_pos, (void *)lmemhdr, sizeof(lmemhdr)) != sizeof(lmemhdr)) {
#else
	if (fp->f_op->read(fp, (void *)lmemhdr, sizeof(lmemhdr), &fp->f_pos) != sizeof(lmemhdr)) {
#endif
		printk("Failed to read lmemhdr from '%s'.\n", fname);
		filp_close(fp, NULL);
		return 0;
	}
#endif
	fp->f_pos = phyHdr.lmemOffset;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,19,0))
   if (kernel_read(fp, pAdslLMem, phyHdr.lmemSize, &fp->f_pos) != phyHdr.lmemSize) {
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(4,0,0))
	if (kernel_read(fp, fp->f_pos, pAdslLMem, phyHdr.lmemSize) != phyHdr.lmemSize) {
#else
	if (fp->f_op->read(fp, pAdslLMem, phyHdr.lmemSize, &fp->f_pos) != phyHdr.lmemSize) {
#endif
		printk("Failed to read LMEM image from '%s'.\n", fname);
		filp_close(fp, NULL);
		return 0;
	}
#if 1 && defined(LMEM_ACCESS_WORKAROUND)
	printk("lmemhdr[2]=0x%08X, pAdslLMem[2]=0x%08X\n", lmemhdr[2], ((uint *)pAdslLMem)[2]);
	pAdslSDRAM = AdslCoreSetSdramImageAddr(ADSL_ENDIAN_CONV_INT32(lmemhdr[2]), ADSL_ENDIAN_CONV_INT32(lmemhdr[3]), phyHdr.sdramSize);
#else
	pAdslSDRAM = AdslCoreSetSdramImageAddr(ADSL_ENDIAN_CONV_INT32(((uint*)pAdslLMem)[2]), ADSL_ENDIAN_CONV_INT32(((uint*)pAdslLMem)[3]), phyHdr.sdramSize);
#endif
	AdslCoreSetXfaceOffset(pAdslLMem, phyHdr.lmemSize);

	fp->f_pos = phyHdr.sdramOffset;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,19,0))
   if (kernel_read(fp, pAdslSDRAM, phyHdr.sdramSize, &fp->f_pos) != phyHdr.sdramSize) {
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(4,0,0))
	if (kernel_read(fp, fp->f_pos, pAdslSDRAM, phyHdr.sdramSize) != phyHdr.sdramSize) {
#else
	if (fp->f_op->read(fp, pAdslSDRAM, phyHdr.sdramSize, &fp->f_pos) != phyHdr.sdramSize) {
#endif
		printk("Failed to read SDRAM image from '%s'.\n", fname);
		filp_close(fp, NULL);
		return 0;
	}
#ifdef SUPPORT_PHY_BIN_FROM_SDRAM
	printk("%s: lmemOffset=%ld, lmemSize=%ld, sdramOffset=%ld, sdramSize=%ld, sizeof(phyHdr)=%d\n",
		__FUNCTION__, phyHdr.lmemOffset, phyHdr.lmemSize, phyHdr.sdramOffset, phyHdr.sdramSize, sizeof(phyHdr));
	memcpy(&phyBinAddr[phyHdr.lmemOffset], pAdslLMem, phyHdr.lmemSize);
	memcpy(&phyBinAddr[phyHdr.sdramOffset], pAdslSDRAM, phyHdr.sdramSize);
#endif
	filp_close(fp, NULL);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4,0,0))
	set_fs(fs);
#endif
	return phyHdr.lmemSize + phyHdr.sdramSize;
}


#else


#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0))
#define	adslf_open		sys_open
#define	adslf_read		sys_read
#define	adslf_close		sys_close
#define	adslf_lseek		sys_lseek
#else
#define	adslf_open		open
#define	adslf_read		read
#define	adslf_close		close
#define	adslf_lseek		lseek

static int errno;
#endif

int AdslFileLoadImage(char * fname, void *pAdslLMem, void *pAdslSDRAM)
{
	adslPhyImageHdr		phyHdr;
	int			fd;
	mm_segment_t		fs = get_fs();
	set_fs(get_ds());

	fd = adslf_open(fname, 0, 0);
	if (fd == -1) {
		printk("Unable to load '%s'.\n", fname);
		return 0;
	}

	if (adslf_read(fd, (void *)&phyHdr, sizeof(phyHdr)) != sizeof(phyHdr)) {
		printk("Failed to read image header from '%s'.\n", fname);
		adslf_close(fd);
		return 0;
	}

	adslf_lseek(fd, phyHdr.lmemOffset, 0);
	if (adslf_read(fd, pAdslLMem, phyHdr.lmemSize) != phyHdr.lmemSize) {
		printk("Failed to read LMEM image from '%s'.\n", fname);
		adslf_close(fd);
		return 0;
	}

	pAdslSDRAM = AdslCoreSetSdramImageAddr(ADSL_ENDIAN_CONV_INT32(((uint*)pAdslLMem)[2]), ADSL_ENDIAN_CONV_INT32(((uint*)pAdslLMem)[3]), phyHdr.sdramSize);
	AdslCoreSetXfaceOffset(pAdslLMem, phyHdr.lmemSize);
	
	adslf_lseek(fd, phyHdr.sdramOffset, 0);
	if (adslf_read(fd, pAdslSDRAM, phyHdr.sdramSize) != phyHdr.sdramSize) {
		printk("Failed to read SDRAM image from '%s'.\n", fname);
		adslf_close(fd);
		return 0;
	}

#if 0
	printk ("Image Load: LMEM=(0x%lX, %ld,%ld) SDRAM=(0x%lX, %ld,%ld)\n", 
		pAdslLMem, phyHdr.lmemOffset, phyHdr.lmemSize, 
		pAdslSDRAM, phyHdr.sdramOffset, phyHdr.sdramSize);
#endif
	adslf_close(fd);
	set_fs(fs);
	return phyHdr.lmemSize + phyHdr.sdramSize;
}


#endif

