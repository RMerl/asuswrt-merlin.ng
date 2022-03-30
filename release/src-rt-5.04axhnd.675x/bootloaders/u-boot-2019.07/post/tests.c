// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#include <common.h>

#include <post.h>

extern int ocm_post_test (int flags);
extern int cache_post_test (int flags);
extern int watchdog_post_test (int flags);
extern int i2c_post_test (int flags);
extern int rtc_post_test (int flags);
extern int memory_post_test (int flags);
extern int cpu_post_test (int flags);
extern int fpu_post_test (int flags);
extern int uart_post_test (int flags);
extern int ether_post_test (int flags);
extern int spi_post_test (int flags);
extern int usb_post_test (int flags);
extern int spr_post_test (int flags);
extern int sysmon_post_test (int flags);
extern int dsp_post_test (int flags);
extern int codec_post_test (int flags);
extern int ecc_post_test (int flags);
extern int flash_post_test(int flags);

extern int dspic_init_post_test (int flags);
extern int dspic_post_test (int flags);
extern int gdc_post_test (int flags);
extern int fpga_post_test (int flags);
extern int lwmon5_watchdog_post_test(int flags);
extern int sysmon1_post_test(int flags);
extern int coprocessor_post_test(int flags);
extern int led_post_test(int flags);
extern int button_post_test(int flags);
extern int memory_regions_post_test(int flags);

extern int sysmon_init_f (void);

extern void sysmon_reloc (void);


