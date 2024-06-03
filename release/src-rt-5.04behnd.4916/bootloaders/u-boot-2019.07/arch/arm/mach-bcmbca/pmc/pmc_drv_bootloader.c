// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2019 Broadcom
 */
/* SPDX-License-Identifier: GPL-2.0+
 *
 * Copyright 2019 Broadcom Ltd.
 */
/*

*/

/*****************************************************************************
 *  Description:
 *      PMC driver code for bootloaders 
 *****************************************************************************/
#include "pmc_drv.h"
#include "asm/arch/BPCM.h"
#if IS_BCMCHIP(4908)
#include "asm/arch/otp.h"
#endif
#include "command.h"
#include "bca_common.h"
#include "bcm_strap_drv.h"

extern int pmc_mode;

#if IS_BCMCHIP(4908) && !defined _ATF_
int SendCommand(int cmdID, int devAddr, int zone, int island, uint32_t word2,
                uint32_t word3, TCommand * rsp);
#endif

#if defined(PMC_RAM_BOOT)
/* Include the platform specific PMC boot code */
#if IS_BCMCHIP(63178)
#include "pmc_firmware_63178.h"
#elif IS_BCMCHIP(47622)
#include "pmc_firmware_47622.h"
#elif IS_BCMCHIP(6756)
#include "pmc_firmware_6756.h"
#elif IS_BCMCHIP(63158)
#include "pmc_romdata_63158.h"
#elif IS_BCMCHIP(6846)
#include "pmc_firmware_68460.h"
#elif IS_BCMCHIP(6856)
#include "pmc_firmware_68560.h"
#elif IS_BCMCHIP(63146)
#include "pmc_firmware_63146.h"
#elif IS_BCMCHIP(6765)
#include "pmc_firmware_6765.h"
#elif IS_BCMCHIP(4912)
#include "pmc_firmware_4912.h"
#elif IS_BCMCHIP(6813)
#include "pmc_firmware_6813.h"
#undef PMC_VER_CHANGELIST	// in case this macro is different between 6813 and 4916
#include "pmc_firmware_6813_4916.h"
#elif IS_BCMCHIP(6888)
#include "pmc_firmware_6888.h"
#endif


#if IS_BCMCHIP(63158)
// PMC boot options and parameters
typedef struct PMCBootOption {
	unsigned int option;	// Boot option
	unsigned int opt_param;	// Parameter to boot option
} __attribute__ ((packed)) TPMCBootOption;

typedef struct PMCBootParams {
	unsigned int pmc_image_addr;	// PMC image address
	unsigned int pmc_image_size;	// PMC image size
	unsigned int pmc_image_max_size;	// PMC max size
	unsigned int pmc_boot_option_cnt;	// Number of boot options
} __attribute__ ((packed)) TPMCBootParams;

#define LOCATE_BOOT_OPTIONS(a) ((long)(a) + sizeof(TPMCBootParams))

typedef struct PMCBootLog {
	int cfe_rd_idx;		// PMC read index
	int pmc_log_type;	// PMC log type
} __attribute__ ((packed)) TPMCBootLog;

TPMCBootParams *pmcBootParams = NULL;

void set_pmc_boot_param(unsigned int boot_option, unsigned int boot_param)
{
	void *bootParamEnd =
	    (void *)pmcBootParams + sizeof(TPMCBootParams) +
	    (pmcBootParams->pmc_boot_option_cnt + 1) * sizeof(TPMCBootOption);
	TPMCBootOption *bootOptions = NULL;

	// check boot parameters do not corss the boundary
	if ((unsigned long)bootParamEnd >
	    pmcBootParams->pmc_image_addr + pmcBootParams->pmc_image_max_size) {
		printk
		    ("Error: %s : Space not available for PMC boot parameter\n",
		     __func__);
		return;
	}
	// Locate boot option area
	bootOptions = (TPMCBootOption *) LOCATE_BOOT_OPTIONS(pmcBootParams);
	// set boot options
	switch (boot_option) {
	case kPMCBootDefault:
		pmcBootParams->pmc_boot_option_cnt = 0;
		break;
	case kPMCBootAVSDisable:
	case kPMCBootAVSTrackDisable:
	case kPMCBootLogBuffer:
	case kPMCBootLogSize:
		bootOptions[pmcBootParams->pmc_boot_option_cnt].option =
		    boot_option;
		bootOptions[pmcBootParams->pmc_boot_option_cnt].opt_param =
		    boot_param;
		pmcBootParams->pmc_boot_option_cnt++;
		break;
	default:
		printk("Error: %s : Invalid PMC boot option\n", __func__);
	}
}

