/*
<:copyright-BRCM:2021:DUAL/GPL:standard

   Copyright (c) 2021 Broadcom 
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
/***************************************************************************/
#ifndef _PCM_REGS_H
#define _PCM_REGS_H

#include <dsphal_chip.h>

struct pcm_ctrl_regs
{
    uint32 pcm_ctrl;                            // 00 offset from PCM_BASE
#define  PCM_ENABLE              0x80000000     // PCM block master enable
#define  PCM_ENABLE_SHIFT        31
#define  PCM_SLAVE_SEL           0x40000000     // PCM TDM slave mode select (1 - TDM slave, 0 - TDM master)
#define  PCM_SLAVE_SEL_SHIFT     30
#define  PCM_CLOCK_INV           0x20000000     // PCM SCLK invert select (1 - invert, 0 - normal)
#define  PCM_CLOCK_INV_SHIFT     29
#define  PCM_FS_INVERT           0x10000000     // PCM FS invert select (1 - invert, 0 - normal)
#define  PCM_FS_INVERT_SHIFT     28
#define  PCM_FS_FREQ_16_8        0x08000000     // PCM FS 16/8 Khz select (1 - 16Khz, 0 - 8Khz)
#define  PCM_FS_FREQ_16_8_SHIFT  27
#define  PCM_FS_LONG             0x04000000     // PCM FS long/short select (1 - long FS, 0 - short FS)
#define  PCM_FS_LONG_SHIFT       26
#define  PCM_FS_TRIG             0x02000000     // PCM FS trigger (1 - falling edge, 0 - rising edge trigger)
#define  PCM_FS_TRIG_SHIFT       25
#define  PCM_DATA_OFF            0x01000000     // PCM data offset from FS (1 - one clock from FS, 0 - no offset)
#define  PCM_DATA_OFF_SHIFT      24
#define  PCM_DATA_16_8           0x00800000     // PCM data word length (1 - 16 bits, 0 - 8 bits)
#define  PCM_DATA_16_8_SHIFT     23
#define  PCM_CLOCK_SEL           0x00700000     // PCM SCLK freq select
#define  PCM_CLOCK_SEL_SHIFT     20
                                                  // 000 - 8192 Khz
                                                  // 001 - 4096 Khz
                                                  // 010 - 2048 Khz
                                                  // 011 - 1024 Khz
                                                  // 100 - 512 Khz
                                                  // 101 - 256 Khz
                                                  // 110 - 128 Khz
                                                  // 111 - reserved
#define  PCM_LSB_FIRST           0x00040000     // PCM shift direction (1 - LSBit first, 0 - MSBit first)
#define  PCM_LSB_FIRST_SHIFT     18
#define  PCM_LOOPBACK            0x00020000     // PCM diagnostic loobback enable
#define  PCM_LOOPBACK_SHIFT      17
#define  PCM_EXTCLK_SEL          0x00010000     // PCM external timing clock select -- Maybe removed in 63138
#define  PCM_EXTCLK_SEL_SHIFT    16
#define  PCM_NTR_ENABLE          0x00008000     // PCM NTR counter enable -- Nayve removed in 63138
#define  PCM_NTR_ENABLE_SHIFT    15
#define  PCM_BITS_PER_FRAME_1024 0x00000400     // 1024 - Max
#define  PCM_BITS_PER_FRAME_256  0x00000100     // 256
#define  PCM_BITS_PER_FRAME_8    0x00000008     // 8    - Min
#define  PCM_AP_SEL              0x00000001     // 1 - Connect pcm to pads, 0 - Connects audio test interface

    uint32 pcm_chan_ctrl;                       // 04
#define  PCM_TX0_EN              0x00000001     // PCM transmit channel 0 enable
#define  PCM_TX1_EN              0x00000002     // PCM transmit channel 1 enable
#define  PCM_TX2_EN              0x00000004     // PCM transmit channel 2 enable
#define  PCM_TX3_EN              0x00000008     // PCM transmit channel 3 enable
#define  PCM_TX4_EN              0x00000010     // PCM transmit channel 4 enable
#define  PCM_TX5_EN              0x00000020     // PCM transmit channel 5 enable
#define  PCM_TX6_EN              0x00000040     // PCM transmit channel 6 enable
#define  PCM_TX7_EN              0x00000080     // PCM transmit channel 7 enable
#define  PCM_RX0_EN              0x00000100     // PCM receive channel 0 enable
#define  PCM_RX1_EN              0x00000200     // PCM receive channel 1 enable
#define  PCM_RX2_EN              0x00000400     // PCM receive channel 2 enable
#define  PCM_RX3_EN              0x00000800     // PCM receive channel 3 enable
#define  PCM_RX4_EN              0x00001000     // PCM receive channel 4 enable
#define  PCM_RX5_EN              0x00002000     // PCM receive channel 5 enable
#define  PCM_RX6_EN              0x00004000     // PCM receive channel 6 enable
#define  PCM_RX7_EN              0x00008000     // PCM receive channel 7 enable
#define  PCM_RX_PACKET_SIZE      0x00ff0000     // PCM Rx DMA quasi-packet size
#define  PCM_RX_PACKET_SIZE_SHIFT  16

