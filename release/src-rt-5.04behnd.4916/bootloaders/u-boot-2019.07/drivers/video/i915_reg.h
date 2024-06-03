/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright 2003 Tungsten Graphics, Inc., Cedar Park, Texas.
 * All Rights Reserved.
 */

#ifndef _I915_REG_H_
#define _I915_REG_H_

/* Hotplug control (945+ only) */
#define PORT_HOTPLUG_EN		0x61110
#define   HDMIB_HOTPLUG_INT_EN			(1 << 29)
#define   DPB_HOTPLUG_INT_EN			(1 << 29)
#define   HDMIC_HOTPLUG_INT_EN			(1 << 28)
#define   DPC_HOTPLUG_INT_EN			(1 << 28)
#define   HDMID_HOTPLUG_INT_EN			(1 << 27)
#define   DPD_HOTPLUG_INT_EN			(1 << 27)
#define   SDVOB_HOTPLUG_INT_EN			(1 << 26)
#define   SDVOC_HOTPLUG_INT_EN			(1 << 25)
#define   TV_HOTPLUG_INT_EN			(1 << 18)
#define   CRT_HOTPLUG_INT_EN			(1 << 9)
#define   CRT_HOTPLUG_FORCE_DETECT		(1 << 3)
#define CRT_HOTPLUG_ACTIVATION_PERIOD_32	(0 << 8)
/* must use period 64 on GM45 according to docs */
#define CRT_HOTPLUG_ACTIVATION_PERIOD_64	(1 << 8)
#define CRT_HOTPLUG_DAC_ON_TIME_2M		(0 << 7)
#define CRT_HOTPLUG_DAC_ON_TIME_4M		(1 << 7)
#define CRT_HOTPLUG_VOLTAGE_COMPARE_40		(0 << 5)
#define CRT_HOTPLUG_VOLTAGE_COMPARE_50		(1 << 5)
#define CRT_HOTPLUG_VOLTAGE_COMPARE_60		(2 << 5)
#define CRT_HOTPLUG_VOLTAGE_COMPARE_70		(3 << 5)
#define CRT_HOTPLUG_VOLTAGE_COMPARE_MASK	(3 << 5)
#define CRT_HOTPLUG_DETECT_DELAY_1G		(0 << 4)
#define CRT_HOTPLUG_DETECT_DELAY_2G		(1 << 4)
#define CRT_HOTPLUG_DETECT_VOLTAGE_325MV	(0 << 2)
#define CRT_HOTPLUG_DETECT_VOLTAGE_475MV	(1 << 2)

/* Backlight control */
#define BLC_PWM_CTL2		0x61250 /* 965+ only */
#define   BLM_PWM_ENABLE		(1 << 31)
#define   BLM_COMBINATION_MODE		(1 << 30) /* gen4 only */
#define   BLM_PIPE_SELECT		(1 << 29)
#define   BLM_PIPE_SELECT_IVB		(3 << 29)
#define   BLM_PIPE_A			(0 << 29)
#define   BLM_PIPE_B			(1 << 29)
#define   BLM_PIPE_C			(2 << 29) /* ivb + */
#define   BLM_PIPE(pipe)		((pipe) << 29)
#define   BLM_POLARITY_I965		(1 << 28) /* gen4 only */
#define   BLM_PHASE_IN_INTERUPT_STATUS	(1 << 26)
#define   BLM_PHASE_IN_ENABLE		(1 << 25)
#define   BLM_PHASE_IN_INTERUPT_ENABL	(1 << 24)
#define   BLM_PHASE_IN_TIME_BASE_SHIFT	(16)
#define   BLM_PHASE_IN_TIME_BASE_MASK	(0xff << 16)
#define   BLM_PHASE_IN_COUNT_SHIFT	(8)
#define   BLM_PHASE_IN_COUNT_MASK	(0xff << 8)
#define   BLM_PHASE_IN_INCR_SHIFT	(0)
#define   BLM_PHASE_IN_INCR_MASK	(0xff << 0)
#define BLC_PWM_CTL		0x61254
/*
 * This is the most significant 15 bits of the number of backlight cycles in a
 * complete cycle of the modulated backlight control.
 *
 * The actual value is this field multiplied by two.
 */
