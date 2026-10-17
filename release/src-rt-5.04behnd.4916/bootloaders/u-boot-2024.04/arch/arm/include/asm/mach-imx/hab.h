/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2012-2015 Freescale Semiconductor, Inc. All Rights Reserved.
 *
*/

#ifndef __SECURE_MX6Q_H__
#define __SECURE_MX6Q_H__

#include <linux/types.h>
#include <linux/compiler.h>

/*
 * IVT header definitions
 * Security Reference Manual for i.MX 7Dual and 7Solo Applications Processors,
 * Rev. 0, 03/2017
 * Section : 6.7.1.1
 */
#define IVT_HEADER_MAGIC	0xD1
#define IVT_TOTAL_LENGTH	0x20

struct __packed ivt_header {
	uint8_t		magic;
	uint16_t	length;
	uint8_t		version;
};

struct ivt {
	struct ivt_header hdr;	/* IVT header above */
	uint32_t entry;		/* Absolute address of first instruction */
	uint32_t reserved1;	/* Reserved should be zero */
	uint32_t dcd;		/* Absolute address of the image DCD */
	uint32_t boot;		/* Absolute address of the boot data */
	uint32_t self;		/* Absolute address of the IVT */
	uint32_t csf;		/* Absolute address of the CSF */
	uint32_t reserved2;	/* Reserved should be zero */
};

struct __packed hab_hdr {
	u8 tag;              /* Tag field */
	u8 len[2];           /* Length field in bytes (big-endian) */
	u8 par;              /* Parameters field */
};

/* Default event structure */
struct __packed evt_def {
	struct hab_hdr hdr;		/* Header */
	uint32_t sts;			/* Status */
	uint32_t ctx;			/* Default context */
	uint8_t *data;			/* Default data location */
	size_t bytes;			/* Size of default data */
};

/* -------- start of HAB API updates ------------*/
/* The following are taken from HAB4 SIS */

/* Status definitions */
enum hab_status {
	HAB_STS_ANY = 0x00,
	HAB_FAILURE = 0x33,
	HAB_WARNING = 0x69,
	HAB_SUCCESS = 0xf0
};

/* Security Configuration definitions */
enum hab_config {
	HAB_CFG_RETURN = 0x33,	/* < Field Return IC */
	HAB_CFG_OPEN = 0xf0,	/* < Non-secure IC */
	HAB_CFG_CLOSED = 0xcc	/* < Secure IC */
};

/* State definitions */
enum hab_state {
	HAB_STATE_INITIAL = 0x33,	/* Initialising state (transitory) */
	HAB_STATE_CHECK = 0x55,		/* Check state (non-secure) */
	HAB_STATE_NONSECURE = 0x66,	/* Non-secure state */
	HAB_STATE_TRUSTED = 0x99,	/* Trusted state */
	HAB_STATE_SECURE = 0xaa,	/* Secure state */
	HAB_STATE_FAIL_SOFT = 0xcc, /* Soft fail state */
	HAB_STATE_FAIL_HARD = 0xff, /* Hard fail state (terminal) */
	HAB_STATE_NONE = 0xf0,		/* No security state machine */
	HAB_STATE_MAX
};

