/* SPDX-License-Identifier:     GPL-2.0+ */
/*
 * Copyright 2017-2018 NXP
 *
 */

#ifndef SC_RPC_H
#define SC_RPC_H

/* Note: Check SCFW API Released DOC before you want to modify something */
/* Defines */

#define SCFW_API_VERSION_MAJOR  1U
#define SCFW_API_VERSION_MINOR  21U

#define SC_RPC_VERSION          1U

#define SC_RPC_MAX_MSG          8U

#define RPC_VER(MSG)            ((MSG)->version)
#define RPC_SIZE(MSG)           ((MSG)->size)
#define RPC_SVC(MSG)            ((MSG)->svc)
#define RPC_FUNC(MSG)           ((MSG)->func)
#define RPC_R8(MSG)             ((MSG)->func)
#define RPC_I64(MSG, IDX)       ((s64)(RPC_U32((MSG), (IDX))) << 32ULL) | \
				(s64)(RPC_U32((MSG), (IDX) + 4U))
#define RPC_I32(MSG, IDX)       ((MSG)->DATA.i32[(IDX) / 4U])
#define RPC_I16(MSG, IDX)       ((MSG)->DATA.i16[(IDX) / 2U])
#define RPC_I8(MSG, IDX)        ((MSG)->DATA.i8[(IDX)])
#define RPC_U64(MSG, IDX)       ((u64)(RPC_U32((MSG), (IDX))) << 32ULL) | \
				(u64)(RPC_U32((MSG), (IDX) + 4U))
#define RPC_U32(MSG, IDX)       ((MSG)->DATA.u32[(IDX) / 4U])
#define RPC_U16(MSG, IDX)       ((MSG)->DATA.u16[(IDX) / 2U])
#define RPC_U8(MSG, IDX)        ((MSG)->DATA.u8[(IDX)])

#define SC_RPC_SVC_UNKNOWN      0U
#define SC_RPC_SVC_RETURN       1U
#define SC_RPC_SVC_PM           2U
#define SC_RPC_SVC_RM           3U
#define SC_RPC_SVC_TIMER        5U
#define SC_RPC_SVC_PAD          6U
#define SC_RPC_SVC_MISC         7U
#define SC_RPC_SVC_IRQ          8U
#define SC_RPC_SVC_SECO         9U
#define SC_RPC_SVC_ABORT        10U


/* Types */

struct sc_rpc_msg_s {
	u8 version;
	u8 size;
	u8 svc;
	u8 func;
	union {
		s32 i32[(SC_RPC_MAX_MSG - 1U)];
		s16 i16[(SC_RPC_MAX_MSG - 1U) * 2U];
		s8 i8[(SC_RPC_MAX_MSG - 1U) * 4U];
		u32 u32[(SC_RPC_MAX_MSG - 1U)];
		u16 u16[(SC_RPC_MAX_MSG - 1U) * 2U];
		u8 u8[(SC_RPC_MAX_MSG - 1U) * 4U];
	} DATA;
};

/* PM RPC */
#define PM_FUNC_UNKNOWN				0
#define PM_FUNC_SET_SYS_POWER_MODE		19U
#define PM_FUNC_SET_PARTITION_POWER_MODE	1U
#define PM_FUNC_GET_SYS_POWER_MODE		2U
#define PM_FUNC_PARTITION_WAKE			28U
#define PM_FUNC_SET_RESOURCE_POWER_MODE		3U
#define PM_FUNC_SET_RESOURCE_POWER_MODE_ALL	22U
#define PM_FUNC_GET_RESOURCE_POWER_MODE		4U
#define PM_FUNC_REQ_LOW_POWER_MODE		16U
#define PM_FUNC_REQ_CPU_LOW_POWER_MODE		20U
#define PM_FUNC_SET_CPU_RESUME_ADDR		17U
#define PM_FUNC_SET_CPU_RESUME			21U
#define PM_FUNC_REQ_SYS_IF_POWER_MODE		18U
#define PM_FUNC_SET_CLOCK_RATE			5U
#define PM_FUNC_GET_CLOCK_RATE			6U
#define PM_FUNC_CLOCK_ENABLE			7U
#define PM_FUNC_SET_CLOCK_PARENT		14U
#define PM_FUNC_GET_CLOCK_PARENT		15U
#define PM_FUNC_RESET				13U
#define PM_FUNC_RESET_REASON			10U
#define PM_FUNC_GET_RESET_PART			26U
#define PM_FUNC_BOOT				8U
#define PM_FUNC_SET_BOOT_PARM			27U
#define PM_FUNC_REBOOT				9U
#define PM_FUNC_REBOOT_PARTITION		12U
#define PM_FUNC_REBOOT_CONTINUE			25U
#define PM_FUNC_CPU_START			11U
#define PM_FUNC_CPU_RESET			23U
#define PM_FUNC_RESOURCE_RESET			29U
#define PM_FUNC_IS_PARTITION_STARTED		24U