#define   BACKLIGHT_MODULATION_FREQ_SHIFT	(17)
#define   BACKLIGHT_MODULATION_FREQ_MASK	(0x7fff << 17)
#define   BLM_LEGACY_MODE			(1 << 16) /* gen2 only */
/*
 * This is the number of cycles out of the backlight modulation cycle for which
 * the backlight is on.
 *
 * This field must be no greater than the number of cycles in the complete
 * backlight modulation cycle.
 */
#define   BACKLIGHT_DUTY_CYCLE_SHIFT		(0)
#define   BACKLIGHT_DUTY_CYCLE_MASK		(0xffff)
#define   BACKLIGHT_DUTY_CYCLE_MASK_PNV		(0xfffe)
#define   BLM_POLARITY_PNV			(1 << 0) /* pnv only */

#define BLC_HIST_CTL		0x61260

/*
 * New registers for PCH-split platforms. Safe where new bits show up, the
 * register layout machtes with gen4 BLC_PWM_CTL[12]
 */
#define BLC_PWM_CPU_CTL2	0x48250
#define  BLC_PWM2_ENABLE        (1<<31)
#define BLC_PWM_CPU_CTL		0x48254

#define BLM_HIST_CTL			0x48260
#define  ENH_HIST_ENABLE		(1<<31)
#define  ENH_MODIF_TBL_ENABLE		(1<<30)
#define  ENH_PIPE_A_SELECT		(0<<29)
#define  ENH_PIPE_B_SELECT		(1<<29)
#define  ENH_PIPE(pipe) _PIPE(pipe, ENH_PIPE_A_SELECT, ENH_PIPE_B_SELECT)
#define  HIST_MODE_YUV			(0<<24)
#define  HIST_MODE_HSV			(1<<24)
#define  ENH_MODE_DIRECT		(0<<13)
#define  ENH_MODE_ADDITIVE		(1<<13)
#define  ENH_MODE_MULTIPLICATIVE	(2<<13)
#define  BIN_REGISTER_SET		(1<<11)
#define  ENH_NUM_BINS			32

#define BLM_HIST_ENH			0x48264

#define BLM_HIST_GUARD_BAND		0x48268
#define  BLM_HIST_INTR_ENABLE		(1<<31)
#define  BLM_HIST_EVENT_STATUS		(1<<30)
#define  BLM_HIST_INTR_DELAY_MASK	(0xFF<<22)
#define  BLM_HIST_INTR_DELAY_SHIFT	22

/*
 * PCH CTL1 is totally different, all but the below bits are reserved. CTL2 is
 * like the normal CTL from gen4 and earlier. Hooray for confusing naming.
 */
#define BLC_PWM_PCH_CTL1	0xc8250
#define   BLM_PCH_PWM_ENABLE			(1 << 31)
#define   BLM_PCH_OVERRIDE_ENABLE		(1 << 30)
#define   BLM_PCH_POLARITY			(1 << 29)
#define BLC_PWM_PCH_CTL2	0xc8254

/* digital port hotplug */
#define PCH_PORT_HOTPLUG        0xc4030		/* SHOTPLUG_CTL */
#define PORTD_HOTPLUG_ENABLE            (1 << 20)
#define PORTD_PULSE_DURATION_2ms        (0)
#define PORTD_PULSE_DURATION_4_5ms      (1 << 18)
#define PORTD_PULSE_DURATION_6ms        (2 << 18)
#define PORTD_PULSE_DURATION_100ms      (3 << 18)
#define PORTD_PULSE_DURATION_MASK	(3 << 18)
#define PORTD_HOTPLUG_NO_DETECT         (0)
#define PORTD_HOTPLUG_SHORT_DETECT      (1 << 16)
#define PORTD_HOTPLUG_LONG_DETECT       (1 << 17)
#define PORTC_HOTPLUG_ENABLE            (1 << 12)
#define PORTC_PULSE_DURATION_2ms        (0)
#define PORTC_PULSE_DURATION_4_5ms      (1 << 10)
#define PORTC_PULSE_DURATION_6ms        (2 << 10)
#define PORTC_PULSE_DURATION_100ms      (3 << 10)
#define PORTC_PULSE_DURATION_MASK	(3 << 10)
#define PORTC_HOTPLUG_NO_DETECT         (0)
#define PORTC_HOTPLUG_SHORT_DETECT      (1 << 8)
#define PORTC_HOTPLUG_LONG_DETECT       (1 << 9)
#define PORTB_HOTPLUG_ENABLE            (1 << 4)
#define PORTB_PULSE_DURATION_2ms        (0)
#define PORTB_PULSE_DURATION_4_5ms      (1 << 2)
#define PORTB_PULSE_DURATION_6ms        (2 << 2)
#define PORTB_PULSE_DURATION_100ms      (3 << 2)
#define PORTB_PULSE_DURATION_MASK	(3 << 2)
#define PORTB_HOTPLUG_NO_DETECT         (0)
#define PORTB_HOTPLUG_SHORT_DETECT      (1 << 0)
#define PORTB_HOTPLUG_LONG_DETECT       (1 << 1)