enum hab_reason {
	HAB_RSN_ANY = 0x00,			/* Match any reason */
	HAB_ENG_FAIL = 0x30,		/* Engine failure */
	HAB_INV_ADDRESS = 0x22,		/* Invalid address: access denied */
	HAB_INV_ASSERTION = 0x0c,   /* Invalid assertion */
	HAB_INV_CALL = 0x28,		/* Function called out of sequence */
	HAB_INV_CERTIFICATE = 0x21, /* Invalid certificate */
	HAB_INV_COMMAND = 0x06,     /* Invalid command: command malformed */
	HAB_INV_CSF = 0x11,			/* Invalid csf */
	HAB_INV_DCD = 0x27,			/* Invalid dcd */
	HAB_INV_INDEX = 0x0f,		/* Invalid index: access denied */
	HAB_INV_IVT = 0x05,			/* Invalid ivt */
	HAB_INV_KEY = 0x1d,			/* Invalid key */
	HAB_INV_RETURN = 0x1e,		/* Failed callback function */
	HAB_INV_SIGNATURE = 0x18,   /* Invalid signature */
	HAB_INV_SIZE = 0x17,		/* Invalid data size */
	HAB_MEM_FAIL = 0x2e,		/* Memory failure */
	HAB_OVR_COUNT = 0x2b,		/* Expired poll count */
	HAB_OVR_STORAGE = 0x2d,		/* Exhausted storage region */
	HAB_UNS_ALGORITHM = 0x12,   /* Unsupported algorithm */
	HAB_UNS_COMMAND = 0x03,		/* Unsupported command */
	HAB_UNS_ENGINE = 0x0a,		/* Unsupported engine */
	HAB_UNS_ITEM = 0x24,		/* Unsupported configuration item */
	HAB_UNS_KEY = 0x1b,	        /* Unsupported key type/parameters */
	HAB_UNS_PROTOCOL = 0x14,	/* Unsupported protocol */
	HAB_UNS_STATE = 0x09,		/* Unsuitable state */
	HAB_RSN_MAX
};

enum hab_context {
	HAB_CTX_ANY = 0x00,			/* Match any context */
	HAB_CTX_FAB = 0xff,		    /* Event logged in hab_fab_test() */
	HAB_CTX_ENTRY = 0xe1,		/* Event logged in hab_rvt.entry() */
	HAB_CTX_TARGET = 0x33,	    /* Event logged in hab_rvt.check_target() */
	HAB_CTX_AUTHENTICATE = 0x0a,/* Logged in hab_rvt.authenticate_image() */
	HAB_CTX_DCD = 0xdd,         /* Event logged in hab_rvt.run_dcd() */
	HAB_CTX_CSF = 0xcf,         /* Event logged in hab_rvt.run_csf() */
	HAB_CTX_COMMAND = 0xc0,     /* Event logged executing csf/dcd command */
	HAB_CTX_AUT_DAT = 0xdb,		/* Authenticated data block */
	HAB_CTX_ASSERT = 0xa0,		/* Event logged in hab_rvt.assert() */
	HAB_CTX_EXIT = 0xee,		/* Event logged in hab_rvt.exit() */
	HAB_CTX_MAX
};

enum hab_target {
	HAB_TGT_MEMORY		= 0x0f,
	HAB_TGT_PERIPHERAL	= 0xf0,
	HAB_TGT_ANY		= 0x55,
};

struct imx_sec_config_fuse_t {
	int bank;
	int word;
};

#if defined(CONFIG_IMX_HAB)
extern struct imx_sec_config_fuse_t const imx_sec_config_fuse;
#endif

/*Function prototype description*/
typedef enum hab_status hab_rvt_report_event_t(enum hab_status, uint32_t,
		uint8_t* , size_t*);
typedef enum hab_status hab_rvt_report_status_t(enum hab_config *,
		enum hab_state *);
typedef enum hab_status hab_loader_callback_f_t(void**, size_t*, const void*);
typedef enum hab_status hab_rvt_entry_t(void);
typedef enum hab_status hab_rvt_exit_t(void);
typedef void *hab_rvt_authenticate_image_t(uint8_t, ptrdiff_t,
		void **, size_t *, hab_loader_callback_f_t);
typedef enum hab_status hab_rvt_check_target_t(enum hab_target, const void *,
					       size_t);
typedef void hab_rvt_failsafe_t(void);
typedef void hapi_clock_init_t(void);

#define HAB_ENG_ANY		0x00   /* Select first compatible engine */
#define HAB_ENG_SCC		0x03   /* Security controller */
#define HAB_ENG_RTIC	0x05   /* Run-time integrity checker */
#define HAB_ENG_SAHARA  0x06   /* Crypto accelerator */
#define HAB_ENG_CSU		0x0a   /* Central Security Unit */
#define HAB_ENG_SRTC	0x0c   /* Secure clock */
#define HAB_ENG_DCP		0x1b   /* Data Co-Processor */
#define HAB_ENG_CAAM	0x1d   /* CAAM */
#define HAB_ENG_SNVS	0x1e   /* Secure Non-Volatile Storage */
#define HAB_ENG_OCOTP	0x21   /* Fuse controller */
#define HAB_ENG_DTCP	0x22   /* DTCP co-processor */
#define HAB_ENG_ROM		0x36   /* Protected ROM area */
#define HAB_ENG_HDCP	0x24   /* HDCP co-processor */
#define HAB_ENG_RTL		0x77   /* RTL simulation engine */
#define HAB_ENG_SW		0xff   /* Software engine */

