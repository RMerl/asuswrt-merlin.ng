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
#include "boot_blob.h"
#include "boot_flash.h"
#include "tpl_params.h"
#include "spl_env.h"
#include "early_abort.h"
#include "bcm_secure.h"
#include "bca_common.h"
#if defined(CONFIG_BCMBCA_OTP) 
#include "bcm_otp.h"
#endif
#include "pmc_drv.h"
#if defined(CONFIG_SPL_BCMBCA_UBUS4)
#include "bcm_ubus4.h"
#endif
#include "bcm_strap_drv.h"

#include <wdt.h>
#include <uboot_aes.h>
DECLARE_GLOBAL_DATA_PTR;

tpl_params tplparams;

void spl_board_deinit(void);
extern void jump_to_image(uintptr_t entry, void* param, int clean_cache);

static void setup_tpl_parms(tpl_params *parms)
{
	parms->environment = NULL;
	/* tplparams.early_flags = boot_params; */
#if defined(CONFIG_BCMBCA_DDRC)
	parms->ddr_size = get_ddr_size();
#else
	parms->ddr_size = 64*1024*1024;
#endif
	parms->boot_device = (u8)bcm_get_boot_device();
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

#if !defined(CONFIG_BCMBCA_IKOS) 
	bcm_sec_abort();		
#endif
}

__weak void arch_cpu_deinit(void)
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

#if !CONFIG_IS_ENABLED(SYS_DCACHE_OFF)
int reserve_mmu(void)
{
#ifdef CONFIG_BCMBCA_PGTBL_IN_MEMC_SRAM
	bcmbca_enable_memc_sram(CONFIG_SYS_PAGETBL_BASE, CONFIG_SYS_PAGETBL_SIZE);
#endif
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
		TOPCTRL->LdoCtl &= ~LDO_VREG_CTRL_TRIM_MASK;	
		TOPCTRL->LdoCtl |=
			(trim<<LDO_VREG_CTRL_TRIM_SHIFT) & LDO_VREG_CTRL_TRIM_MASK;
	}
}
#endif

__weak void bcm_setsw(void)
{
}

__weak void reset_plls(void)
{
}

#if defined(XD4PRO) || defined(XT8PRO) || defined(BT12) || defined(BT10) || defined(BQ16) || defined(BQ16_PRO) || defined(BM68) || defined(XT8_V2) || defined(ET8PRO) || defined(ET8_V2)
#define AL_AH_REG       0xFF803014
#define SW_INPUT_REG    0xFF803010
#define LED_ACT_CFG     0xFF80301c
#define LED_PPOL        0xFF803018
#define LED_OFF_DELAY      200000
#define LED_START_DELAY    777777

#if defined(BQ16) || defined(BQ16_PRO)
#define LED_R_REG_0	0xff803140	/* LED18 */
#define LED_R_REG_1	0xff803144	/* LED18 */
#define LED_R_REG_2	0xff803148	/* LED18 */
#define LED_R_REG_3	0xff80314c	/* LED18 */
#define LED_G_REG_0	0xff803150	/* LED19 */
#define LED_G_REG_1	0xff803154	/* LED19 */
#define LED_G_REG_2	0xff803158	/* LED19 */
#define LED_G_REG_3	0xff80315c	/* LED19 */
#define LED_B_REG_0	0xff803160	/* LED20 */
#define LED_B_REG_1	0xff803164	/* LED20 */
#define LED_B_REG_2	0xff803168	/* LED20 */
#define LED_B_REG_3	0xff80316c	/* LED20 */
#define LED_W_REG_0	0xff8031a0	/* LED24 */
#define LED_W_REG_1	0xff8031a4	/* LED24 */
#define LED_W_REG_2	0xff8031a8	/* LED24 */
#define LED_W_REG_3	0xff8031ac	/* LED24 */
#elif defined(BT10)
#define LED_R_REG_0	0xff803150	/* LED19 */
#define LED_R_REG_1	0xff803154	/* LED19 */
#define LED_R_REG_2	0xff803158	/* LED19 */
#define LED_R_REG_3	0xff80315c	/* LED19 */
#define LED_G_REG_0	0xff803170	/* LED21 */
#define LED_G_REG_1	0xff803174	/* LED21 */
#define LED_G_REG_2	0xff803178	/* LED21 */
#define LED_G_REG_3	0xff80317c	/* LED21 */
#define LED_B_REG_0	0xff803060	/* LED4 */
#define LED_B_REG_1	0xff803064	/* LED4 */
#define LED_B_REG_2	0xff803068	/* LED4 */
#define LED_B_REG_3	0xff80306c	/* LED4 */
#else
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
#endif