#define PCH_GPIOA               0xc5010
#define PCH_GPIOB               0xc5014
#define PCH_GPIOC               0xc5018
#define PCH_GPIOD               0xc501c
#define PCH_GPIOE               0xc5020
#define PCH_GPIOF               0xc5024

#define PCH_GMBUS0		0xc5100
#define PCH_GMBUS1		0xc5104
#define PCH_GMBUS2		0xc5108
#define PCH_GMBUS3		0xc510c
#define PCH_GMBUS4		0xc5110
#define PCH_GMBUS5		0xc5120

#define _PCH_DPLL_A              0xc6014
#define _PCH_DPLL_B              0xc6018
#define _PCH_DPLL(pll) (pll == 0 ? _PCH_DPLL_A : _PCH_DPLL_B)

#define _PCH_FPA0                0xc6040
#define  FP_CB_TUNE		(0x3<<22)
#define _PCH_FPA1                0xc6044
#define _PCH_FPB0                0xc6048
#define _PCH_FPB1                0xc604c
#define _PCH_FP0(pll) (pll == 0 ? _PCH_FPA0 : _PCH_FPB0)
#define _PCH_FP1(pll) (pll == 0 ? _PCH_FPA1 : _PCH_FPB1)

#define PCH_DPLL_TEST           0xc606c

#define PCH_DREF_CONTROL        0xC6200
#define  DREF_CONTROL_MASK      0x7fc3
#define  DREF_CPU_SOURCE_OUTPUT_DISABLE         (0<<13)
#define  DREF_CPU_SOURCE_OUTPUT_DOWNSPREAD      (2<<13)
#define  DREF_CPU_SOURCE_OUTPUT_NONSPREAD       (3<<13)
#define  DREF_CPU_SOURCE_OUTPUT_MASK		(3<<13)
#define  DREF_SSC_SOURCE_DISABLE                (0<<11)
#define  DREF_SSC_SOURCE_ENABLE                 (2<<11)
#define  DREF_SSC_SOURCE_MASK			(3<<11)
#define  DREF_NONSPREAD_SOURCE_DISABLE          (0<<9)
#define  DREF_NONSPREAD_CK505_ENABLE		(1<<9)
#define  DREF_NONSPREAD_SOURCE_ENABLE           (2<<9)
#define  DREF_NONSPREAD_SOURCE_MASK		(3<<9)
#define  DREF_SUPERSPREAD_SOURCE_DISABLE        (0<<7)
#define  DREF_SUPERSPREAD_SOURCE_ENABLE         (2<<7)
#define  DREF_SUPERSPREAD_SOURCE_MASK		(3<<7)
#define  DREF_SSC4_DOWNSPREAD                   (0<<6)
#define  DREF_SSC4_CENTERSPREAD                 (1<<6)
#define  DREF_SSC1_DISABLE                      (0<<1)
#define  DREF_SSC1_ENABLE                       (1<<1)
#define  DREF_SSC4_DISABLE                      (0)
#define  DREF_SSC4_ENABLE                       (1)

#define PCH_RAWCLK_FREQ         0xc6204
#define  FDL_TP1_TIMER_SHIFT    12
#define  FDL_TP1_TIMER_MASK     (3<<12)
#define  FDL_TP2_TIMER_SHIFT    10
#define  FDL_TP2_TIMER_MASK     (3<<10)
#define  RAWCLK_FREQ_MASK       0x3ff

#define PCH_DPLL_TMR_CFG        0xc6208

#define PCH_SSC4_PARMS          0xc6210
#define PCH_SSC4_AUX_PARMS      0xc6214