/* MISC RPC */
#define MISC_FUNC_UNKNOWN			0
#define MISC_FUNC_SET_CONTROL			1U
#define MISC_FUNC_GET_CONTROL			2U
#define MISC_FUNC_SET_MAX_DMA_GROUP		4U
#define MISC_FUNC_SET_DMA_GROUP			5U
#define MISC_FUNC_DEBUG_OUT			10U
#define MISC_FUNC_WAVEFORM_CAPTURE		6U
#define MISC_FUNC_BUILD_INFO			15U
#define MISC_FUNC_API_VER			35U
#define MISC_FUNC_UNIQUE_ID			19U
#define MISC_FUNC_SET_ARI			3U
#define MISC_FUNC_BOOT_STATUS			7U
#define MISC_FUNC_BOOT_DONE			14U
#define MISC_FUNC_OTP_FUSE_READ			11U
#define MISC_FUNC_OTP_FUSE_WRITE		17U
#define MISC_FUNC_SET_TEMP			12U
#define MISC_FUNC_GET_TEMP			13U
#define MISC_FUNC_GET_BOOT_DEV			16U
#define MISC_FUNC_GET_BOOT_TYPE			33U
#define MISC_FUNC_GET_BOOT_CONTAINER		36U
#define MISC_FUNC_GET_BUTTON_STATUS		18U
#define MISC_FUNC_ROMPATCH_CHECKSUM		26U
#define MISC_FUNC_BOARD_IOCTL			34U

/* PAD RPC */
#define PAD_FUNC_UNKNOWN			0
#define PAD_FUNC_SET_MUX			1U
#define PAD_FUNC_GET_MUX			6U
#define PAD_FUNC_SET_GP				2U
#define PAD_FUNC_GET_GP				7U
#define PAD_FUNC_SET_WAKEUP			4U
#define PAD_FUNC_GET_WAKEUP			9U
#define PAD_FUNC_SET_ALL			5U
#define PAD_FUNC_GET_ALL			10U
#define PAD_FUNC_SET				15U
#define PAD_FUNC_GET				16U
#define PAD_FUNC_SET_GP_28FDSOI			11U
#define PAD_FUNC_GET_GP_28FDSOI			12U
#define PAD_FUNC_SET_GP_28FDSOI_HSIC		3U
#define PAD_FUNC_GET_GP_28FDSOI_HSIC		8U
#define PAD_FUNC_SET_GP_28FDSOI_COMP		13U
#define PAD_FUNC_GET_GP_28FDSOI_COMP		14U

