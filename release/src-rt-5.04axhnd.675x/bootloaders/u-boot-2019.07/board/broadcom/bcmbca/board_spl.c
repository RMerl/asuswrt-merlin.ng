/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2019 Broadcom Ltd.
 */

#include <common.h>
#include <fdtdec.h>
#include <linux/ctype.h>
#include <linux/io.h>
#include <spl.h>
#include <nand.h>
#include "spl_ddrinit.h"
#include <asm/arch/misc.h>
#if defined(CONFIG_BCMBCA_DDRC)
#include "asm/arch/ddr.h"
#endif
#include "boot_blob.h"
#include "boot_flash.h"
#include "tpl_params.h"
#include "spl_env.h"
#include "early_abort.h"
#include "bcm_secure.h"

#include <uboot_aes.h>
DECLARE_GLOBAL_DATA_PTR;

tpl_params tplparams;

void spl_board_deinit(void);

static void setup_tpl_parms(tpl_params *parms)
{
	tplparams.environment = NULL;
	/* tplparams.early_flags = boot_params; */
#if defined(CONFIG_BCMBCA_DDRC)
	tplparams.ddr_size = get_ddr_size();
#else
	tplparams.ddr_size = 64*1024*1024;
#endif
	tplparams.boot_device = bcmbca_get_boot_device();
	parms->environment = load_spl_env((void*)TPL_ENV_ADDR);
}
__weak int decrypt_tpl(void* img, u32 len)
{
#if defined(CONFIG_BCMBCA_DECRYPT_TPL)
	if (bcm_sec_state() == SEC_STATE_GEN3_MFG || 
		bcm_sec_state() == SEC_STATE_GEN3_FLD ) {
		u8* key = NULL; 
		u32 num_aes_blocks;
		u8 key_schedule[AES128_EXPAND_KEY_LENGTH];
		bcm_sec_cb_arg_t cb_args[SEC_CTRL_ARG_MAX] = {0};
		cb_args[SEC_CTRL_ARG_KEY].arg[0].ctrl = SEC_CTRL_KEY_GET;
		bcm_sec_do(SEC_SET, cb_args);
		bcm_sec_get_active_aes_key(&key); 
		if (!key) {
			return -1;
		}	
		printf("SPL: Decrypting TPL ...\n");
		aes_expand_key(key, AES128_KEY_LENGTH, key_schedule);
		num_aes_blocks = (len + AES128_KEY_LENGTH - 1) / AES128_KEY_LENGTH;
		aes_cbc_decrypt_blocks(AES128_KEY_LENGTH, key_schedule, 
			key + AES128_KEY_LENGTH, img, img, num_aes_blocks);
	}
#endif
	return 0;
}
/* spl load and start tpl. never return */
__weak void start_tpl(tpl_params *parms)
{
	typedef void __noreturn(*image_entry_t) (void *);
	image_entry_t image_entry =
		(image_entry_t) CONFIG_TPL_TEXT_BASE;
	void *new_params = (void*)TPL_PARAMS_ADDR;
	int size = CONFIG_TPL_MAX_SIZE;

	memcpy(new_params, parms, sizeof(tpl_params));

	if (load_boot_blob(TPL_TABLE_MAGIC, 0x0, (void *)CONFIG_TPL_TEXT_BASE,
		&size) == 0) {
		decrypt_tpl((void *)CONFIG_TPL_TEXT_BASE, size);
		spl_board_deinit();
		image_entry((void *)new_params);
	}

	/* disable mmu/dcache to enable loading of uboot to memory by jtag if needed */
	dcache_disable();
	hang();
}

__weak void arch_cpu_deinit()
{

}

int board_fit_config_name_match(const char *name)
{
	return 0;
}

u32 spl_boot_device(void)
{
	return BOOT_DEVICE_NONE;
}

__weak void enable_memc_sram(void)
{
#if defined(CONFIG_BRCM_SPL_MEMC_SRAM)
	/* enable 64KB sram in MEMC controller for MMU table */
	MEMC->SRAM_REMAP_CTRL = (CONFIG_SYS_PAGETBL_BASE | 0x00000040);
	MEMC->SRAM_REMAP_CTRL |= 0x2;
	MEMC->SRAM_REMAP_CTRL;
#endif
}

