/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * max98090.h -- MAX98090 ALSA SoC Audio driver
 *
 * Copyright 2011 Maxim Integrated Products
 */

#ifndef _MAX98090_H
#define _MAX98090_H

#include "maxim_codec.h"

/* MAX98090 Registers Definition */

#define M98090_REG_SOFTWARE_RESET               0x00
#define M98090_REG_DEVICE_STATUS                0x01

#define M98090_REG_QUICK_SAMPLE_RATE            0x05
#define M98090_REG_DAI_INTERFACE                0x06
#define M98090_REG_DAC_PATH                     0x07

#define M98090_REG_MIC_BIAS_VOLTAGE		0x12
#define M98090_REG_DIGITAL_MIC_ENABLE		0x13
#define M98090_REG_DIGITAL_MIC_CONFIG		0x14
#define M98090_REG_SYSTEM_CLOCK			0x1B
#define M98090_REG_CLOCK_RATIO_NI_MSB		0x1D
#define M98090_REG_CLOCK_MODE			0x1C
#define M98090_REG_CLOCK_RATIO_NI_LSB		0x1E

#define M98090_REG_MASTER_MODE			0x21
#define M98090_REG_INTERFACE_FORMAT		0x22
#define M98090_REG_IO_CONFIGURATION             0x25
#define M98090_REG_FILTER_CONFIG                0x26

#define M98090_REG_LEFT_HP_MIXER                0x29
#define M98090_REG_RIGHT_HP_MIXER               0x2a
#define M98090_REG_HP_CONTROL                   0x2b
#define M98090_REG_LEFT_HP_VOLUME               0x2c
#define M98090_REG_RIGHT_HP_VOLUME              0x2d
#define M98090_REG_LEFT_SPK_MIXER               0x2e
#define M98090_REG_RIGHT_SPK_MIXER              0x2f
#define M98090_REG_SPK_CONTROL                  0x30
#define M98090_REG_LEFT_SPK_VOLUME              0x31
#define M98090_REG_RIGHT_SPK_VOLUME             0x32

#define M98090_REG_RCV_LOUTL_CONTROL            0x38
#define M98090_REG_RCV_LOUTL_VOLUME             0x39
#define M98090_REG_LOUTR_MIXER                  0x3a
#define M98090_REG_LOUTR_CONTROL                0x3b
#define M98090_REG_LOUTR_VOLUME                 0x3c
#define M98090_REG_JACK_DETECT                  0x3d
#define M98090_REG_INPUT_ENABLE                 0x3e
#define M98090_REG_OUTPUT_ENABLE                0x3f
#define M98090_REG_LEVEL_CONTROL                0x40
#define M98090_REG_DSP_FILTER_ENABLE            0x41
#define M98090_REG_BIAS_CONTROL                 0x42
#define M98090_REG_DAC_CONTROL                  0x43
#define M98090_REG_ADC_CONTROL                  0x44
#define M98090_REG_DEVICE_SHUTDOWN              0x45

#define M98090_REG_REVISION_ID                  0xff

#define M98090_REG_CNT				(0xff + 1)
#define M98090_REG_MAX_CACHed			0x45

/* MAX98090 Registers Bit Fields */

/*
 * M98090_REG_SOFTWARE_RESET		0x00
 */
#define M98090_SWRESET_MASK             BIT(7)

/*
 * M98090_REG_QUICK_SAMPLE_RATE		0x05
 */
#define M98090_SR_96K_MASK              BIT(5)
#define M98090_SR_96K_SHIFT             5
#define M98090_SR_96K_WIDTH             1
#define M98090_SR_32K_MASK              BIT(4)
#define M98090_SR_32K_SHIFT             4
#define M98090_SR_32K_WIDTH             1
#define M98090_SR_48K_MASK              BIT(3)
#define M98090_SR_48K_SHIFT             3
#define M98090_SR_48K_WIDTH             1
#define M98090_SR_44K1_MASK             BIT(2)
#define M98090_SR_44K1_SHIFT            2
#define M98090_SR_44K1_WIDTH            1
#define M98090_SR_16K_MASK              BIT(1)
#define M98090_SR_16K_SHIFT             1
#define M98090_SR_16K_WIDTH             1
#define M98090_SR_8K_MASK               BIT(0)
#define M98090_SR_8K_SHIFT              0
#define M98090_SR_8K_WIDTH              1
#define M98090_SR_MASK                  0x3F
#define M98090_SR_ALL_SHIFT             0
#define M98090_SR_ALL_WIDTH             8
#define M98090_SR_ALL_NUM               BIT(M98090_SR_ALL_WIDTH)