#define LED_OFF_DELAY	200000
#define LED_START_DELAY	777777

#if defined(BQ16) || defined(BQ16_PRO)
// led 18, 19, 20, 24 active low nMask
#define LED_N_Bit_MASK	0xfee3ffff
#elif defined(BT10)
// led 19, 21, 4 active low nMask
#define LED_N_Bit_MASK	0xFFD7FFEF
#else
// gpio 14, 15, 16, 29(BT) active low nMask
#define LED_N_Bit_MASK	0xDFFE3FFF
#endif

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
#if defined(BQ16) || defined(BQ16_PRO)
	/* LED 18, 19, 20, 24 */
    *(volatile uint32_t *)(LED_PPOL) = 0x011C0000;
#elif defined(BT10)
	/* LED 19, 21, 4 */
    *(volatile uint32_t *)(LED_PPOL) = 0x00280010;
#else
    *(volatile uint32_t *)(LED_PPOL) = 0;
#endif

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
#if defined(BQ16) || defined(BQ16_PRO)
	/* LED 24 */
    volatile uint32_t *cled_led_w_reg = (void *)(LED_W_REG_0);
    volatile uint32_t *cled_led_w_reg_1 = (void *)(LED_W_REG_1);
    volatile uint32_t *cled_led_w_reg_2 = (void *)(LED_W_REG_2);
    volatile uint32_t *cled_led_w_reg_3 = (void *)(LED_W_REG_3);
#endif
    volatile uint32_t *cled_act_cfg = (void *)(LED_ACT_CFG);
	// volatile uint32_t *gpio_blk_data_lsb = (void *)(GPIO_TEST_PORT_BLK_DATA_LSB);
	// volatile uint32_t *gpio_command = (void *)(GPIO_TEST_PORT_COMMAND);
    uint32_t val32;

	led_init();
    
#if defined(BQ16) || defined(BQ16_PRO)
    /* Set GPIO 44, 45, 46, 50 pinmux to 2, CLED function */
    SetPinmux(44, 2);
    SetPinmux(45, 2);
    SetPinmux(46, 2);
    SetPinmux(50, 2);
#elif defined(BT10)
    /* Set GPIO 19, 21, 37 pinmux to 2, CLED function */
    SetPinmux(19, 2);
    SetPinmux(21, 2);
    SetPinmux(37, 2);
#else
    /* Set GPIO 14, 15, 16, 29 pinmux to 2, CLED function */
    SetPinmux(14, 2);
    SetPinmux(15, 2);
    SetPinmux(16, 2);
    SetPinmux(29, 2);
#endif
    
#if defined(BQ16) || defined(BQ16_PRO)
    /* set LED18(R), LED19(G), LED20(B), LED24 (White) as active low */
#elif defined(BT10)
    /* set LED19(R), LED21(G), LED4(B) as active low */
#else
    /* set LED14(R), LED15(G), LED16(B), LED29 (BT)  as active low */
#endif
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

#if defined(BQ16) || defined(BQ16_PRO)
    *cled_input_reg = LED_N_Bit_MASK;
#elif defined(BT10)
    *cled_input_reg = LED_N_Bit_MASK;
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
    /* Turn Green, Red, Blue on */
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

#if defined(BQ16) || defined(BQ16_PRO)
	/* White LED */
	*cled_led_w_reg = 0x0000;
	*cled_led_w_reg_1 = 0x0000;
	*cled_led_w_reg_2 = 0x0000;
	*cled_led_w_reg_3 = 0x0000;
#endif

#if defined(BQ16) || defined(BQ16_PRO)
    /* active */
	/* LED 18, 19, 20, 24 */
    *cled_act_cfg = 0x011c0000;