__weak void disable_memc_sram(void)
{
#if defined(CONFIG_BRCM_SPL_MEMC_SRAM)
	/* disable 64KB sram in MEMC controller for MMU table */
	MEMC->SRAM_REMAP_CTRL = 0;
	MEMC->SRAM_REMAP_CTRL;
#endif
}

#if !CONFIG_IS_ENABLED(SYS_DCACHE_OFF)
int reserve_mmu(void)
{
	enable_memc_sram();
	gd->arch.tlb_addr = CONFIG_SYS_PAGETBL_BASE;
	gd->arch.tlb_size = CONFIG_SYS_PAGETBL_SIZE;

	return 0;
}
#endif

#ifdef CONFIG_BCMBCA_LDO_TRIM
static void bcmbca_set_ldo_trim(void)
{
	u32 trim = 0;

	bcm_otp_get_ldo_trim(&trim);
	if (trim) {
		printf("Apply trim code 0x%x reg 0x%x from otp to LDO controller...\n", 
			trim, (trim<<LDO_VREG_CTRL_TRIM_SHIFT)&LDO_VREG_CTRL_TRIM_MASK );
		TOPCTRL->LdoCtl =
			(trim<<LDO_VREG_CTRL_TRIM_SHIFT) & LDO_VREG_CTRL_TRIM_MASK;
	}
}
#endif

#if defined(XD4PRO) || defined(XT8PRO) || defined(BM68) || defined(XT8_V2) || defined(ET8PRO) || defined(ET8_V2)
#define AL_AH_REG       0xFF803014
#define SW_INPUT_REG    0xFF803010
#define LED_ACT_CFG     0xFF80301c
#define LED_PPOL        0xFF803018
#define LED_OFF_DELAY      200000
#define LED_START_DELAY    777777

#define LED_R_REG_0	0xff803100	/* GPIO 14 */
#define LED_R_REG_1	0xff803104	/* GPIO 14 */
#define LED_R_REG_2	0xff803108	/* GPIO 14 */
#define LED_R_REG_3	0xff80310c	/* GPIO 14 */
#define LED_G_REG_0	0xff803110	/* GPIO 15 */
#define LED_G_REG_1	0xff803114	/* GPIO 15 */
#define LED_G_REG_2	0xff803118	/* GPIO 15 */
#define LED_G_REG_3	0xff80311c	/* GPIO 15 */
#define LED_B_REG_0	0xff803120	/* GPIO 16 */
#define LED_B_REG_1	0xff803124	/* GPIO 16 */
#define LED_B_REG_2	0xff803128	/* GPIO 16 */
#define LED_B_REG_3	0xff80312c	/* GPIO 16 */

#define LED_OFF_DELAY	200000
#define LED_START_DELAY	777777

// gpio 14, 15, 16, 29(BT) active low nMask
#define LED_N_Bit_MASK	0xDFFE3FFF

/* amber */
/* configure the color which you need by modify LED_x_BRIGHTNESS, 0x00~0x80 */
#define LED_R_BRIGHTNESS    0x80
#define LED_G_BRIGHTNESS    0x80
#define LED_B_BRIGHTNESS    0x80

#define msleep(a) udelay(a * 1000)
static void SetPinmux(uint32_t gpio, uint32_t pinmux)
{
    volatile uint32_t *GPIO_TestPortBlkDataMsb = (void *)(0xff800554);
    volatile uint32_t *GPIO_TestPortBlkDataLsb = (void *)(0xff800558);
    volatile uint32_t *GPIO_TestPortCommand = (void *)(0xff80055c);
    uint32_t val32;

    *GPIO_TestPortBlkDataMsb = 0;
    *GPIO_TestPortBlkDataLsb = (pinmux << 12) | gpio;
    *GPIO_TestPortCommand = 0x21;

    return;
}

