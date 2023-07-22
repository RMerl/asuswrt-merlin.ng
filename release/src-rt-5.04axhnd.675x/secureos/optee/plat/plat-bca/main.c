/*
<:copyright-BRCM:2019:DUAL/GPL:standard

   Copyright (c) 2019 Broadcom 
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

#include <arm.h>
#include <console.h>
#include <drivers/gic.h>
#include <bcm_drivers/bcm_uart.h>
#include <drivers/tzc400.h>
#include <initcall.h>
#include <keep.h>
#include <kernel/generic_boot.h>
#include <kernel/misc.h>
#include <kernel/panic.h>
#include <kernel/pm_stubs.h>
#include <kernel/tee_time.h>
#include <mm/core_memprot.h>
#include <mm/core_mmu.h>
#include <platform_config.h>
#include <sm/psci.h>
#include <stdint.h>
#include <string.h>
#include <tee/entry_fast.h>
#include <tee/entry_std.h>
#include <trace.h>
#include <drivers/pl011.h>
#include <kernel/tee_common_otp.h>

#if defined(ARM32)
#include <arm32.h>
extern void _start(void);
uint32_t nsec_secondary_entry;
#endif

#if defined(PLATFORM_FLAVOR_63138)
#include <platform_support_63138.h>
extern void reset_plat_cpu(void);
#endif

/* NOTE: Following data structure **MUST** match with the one used in CFE (bcm63xx_optee.h) */
#define OPTEE_KEY_SIZE     64
struct optee_boot_param_t
{
  char ddk[OPTEE_KEY_SIZE];      /* Derived device key. Used for OPTEE's file encryption */
  char fek[OPTEE_KEY_SIZE];      /* File-system encryption key. Used for Linux's file system encryption */
  char fek_enc[OPTEE_KEY_SIZE];  /* Encrypted "fek" */
   /* Various key lengths ...    */
  int  ddk_len;
  int  fek_len;
  int  fek_enc_len;
};

struct optee_boot_param_t optee_params;

void get_boot_param(struct optee_boot_param_t *params);
void main_misc_init(void);

extern void powerOn(int cpuNum, void *physBootAddr);

static const struct thread_handlers handlers = {
#if defined(CFG_WITH_ARM_TRUSTED_FW)
	.cpu_on = cpu_on_handler,
#else
	.cpu_on = pm_panic,
#endif
	.cpu_off = pm_do_nothing,
	.cpu_suspend = pm_do_nothing,
	.cpu_resume = pm_do_nothing,
	.system_off = pm_do_nothing,
	.system_reset = pm_do_nothing,
};

static struct gic_data gic_data;
struct bcm_uart_data console_data;
struct pl011_data pl011_console_data;

register_phys_mem(MEM_AREA_IO_SEC, CONSOLE_UART_BASE, BCM_UART_REG_SIZE);
register_phys_mem(MEM_AREA_IO_SEC, BOOT_LOOKUP_BASE, BOOT_LOOKUP_SIZE);
#if defined(ARM32)
register_phys_mem(MEM_AREA_IO_SEC, PMC_PHYS_BASE, PMC_MAP_SIZE);
#endif
register_phys_mem(MEM_AREA_IO_SEC, RANGE_CHK_PHYS_BASE, RANGE_CHK_PHYS_SIZE);
#if defined(CLUSTER_RESET_BASE)
register_phys_mem(MEM_AREA_IO_SEC, CLUSTER_RESET_BASE, CLUSTER_RESET_SIZE);
#endif

#ifdef CFG_PL310
register_phys_mem(MEM_AREA_IO_SEC, PL310_BASE, PL310_MAP_SIZE);
#endif

#ifdef GT_BASE
register_phys_mem(MEM_AREA_IO_SEC, GT_BASE, GT_REG_SIZE);
#endif
register_phys_mem(MEM_AREA_IO_SEC, GIC_BASE, GIC_DIST_REG_SIZE);



const struct thread_handlers *generic_boot_get_handlers(void)
{
	return &handlers;
}