/*
 * M98090_REG_DAI_INTERFACE		0x06
 */
#define M98090_RJ_M_MASK                BIT(5)
#define M98090_RJ_M_SHIFT               5
#define M98090_RJ_M_WIDTH               1
#define M98090_RJ_S_MASK                BIT(4)
#define M98090_RJ_S_SHIFT               4
#define M98090_RJ_S_WIDTH               1
#define M98090_LJ_M_MASK                BIT(3)
#define M98090_LJ_M_SHIFT               3
#define M98090_LJ_M_WIDTH               1
#define M98090_LJ_S_MASK                BIT(2)
#define M98090_LJ_S_SHIFT               2
#define M98090_LJ_S_WIDTH               1
#define M98090_I2S_M_MASK               BIT(1)
#define M98090_I2S_M_SHIFT              1
#define M98090_I2S_M_WIDTH              1
#define M98090_I2S_S_MASK               BIT(0)
#define M98090_I2S_S_SHIFT              0
#define M98090_I2S_S_WIDTH              1
#define M98090_DAI_ALL_SHIFT            0
#define M98090_DAI_ALL_WIDTH            8
#define M98090_DAI_ALL_NUM              BIT(M98090_DAI_ALL_WIDTH)

/*
 * M98090_REG_DAC_PATH			0x07
 */
#define M98090_DIG2_HP_MASK             BIT(7)
#define M98090_DIG2_HP_SHIFT            7
#define M98090_DIG2_HP_WIDTH            1
#define M98090_DIG2_EAR_MASK            BIT(6)
#define M98090_DIG2_EAR_SHIFT           6
#define M98090_DIG2_EAR_WIDTH           1
#define M98090_DIG2_SPK_MASK            BIT(5)
#define M98090_DIG2_SPK_SHIFT           5
#define M98090_DIG2_SPK_WIDTH           1
#define M98090_DIG2_LOUT_MASK           BIT(4)
#define M98090_DIG2_LOUT_SHIFT          4
#define M98090_DIG2_LOUT_WIDTH          1
#define M98090_DIG2_ALL_SHIFT           0
#define M98090_DIG2_ALL_WIDTH           8
#define M98090_DIG2_ALL_NUM             BIT(M98090_DIG2_ALL_WIDTH)

/*
 * M98090_REG_MIC_BIAS_VOLTAGE		0x12
 */
#define M98090_MBVSEL_MASK              (3 << 0)
#define M98090_MBVSEL_SHIFT             0
#define M98090_MBVSEL_WIDTH             2
#define M98090_MBVSEL_2V8               (3 << 0)
#define M98090_MBVSEL_2V55              (2 << 0)
#define M98090_MBVSEL_2V4               BIT(0)
#define M98090_MBVSEL_2V2               (0 << 0)

/*
 * M98090_REG_DIGITAL_MIC_ENABLE	0x13
 */
#define M98090_MICCLK_MASK		(7 << 4)
#define M98090_MICCLK_SHIFT		4
#define M98090_MICCLK_WIDTH		3
#define M98090_DIGMIC4_MASK		BIT(3)
#define M98090_DIGMIC4_SHIFT		3
#define M98090_DIGMIC4_WIDTH		1
#define M98090_DIGMIC4_NUM		BIT(M98090_DIGMIC4_WIDTH)
#define M98090_DIGMIC3_MASK		BIT(2)
#define M98090_DIGMIC3_SHIFT		2
#define M98090_DIGMIC3_WIDTH		1
#define M98090_DIGMIC3_NUM		BIT(M98090_DIGMIC3_WIDTH)
#define M98090_DIGMICR_MASK		BIT(1)
#define M98090_DIGMICR_SHIFT		1
#define M98090_DIGMICR_WIDTH		1
#define M98090_DIGMICR_NUM		BIT(M98090_DIGMICR_WIDTH)
#define M98090_DIGMICL_MASK		BIT(0)
#define M98090_DIGMICL_SHIFT		0
#define M98090_DIGMICL_WIDTH		1
#define M98090_DIGMICL_NUM		BIT(M98090_DIGMICL_WIDTH)

