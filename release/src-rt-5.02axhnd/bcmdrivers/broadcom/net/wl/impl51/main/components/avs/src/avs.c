/*
 * Common interface for Adaptive Voltage Scaling module
 *
 * Copyright 2019 Broadcom
 *
 * This program is the proprietary software of Broadcom and/or
 * its licensors, and may only be used, duplicated, modified or distributed
 * pursuant to the terms and conditions of a separate, written license
 * agreement executed between you and Broadcom (an "Authorized License").
 * Except as set forth in an Authorized License, Broadcom grants no license
 * (express or implied), right to use, or waiver of any kind with respect to
 * the Software, and Broadcom expressly reserves all rights in and to the
 * Software and all intellectual property rights therein.  IF YOU HAVE NO
 * AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY
 * WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF
 * THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use
 * all reasonable efforts to protect the confidentiality thereof, and to
 * use this information only in connection with your use of Broadcom
 * integrated circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
 * REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR
 * OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
 * DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
 * NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
 * ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 * CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
 * OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
 * BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL,
 * SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR
 * IN ANY WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
 * IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii)
 * ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF
 * OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY
 * NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 * <<Broadcom-WL-IPTag/Proprietary:>>
 *
 * $Id$
 */

#include "avs.h"
#include "avs_regs.h"

#include <bcmutils.h>
#include <bcmdevs.h>
#include <typedefs.h>
#include <osl.h>
#include <wl_dbg.h>
#include <wlioctl_defs.h>
#include <hndpmu.h>
#include <hndsoc.h>

#ifndef DONGLEBUILD
extern uint32 wl_backplane_read(void *context, uint32 address);
extern void wl_backplane_write(void *context, uint32 address, uint32 value);
extern uint32 wl_pmu_access(void *context, uint32 address, uint32 mask, uint32 value);
#endif // endif

#define AVS_PRINT(args)				printf args
#define AVS_ERROR(args)				WL_ERROR(args)
#define AVS_ERROR_IF(cond, args)		do { if (cond) AVS_ERROR(args); } while(0)
#define AVS_INFORM(args)			WL_INFORM(args)
#define AVS_TRACE(args)				WL_TRACE(args)
#define AVS_TRACE_IF(cond, args)		do { if (cond) AVS_TRACE(args); } while(0)

static const char BCMATTACHDATA(rstr_avsvstep)[]   = "avsvstep";
static const char BCMATTACHDATA(rstr_avsdacstep)[] = "avsdacstep";
static const char BCMATTACHDATA(rstr_avssettle)[]  = "avssettle";
static const char BCMATTACHDATA(rstr_avsmargins)[] = "avsmargins";
static const char BCMATTACHDATA(rstr_avslimits)[]  = "avslimits";
static const char BCMATTACHDATA(rstr_avsinitial)[] = "avsinitial";
static const char BCMATTACHDATA(rstr_avsthrm)[]    = "avsthrm";

/* @note Constants marked with [BOARD] are board-dependent */

/*
 * Hardware related settings
 */

#define NUMBER_OF_CENTRALS			24		/* Should adjust CEN_ROSC_MASK_0 accordingly */
#define NUMBER_OF_REMOTES_43684			9
#define NUMBER_OF_REMOTES_6710			5
#define NUMBER_OF_REMOTES(sih)			(BCM6710_CHIP(sih->chip) ? NUMBER_OF_REMOTES_6710 : NUMBER_OF_REMOTES_43684)
#define NUMBER_OF_REMOTES_MAX			MAX(NUMBER_OF_REMOTES_43684, NUMBER_OF_REMOTES_6710)
#define AVS_DETECT_THRESHOLD			0.010		/* Required voltage delta (V) for detecting AVS feature */
#define ROSC_DETECT_THRESHOLD			100		/* Required counts per measuring interval for detecting oscillator */
#define LVM_ENABLE_THRESHOLD			0.920		/* Threshold (V) for enabling Low Voltage Memory, this */
								/* value includes a margin of 20mV for dynamic IR drop at the */
								/* local memories and voltage measurement error */
#define LVM_ENABLE_HYSTERESIS			0.020		/* Hysteresis (V) to apply to LVM control */
#define ADC_FULL_SCALE_VOLTAGE			0.880		/* (V) */
#define PVT_AVERAGING_ACCURATE			3		/* PVT value averaging (2^x times) for accurate measurements */
#define PVT_AVERAGING_FAST			0		/* PVT value averaging (2^x times) for fast measurements */
#define DEFAULT_DAC_VOLTAGE_STEP_MV		0.600		/* Voltage step corresponding to 1 DAC code (mV) [BOARD] */
#define DEFAULT_REGULATOR_SETTLE_TIME_MS	10		/* Voltage regulator settle time (ms) [BOARD] */

/*
 * Hard limits (for configuration validation)
 */

#define MAX_VMARGIN				0.200		/* Maximum margin in either corner (V) */
#define MIN_VLIMIT				0.800		/* Minimum operating voltage (V) */
#define MAX_VLIMIT				1.100		/* Maximum operating voltage (V) */
#define MIN_THRESHOLD_MARGIN			0.001		/* Minimum treshold margin (V) */
#define MAX_THRESHOLD_MARGIN			0.020		/* Maximum treshold margin (V) */
#define MIN_INITIAL_DAC_CODE			500		/* Minimum initial DAC code */
#define MAX_INITIAL_DAC_CODE			800		/* Maximum initial DAC code */
#define MIN_DAC_VOLTAGE_STEP_MV			0.500		/* Minimum voltage step per dac code (mV) */
#define MAX_DAC_VOLTAGE_STEP_MV			1.300		/* Maximum voltage step per dac code (mV) */
#define MIN_REGULATOR_SETTLE_TIME_MS		1		/* Minimum regulator settle time (ms) */
#define	MAX_REGULATOR_SETTLE_TIME_MS		100		/* Maximum regulator settle time (ms) */

/*
 * Margins
 *
 * The following parameters define the safety margins applied to calculated voltages. These are
 * typically chip-dependent and need to be customized for each chip based on characterization. It
 * is possible to apply the same set of parameters to multiple chips if there are enough system
 * and design similarities and a little bit if extra margin is built-in.
 *
 * Below bench margins are usually set to ATE + 0.02V provided power budget and performance are met
 * (+10mV aging and +10mV regulator guard band). If regulators are very precise, bench margins
 * could be set to ATE + 0.010V
 *
 * The threshold margin determines the dead band for making voltage adjustments.
 *
 * When adding support for additional chips, add code for loading defaults to @see avs_attach
 */

#define DEFAULT_VMARGIN_L_43684			0.055		/* Margin given in FF corner (V) [BOARD] */
#define DEFAULT_VMARGIN_H_43684			0.060		/* Margin given in SS corner (V) [BOARD] */
#define DEFAULT_VLIMIT_L_43684			0.860		/* Minimum operating voltage (V) [BOARD] */
#define DEFAULT_VLIMIT_H_43684			1.035		/* Maximum operating voltage (V) [BOARD] */

#define DEFAULT_VMARGIN_L_6710			0.055		/* Margin given in FF corner (V) [BOARD] */
#define DEFAULT_VMARGIN_H_6710			0.060		/* Margin given in SS corner (V) [BOARD] */
#define DEFAULT_VLIMIT_L_6710			0.860		/* Minimum operating voltage (V) [BOARD] */
#define DEFAULT_VLIMIT_H_6710			1.035		/* Maximum operating voltage (V) [BOARD] */

#define DEFAULT_THRESHOLD_MARGIN		0.005		/* Margin between low and high rosc threshold (V) */

/*
 * DAC
 */

#define DEFAULT_INITIAL_DAC_CODE		715		/* Initial DAC code [BOARD] */
#define DAC_STEP_SIZE				12		/* Max step for large voltage changes (DAC codes) [BOARD] */
#define DAC_ADJUST_SIZE				1		/* Voltage adjustment step size (DAC codes) */
#define MAX_DAC_CODE				0x3ff
#define MAX_ADC_CODE				0x3ff
#define INVALID_DAC_CODE			0		/* Value used for indicating uninitialized DAC code */
#define INVALID_PVT_VALUE			0x7fffffff	/* Value used for indicating invalid PVT values */

/*
 * Prediction algorithm
 */

#define PREDICT_PASSES				2		/* Number of linear fittings done during the predict phase */
#define LOCAL_FIT_DAC_OFFSET			0.030		/* DAC offset for local fit (V) */
#define GLOBAL_FIT_SAFETY_MARGIN		0.020		/* Safety margin for global fit (V) */
#define NOMINAL_VOLTAGE				1.030		/* Nominal voltage */
#define NOMINAL_VOLTAGE_ERR			0.005		/* Max distance from nominal voltage (V) */
#define NOMINAL_VOLTAGE_MAX_ITERATIONS		10		/* Max steps to find nominal voltage */
#define PVT_REGISTER_REREAD_COUNT		32		/* Register read limit */
#define PVT_REGISTER_REREAD_DELAY_US		2000		/* Delay between register reads */
#define PVT_REGISTER_READ_DELAY_US		14000		/* Delay before first register read */
#define ROSC_MEASUREMENT_TIME_CONTROL		0x7f		/* MSB of reference counter count interval, eg. 0x3f is 0x3fff clocks */
#define ROSC_FREQ_43684				54		/* ROSC frequency (MHz) */
#define ROSC_FREQ_6710				50		/* ROSC frequency (MHz) */
#define ROSC_FREQ(sih)				(BCM6710_CHIP(sih->chip) ? ROSC_FREQ_6710 : ROSC_FREQ_43684)

/* Calculate ROSC count interval, in clocks */
#define ROSC_MEASUREMENT_TIME			((ROSC_MEASUREMENT_TIME_CONTROL << 8) | 0xff)

/*
 * Sensor sequencer
 */

/* Specify which PVT sensors to measure, 1=skip, 0=measure */
#define PVT_MONITOR_MASK			~((1 << PVT_TEMPERATURE) | (1 << PVT_1V_0))

/* Specify which central oscillators to measure, 1=skip, 0=measure */
#define CEN_ROSC_MASK_0				0xff000000
#define CEN_ROSC_MASK_1				0xffffffff

/*
 * Remote registers
 *
 * The PMB remotes are on a separate bus. This means it takes longer than normal to access any
 * of the registers. This applies mostly to read operations.
 */

#define PMB_REMOTE_ROSC_DELAY_US		200		/* Delay after PMB remote oscillator register access */

/*
 * Integer scaling
 * BEWARE: compile-time rounding errors can occur when scaling very small values.
 */

#define SCALING_FACTOR				10000		/* Float to int scaling factor */
#define FLOAT_TO_INT(x)				((int)((x)*SCALING_FACTOR))
#define FROM_MV(x)				((x)*10)
#define TO_MV(x)				((x)/10)	/* Millivolts */
#define TO_UV(x)				((x)/10)	/* Microvolts */
#define TO_KHZ(x)				((x)/10)	/* KiloHertz */
#define TO_MDC(x)				((x)/10)	/* Millidegrees Celcius */

/*
 * Various
 */

#define REMOTE_ROSC_CONTROL_VALID		(BCHP_AVS_PMB_S_000_AVS_ROSC_CONTROL_VALID_S | BCHP_AVS_PMB_S_000_AVS_ROSC_CONTROL_VALID_H)
#define NUMBER_OF_OSCILLATORS(sih)		(NUMBER_OF_CENTRALS + 2*NUMBER_OF_REMOTES(sih))
#define NUMBER_OF_OSCILLATORS_MAX		(NUMBER_OF_CENTRALS + 2*NUMBER_OF_REMOTES_MAX)
#define IGNORE_OSCILLATOR			0
#define INVALID_OSCILLATOR_COUNT		0

/*
 * Target performance for all oscillators
 *
 * These are central oscillator targets based on the measured or predicted worst-case
 * timing of the design. Remote oscillator targets are derived from these in @see get_rosc_threshold.
 */

 typedef uint osc_freq_t;

 /* 43684 central oscillator frequency thresholds at 0.92V, SS, 125C condition, units are MHz */
static const osc_freq_t oscillator_thresholds_43684[] = {
	/*  0 */	FLOAT_TO_INT(2.7490),
	/*  1 */	FLOAT_TO_INT(2.5953),
	/*  2 */	FLOAT_TO_INT(2.1252),
	/*  3 */	FLOAT_TO_INT(1.3756),
	/*  4 */	FLOAT_TO_INT(3.7255),
	/*  5 */	FLOAT_TO_INT(3.4551),
	/*  6 */	FLOAT_TO_INT(2.8331),
	/*  7 */	FLOAT_TO_INT(1.8509),
	/*  8 */	FLOAT_TO_INT(10.722),
	/*  9 */	FLOAT_TO_INT(9.8360),
	/* 10 */	FLOAT_TO_INT(8.1625),
	/* 11 */	FLOAT_TO_INT(5.2245),
	/* 12 */	FLOAT_TO_INT(7.7715),
	/* 13 */	FLOAT_TO_INT(6.8730),
	/* 14 */	FLOAT_TO_INT(5.7190),
	/* 15 */	FLOAT_TO_INT(3.5984),
	/* 16 */	FLOAT_TO_INT(7.2590),
	/* 17 */	FLOAT_TO_INT(6.3300),
	/* 18 */	FLOAT_TO_INT(5.3105),
	/* 19 */	FLOAT_TO_INT(3.4723),
	/* 20 */	FLOAT_TO_INT(4.2323),
	/* 21 */	FLOAT_TO_INT(3.6809),
	/* 22 */	IGNORE_OSCILLATOR,
	/* 23 */	IGNORE_OSCILLATOR
};

 /* 6710 central oscillator frequency thresholds at 0.92V, SS, 125C condition, units are MHz */