#ifdef CONFIG_ARM64
#ifdef CONFIG_IMX8MQ
#define HAB_RVT_BASE                   0x00000880
#else
#define HAB_RVT_BASE                   0x00000900
#endif

#define HAB_RVT_ENTRY			(*(ulong *)(HAB_RVT_BASE + 0x08))
#define HAB_RVT_EXIT			(*(ulong *)(HAB_RVT_BASE + 0x10))
#define HAB_RVT_CHECK_TARGET		(*(ulong *)(HAB_RVT_BASE + 0x18))
#define HAB_RVT_AUTHENTICATE_IMAGE	(*(ulong *)(HAB_RVT_BASE + 0x20))
#define HAB_RVT_REPORT_EVENT		(*(ulong *)(HAB_RVT_BASE + 0x40))
#define HAB_RVT_REPORT_STATUS		(*(ulong *)(HAB_RVT_BASE + 0x48))
#define HAB_RVT_FAILSAFE		(*(ulong *)(HAB_RVT_BASE + 0x50))
#else

#ifdef CONFIG_ROM_UNIFIED_SECTIONS
#define HAB_RVT_BASE			0x00000100
#else
#define HAB_RVT_BASE_NEW		0x00000098
#define HAB_RVT_BASE_OLD		0x00000094
#define HAB_RVT_BASE ((is_mx6dqp()) ?					\
			HAB_RVT_BASE_NEW :				\
			(is_mx6dq() && (soc_rev() >= CHIP_REV_1_3)) ?	\
			HAB_RVT_BASE_NEW :				\
			(is_mx6sdl() && (soc_rev() >= CHIP_REV_1_2)) ?	\
			HAB_RVT_BASE_NEW : HAB_RVT_BASE_OLD)
#endif

#define HAB_RVT_ENTRY			(*(uint32_t *)(HAB_RVT_BASE + 0x04))
#define HAB_RVT_EXIT			(*(uint32_t *)(HAB_RVT_BASE + 0x08))
#define HAB_RVT_CHECK_TARGET		(*(uint32_t *)(HAB_RVT_BASE + 0x0C))
#define HAB_RVT_AUTHENTICATE_IMAGE	(*(uint32_t *)(HAB_RVT_BASE + 0x10))
#define HAB_RVT_REPORT_EVENT		(*(uint32_t *)(HAB_RVT_BASE + 0x20))
#define HAB_RVT_REPORT_STATUS		(*(uint32_t *)(HAB_RVT_BASE + 0x24))
#define HAB_RVT_FAILSAFE		(*(uint32_t *)(HAB_RVT_BASE + 0x28))

#endif /*CONFIG_ARM64*/

#define HAB_CID_ROM 0 /**< ROM Caller ID */
#define HAB_CID_UBOOT 1 /**< UBOOT Caller ID*/

#define HAB_TAG_RVT          0xDD  /* ROM Vector Table */
#define HAB_CMD_HDR          0xD4  /* CSF Header */
#define HAB_CMD_WRT_DAT      0xCC  /* Write Data command tag */
#define HAB_CMD_CHK_DAT      0xCF  /* Check Data command tag */
#define HAB_CMD_SET          0xB1  /* Set command tag */
#define HAB_PAR_MID          0x01  /* MID parameter value */

#define IVT_SIZE			0x20
#define CSF_PAD_SIZE			0x2000

#define HAB_TAG_EVT		0xDB
#define HAB_TAG_EVT_DEF		0x0C

#define HAB_MAJ_VER		0x40
#define HAB_MAJ_MASK		0xF0

/* ----------- end of HAB API updates ------------*/

int imx_hab_authenticate_image(uint32_t ddr_start, uint32_t image_size,
			       uint32_t ivt_offset);
bool imx_hab_is_enabled(void);

#endif
