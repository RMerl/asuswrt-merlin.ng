/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * This is from the ARM TF Project,
 * Repository: https://github.com/ARM-software/arm-trusted-firmware.git
 * File: include/common/bl_common.h
 * Portions copyright (c) 2013-2016, ARM Limited and Contributors. All rights
 * reserved.
 * Copyright (C) 2016-2017 Rockchip Electronic Co.,Ltd
 */

#ifndef __BL_COMMON_H__
#define __BL_COMMON_H__

#define ATF_PARAM_EP		0x01
#define ATF_PARAM_IMAGE_BINARY	0x02
#define ATF_PARAM_BL31		0x03

#define ATF_VERSION_1	0x01

#define ATF_EP_SECURE	0x0
#define ATF_EP_NON_SECURE	0x1

#define SET_PARAM_HEAD(_p, _type, _ver, _attr) do { \
	(_p)->h.type = (uint8_t)(_type); \
	(_p)->h.version = (uint8_t)(_ver); \
	(_p)->h.size = (uint16_t)sizeof(*_p); \
	(_p)->h.attr = (uint32_t)(_attr) ; \
	} while (0)

#define MODE_RW_SHIFT	0x4
#define MODE_RW_MASK	0x1
#define MODE_RW_64	0x0
#define MODE_RW_32	0x1

#define MODE_EL_SHIFT	0x2
#define MODE_EL_MASK	0x3
#define MODE_EL3	0x3
#define MODE_EL2	0x2
#define MODE_EL1	0x1
#define MODE_EL0	0x0

#define MODE_SP_SHIFT	0x0
#define MODE_SP_MASK	0x1
#define MODE_SP_EL0	0x0
#define MODE_SP_ELX	0x1

#define SPSR_DAIF_SHIFT	6
#define SPSR_DAIF_MASK	0x0f

#define SPSR_64(el, sp, daif)		\
	(MODE_RW_64 << MODE_RW_SHIFT |	\
	 ((el) & MODE_EL_MASK) << MODE_EL_SHIFT |	\
	 ((sp) & MODE_SP_MASK) << MODE_SP_SHIFT |	\
	 ((daif) & SPSR_DAIF_MASK) << SPSR_DAIF_SHIFT)

#define SPSR_FIQ             (1 << 6)
#define SPSR_IRQ             (1 << 7)
#define SPSR_SERROR          (1 << 8)
#define SPSR_DEBUG           (1 << 9)
#define SPSR_EXCEPTION_MASK  (SPSR_FIQ | SPSR_IRQ | SPSR_SERROR | SPSR_DEBUG)

#define DAIF_FIQ_BIT (1<<0)
#define DAIF_IRQ_BIT (1<<1)
#define DAIF_ABT_BIT (1<<2)
#define DAIF_DBG_BIT (1<<3)
#define DISABLE_ALL_EXECPTIONS	\
	(DAIF_FIQ_BIT | DAIF_IRQ_BIT | DAIF_ABT_BIT | DAIF_DBG_BIT)

#ifndef __ASSEMBLY__

/*******************************************************************************
 * Structure used for telling the next BL how much of a particular type of
 * memory is available for its use and how much is already used.
 ******************************************************************************/
struct aapcs64_params {
	unsigned long arg0;
	unsigned long arg1;
	unsigned long arg2;
	unsigned long arg3;
	unsigned long arg4;
	unsigned long arg5;
	unsigned long arg6;
	unsigned long arg7;
};

/***************************************************************************
 * This structure provides version information and the size of the
 * structure, attributes for the structure it represents
 ***************************************************************************/
struct param_header {
	uint8_t type;		/* type of the structure */
	uint8_t version;    /* version of this structure */
	uint16_t size;      /* size of this structure in bytes */
	uint32_t attr;      /* attributes: unused bits SBZ */
};

/*****************************************************************************
 * This structure represents the superset of information needed while
 * switching exception levels. The only two mechanisms to do so are
 * ERET & SMC. Security state is indicated using bit zero of header
 * attribute
 * NOTE: BL1 expects entrypoint followed by spsr at an offset from the start
 * of this structure defined by the macro `ENTRY_POINT_INFO_PC_OFFSET` while
 * processing SMC to jump to BL31.
 *****************************************************************************/
struct entry_point_info {
	struct param_header h;
	uintptr_t pc;
	uint32_t spsr;
	struct aapcs64_params args;
};

/*****************************************************************************
 * Image info binary provides information from the image loader that
 * can be used by the firmware to manage available trusted RAM.
 * More advanced firmware image formats can provide additional
 * information that enables optimization or greater flexibility in the
 * common firmware code
 *****************************************************************************/
struct atf_image_info {
	struct param_header h;
	uintptr_t image_base;   /* physical address of base of image */
	uint32_t image_size;    /* bytes read from image file */
};

/*****************************************************************************
 * The image descriptor struct definition.
 *****************************************************************************/
struct image_desc {
	/* Contains unique image id for the image. */
	unsigned int image_id;
	/*
	 * This member contains Image state information.
	 * Refer IMAGE_STATE_XXX defined above.
	 */
	unsigned int state;
	uint32_t copied_size;	/* image size copied in blocks */
	struct atf_image_info atf_image_info;
	struct entry_point_info ep_info;
};

/*******************************************************************************
 * This structure represents the superset of information that can be passed to
 * BL31 e.g. while passing control to it from BL2. The BL32 parameters will be
 * populated only if BL2 detects its presence. A pointer to a structure of this
 * type should be passed in X0 to BL31's cold boot entrypoint.
 *
 * Use of this structure and the X0 parameter is not mandatory: the BL31
 * platform code can use other mechanisms to provide the necessary information
 * about BL32 and BL33 to the common and SPD code.
 *
 * BL31 image information is mandatory if this structure is used. If either of
 * the optional BL32 and BL33 image information is not provided, this is
 * indicated by the respective image_info pointers being zero.
 ******************************************************************************/
struct bl31_params {
	struct param_header h;
	struct atf_image_info *bl31_image_info;
	struct entry_point_info *bl32_ep_info;
	struct atf_image_info *bl32_image_info;
	struct entry_point_info *bl33_ep_info;
	struct atf_image_info *bl33_image_info;
};

/*******************************************************************************
 * This structure represents the superset of information that is passed to
 * BL31, e.g. while passing control to it from BL2, bl31_params
 * and other platform specific params
 ******************************************************************************/
struct bl2_to_bl31_params_mem {
	struct bl31_params bl31_params;
	struct atf_image_info bl31_image_info;
	struct atf_image_info bl32_image_info;
	struct atf_image_info bl33_image_info;
	struct entry_point_info bl33_ep_info;
	struct entry_point_info bl32_ep_info;
	struct entry_point_info bl31_ep_info;
};

#endif /*__ASSEMBLY__*/

#endif /* __BL_COMMON_H__ */