static const osc_freq_t oscillator_thresholds_6710[] = {
	/*  0 */	FLOAT_TO_INT(2.7490),
	/*  1 */	FLOAT_TO_INT(2.5953),
	/*  2 */	FLOAT_TO_INT(2.1252),
	/*  3 */	FLOAT_TO_INT(1.3756),
	/*  4 */	FLOAT_TO_INT(3.7255),
	/*  5 */	FLOAT_TO_INT(3.4551),
	/*  6 */	FLOAT_TO_INT(2.8331),
	/*  7 */	FLOAT_TO_INT(1.8509),
	/*  8 */	FLOAT_TO_INT(10.722),
	/*  9 */	FLOAT_TO_INT(9.8360),
	/* 10 */	FLOAT_TO_INT(8.1625),
	/* 11 */	FLOAT_TO_INT(5.2245),
	/* 12 */	FLOAT_TO_INT(7.7715),
	/* 13 */	FLOAT_TO_INT(6.8730),
	/* 14 */	FLOAT_TO_INT(5.7190),
	/* 15 */	FLOAT_TO_INT(3.5984),
	/* 16 */	FLOAT_TO_INT(7.2590),
	/* 17 */	FLOAT_TO_INT(6.3300),
	/* 18 */	FLOAT_TO_INT(5.3105),
	/* 19 */	FLOAT_TO_INT(3.4723),
	/* 20 */	FLOAT_TO_INT(4.2323),
	/* 21 */	FLOAT_TO_INT(3.6809),
	/* 22 */	IGNORE_OSCILLATOR,
	/* 23 */	IGNORE_OSCILLATOR
};

/*
 * Low Voltage Mode (LVM)
 */

#ifdef AVS_ENABLE_LVM
#define LVM_SUPPORT(sih)			BCM43684_CHIP((sih)->chip) /* 43684 only for now */
#else
#define LVM_SUPPORT(sih)			FALSE
#endif /* AVS_ENABLE_LVM */

/* 43684-specific PMU ChipControl0 bits for LVM */
#define BCM43684_FORCE_MEM_LVM_EN		0x400	/* Force memory into LVM mode */
#define BCM43684_PMU_RSRC_CNTRL_LVM_EN		0x800	/* Enable PMU resource control to control LVM mode of memory */

/*
 *
 * Private data structures
 *
 */

typedef enum {
	PVT_TEMPERATURE = 0,			/* Bit 0 - Temperature measurement */
	PVT_0P85V_0     = 1,			/* Bit 1 - Voltage 0p85V<0> measurement */
	PVT_0P85V_1     = 2,			/* Bit 2 - Voltage 0p85V<1> measurement */
	PVT_1V_0        = 3,			/* Bit 3 - Voltage 1V<0> measurement */
	PVT_1V_1        = 4,			/* Bit 4 - Voltage 1V<1> measurement */
	PVT_1p8V        = 5,			/* Bit 5 - Voltage 1p8V measurement */
	PVT_3p3V        = 6,			/* Bit 6 - Voltage 3p3V measurement */
	PVT_TESTMODE    = 7			/* Bit 7 - Testmode measurement */
} pvt_t;

typedef enum {
	RESET_PVT	= 0x1,			/* Reset PVT registers */
	RESET_CENTRAL	= 0x2,			/* Reset centrol oscillator counts */
	RESET_REMOTE	= 0x4,			/* Reset remote oscillator counts */
	RESET_ALL	= 0xff			/* Reset all */
} reset_t;

typedef uint32 dac_code_t;
typedef uint osc_count_t;

typedef struct {
	int		slope;
	int		intercept;		/* y-intercept */
} linear_eq_t;

typedef struct {
	osc_count_t	low;			/* Lower threshold (counts) */
	osc_count_t	high;			/* Upper threshold (counts) */
} thresholds_t;

typedef struct {
	uint		pass;
	osc_count_t	initial_count[NUMBER_OF_OSCILLATORS_MAX];	/* Upper voltage oscillator counts */
	uint		threshold_voltage[NUMBER_OF_OSCILLATORS_MAX];	/* Calculated minimum oscillator voltage */
	int		slope[NUMBER_OF_OSCILLATORS_MAX];		/* Calculated oscillator slope (counts) */
	dac_code_t	dac_low;		/* Lower DAC code */
	dac_code_t	dac_high;		/* Upper DAC code */
	uint		vlow;			/* Measured voltage at lower DAC code */
	uint		vhigh;			/* Measured voltage at upper DAC code */
	uint		vconv;			/* Calculated convergence voltage */
} predict_context_t;

struct avs_context {
	osl_t		*osh;			/* OSL handle */
	si_t		*sih;			/* SI handle */
#ifdef DONGLEBUILD
	osl_ext_mutex_t status_lock;		/* Global lock */
#else
	struct semaphore status_lock;		/* Global lock */
	void		*reg_ops_context;	/* Context for register read/write operations */
#endif // endif
	bool		initialized;
	uint		iterations;
	uint		dac_adjustments[2];	/* Down/up DAC adjustment count */
	bool		dac_enabled;
	uint		function_mode;		/* Current AVS function mode */
	dac_code_t	dac_code;		/* Current DAC code */
	dac_code_t	dac_code_max;		/* Upper limit, corresponding to vlimit_high */
	dac_code_t	dac_code_min;		/* Lower limit, corresponding to vlimit_low */

	uint		dac_min_vstep;		/* Voltage step corresponding to 1 DAC code (mV) */
	uint		reg_settle_time;	/* Voltage regulator settle time (us) */
	dac_code_t	initial_dac_code;	/* Initial DAC code */
	dac_code_t	local_fit_dac_offset;	/* DAC offset for local fit */

	uint		vmargin_low;		/* Margin given in FF corner */
	uint		vmargin_high;		/* Margin given in SS corner */
	uint		vlimit_low;		/* Minimum operating voltage */
	uint		vlimit_high;		/* Maximum operating voltage */
	bool		use_dac_stepping;	/* Use small DAC increments to avoid voltage spikes */

	linear_eq_t	voltage_margin;		/* Relation between convergence voltage and margin */
	linear_eq_t	voltage_daccode;	/* Relation between voltage and DAC code */

	thresholds_t	thresholds[NUMBER_OF_OSCILLATORS_MAX];	/* Calculated thresholds (counts) */
	uint		threshold_margin;	/* Oscillator frequency hysteresis (V) */
	bool		lvm_state;		/* Memory Low Voltage Mode state */

	uint		last_voltage;
	int		last_avs_temp;
	int		last_arm_temp;

#if defined(BCMDBG) || defined(AVS_ENABLE_STATUS)
	dac_code_t	dac_low;		/* Lower DAC code during prediction */
	dac_code_t	dac_high;		/* Upper DAC code during prediction */
	uint		vlow;			/* Measured voltage at lower DAC code */
	uint		vhigh;			/* Measured voltage at upper DAC code */
	uint		vconv;			/* Calculated convergence voltage */
	uint		vmargin;		/* Calculated margin */
	uint		vsum;
	uint		voltage_feedback;	/* Measured regulator feedback voltage */
	uint		voltage_open;		/* Open loop voltage */
	dac_code_t	board_dac_code;		/* DAC code corresponding to open loop voltage */
	dac_code_t	dac_range[2];		/* Lowest/highest DAC code */
	uint		voltage_range[2];	/* Lowest/highest voltage */
	int		temp_range[2];		/* Lowest/highest temperature */
	uint		last_converge_time_us;	/* Duration of last convergence phase */
	uint		last_track_time_us;	/* Duration of last tracking phase */
#endif // endif
};

/*
 *
 * Internal Functions
 *
 */

static inline void
sleep(uint us)
{
/* Need this because OSL_SLEEP differs between RTE and Linux */
#ifdef DONGLEBUILD
	hnd_thread_sleep(us);
#else
	osl_sleep(us > 1000 ? us / 1000 : 1);
#endif /* DONGLEBUILD */
}

/**
 * Initialize global lock.
 *
 * Global lock will provide mutual exclusion of access to AVS state between AVS thread and
 * the main thread.
 *
 * @param avs		Pointer to AVS context.
 */

static inline void
init_lock(avs_context_t *avs)
{
#ifdef DONGLEBUILD
	osl_ext_mutex_create(0, &avs->status_lock);
#else
	sema_init(&avs->status_lock, 1);
#endif // endif
}

/**
 * Acquire global lock.
 *
 * @param avs		Pointer to AVS context.
 * @param timeout	Timeout in ms, OSL_EXT_TIME_FOREVER, or 0 to disable waiting.
 * @return		TRUE if lock was aquired.
 */

static inline bool
lock(avs_context_t *avs, osl_ext_time_ms_t timeout)
{
#ifdef DONGLEBUILD
	return osl_ext_mutex_acquire(&avs->status_lock, timeout) == OSL_EXT_SUCCESS;
#else
	if (timeout == 0) {
		return down_trylock(&avs->status_lock) == 0;
	} else if (timeout == OSL_EXT_TIME_FOREVER) {
		return down_interruptible(&avs->status_lock) == 0;
	} else {
		return down_timeout(&avs->status_lock, msecs_to_jiffies(timeout)) == 0;
	}
#endif // endif
}

/**
 * Release global lock.
 *
 * @param avs		Pointer to AVS context.
 */

static inline void
unlock(avs_context_t *avs)
{
#ifdef DONGLEBUILD
	osl_ext_mutex_release(&avs->status_lock);
#else
	up(&avs->status_lock);
#endif // endif
}

/**
 * Write register.
 *
 * @param avs		Pointer to AVS context.
 * @param address	Memory address.
 * @param value		Value to write.
 */

static inline void
write_register(avs_context_t *avs, uint32 address, uint32 value)
{
#ifdef DONGLEBUILD
	W_REG(avs->osh, (uint32*)address, value);
#else
	wl_backplane_write(avs->reg_ops_context, address, value);
#endif // endif
}

/**
 * Write PMU register.
 *
 * @param avs		Pointer to AVS context.
 * @param address	Memory address.
 * @param mask		Mask.
 * @param value		Value to write.
 */

static inline void
write_pmu_register(avs_context_t *avs, uint32 address, uint32 mask, uint32 value)
{
	ASSERT(mask || value);
#ifdef DONGLEBUILD
	si_pmu_chipcontrol(avs->sih, address, mask, value);
#else
	wl_pmu_access(avs->reg_ops_context, address, mask, value);
#endif // endif
}

/**
 * Read register.
 *
 * @param avs		Pointer to AVS context.
 * @param address	Register address.
 * @return		Value.
 */

static inline uint32
read_register(avs_context_t *avs, uint32 address)
{
#ifdef DONGLEBUILD
	return R_REG(avs->osh, (uint32*)address);
#else
	return wl_backplane_read(avs->reg_ops_context, address);
#endif // endif
}

/**
 * Modify register.
 *
 * Uses read-modify-write to modify register bits indicated by a mask.
 *
 * @param avs		Pointer to AVS context.
 * @param address	Register address.
 * @param value		Value.
 * @param mask		Mask.
 */

static void
modify_register(avs_context_t *avs, uint32 address, uint32 value, uint32 mask)
{
	ASSERT((value & ~mask) == 0);

	write_register(avs, address, (read_register(avs, address) & ~mask) | (value & mask));
}

/**
 * Pulse register.
 *
 * Write a value, then clear the register.
 *
 * @param avs		Pointer to AVS context.
 * @param address	Register address.
 * @param value		Value.
 */

static void
pulse_register(avs_context_t *avs, uint32 address, uint32 value)
{
	write_register(avs, address, value);
	write_register(avs, address, 0x0);
}

#ifdef BCMDBG

/**
 * Convert oscillator count to frequency.
 *
 * @param count		Oscillator count.
 * @return		Oscillator frequency (MHz * SCALING_FACTOR)
 */

static osc_freq_t
rosc_count_to_freq(avs_context_t *avs, osc_count_t count)
{
	STATIC_ASSERT(SCALING_FACTOR == 10000);

	/* This assumes oscillator is configured to count rising edges only */
	/* To avoid overflows, divide by less than the scaling factor and correct afterwards */
	return (osc_freq_t)((count * ROSC_FREQ(avs->sih) * (SCALING_FACTOR/10)) / ROSC_MEASUREMENT_TIME) * (SCALING_FACTOR/1000);
}

#endif /* BCMDBG */

/**
 * Convert oscillator frequency to count.
 *
 * @param freq		Oscillator frequency (MHz * SCALING_FACTOR)
 * @return		Oscillator count.
 */

static osc_count_t
rosc_freq_to_count(avs_context_t *avs, osc_freq_t freq)
{
	/* This assumes oscillator is configured to count rising edges only */
	return (osc_count_t)(((freq / ROSC_FREQ(avs->sih)) * ROSC_MEASUREMENT_TIME) / SCALING_FACTOR);
}