/* RM RPC */
#define RM_FUNC_UNKNOWN				0
#define RM_FUNC_PARTITION_ALLOC			1U
#define RM_FUNC_SET_CONFIDENTIAL		31U
#define RM_FUNC_PARTITION_FREE			2U
#define RM_FUNC_GET_DID				26U
#define RM_FUNC_PARTITION_STATIC		3U
#define RM_FUNC_PARTITION_LOCK			4U
#define RM_FUNC_GET_PARTITION			5U
#define RM_FUNC_SET_PARENT			6U
#define RM_FUNC_MOVE_ALL			7U
#define RM_FUNC_ASSIGN_RESOURCE			8U
#define RM_FUNC_SET_RESOURCE_MOVABLE		9U
#define RM_FUNC_SET_SUBSYS_RSRC_MOVABLE		28U
#define RM_FUNC_SET_MASTER_ATTRIBUTES		10U
#define RM_FUNC_SET_MASTER_SID			11U
#define RM_FUNC_SET_PERIPHERAL_PERMISSIONS	12U
#define RM_FUNC_IS_RESOURCE_OWNED		13U
#define RM_FUNC_GET_RESOURCE_OWNER 33U
#define RM_FUNC_IS_RESOURCE_MASTER		14U
#define RM_FUNC_IS_RESOURCE_PERIPHERAL		15U
#define RM_FUNC_GET_RESOURCE_INFO		16U
#define RM_FUNC_MEMREG_ALLOC			17U
#define RM_FUNC_MEMREG_SPLIT			29U
#define RM_FUNC_MEMREG_FRAG			32U
#define RM_FUNC_MEMREG_FREE			18U
#define RM_FUNC_FIND_MEMREG			30U
#define RM_FUNC_ASSIGN_MEMREG			19U
#define RM_FUNC_SET_MEMREG_PERMISSIONS		20U
#define RM_FUNC_IS_MEMREG_OWNED			21U
#define RM_FUNC_GET_MEMREG_INFO			22U
#define RM_FUNC_ASSIGN_PAD			23U
#define RM_FUNC_SET_PAD_MOVABLE			24U
#define RM_FUNC_IS_PAD_OWNED			25U
#define RM_FUNC_DUMP				27U

/* SECO RPC */
#define SECO_FUNC_UNKNOWN 0 /* Unknown function */
#define SECO_FUNC_IMAGE_LOAD 1U /* Index for seco_image_load() RPC call */
#define SECO_FUNC_AUTHENTICATE 2U /* Index for seco_authenticate() RPC call */
#define SECO_FUNC_ENH_AUTHENTICATE 24U /* Index for sc_seco_enh_authenticate() RPC call */
#define SECO_FUNC_FORWARD_LIFECYCLE 3U /* Index for seco_forward_lifecycle() RPC call */
#define SECO_FUNC_RETURN_LIFECYCLE 4U /* Index for seco_return_lifecycle() RPC call */
#define SECO_FUNC_COMMIT 5U /* Index for seco_commit() RPC call */
#define SECO_FUNC_ATTEST_MODE 6U /* Index for seco_attest_mode() RPC call */
#define SECO_FUNC_ATTEST 7U /* Index for seco_attest() RPC call */
#define SECO_FUNC_GET_ATTEST_PKEY 8U /* Index for seco_get_attest_pkey() RPC call */
#define SECO_FUNC_GET_ATTEST_SIGN 9U /* Index for seco_get_attest_sign() RPC call */
#define SECO_FUNC_ATTEST_VERIFY 10U /* Index for seco_attest_verify() RPC call */
#define SECO_FUNC_GEN_KEY_BLOB 11U /* Index for seco_gen_key_blob() RPC call */
#define SECO_FUNC_LOAD_KEY 12U /* Index for seco_load_key() RPC call */
#define SECO_FUNC_GET_MP_KEY 13U /* Index for seco_get_mp_key() RPC call */
#define SECO_FUNC_UPDATE_MPMR 14U /* Index for seco_update_mpmr() RPC call */
#define SECO_FUNC_GET_MP_SIGN 15U /* Index for seco_get_mp_sign() RPC call */
#define SECO_FUNC_BUILD_INFO 16U /* Index for seco_build_info() RPC call */
#define SECO_FUNC_V2X_BUILD_INFO 30U /* Index for sc_seco_v2x_build_info() RPC call */
#define SECO_FUNC_CHIP_INFO 17U /* Index for seco_chip_info() RPC call */
#define SECO_FUNC_ENABLE_DEBUG 18U /* Index for seco_enable_debug() RPC call */
#define SECO_FUNC_GET_EVENT 19U /* Index for seco_get_event() RPC call */
#define SECO_FUNC_FUSE_WRITE 20U /* Index for seco_fuse_write() RPC call */
#define SECO_FUNC_PATCH 21U /* Index for sc_seco_patch() RPC call */
#define SECO_FUNC_START_RNG 22U /* Index for sc_seco_start_rng() RPC call */
#define SECO_FUNC_SAB_MSG 23U /* Index for sc_seco_sab_msg() RPC call */
#define SECO_FUNC_SECVIO_ENABLE 25U /* Index for sc_seco_secvio_enable() RPC call */
#define SECO_FUNC_SECVIO_CONFIG 26U /* Index for sc_seco_secvio_config() RPC call */
#define SECO_FUNC_SECVIO_DGO_CONFIG 27U /* Index for sc_seco_secvio_dgo_config() RPC call */

