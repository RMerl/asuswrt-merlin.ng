#if defined(CONFIG_BCM_KF_MIPS_BCM963XX) && defined(CONFIG_MIPS_BCM963XX)
/*
* <:copyright-BRCM:2004:DUAL/GPL:standard
* 
*    Copyright (c) 2004 Broadcom 
*    All Rights Reserved
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
 
*/
/*
 * prom.c: PROM library initialization code.
 *
 */
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/bootmem.h>
#include <linux/blkdev.h>
#include <asm/addrspace.h>
#include <asm/bootinfo.h>
#include <asm/cpu.h>
#include <asm/time.h>

#include <bcm_map_part.h>
#include <bcm_cpu.h>
#include <board.h>
#include <bcm_extirq.h>
#include <boardparms.h>
#if defined(CONFIG_BCM96848)
#include <bcm_otp.h>
#endif
#include <linux/of.h>
#include <linux/of_fdt.h>


extern bool early_init_dt_verify(void *params);
extern void unflatten_and_copy_device_tree(void);
extern int __init bcm_scan_fdt(void);
extern int __init bcm_dt_postinit(void);

void __init device_tree_init(void)
{
    unflatten_and_copy_device_tree();
}


#ifdef CONFIG_BCM_CFE_XARGS_EARLY
extern void __init bl_xparms_setup(const unsigned char* blparms, unsigned int size);
#endif

extern int  do_syslog(int, char *, int);
#ifndef CONFIG_OF
static void __init create_cmdline(char *cmdline);
#endif
UINT32 __init calculateCpuSpeed(void);
void __init retrieve_boot_loader_parameters(void*);



#if defined (CONFIG_BCM963268)
const uint32 cpu_speed_table[0x20] = {
    0, 0, 400, 320, 0, 0, 0, 0, 0, 0, 333, 400, 0, 0, 320, 400,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};
#endif


#if defined (CONFIG_BCM960333)
const uint32 cpu_speed_table[0x04] = {
    200, 400, 333, 0
};
#endif






#if defined (CONFIG_BCM96838)
const uint32 cpu_speed_table[0x3] = {
    600, 400, 240
};
#endif

#if defined (CONFIG_BCM963381)
const uint32 cpu_speed_table[0x04] = {
    300, 800, 480, 600
};
#endif

#if defined (CONFIG_BCM96848)
const uint32 cpu_speed_table[8] = {
    250, 250, 400, 400, 250, 250, 428, 600 
};
#endif

static char promBoardIdStr[NVRAM_BOARD_ID_STRING_LEN];
const char *get_system_type(void)
{
    kerSysNvRamGetBoardId(promBoardIdStr);
    return(promBoardIdStr);
}


/* --------------------------------------------------------------------------
    Name: prom_init
 -------------------------------------------------------------------------- */

#define BOOT_FDT 0 
#define BOOT_LEGACY 1 

int boot_param_status_legacy = BOOT_LEGACY;

extern struct plat_smp_ops brcm_smp_ops;