void main_misc_init(void)
{
	volatile vaddr_t range_base;
#if defined(PLATFORM_FLAVOR_63138)
	vaddr_t  gt_base = (vaddr_t)phys_to_virt(GT_BASE, MEM_AREA_IO_SEC) + GT_OFFSET;
	*(uint32_t*)(gt_base + GT_CONTROL_OFFSET) = GT_CONTROL_ENABLE;
#endif

	/* Configure secure memory region */
	range_base = (vaddr_t)phys_to_virt(RANGE_CHK_PHYS_BASE, MEM_AREA_IO_SEC);
	/* Configure secure region for ATF */
	*(uint32_t*)(range_base + RANGE_CHK_BASE_1_OFFSET) = RANGE_CHK_BASE_1_VAL;
	*(uint32_t*)(range_base + RANGE_CHK_CONTROL_1_OFFSET) = RANGE_CHK_CONTROL_SEC_RW;
	*(uint32_t*)(range_base + RANGE_CHK_UBUS_1_LOW_OFFSET) = RANGE_CHK_UBUS_EN;
	*(uint32_t*)(range_base + RANGE_CHK_BASE_2_OFFSET) = RANGE_CHK_BASE_2_VAL;
	*(uint32_t*)(range_base + RANGE_CHK_CONTROL_2_OFFSET) = RANGE_CHK_CONTROL_SEC_RW;
	*(uint32_t*)(range_base + RANGE_CHK_UBUS_2_LOW_OFFSET) = RANGE_CHK_UBUS_EN;
	/* Configure secure region for OPTEE */
	*(uint32_t*)(range_base + RANGE_CHK_BASE_3_OFFSET) = RANGE_CHK_BASE_3_VAL;
	*(uint32_t*)(range_base + RANGE_CHK_CONTROL_3_OFFSET) = RANGE_CHK_CONTROL_SEC_RW;
	*(uint32_t*)(range_base + RANGE_CHK_UBUS_3_LOW_OFFSET) = RANGE_CHK_UBUS_EN;
#if !defined(PLATFORM_FLAVOR_63138)
	*(uint32_t*)(range_base + RANGE_CHK_UBUS_1_UPP_OFFSET) = RANGE_CHK_UBUS_EN;
	*(uint32_t*)(range_base + RANGE_CHK_UBUS_2_UPP_OFFSET) = RANGE_CHK_UBUS_EN;
	*(uint32_t*)(range_base + RANGE_CHK_UBUS_3_UPP_OFFSET) = RANGE_CHK_UBUS_EN;
#endif
	*(uint32_t*)(range_base + RANGE_CHK_LOCK_OFFSET) = RANGE_CHK_LOCK_EN | RANGE_CHK_LOCK(1) | RANGE_CHK_LOCK(2) | RANGE_CHK_LOCK(3);
}


void main_init_gic(void)
{
	vaddr_t gicc_base;
	vaddr_t gicd_base;

	main_misc_init();

#if defined(ARM32)
	write_nsacr(read_nsacr() | NSACR_NS_SMP | NSACR_NS_L2ERR);
#endif

	gicc_base = (vaddr_t)phys_to_virt(GIC_BASE + GICC_OFFSET,
					  MEM_AREA_IO_SEC);
	gicd_base = (vaddr_t)phys_to_virt(GIC_BASE + GICD_OFFSET,
					  MEM_AREA_IO_SEC);
	if (!gicc_base || !gicd_base)
		panic();

	/* GIC configuration is initialized in ARM-TF */
	gic_init_base_addr(&gic_data, gicc_base, gicd_base);
	itr_init(&gic_data.chip);
}

#ifdef CORTEX_A9
uint64_t get_counter_value(void)
{
	uint32_t val, low;
	uint64_t counter;
	volatile vaddr_t gt_base = (vaddr_t)phys_to_virt(GT_BASE, MEM_AREA_IO_SEC) + GT_OFFSET;

	low = *(volatile uint32_t*)(gt_base + GT_COUNTER_LOW_OFFSET);
	val  = *(volatile uint32_t*)(gt_base + GT_COUNTER_HIGH_OFFSET);
	counter = val;
	return counter << 32 | low;
}

uint32_t get_counter_freq(void)
{
	return GT_COUNTER_FREQ;
}
#endif

#if !defined(CFG_WITH_ARM_TRUSTED_FW)
void main_secondary_init_gic(void)
{
	gic_cpu_init(&gic_data);

#if defined(ARM32)
	write_nsacr(read_nsacr() | NSACR_NS_SMP | NSACR_NS_L2ERR);
#endif
}
#endif /* CFG_WITH_ARM_TRUSTED_FW */


static void main_fiq(void)
{
	gic_it_handle(&gic_data);
}

/* Called from the entry code in assembly, to get the boot parameters */
void get_boot_param(struct optee_boot_param_t *params)
{
	memcpy(&optee_params, params, sizeof(optee_params));
}


TEE_Result tee_otp_get_hw_unique_key(struct tee_hw_unique_key *hwkey)
{
	memcpy(&hwkey->data[0], optee_params.ddk, sizeof(hwkey->data));
	return TEE_SUCCESS;
}


void console_init(void)
{
	struct serial_chip *console_chip;
#if defined(PLATFORM_FLAVOR_63138) || defined(PLATFORM_FLAVOR_6856) || defined(PLATFORM_FLAVOR_6858) || defined(PLATFORM_FLAVOR_4908)
	bcm_uart_init(&console_data, CONSOLE_UART_BASE + CONSOLE_UART_OFFSET);
	console_chip = &console_data.chip;
#else
	bcm_pl011_init(&pl011_console_data, CONSOLE_UART_BASE + CONSOLE_UART_OFFSET);
	console_chip = &pl011_console_data.chip;
#endif
	register_serial_console(console_chip);
}

#ifdef IT_CONSOLE_UART
static enum itr_return console_itr_cb(struct itr_handler *h __unused)
{
	struct serial_chip *cons = &console_data.chip;