#elif defined(BT10)
    /* active */
	/* LED 19, 21, 4 */
    *cled_act_cfg = 0x00280010;
#else
    /* active */
	/* GPIO 14, 15, 16, 29 */
    *cled_act_cfg = 0x2001C000;
#endif

    return;
}
#endif

#if defined(RTBE96U) || defined(GTBE96) || defined(GTBE98) || defined(GTBE98PRO) || defined(GTBE19000) || defined(GTBE19000AI) || defined(GTBE96_AI) || defined(GSBE18000) || defined(GSBE12000) || defined(GS7_PRO) || defined(RTBE82M) || defined(RTBE86U) || defined(RTBE58U) || defined(TUFBE3600) || defined(RTBE58U_V2) || defined(TUFBE3600_V2) || defined(RTBE55) || defined(RTBE92U) || defined(RTBE88U) || defined(GT7)
void PowerLEDOn(void)
{
    volatile uint32_t *GPIO_DIR = (void *)(0xff800500);
    volatile uint32_t *GPIO_DATA = (void *)(0xff800520);
    volatile uint32_t *GPIO_DIR_063_032 = (void *)(0xff800504);
    volatile uint32_t *GPIO_DATA_063_032 = (void *)(0xff800524);
    volatile uint32_t *reg_gpio_dir;
    volatile uint32_t *reg_gpio_data;
    uint32_t val32;
#if defined(GSBE18000) || defined(GSBE12000) || defined(GS7_PRO)
    int gpio = 10;
#elif defined(RTBE82M)
    int gpio = 7;
#elif defined(RTBE86U)
    int gpio = 2;
#elif defined(RTBE58U) || defined(TUFBE3600) || defined(RTBE58U_V2) || defined(TUFBE3600_V2) || defined(RTBE55)
    int gpio = 3;
#elif defined(RTBE92U)
    int gpio = 28;
#elif defined(GT7)
    int gpio = 13;
#else
    int gpio = 50;
#endif

    if (gpio > 31) {
        reg_gpio_dir = GPIO_DIR_063_032;
        reg_gpio_data = GPIO_DATA_063_032;
    } else {
        reg_gpio_dir = GPIO_DIR;
        reg_gpio_data = GPIO_DATA;
    }

    /* Set GPIO_X to output */
    val32 = *reg_gpio_dir;
    if (gpio > 31)
    	val32 |= (1 << (gpio - 32));
    else
	val32 |= (1 << (gpio));
    *reg_gpio_dir = val32;

    /* Turn on LED/GPIO_X */
    /* example, active low, 0: ON, 1: OFF */
    val32 = *reg_gpio_data;
#if defined(RTBE82M) || defined(RTBE86U) || defined(RTBE58U) || defined(TUFBE3600) || defined(RTBE58U_V2) || defined(TUFBE3600_V2) || defined(RTBE55) || defined(RTBE92U)
    if (gpio > 31)
	val32 |= (1 << (gpio - 32));	// active high
    else
	val32 |= (1 << (gpio));		// active high
#else
    if (gpio > 31)
	val32 &= ~(1 << (gpio - 32));	// active low
    else
	val32 &= ~(1 << (gpio));	// active low
#endif
    *reg_gpio_data = val32;

    return;
}
#endif

