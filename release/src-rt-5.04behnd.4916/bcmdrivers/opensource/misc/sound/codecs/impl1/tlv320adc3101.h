/*
<:copyright-BRCM:2018:DUAL/GPL:standard

   Copyright (c) 2018 Broadcom 
   All Rights Reserved

Unless you and Broadcom execute a separate written software license
agreement governing use of this software, this software is licensed
to you under the terms of the GNU General Public License version 2
(the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
with the following added to such license:

   As a special exception, the copyright holders of this software give
   you permission to link this software with independent modules, and
   to copy and distribute the resulting executable under terms of your
   choice, provided that you also meet, for each linked independent
   module, the terms and conditions of the license of that module.
   An independent module is a module which is not derived from this
   software.  The special exception does not apply to any modifications
   of the software.

Not withstanding the above, under no circumstances may you combine
this software in any way with any other Broadcom software provided
under a license other than the GPL, without Broadcom's express prior
written consent.

:>
*/

#ifndef _TLV320ADC3101_H
#define _TLV320ADC3101_H

#define ADC3101_RATES SNDRV_PCM_RATE_8000_192000
#define ADC3101_FORMATS (SNDRV_PCM_FMTBIT_S16_LE | \
                         SNDRV_PCM_FMTBIT_S24_LE | \
                         SNDRV_PCM_FMTBIT_S32_LE)

#define STUB_RATES SNDRV_PCM_RATE_8000_192000

#define STUB_FORMATS (SNDRV_PCM_FMTBIT_S16_LE | \
                         SNDRV_PCM_FMTBIT_S24_LE | \
                         SNDRV_PCM_FMTBIT_S32_LE)

#define MCLK_8192000    8192000
#define MCLK_11289600  11289600
#define MCLK_12288000  12288000
#define MCLK_16384000  16384000
#define MCLK_22579200  22579200
#define MCLK_24576000  24576000
#define MCLK_32768000  32768000 
#define MCLK_45158400  45158400
#define MCLK_49152000  49152000

#define ADC3101_PAGE1                    128
/* page 0 */
#define ADC3101_PSEL                     0  /*reg 0*/
#define ADC3101_RESET                    1
#define ADC3101_CLKMUX                   4  /* reg 4 */
#define ADC3101_PLLCLKIN                 0x03
#define ADC3101_CODECCLKIN_MCLK          ( 0x00 << 0 )
#define ADC3101_CODECCLKIN_BCLK          ( 0x01 << 0 )
#define ADC3101_CODECCLKIN_PLL           ( 0x11 << 0 )
#define ADC3101_PLLCLKIN_MCLK            ( 0x00 << 2 )
#define ADC3101_PLLCLKIN_BCLK            ( 0x01 << 2 )
#define ADC3101_PLLCLKIN_LOGIC           ( 0x11 << 2 )
#define ADC3101_PLLPR                    5
#define ADC3101_PLLEN                    ( 0x01 << 7 )
#define ADC3101_PLLJ                     6
#define ADC3101_PLLDMSB                  7
#define ADC3101_PLLDLSB                  8
#define ADC3101_NDAC                     11
#define ADC3101_NDACEN                   ( 0x01 << 7 )
#define ADC3101_MDAC                     12
#define ADC3101_MDACEN                   ( 0x01 << 7 )
#define ADC3101_DOSRMSB                  13
#define ADC3101_DOSRLSB                  14
#define ADC3101_NADC                     18
#define ADC3101_NADCEN                   ( 0x01 << 7 )
#define	ADC3101_MADC                     19
#define ADC3101_MADCEN                   ( 0x01 << 7 )
#define ADC3101_AOSR                     20
#define ADC3101_CLKMUX2                  25
#define ADC3101_CLKOUTM                  26
#define ADC3101_IFACE1                   27
#define ADC3101_AUDIO_INTERFACE_SHIFT    6
#define ADC3101_I2S_MODE                 0x00
#define ADC3101_DSP_MODE                 0x01
#define ADC3101_RIGHT_JUSTIFIED_MODE     0x02
#define ADC3101_LEFT_JUSTIFIED_MODE      0x03
#define ADC3101_AUDIO_DATA_LENGTH_SHIFT  4
#define ADC3101_WORD_LEN_16BITS          0x00
#define ADC3101_WORD_LEN_20BITS          0x01
#define ADC3101_WORD_LEN_24BITS          0x02
#define ADC3101_WORD_LEN_32BITS          0x03
#define ADC3101_BCLK_DIR_SHIFT           3
#define ADC3101_WCLK_DIR_SHIFT           2
#define ADC3101_PLLJ_SHIFT               6
#define ADC3101_DOSRMSB_SHIFT            4
#define ADC3101_DATASLOT_OFFSET          28
#define ADC3101_IFACE2                   29
#define	ADC3101_DACMOD2BCLK              0x01
#define I2S_DEFAULT_BIT_POL              0
#define I2S_DEFAULT_BIT_POL_SHIFT        3
#define I2S_DEFAULT_BIT_POL_MASK         ( 1<<I2S_DEFAULT_BIT_POL_SHIFT )
#define ADC3101_BCLKN                    30
#define ADC3101_BCLKEN                   ( 0x01 << 7 )
#define ADC3101_IFACE4                   31
#define ADC3101_IFACE5                   32
#define ADC3101_IFACE6                   33
#define ADC3101_DOUTCTL                  53
#define ADC3101_DINCTL                   54
#define ADC3101_DACSPB                   60
#define ADC3101_DACSPB_BLK(x)            x
#define AIC32x4_DACSPB_BLK_PRB_P1        1
#define AIC32x4_DACSPB_BLK_PRB_P2        2
#define AIC32x4_DACSPB_BLK_PRB_P8        8
#define ADC3101_ADCSPB                   61
#define ADC3101_SPB_PRB_R1               0x01
#define ADC3101_DACSETUP                 63
#define ADC3101_RDAC2LCHN                ( 0x02 << 2 )
#define ADC3101_LDAC2RCHN                ( 0x02 << 4 )
#define ADC3101_LDAC2LCHN                ( 0x01 << 4 )
#define ADC3101_RDAC2RCHN                ( 0x01 << 2 )
#define ADC3101_DAC_CHAN_MASK            0x3c
#define ADC3101_LDAC_PWRUP               ( 0x01 << 7 )
#define ADC3101_RDAC_PWRUP               ( 0x01 << 6 )
#define ADC3101_DACCHVOL_SFTSTP_DISABLE  ( 0x02 << 0 )