/**
 * Get the performance threshold (target counts) of the specified ring oscillator.
 *
 * @param osc_index	Linear oscillator index.
 * @return		Target count, or IGNORE_OSCILLATOR.
 */

static osc_count_t
get_rosc_threshold(avs_context_t *avs, uint osc_index)
{
	osc_freq_t freq = IGNORE_OSCILLATOR;

	ASSERT(osc_index < NUMBER_OF_OSCILLATORS(avs->sih));

	switch (CHIPID(avs->sih->chip)) {
		case BCM6710_CHIP_ID:
			STATIC_ASSERT(NUMBER_OF_CENTRALS <= sizeof(oscillator_thresholds_6710) / sizeof(oscillator_thresholds_6710[0]));
			if (osc_index < NUMBER_OF_CENTRALS) {
				/* Central oscillators have hardcoded thresholds */
				freq = oscillator_thresholds_6710[osc_index];
			} else {
				/* Remote oscillators share a threshold based on type */
				osc_index -= NUMBER_OF_CENTRALS;
				freq = (osc_index % 2) ?
					oscillator_thresholds_6710[3] :		/* Remote HVT ROSC is a copy of Central ROSC 3 */
					oscillator_thresholds_6710[1];		/* Remote SVT ROSC is a copy of Central ROSC 1 */
			}
			break;
		case BCM43684_CHIP_ID:
		default:
			STATIC_ASSERT(NUMBER_OF_CENTRALS <= sizeof(oscillator_thresholds_43684) / sizeof(oscillator_thresholds_43684[0]));
			if (osc_index < NUMBER_OF_CENTRALS) {
				/* Central oscillators have hardcoded thresholds */
				freq = oscillator_thresholds_43684[osc_index];
			} else {
				/* Remote oscillators share a threshold based on type */
				osc_index -= NUMBER_OF_CENTRALS;
				freq = (osc_index % 2) ?
					oscillator_thresholds_43684[3] :	/* Remote HVT ROSC is a copy of Central ROSC 3 */
					oscillator_thresholds_43684[1];		/* Remote SVT ROSC is a copy of Central ROSC 1 */
			}
			break;
	}

	return (freq != IGNORE_OSCILLATOR) ? rosc_freq_to_count(avs, freq) : IGNORE_OSCILLATOR;
}

/**
 * Convert voltage to DAC code.
 *
 * @param voltage_daccode	Linear equation for voltage-daccode relation.
 * @param voltage		Voltage (V * SCALING_FACTOR).
 * @return			DAC code.
 */

static dac_code_t
voltage_to_dac_code(linear_eq_t voltage_daccode, uint voltage)
{
	dac_code_t dac_code;

	ASSERT(!(voltage_daccode.slope == 0 && voltage_daccode.intercept == 0));

	dac_code = (dac_code_t)(((voltage_daccode.slope * voltage) / SCALING_FACTOR) + voltage_daccode.intercept);
	AVS_ERROR_IF(dac_code == INVALID_DAC_CODE || dac_code > MAX_DAC_CODE, ("%s: invalid dac_code %u, v=%u\n", __FUNCTION__, dac_code, voltage));

	return dac_code;
}

/**
 * Convert voltage to voltage margin.
 *
 * @param voltage_margin	Linear equation for voltage-margin relation.
 * @param voltage		Voltage (V * SCALING_FACTOR).
 * @return			Voltage margin (V * SCALING_FACTOR).
 */

static uint
voltage_to_margin(linear_eq_t voltage_margin, uint voltage)
{
	int margin;

	ASSERT(!(voltage_margin.slope == 0 && voltage_margin.intercept == 0));

	margin = (int)(((voltage_margin.slope * voltage) / SCALING_FACTOR) + voltage_margin.intercept);
	AVS_ERROR_IF(margin < 0, ("%s: invalid margin %u, v=%u\n", __FUNCTION__, margin, voltage));

	return (uint)margin;
}

/**
 * Read the most recent CENTRAL oscillator count.
 *
 * @param avs		Pointer to AVS context.
 * @param osc_num	Oscillator index.
 * @return		Oscillator count, or INVALID_OSCILLATOR_COUNT on invalid index or read error.
 */

static osc_count_t
read_central_rosc_count(avs_context_t *avs, uint osc_num)
{
	int i = PVT_REGISTER_REREAD_COUNT;
	uint count;
	uint32 reg;

	ASSERT(osc_num < NUMBER_OF_CENTRALS);

	if (osc_num >= NUMBER_OF_CENTRALS) {
		return 0;
	}

	while(i--) {
		reg = read_register(avs, BCHP_AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_0 + (osc_num * 4));
		if (reg & BCHP_AVS_RO_REGISTERS_0_PVT_TESTMODE_MNTR_STATUS_done_MASK) {
			count = reg & BCHP_AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_0_data_MASK;
			return count >= ROSC_DETECT_THRESHOLD ? count : INVALID_OSCILLATOR_COUNT;
		}

		sleep(PVT_REGISTER_REREAD_DELAY_US);
	}

	AVS_ERROR(("%s: failed to read central OSC %u\n", __FUNCTION__, osc_num));

	return INVALID_OSCILLATOR_COUNT;
}

/**
 * Read the most recent REMOTE oscillator count pair.
 *
 * @param avs		Pointer to AVS context.
 * @param osc_num	Oscillator index.
 * @param svt_count	SVT oscillator count, or INVALID_OSCILLATOR_COUNT on invalid index or read error.
 * @param hvt_count	HVT oscillator count, or INVALID_OSCILLATOR_COUNT on invalid index or read error.
 */

static void
read_remote_rosc_count(avs_context_t *avs, uint osc_num, osc_count_t *svt_count, osc_count_t *hvt_count)
{
	int i = PVT_REGISTER_REREAD_COUNT;
	uint32 reg;

	ASSERT(osc_num < NUMBER_OF_REMOTES(avs->sih));
	ASSERT(svt_count != NULL && hvt_count != NULL);

	*svt_count = *hvt_count = INVALID_OSCILLATOR_COUNT;

	if (osc_num >= NUMBER_OF_REMOTES(avs->sih)) {
		return;
	}

	while(i--) {
		/* Read control register */
		reg = read_register(avs, BCHP_AVS_PMB_S_000_AVS_ROSC_CONTROL + (osc_num * BCHP_AVS_PMB_OFFSET));
		if ((reg & REMOTE_ROSC_CONTROL_VALID) == REMOTE_ROSC_CONTROL_VALID) {
			/* Read counters */
			sleep(PMB_REMOTE_ROSC_DELAY_US);
			reg = read_register(avs, BCHP_AVS_PMB_S_000_AVS_ROSC_COUNT + (osc_num * BCHP_AVS_PMB_OFFSET));
			sleep(PMB_REMOTE_ROSC_DELAY_US);

			*svt_count = (reg & BCHP_AVS_PMB_S_000_AVS_ROSC_COUNT_COUNT_S_MASK) >> BCHP_AVS_PMB_S_000_AVS_ROSC_COUNT_COUNT_S_SHIFT;
			*hvt_count = (reg & BCHP_AVS_PMB_S_000_AVS_ROSC_COUNT_COUNT_H_MASK) >> BCHP_AVS_PMB_S_000_AVS_ROSC_COUNT_COUNT_H_SHIFT;

			/* Ignore any remote that returns an oscillator value below the threshold */
			if (*svt_count < ROSC_DETECT_THRESHOLD) *svt_count = INVALID_OSCILLATOR_COUNT;
			if (*hvt_count < ROSC_DETECT_THRESHOLD) *hvt_count = INVALID_OSCILLATOR_COUNT;
			return;
		}

		sleep(PVT_REGISTER_REREAD_DELAY_US);
	}

	AVS_ERROR(("%s: failed to read remote OSC %u\n", __FUNCTION__, osc_num));
}

/**
 * Initialize central and remote oscillators.
 *
 * @param avs		Pointer to AVS context.
 */

static void
initialize_oscillators(avs_context_t *avs)
{
	int i;

	/* Initialize the sensor sequencer */
	pulse_register(avs, BCHP_AVS_HW_MNTR_SEQUENCER_INIT, 0x1);

	/* Specify the PVT monitor sensors to measure */
	write_register(avs, BCHP_AVS_HW_MNTR_SEQUENCER_MASK_PVT_MNTR, PVT_MONITOR_MASK);

	/* Specify the central oscillators to measure */
	write_register(avs, BCHP_AVS_HW_MNTR_SEQUENCER_MASK_CEN_ROSC_0, CEN_ROSC_MASK_0);
#if (NUMBER_OF_CENTRALS > 31)
	write_register(avs, BCHP_AVS_HW_MNTR_SEQUENCER_MASK_CEN_ROSC_1, CEN_ROSC_MASK_1);
#endif // endif

	/* Set reference counter interval */
	write_register(avs, BCHP_AVS_HW_MNTR_ROSC_MEASUREMENT_TIME_CONTROL, ROSC_MEASUREMENT_TIME_CONTROL);

	/* Control counting event for CENTRAL ROSC signal counter */
	write_register(avs, BCHP_AVS_HW_MNTR_ROSC_COUNTING_MODE, 1);		/* Rising edge only */

	/* Configure remote oscillators (which are on the PMB bus) */
	for(i=0; i<NUMBER_OF_REMOTES(avs->sih); i++) {
		/* Enable oscillators, counters, and set to continuous counting mode */
		write_register(avs, BCHP_AVS_PMB_S_000_AVS_ROSC_CONTROL + (i * BCHP_AVS_PMB_OFFSET),
			BCHP_AVS_PMB_S_000_AVS_ROSC_CONTROL_RO_EN_S |
			BCHP_AVS_PMB_S_000_AVS_ROSC_CONTROL_RO_EN_H |
			BCHP_AVS_PMB_S_000_AVS_ROSC_CONTROL_ECTR_EN_S |
			BCHP_AVS_PMB_S_000_AVS_ROSC_CONTROL_ECTR_EN_H |
			BCHP_AVS_PMB_S_000_AVS_ROSC_CONTROL_CONTINUOUS_S |
			BCHP_AVS_PMB_S_000_AVS_ROSC_CONTROL_CONTINUOUS_H |
			(ROSC_MEASUREMENT_TIME << BCHP_AVS_PMB_S_000_AVS_ROSC_CONTROL_TEST_INTERVAL_SHIFT));
		sleep(PMB_REMOTE_ROSC_DELAY_US);
	}
}

/**
 * Convert raw PVT monitor sensor data to value.
 *
 * Conversion formulas are from PVTMON specification v5.2
 *
 * @param pvt		PVT monitor sensor.
 * @param reg		Raw register value.
 * @return		Sensor value, or 0 on error.
 */

static int
pvt_register_to_value(pvt_t pvt, uint32 reg)
{
	AVS_ERROR_IF(reg == INVALID_PVT_VALUE || reg > MAX_DAC_CODE, ("%s: invalid value %u(0x%x)\n", __FUNCTION__, reg, reg));

	if (reg == INVALID_PVT_VALUE) {
		return 0;
	}

	switch(pvt) {
		case PVT_TEMPERATURE:
			/* Convert PVT measurement to temperature, unit is C * SCALING_FACTOR
			 *
			 *	t = 413.35 - (adc * 0.49055)
			 */
			return FLOAT_TO_INT(413.35) - ((int)reg * FLOAT_TO_INT(0.49055));

		case PVT_1V_0:
			/* Convert PVT measurement to voltage, unit is V * SCALING_FACTOR
			 *
			 *	v = adc / 1024 * ADC_FULL_SCALE_VOLTAGE / 0.7
			 *	v = (ADC_FULL_SCALE_VOLTAGE * adc) / 716.8
			 *	v = (ADC_FULL_SCALE_VOLTAGE * adc * 10) / 7168
			 */
			return (reg * FLOAT_TO_INT(ADC_FULL_SCALE_VOLTAGE) * 10) / 7168;

		default:
			ASSERT(FALSE);
			return 0;
	}
}

#if defined(BCMDBG) || defined(AVS_ENABLE_STATUS)

static int
raw_adc_register_to_voltage(uint32 reg)
{
	AVS_ERROR_IF(reg > MAX_ADC_CODE, ("%s: invalid value %u(0x%x)\n", __FUNCTION__, reg, reg));

	/* Convert raw ADC measurement to voltage, unit is V * SCALING_FACTOR
	 *
	 *	v = adc / 1024 * ADC_FULL_SCALE_VOLTAGE
	 *	v = (ADC_FULL_SCALE_VOLTAGE * adc) / 1024
	 */
	return (reg * FLOAT_TO_INT(ADC_FULL_SCALE_VOLTAGE)) / 1024;
}

#endif // endif

static dac_code_t
raw_adc_register_to_dac_code(uint32 reg)
{
	AVS_ERROR_IF(reg > MAX_ADC_CODE, ("%s: invalid value %u(0x%x)\n", __FUNCTION__, reg, reg));

	/* Convert raw ADC measurement to the corresponding DAC code
	 *
	 *	reg = round(1658.2 - 1.3892 * dac)
	 *	dac = round((1658.2 - reg) / 1.3892)
	 */
	return (FLOAT_TO_INT(1658.2) - (reg * SCALING_FACTOR) + SCALING_FACTOR) / FLOAT_TO_INT(1.3892);
}

/**
 * Reset the measurement valid bits of all PVT monitor sensors and central oscillators.
 *
 * @param avs		Pointer to AVS context.
 * @param reset		Category to reset.
 */