int get_pmc_boot_param(unsigned int boot_option, unsigned int *boot_param)
{
	TPMCBootOption *bootOptions;
	int i;
	// Locate boot option area
	bootOptions = (TPMCBootOption *) LOCATE_BOOT_OPTIONS(pmcBootParams);

	for (i = 0; pmcBootParams && i < pmcBootParams->pmc_boot_option_cnt;
	     i++) {
		if (bootOptions[i].option == boot_option) {
			*boot_param = bootOptions[i].opt_param;
			return 0;
		}
	}
	return -1;
}

void init_pmc_boot_param(long image_addr, int image_size, int max_size)
{
	// loacate boot parameter location at the end of image.
	pmcBootParams =
	    (TPMCBootParams *) (image_addr + image_size -
				sizeof(TPMCBootParams));
	// Set boot param related global variables
	pmcBootParams->pmc_image_addr = image_addr;
	pmcBootParams->pmc_image_size = image_size;
	pmcBootParams->pmc_image_max_size = max_size;
	// Set default option
	set_pmc_boot_param(kPMCBootDefault, 0);
	printk("Boot pmc_image_addr 0x%08X\n", pmcBootParams->pmc_image_addr);
	printk("Boot pmc_image_size 0x%08X\n", pmcBootParams->pmc_image_size);
	printk("Boot pmc_image_max_size 0x%08X\n",
	       pmcBootParams->pmc_image_max_size);
}
#endif

#ifdef PMC_LOG_IN_DTCM
static void pmc_show_boot_log(void)
{
	int i;

	printk("\n---start of pmc firmware boot log---\n");

	for (i = 0; i < *(unsigned short *) CFG_BOOT_PMC_LOG_ADDR; i++)
		printk("%c", *(char *) (CFG_BOOT_PMC_LOG_ADDR +
					sizeof(unsigned short) + i));

	printk("\n====end of pmc firmware boot log====\n");
}

#endif