/*
 * M98090_REG_DIGITAL_MIC_CONFIG	0x14
 */
#define M98090_DMIC_COMP_MASK		(15 << 4)
#define M98090_DMIC_COMP_SHIFT		4
#define M98090_DMIC_COMP_WIDTH		4
#define M98090_DMIC_COMP_NUM		BIT(M98090_DMIC_COMP_WIDTH)
#define M98090_DMIC_FREQ_MASK		(3 << 0)
#define M98090_DMIC_FREQ_SHIFT		0
#define M98090_DMIC_FREQ_WIDTH		2

/*
 * M98090_REG_CLOCK_MODE		0x1B
 */
#define M98090_PSCLK_MASK               (3 << 4)
#define M98090_PSCLK_SHIFT              4
#define M98090_PSCLK_WIDTH              2
#define M98090_PSCLK_DISABLED           (0 << 4)
#define M98090_PSCLK_DIV1               BIT(4)
#define M98090_PSCLK_DIV2               (2 << 4)
#define M98090_PSCLK_DIV4               (3 << 4)

/*
 * M98090_REG_INTERFACE_FORMAT		0x22
 */
#define M98090_RJ_MASK			BIT(5)
#define M98090_RJ_SHIFT			5
#define M98090_RJ_WIDTH			1
#define M98090_WCI_MASK			BIT(4)
#define M98090_WCI_SHIFT		4
#define M98090_WCI_WIDTH		1
#define M98090_BCI_MASK			BIT(3)
#define M98090_BCI_SHIFT		3
#define M98090_BCI_WIDTH		1
#define M98090_DLY_MASK			BIT(2)
#define M98090_DLY_SHIFT		2
#define M98090_DLY_WIDTH		1
#define M98090_WS_MASK			(3 << 0)
#define M98090_WS_SHIFT			0
#define M98090_WS_WIDTH			2
#define M98090_WS_NUM			BIT(M98090_WS_WIDTH)

/* M98090_REG_IO_CONFIGURATION	0x25 */
#define M98090_LTEN_MASK                BIT(5)
#define M98090_LTEN_SHIFT               5
#define M98090_LTEN_WIDTH               1
#define M98090_LTEN_NUM                 BIT(M98090_LTEN_WIDTH)
#define M98090_LBEN_MASK                BIT(4)
#define M98090_LBEN_SHIFT               4
#define M98090_LBEN_WIDTH               1
#define M98090_LBEN_NUM                 BIT(M98090_LBEN_WIDTH)
#define M98090_DMONO_MASK               BIT(3)
#define M98090_DMONO_SHIFT              3
#define M98090_DMONO_WIDTH              1
#define M98090_DMONO_NUM                BIT(M98090_DMONO_WIDTH)
#define M98090_HIZOFF_MASK              BIT(2)
#define M98090_HIZOFF_SHIFT             2
#define M98090_HIZOFF_WIDTH             1
#define M98090_HIZOFF_NUM               BIT(M98090_HIZOFF_WIDTH)
#define M98090_SDOEN_MASK               BIT(1)
#define M98090_SDOEN_SHIFT              1
#define M98090_SDOEN_WIDTH              1
#define M98090_SDOEN_NUM                BIT(M98090_SDOEN_WIDTH)
#define M98090_SDIEN_MASK               BIT(0)
#define M98090_SDIEN_SHIFT              0
#define M98090_SDIEN_WIDTH              1
#define M98090_SDIEN_NUM                BIT(M98090_SDIEN_WIDTH)

/*
 * M98090_REG_FILTER_CONFIG		0x26
 */