static void
reset_measurements(avs_context_t *avs, reset_t reset)
{
	int i;

	if (reset & RESET_PVT) {
		/* Reset PVT monitor sensors */
		pulse_register(avs, BCHP_AVS_HW_MNTR_MEASUREMENTS_INIT_PVT_MNTR, BCHP_AVS_HW_MNTR_MEASUREMENTS_INIT_PVT_MNTR_m_init_pvt_mntr_MASK);
	}

	if (reset & RESET_CENTRAL) {
		/* Reset central oscillators */
		pulse_register(avs, BCHP_AVS_HW_MNTR_MEASUREMENTS_INIT_CEN_ROSC_0, BCHP_AVS_HW_MNTR_MEASUREMENTS_INIT_CEN_ROSC_0_m_init_cen_rosc_MASK);
#if (NUMBER_OF_CENTRALS > 31)
		pulse_register(avs, BCHP_AVS_HW_MNTR_MEASUREMENTS_INIT_CEN_ROSC_1, BCHP_AVS_HW_MNTR_MEASUREMENTS_INIT_CEN_ROSC_1_m_init_cen_rosc_MASK);
#endif // endif
	}

	if (reset & RESET_REMOTE) {
		/* Reset remote oscillators */
		for (i=0; i<NUMBER_OF_REMOTES(avs->sih); i++) {
			/* Read counters to clear valid bits in counter control registers */
			read_register(avs, BCHP_AVS_PMB_S_000_AVS_ROSC_COUNT + (i * BCHP_AVS_PMB_OFFSET));
			sleep(PMB_REMOTE_ROSC_DELAY_US);
		}
	}
}

/**
 * Read PVT monitor sensor data.
 *
 * The sensor sequencer continuously measures all relevant sensors, and this function returns the
 * result of the last measurement. Use @see reset_measurements to clear 'measurement valid' bits to
 * make sure this function polls the register until a new measurement is completed.
 *
 * @param avs		Pointer to AVS context.
 * @param pvt		PVT monitor sensor.
 * @param averaging	Calculate average of 2^averaging measurements.
 * @return		Raw sensor value, or INVALID_PVT_VALUE on error.
 */

static uint32
read_pvt_register_single(avs_context_t *avs, pvt_t pvt)
{
	int i = PVT_REGISTER_REREAD_COUNT;
	uint32 reg;

	while(i--) {
		switch(pvt) {
			case PVT_TEMPERATURE:
				reg = read_register(avs, BCHP_AVS_RO_REGISTERS_0_PVT_TEMPERATURE_MNTR_STATUS);
				break;
			case PVT_1V_0:
				reg = read_register(avs, BCHP_AVS_RO_REGISTERS_0_PVT_1V_0_MNTR_STATUS);
				break;
			default:
				ASSERT(FALSE);
				return INVALID_PVT_VALUE;
		}

		/* Check if we got a valid reading */
		if (reg & BCHP_AVS_RO_REGISTERS_0_PVT_TESTMODE_MNTR_STATUS_done_MASK) {
			ASSERT(reg & BCHP_AVS_RO_REGISTERS_0_PVT_TESTMODE_MNTR_STATUS_valid_data_MASK);
			return (reg & BCHP_AVS_RO_REGISTERS_0_PVT_TESTMODE_MNTR_STATUS_data_MASK);
		}

		sleep(PVT_REGISTER_REREAD_DELAY_US);
	}

	AVS_ERROR(("%s: failed to read pvt %u\n", __FUNCTION__, pvt));

	return INVALID_PVT_VALUE;
}

/**
 * Read PVT monitor sensor data with averaging.
 *
 * Calls @see read_pvt_register_single multiple times and returns the average value.
 *
 * @param avs		Pointer to AVS context.
 * @param pvt		PVT monitor sensor.
 * @return		Raw sensor value, or INVALID_PVT_VALUE on error.
 */

static uint32
read_pvt_register(avs_context_t *avs, pvt_t pvt, uint averaging)
{
	if (averaging == 0) {
		return read_pvt_register_single(avs, pvt);
	} else {
		uint i = 1U << averaging;
		uint32 value, total = 0;

		while(i--) {
			value = read_pvt_register_single(avs, pvt);
			if (value == INVALID_PVT_VALUE) {
				return value;
			}
			total += value;
			if (i != 0) {
				reset_measurements(avs, RESET_PVT);
				sleep(PVT_REGISTER_READ_DELAY_US);
			}
		}

		return total >> averaging;
	}
}

/**
 * Read PVT monitor sensor data and convert to value.
 *
 * @param avs		Pointer to AVS context.
 * @param pvt		PVT monitor sensor.
 * @param averaging	Calculate average of 2^averaging measurements.
 * @return		Sensor value, or INVALID_PVT_VALUE on error.
 */

static int
read_pvt_value(avs_context_t *avs, pvt_t pvt, uint averaging)
{
	return pvt_register_to_value(pvt, read_pvt_register(avs, pvt, averaging));
}

#if defined(BCMDBG) || defined(AVS_ENABLE_STATUS)

/**
 * Read TMON sensor data and convert to value.
 *
 * @param avs		Pointer to AVS context.
 * @return		Sensor temperature value, or 0 on error.
 */

static int
read_tmon_value(avs_context_t *avs)
{
	uint32 reg;

	if (!BCM6710_CHIP(avs->sih->chip)) {
		reg = read_register(avs, BCHP_AVS_TMON_TEMPERATURE_MEASUREMENT_STATUS);
		if (reg & BCHP_AVS_TMON_TEMPERATURE_MEASUREMENT_VALID) {
			/* After right shift, TMON conversion is same as AVS TMON */
			return pvt_register_to_value(PVT_TEMPERATURE, (reg & BCHP_AVS_TMON_TEMPERATURE_MEASUREMENT_DATA_MASK) >> 1);
		}
	}

	return 0;
}

#endif // endif

/**
 * Configure the PVTMON function mode.
 *
 * @param avs		Pointer to AVS context.
 * @param mode		Function mode.
 */

static void
set_function_mode(avs_context_t *avs, uint32 mode, bool force)
{
	ASSERT(mode <= BCHP_AVS_PVT_MNTR_CONFIG_PVT_MNTR_CTRL_MODE_EXPERT);

	if (force == FALSE && mode == avs->function_mode) {
		return;
	}

	/* When switching to DAC driving mode, make sure DAC is configured correctly */
	ASSERT(mode != BCHP_AVS_PVT_MNTR_CONFIG_PVT_MNTR_CTRL_MODE_DAC_DRIVE || avs->dac_enabled == TRUE);
	ASSERT(mode != BCHP_AVS_PVT_MNTR_CONFIG_PVT_MNTR_CTRL_MODE_DAC_DRIVE || avs->dac_code != INVALID_DAC_CODE);

	/* Set new mode */
	modify_register(avs, BCHP_AVS_PVT_MNTR_CONFIG_PVT_MNTR_CTRL,
		mode << BCHP_AVS_PVT_MNTR_CONFIG_PVT_MNTR_CTRL_MODE_SHIFT,
		BCHP_AVS_PVT_MNTR_CONFIG_PVT_MNTR_CTRL_MODE_MASK);

	avs->function_mode = mode;
}

/**
 * Set/reset the LVM bit.
 *
 * @param avs		Pointer to AVS context.
 * @param enable	TRUE to set LVM bit.
 */

static inline void
set_LVM_enable(avs_context_t *avs, bool enable)
{
	ASSERT(BCM43684_CHIP(avs->sih->chip));

	write_pmu_register(avs, PMU_CHIPCTL0, BCM43684_FORCE_MEM_LVM_EN,
		enable ? BCM43684_FORCE_MEM_LVM_EN : 0);
	avs->lvm_state = enable;
}

/**
 * Set the LVM flag if needed.
 *
 * Some LL SRAM memories in 28nm require enabling of the Low Voltage Mode (LVM) input
 * if the voltage is below 0.9V.
 *
 * @param avs		Pointer to AVS context.
 * @param voltage	Current voltage, or 0 to measure it.
 */

static void
check_LVM(avs_context_t *avs, uint voltage)
{
	if (LVM_SUPPORT(avs->sih)) {
		bool enable;

		/* Read curent voltage if needed */
		if (voltage == 0) {
			voltage = read_pvt_value(avs, PVT_1V_0, PVT_AVERAGING_FAST);
		}

		if (avs->lvm_state == FALSE && voltage <= FLOAT_TO_INT(LVM_ENABLE_THRESHOLD)) {
			enable = TRUE;
		} else if (avs->lvm_state == TRUE && voltage > FLOAT_TO_INT(LVM_ENABLE_THRESHOLD + LVM_ENABLE_HYSTERESIS)) {
			enable = FALSE;
		} else {
			return;
		}

		set_LVM_enable(avs, enable);
	}
}

#ifdef AVS_GET_BOARD_DAC_CODE

/**
 * Determine the DAC code corresponding to the open loop regulator feedback voltage.
 *
 * @note This changes the AVS function mode.
 *
 * @param avs		Pointer to AVS context.
 * @return		DAC code, or INVALID_DAC_CODE on error.
 */

static dac_code_t
get_board_dac_code(avs_context_t *avs)
{
	uint32 reg;
	dac_code_t dac_code = INVALID_DAC_CODE;

	/*
	 * Set PVTMON function mode to measure the feedback voltage (Vfb) of the external
	 * regulator through pad_ADC
	 */
	set_function_mode(avs, BCHP_AVS_PVT_MNTR_CONFIG_PVT_MNTR_CTRL_MODE_MEASURE_PAD_ADC, FALSE);

	sleep(avs->reg_settle_time);	/* Allow for settling */
	reset_measurements(avs, RESET_PVT);

	/* Read any PVT to get Vfb */
	reg = read_pvt_register(avs, PVT_1V_0, PVT_AVERAGING_ACCURATE);
	if (reg != INVALID_PVT_VALUE) {
		/* Convert the ADC value to the DAC code corresponding to Vfb */
		dac_code = raw_adc_register_to_dac_code(reg);
		if (dac_code == INVALID_DAC_CODE || dac_code > MAX_DAC_CODE) {
			AVS_ERROR(("%s: invalid DAC code, reg=%u(0x%08x) dac_code=%u\n", __FUNCTION__, reg, reg, dac_code));
			return INVALID_DAC_CODE;
		}
	}

	AVS_TRACE(("%s: adc_code=%u Vfb=%umV dac_code=%u\n", __FUNCTION__, reg, TO_MV(raw_adc_register_to_voltage(reg)), dac_code));

#if defined(BCMDBG) || defined(AVS_ENABLE_STATUS)
	avs->board_dac_code   = dac_code;
	avs->voltage_feedback = raw_adc_register_to_voltage(reg);
#endif // endif

	return dac_code;
}

#endif /* AVS_GET_BOARD_DAC_CODE */

static void
set_dac_enable(avs_context_t *avs, bool enable, bool force)
{
	if (force == FALSE && avs->dac_enabled == enable) {
		return;
	}

	/* When disabling the DAC, make sure we are not in DAC driving mode */
	ASSERT(enable || avs->function_mode != BCHP_AVS_PVT_MNTR_CONFIG_PVT_MNTR_CTRL_MODE_DAC_DRIVE);

	/* Set DAC state */
	modify_register(avs, BCHP_AVS_PVT_MNTR_CONFIG_PVT_MNTR_CTRL,
		enable ? BCHP_AVS_PVT_MNTR_CONFIG_PVT_MNTR_CTRL_DAC_ENABLE : 0,
		BCHP_AVS_PVT_MNTR_CONFIG_PVT_MNTR_CTRL_DAC_ENABLE);

	avs->dac_enabled = enable;
}

/**
 * Write DAC code.
 *
 * This function writes the specified code to the DAC register and enables the DAC
 * if needed.
 *
 * @param avs		Pointer to AVS context.
 * @param dac_code	DAC code.
 */

static void
write_dac_code(avs_context_t *avs, dac_code_t dac_code)
{
	AVS_TRACE(("%s: code=%u\n", __FUNCTION__, dac_code));

	ASSERT(dac_code != INVALID_DAC_CODE && dac_code <= MAX_DAC_CODE);

	/* Write DAC code */
	write_register(avs, BCHP_AVS_PVT_MNTR_CONFIG_DAC_CODE, dac_code & BCHP_AVS_PVT_MNTR_CONFIG_DAC_CODE_dac_code_MASK);
	write_register(avs, BCHP_AVS_PVT_MNTR_CONFIG_DAC_CODE_PROGRAMMING_ENABLE, 1);

	/* Make sure DAC is enabled */
	set_dac_enable(avs, TRUE, FALSE);

	/* Make sure we are in DAC driving mode */
	set_function_mode(avs, BCHP_AVS_PVT_MNTR_CONFIG_PVT_MNTR_CTRL_MODE_DAC_DRIVE, FALSE);

	sleep(avs->reg_settle_time);	/* Allow for settling */

	/* Read current voltage and update LVM state if needed */
	reset_measurements(avs, RESET_PVT);
	check_LVM(avs, 0);
}

/**
 * Reset AVS algorithm and restore hardware to defaults.
 *
 * @param avs		Pointer to AVS context.
 */