#define CLED_CTRL               0xFF803000
#define CLED_HW_EN              0xFF803004
#define CLED_SERIAL_POLARITY    0xFF803054
static void led_init(void)
{
    *(volatile uint32_t *)(CLED_CTRL) &= ~(0x4a);
    *(volatile uint32_t *)(CLED_CTRL) |= 0x4a;
    *(volatile uint32_t *)(CLED_HW_EN) = 0;
    *(volatile uint32_t *)(CLED_SERIAL_POLARITY) = 0;
    *(volatile uint32_t *)(LED_PPOL) = 0;

    return;
}

void PowerCLEDOn(void)
{
    volatile uint32_t *cled_al_ah_reg = (void *)(AL_AH_REG);
    volatile uint32_t *cled_input_reg = (void *)(SW_INPUT_REG);
    volatile uint32_t *cled_led_r_reg = (void *)(LED_R_REG_0);
    volatile uint32_t *cled_led_r_reg_1 = (void *)(LED_R_REG_1);
    volatile uint32_t *cled_led_r_reg_2 = (void *)(LED_R_REG_2);
    volatile uint32_t *cled_led_r_reg_3 = (void *)(LED_R_REG_3);
    volatile uint32_t *cled_led_g_reg = (void *)(LED_G_REG_0);
    volatile uint32_t *cled_led_g_reg_1 = (void *)(LED_G_REG_1);
    volatile uint32_t *cled_led_g_reg_2 = (void *)(LED_G_REG_2);
    volatile uint32_t *cled_led_g_reg_3 = (void *)(LED_G_REG_3);
    volatile uint32_t *cled_led_b_reg = (void *)(LED_B_REG_0);
    volatile uint32_t *cled_led_b_reg_1 = (void *)(LED_B_REG_1);
    volatile uint32_t *cled_led_b_reg_2 = (void *)(LED_B_REG_2);
    volatile uint32_t *cled_led_b_reg_3 = (void *)(LED_B_REG_3);
    volatile uint32_t *cled_act_cfg = (void *)(LED_ACT_CFG);
	// volatile uint32_t *gpio_blk_data_lsb = (void *)(GPIO_TEST_PORT_BLK_DATA_LSB);
	// volatile uint32_t *gpio_command = (void *)(GPIO_TEST_PORT_COMMAND);
    uint32_t val32;

	led_init();
    
    /* Set GPIO 14, 15, 16, 29 pinmux to 2, CLED function */
    SetPinmux(14, 2);
    SetPinmux(15, 2);
    SetPinmux(16, 2);
    SetPinmux(29, 2);
    
    /* set LED14(R), LED15(G), LED16(B) as active low */
    val32 = *cled_al_ah_reg;
    val32 &= LED_N_Bit_MASK;
    *cled_al_ah_reg = val32;

    /* Clear input register */
#if 0
    /* set LED14(R), LED15(G), LED16(B), LED29(BT) ON */
    val32 = *cled_input_reg;
    val32 &= LED_N_Bit_MASK;
    *cled_input_reg = val32;
#else
    *cled_input_reg = 0;
#endif

#if 0
	/* LED off */
	*cled_led_r_reg = 0x0002c000;
	*cled_led_g_reg = 0x0002c000;
	*cled_led_b_reg = 0x0002c000;
	*cled_act_cfg = 0x2001C000;
#endif

#if 0
    /* Turn 14, 15, 16 on */
    *cled_led_r_reg = (LED_R_BRIGHTNESS << 6);
    *cled_led_g_reg = (LED_G_BRIGHTNESS << 6);
    *cled_led_b_reg = (LED_B_BRIGHTNESS << 6);
#else
    /* Turn 14, 15, 16 on */
    *cled_led_r_reg = (LED_R_BRIGHTNESS << 6);
    *cled_led_g_reg = (LED_G_BRIGHTNESS << 6);
    *cled_led_b_reg = (LED_B_BRIGHTNESS << 6);
	msleep(1000);
	/* Red */
	*cled_led_r_reg = 0x0002c000;
	*cled_led_r_reg_1 = 0x00a34a32;
	*cled_led_r_reg_2 = 0x00000c34;
	*cled_led_r_reg_3 = 0x00000000;
	/* Green */
	*cled_led_g_reg = 0x00036002;
	*cled_led_g_reg_1 = 0x00a34a32;
	*cled_led_g_reg_2 = 0x00000c34;
	*cled_led_g_reg_3 = 0x00000000;
	/* Blue */
	*cled_led_b_reg = 0x0002c000;
	*cled_led_b_reg_1 = 0x00a34a32;
	*cled_led_b_reg_2 = 0x00000c34;
	*cled_led_b_reg_3 = 0x00000000;
#endif

    /* active */
	/* GPIO 14, 15, 16, 29 */
    *cled_act_cfg = 0x2001C000;

    return;
}
#endif