#define ADC3101_DACMUTE                  64
#define ADC3101_MUTEON                   0x0C
#define ADC3101_DAC_AUTO_MUTE_DISABLED   ( 0x00 << 4 )
#define ADC3101_LDAC_CH_NOT_MUTED        ( 0x00 << 3 )
#define ADC3101_RDAC_CH_NOT_MUTED        ( 0x00 << 2 )
#define ADC3101_DAC_MASTER_VOLCTL_INDEP  ( 0x00 << 0 )

#define ADC3101_LDACVOL                  65
#define ADC3101_RDACVOL                  66
#define ADC3101_ADCSETUP                 81
#define ADC3101_LADC_EN                  ( 1 << 7 )
#define ADC3101_RADC_EN                  ( 1 << 6 )
#define ADC3101_DISABLE_SFTSTEP          0x02
#define ADC3101_ADCFGA                   82
#define ADC3101_LADC_MUTED               ( 1<< 7 )
#define ADC3101_RADC_MUTED               ( 1<< 3 )

#define ADC3101_LADCVOL                  83
#define ADC3101_RADCVOL                  84
#define ADC3101_LAGC1                    86
#define ADC3101_LAGC2                    87
#define ADC3101_LAGC3                    88
#define ADC3101_LAGC4                    89
#define ADC3101_LAGC5                    90
#define ADC3101_LAGC6                    91
#define ADC3101_LAGC7                    92
#define ADC3101_RAGC1                    94
#define ADC3101_RAGC2                    95
#define ADC3101_RAGC3                    96
#define ADC3101_RAGC4                    97
#define ADC3101_RAGC5                    98
#define ADC3101_RAGC6                    99
#define ADC3101_RAGC7                    100
#define ADC3101_PWRCFG                   (ADC3101_PAGE1 + 1)
#define ADC3101_AVDDWEAKDISABLE          ( 0x01 << 3 )
#define ADC3101_LDOCTL                   (ADC3101_PAGE1 + 2)
#define ADC3101_AVDD_LDOOUT_172          ( 0x00 << 4 )
#define ADC3101_ABLKPWR_EN               ( 0x00 << 3 )
#define ADC3101_LDOCTLEN                 ( 0x01 << 0 )

#define ADC3101_OUTPWRCTL                (ADC3101_PAGE1 + 9)
#define ADC3101_HPL_PWRUP                ( 1 << 5 )
#define ADC3101_HPR_PWRUP                ( 1 << 4 )
#define ADC3101_CMMODE                   (ADC3101_PAGE1 + 10)
#define ADC3101_LDOIN_18_36              ( 0x01 << 0 )
#define ADC3101_LDOIN2HP                 ( 0x01 << 1 )