static void
avs_reset_internal(avs_context_t *avs)
{
	AVS_INFORM(("%s\n", __FUNCTION__));

	/* Stop controlling the voltage */
	set_function_mode(avs, BCHP_AVS_PVT_MNTR_CONFIG_PVT_MNTR_CTRL_MODE_VTMON, TRUE);
	set_dac_enable(avs, FALSE, TRUE);

	if (LVM_SUPPORT(avs->sih)) {
		/* Release memory LVM control */
		sleep(avs->reg_settle_time);
		set_LVM_enable(avs, FALSE);

		ASSERT(BCM43684_CHIP(avs->sih->chip));
		write_pmu_register(avs, PMU_CHIPCTL0, BCM43684_PMU_RSRC_CNTRL_LVM_EN, BCM43684_PMU_RSRC_CNTRL_LVM_EN);
	}

	/* Power down PVT monitor */
	/* Disable ROSCs */

	avs->initialized = FALSE;
	avs->iterations = 0;
}

/**
 * Set the DAC output voltage.
 *
 * If DAC stepping is enabled, large voltage adjustments are split up in multiple smaller steps
 * in order to avoid voltage spikes on certain boards.
 *
 * @param avs		Pointer to AVS context.
 * @param dac_code	New DAC code.
 */

static void
set_dac_voltage(avs_context_t *avs, dac_code_t dac_code)
{
	int delta, step_size;
	uint steps;

	AVS_ERROR_IF(dac_code == INVALID_DAC_CODE || dac_code > MAX_DAC_CODE, ("%s: invalid dac_code %u\n", __FUNCTION__, dac_code));

	/* After the convergence process (when the relation between voltage and DAC code is known)
	 * we calculated the DAC codes corresponding to the minimum and maximum allowable chip voltage.
	 * Make sure the new DAC code is within the allowable range.
	 */
	if (avs->dac_code_min != INVALID_DAC_CODE && avs->dac_code_max != INVALID_DAC_CODE) {
		dac_code = MIN(dac_code, avs->dac_code_max);
		dac_code = MAX(dac_code, avs->dac_code_min);
	}

	if (dac_code == INVALID_DAC_CODE || dac_code > MAX_DAC_CODE) {
		return;
	}

	if (dac_code == avs->dac_code) {
		return;
	}

	AVS_TRACE(("%s: old_code=%u new_code=%u limits=%u/%u\n", __FUNCTION__,
		avs->dac_code, dac_code, avs->dac_code_min, avs->dac_code_max));

	/* Only do DAC stepping if the cached DAC code is valid */
	if (avs->use_dac_stepping == TRUE && avs->dac_code != INVALID_DAC_CODE) {
		delta     = (int)(dac_code - avs->dac_code);
		steps     = ABS(delta) / DAC_STEP_SIZE;
		step_size = (dac_code > avs->dac_code) ? DAC_STEP_SIZE : -DAC_STEP_SIZE;

		while(steps--) {
			avs->dac_code += step_size;
			write_dac_code(avs, avs->dac_code);
		}
	}

	/* Set final value */
	if (avs->dac_code != dac_code) {
		avs->dac_code = dac_code;
		write_dac_code(avs, dac_code);
	}

	/* Make sure subsequent sensor reads return updated values */
	reset_measurements(avs, RESET_CENTRAL | RESET_REMOTE);

	AVS_TRACE(("v=%umV\n", TO_MV(read_pvt_value(avs, PVT_1V_0, PVT_AVERAGING_ACCURATE))));
}

/**
 * Helper function to check the thresholds of the specified oscillator.
 *
 * This function uses linear oscillator indices, where central and remote oscillators are mapped
 * to a linear range from 0 to (NUMBER_OF_CENTRALS + 2 * NUMBER_OF_REMOTES) to make iteration easier.
 *
 * The increase and decrease flags will be manipulated to implement a hysteresis. The increase
 * flag is assumed to be cleared and the decrease flag is assumed to be set before checking
 * the first threshold.
 *
 * @param avs		Pointer to AVS context.
 * @param osc_index	Linear oscillator index.
 * @param count		Current oscillator count.
 * @param increase	Pointer to increase flag.
 * @param decrease	Pointer to decrease flag.
 */

static void
check_thresholds(avs_context_t *avs, uint osc_index, osc_count_t count, bool *increase, bool *decrease)
{
	ASSERT(increase != NULL && decrease != NULL);

	AVS_TRACE(("%c OSC%02d ",
		osc_index < NUMBER_OF_CENTRALS ? 'C' : 'R',
		osc_index < NUMBER_OF_CENTRALS ? osc_index : osc_index - NUMBER_OF_CENTRALS));

	if (avs->thresholds[osc_index].low == IGNORE_OSCILLATOR && count != INVALID_OSCILLATOR_COUNT) {
		/* Oscillator is configured to ignore */
		AVS_TRACE(("IGNORE\n"));
		return;
	}

	if (count == INVALID_OSCILLATOR_COUNT) {
		/* Oscillator is returning invalid counts, ignore */
		AVS_TRACE(("INVALID\n"));
		AVS_ERROR(("%s: invalid count for %c OSC%02d\n", __FUNCTION__,
			osc_index < NUMBER_OF_CENTRALS ? 'C' : 'R',
			osc_index < NUMBER_OF_CENTRALS ? osc_index : osc_index - NUMBER_OF_CENTRALS));
		return;
	}

	AVS_TRACE(("count=%04u threshold=%04u/%04u ", count,
		avs->thresholds[osc_index].low,
		avs->thresholds[osc_index].high));

	if (count < avs->thresholds[osc_index].low) {
		*increase = TRUE;
		AVS_TRACE(("LOW\n"));
	} else if (count < avs->thresholds[osc_index].high) {
		*decrease = FALSE;
		AVS_TRACE(("OK\n"));
	} else {
		AVS_TRACE(("HIGH\n"));
	}
}

/**
 * Track the performance of all oscillators and adjust the voltage if needed.
 *
 * This function is to be called periodically.
 *
 * @param avs		Pointer to AVS context.
 * @return		BCME_OK on success.
 */

static int
track(avs_context_t *avs)
{
	int i, osc_index = 0;
	uint voltage, duration;
	bool increase = FALSE, decrease = TRUE;
	osc_count_t count[2];
	uint64 tracking_start = OSL_SYSUPTIME_US();

	ASSERT(avs->initialized == TRUE);

	voltage = (uint)read_pvt_value(avs, PVT_1V_0, PVT_AVERAGING_ACCURATE);

	avs->last_voltage  = voltage;
	avs->last_avs_temp = (int)read_pvt_value(avs, PVT_TEMPERATURE, PVT_AVERAGING_FAST);
	avs->last_arm_temp = (int)read_tmon_value(avs);

#if defined(BCMDBG) || defined(AVS_ENABLE_STATUS)
	avs->voltage_range[0] = avs->voltage_range[0] ? MIN(avs->voltage_range[0], voltage) : voltage;
	avs->voltage_range[1] = MAX(avs->voltage_range[1], voltage);

	avs->temp_range[0] = avs->temp_range[0] ? MIN(avs->temp_range[0], avs->last_avs_temp) : avs->last_avs_temp;
	avs->temp_range[1] = avs->temp_range[1] ? MAX(avs->temp_range[1], avs->last_avs_temp) : avs->last_avs_temp;
#endif // endif

	AVS_INFORM(("%s: v=%umV dac_code=%u AVStemp=%dmdC ARMtemp=%dmdC LVM=%u\n",
		__FUNCTION__, TO_MV(voltage), avs->dac_code,
		TO_MDC(avs->last_avs_temp), TO_MDC(avs->last_arm_temp), avs->lvm_state));

	/*
	 * Check voltage limits to determine if voltage should be adjusted. These limits take
	 * precedence over oscillator thresholds.
	 */

	if (voltage < avs->vlimit_low) {
		AVS_ERROR(("%s: lower voltage limit exceeded, current=%umV min=%umV\n", __FUNCTION__, TO_MV(voltage), TO_MV(avs->vlimit_low)));
		increase = TRUE;
	} else if (voltage > avs->vlimit_high) {
		AVS_ERROR(("%s: upper voltage limit exceeded, current=%umV max=%umV\n", __FUNCTION__, TO_MV(voltage), TO_MV(avs->vlimit_high)));
	} else {
		/*
		 * Check all oscillator thresholds to determine if voltage should be adjusted
		 *
		 * Note that this can cause oscillations if oscillator performance requires the voltage
		 * to be increased above the upper threshold, or to be decreased below the lower
		 * threshold.
		 *
		 * Increase the voltage if ANY oscillator is below its low threshold, decrease if ALL
		 * oscillators are above their high threshold. If this adjustment causes the voltage to
		 * exceed the allowed range, it will be adjusted the next time.
		 */

		// XXX Look into returning the required voltage increase to allow faster tracking

#ifdef BCMDBG
		for(i=0; i<NUMBER_OF_CENTRALS; i++)
#else
		for(i=0; i<NUMBER_OF_CENTRALS && increase == FALSE; i++)
#endif /* BCMDBG */
		{
			check_thresholds(avs, osc_index++, read_central_rosc_count(avs, i), &increase, &decrease);
		}

#ifdef BCMDBG
		for(i=0; i<NUMBER_OF_REMOTES(avs->sih); i++)
#else
		for(i=0; i<NUMBER_OF_REMOTES(avs->sih) && increase == FALSE; i++)
#endif /* BCMDBG */
		{
			read_remote_rosc_count(avs, i, &count[0], &count[1]);
			check_thresholds(avs, osc_index++, count[0], &increase, &decrease);
			check_thresholds(avs, osc_index++, count[1], &increase, &decrease);
		}

		if (increase == TRUE) {
			AVS_INFORM(("%s: voltage %umV LOW\n", __FUNCTION__, TO_MV(voltage)));
		} else if (decrease == TRUE) {
			AVS_INFORM(("%s: voltage %umV HIGH\n", __FUNCTION__, TO_MV(voltage)));
		} else {
			AVS_TRACE(("%s: voltage %umV OK\n", __FUNCTION__, TO_MV(voltage)));
		}
	}

	if (increase == TRUE) {
		/* Current voltage is too low, adjust */
		set_dac_voltage(avs, avs->dac_code + DAC_ADJUST_SIZE);
		avs->dac_adjustments[1]++;
	} else if (decrease == TRUE) {
		/* Current voltage is too high, adjust */
		set_dac_voltage(avs, avs->dac_code - MIN(avs->dac_code, DAC_ADJUST_SIZE));
		avs->dac_adjustments[0]++;
	}

	duration = (uint)(OSL_SYSUPTIME_US() - tracking_start);
	AVS_INFORM(("%s: tracking took %uus\n", __FUNCTION__, duration));

#if defined(BCMDBG) || defined(AVS_ENABLE_STATUS)
	avs->dac_range[0] = avs->dac_range[0] ? MIN(avs->dac_range[0], avs->dac_code) : avs->dac_code;
	avs->dac_range[1] = MAX(avs->dac_range[1], avs->dac_code);

	avs->last_track_time_us = duration;
#endif // endif

	return BCME_OK;
}

#ifdef AVS_FIND_NOMINAL_VOLTAGE

/**
 * Find the DAC code corresponding to the defined nominal voltage.
 *
 * This function is called during AVS initialization. At that time, the DAC_code-voltage relation
 * is not yet known, so we need to iteratively adjust the DAC code until we arrive close to the
 * nominal voltage.
 *
 * @param avs			Pointer to AVS context.
 * @param initial_dac_code	Initial DAC code.
 * @return			Determined DAC code, or initial_dac_code on error.
 */

static dac_code_t
find_nominal_voltage(avs_context_t *avs, dac_code_t initial_dac_code)
{
	int iterations = NOMINAL_VOLTAGE_MAX_ITERATIONS;
	int voltage_delta = 0, dac_adjust;
	uint voltage;
	dac_code_t dac_code = initial_dac_code;

	AVS_INFORM(("%s: v=%umV initial_dac=%u\n", __FUNCTION__, TO_MV(FLOAT_TO_INT(NOMINAL_VOLTAGE)), dac_code));

	while(iterations--) {
		/* Measure voltage for the current DAC code */
		set_dac_voltage(avs, dac_code);
		voltage = (uint)read_pvt_value(avs, PVT_1V_0, PVT_AVERAGING_ACCURATE);

		/* Default to the initial DAC code if voltage range is exceeded */
		if (voltage > avs->vlimit_high ||
		    voltage < avs->vlimit_low) {
			AVS_ERROR(("voltage range exceeded, dac_code=%u v=%dmV\n", dac_code, TO_MV(voltage)));
			return initial_dac_code;
		}

		voltage_delta = (int)(voltage - FLOAT_TO_INT(NOMINAL_VOLTAGE));

		AVS_INFORM(("dac_code=%u v=%umV delta=%dmV\n", dac_code, TO_MV(voltage), TO_MV(voltage_delta)));

		/* Exit if the measured voltage is within the required range */
		if (ABS(voltage_delta) <= FLOAT_TO_INT(NOMINAL_VOLTAGE_ERR)) {
			return dac_code;
		}

		/* Adjust the current DAC code based on how far away we are from the target voltage */
		ASSERT(avs->dac_min_vstep != 0);
		dac_adjust = -((voltage_delta * 1000 /* mV */) / (int)avs->dac_min_vstep);
		if (dac_adjust == 0) {
			dac_adjust = (voltage_delta > 0 ? -1 : 1);
		}

		dac_code += dac_adjust;

		/* Default to the initial DAC code if the calculated code exceeds the
		   minimum or maximum limit */
		if ((uint)dac_code > MAX_DAC_CODE) {
			AVS_ERROR(("%s: DAC range exceeded (%u/%u/%d/%d/%u)\n", __FUNCTION__,
				voltage, dac_code, dac_adjust, voltage_delta, avs->dac_min_vstep));
			return initial_dac_code;
		}
	}

	/* Default to the initial DAC code if we can't reach the nominal voltage */
	AVS_ERROR(("%s: could not reach the nominal voltage, delta=%dmV\n", __FUNCTION__, TO_MV(voltage_delta)));
	return initial_dac_code;
}