#define M98090_MODE_MASK                BIT(7)
#define M98090_MODE_SHIFT               7
#define M98090_MODE_WIDTH               1
#define M98090_AHPF_MASK                BIT(6)
#define M98090_AHPF_SHIFT               6
#define M98090_AHPF_WIDTH               1
#define M98090_AHPF_NUM                 BIT(M98090_AHPF_WIDTH)
#define M98090_DHPF_MASK                BIT(5)
#define M98090_DHPF_SHIFT               5
#define M98090_DHPF_WIDTH               1
#define M98090_DHPF_NUM                 BIT(M98090_DHPF_WIDTH)
#define M98090_DHF_MASK                 BIT(4)
#define M98090_DHF_SHIFT                4
#define M98090_DHF_WIDTH                1
#define M98090_FLT_DMIC34MODE_MASK      BIT(3)
#define M98090_FLT_DMIC34MODE_SHIFT     3
#define M98090_FLT_DMIC34MODE_WIDTH     1
#define M98090_FLT_DMIC34HPF_MASK       BIT(2)
#define M98090_FLT_DMIC34HPF_SHIFT      2
#define M98090_FLT_DMIC34HPF_WIDTH      1
#define M98090_FLT_DMIC34HPF_NUM        BIT(M98090_FLT_DMIC34HPF_WIDTH)

/*
 * M98090_REG_CLOCK_MODE
 */
#define M98090_FREQ_MASK		(15 << 4)
#define M98090_FREQ_SHIFT		4
#define M98090_FREQ_WIDTH		4
#define M98090_USE_M1_MASK		BIT(0)
#define M98090_USE_M1_SHIFT		0
#define M98090_USE_M1_WIDTH		1
#define M98090_USE_M1_NUM		BIT(M98090_USE_M1_WIDTH)

/*
 * M98090_REG_LEFT_HP_MIXER		0x29
 */
#define M98090_MIXHPL_MIC2_MASK         BIT(5)
#define M98090_MIXHPL_MIC2_SHIFT        5
#define M98090_MIXHPL_MIC2_WIDTH        1
#define M98090_MIXHPL_MIC1_MASK         BIT(4)
#define M98090_MIXHPL_MIC1_SHIFT        4
#define M98090_MIXHPL_MIC1_WIDTH        1
#define M98090_MIXHPL_LINEB_MASK        BIT(3)
#define M98090_MIXHPL_LINEB_SHIFT       3
#define M98090_MIXHPL_LINEB_WIDTH       1
#define M98090_MIXHPL_LINEA_MASK        BIT(2)
#define M98090_MIXHPL_LINEA_SHIFT       2
#define M98090_MIXHPL_LINEA_WIDTH       1
#define M98090_MIXHPL_DACR_MASK         BIT(1)
#define M98090_MIXHPL_DACR_SHIFT        1
#define M98090_MIXHPL_DACR_WIDTH        1
#define M98090_MIXHPL_DACL_MASK         BIT(0)
#define M98090_MIXHPL_DACL_SHIFT        0
#define M98090_MIXHPL_DACL_WIDTH        1
#define M98090_MIXHPL_MASK              (63 << 0)
#define M98090_MIXHPL_SHIFT             0
#define M98090_MIXHPL_WIDTH             6

/*
 * M98090_REG_RIGHT_HP_MIXER		0x2A
 */
#define M98090_MIXHPR_MIC2_MASK         BIT(5)
#define M98090_MIXHPR_MIC2_SHIFT        5
#define M98090_MIXHPR_MIC2_WIDTH        1
#define M98090_MIXHPR_MIC1_MASK         BIT(4)
#define M98090_MIXHPR_MIC1_SHIFT        4
#define M98090_MIXHPR_MIC1_WIDTH        1
#define M98090_MIXHPR_LINEB_MASK        BIT(3)
#define M98090_MIXHPR_LINEB_SHIFT       3
#define M98090_MIXHPR_LINEB_WIDTH       1
#define M98090_MIXHPR_LINEA_MASK        BIT(2)
#define M98090_MIXHPR_LINEA_SHIFT       2
#define M98090_MIXHPR_LINEA_WIDTH       1
#define M98090_MIXHPR_DACR_MASK         BIT(1)
#define M98090_MIXHPR_DACR_SHIFT        1
#define M98090_MIXHPR_DACR_WIDTH        1
#define M98090_MIXHPR_DACL_MASK         BIT(0)
#define M98090_MIXHPR_DACL_SHIFT        0
#define M98090_MIXHPR_DACL_WIDTH        1
#define M98090_MIXHPR_MASK              (63 << 0)
#define M98090_MIXHPR_SHIFT             0
#define M98090_MIXHPR_WIDTH             6