#define ADC3101_HPLROUTE                 (ADC3101_PAGE1 + 12)
#define ADC3101_LDAC_PTERM_TO_HPL         ( 1 << 3 )         /* route to HPL */
#define ADC3101_HPRROUTE                 (ADC3101_PAGE1 + 13)
#define ADC3101_RDAC_PTERM_TO_HPL         ( 1 << 3 )         /* route to HPL */
#define ADC3101_LOLROUTE                 (ADC3101_PAGE1 + 14)
#define ADC3101_LORROUTE                 (ADC3101_PAGE1 + 15)
#define	ADC3101_HPLGAIN                  (ADC3101_PAGE1 + 16)
#define ADC3101_HPL_GAIN_SET_3DB         0x03
#define ADC3101_HPL_GAIN_SET_6DB         0x06
#define ADC3101_HPL_GAIN_SET_18DB        0x12
#define	ADC3101_HPRGAIN                  (ADC3101_PAGE1 + 17)
#define ADC3101_HPR_GAIN_SET_3DB         0x03
#define ADC3101_HPR_GAIN_SET_6DB         0x06
#define ADC3101_HPR_GAIN_SET_18DB        0x12
#define	ADC3101_LOLGAIN                  (ADC3101_PAGE1 + 18)
#define	ADC3101_LORGAIN                  (ADC3101_PAGE1 + 19)
#define ADC3101_HEADSTART                (ADC3101_PAGE1 + 20)
#define ADC3101_HEADPHONE_SLOW_PWRUP_X6  ( 0x0A << 2 )
#define ADC3101_HEADPHONE_PWRUP_BYRES6K  ( 1 << 0 )
#define ADC3101_MICBIAS                  (ADC3101_PAGE1 + 51)
#define ADC3101_MICBIAS1_DOWN             ( 0x00 << 5 )
#define ADC3101_MICBIAS1_020              ( 0x01 << 5 )
#define ADC3101_MICBIAS1_025              ( 0x02 << 5 )
#define ADC3101_MICBIAS1_AVDD             ( 0x03 << 5 )
#define ADC3101_MICBIAS2_DOWN             ( 0x00 << 3 )
#define ADC3101_MICBIAS2_020              ( 0x01 << 3 )
#define ADC3101_MICBIAS2_025              ( 0x02 << 3 )
#define ADC3101_MICBIAS2_AVDD             ( 0x03 << 3 )

#define ADC3101_LMICPGAPIN               (ADC3101_PAGE1 + 52)
#define ADC3101_LMICSEL1_0DB             0xfc 

#define ADC3101_LMICPGANIN               (ADC3101_PAGE1 + 54)
#define ADC3101_LMICPGANIN_IN2R_10K        0x10
#define ADC3101_LMICPGANIN_CM1L_10K        0x40
#define ADC3101_RMICPGAPIN               (ADC3101_PAGE1 + 55)
#define ADC3101_RMICSEL1_0DB             0xfc 

#define ADC3101_RMICPGANIN               (ADC3101_PAGE1 + 57)
#define ADC3101_RMICPGANIN_IN1L_10K        0x10
#define ADC3101_RMICPGANIN_CM1R_10K        0x40
#define ADC3101_FLOATINGINPUT            (ADC3101_PAGE1 + 58)
#define ADC3101_LMICPGAVOL               (ADC3101_PAGE1 + 59)
#define ADC3101_LMICPGA_25DB             0x32
#define ADC3101_RMICPGAVOL               (ADC3101_PAGE1 + 60)
#define ADC3101_RMICPGA_25DB             0x32
#define ADC3101_DAC_ACTLFLAG             (ADC3101_PAGE1 + 63)

#define ADC3101_REF_PWRUP_CFG            ( ADC3101_PAGE1 + 123 ) /* p1 reg123 */
#define ADC3101_REF_PWRUP_CFG_MASK       0x07 
#define ADC3101_REF_PWRUP_40MS_WHEN_ABLK_PWRUP   1

#define OFFSET_CALLIBRATION              125 /* p1 reg 125 */
#define MAX_REG_PAGE1                    127 /* p1 reg 127 */

struct adc3101_rate_divs {
   u32 mclk;
   u32 rate;
   u8 p_val;
   u8 pll_j;
   u16 pll_d;
   u16 dosr;
   u8 ndac;
   u8 mdac;
   u8 aosr;
   u8 nadc;
   u8 madc;
   u8 blck_N;
};

struct adc3101_priv {
   struct regmap *regmap;
   u32 sysclk;
   u32 power_cfg;
   u32 micpga_routing;
   bool swapdacs;
   int rstn_gpio;
   struct clk *mclk;
};

static const struct adc3101_rate_divs adc3101_divs[] = {
  /* MCLK/NDAC/MDAC/DOSR=rate */
   /* MCLK          rate p  j  d     dosr ndac mdac aosr nadc madc blck_n*/
   {MCLK_8192000,  8000, 0, 0, 0,    128,  2,   4,  128,  2,  4,   0},
   {MCLK_11289600, 11025,0, 0, 0,    128,  2,   4,  128,  2,  4,   0},
   {MCLK_16384000, 16000,0, 0, 0,    128,  2,   4,  128,  2,  4,   0},
   {MCLK_22579200, 22050,0, 0, 0,    128,  2,   4,  128,  2,  4,   0},
   {MCLK_32768000, 32000,0, 0, 0,    128,  2,   4,  128,  2,  4,   0},
   {MCLK_45158400, 44100,0, 0, 0,    128,  2,   4,  128,  2,  4,   0},
   {MCLK_49152000, 48000,0, 0, 0,    128,  2,   4,  128,  2,  4,   0},
   {MCLK_12288000, 96000,0, 0, 0,     64,  1,   2,   64,  1,  2,   0},
   {MCLK_24576000,192000,0, 0, 0,     64,  1,   2,   64,  1,  2,   0},
   {MCLK_49152000,384000,0, 0, 0,     64,  1,   2,   64,  1,  2,   0}
};

#endif