#endif /* AVS_FIND_NOMINAL_VOLTAGE */

/**
 * Initialize the lower and upper performance thresholds (target frequencies) of all central
 * and remote oscillators.
 *
 * @param avs		Pointer to AVS context.
 * @param context	Pointer to the prediction context data structure containing the results
 *			from the prediction algorithm.
 */

static void
initialize_thresholds(avs_context_t *avs, predict_context_t *context)
{
	uint osc_index;
	uint vmargin_low, vmargin_high;
	osc_count_t osc_threshold;
	thresholds_t *thresholds;

	ASSERT(context != NULL);
	ASSERT(!(avs->voltage_margin.slope == 0 && avs->voltage_margin.intercept == 0));

	/* The prediction algorithm calculated the chip convergence voltage (the convergence voltage corresponding to
	 * the most critical oscillator). This value is used together with the voltage margin required for the process
	 * corner this part is in to calculate the lower and upper voltage margins.
	 */

	vmargin_low  = voltage_to_margin(avs->voltage_margin, context->vconv);
	vmargin_high = vmargin_low + avs->threshold_margin;

	AVS_INFORM(("%s: vconv=%umV margin=%umV/%umV\n", __FUNCTION__,
		TO_MV(context->vconv), TO_MV(vmargin_low), TO_MV(vmargin_high)));

	for(osc_index=0; osc_index<NUMBER_OF_OSCILLATORS(avs->sih); osc_index++) {

		AVS_TRACE(("%c OSC%02d ",
			osc_index < NUMBER_OF_CENTRALS ? 'C' : 'R',
			osc_index < NUMBER_OF_CENTRALS ? osc_index : osc_index - NUMBER_OF_CENTRALS));

		thresholds = &avs->thresholds[osc_index];
		thresholds->low = thresholds->high = IGNORE_OSCILLATOR;

		osc_threshold = get_rosc_threshold(avs, osc_index);
		if (osc_threshold == IGNORE_OSCILLATOR) {
			/* Oscillator is configured to ignore */
			AVS_TRACE(("IGNORE\n"));
			continue;
		}

		if (context->threshold_voltage[osc_index] == 0) {
			/* Oscillator is returning invalid counts, ignore */
			AVS_TRACE(("INVALID\n"));
			continue;
		}

		/* Calculate the lower and upper performance thresholds for this oscillator. */
		thresholds->low  = osc_threshold + ((context->slope[osc_index] * vmargin_low ) / SCALING_FACTOR);
		thresholds->high = osc_threshold + ((context->slope[osc_index] * vmargin_high) / SCALING_FACTOR);

		AVS_TRACE(("threshold_voltage=%04umV thresholds=%04u(%05ukHz)/%04u(%05ukHz)\n",
			TO_MV(context->threshold_voltage[osc_index]),
			thresholds->low, TO_KHZ(rosc_count_to_freq(avs, thresholds->low)),
			thresholds->high, TO_KHZ(rosc_count_to_freq(avs, thresholds->high))));

		ASSERT(thresholds->high >= thresholds->low);
	}
}

/**
 * Store the count of the specified oscillator.
 *
 * The oscillator count is stored in the prediction context.
 *
 * This function uses linear oscillator indices, where central and remote oscillators are mapped
 * to a linear range from 0 to (NUMBER_OF_CENTRALS + 2 * NUMBER_OF_REMOTES) to make iteration easier.
 *
 * @param avs		Pointer to AVS context.
 * @param osc_index	Linear oscillator index.
 * @param context	Pointer to prediction context.
 * @param count		Oscillator count.
 */

static void
predict_store_initial_count(avs_context_t *avs, uint osc_index, predict_context_t *context, osc_count_t count)
{
	/* Just store the current count for the next phase */
	context->initial_count[osc_index] = count;

	AVS_TRACE(("%c OSC%02u count=%05u(%05ukHz)%s\n",
		osc_index < NUMBER_OF_CENTRALS ? 'C' : 'R',
		osc_index < NUMBER_OF_CENTRALS ? osc_index : osc_index - NUMBER_OF_CENTRALS,
		count, TO_KHZ(rosc_count_to_freq(avs, count)),
		count == INVALID_OSCILLATOR_COUNT ? " INVALID" : ""));
}

/**
 * Calculate the threshold voltage of the specified oscillator, and find the chip convergence voltage
 * (the threshold voltage of the most critical oscillator).
 *
 * The threshold and convergence voltages are stored in the prediction context.
 *
 * This function uses linear oscillator indices, where central and remote oscillators are mapped
 * to a linear range from 0 to (NUMBER_OF_CENTRALS + 2 * NUMBER_OF_REMOTES) to make iteration easier.
 *
 * @param avs		Pointer to AVS context.
 * @param osc_index	Linear oscillator index.
 * @param context	Pointer to a prediction context containing valid initial counts for all oscillators.
 * @param delta_voltage The voltage difference between initial and current measurements.
 * @param count		Oscillator count.
 * @param threshold	Oscillator threshold.
 */

static void
predict_find_vconv(avs_context_t *avs, uint osc_index, predict_context_t *context, int delta_voltage, osc_count_t count, osc_count_t threshold)
{
	ASSERT(delta_voltage != 0);

	AVS_TRACE(("%c OSC%02d ",
		osc_index < NUMBER_OF_CENTRALS ? 'C' : 'R',
		osc_index < NUMBER_OF_CENTRALS ? osc_index : osc_index - NUMBER_OF_CENTRALS));

	if (threshold == IGNORE_OSCILLATOR) {
		/* Oscillator is configured to ignore */
		AVS_TRACE(("IGNORE\n"));
		return;
	}

	if (count == INVALID_OSCILLATOR_COUNT || context->initial_count[osc_index] == INVALID_OSCILLATOR_COUNT) {
		/* Oscillator is returning invalid counts, ignore */
		context->threshold_voltage[osc_index] = 0;
		AVS_TRACE(("INVALID\n"));
		return;
	}

	/* Calculate and store the oscillator slope */
	context->slope[osc_index] = ((context->initial_count[osc_index] - count) * SCALING_FACTOR) / delta_voltage;

	/* Calculate and store the threshold voltage for this oscillator */
	context->threshold_voltage[osc_index] = context->vlow;
	if (context->slope[osc_index] != 0) {
		context->threshold_voltage[osc_index] += ((int32)(threshold - count) * SCALING_FACTOR) / context->slope[osc_index];
	}

	/* Find the threshold voltage of the most critical oscillator */
	context->vconv = MAX(context->vconv, context->threshold_voltage[osc_index]);

	AVS_TRACE(("count=%05u(%05ukHz) target=%05u(%05ukHz) slope=%05dc/V(%05dkHz/V) vtarget=%04umV%s\n",
		count, TO_KHZ(rosc_count_to_freq(avs, count)),
		threshold, TO_KHZ(rosc_count_to_freq(avs, threshold)),
		context->slope[osc_index],
		TO_KHZ(rosc_count_to_freq(avs, context->slope[osc_index])),
		TO_MV(context->threshold_voltage[osc_index]),
		count < threshold ? " LOW" : ""));
}

/**
 * Run the prediction algorithm.
 *
 * The prediction algorithm is based on taking two voltage and frequency measurement pairs
 * for each oscillator, and deriving the linear frequency-voltage relationship on a per-die
 * basis. Since the target frequency (the lowest valid frequency) of each oscillator is
 * known from simulations, we can predict the convergence voltage (the voltage corresponding
 * to the target frequency) of each oscillator.
 *
 * Algorithm state, including the calculated convergence voltage, is stored in a context data
 * structure, allowing this function to be called multiple times to improve the accuracy of
 * the predicted convergence voltage.
 *
 * @param avs		Pointer to AVS context.
 * @param context	Pointer to predict context.
 * @return		BCME_OK on success, BCME_UNSUPPORTED if AVS is not detected.
 */

static uint32
predict(avs_context_t *avs, predict_context_t* context)
{
	int i, osc_index = 0, delta_voltage;
	osc_count_t threshold, count[2];

	ASSERT(context != NULL);

	/* Caller needs to provide a valid prediction context */
	ASSERT(context->dac_low != 0);
	ASSERT(context->dac_high > context->dac_low);

	context->vconv = 0;

	/* On the first pass, measure oscillator frequencies at the upper voltage */
	if (context->pass == 0) {

		/* Switch to the upper voltage */
		set_dac_voltage(avs, context->dac_high);
		context->vhigh = (uint)read_pvt_value(avs, PVT_1V_0, PVT_AVERAGING_ACCURATE);

		AVS_INFORM(("upper measurement: dac=%u v=%umV\n", context->dac_high, TO_MV(context->vhigh)));

		for(i=0; i<NUMBER_OF_CENTRALS; i++) {
			predict_store_initial_count(avs, osc_index++, context, read_central_rosc_count(avs, i));
		}

		for(i=0; i<NUMBER_OF_REMOTES(avs->sih); i++) {
			read_remote_rosc_count(avs, i, &count[0], &count[1]);
			predict_store_initial_count(avs, osc_index++, context, count[0]);
			predict_store_initial_count(avs, osc_index++, context, count[1]);
		}
	}

	/* Switch to the lower voltage */
	set_dac_voltage(avs, context->dac_low);
	context->vlow = (uint)read_pvt_value(avs, PVT_1V_0, PVT_AVERAGING_ACCURATE);

	AVS_INFORM(("lower measurement: dac=%u v=%umV dv=%dmV\n", context->dac_low, TO_MV(context->vlow), TO_MV(context->vlow) - TO_MV(context->vhigh)));

	/* Abort if the voltage is not changing */
	delta_voltage = context->vhigh - context->vlow;
	if (ABS(delta_voltage) < FLOAT_TO_INT(AVS_DETECT_THRESHOLD)) {
		AVS_ERROR(("%s: unable to control voltage, delta_dac=%d dv=%dmV\n", __FUNCTION__, context->dac_high - context->dac_low, TO_MV(delta_voltage)));
		return BCME_UNSUPPORTED;
	}

	/* Measure the oscillator frequencies values at the lower voltage, and find the convergence voltage */
	osc_index = 0;
	for(i=0; i<NUMBER_OF_CENTRALS; i++) {
		count[0]  = read_central_rosc_count(avs, i);
		threshold = get_rosc_threshold(avs, osc_index);
		predict_find_vconv(avs, osc_index++, context, delta_voltage, count[0], threshold);
	}

	for(i=0; i<NUMBER_OF_REMOTES(avs->sih); i++) {
		read_remote_rosc_count(avs, i, &count[0], &count[1]);
		threshold = get_rosc_threshold(avs, osc_index);
		predict_find_vconv(avs, osc_index++, context, delta_voltage, count[0], threshold);
		threshold = get_rosc_threshold(avs, osc_index);
		predict_find_vconv(avs, osc_index++, context, delta_voltage, count[1], threshold);
	}

	context->pass++;

	return BCME_OK;
}

/**
 * Get the initial DAC code.
 *
 * Before changing the chip voltage using the DAC, we need to determine the initial
 * DAC code that allows the chip to start reliably.
 *
 * Options are:
 *	- Hardcoded initial DAC code.
 *	- DAC code calculated from the measured open loop regulator feedback voltage.
 *	- DAC code determined from iterating towards the nominal voltage.
 *
 * @param avs		Pointer to AVS context.
 * @return		Initial DAC code.
 */

static dac_code_t
get_initial_dac_code(avs_context_t *avs)
{
	dac_code_t board_dac_code, initial_dac_code = avs->initial_dac_code;

	BCM_REFERENCE(board_dac_code);

#ifdef AVS_GET_BOARD_DAC_CODE
	board_dac_code = get_board_dac_code(avs);
	if (board_dac_code != INVALID_DAC_CODE) {
		initial_dac_code = board_dac_code;
	}
#endif /* AVS_GET_BOARD_DAC_CODE */

#ifdef AVS_FIND_NOMINAL_VOLTAGE
	initial_dac_code = find_nominal_voltage(avs, initial_dac_code);
#endif /* AVS_FIND_NOMINAL_VOLTAGE */

	return initial_dac_code;
}

/**
 * Helper function for getting configuration.
 *
 * @param vars		Pointer to environment.
 * @param name		Item name.
 * @param index		Value index (for arrays)
 * @param default_value Value to return if none could be retrieved
 * @return		Item value.
 *
 * @note Does assume non-zero item values only.
 */

static int
BCMATTACHFN(get_config)(char *vars, const char *name, uint index, int default_value)
{
	int value = 0;

	if (getvar(vars, name) != NULL) {
		value = getintvararray(vars, name, index);
	}
	return (value ? value : default_value);
}