#define PCH_DPLL_SEL		0xc7000
#define  TRANSA_DPLL_ENABLE	(1<<3)
#define	 TRANSA_DPLLB_SEL	(1<<0)
#define	 TRANSA_DPLLA_SEL	0
#define  TRANSB_DPLL_ENABLE	(1<<7)
#define	 TRANSB_DPLLB_SEL	(1<<4)
#define	 TRANSB_DPLLA_SEL	(0)
#define  TRANSC_DPLL_ENABLE	(1<<11)
#define	 TRANSC_DPLLB_SEL	(1<<8)
#define	 TRANSC_DPLLA_SEL	(0)

/* transcoder */

#define _TRANS_HTOTAL_A          0xe0000
#define  TRANS_HTOTAL_SHIFT     16
#define  TRANS_HACTIVE_SHIFT    0
#define _TRANS_HBLANK_A          0xe0004
#define  TRANS_HBLANK_END_SHIFT 16
#define  TRANS_HBLANK_START_SHIFT 0
#define _TRANS_HSYNC_A           0xe0008
#define  TRANS_HSYNC_END_SHIFT  16
#define  TRANS_HSYNC_START_SHIFT 0
#define _TRANS_VTOTAL_A          0xe000c
#define  TRANS_VTOTAL_SHIFT     16
#define  TRANS_VACTIVE_SHIFT    0
#define _TRANS_VBLANK_A          0xe0010
#define  TRANS_VBLANK_END_SHIFT 16
#define  TRANS_VBLANK_START_SHIFT 0
#define _TRANS_VSYNC_A           0xe0014
#define  TRANS_VSYNC_END_SHIFT  16
#define  TRANS_VSYNC_START_SHIFT 0
#define _TRANS_VSYNCSHIFT_A	0xe0028

#define _TRANSA_DATA_M1          0xe0030
#define _TRANSA_DATA_N1          0xe0034
#define _TRANSA_DATA_M2          0xe0038
#define _TRANSA_DATA_N2          0xe003c
#define _TRANSA_DP_LINK_M1       0xe0040
#define _TRANSA_DP_LINK_N1       0xe0044
#define _TRANSA_DP_LINK_M2       0xe0048
#define _TRANSA_DP_LINK_N2       0xe004c

/* Per-transcoder DIP controls */

#define _VIDEO_DIP_CTL_A         0xe0200
#define _VIDEO_DIP_DATA_A        0xe0208
#define _VIDEO_DIP_GCP_A         0xe0210

#define _VIDEO_DIP_CTL_B         0xe1200
#define _VIDEO_DIP_DATA_B        0xe1208
#define _VIDEO_DIP_GCP_B         0xe1210

#define TVIDEO_DIP_CTL(pipe) _PIPE(pipe, _VIDEO_DIP_CTL_A, _VIDEO_DIP_CTL_B)
#define TVIDEO_DIP_DATA(pipe) _PIPE(pipe, _VIDEO_DIP_DATA_A, _VIDEO_DIP_DATA_B)
#define TVIDEO_DIP_GCP(pipe) _PIPE(pipe, _VIDEO_DIP_GCP_A, _VIDEO_DIP_GCP_B)

#define VLV_VIDEO_DIP_CTL_A		0x60200
#define VLV_VIDEO_DIP_DATA_A		0x60208
#define VLV_VIDEO_DIP_GDCP_PAYLOAD_A	0x60210

#define VLV_VIDEO_DIP_CTL_B		0x61170
#define VLV_VIDEO_DIP_DATA_B		0x61174
#define VLV_VIDEO_DIP_GDCP_PAYLOAD_B	0x61178

#define VLV_TVIDEO_DIP_CTL(pipe) \
	 _PIPE(pipe, VLV_VIDEO_DIP_CTL_A, VLV_VIDEO_DIP_CTL_B)
#define VLV_TVIDEO_DIP_DATA(pipe) \
	 _PIPE(pipe, VLV_VIDEO_DIP_DATA_A, VLV_VIDEO_DIP_DATA_B)
#define VLV_TVIDEO_DIP_GCP(pipe) \
	_PIPE(pipe, VLV_VIDEO_DIP_GDCP_PAYLOAD_A, VLV_VIDEO_DIP_GDCP_PAYLOAD_B)

/* vlv has 2 sets of panel control regs. */
#define PIPEA_PP_STATUS         0x61200
#define PIPEA_PP_CONTROL        0x61204
#define PIPEA_PP_ON_DELAYS      0x61208
#define PIPEA_PP_OFF_DELAYS     0x6120c
#define PIPEA_PP_DIVISOR        0x61210