void pmc_log(int log_type)
{
#if defined(PMC_SHARED_MEMORY)
	static char cache_buffer[CFG_BOOT_PMC_LOG_SIZE];
	unsigned int log_location = 0;
	unsigned int log_size = 0;
	volatile Pmc *pmc = (volatile Pmc *)g_pmc->pmc_base;

	/* Do log, only if log buffer and log buffer size is known */
	if ((get_pmc_boot_param(kPMCBootLogBuffer, &log_location) != -1) &&
	    (get_pmc_boot_param(kPMCBootLogSize, &log_size) != -1)) {
		TPMCBootLog *log_header = (TPMCBootLog *) PMC_SHARED_MEMORY;
		char *log_buffer = (char *)(uintptr_t)cache_to_uncache(log_location);
		char *format_str = NULL;
		unsigned int value[10];

		/* Let PMC know, CFE wants to read from the begining */
		log_header->cfe_rd_idx = 0;
		log_header->pmc_log_type = log_type;

		/* Keep pumping out the log */
		while (1) {
			int i = 0, fmt_cnt = 0, avl_to_process = 0;
			int pmc_wr, cfe_rd;

			/* Wait until log is available in the buffer */
			do {
				pmc_wr = pmc->ctrl.scratch;
				cfe_rd = log_header->cfe_rd_idx;
				/* Exit if key is pressed */
				if (console_status()) {
					/* Let PMC know that CFE is not interested in reading log */
					log_header->cfe_rd_idx = -1;
					return;
				}
			} while (pmc_wr == cfe_rd);

			avl_to_process =
			    (pmc_wr + log_size - cfe_rd) % log_size;

#ifdef __UBOOT__
			flush_dcache_all();
#endif
			/* Read from the circular log buffer into the cache buffer */
			if (cfe_rd > pmc_wr) {
				/* step 1: Copy from cfe_rd to the end of log_buffer */
				memcpy((void *)cache_buffer,
				       (void *)&log_buffer[cfe_rd],
				       log_size - cfe_rd);
				/* step 2: Copy from the begining of the log_buffer to pmc_wr */
				memcpy((void *)(cache_buffer + log_size -
						cfe_rd), (void *)&log_buffer[0],
				       pmc_wr);
			} else
				/* Copy everything between cfe_rd and pmc_wr */
				memcpy((void *)cache_buffer,
				       (void *)&log_buffer[cfe_rd],
				       pmc_wr - cfe_rd);

			/* Read is done. Let PMC know how far has been read */
			log_header->cfe_rd_idx =
			    (log_header->cfe_rd_idx +
			     avl_to_process) % log_size;

			/* Process all inside the cache_buffer that has been read */
			i = 0;
			while (i < avl_to_process) {
				format_str = &cache_buffer[i];
				/* Scan for format count inside the NULL terminated format string */
				fmt_cnt = 0;
				while (cache_buffer[i]) {
					if (cache_buffer[i] == '%')
						fmt_cnt++;
					i++;
				}
				/* collect the values after the format string, if there is any */
				i++;
				if (fmt_cnt == 0)
					continue;
				memcpy((void *)value, (void *)&cache_buffer[i],
				       sizeof(unsigned int) * fmt_cnt);
				printf(format_str, value[0], value[1], value[2],
				       value[3], value[4], value[5], value[6],
				       value[7], value[8], value[9]);
				/* Look for next format string, inside the cache_buffer */
				i += fmt_cnt * sizeof(unsigned int);
			}
		}
	}
#endif /* PMC_SHARED_MEMORY */
#ifdef PMC_LOG_IN_DTCM
	pmc_show_boot_log();
	pmc_show_live_log();
#endif
}

#if IS_BCMCHIP(6813)
static int is_cpufreq_2p6(void)
{
	u32 vco_strap = bcm_strap_get_field_val("strap-vco-clock");
	return vco_strap == 2;	// strapped CPU freq to 2.6GHz
}
#endif

#ifdef PMC_LOG_IN_DTCM
static const unsigned char * get_pmcdata_ptr(void)
{
#if IS_BCMCHIP(6813)
	if (is_cpufreq_2p6())
		return pmcdata_4916;
#endif
	return pmcdata;
}

static unsigned int get_pmcdata_len(void)
{
#if IS_BCMCHIP(6813)
	if (is_cpufreq_2p6())
		return sizeof(pmcdata_4916);
#endif
	return sizeof(pmcdata);
}
#endif // #ifdef PMC_LOG_IN_DTCM

static const unsigned char * get_pmcappdata_ptr(void)
{
#if IS_BCMCHIP(6813)
	if (is_cpufreq_2p6())
		return pmcappdata_4916;
#endif
	return pmcappdata;
}

static unsigned int get_pmcappdata_len(void)
{
#if IS_BCMCHIP(6813)
	if (is_cpufreq_2p6())
		return sizeof(pmcappdata_4916);
#endif
	return sizeof(pmcappdata);
}

