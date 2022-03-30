/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2017, STMicroelectronics - All Rights Reserved
 * Author(s): Vikas Manocha, <vikas.manocha@st.com> for STMicroelectronics.
 */

#ifndef _ASM_ARMV7_MPU_H
#define _ASM_ARMV7_MPU_H

#ifdef CONFIG_CPU_V7M
#define AP_SHIFT			24
#define XN_SHIFT			28
#define TEX_SHIFT			19
#define S_SHIFT				18
#define C_SHIFT				17
#define B_SHIFT				16
#else /* CONFIG_CPU_V7R */
#define XN_SHIFT			12
#define AP_SHIFT			8
#define TEX_SHIFT			3
#define S_SHIFT				2
#define C_SHIFT				1
#define B_SHIFT				0
#endif /* CONFIG_CPU_V7R */

#define CACHEABLE			BIT(C_SHIFT)
#define BUFFERABLE			BIT(B_SHIFT)
#define SHAREABLE			BIT(S_SHIFT)
#define REGION_SIZE_SHIFT		1
#define ENABLE_REGION			BIT(0)
#define DISABLE_REGION			0

enum region_number {
	REGION_0 = 0,
	REGION_1,
	REGION_2,
	REGION_3,
	REGION_4,
	REGION_5,
	REGION_6,
	REGION_7,
};

enum ap {
	NO_ACCESS = 0,
	PRIV_RW_USR_NO,
	PRIV_RW_USR_RO,
	PRIV_RW_USR_RW,
	UNPREDICTABLE,
	PRIV_RO_USR_NO,
	PRIV_RO_USR_RO,
};

enum mr_attr {
	STRONG_ORDER = 0,
	SHARED_WRITE_BUFFERED,
	O_I_WT_NO_WR_ALLOC,
	O_I_WB_NO_WR_ALLOC,
	O_I_NON_CACHEABLE,
	O_I_WB_RD_WR_ALLOC,
	DEVICE_NON_SHARED,
};
enum size {
	REGION_8MB = 22,
	REGION_16MB,
	REGION_32MB,
	REGION_64MB,
	REGION_128MB,
	REGION_256MB,
	REGION_512MB,
	REGION_1GB,
	REGION_2GB,
	REGION_4GB,
};

enum xn {
	XN_DIS = 0,
	XN_EN,
};

struct mpu_region_config {
	uint32_t start_addr;
	enum region_number region_no;
	enum xn xn;
	enum ap ap;
	enum mr_attr mr_attr;
	enum size reg_size;
};

void disable_mpu(void);
void enable_mpu(void);
int mpu_enabled(void);
void mpu_config(struct mpu_region_config *reg_config);
void setup_mpu_regions(struct mpu_region_config *rgns, u32 num_rgns);

static inline u32 get_attr_encoding(u32 mr_attr)
{
	u32 attr;

	switch (mr_attr) {
	case STRONG_ORDER:
		attr = SHAREABLE;
		break;
	case SHARED_WRITE_BUFFERED:
		attr = BUFFERABLE;
		break;
	case O_I_WT_NO_WR_ALLOC:
		attr = CACHEABLE;
		break;
	case O_I_WB_NO_WR_ALLOC:
		attr = CACHEABLE | BUFFERABLE;
		break;
	case O_I_NON_CACHEABLE:
		attr = 1 << TEX_SHIFT;
		break;
	case O_I_WB_RD_WR_ALLOC:
		attr = (1 << TEX_SHIFT) | CACHEABLE | BUFFERABLE;
		break;
	case DEVICE_NON_SHARED:
		attr = (2 << TEX_SHIFT) | BUFFERABLE;
		break;
	default:
		attr = 0; /* strongly ordered */
		break;
	};

	return attr;
}

#endif /* _ASM_ARMV7_MPU_H */