/*
 * M98090_REG_LEFT_HP_VOLUME		0x2C
 */
#define M98090_HPLM_MASK                BIT(7)
#define M98090_HPLM_SHIFT               7
#define M98090_HPLM_WIDTH               1
#define M98090_HPVOLL_MASK              (31 << 0)
#define M98090_HPVOLL_SHIFT             0
#define M98090_HPVOLL_WIDTH             5
#define M98090_HPVOLL_NUM               BIT(M98090_HPVOLL_WIDTH)

/*
 * M98090_REG_RIGHT_HP_VOLUME	0x2D
 */
#define M98090_HPRM_MASK                BIT(7)
#define M98090_HPRM_SHIFT               7
#define M98090_HPRM_WIDTH               1
#define M98090_HPVOLR_MASK              (31 << 0)
#define M98090_HPVOLR_SHIFT             0
#define M98090_HPVOLR_WIDTH             5
#define M98090_HPVOLR_NUM               BIT(M98090_HPVOLR_WIDTH)

/*
 * M98090_REG_LEFT_SPK_MIXER		0x2E
 */
#define M98090_MIXSPL_MIC2_MASK         BIT(5)
#define M98090_MIXSPL_MIC2_SHIFT        5
#define M98090_MIXSPL_MIC2_WIDTH        1
#define M98090_MIXSPL_MIC1_MASK         BIT(4)
#define M98090_MIXSPL_MIC1_SHIFT        4
#define M98090_MIXSPL_MIC1_WIDTH        1
#define M98090_MIXSPL_LINEB_MASK        BIT(3)
#define M98090_MIXSPL_LINEB_SHIFT       3
#define M98090_MIXSPL_LINEB_WIDTH       1
#define M98090_MIXSPL_LINEA_MASK        BIT(2)
#define M98090_MIXSPL_LINEA_SHIFT       2
#define M98090_MIXSPL_LINEA_WIDTH       1
#define M98090_MIXSPL_DACR_MASK         BIT(1)
#define M98090_MIXSPL_DACR_SHIFT        1
#define M98090_MIXSPL_DACR_WIDTH        1
#define M98090_MIXSPL_DACL_MASK         BIT(0)
#define M98090_MIXSPL_DACL_SHIFT        0
#define M98090_MIXSPL_DACL_WIDTH        1
#define M98090_MIXSPL_MASK              (63 << 0)
#define M98090_MIXSPL_SHIFT             0
#define M98090_MIXSPL_WIDTH             6
#define M98090_MIXSPR_DACR_MASK         BIT(1)
#define M98090_MIXSPR_DACR_SHIFT        1
#define M98090_MIXSPR_DACR_WIDTH        1

/*
 * M98090_REG_RIGHT_SPK_MIXER		0x2F
 */
#define M98090_SPK_SLAVE_MASK           BIT(6)
#define M98090_SPK_SLAVE_SHIFT          6
#define M98090_SPK_SLAVE_WIDTH          1
#define M98090_MIXSPR_MIC2_MASK         BIT(5)
#define M98090_MIXSPR_MIC2_SHIFT        5
#define M98090_MIXSPR_MIC2_WIDTH        1
#define M98090_MIXSPR_MIC1_MASK         BIT(4)
#define M98090_MIXSPR_MIC1_SHIFT        4
#define M98090_MIXSPR_MIC1_WIDTH        1
#define M98090_MIXSPR_LINEB_MASK        BIT(3)
#define M98090_MIXSPR_LINEB_SHIFT       3
#define M98090_MIXSPR_LINEB_WIDTH       1
#define M98090_MIXSPR_LINEA_MASK        BIT(2)
#define M98090_MIXSPR_LINEA_SHIFT       2
#define M98090_MIXSPR_LINEA_WIDTH       1
#define M98090_MIXSPR_DACR_MASK         BIT(1)
#define M98090_MIXSPR_DACR_SHIFT        1
#define M98090_MIXSPR_DACR_WIDTH        1
#define M98090_MIXSPR_DACL_MASK         BIT(0)
#define M98090_MIXSPR_DACL_SHIFT        0
#define M98090_MIXSPR_DACL_WIDTH        1
#define M98090_MIXSPR_MASK              (63 << 0)
#define M98090_MIXSPR_SHIFT             0
#define M98090_MIXSPR_WIDTH             6