static void pmc_boot(void)
{
#if !defined (PMC_IMPL_3_X)
	volatile unsigned int dummy;
#else
	volatile MaestroMisc *maestro = (volatile MaestroMisc *)g_pmc->maestro_base;
#endif
	unsigned long physAddrPmc;
	unsigned int len;
	const unsigned char *pmccode = NULL;
	void *pmc_log_start = NULL;

#if !defined (PMC_IMPL_3_X) || (defined(PMC_LOG_IN_DTCM) && defined (PMC_IMPL_3_X))
	volatile Pmc *pmc = (volatile Pmc *)g_pmc->pmc_base;
#endif
#ifdef PMC_LOG_IN_DTCM
	volatile uint32_t *dtcm = (uint32_t *) g_pmc->dtcm_base;  
	if (!is_pmcfw_data_loaded()) {
		unsigned int i; 
		const unsigned char *data = get_pmcdata_ptr();

		/* Copy PMC data into DTCM, needed to boot PMC */
		/* Copy to correct area */
		len = get_pmcdata_len();
		for (i = 0; i < len; i += 4)
			dtcm[(i + PMC_DTCM_LOG_SIZE) >> 2] =
			(data[i + 3] << 24) | (data[i + 2] << 16) |
			(data[i + 1] << 8)  | data[i];
	}

	/* set bootlog_len to 0 */
	* (unsigned short *) CFG_BOOT_PMC_LOG_ADDR = 0;
	
	if (getAVSConfig()) {
		printk
		    ("%s: nvram opted to disable AVS, PMC firmware code not loaded\n",
		     __func__);
		return;
	}
#elif defined(CFG_BOOT_PMC3_DATA_ADDR)

	if (!is_pmcfw_data_loaded()) {
		/* Copy PMC data into DTCM, needed to boot PMC */
		/* Copy to correct area */

		len = sizeof(pmcdata);
		memcpy((void *)CFG_BOOT_PMC3_DATA_ADDR, pmcdata, len);
        if(env_get("print_avs_log"))
            pmc_log_start = (void *)CFG_BOOT_PMC3_DATA_ADDR;
    }
#endif

	// copy image to aligned address in pmc boot area
	len = get_pmcappdata_len();
	pmccode = get_pmcappdata_ptr();
#ifdef PMC_FW_IN_ITCM
	physAddrPmc = (unsigned long) g_pmc->itcm_base;
#elif defined(CFG_BOOT_PMC3_START_ADDR)
	physAddrPmc = CFG_BOOT_PMC3_START_ADDR;
#else
	physAddrPmc = CFG_BOOT_PMC_ADDR;
#endif

	if (len > PMC_RESERVED_MEM_SIZE
#ifdef PMC_FW_IN_ITCM
		|| len > g_pmc->itcm_size
#endif
		) {
		printk
		    ("%s: ** ERROR ** PMC image is too big to fit in memory \n",
		     __func__);
		pmc_mode = PMC_MODE_PMB_DIRECT;
		return;
	}

	if (!is_pmcfw_code_loaded())
		memcpy((void *)physAddrPmc, pmccode, len);

#if IS_BCMCHIP(63158)
	pmc_reset();

	init_pmc_boot_param(physAddrPmc, len, CFG_BOOT_PMC_SIZE);
	/* Setup PMC log buffer */
	set_pmc_boot_param(kPMCBootLogBuffer, CFG_BOOT_PMC_LOG_ADDR);
	set_pmc_boot_param(kPMCBootLogSize, CFG_BOOT_PMC_LOG_SIZE);

	if (getAVSConfig() != 0) {
		printk("Info: %s PMC opted to disable AVS\n", __func__);
		set_pmc_boot_param(kPMCBootAVSDisable, 1);
	}
#endif
	/* Boot code and boot params are loaded. Time to flush the content */
#ifdef __UBOOT__
	flush_dcache_all();
	invalidate_icache_all();
#else
	_cfe_flushcache(CFE_CACHE_FLUSH_D, 0, 0);
#endif

#if defined(PMC_IMPL_3_X)
	printf("Take PMC out of reset\n");
	// clear/reset fields of PVTMONRO_ACQ_TEMP_WARN_RESET
	write_bpcm_reg_direct(PMB_ADDR_PVTMON, 0x54 >> 2,
			      (1 << 30) | (1 << 14));
#ifdef PMC_LOG_IN_DTCM
	pmc->ctrl.hostMboxOut = 1; // request sync dtcm log
#endif
#if IS_BCMCHIP(6813)
	if (is_cpufreq_2p6())
		maestro->coreState.usrMbx[0] = 1;
	else
		maestro->coreState.usrMbx[0] = 0;

#endif
#ifdef PMC_FW_IN_ITCM
	maestro->coreCtrl.resetVector = 0;
#else
	maestro->coreCtrl.resetVector = (uint32_t) physAddrPmc;
#endif
	maestro->coreCtrl.coreEnable = 1;
#else /* defined(PMC_IMPL_3_X */
#if IS_BCMCHIP(63158)
	/* open window for the  PMC to see peripheral address space */
	pmc->ctrl.addr2WndwMask = ~((1 << 16) - 1);
	pmc->ctrl.addr2WndwBaseIn = 0x10000000;
	pmc->ctrl.addr2WndwBaseOut = PERF_PHYS_BASE;
	dummy = pmc->ctrl.addr2WndwBaseOut;	// dummy, just for sync
#else
	pmc->ctrl.addr2WndwMask = 0;
	pmc->ctrl.addr2WndwBaseIn = 0;
	pmc->ctrl.addr2WndwBaseOut = 0;
#endif
	/* open window for the  PMC to see DDR */
	pmc->ctrl.addr1WndwMask = ~(PMC_RESERVED_MEM_SIZE - 1);
	pmc->ctrl.addr1WndwBaseIn = 0x1fc00000;
	pmc->ctrl.addr1WndwBaseOut = (uint32_t) physAddrPmc;

	dummy = pmc->ctrl.addr1WndwBaseOut;	// dummy, just for sync
	dummy = dummy;

	printf("Take PMC out of reset\n");
	pmc->ctrl.softResets = 0x0;
#endif

	pmc_mode = PMC_MODE_DQM;

	printf("waiting for PMC finish booting\n");
	WaitPmc(PMC_IN_MAIN_LOOP, pmc_log_start);
#if defined(PMC_IMPL_3_X)
	{
		uint32_t change;
		uint32_t revision;
		if (!GetRevision(&change, &revision)) {
			printf("PMC rev: %d.%d.%d.%d running\n",
			       (revision >> 28) & 0xf, (revision >> 20) & 0xff,
			       (revision & 0xfffff), change);
		}
#ifdef PMC_LOG_IN_DTCM
		pmc->ctrl.hostMboxOut = 0; // ignore dtcm log
		pmc_show_boot_log();
#endif
#if IS_BCMCHIP(6813)
		boost_cpu_clock();
#endif
	}
#endif
}
#endif //defined(PMC_RAM_BOOT)