void __init prom_init(void)
{
    u32 *argv = (u32 *)CKSEG0ADDR(fw_arg1);
     int argc = fw_arg0;
#ifdef CONFIG_OF
    if (early_init_dt_verify((void*)argv)) {
        boot_param_status_legacy = BOOT_FDT;
	of_scan_flat_dt(early_init_dt_scan_root, NULL);
	of_scan_flat_dt(early_init_dt_scan_chosen, boot_command_line);
    	bcm_scan_fdt();
        bcm_dt_postinit();
    } else 
#endif
    if (*argv == BLPARMS_MAGIC) {
	retrieve_boot_loader_parameters(argv+1);
    } else { 
	printk(KERN_CRIT "ERROR: bootloader params are missing\n");
	BUG();
	return;
    }

    kerSysEarlyFlashInit();
    bcm_extirq_init();

    // too early in bootup sequence to acquire spinlock, not needed anyways
    // only the kernel is running at this point
    kerSysNvRamGetBoardIdLocked(promBoardIdStr);
    printk( "%s prom init\n", promBoardIdStr );
    printk(KERN_DEBUG "bootloader args count %u \n", argc );

    PERF->IrqControl[0].IrqMask=0;

#ifdef CONFIG_OF
    strncpy(arcs_cmdline, boot_command_line, COMMAND_LINE_SIZE);
#else
    {
       int i;
       arcs_cmdline[0] = '\0';
       create_cmdline(arcs_cmdline);
       strcat(arcs_cmdline, " ");

       for (i = 1; i < argc; i++) {
            strcat(arcs_cmdline, (char *)CKSEG0ADDR(argv[i]));
            if (i < (argc - 1))
                strcat(arcs_cmdline, " ");
        }
    }
#endif

    /* Count register increments every other clock */
    mips_hpt_frequency = calculateCpuSpeed() / 2;

#if defined (CONFIG_SMP)
    register_smp_ops(&brcm_smp_ops);
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

#ifndef CONFIG_OF
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

#endif /* CONFIG_OF*/
 
/*  *********************************************************************
    *  calculateCpuSpeed()
    *      Calculate the BCM63xx CPU speed by reading the PLL Config register
    *      and applying the following formula:
    *      Fcpu_clk = (25 * MIPSDDR_NDIV) / MIPS_MDIV
    *  Input parameters:
    *      none
    *  Return value:
    *      none
    ********************************************************************* */

#if defined(CONFIG_BCM963268) || defined(CONFIG_BCM963381)
UINT32 __init calculateCpuSpeed(void)
{
    UINT32 mips_pll_fvco;

    mips_pll_fvco = MISC->miscStrapBus & MISC_STRAP_BUS_MIPS_PLL_FVCO_MASK;
    mips_pll_fvco >>= MISC_STRAP_BUS_MIPS_PLL_FVCO_SHIFT;

    return cpu_speed_table[mips_pll_fvco] * 1000000;
}
#endif

#if defined(CONFIG_BCM960333)
UINT32 __init calculateCpuSpeed(void)
{
	UINT32 uiCpuSpeedTableIdx;				// Index into the CPU speed table (0 to 3)
	
	// Get the strapOverrideBus bits to index into teh CPU speed table	
	uiCpuSpeedTableIdx = STRAP->strapOverrideBus & STRAP_BUS_MIPS_FREQ_MASK;
	uiCpuSpeedTableIdx >>= STRAP_BUS_MIPS_FREQ_SHIFT;
    
    return cpu_speed_table[uiCpuSpeedTableIdx] * 1000000;
}
#endif

#if defined(CONFIG_BCM96838)
UINT32 __init calculateCpuSpeed(void)
{ 
#define OTP_SHADOW_BRCM_BITS_0_31               0x40
#define OTP_BRCM_VIPER_FREQ_SHIFT               18
#define OTP_BRCM_VIPER_FREQ_MASK                (0x7 << OTP_BRCM_VIPER_FREQ_SHIFT)

    UINT32 otp_shadow_reg = *((volatile UINT32*)(OTP_BASE+OTP_SHADOW_BRCM_BITS_0_31));
	UINT32 uiCpuSpeedTableIdx = (otp_shadow_reg & OTP_BRCM_VIPER_FREQ_MASK) >> OTP_BRCM_VIPER_FREQ_SHIFT;
	
	return cpu_speed_table[uiCpuSpeedTableIdx] * 1000000;
}
#endif

#if defined(CONFIG_BCM96848)
UINT32 __init calculateCpuSpeed(void)
{
    UINT32 clock_sel_strap = (MISC->miscStrapBus & MISC_STRAP_CLOCK_SEL_MASK) >> MISC_STRAP_CLOCK_SEL_SHIFT;
    UINT32 clock_sel_otp = bcm_otp_get_max_clksel();
 
    if (cpu_speed_table[clock_sel_strap] <= cpu_speed_table[clock_sel_otp])
        return cpu_speed_table[clock_sel_strap] * 1000000;
    else
        return cpu_speed_table[clock_sel_otp] * 1000000;
}
#endif

/* Retrieve a buffer of paramters passed by the boot loader.  Functions in
 * board.c can return requested parameter values to a calling Linux function.
 */
void __init retrieve_boot_loader_parameters(void* bl_parm)
{
    unsigned char *src = (unsigned char *) bl_parm;
    const unsigned char *dst_buf = bcm_get_blparms();
    unsigned int dst_buf_size = bcm_get_blparms_size();
    unsigned char *dst = (unsigned char*)dst_buf;
    unsigned char *dst_end = (unsigned char*)dst + dst_buf_size - 2;

    if (!dst_buf) {
	printk(KERN_ERR "%s:%d Unable to get BCM blparms buffer\n",__func__,__LINE__);
	return;
    }
    do
    {
        *dst++ = *src++;
    } while( (src[0] != '\0' || src[1] != '\0') && (dst < dst_end) );

    dst[0] = dst[1] = '\0';
#ifdef CONFIG_BCM_CFE_XARGS_EARLY
    bl_xparms_setup(dst_buf, dst_buf_size);
#endif
}

#endif // defined(CONFIG_BCM_KF_MIPS_BCM963XX)