#if defined(GTBE19000AI) || defined(GTBE96_AI)
void SOPOff(void)
{
	volatile uint32_t *GPIO_DIR = (void *)(0xff800500);
	volatile uint32_t *GPIO_DATA = (void *)(0xff800520);
	volatile uint32_t *GPIO_DIR_063_032 = (void *)(0xff800504);
	volatile uint32_t *GPIO_DATA_063_032 = (void *)(0xff800524);
	volatile uint32_t *GPIO_DIR_095_064 = (void *)(0xff800508);
    volatile uint32_t *GPIO_DATA_095_064 = (void *)(0xff800528);
	volatile uint32_t *reg_gpio_dir;
	volatile uint32_t *reg_gpio_data;
	uint32_t val32;
	int sop_gpio[] = { 71, 72, 73 };
	int i;

	for (i = 0; i < 3; i++) {
		if (0 <= sop_gpio[i] && sop_gpio[i] <= 31) {
			reg_gpio_dir = GPIO_DIR;
			reg_gpio_data = GPIO_DATA;
		} else if (32 <= sop_gpio[i] && sop_gpio[i] <= 63) {
			reg_gpio_dir = GPIO_DIR_063_032;
			reg_gpio_data = GPIO_DATA_063_032;
		} else if (64 <= sop_gpio[i] && sop_gpio[i] <= 95) {
			reg_gpio_dir = GPIO_DIR_095_064;
			reg_gpio_data = GPIO_DATA_095_064;
		} else {
			continue;
		}

		/* Set Output Mode */
		val32 = *reg_gpio_dir;
		val32 |= (1 << (sop_gpio[i]));
		*reg_gpio_dir = val32;

		/* Set Low */
		val32 = *reg_gpio_data;
		val32 &= ~(1 << (sop_gpio[i]));
		*reg_gpio_data = val32;
	}

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
#if !defined(CONFIG_BCMBCA_IKOS) 
		bcm_sec_abort();
#endif

	/* UART clocks enabled and gd valid - init serial console */
	preloader_console_init();
#if defined(PERF)
	printf("RevID: %X\n",PERF->RevID);
#endif

#if defined(CONFIG_BCMBCA_IKOS_SPL_JUMP_TO_UBOOT)
	jump_to_image((uintptr_t)CONFIG_SYS_TEXT_BASE, NULL, 0);
#endif

	bcm_strap_drv_reg();

#if  defined(CONFIG_BCMBCA_OTP)
	if (bcm_otp_init()) {
#if !defined(CONFIG_BCMBCA_IKOS) 
		bcm_sec_abort();
#endif
	}
#endif
	bcm_sec_init();

#if defined(BUILD_TAG)
	printf("$SPL: "BUILD_TAG" $\n");
#endif
	early_abort();

#ifdef CONFIG_BCMBCA_DDRC_WBF_EARLY_INIT
	bcm_ddrc_mc2_wbf_buffers_init();
#endif
#if defined(CONFIG_SPL_BCMBCA_UBUS4) &&\
	(defined(CONFIG_BCM63178) || defined(CONFIG_BCM47622))
	bcm_ubus_drv_init();
	/* force axi write reply to workaround wifi memory write ordering issue */
	ubus_master_cpu_enable_axi_write_cache(0);	
#endif		

#if defined(CONFIG_BCMBCA_PMC)
 	bcm_setsw();
#if defined(EMMC_RESET_PLL) || defined(CPU_RESET_PLL) 
	bcm_pmc_drv_reg();
 	reset_plls();
#endif
#endif

#if !CONFIG_IS_ENABLED(SYS_DCACHE_OFF)
	reserve_mmu();
	enable_caches();
#endif
#if defined(XD4PRO) || defined(XT8PRO) || defined(BT12) || defined(BT10) || defined(BQ16) || defined(BQ16_PRO) || defined(BM68) || defined(XT8_V2) || defined(ET8PRO) || defined(ET8_V2)
	PowerCLEDOn();
#endif
#if defined(RTBE96U) || defined(GTBE96) || defined(GTBE98) || defined(GTBE98PRO) || defined(GTBE19000) || defined(GTBE19000AI) || defined(GTBE96_AI) || defined(GSBE18000) || defined(GSBE12000) || defined(GS7_PRO) || defined(RTBE82M) || defined(RTBE86U) || defined(RTBE58U) || defined(TUFBE3600) || defined(RTBE58U_V2) || defined(TUFBE3600_V2) || defined(RTBE55) || defined(RTBE92U) || defined(RTBE88U) || defined(GT7)
	PowerLEDOn();
#endif
#if defined(GTBE19000AI) || defined(GTBE96_AI)
	SOPOff();
#endif
}