/*
 * M98090_REG_LEFT_SPK_VOLUME		0x31
 */
#define M98090_SPLM_MASK                BIT(7)
#define M98090_SPLM_SHIFT               7
#define M98090_SPLM_WIDTH               1
#define M98090_SPVOLL_MASK              (63 << 0)
#define M98090_SPVOLL_SHIFT             0
#define M98090_SPVOLL_WIDTH             6
#define M98090_SPVOLL_NUM               40

/*
 * M98090_REG_RIGHT_SPK_VOLUME		0x32
 */
#define M98090_SPRM_MASK                BIT(7)
#define M98090_SPRM_SHIFT               7
#define M98090_SPRM_WIDTH               1
#define M98090_SPVOLR_MASK              (63 << 0)
#define M98090_SPVOLR_SHIFT             0
#define M98090_SPVOLR_WIDTH             6
#define M98090_SPVOLR_NUM               40

/*
 * M98090_REG_RCV_LOUTL_MIXER		0x37
 */
#define M98090_MIXRCVL_MIC2_MASK        BIT(5)
#define M98090_MIXRCVL_MIC2_SHIFT       5
#define M98090_MIXRCVL_MIC2_WIDTH       1
#define M98090_MIXRCVL_MIC1_MASK        BIT(4)
#define M98090_MIXRCVL_MIC1_SHIFT       4
#define M98090_MIXRCVL_MIC1_WIDTH       1
#define M98090_MIXRCVL_LINEB_MASK       BIT(3)
#define M98090_MIXRCVL_LINEB_SHIFT      3
#define M98090_MIXRCVL_LINEB_WIDTH      1
#define M98090_MIXRCVL_LINEA_MASK       BIT(2)
#define M98090_MIXRCVL_LINEA_SHIFT      2
#define M98090_MIXRCVL_LINEA_WIDTH      1
#define M98090_MIXRCVL_DACR_MASK        BIT(1)
#define M98090_MIXRCVL_DACR_SHIFT       1
#define M98090_MIXRCVL_DACR_WIDTH       1
#define M98090_MIXRCVL_DACL_MASK        BIT(0)
#define M98090_MIXRCVL_DACL_SHIFT       0
#define M98090_MIXRCVL_DACL_WIDTH       1
#define M98090_MIXRCVL_MASK             (63 << 0)
#define M98090_MIXRCVL_SHIFT            0
#define M98090_MIXRCVL_WIDTH            6

/*
 * M98090_REG_RCV_LOUTL_CONTROL		0x38
 */
#define M98090_MIXRCVLG_MASK            (3 << 0)
#define M98090_MIXRCVLG_SHIFT           0
#define M98090_MIXRCVLG_WIDTH           2
#define M98090_MIXRCVLG_NUM             BIT(M98090_MIXRCVLG_WIDTH)

/*
 * M98090_REG_RCV_LOUTL_VOLUME		0x39
 */
#define M98090_RCVLM_MASK               BIT(7)
#define M98090_RCVLM_SHIFT              7
#define M98090_RCVLM_WIDTH              1
#define M98090_RCVLVOL_MASK             (31 << 0)
#define M98090_RCVLVOL_SHIFT            0
#define M98090_RCVLVOL_WIDTH            5
#define M98090_RCVLVOL_NUM              BIT(M98090_RCVLVOL_WIDTH)

/*
 * M98090_REG_LOUTR_MIXER		0x3A
 */