#if IS_BCMCHIP(4908) && !defined _ATF_
static void pmc_patch_4908(void)
{
	static const
#include "pmc_patch_4908.h"
	    // relocate to end of pmc shared memory
	const unsigned linkaddr = (0xb6004800 - sizeof track_bin) & ~15;
	volatile Pmc *pmc = (volatile Pmc *)g_pmc->pmc_base;
	TCommand rsp;

	memcpy((void *)&pmc->sharedMem[(linkaddr & 0x7ff) / 4],
	       track_bin, sizeof track_bin);

	// register command
	if (SendCommand(cmdRegisterCmdHandler, 0, 0, 0, 96, linkaddr, &rsp)
	    || rsp.word0.Bits.error)
		printk("%s:%d %d\n", __func__,
		       rsp.word0.Bits.cmdID, rsp.word0.Bits.error);
	else {
		// check sec_chipvar and send command
		uint32_t cap = ((uint32_t *) JTAG_OTP_BASE)[12];
		uint32_t slow, fast;	// margin

		if ((cap & 15) != 0) {
			slow = 80, fast = 55;
		} else {
			slow = 100, fast = 100;
		}

		if (SendCommand(96, 0, 0, 0, slow, fast, &rsp)
		    || rsp.word0.Bits.error)
			printk("%s:%d %d %x %x %x\n", __func__,
			       rsp.word0.Bits.cmdID, rsp.word0.Bits.error,
			       rsp.word1.Reg32, rsp.u.cmdResponse.word2,
			       rsp.u.cmdResponse.word3);
	}
}
#endif

#if !defined(CONFIG_SPL_BUILD)
#if defined(CONFIG_SWREG_ADJUSTMENT) || IS_BCMCHIP(63148) || IS_BCMCHIP(63138)