    uint32 pcm_int_pending;                     // 08
    uint32 pcm_int_mask;                        // 0c
#define  PCM_TX_UNDERFLOW        0x00000001     // PCM DMA receive overflow
#define  PCM_RX_OVERFLOW         0x00000002     // PCM DMA transmit underflow
#define  PCM_TDM_FRAME           0x00000004     // PCM frame boundary
#define  PCM_RX_IRQ              0x00000008     // IUDMA interrupts
#define  PCM_TX_IRQ              0x00000010

#if defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148) || defined(CONFIG_BCM_PON) || defined(CONFIG_BCM94908) || \
    defined(CONFIG_BCM963158) || defined(CONFIG_BCM96846) || defined(CONFIG_BCM963178) || defined(CONFIG_BCM947622) || \
    defined(CONFIG_BCM96878)  || defined(CONFIG_BCM963146) || defined(CONFIG_BCM94912) || defined(CONFIG_BCM96756) || \
    defined(CONFIG_BCM96855)  || defined(CONFIG_BCM96888)  || defined(CONFIG_BCM968880) || defined(CONFIG_BCM96837) || defined(CONFIG_BCM96765)/* IMPL1 */
    uint32        reg_pcm_clk_cntl_0;             // (0x210) PCM Clock Control 0 (NCO_FCW_MISC)
    uint32        reg_pcm_clk_cntl_1;             // (0x214) PCM Clock Control 1 (NCO_SCALE)
    uint32        reg_pcm_clk_cntl_2;             // (0x218) PCM Clock Control 2
#define   PCM_NCO_SHIFT          0x0000000f
#define   PCM_NCO_MUX_CNTL       0x00000030
#define   PCM_NCO_LOAD_MISC      0x00000040
#define   PCM_NCO_SOFT_INIT      0x00000080

    uint32        pcm_apm_fcw_readback;            // FCW after scaling
    uint32        pcm_zds_intf;                    // ZSI Config register ( New in 63138 )
    uint32        pcm_msif_intf;                   // ISI Config register ( New in 63138 )
    uint32        pcm_ntr_div_clk_ctrl;            // NTR feedback clock divider ( New in 63138 )
#if defined(CONFIG_BCM96888) || defined(CONFIG_BCM968880) || defined(CONFIG_BCM96837) || defined(CONFIG_BCM96765)
    uint32        pcm_pcm_misc; /* 2c, fix the time slot shift issue */
#define   PCM_TIME_SLOT_SHIFT_FIX_2_EN          0x00000002 //Reset value is 0x1
#define   PCM_TIME_SLOT_SHIFT_FIX_1_EN          0x00000001 //Reset value is 0x0
    uint32        pcm_tx_idle_pat_0;
    uint32        pcm_tx_idle_pat_1;
    uint32        pcm_tx_rx_fifo_thresh;
    uint32        pcm_tx_rx_lostFrameCnt;      /* 3c*/
#else
    uint32 ignore[5];
#endif // !(defined(CONFIG_BCM94912) || defined(CONFIG_BCM963146) || defined(CONFIG_BCM96756))

#if defined(CONFIG_BCM96858) || defined(CONFIG_BCM96846) || defined(CONFIG_BCM963158) \
    || defined(CONFIG_BCM96856) || defined(CONFIG_BCM963178) || defined(CONFIG_BCM947622) \
    || defined(CONFIG_BCM96878) || defined(CONFIG_BCM963146) || defined(CONFIG_BCM94912) \
    || defined(CONFIG_BCM96756) || defined(CONFIG_BCM96855)  || defined(CONFIG_BCM96888) \
    || defined(CONFIG_BCM968880)|| defined(CONFIG_BCM96837)  || defined(CONFIG_BCM96765)
  #define PCM_MAX_TIMESLOT_REGS   4
#else
  #define PCM_MAX_TIMESLOT_REGS   16
#endif
    uint32 pcm_slot_alloc_tbl[PCM_MAX_TIMESLOT_REGS];
#define  PCM_TS_VALID            0x8            // valid marker for TS alloc ram entry

#else /* IMPL2 */
    uint32 pcm_pll_ctrl1;                       // 10
#define  PCM_PLL_PWRDN           0x80000000     // PLL PWRDN
#define  PCM_PLL_PWRDN_CH1       0x40000000     // PLL CH PWRDN
#define  PCM_PLL_REFCMP_PWRDN    0x20000000     // PLL REFCMP PWRDN
#define  PCM_CLK16_RESET         0x10000000     // 16.382 MHz PCM interface circuitry reset. 
#define  PCM_PLL_ARESET          0x08000000     // PLL Analog Reset
#define  PCM_PLL_DRESET          0x04000000     // PLL Digital Reset

    uint32 pcm_pll_ctrl2;                       // 14
    uint32 pcm_pll_ctrl3;                       // 18
    uint32 pcm_pll_ctrl4;                       // 1c

    uint32 pcm_pll_stat;                        // 20
#define  PCM_PLL_LOCK            0x00000001     // Asserted when PLL is locked to programmed frequency

    uint32 pcm_ntr_counter;                     // 24

    uint32 unused[6];

#define  PCM_MAX_TIMESLOT_REGS   16             // Number of PCM time slot registers in the table.
                                                // Each register provides the settings for 8 timeslots (4 bits per timeslot)
    uint32 pcm_slot_alloc_tbl[PCM_MAX_TIMESLOT_REGS];
#define  PCM_TS_VALID            0x8            // valid marker for TS alloc ram entry

    uint32 pcm_pll_ch2_ctrl;                    // 80
    uint32 pcm_msif_intf;                       // 84
    uint32 pcm_pll_ch3_ctrl;                    // 88
#endif /* IMPL */
};
#define PCM ((volatile struct pcm_ctrl_regs * const) (((char*)apm_reg)+APM_PCM_CTRL))
#endif /* _PCM_REGS_H */