#define M98090_LINMOD_MASK              BIT(7)
#define M98090_LINMOD_SHIFT             7
#define M98090_LINMOD_WIDTH             1
#define M98090_MIXRCVR_MIC2_MASK        BIT(5)
#define M98090_MIXRCVR_MIC2_SHIFT       5
#define M98090_MIXRCVR_MIC2_WIDTH       1
#define M98090_MIXRCVR_MIC1_MASK        BIT(4)
#define M98090_MIXRCVR_MIC1_SHIFT       4
#define M98090_MIXRCVR_MIC1_WIDTH       1
#define M98090_MIXRCVR_LINEB_MASK       BIT(3)
#define M98090_MIXRCVR_LINEB_SHIFT      3
#define M98090_MIXRCVR_LINEB_WIDTH      1
#define M98090_MIXRCVR_LINEA_MASK       BIT(2)
#define M98090_MIXRCVR_LINEA_SHIFT      2
#define M98090_MIXRCVR_LINEA_WIDTH      1
#define M98090_MIXRCVR_DACR_MASK        BIT(1)
#define M98090_MIXRCVR_DACR_SHIFT       1
#define M98090_MIXRCVR_DACR_WIDTH       1
#define M98090_MIXRCVR_DACL_MASK        BIT(0)
#define M98090_MIXRCVR_DACL_SHIFT       0
#define M98090_MIXRCVR_DACL_WIDTH       1
#define M98090_MIXRCVR_MASK             (63 << 0)
#define M98090_MIXRCVR_SHIFT            0
#define M98090_MIXRCVR_WIDTH            6

/*
 * M98090_REG_LOUTR_VOLUME		0x3C
 */
#define M98090_RCVRM_MASK               BIT(7)
#define M98090_RCVRM_SHIFT              7
#define M98090_RCVRM_WIDTH              1
#define M98090_RCVRVOL_MASK             (31 << 0)
#define M98090_RCVRVOL_SHIFT            0
#define M98090_RCVRVOL_WIDTH            5
#define M98090_RCVRVOL_NUM              BIT(M98090_RCVRVOL_WIDTH)

/*
 * M98090_REG_JACK_DETECT		0x3D
 */
#define M98090_JDETEN_MASK              BIT(7)
#define M98090_JDETEN_SHIFT             7
#define M98090_JDETEN_WIDTH             1
#define M98090_JDWK_MASK                BIT(6)
#define M98090_JDWK_SHIFT               6
#define M98090_JDWK_WIDTH               1
#define M98090_JDEB_MASK                (3 << 0)
#define M98090_JDEB_SHIFT               0
#define M98090_JDEB_WIDTH               2
#define M98090_JDEB_25MS                (0 << 0)
#define M98090_JDEB_50MS                BIT(0)
#define M98090_JDEB_100MS               (2 << 0)
#define M98090_JDEB_200MS               (3 << 0)

/*
 * M98090_REG_INPUT_ENABLE		0x3E
 */
#define M98090_MBEN_MASK                BIT(4)
#define M98090_MBEN_SHIFT               4
#define M98090_MBEN_WIDTH               1
#define M98090_LINEAEN_MASK             BIT(3)
#define M98090_LINEAEN_SHIFT            3
#define M98090_LINEAEN_WIDTH            1
#define M98090_LINEBEN_MASK             BIT(2)
#define M98090_LINEBEN_SHIFT            2
#define M98090_LINEBEN_WIDTH            1
#define M98090_ADREN_MASK               BIT(1)
#define M98090_ADREN_SHIFT              1
#define M98090_ADREN_WIDTH              1
#define M98090_ADLEN_MASK               BIT(0)
#define M98090_ADLEN_SHIFT              0
#define M98090_ADLEN_WIDTH              1

/*
 * M98090_REG_OUTPUT_ENABLE		0x3F
 */
#define M98090_HPREN_MASK               BIT(7)
#define M98090_HPREN_SHIFT              7
#define M98090_HPREN_WIDTH              1
#define M98090_HPLEN_MASK               BIT(6)
#define M98090_HPLEN_SHIFT              6
#define M98090_HPLEN_WIDTH              1
#define M98090_SPREN_MASK               BIT(5)
#define M98090_SPREN_SHIFT              5
#define M98090_SPREN_WIDTH              1
#define M98090_SPLEN_MASK               BIT(4)
#define M98090_SPLEN_SHIFT              4
#define M98090_SPLEN_WIDTH              1
#define M98090_RCVLEN_MASK              BIT(3)
#define M98090_RCVLEN_SHIFT             3
#define M98090_RCVLEN_WIDTH             1
#define M98090_RCVREN_MASK              BIT(2)
#define M98090_RCVREN_SHIFT             2
#define M98090_RCVREN_WIDTH             1
#define M98090_DAREN_MASK               BIT(1)
#define M98090_DAREN_SHIFT              1
#define M98090_DAREN_WIDTH              1
#define M98090_DALEN_MASK               BIT(0)
#define M98090_DALEN_SHIFT              0
#define M98090_DALEN_WIDTH              1