	while (cons->ops->have_rx_data(cons)) {
		int ch __maybe_unused = cons->ops->getchar(cons);

		DMSG("cpu %zu: got 0x%x", get_core_pos(), ch);
	}
	return ITRR_HANDLED;
}

static struct itr_handler console_itr = {
	.it = IT_CONSOLE_UART,
	.flags = ITRF_TRIGGER_LEVEL,
	.handler = console_itr_cb,
};
KEEP_PAGER(console_itr);

static TEE_Result init_console_itr(void)
{
	itr_add(&console_itr);
	itr_enable(IT_CONSOLE_UART);
	return TEE_SUCCESS;
}
driver_init(init_console_itr);
#endif

#if defined(ARM32)
void init_sec_mon(unsigned long nsec_entry)
{
	struct sm_nsec_ctx *nsec_ctx;

	/* Initialize secure monitor */
	nsec_ctx = sm_get_nsec_ctx();
	nsec_ctx->mon_lr = nsec_entry;
	nsec_ctx->mon_spsr = CPSR_MODE_SVC | CPSR_I | CPSR_F | CPSR_A;
}
#endif

/* Following function is invoked, in response to PSCI_0_2_FN_MEM_INFO request
   to protect Linux kernel's code and static data space from corruption.
 */
int psci_mem_info(uint32_t stext, uint32_t etext, uint32_t sdata);
int psci_mem_info(uint32_t stext, uint32_t etext, uint32_t sdata)
{
	volatile vaddr_t range_base;
	int32_t page_count, size, lock_val;
    if(0){
	range_base = (vaddr_t)phys_to_virt(RANGE_CHK_PHYS_BASE, MEM_AREA_IO_SEC);

	/* Unlock the range checker */
	lock_val = *(uint32_t*)(range_base + RANGE_CHK_LOCK_OFFSET) & ~RANGE_CHK_LOCK_VAL;
	*(uint32_t*)(range_base + RANGE_CHK_LOCK_OFFSET) = lock_val;

	/* Make sure DDR access to first 512K (2^7 * 4096) is always allowed */
	*(uint32_t*)(range_base + RANGE_CHK_BASE_3_OFFSET) = 0x00000000 | 0x7;
	*(uint32_t*)(range_base + RANGE_CHK_CONTROL_3_OFFSET) = RANGE_CHK_CONTROL_ALL_RW;
	*(uint32_t*)(range_base + RANGE_CHK_UBUS_3_LOW_OFFSET) = RANGE_CHK_UBUS_EN;
#if !defined(PLATFORM_FLAVOR_63138)
	*(uint32_t*)(range_base + RANGE_CHK_UBUS_3_UPP_OFFSET) = RANGE_CHK_UBUS_EN;
#endif

	/* Find the total 4k pages that need write restriction */
	page_count = ((etext - stext) + ((1 << 12) - 1)) >> 12;
	/* Convert the total page count in power of two */
	for(size = 0; (1 << size) < page_count; size++);
	*(uint32_t*)(range_base + RANGE_CHK_BASE_2_OFFSET) = (stext & ~0xFFF) | size;
	*(uint32_t*)(range_base + RANGE_CHK_CONTROL_2_OFFSET) = RANGE_CHK_CONTROL_NSEC_RD;
	*(uint32_t*)(range_base + RANGE_CHK_UBUS_2_LOW_OFFSET) = RANGE_CHK_UBUS_EN;
#if !defined(PLATFORM_FLAVOR_63138)
	*(uint32_t*)(range_base + RANGE_CHK_UBUS_2_UPP_OFFSET) = RANGE_CHK_UBUS_EN;
#endif

	/* Find the actual end address covered by the range checker 2 */
	etext = (1 << size) * 4096;
	/* Actual "etext" can overlap data space starting from sdata */
	/* Give full access to overlapped space starting from sdata */
	/* Find the total overlapped page count */
	page_count = ((etext - sdata) + ((1 << 12) - 1)) >> 12;
	/* Convert the total page count in power of two */
	for(size = 0; (1 << size) < page_count; size++);
	*(uint32_t*)(range_base + RANGE_CHK_BASE_4_OFFSET) = (sdata & ~0xFFF) | size;
	*(uint32_t*)(range_base + RANGE_CHK_CONTROL_4_OFFSET) = RANGE_CHK_CONTROL_ALL_RW;
	*(uint32_t*)(range_base + RANGE_CHK_UBUS_4_LOW_OFFSET) = RANGE_CHK_UBUS_EN;
#if !defined(PLATFORM_FLAVOR_63138)
	*(uint32_t*)(range_base + RANGE_CHK_UBUS_4_UPP_OFFSET) = RANGE_CHK_UBUS_EN;
#endif
	/* Lock the ranges */
	lock_val |= RANGE_CHK_LOCK_EN | RANGE_CHK_LOCK(2) | RANGE_CHK_LOCK(3) | RANGE_CHK_LOCK(4);
	*(uint32_t*)(range_base + RANGE_CHK_LOCK_OFFSET) = lock_val;
    }
	return PSCI_RET_SUCCESS;
}