struct post_test post_list[] =
{
#if CONFIG_POST & CONFIG_SYS_POST_OCM
    {
	"OCM test",
	"ocm",
	"This test checks on chip memory (OCM).",
	POST_ROM | POST_ALWAYS | POST_PREREL | POST_CRITICAL | POST_STOP,
	&ocm_post_test,
	NULL,
	NULL,
	CONFIG_SYS_POST_OCM
    },
#endif
#if CONFIG_POST & CONFIG_SYS_POST_CACHE
    {
	"Cache test",
	"cache",
	"This test verifies the CPU cache operation.",
	POST_RAM | POST_ALWAYS,
	&cache_post_test,
	NULL,
	NULL,
	CONFIG_SYS_POST_CACHE
    },
#endif
#if CONFIG_POST & CONFIG_SYS_POST_WATCHDOG
#if defined(CONFIG_POST_WATCHDOG)
	CONFIG_POST_WATCHDOG,
#else
    {
	"Watchdog timer test",
	"watchdog",
	"This test checks the watchdog timer.",
	POST_RAM | POST_POWERON | POST_SLOWTEST | POST_MANUAL | POST_REBOOT,
	&watchdog_post_test,
	NULL,
	NULL,
	CONFIG_SYS_POST_WATCHDOG
    },
#endif
#endif
#if CONFIG_POST & CONFIG_SYS_POST_I2C
    {
	"I2C test",
	"i2c",
	"This test verifies the I2C operation.",
	POST_RAM | POST_ALWAYS,
	&i2c_post_test,
	NULL,
	NULL,
	CONFIG_SYS_POST_I2C
    },
#endif
#if CONFIG_POST & CONFIG_SYS_POST_RTC
    {
	"RTC test",
	"rtc",
	"This test verifies the RTC operation.",
	POST_RAM | POST_SLOWTEST | POST_MANUAL,
	&rtc_post_test,
	NULL,
	NULL,
	CONFIG_SYS_POST_RTC
    },
#endif
#if CONFIG_POST & CONFIG_SYS_POST_MEMORY
    {
	"Memory test",
	"memory",
	"This test checks RAM.",
	POST_ROM | POST_POWERON | POST_SLOWTEST | POST_PREREL,
	&memory_post_test,
	NULL,
	NULL,
	CONFIG_SYS_POST_MEMORY
    },
#endif
#if CONFIG_POST & CONFIG_SYS_POST_CPU
    {
	"CPU test",
	"cpu",
	"This test verifies the arithmetic logic unit of"
	" CPU.",
	POST_RAM | POST_ALWAYS,
	&cpu_post_test,
	NULL,
	NULL,
	CONFIG_SYS_POST_CPU
    },
#endif
#if CONFIG_POST & CONFIG_SYS_POST_FPU
    {
	"FPU test",
	"fpu",
	"This test verifies the arithmetic logic unit of"
	" FPU.",
	POST_RAM | POST_ALWAYS,
	&fpu_post_test,
	NULL,
	NULL,
	CONFIG_SYS_POST_FPU
    },
#endif
#if CONFIG_POST & CONFIG_SYS_POST_UART
#if defined(CONFIG_POST_UART)
	CONFIG_POST_UART,
#else
    {
	"UART test",
	"uart",
	"This test verifies the UART operation.",
	POST_RAM | POST_SLOWTEST | POST_MANUAL,
	&uart_post_test,
	NULL,
	NULL,
	CONFIG_SYS_POST_UART
    },
#endif /* CONFIG_POST_UART */
#endif
#if CONFIG_POST & CONFIG_SYS_POST_ETHER
    {
	"ETHERNET test",
	"ethernet",
	"This test verifies the ETHERNET operation.",
	POST_RAM | POST_ALWAYS,
	&ether_post_test,
	NULL,
	NULL,
	CONFIG_SYS_POST_ETHER
    },
#endif
#if CONFIG_POST & CONFIG_SYS_POST_USB
    {
	"USB test",
	"usb",
	"This test verifies the USB operation.",
	POST_RAM | POST_ALWAYS,
	&usb_post_test,
	NULL,
	NULL,
	CONFIG_SYS_POST_USB
    },
#endif
#if CONFIG_POST & CONFIG_SYS_POST_SPR
    {
	"SPR test",
	"spr",
	"This test checks SPR contents.",
	POST_RAM | POST_ALWAYS,
	&spr_post_test,
	NULL,
	NULL,
	CONFIG_SYS_POST_SPR
    },
#endif
#if CONFIG_POST & CONFIG_SYS_POST_SYSMON
    {
	"SYSMON test",
	"sysmon",
	"This test monitors system hardware.",
	POST_RAM | POST_ALWAYS,
	&sysmon_post_test,
	&sysmon_init_f,
	&sysmon_reloc,
	CONFIG_SYS_POST_SYSMON
    },
#endif
#if CONFIG_POST & CONFIG_SYS_POST_DSP
    {
	"DSP test",
	"dsp",
	"This test checks any connected DSP(s).",
	POST_RAM | POST_ALWAYS,
	&dsp_post_test,
	NULL,
	NULL,
	CONFIG_SYS_POST_DSP
    },
#endif
#if CONFIG_POST & CONFIG_SYS_POST_CODEC
    {
	"CODEC test",
	"codec",
	"This test checks any connected codec(s).",
	POST_RAM | POST_MANUAL,
	&codec_post_test,
	NULL,
	NULL,
	CONFIG_SYS_POST_CODEC
    },
#endif
#if CONFIG_POST & CONFIG_SYS_POST_ECC
    {
	"ECC test",
	"ecc",
	"This test checks the ECC facility of memory.",
	POST_ROM | POST_ALWAYS | POST_PREREL,
	&ecc_post_test,
	NULL,
	NULL,
	CONFIG_SYS_POST_ECC
    },
#endif
#if CONFIG_POST & CONFIG_SYS_POST_BSPEC1
	CONFIG_POST_BSPEC1,
#endif
#if CONFIG_POST & CONFIG_SYS_POST_BSPEC2
	CONFIG_POST_BSPEC2,
#endif
#if CONFIG_POST & CONFIG_SYS_POST_BSPEC3
	CONFIG_POST_BSPEC3,
#endif
#if CONFIG_POST & CONFIG_SYS_POST_BSPEC4
	CONFIG_POST_BSPEC4,
#endif
#if CONFIG_POST & CONFIG_SYS_POST_BSPEC5
	CONFIG_POST_BSPEC5,
#endif
#if CONFIG_POST & CONFIG_SYS_POST_COPROC
    {
	"Coprocessors communication test",
	"coproc_com",
	"This test checks communication with coprocessors.",
	POST_RAM | POST_ALWAYS | POST_CRITICAL,
	&coprocessor_post_test,
	NULL,
	NULL,
	CONFIG_SYS_POST_COPROC
    },
#endif
#if CONFIG_POST & CONFIG_SYS_POST_FLASH
    {
	"Parallel NOR flash test",
	"flash",
	"This test verifies parallel flash operations.",
	POST_RAM | POST_SLOWTEST | POST_MANUAL,
	&flash_post_test,
	NULL,
	NULL,
	CONFIG_SYS_POST_FLASH
    },
#endif
#if CONFIG_POST & CONFIG_SYS_POST_MEM_REGIONS
    {
	"Memory regions test",
	"mem_regions",
	"This test checks regularly placed regions of the RAM.",
	POST_ROM | POST_SLOWTEST | POST_PREREL,
	&memory_regions_post_test,
	NULL,
	NULL,
	CONFIG_SYS_POST_MEM_REGIONS
    },
#endif
};

unsigned int post_list_size = ARRAY_SIZE(post_list);
