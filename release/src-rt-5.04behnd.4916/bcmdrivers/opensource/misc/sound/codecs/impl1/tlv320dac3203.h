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

#ifndef _TLV320DAC3203_H
#define _TLV320DAC3203_H

#define DAC3203_RATES    SNDRV_PCM_RATE_8000_192000
#define DAC3203_FORMATS  (SNDRV_PCM_FMTBIT_S16_LE | \
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

#define DAC3203_PAGE1                    128
/* page 0 */
#define DAC3203_PSEL                     0  /*reg 0*/
#define DAC3203_RESET                    1
#define DAC3203_CLKMUX                   4
#define DAC3203_PLLCLKIN                 0x03
#define DAC3203_PLLPR                    5
#define DAC3203_PLLEN                    ( 0x01 << 7 )
#define DAC3203_PLLJ                     6
#define DAC3203_PLLDMSB                  7
#define DAC3203_PLLDLSB                  8
#define DAC3203_NDAC                     11
#define DAC3203_NDACEN                   ( 0x01 << 7 )
#define DAC3203_MDAC                     12
#define DAC3203_MDACEN                   ( 0x01 << 7 )
#define DAC3203_DOSRMSB                  13
#define DAC3203_DOSRLSB                  14
#define DAC3203_NADC                     18
#define DAC3203_NADCEN                   ( 0x01 << 7 )
#define	DAC3203_MADC                     19
#define DAC3203_MADCEN                   ( 0x01 << 7 )
#define DAC3203_AOSR                     20
#define DAC3203_CLKMUX2                  25
#define DAC3203_CLKOUTM                  26
#define DAC3203_IFACE1                   27
#define DAC3203_AUDIO_INTERFACE_SHIFT    6
#define DAC3203_I2S_MODE                 0x00
#define DAC3203_DSP_MODE                 0x01
#define DAC3203_RIGHT_JUSTIFIED_MODE     0x02
#define DAC3203_LEFT_JUSTIFIED_MODE      0x03
#define DAC3203_AUDIO_DATA_LENGTH_SHIFT  4
#define DAC3203_WORD_LEN_16BITS          0x00
#define DAC3203_WORD_LEN_20BITS          0x01
#define DAC3203_WORD_LEN_24BITS          0x02
#define DAC3203_WORD_LEN_32BITS          0x03
#define DAC3203_BCLK_DIR_SHIFT           3
#define DAC3203_WCLK_DIR_SHIFT           2
#define DAC3203_PLLJ_SHIFT               6
#define DAC3203_DOSRMSB_SHIFT            4
#define DAC3203_IFACE2                   28
#define DAC3203_IFACE3                   29
#define DAC3203_DACMOD2BCLK              0x01
#define I2S_DEFAULT_BIT_POL              0
#define I2S_DEFAULT_BIT_POL_SHIFT        3
#define I2S_DEFAULT_BIT_POL_MASK         ( 1<<I2S_DEFAULT_BIT_POL_SHIFT )
#define DAC3203_BCLKN                    30
#define DAC3203_BCLKEN                   ( 0x01 << 7 )
#define DAC3203_IFACE4                   31
#define DAC3203_IFACE5                   32
#define DAC3203_IFACE6                   33
#define DAC3203_DOUTCTL                  53
#define DAC3203_DINCTL                   54
#define DAC3203_DACSPB                   60
#define DAC3203_DACSPB_BLK(x)            x
#define DAC3203_DACSPB_BLK_PRB_P1        1
#define DAC3203_DACSPB_BLK_PRB_P2        2
#define DAC3203_DACSPB_BLK_PRB_P8        8
#define DAC3203_ADCSPB                   61
#define DAC3203_DACSETUP                 63
#define DAC3203_RDAC2LCHN                ( 0x02 << 2 )
#define DAC3203_LDAC2RCHN                ( 0x02 << 4 )
#define DAC3203_LDAC2LCHN                ( 0x01 << 4 )
#define DAC3203_RDAC2RCHN                ( 0x01 << 2 )
#define DAC3203_DAC_CHAN_MASK            0x3c
#define DAC3203_LDAC_PWRUP               ( 0x01 << 7 )
#define DAC3203_RDAC_PWRUP               ( 0x01 << 6 )
#define DAC3203_DACCHVOL_SFTSTP_DISABLE  ( 0x02 << 0 )

#define DAC3203_DACMUTE                  64
#define DAC3203_MUTEON                   0x0C
#define DAC3203_DAC_AUTO_MUTE_DISABLED   ( 0x00 << 4 )
#define DAC3203_LDAC_CH_NOT_MUTED        ( 0x00 << 3 )
#define DAC3203_RDAC_CH_NOT_MUTED        ( 0x00 << 2 )
#define DAC3203_DAC_MASTER_VOLCTL_INDEP  ( 0x00 << 0 )

#define DAC3203_LDACVOL                  65
#define DAC3203_RDACVOL                  66
#define DAC3203_ADCSETUP                 81
#define DAC3203_LADC_EN                  ( 1 << 7 )
#define DAC3203_RADC_EN                  ( 1 << 6 )
#define	DAC3203_ADCFGA                   82
#define DAC3203_LADCVOL                  83
#define DAC3203_RADCVOL                  84
#define DAC3203_PWRCFG                   (DAC3203_PAGE1 + 1)
#define DAC3203_AVDDWEAKDISABLE          ( 0x01 << 3 )
#define DAC3203_LDOCTL                   (DAC3203_PAGE1 + 2)
#define DAC3203_AVDD_LDOOUT_172          ( 0x00 << 4 )
#define DAC3203_ABLKPWR_DISABLE          ( 1 << 3 )
#define DAC3203_AVDD_LDO_PWRUP           ( 1 << 0 )