#define PIPEB_PP_STATUS         0x61300
#define PIPEB_PP_CONTROL        0x61304
#define PIPEB_PP_ON_DELAYS      0x61308
#define PIPEB_PP_OFF_DELAYS     0x6130c
#define PIPEB_PP_DIVISOR        0x61310

#define PCH_PP_STATUS		0xc7200
#define PCH_PP_CONTROL		0xc7204
#define  PANEL_UNLOCK_REGS	(0xabcd << 16)
#define  PANEL_UNLOCK_MASK	(0xffff << 16)
#define  EDP_FORCE_VDD		(1 << 3)
#define  EDP_BLC_ENABLE		(1 << 2)
#define  PANEL_POWER_RESET	(1 << 1)
#define  PANEL_POWER_OFF	(0 << 0)
#define  PANEL_POWER_ON		(1 << 0)
#define PCH_PP_ON_DELAYS	0xc7208
#define  PANEL_PORT_SELECT_MASK	(3 << 30)
#define  PANEL_PORT_SELECT_LVDS	(0 << 30)
#define  PANEL_PORT_SELECT_DPA	(1 << 30)
#define  EDP_PANEL		(1 << 30)
#define  PANEL_PORT_SELECT_DPC	(2 << 30)
#define  PANEL_PORT_SELECT_DPD	(3 << 30)
#define  PANEL_POWER_UP_DELAY_MASK	(0x1fff0000)
#define  PANEL_POWER_UP_DELAY_SHIFT	16
#define  PANEL_LIGHT_ON_DELAY_MASK	(0x1fff)
#define  PANEL_LIGHT_ON_DELAY_SHIFT	0

#define PCH_PP_OFF_DELAYS	0xc720c
#define  PANEL_POWER_PORT_SELECT_MASK	(0x3 << 30)
#define  PANEL_POWER_PORT_LVDS		(0 << 30)
#define  PANEL_POWER_PORT_DP_A		(1 << 30)
#define  PANEL_POWER_PORT_DP_C		(2 << 30)
#define  PANEL_POWER_PORT_DP_D		(3 << 30)
#define  PANEL_POWER_DOWN_DELAY_MASK	(0x1fff0000)
#define  PANEL_POWER_DOWN_DELAY_SHIFT	16
#define  PANEL_LIGHT_OFF_DELAY_MASK	(0x1fff)
#define  PANEL_LIGHT_OFF_DELAY_SHIFT	0

#define PCH_PP_DIVISOR		0xc7210
#define  PP_REFERENCE_DIVIDER_MASK	(0xffffff00)
#define  PP_REFERENCE_DIVIDER_SHIFT	8
#define  PANEL_POWER_CYCLE_DELAY_MASK	(0x1f)
#define  PANEL_POWER_CYCLE_DELAY_SHIFT	0

#define PCH_DP_B		0xe4100
#define PCH_DPB_AUX_CH_CTL	0xe4110
#define PCH_DPB_AUX_CH_DATA1	0xe4114
#define PCH_DPB_AUX_CH_DATA2	0xe4118
#define PCH_DPB_AUX_CH_DATA3	0xe411c
#define PCH_DPB_AUX_CH_DATA4	0xe4120
#define PCH_DPB_AUX_CH_DATA5	0xe4124

#define PCH_DP_C		0xe4200
#define PCH_DPC_AUX_CH_CTL	0xe4210
#define PCH_DPC_AUX_CH_DATA1	0xe4214
#define PCH_DPC_AUX_CH_DATA2	0xe4218
#define PCH_DPC_AUX_CH_DATA3	0xe421c
#define PCH_DPC_AUX_CH_DATA4	0xe4220
#define PCH_DPC_AUX_CH_DATA5	0xe4224

#define PCH_DP_D		0xe4300
#define PCH_DPD_AUX_CH_CTL	0xe4310
#define PCH_DPD_AUX_CH_DATA1	0xe4314
#define PCH_DPD_AUX_CH_DATA2	0xe4318
#define PCH_DPD_AUX_CH_DATA3	0xe431c
#define PCH_DPD_AUX_CH_DATA4	0xe4320
#define PCH_DPD_AUX_CH_DATA5	0xe4324

#endif /* _I915_REG_H_ */
