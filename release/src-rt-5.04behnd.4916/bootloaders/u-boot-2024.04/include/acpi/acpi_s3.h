/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2017, Bin Meng <bmeng.cn@gmail.com>
 */

#ifndef __ASM_ACPI_S3_H__
#define __ASM_ACPI_S3_H__

#define WAKEUP_BASE	0x600

/* PM1_STATUS register */
#define WAK_STS		(1 << 15)
#define PCIEXPWAK_STS	(1 << 14)
#define RTC_STS		(1 << 10)
#define SLPBTN_STS	(1 << 9)
#define PWRBTN_STS	(1 << 8)
#define GBL_STS		(1 << 5)
#define BM_STS		(1 << 4)
#define TMR_STS		(1 << 0)

/* PM1_CNT register */
#define SLP_EN		(1 << 13)
#define SLP_TYP_SHIFT	10
#define SLP_TYP		(7 << SLP_TYP_SHIFT)
#define SLP_TYP_S0	0
#define SLP_TYP_S1	1
#define SLP_TYP_S3	5
#define SLP_TYP_S4	6
#define SLP_TYP_S5	7

/* PM1_STS register */
#define RTC_EN		BIT(10)
#define PWRBTN_EN	BIT(8)

/* Memory size reserved for S3 resume */
#define S3_RESERVE_SIZE	0x1000

#ifndef __ASSEMBLY__

extern char __wakeup[];
extern int __wakeup_size;

enum acpi_sleep_state {
	ACPI_S0,
	ACPI_S1,
	ACPI_S2,
	ACPI_S3,
	ACPI_S4,
	ACPI_S5,
};

/**
 * acpi_ss_string() - get ACPI-defined sleep state string
 *
 * @pm1_cnt:	ACPI-defined sleep state
 * @return:	a pointer to the sleep state string.
 */
static inline char *acpi_ss_string(enum acpi_sleep_state state)
{
	char *ss_string[] = { "S0", "S1", "S2", "S3", "S4", "S5"};

	return ss_string[state];
}

/**
 * acpi_sleep_from_pm1() - get ACPI-defined sleep state from PM1_CNT register
 *
 * @pm1_cnt:	PM1_CNT register value
 * @return:	ACPI-defined sleep state if given valid PM1_CNT register value,
 *		-EINVAL otherwise.
 */
static inline enum acpi_sleep_state acpi_sleep_from_pm1(u32 pm1_cnt)
{
	switch ((pm1_cnt & SLP_TYP) >> SLP_TYP_SHIFT) {
	case SLP_TYP_S0:
		return ACPI_S0;
	case SLP_TYP_S1:
		return ACPI_S1;
	case SLP_TYP_S3:
		return ACPI_S3;
	case SLP_TYP_S4:
		return ACPI_S4;
	case SLP_TYP_S5:
		return ACPI_S5;
	}

	return -EINVAL;
}

/**
 * chipset_prev_sleep_state() - Get chipset previous sleep state
 *
 * This returns chipset previous sleep state from ACPI registers.
 * Platform codes must supply this routine in order to support ACPI S3.
 *
 * Return: ACPI_S0/S1/S2/S3/S4/S5.
 */
enum acpi_sleep_state chipset_prev_sleep_state(void);

/**
 * chipset_clear_sleep_state() - Clear chipset sleep state
 *
 * This clears chipset sleep state in ACPI registers.
 * Platform codes must supply this routine in order to support ACPI S3.
 */
void chipset_clear_sleep_state(void);

struct acpi_fadt;
/**
 * acpi_resume() - Do ACPI S3 resume
 *
 * This calls U-Boot wake up assembly stub and jumps to OS's wake up vector.
 *
 * @fadt:	FADT table pointer in the ACPI table
 * @return:	Never returns
 */
void acpi_resume(struct acpi_fadt *fadt);

/**
 * acpi_s3_reserve() - Reserve memory for ACPI S3 resume
 *
 * This copies memory where real mode interrupt handler stubs reside to the
 * reserved place on the stack.
 *
 * This routine should be called by reserve_arch() before U-Boot is relocated
 * when ACPI S3 resume is enabled.
 *
 * @return:	0 always
 */
int acpi_s3_reserve(void);

#endif /* __ASSEMBLY__ */

#endif /* __ASM_ACPI_S3_H__ */