void spl_board_deinit(void)
{
#if defined(CONFIG_BCMBCA_DISABLE_SECURE_VERIFY)
	/* Lock SOTP, wipe keys in SRAM and locally in .data */
	bcm_sec_deinit();		
#else
	/* Just wipe keys stored locally in .data */
	bcm_sec_clean_keys(bcm_sec());
#endif

#if defined(CONFIG_BCMBCA_OTP)
	/* Wipe temp SOTP items */
	bcm_otp_deinit();
#endif
	/* 
	 * even thought the name says linux but it does everything needed for
	 * boot to the next image: flush and disable cache, disable mmu
	 */
	cleanup_before_linux();

	arch_cpu_deinit();

#ifdef CONFIG_BCMBCA_PGTBL_IN_MEMC_SRAM
	bcmbca_disable_memc_sram();
#endif
}

void spl_board_ddrinit(early_abort_t* ea_info)
{
#if defined(CONFIG_BCMBCA_DDRC)  
	uint32_t mcb_sel = 0,mcb_mode = 0;

	if ((ea_info->status&SPL_EA_DDR_MCB_SEL)) {
		mcb_mode = (SPL_DDR_INIT_MCB_OVRD|SPL_DDR_INIT_MCB_SEL);
		mcb_sel = ea_info->data;
	}
	else if ((ea_info->status&(SPL_EA_DDR3_SAFE_MODE))) {
		mcb_mode = (SPL_DDR_INIT_MCB_OVRD|SPL_DDR_INIT_DDR3_SAFE_MODE);
	}
#if defined(CONFIG_BCMBCA_DDR4)
	else if ((ea_info->status&SPL_EA_DDR4_SAFE_MODE)) {
		mcb_mode = (SPL_DDR_INIT_MCB_OVRD|SPL_DDR_INIT_DDR4_SAFE_MODE);
	}
#endif
#if defined(CONFIG_BCMBCA_LPDDR4)
	else if ((ea_info->status&SPL_EA_LPDDR4_SAFE_MODE)) {
		mcb_mode = (SPL_DDR_INIT_MCB_OVRD|SPL_DDR_INIT_LPDDR4_SAFE_MODE);
	}
	else if ((ea_info->status&SPL_EA_LPDDR4X_SAFE_MODE)) {
		mcb_mode = (SPL_DDR_INIT_MCB_OVRD|SPL_DDR_INIT_LPDDR4X_SAFE_MODE);
	}
#endif
#if defined(CONFIG_BCMBCA_LPDDR5)
	else if ((ea_info->status&SPL_EA_LPDDR5_SAFE_MODE)) {
		mcb_mode = (SPL_DDR_INIT_MCB_OVRD|SPL_DDR_INIT_LPDDR5_SAFE_MODE);
	}
	else if ((ea_info->status&SPL_EA_LPDDR5X_SAFE_MODE)) {
		mcb_mode = (SPL_DDR_INIT_MCB_OVRD|SPL_DDR_INIT_LPDDR5X_SAFE_MODE);
	}
#endif

	/*printf("\nGot mcb_mode 0x%x\n",mcb_mode);*/
	spl_ddrinit(mcb_mode, mcb_sel);
#endif
	return;
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
	spl_board_ddrinit(ea_info);

	if ((ea_info->status&(SPL_EA_IMAGE_FB))) {
		tplparams.early_flags = SPL_EA_IMAGE_FB;
	}else if ((ea_info->status&SPL_EA_IMAGE_RECOV)) {
		tplparams.early_flags = SPL_EA_IMAGE_RECOV;
	}
	if ((ea_info->status&SPL_EA_IGNORE_BOARDID)) {
		tplparams.early_flags |= SPL_EA_IGNORE_BOARDID;
	}
	if ((ea_info->status&SPL_EA_DDR_SAFE_MODE_MASK)) {
		tplparams.early_flags |= (ea_info->status&SPL_EA_DDR_SAFE_MODE_MASK);
	}	
	/*printf("\nGot TPL flags 0x%x\n",tplparams.early_flags);*/
	setup_tpl_parms(&tplparams);

#if defined(CONFIG_SPL_WATCHDOG_SUPPORT) && defined(CONFIG_WDT)
	if (!(ea_info->status&(SPL_EA_WDT_DISABLE))) 
		initr_watchdog();
#endif
	start_tpl(&tplparams);
}