#define SWR_READ_CMD_P 0xB800
#define SWR_WR_CMD_P   0xB400
#define SWR_EN         0x1000
#define SET_ADDR(ps, reg)  (((ps) << 5 | ((reg) & 0x1f)) & 0x2ff)
#define SW_COMP_TIMEOUT 1000

void sw_complete_wait(int err_code, int timeout_count)
{
    volatile SSBMaster *ssb_master; 
 #if defined(PMC_ON_HOSTCPU)
    ssb_master = &((volatile Pmc *)(g_pmc->pmc_base))->ssbMaster;
#else
    ssb_master = &((volatile Procmon *)(g_pmc->procmon_base))->ssbMaster;
#endif
    for(;(((ssb_master->control) & 0x8000) && (timeout_count > 0)) ; timeout_count--)
        ;

    if(!timeout_count) 
        printf("sw_complete_wait: Error code %d timeout !!!", err_code);
  
}

unsigned int swr_read(unsigned int ps, unsigned int reg)
{
    unsigned int cmd = SWR_READ_CMD_P | SET_ADDR(ps, reg);
    volatile SSBMaster *ssb_master; 
#if defined(PMC_ON_HOSTCPU)
    ssb_master = &((volatile Pmc *)(g_pmc->pmc_base))->ssbMaster;
#else
    ssb_master = &((volatile Procmon *)(g_pmc->procmon_base))->ssbMaster;
#endif

    ssb_master->control = SWR_EN;
    ssb_master->control = cmd;
    sw_complete_wait(22, SW_COMP_TIMEOUT);
    return ssb_master->rd_data;
}

void swr_write(unsigned int ps, unsigned int reg, unsigned int val)
{
    unsigned int cmd = 0;
    unsigned int cmd1 = 0;
    unsigned int reg0 = 0;
    volatile SSBMaster *ssb_master; 
#if defined(PMC_ON_HOSTCPU)
    ssb_master = &((volatile Pmc *)(g_pmc->pmc_base))->ssbMaster;
#else
	ssb_master = &((volatile Procmon *)(g_pmc->procmon_base))->ssbMaster;
#endif

   ssb_master->control = SWR_EN;

    if (reg == 0) {
        /* no need read reg0 in case that we write to it , we know wal :) */
        reg0 = val;
    } else {
        /* read reg0 */
        cmd1 = SWR_READ_CMD_P | SET_ADDR(ps, 0);
        ssb_master->control = cmd1;
        sw_complete_wait(1, SW_COMP_TIMEOUT);
        reg0 = ssb_master->rd_data;
    }
    /* write reg */
    ssb_master->wr_data = val;
    cmd = SWR_WR_CMD_P | SET_ADDR(ps, reg);
    ssb_master->control = cmd;
    sw_complete_wait(2, SW_COMP_TIMEOUT);
    /*toggele bit 1 reg0 this load the new regs value */
    cmd1 = SWR_WR_CMD_P | SET_ADDR(ps, 0);
    ssb_master->wr_data = reg0 & ~0x2;
    ssb_master->control = cmd1;
    sw_complete_wait(3, SW_COMP_TIMEOUT);
    ssb_master->wr_data = reg0 | 0x2;
    ssb_master->control = cmd1;
    sw_complete_wait(4, SW_COMP_TIMEOUT);
    ssb_master->wr_data = reg0 & ~0x2;
    ssb_master->control = cmd1;
    sw_complete_wait(5, SW_COMP_TIMEOUT);
}

#if defined(CONFIG_SWREG_ADJUSTMENT)

int swr_read_v(unsigned int ps, unsigned int reg)
{
    if (ps < SWR_FIRST || ps > SWR_LAST)
    {
        printf("Requested SWREG %d is out of range\n", ps);
        return -1;
    }

    if (reg > 9)
    {
        printf("Requested register %d is out of range\n", reg);
        return -1;
    }

    printf("%s, reg=0x%02x, val=0x%04x\n", swreg_names[ps], reg, swr_read(ps, reg));
    return 0;
}