/*
 * M98090_REG_LEVEL_CONTROL		0x40
 */
#define M98090_ZDENN_MASK               BIT(2)
#define M98090_ZDENN_SHIFT              2
#define M98090_ZDENN_WIDTH              1
#define M98090_ZDENN_NUM                BIT(M98090_ZDENN_WIDTH)
#define M98090_VS2ENN_MASK              BIT(1)
#define M98090_VS2ENN_SHIFT             1
#define M98090_VS2ENN_WIDTH             1
#define M98090_VS2ENN_NUM               BIT(M98090_VS2ENN_WIDTH)
#define M98090_VSENN_MASK               BIT(0)
#define M98090_VSENN_SHIFT              0
#define M98090_VSENN_WIDTH              1
#define M98090_VSENN_NUM                BIT(M98090_VSENN_WIDTH)

/*
 * M98090_REG_BIAS_CONTROL		0x42
 */
#define M98090_VCM_MODE_MASK            BIT(0)
#define M98090_VCM_MODE_SHIFT           0
#define M98090_VCM_MODE_WIDTH           1
#define M98090_VCM_MODE_NUM             BIT(M98090_VCM_MODE_WIDTH)

/*
 * M98090_REG_DAC_CONTROL		0x43
 */
#define M98090_PERFMODE_MASK            BIT(1)
#define M98090_PERFMODE_SHIFT           1
#define M98090_PERFMODE_WIDTH           1
#define M98090_PERFMODE_NUM             BIT(M98090_PERFMODE_WIDTH)
#define M98090_DACHP_MASK               BIT(0)
#define M98090_DACHP_SHIFT              0
#define M98090_DACHP_WIDTH              1
#define M98090_DACHP_NUM                BIT(M98090_DACHP_WIDTH)

/*
 * M98090_REG_ADC_CONTROL		0x44
 */
#define M98090_OSR128_MASK              BIT(2)
#define M98090_OSR128_SHIFT             2
#define M98090_OSR128_WIDTH             1
#define M98090_ADCDITHER_MASK           BIT(1)
#define M98090_ADCDITHER_SHIFT          1
#define M98090_ADCDITHER_WIDTH          1
#define M98090_ADCDITHER_NUM            BIT(M98090_ADCDITHER_WIDTH)
#define M98090_ADCHP_MASK               BIT(0)
#define M98090_ADCHP_SHIFT              0
#define M98090_ADCHP_WIDTH              1
#define M98090_ADCHP_NUM                BIT(M98090_ADCHP_WIDTH)

/*
 * M98090_REG_DEVICE_SHUTDOWN		0x45
 */
#define M98090_SHDNN_MASK               BIT(7)
#define M98090_SHDNN_SHIFT              7
#define M98090_SHDNN_WIDTH              1

/*
 * M98090_REG_REVISION_ID		0xFF
 */
#define M98090_REVID_MASK               (255 << 0)
#define M98090_REVID_SHIFT              0
#define M98090_REVID_WIDTH              8
#define M98090_REVID_NUM                BIT(M98090_REVID_WIDTH)

/* function prototype */

/*
 * initialise max98090 sound codec device for the given configuration
 *
 * @param blob			FDT node for codec values
 * @param sampling_rate		Sampling rate (Hz)
 * @param mclk_freq		MCLK Frequency (Hz)
 * @param bits_per_sample	bits per Sample (must be 16 or 24)
 *
 * @returns -1 for error and 0 Success.
 */
int max98090_init(const void *blob, int sampling_rate, int mclk_freq,
		  int bits_per_sample);
int max98090_set_sysclk(struct maxim_priv *max98090, uint freq);
int max98090_hw_params(struct maxim_priv *max98090, uint rate,
		       uint bits_per_sample);
int max98090_device_init(struct maxim_priv *max98090);
int max98090_set_fmt(struct maxim_priv *max98090, int fmt);
#endif