/**
 * Helper function for validating configuration.
 *
 * @param avs		Pointer to AVS context.
 * @return		BCME_OK on success.
 */

static int
validate_config(avs_context_t *avs)
{
	/* Voltage limit checks */
	if (avs->vlimit_low < FLOAT_TO_INT(MIN_VLIMIT))			return BCME_ERROR;
	if (avs->vlimit_high > FLOAT_TO_INT(MAX_VLIMIT))		return BCME_ERROR;
	if (avs->vlimit_low > avs->vlimit_high)				return BCME_ERROR;

	/* Margin checks */
	if (avs->vmargin_low > FLOAT_TO_INT(MAX_VMARGIN))		return BCME_ERROR;
	if (avs->vmargin_high > FLOAT_TO_INT(MAX_VMARGIN))		return BCME_ERROR;

	/* Voltage step checks */
	if (avs->dac_min_vstep < FLOAT_TO_INT(MIN_DAC_VOLTAGE_STEP_MV))	return BCME_ERROR;
	if (avs->dac_min_vstep > FLOAT_TO_INT(MAX_DAC_VOLTAGE_STEP_MV))	return BCME_ERROR;

	/* Regulator settle time */
	if (avs->reg_settle_time < MIN_REGULATOR_SETTLE_TIME_MS * 1000)	return BCME_ERROR;
	if (avs->reg_settle_time > MAX_REGULATOR_SETTLE_TIME_MS * 1000) return BCME_ERROR;

	/* Initial DAC code */
	if (avs->initial_dac_code < MIN_INITIAL_DAC_CODE)		return BCME_ERROR;
	if (avs->initial_dac_code > MAX_INITIAL_DAC_CODE)		return BCME_ERROR;

	/* Threshold margin */
	if (avs->threshold_margin < FLOAT_TO_INT(MIN_THRESHOLD_MARGIN)) return BCME_ERROR;
	if (avs->threshold_margin > FLOAT_TO_INT(MAX_THRESHOLD_MARGIN)) return BCME_ERROR;

	return BCME_OK;
}

/**
 * Initialize AVS.
 *
 * This function initializes the AVS hardware and runs the convergence process.
 *
 * @param avs		Pointer to AVS context.
 * @return		BCME_OK on success.
 */

static int
initialize_avs(avs_context_t *avs)
{
	int result = BCME_ERROR, pass, delta;
	uint vmargin = 0, vsum, vsum_safe, vdelta, duration;
	uint64 init_start = OSL_SYSUPTIME_US();
	dac_code_t dac_code;
	predict_context_t *predict_context = NULL;

	STATIC_ASSERT(ROSC_MEASUREMENT_TIME_CONTROL <= 0x7f);
	STATIC_ASSERT(NUMBER_OF_REMOTES(avs->sih) <= NUMBER_OF_REMOTES_MAX);

	AVS_INFORM(("initializing AVS\n"));
	avs->initialized = FALSE;

	/* Reset AVS hardware */
	set_function_mode(avs, BCHP_AVS_PVT_MNTR_CONFIG_PVT_MNTR_CTRL_MODE_VTMON, TRUE);
	set_dac_enable(avs, FALSE, TRUE);

	result = validate_config(avs);
	if (result != BCME_OK) {
		AVS_ERROR(("%s: invalid configuration\n", __FUNCTION__));
		goto exit;
	}

	predict_context = (predict_context_t*)MALLOCZ(avs->osh, sizeof(predict_context_t));
	if (predict_context == NULL) {
		AVS_ERROR(("%s: out of mem, malloced %d bytes\n", __FUNCTION__, MALLOCED(avs->osh)));
		result = BCME_ERROR;
		goto exit;
	}

	/*
	 * Hardware initialization
	 */

	if (LVM_SUPPORT(avs->sih)) {
		/* Bring memory LVM mode under software control */
		sleep(avs->reg_settle_time);
		set_LVM_enable(avs, FALSE);

		ASSERT(BCM43684_CHIP(avs->sih->chip));
		write_pmu_register(avs, PMU_CHIPCTL0, BCM43684_PMU_RSRC_CNTRL_LVM_EN, 0);
	}

	initialize_oscillators(avs);

	/*
	 * Determine the relation between convergence voltage (process corner) and margin.
	 *
	 * The AVS algorithm allows different margins for slow silicon versus fast silicon, and therefore
	 * the margin is a function of process corner. Since the convergence voltage is a linear function
	 * of process corner, we can use a linear equation to describe the margin as a function of the
	 * convergence voltage.
	 *
	 * We use the configured margins for SS/FF corners and the convergence voltage limits to determine
	 * a linear equation for the voltage-margin relation.
	 */

	AVS_INFORM(("margins=%umV/%umV voltage=%umV/%umV\n",
		TO_MV(avs->vmargin_low), TO_MV(avs->vmargin_high),
		TO_MV(avs->vlimit_low), TO_MV(avs->vlimit_high)));

	vdelta = (avs->vlimit_high - avs->vlimit_low) + (avs->vmargin_low - avs->vmargin_high);
	if (vdelta == 0) {
		AVS_ERROR(("%s: invalid configuration (vdelta)\n", __FUNCTION__));
		result = BCME_ERROR;
		goto exit;
	}

	avs->voltage_margin.slope     = ((avs->vmargin_high - avs->vmargin_low) * SCALING_FACTOR) / vdelta;
	avs->voltage_margin.intercept = avs->vmargin_low - ((avs->voltage_margin.slope * (avs->vlimit_low - avs->vmargin_low)) / SCALING_FACTOR);

	AVS_INFORM(("margins: slope=%dmV/V intercept=%dmV\n", TO_MV(avs->voltage_margin.slope), TO_MV(avs->voltage_margin.intercept)));

#if defined(BCMDBG) || defined(AVS_ENABLE_STATUS)
	/* Measure open loop voltage */
	sleep(avs->reg_settle_time);	/* Allow for settling */
	reset_measurements(avs, RESET_PVT);

	avs->voltage_open = (uint)read_pvt_value(avs, PVT_1V_0, PVT_AVERAGING_ACCURATE);
	AVS_INFORM(("v_open_loop=%umV\n", TO_MV(avs->voltage_open)));
#endif // endif

	/*
	 * Determine initial DAC high and low codes for the first prediction pass (local fit).
	 */

	avs->local_fit_dac_offset = (dac_code_t)((FLOAT_TO_INT(LOCAL_FIT_DAC_OFFSET) * 1000 /* mV */) / avs->dac_min_vstep);
	ASSERT(avs->local_fit_dac_offset > 0);

	AVS_INFORM(("local fit: dac_offset=%u (%umV)\n", avs->local_fit_dac_offset, TO_MV(FLOAT_TO_INT(LOCAL_FIT_DAC_OFFSET))));

	predict_context->dac_high = get_initial_dac_code(avs);
	predict_context->dac_low  = predict_context->dac_high - avs->local_fit_dac_offset;

	/*
	 * Calculate the final voltage in two passes (local/global fit).
	 */

	for(pass=0; pass<PREDICT_PASSES; pass++) {

		AVS_INFORM(("predict pass %u of %u\n", pass+1, PREDICT_PASSES));

		/* Predict the convergence voltage, store state in the prediction context. */
		result = predict(avs, predict_context);
		if (result != BCME_OK) {
			goto exit;
		}

		/* Apply a margin associated with the calculated convergence voltage (process corner) */
		vmargin = voltage_to_margin(avs->voltage_margin, predict_context->vconv);
		vsum    = predict_context->vconv + vmargin;

		/* Make sure target voltage is NEVER outside the allowable voltage min/max values */
		vsum = MAX(vsum, avs->vlimit_low);
		vsum = MIN(vsum, avs->vlimit_high);

		AVS_INFORM(("vconv=%umV margin=%umV vsum=%umV\n",
			TO_MV(predict_context->vconv), TO_MV(vmargin), TO_MV(vsum)));

		/* Calculate linear equation for DAC_code-voltage relation */
		ASSERT(predict_context->vhigh - predict_context->vlow != 0);
		avs->voltage_daccode.slope     = ((predict_context->dac_high - predict_context->dac_low) * SCALING_FACTOR) / (predict_context->vhigh - predict_context->vlow);
		avs->voltage_daccode.intercept = predict_context->dac_low - ((avs->voltage_daccode.slope * predict_context->vlow) / SCALING_FACTOR);

		AVS_INFORM(("dac: slope=%dsteps/V intercept=%dsteps exp=%uuV/step real=%uuV/step\n",
			avs->voltage_daccode.slope, avs->voltage_daccode.intercept,
			TO_UV(avs->dac_min_vstep),
			1000000 /* uV */ / avs->voltage_daccode.slope));

		if (pass < PREDICT_PASSES-1) {
			/* Apply an extra safety margin to avoid dropping too low on subsequent passes */
			vsum_safe = vsum + FLOAT_TO_INT(GLOBAL_FIT_SAFETY_MARGIN);
			vsum_safe = MIN(vsum_safe, avs->vlimit_high);
			AVS_INFORM(("safety_margin=%umV vsum_safe=%umV\n",
				TO_MV(FLOAT_TO_INT(GLOBAL_FIT_SAFETY_MARGIN)), TO_MV(vsum_safe)));

			/* Adjust the low DAC code to increase the range (and precision) of subsequent passes */
			dac_code = voltage_to_dac_code(avs->voltage_daccode, vsum_safe);
			if (predict_context->dac_low < dac_code) {
				/* Dropped too low, abort prediction */
				AVS_ERROR(("dropped too low, v=%umV but need %umV\n", TO_MV(predict_context->vlow), TO_MV(vsum_safe)));
				break;
			}

			predict_context->dac_low = dac_code;
		}
	}

	/* Show a warning if the configured relation between voltage and DAC code deviates more than 10% from
	 * the measured value. In this case dac_min_vstep needs to be adjusted for the current board.
	 */
	ASSERT(avs->voltage_daccode.slope != 0);

	delta = (1000000 / avs->voltage_daccode.slope) - TO_UV(avs->dac_min_vstep);
	if (ABS(delta) > TO_UV(avs->dac_min_vstep) / 10) {
		AVS_ERROR(("%s: incorrect DAC voltage step, exp=%uuV/step real=%uuV/step\n", __FUNCTION__,
			TO_UV(avs->dac_min_vstep),
			1000000 /* uV */ / avs->voltage_daccode.slope));
	}

	/* Now that the DAC_code-voltage relation is known, calculate the DAC code limits from the voltage limits */
	avs->dac_code_min = voltage_to_dac_code(avs->voltage_daccode, avs->vlimit_low);
	avs->dac_code_max = voltage_to_dac_code(avs->voltage_daccode, avs->vlimit_high);

	AVS_INFORM(("dac: limits=%u/%u\n", avs->dac_code_min, avs->dac_code_max));

	/* Calculate and switch to the final voltage */
	dac_code = voltage_to_dac_code(avs->voltage_daccode, vsum);
	set_dac_voltage(avs, dac_code);

	AVS_INFORM(("final: vconv=%umV vmargin=%umV vsum=%umV dac=%u v=%umV\n",
		TO_MV(predict_context->vconv), TO_MV(vmargin), TO_MV(vsum),
		dac_code, TO_MV(read_pvt_value(avs, PVT_1V_0, PVT_AVERAGING_ACCURATE))));

	/*
	 * Calculate oscillator thresholds
	 */

	initialize_thresholds(avs, predict_context);

	duration = (uint)(OSL_SYSUPTIME_US() - init_start);
	AVS_TRACE(("%s: convergence took %uus\n", __FUNCTION__, duration));

#if defined(BCMDBG) || defined(AVS_ENABLE_STATUS)
	/* Save some stats from the predict context */
	avs->dac_high = predict_context->dac_high;
	avs->dac_low  = predict_context->dac_low;
	avs->vhigh    = predict_context->vhigh;
	avs->vlow     = predict_context->vlow;
	avs->vmargin  = vmargin;
	avs->vconv    = predict_context->vconv;
	avs->vsum     = vsum;

	avs->last_converge_time_us = duration;
	avs->last_track_time_us    = 0;
#endif // endif

	avs->iterations = 0;
	avs->initialized = TRUE;

exit:
	if (predict_context != NULL) {
		MFREE(avs->osh, predict_context, sizeof(predict_context_t));
	}

	return result;
}

/*
 *
 * Export functions
 *
 */

/**
 * Determine if AVS can be enabled.
 *
 * @param sih		SI handle.
 * @return		TRUE if AVS can be enabled for this chip.
 */

bool
BCMATTACHFN(avs_supported)(osl_t *osh, si_t *sih)
{
	BCM_REFERENCE(osh);

	/* Check for AVS hardware */
	if (si_findcoreidx(sih, AVS_CORE_ID, 0) != BADIDX) {
		/* AVS whitelisted chips */
		if (BCM43684_CHIP(sih->chip) || BCM6710_CHIP(sih->chip))
			return TRUE;
	}

	return FALSE;
}

/**
 * Attach to AVS.
 *
 * @param osh		OSL handle.
 * @param sih		SI handle.
 * @param context	Context for calling @see wl_backplane_read and @see wl_backplane_write (NIC only)
 * @param vars		Environment vars array.
 * @return		AVS context, or NULL on error.
 */

