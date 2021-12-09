#if defined(CONFIG_BCM_KF_ARM_BCM963XX)
/*
<:copyright-BRCM:2013:DUAL/GPL:standard

   Copyright (c) 2013 Broadcom 
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

// FIXME!! the following ifdef will be redesigned for ARM, at this point,
// it is commented out for compilation purpose.  ARM has different way of
// setting up boot param rather than using PROM library.
/*
 * prom.c: PROM library initialization code.
 */
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/bootmem.h>
#include <linux/blkdev.h>
#include <asm/cpu.h>
#if 0
#include <asm/bootinfo.h>
#include <asm/addrspace.h>
#include <asm/time.h>
#endif

#include <bcm_map_part.h>
#include <bcm_cpu.h>
#include <board.h>
#include <boardparms.h>

// FIXME!! I am just putting this piece of code here.
// It will be needed; however, as for now, we don't use it.
extern int  do_syslog(int, char *, int);

unsigned char g_blparms_buf[1024];

#if 0
static void __init create_cmdline(char *cmdline);
#endif
UINT32 __init calculateCpuSpeed(void);
void __init retrieve_boot_loader_parameters(void);

/* --------------------------------------------------------------------------
    Name: prom_init
 -------------------------------------------------------------------------- */

void __init prom_init(void)
{
#if 0
    int argc = fw_arg0;
    u32 *argv = (u32 *)CKSEG0ADDR(fw_arg1);
    int i;

    kerSysEarlyFlashInit();

    // too early in bootup sequence to acquire spinlock, not needed anyways
    // only the kernel is running at this point
    kerSysNvRamGetBoardIdLocked(promBoardIdStr);
    printk( "%s prom init\n", promBoardIdStr );

    PERF->IrqControl[0].IrqMask=0;

    arcs_cmdline[0] = '\0';

    create_cmdline(arcs_cmdline);

    strcat(arcs_cmdline, " ");

    for (i = 1; i < argc; i++) {
        strcat(arcs_cmdline, (char *)CKSEG0ADDR(argv[i]));
        if (i < (argc - 1))
            strcat(arcs_cmdline, " ");
    }


    /* Count register increments every other clock */
    mips_hpt_frequency = calculateCpuSpeed() / 2;

    retrieve_boot_loader_parameters();
#endif
}


/* --------------------------------------------------------------------------
    Name: prom_free_prom_memory
Abstract: 
 -------------------------------------------------------------------------- */
void __init prom_free_prom_memory(void)
{

}

#if defined(CONFIG_ROOT_NFS) && defined(SUPPORT_SWMDK)
  /* We can't use gendefconfig to automatically fix this, so instead we will
     raise an error here */
  #error "Kernel cannot be configured for both SWITCHMDK and NFS."
#endif

#define HEXDIGIT(d) ((d >= '0' && d <= '9') ? (d - '0') : ((d | 0x20) - 'W'))
#define HEXBYTE(b)  (HEXDIGIT((b)[0]) << 4) + HEXDIGIT((b)[1])

#ifndef CONFIG_ROOT_NFS_DIR
#define CONFIG_ROOT_NFS_DIR	"h:/"
#endif

#ifdef CONFIG_BLK_DEV_RAM_SIZE
#define RAMDISK_SIZE		CONFIG_BLK_DEV_RAM_SIZE
#else
#define RAMDISK_SIZE		0x800000
#endif

/*
 * This function reads in a line that looks something like this from NvRam:
 *
 * CFE bootline=bcmEnet(0,0)host:vmlinux e=192.169.0.100:ffffff00 h=192.169.0.1
 *
 * and retuns in the cmdline parameter based on the boot_type that CFE sets up.
 *
 * for boot from flash, it will use the definition in CONFIG_ROOT_FLASHFS
 *
 * for boot from NFS, it will look like below:
 * CONFIG_CMDLINE="root=/dev/nfs nfsroot=192.168.0.1:/opt/targets/96345R/fs
 * ip=192.168.0.100:192.168.0.1::255.255.255.0::eth0:off rw"
 *
 * for boot from tftp, it will look like below:
 * CONFIG_CMDLINE="root=/dev/ram rw rd_start=0x81000000 rd_size=0x1800000"
 */