/* IRQ RPC */
#define IRQ_FUNC_UNKNOWN 0 /* Unknown function */
#define IRQ_FUNC_ENABLE 1U /* Index for sc_irq_enable() RPC call */
#define IRQ_FUNC_STATUS 2U /* Index for sc_irq_status() RPC call */

/* TIMER RPC */
#define TIMER_FUNC_UNKNOWN 0 /* Unknown function */
#define TIMER_FUNC_SET_WDOG_TIMEOUT 1U /* Index for sc_timer_set_wdog_timeout() RPC call */
#define TIMER_FUNC_SET_WDOG_PRE_TIMEOUT 12U /* Index for sc_timer_set_wdog_pre_timeout() RPC call */
#define TIMER_FUNC_SET_WDOG_WINDOW 19U /* Index for sc_timer_set_wdog_window() RPC call */
#define TIMER_FUNC_START_WDOG 2U /* Index for sc_timer_start_wdog() RPC call */
#define TIMER_FUNC_STOP_WDOG 3U /* Index for sc_timer_stop_wdog() RPC call */
#define TIMER_FUNC_PING_WDOG 4U /* Index for sc_timer_ping_wdog() RPC call */
#define TIMER_FUNC_GET_WDOG_STATUS 5U /* Index for sc_timer_get_wdog_status() RPC call */
#define TIMER_FUNC_PT_GET_WDOG_STATUS 13U /* Index for sc_timer_pt_get_wdog_status() RPC call */
#define TIMER_FUNC_SET_WDOG_ACTION 10U /* Index for sc_timer_set_wdog_action() RPC call */
#define TIMER_FUNC_SET_RTC_TIME 6U /* Index for sc_timer_set_rtc_time() RPC call */
#define TIMER_FUNC_GET_RTC_TIME 7U /* Index for sc_timer_get_rtc_time() RPC call */
#define TIMER_FUNC_GET_RTC_SEC1970 9U /* Index for sc_timer_get_rtc_sec1970() RPC call */
#define TIMER_FUNC_SET_RTC_ALARM 8U /* Index for sc_timer_set_rtc_alarm() RPC call */
#define TIMER_FUNC_SET_RTC_PERIODIC_ALARM 14U /* Index for sc_timer_set_rtc_periodic_alarm() RPC call */
#define TIMER_FUNC_CANCEL_RTC_ALARM 15U /* Index for sc_timer_cancel_rtc_alarm() RPC call */
#define TIMER_FUNC_SET_RTC_CALB 11U /* Index for sc_timer_set_rtc_calb() RPC call */
#define TIMER_FUNC_SET_SYSCTR_ALARM 16U /* Index for sc_timer_set_sysctr_alarm() RPC call */
#define TIMER_FUNC_SET_SYSCTR_PERIODIC_ALARM 17U /* Index for sc_timer_set_sysctr_periodic_alarm() RPC call */
#define TIMER_FUNC_CANCEL_SYSCTR_ALARM 18U /* Index for sc_timer_cancel_sysctr_alarm() RPC call */

#endif /* SC_RPC_H */