int swr_write_v(unsigned int ps, unsigned int reg, unsigned int val)
{
    if (ps < SWR_FIRST || ps > SWR_LAST)
    {
        printf("Requested SWREG %d is out of range\n", ps);
        return -1;
    }

    if (reg > 9)
    {
        printf("Requested register %d is out of range\n", reg);
        return -1;
    }

    swr_write(ps, reg, val);
    return 0;
}

static int park_not_used_swregs(void)
{
    int i;
    uint32_t chipid = bcmbca_get_chipid(); 
    
    if ((chipid == 0x6753) || (chipid == 0x68252C) || (chipid == 0x68252D))
    {
        /* Turn off all SWREGs for 17X17 package */
        for (i = SWR_FIRST; i < SWR_LAST; i++)
        {
            unsigned int val;
            val = swr_read(i, 5);
            val |= 1;
            swr_write(i, 5, val);

            val = swr_read(i, 0);
            val |= 1;
            swr_write(i, 0, val);
        }
        return 0; /* no dump is needed */
    }
    return 1;
}

void dump_swregs(int ps)
{
    int i, j;

    if (ps == -1)
    {
        printf("Dump Current setting of SWREGs\n");
        i = SWR_FIRST;
        ps = SWR_LAST;
    }
    else if (ps >= SWR_FIRST && ps < SWR_LAST)
    {
        printf("Dump Current setting of SWREG\n");
        i = ps;
        ps++;
    }
    else
    {
        printf("Requested SWREG %d is out of range\n", ps);
        return;
    }

    for (; i < ps; i++)
        for (j = 0; j < 10; j++) 
        {
            printf("%s, reg=0x%02x, val=0x%04x\n", swreg_names[i], j, swr_read(i,j));
        }
}

void list_swregs(void)
{
    int i;
    printf("List of supported SWREGs\n");
    for (i = SWR_FIRST; i < SWR_LAST; i++)
    {
        printf("%d - %s\n", i, swreg_names[i]);
    }
}

#endif
#endif
#endif

#if IS_BCMCHIP(6856)
static void pmc_patch_6856(void)
{
	// Disable force bit on CLKRST BPCM to allow correct pinmuxing
	uint32_t target;

	ReadBPCMRegister(PMB_ADDR_CHIP_CLKRST,
			 CLKRSTBPCMRegOffset(clkrst_control), &target);
	target &= 0xfffbffff;
	WriteBPCMRegister(PMB_ADDR_CHIP_CLKRST,
			  CLKRSTBPCMRegOffset(clkrst_control), target);
}
#endif

#if IS_BCMCHIP(6878)
void swreg_clk_sync(void)
{
	uint32_t target;
	/* Remove the force observe clock to allow of correct pinmuxing of GPIO_7 */

	ReadBPCMRegister(PMB_ADDR_CHIP_CLKRST,
			 CLKRSTBPCMRegOffset(clkrst_ena_force), &target);
	target |= 0xfffffffe;
	WriteBPCMRegister(PMB_ADDR_CHIP_CLKRST,
			  CLKRSTBPCMRegOffset(clkrst_ena_force), target);
}
#endif

int pmc_init(void)
{
	int rc = 0;

	pmc_initmode();

#if !defined(CONFIG_SPL_BUILD)
#if defined(CONFIG_SWREG_ADJUSTMENT)
    if(park_not_used_swregs())
        dump_swregs(-1);
#endif
#endif

#if IS_BCMCHIP(6856)
	pmc_patch_6856();
#endif
#if IS_BCMCHIP(6878)
    swreg_clk_sync();
#endif

#if defined(PMC_RAM_BOOT)
	pmc_boot();
#endif

	printk("%s:PMC using %s mode\n", __func__,
	       pmc_mode == PMC_MODE_PMB_DIRECT ? "PMB_DIRECT" : "DQM");
	if (pmc_mode == PMC_MODE_PMB_DIRECT)
		return 0;

#if IS_BCMCHIP(4908) && !defined _ATF_
	if (getAVSConfig() == 0) {
		pmc_patch_4908();
	} else
		printk("%s:AVS disabled\n", __func__);
#endif

	return rc;
}