#if 0
static void __init create_cmdline(char *cmdline)
{
	char boot_type = '\0', mask[16] = "";
	char bootline[NVRAM_BOOTLINE_LEN] = "";
	char *localip = NULL, *hostip = NULL, *p = bootline, *rdaddr = NULL;

	/*
	 * too early in bootup sequence to acquire spinlock, not needed anyways
	 * only the kernel is running at this point
	 */
	kerSysNvRamGetBootlineLocked(bootline);

	while (*p) {
		if (p[0] == 'e' && p[1] == '=') {
			/* Found local ip address */
			p += 2;
			localip = p;
			while (*p && *p != ' ' && *p != ':')
				p++;
			if (*p == ':') {
				/* Found network mask (eg FFFFFF00 */
				*p++ = '\0';
				sprintf(mask, "%u.%u.%u.%u", HEXBYTE(p),
					HEXBYTE(p + 2),
				HEXBYTE(p + 4), HEXBYTE(p + 6));
				p += 4;
			} else if (*p == ' ')
				*p++ = '\0';
		} else if (p[0] == 'h' && p[1] == '=') {
			/* Found host ip address */
			p += 2;
			hostip = p;
			while (*p && *p != ' ')
				p++;
			if (*p == ' ')
				*p++ = '\0';
		} else if (p[0] == 'r' && p[1] == '=') {
			/* Found boot type */
			p += 2;
			boot_type = *p;
			while (*p && *p != ' ')
				p++;
			if (*p == ' ')
				*p++ = '\0';
		} else if (p[0] == 'a' && p[1] == '=') {
			p += 2;
			rdaddr = p;
			while (*p && *p != ' ')
				p++;
			if (*p == ' ')
				*p++ = '\0';
		} else 
			p++;
	}

	if (boot_type == 'h' && localip && hostip) {
		/* Boot from NFS with proper IP addresses */
		sprintf(cmdline, "root=/dev/nfs nfsroot=%s:" CONFIG_ROOT_NFS_DIR
				" ip=%s:%s::%s::eth0:off rw",
				hostip, localip, hostip, mask);
	} else if (boot_type == 'c') {
		/* boot from tftp */
		sprintf(cmdline, "root=/dev/ram0 ro rd_start=%s rd_size=0x%x",
				rdaddr, RAMDISK_SIZE << 10);
	} else {
		/* go with the default, boot from flash */
#ifdef CONFIG_ROOT_FLASHFS
		strcpy(cmdline, CONFIG_ROOT_FLASHFS);
#endif
	}
}
#endif

/* Retrieve a buffer of paramters passed by the boot loader.  Functions in
 * board.c can return requested parameter values to a calling Linux function.
 */
void __init retrieve_boot_loader_parameters(void)
{
#if 0
    extern unsigned char _text;
    unsigned long blparms_magic = *(unsigned long *) (&_text - 8);
    unsigned long blparms_buf = *(unsigned long *) (&_text - 4);
    unsigned char *src = (unsigned char *) blparms_buf;
    unsigned char *dst = g_blparms_buf;

    if( blparms_magic != BLPARMS_MAGIC )
    {
        /* Subtract four more bytes for NAND flash images. */
        blparms_magic = *(unsigned long *) (&_text - 12);
        blparms_buf = *(unsigned long *) (&_text - 8);
        src = (unsigned char *) blparms_buf;
    }

    if( blparms_magic == BLPARMS_MAGIC )
    {
        do
        {
            *dst++ = *src++;
        } while( (src[0] != '\0' || src[1] != '\0') &&
          (unsigned long) (dst - g_blparms_buf) < sizeof(g_blparms_buf) - 2);
    }

    dst[0] = dst[1] = '\0';
#endif
}
#endif /* CONFIG_BCM_KF_ARM_BCM963XX */