#define DAC3203_OUTPWRCTL                (DAC3203_PAGE1 + 9)
#define DAC3203_HPL_PWRUP                ( 1 << 5 )
#define DAC3203_HPR_PWRUP                ( 1 << 4 )
#define DAC3203_CMMODE                   (DAC3203_PAGE1 + 10)
#define DAC3203_LDOIN_18_36              ( 0x01 << 0 )
#define DAC3203_LDOIN2HP                 ( 0x01 << 1 )

#define DAC3203_HPLROUTE                 (DAC3203_PAGE1 + 12)
#define DAC3203_LDAC_PTERM_TO_HPL         ( 1 << 3 )         /* route to HPL */
#define DAC3203_HPRROUTE                 (DAC3203_PAGE1 + 13)
#define DAC3203_RDAC_PTERM_TO_HPL         ( 1 << 3 )         /* route to HPL */
#define DAC3203_LOLROUTE                 (DAC3203_PAGE1 + 14)
#define DAC3203_LORROUTE                 (DAC3203_PAGE1 + 15)
#define	DAC3203_HPLGAIN                  (DAC3203_PAGE1 + 16)
#define DAC3203_HPL_GAIN_SET_3DB         0x03
#define DAC3203_HPL_GAIN_SET_6DB         0x06
#define DAC3203_HPL_GAIN_SET_18DB        0x12
#define	DAC3203_HPRGAIN                  (DAC3203_PAGE1 + 17)
#define DAC3203_HPR_GAIN_SET_3DB         0x03
#define DAC3203_HPR_GAIN_SET_6DB         0x06
#define DAC3203_HPR_GAIN_SET_18DB        0x12
#define DAC3203_HEADSTART                (DAC3203_PAGE1 + 20)
#define DAC3203_HEADPHONE_SLOW_PWRUP_X6  ( 0x0A << 2 )
#define DAC3203_HEADPHONE_PWRUP_BYRES6K  ( 1 << 0 )
#define DAC3203_MICBIAS                  (DAC3203_PAGE1 + 51)
#define DAC3203_MICBIAS_LDOIN            0x08
#define DAC3203_MICBIAS_2075V            0x60
#define DAC3203_LMICPGAPIN               (DAC3203_PAGE1 + 52)
#define DAC3203_LMICPGANIN               (DAC3203_PAGE1 + 54)
#define DAC3203_LMICPGANIN_IN2R_10K      0x10
#define DAC3203_LMICPGANIN_CM1L_10K      0x40
#define DAC3203_RMICPGAPIN               (DAC3203_PAGE1 + 55)
#define DAC3203_RMICPGANIN               (DAC3203_PAGE1 + 57)
#define DAC3203_RMICPGANIN_IN1L_10K      0x10
#define DAC3203_RMICPGANIN_CM1R_10K      0x40
#define DAC3203_FLOATINGINPUT            (DAC3203_PAGE1 + 58)
#define DAC3203_DAC_ACTLFLAG             (DAC3203_PAGE1 + 63)

#define DAC3203_REF_PWRUP_CFG            ( DAC3203_PAGE1 + 123 ) /* p1 reg123 */
#define DAC3203_REF_PWRUP_CFG_MASK       0x07 
#define DAC3203_REF_PWRUP_40MS_WHEN_ABLK_PWRUP   1

#define OFFSET_CALLIBRATION              125 /* p1 reg 125 */
#define MAX_REG_PAGE1                    127 /* p1 reg 127 */

struct dac3203_pdata {
   u32 power_cfg;
   u32 micpga_routing;
   bool swapdacs;
   int rstn_gpio;
};

struct dac3203_rate_divs {
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

struct dac3203_priv {
   struct regmap *regmap;
   u32 sysclk;
   u32 power_cfg;
   u32 micpga_routing;
   bool swapdacs;
   int rstn_gpio;
   struct clk *mclk;
};

static const struct dac3203_rate_divs dac3203_divs[] = {
  /* MCLK/NDAC/MDAC/DOSR=rate */
   /* MCLK        rate   p  j  d     dosr ndac mdac aosr */
   {MCLK_8192000,  8000, 0, 0, 0,    128,  2,   4,  0,   0,   0},
   {MCLK_11289600, 11025,0, 0, 0,    128,  2,   4,  0,   0,   0},
   {MCLK_16384000, 16000,0, 0, 0,    128,  2,   4,  0,   0,   0},
   {MCLK_22579200, 22050,0, 0, 0,    128,  2,   4,  0,   0,   0},
   {MCLK_32768000, 32000,0, 0, 0,    128,  2,   4,  0,   0,   0},
   {MCLK_45158400, 44100,0, 0, 0,    128,  2,   4,  0,   0,   0},
   {MCLK_49152000, 48000,0, 0, 0,    128,  2,   4,  0,   0,   0},
   {MCLK_12288000, 96000,0, 0, 0,     64,  1,   2,  0,   0,   0},
   {MCLK_24576000,192000,0, 0, 0,     64,  1,   2,  0,   0,   0},
   {MCLK_49152000,384000,0, 0, 0,     64,  1,   2,  0,   0,   0}
};

#endif