avs_context_t*
BCMATTACHFN(avs_attach)(osl_t *osh, si_t *sih, void *context, char *vars)
{
	int result;
	avs_context_t *avs;

	BCM_REFERENCE(result);
	BCM_REFERENCE(context);

	/* Caller should do this check first */
	ASSERT(avs_supported(osh, sih));

	avs = MALLOCZ(osh, sizeof(avs_context_t));
	if (avs == NULL) {
		AVS_ERROR(("%s: out of mem, malloced %d bytes\n", __FUNCTION__, MALLOCED(osh)));
		return NULL;
	}

	init_lock(avs);

	avs->osh	  = osh;
	avs->sih	  = sih;
	avs->dac_code	  = INVALID_DAC_CODE;
	avs->dac_code_min = avs->dac_code_max = INVALID_DAC_CODE;
#ifndef DONGLEBUILD
	avs->reg_ops_context = context;
#endif /* DONGLEBUILD */

	/* Default chip-specific algorithm configuration */
	switch(CHIPID(avs->sih->chip)) {
		case BCM6710_CHIP_ID:
			avs->vmargin_low	= FLOAT_TO_INT(DEFAULT_VMARGIN_L_6710);
			avs->vmargin_high	= FLOAT_TO_INT(DEFAULT_VMARGIN_H_6710);
			avs->vlimit_low		= FLOAT_TO_INT(DEFAULT_VLIMIT_L_6710);
			avs->vlimit_high	= FLOAT_TO_INT(DEFAULT_VLIMIT_H_6710);
			break;
		case BCM43684_CHIP_ID:
		default:
			avs->vmargin_low	= FLOAT_TO_INT(DEFAULT_VMARGIN_L_43684);
			avs->vmargin_high	= FLOAT_TO_INT(DEFAULT_VMARGIN_H_43684);
			avs->vlimit_low		= FLOAT_TO_INT(DEFAULT_VLIMIT_L_43684);
			avs->vlimit_high	= FLOAT_TO_INT(DEFAULT_VLIMIT_H_43684);
			break;
	}

	/* Override default configuration if needed. Configuration will be checked in @see initialize_avs() */
	avs->vmargin_low	= (uint)get_config(vars, rstr_avsmargins, 0, avs->vmargin_low);
	avs->vmargin_high	= (uint)get_config(vars, rstr_avsmargins, 1, avs->vmargin_high);
	avs->vlimit_low		= (uint)get_config(vars, rstr_avslimits,  0, avs->vlimit_low);
	avs->vlimit_high	= (uint)get_config(vars, rstr_avslimits,  1, avs->vlimit_high);
	avs->dac_min_vstep	= (uint)get_config(vars, rstr_avsvstep,   0, FLOAT_TO_INT(DEFAULT_DAC_VOLTAGE_STEP_MV));
	avs->use_dac_stepping	= (bool)get_config(vars, rstr_avsdacstep, 0, FALSE);
	avs->reg_settle_time	= (uint)get_config(vars, rstr_avssettle,  0, DEFAULT_REGULATOR_SETTLE_TIME_MS) * 1000;
	avs->initial_dac_code	= (dac_code_t)get_config(vars, rstr_avsinitial, 0, DEFAULT_INITIAL_DAC_CODE);
	avs->threshold_margin	= (uint)get_config(vars, rstr_avsthrm,    0, FLOAT_TO_INT(DEFAULT_THRESHOLD_MARGIN));

	return avs;
}

/**
 * Detach from AVS.
 *
 * Caller should call @see avs_reset to reset hardware to default.
 *
 * @param avs		AVS context.
 */

void
BCMATTACHFN(avs_detach)(avs_context_t *avs)
{
	if (avs != NULL) {
		MFREE(avs->osh, avs, sizeof(avs_context_t));
	}
}

/**
 * Run the AVS tracking process.
 *
 * This function should be called periodically (e.g. every 100ms - 1000ms) until an error is returned.
 *
 * @param avs		AVS context.
 * @return		BCME_OK on success.
 */

int
avs_track(avs_context_t *avs)
{
	int result;

	ASSERT(avs != NULL);

	if (!lock(avs, OSL_EXT_TIME_FOREVER)) {
		return BCME_ERROR;
	}

	if (avs->initialized == FALSE) {
		/* Run AVS initialisation and convergence processes. */
		result = initialize_avs(avs);
		if (result != BCME_OK) {
			AVS_ERROR(("%s: unable to initialize algorithm (%d)\n", __FUNCTION__, result));
			avs_reset_internal(avs);
		}
	} else {
		/* Run AVS tracking process. */
		avs->iterations++;
		result = track(avs);
		if (result != BCME_OK) {
			AVS_ERROR(("%s: unable to run tracking process (%d)\n", __FUNCTION__, result));
			avs_reset_internal(avs);
		}
	}

	unlock(avs);

	return result;
}

/**
 * Reset AVS algorithm and restore hardware to defaults.
 *
 * @param avs		Pointer to AVS context.
 */

void
avs_reset(avs_context_t *avs)
{
	AVS_INFORM(("%s\n", __FUNCTION__));

	lock(avs, OSL_EXT_TIME_FOREVER);
	avs_reset_internal(avs);
	unlock(avs);
}

/**
 * Access internal AVS state.
 *
 * @param avs		Pointer to AVS context.
 * @param index		State variable index.
 * @param value		Pointer to target register.
 * @return		BCME_OK on success.
 *
 * @note No lock is taken.
 */

int
avs_get_value(avs_context_t *avs, avs_value_index_t index, uint *value)
{
	ASSERT(avs != NULL);
	ASSERT(value);

	*value = 0;

	if (avs->initialized) {
		switch(index) {
			case AVS_INDEX_VOLTAGE:	 *value = TO_MV(avs->last_voltage); break;
			case AVS_INDEX_TEMP:     *value = TO_MDC(avs->last_avs_temp); break;
			case AVS_INDEX_ARM_TEMP: *value = TO_MDC(avs->last_arm_temp); break;
			default:		 *value = 0; return BCME_UNSUPPORTED;
		}
		return BCME_OK;
	}
	return BCME_NOTREADY;
}

#ifdef BCMDBG

/* Below setter functions don't do any validation. If an invalid value is specified,
 * it will be caught during a subsequent AVS initialisation and an error will be returned.
 */

int
avs_set_vlimit_low(avs_context_t *avs, uint value)
{
	if (avs->initialized)
		return BCME_BUSY;

	avs->vlimit_low = FROM_MV(value);
	AVS_INFORM(("vlimit_low=%umV\n", TO_MV(avs->vlimit_low)));
	return BCME_OK;
}

int
avs_set_vlimit_high(avs_context_t *avs, uint value)
{
	if (avs->initialized)
		return BCME_BUSY;

	avs->vlimit_high = FROM_MV(value);
	AVS_INFORM(("vlimit_high=%umV\n", TO_MV(avs->vlimit_high)));
	return BCME_OK;
}

int
avs_set_vmargin_low(avs_context_t *avs, uint value)
{
	if (avs->initialized)
		return BCME_BUSY;

	avs->vmargin_low = FROM_MV(value);
	AVS_INFORM(("vmargin_low=%umV\n", TO_MV(avs->vmargin_low)));
	return BCME_OK;
}

int
avs_set_vmargin_high(avs_context_t *avs, uint value)
{
	if (avs->initialized)
		return BCME_BUSY;

	avs->vmargin_high = FROM_MV(value);
	AVS_INFORM(("vmargin_high=%umV\n", TO_MV(avs->vmargin_high)));
	return BCME_OK;
}

int
avs_set_threshold_margin(avs_context_t *avs, uint value)
{
	if (avs->initialized)
		return BCME_BUSY;

	avs->threshold_margin = FROM_MV(value);
	AVS_INFORM(("threshold_margin=%umV\n", TO_MV(avs->threshold_margin)));
	return BCME_OK;
}

#endif /* BCMDBG */

#if defined(BCMDBG) || defined(AVS_ENABLE_STATUS)

/**
 * Dump internal AVS state.
 *
 * @param avs		Pointer to AVS context.
 * @param buf		Target stringbuffer.
 * @return		BCME_OK on success.
 *
 * @note No lock is taken.
 */

int
avs_status(avs_context_t *avs, struct bcmstrbuf *buf)
{
	static const char *fmt = "%18.18s: ";

	ASSERT(avs != NULL);

	if (avs->initialized) {
		/* Current voltage and temperatures */
		bcm_bprintf(buf, fmt, "voltage");		bcm_bprintf(buf, "%umV\n", TO_MV(avs->last_voltage));
		bcm_bprintf(buf, fmt, "AVStemp");		bcm_bprintf(buf, "%dmdC\n", TO_MDC(avs->last_avs_temp));
		bcm_bprintf(buf, fmt, "ARMtemp");		bcm_bprintf(buf, "%dmdC\n", TO_MDC(avs->last_arm_temp));

		/* Current state */
		bcm_bprintf(buf, fmt, "iterations");		bcm_bprintf(buf, "%u\n", avs->iterations);
		bcm_bprintf(buf, fmt, "dac_code");		bcm_bprintf(buf, "%u\n", avs->dac_code);
		bcm_bprintf(buf, fmt, "LVM");			bcm_bprintf(buf, "%u\n", avs->lvm_state);

		/* Voltage regulator characteristics */
		bcm_bprintf(buf, fmt, "Vopen_loop");		bcm_bprintf(buf, "%umV\n", TO_MV(avs->voltage_open));
		bcm_bprintf(buf, fmt, "Vfeedback");		bcm_bprintf(buf, "%umV\n", TO_MV(avs->voltage_feedback));
		bcm_bprintf(buf, fmt, "board_dac_code");	bcm_bprintf(buf, "%u\n", avs->board_dac_code);
		bcm_bprintf(buf, fmt, "voltage_daccode_eq");	bcm_bprintf(buf, "slope=%dsteps/V int=%dsteps\n", avs->voltage_daccode.slope, avs->voltage_daccode.intercept);
		bcm_bprintf(buf, fmt, "dac_voltage_step");	bcm_bprintf(buf, "%uuV/step (exp %uuV/step)\n", 1000000 /* uV */ / avs->voltage_daccode.slope, TO_UV(avs->dac_min_vstep));
		bcm_bprintf(buf, fmt, "reg_settle_time");	bcm_bprintf(buf, "%ums\n", avs->reg_settle_time / 1000);

		/* Margins and limits */
		bcm_bprintf(buf, fmt, "Vmargins");		bcm_bprintf(buf, "%umV/%umV\n", TO_MV(avs->vmargin_low), TO_MV(avs->vmargin_high));
		bcm_bprintf(buf, fmt, "Vlimits");		bcm_bprintf(buf, "%umV/%umV\n", TO_MV(avs->vlimit_low), TO_MV(avs->vlimit_high));
		bcm_bprintf(buf, fmt, "voltage_margin_eq");	bcm_bprintf(buf, "slope=%dmV/V int=%dmV\n", TO_MV(avs->voltage_margin.slope), TO_MV(avs->voltage_margin.intercept));
		bcm_bprintf(buf, fmt, "threshold_margin");	bcm_bprintf(buf, "%dmV\n", TO_MV(avs->threshold_margin));

		/* Convergence process results */
		bcm_bprintf(buf, fmt, "dac_high");		bcm_bprintf(buf, "%u(%umV)\n", avs->dac_high, TO_MV(avs->vhigh));
		bcm_bprintf(buf, fmt, "dac_low");		bcm_bprintf(buf, "%u(%umV)\n", avs->dac_low, TO_MV(avs->vlow));
		bcm_bprintf(buf, fmt, "Vconv");			bcm_bprintf(buf, "%umV\n", TO_MV(avs->vconv));
		bcm_bprintf(buf, fmt, "Vmargin");		bcm_bprintf(buf, "%umV\n", TO_MV(avs->vmargin));
		bcm_bprintf(buf, fmt, "Vsum");			bcm_bprintf(buf, "%umV\n", TO_MV(avs->vsum));
		bcm_bprintf(buf, fmt, "dac_limits");		bcm_bprintf(buf, "%u/%u\n", avs->dac_code_min, avs->dac_code_max);

		/* Statistics */
		bcm_bprintf(buf, fmt, "dac_range");		bcm_bprintf(buf, "%u-%u\n", avs->dac_range[0], avs->dac_range[1]);
		bcm_bprintf(buf, fmt, "V_range");		bcm_bprintf(buf, "%u-%umV\n", TO_MV(avs->voltage_range[0]), TO_MV(avs->voltage_range[1]));
		bcm_bprintf(buf, fmt, "T_range");		bcm_bprintf(buf, "%d-%dmdC\n", TO_MDC(avs->temp_range[0]), TO_MDC(avs->temp_range[1]));
		bcm_bprintf(buf, fmt, "dac_adjustments");	bcm_bprintf(buf, "%u up/%u dn\n", avs->dac_adjustments[1], avs->dac_adjustments[0]);
		bcm_bprintf(buf, fmt, "converge_time");		bcm_bprintf(buf, "%uus\n", avs->last_converge_time_us);
		bcm_bprintf(buf, fmt, "track_time");		bcm_bprintf(buf, "%uus\n", avs->last_track_time_us);
	} else {
		bcm_bprintf(buf, "AVS not initialized\n");
	}

	return BCME_OK;
}

#endif // endif