void board_init_f(ulong dummy)
{
#if defined(CONFIG_ARCH_CPU_INIT)
	arch_cpu_init();
#endif
#if defined(CONFIG_SYS_ARCH_TIMER)
	timer_init();
#endif
	if (spl_early_init())
		hang();

	/* UART clocks enabled and gd valid - init serial console */
	preloader_console_init();
	printf("Strap register: 0x%x\n", MISC->miscStrapBus);
	if (bcm_otp_init()) {
		hang();
	}

	bcm_sec_init();
#if defined(BUILD_TAG)
	printf("$SPL: "BUILD_TAG" $\n");
#endif
	early_abort();

#ifdef CONFIG_BCMBCA_DDRC_WBF_WAR
	bcm_ddrc_wbf_workaround();
#endif

#if !CONFIG_IS_ENABLED(SYS_DCACHE_OFF)
	reserve_mmu();
	enable_caches();
#endif
#if defined(XD4PRO) || defined(XT8PRO) || defined(BM68) || defined(XT8_V2) || defined(ET8PRO) || defined(ET8_V2)
	PowerCLEDOn();
#endif
}

void spl_board_deinit(void)
{
	/* 
	 * even thought the name says linux but it does everything needed for
	 * boot to the next image: flush and disable cache, disable mmu
	 */
	cleanup_before_linux();

	arch_cpu_deinit();

	disable_memc_sram();
}

void spl_board_init(void)
{
	early_abort_t* ea_info;

#ifdef CONFIG_BCMBCA_LDO_TRIM
	bcmbca_set_ldo_trim();
#endif
	boot_flash_init();

	ea_info = early_abort_info();

#ifdef CONFIG_BCMBCA_EARLY_ABORT_JTAG_UNLOCK
	printf("WARNING -- JTAG UNLOCK IS ENABLED\n");
#endif
#if defined(CONFIG_BCMBCA_DDRC)
	{
		uint32_t mcb_sel = 0,
			mcb_mode = 0;
		if ((ea_info->status&SPL_EA_DDR_MCB_SEL)) {
			mcb_mode = (SPL_DDR_INIT_MCB_OVRD|SPL_DDR_INIT_MCB_SEL);
			mcb_sel = ea_info->data;
		} else if ((ea_info->status&(SPL_EA_DDR3_SAFE_MODE))) {
			mcb_mode = (SPL_DDR_INIT_MCB_OVRD|SPL_DDR_INIT_DDR3_SAFE_MODE);
		} else if ((ea_info->status&SPL_EA_DDR4_SAFE_MODE)) {
			mcb_mode = (SPL_DDR_INIT_MCB_OVRD|SPL_DDR_INIT_DDR4_SAFE_MODE);
		}
		/*printf("\nGot mcb_mode 0x%x\n",mcb_mode);*/
		spl_ddrinit(mcb_mode, mcb_sel);
	}
#endif

	if ((ea_info->status&(SPL_EA_IMAGE_FB))) {
		tplparams.early_flags = SPL_EA_IMAGE_FB;
	}else if ((ea_info->status&SPL_EA_IMAGE_RECOV)) {
		tplparams.early_flags = SPL_EA_IMAGE_RECOV;
	}
	if ((ea_info->status&SPL_EA_IGNORE_BOARDID)) {
		tplparams.early_flags |= SPL_EA_IGNORE_BOARDID;
	}
	/* printf("\nGot TPL flags 0x%x\n",tplparams.early_flags); */
	setup_tpl_parms(&tplparams);

	start_tpl(&tplparams);
}
