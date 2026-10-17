// SPDX-License-Identifier: GPL-2.0+
/*
 * Video driver for Marvell Armada XP SoC
 *
 * Initialization of LCD interface and setup of SPLASH screen image
 */

#include <common.h>
#include <dm.h>
#include <part.h>
#include <video.h>
#include <asm/cache.h>
#include <dm/device_compat.h>
#include <linux/delay.h>
#include <linux/mbus.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>
#include <asm/arch/soc.h>

#define MVEBU_LCD_WIN_CONTROL(w)	(0xf000 + ((w) << 4))
#define MVEBU_LCD_WIN_BASE(w)		(0xf004 + ((w) << 4))
#define MVEBU_LCD_WIN_REMAP(w)		(0xf00c + ((w) << 4))

#define MVEBU_LCD_CFG_DMA_START_ADDR_0	0x00cc
#define MVEBU_LCD_CFG_DMA_START_ADDR_1	0x00dc

#define MVEBU_LCD_CFG_GRA_START_ADDR0	0x00f4
#define MVEBU_LCD_CFG_GRA_START_ADDR1	0x00f8
#define MVEBU_LCD_CFG_GRA_PITCH		0x00fc
#define MVEBU_LCD_SPU_GRA_OVSA_HPXL_VLN	0x0100
#define MVEBU_LCD_SPU_GRA_HPXL_VLN	0x0104
#define MVEBU_LCD_SPU_GZM_HPXL_VLN	0x0108
#define MVEBU_LCD_SPU_HWC_OVSA_HPXL_VLN	0x010c
#define MVEBU_LCD_SPU_HWC_HPXL_VLN	0x0110
#define MVEBU_LCD_SPUT_V_H_TOTAL	0x0114
#define MVEBU_LCD_SPU_V_H_ACTIVE	0x0118
#define MVEBU_LCD_SPU_H_PORCH		0x011c
#define MVEBU_LCD_SPU_V_PORCH		0x0120
#define MVEBU_LCD_SPU_BLANKCOLOR	0x0124
#define MVEBU_LCD_SPU_ALPHA_COLOR1	0x0128
#define MVEBU_LCD_SPU_ALPHA_COLOR2	0x012c
#define MVEBU_LCD_SPU_COLORKEY_Y	0x0130
#define MVEBU_LCD_SPU_COLORKEY_U	0x0134
#define MVEBU_LCD_SPU_COLORKEY_V	0x0138
#define MVEBU_LCD_CFG_RDREG4F		0x013c
#define MVEBU_LCD_SPU_SPI_RXDATA	0x0140
#define MVEBU_LCD_SPU_ISA_RXDATA	0x0144
#define MVEBU_LCD_SPU_DBG_ISA		0x0148

#define MVEBU_LCD_SPU_HWC_RDDAT		0x0158
#define MVEBU_LCD_SPU_GAMMA_RDDAT	0x015c
#define MVEBU_LCD_SPU_PALETTE_RDDAT	0x0160
#define MVEBU_LCD_SPU_IOPAD_IN		0x0178
#define MVEBU_LCD_FRAME_COUNT		0x017c
#define MVEBU_LCD_SPU_DMA_CTRL0		0x0190
#define MVEBU_LCD_SPU_DMA_CTRL1		0x0194
#define MVEBU_LCD_SPU_SRAM_CTRL		0x0198
#define MVEBU_LCD_SPU_SRAM_WRDAT	0x019c
#define MVEBU_LCD_SPU_SRAM_PARA0	0x01a0
#define MVEBU_LCD_SPU_SRAM_PARA1	0x01a4
#define MVEBU_LCD_CFG_SCLK_DIV		0x01a8
#define MVEBU_LCD_SPU_CONTRAST		0x01ac
#define MVEBU_LCD_SPU_SATURATION	0x01b0
#define MVEBU_LCD_SPU_CBSH_HUE		0x01b4
#define MVEBU_LCD_SPU_DUMB_CTRL		0x01b8
#define MVEBU_LCD_SPU_IOPAD_CONTROL	0x01bc
#define MVEBU_LCD_SPU_IRQ_ENA_2		0x01d8
#define MVEBU_LCD_SPU_IRQ_ISR_2		0x01dc
#define MVEBU_LCD_SPU_IRQ_ENA		0x01c0
#define MVEBU_LCD_SPU_IRQ_ISR		0x01c4
#define MVEBU_LCD_ADLL_CTRL		0x01c8
#define MVEBU_LCD_CLK_DIS		0x01cc
#define MVEBU_LCD_VGA_HVSYNC_DELAY	0x01d4
#define MVEBU_LCD_CLK_CFG_0		0xf0a0
#define MVEBU_LCD_CLK_CFG_1		0xf0a4
#define MVEBU_LCD_LVDS_CLK_CFG		0xf0ac

#define MVEBU_LVDS_PADS_REG		(MVEBU_SYSTEM_REG_BASE + 0xf0)

enum {
	/* Maximum LCD size we support */
	LCD_MAX_WIDTH		= 640,
	LCD_MAX_HEIGHT		= 480,
	LCD_MAX_LOG2_BPP	= VIDEO_BPP16,
};

struct mvebu_lcd_info {
	u32 fb_base;
	int x_res;
	int y_res;
	int x_fp;
	int y_fp;
	int x_bp;
	int y_bp;
};

struct mvebu_video_priv {
	uintptr_t regs;
};

/* Setup Mbus Bridge Windows for LCD */
static void mvebu_lcd_conf_mbus_registers(uintptr_t regs)
{
	const struct mbus_dram_target_info *dram;
	int i;

	dram = mvebu_mbus_dram_info();

	/* Disable windows, set size/base/remap to 0  */
	for (i = 0; i < 6; i++) {
		writel(0, regs + MVEBU_LCD_WIN_CONTROL(i));
		writel(0, regs + MVEBU_LCD_WIN_BASE(i));
		writel(0, regs + MVEBU_LCD_WIN_REMAP(i));
	}

	/* Write LCD bridge window registers */
	for (i = 0; i < dram->num_cs; i++) {
		const struct mbus_dram_window *cs = dram->cs + i;
		writel(((cs->size - 1) & 0xffff0000) | (cs->mbus_attr << 8) |
		       (dram->mbus_dram_target_id << 4) | 1,
		       regs + MVEBU_LCD_WIN_CONTROL(i));

		writel(cs->base & 0xffff0000, regs + MVEBU_LCD_WIN_BASE(i));
	}
}

/* Initialize LCD registers */
static void mvebu_lcd_register_init(struct mvebu_lcd_info *lcd_info,
				    uintptr_t regs)
{
	/* Local variable for easier handling */
	int x = lcd_info->x_res;
	int y = lcd_info->y_res;
	u32 val;

	/* Setup Mbus Bridge Windows */
	mvebu_lcd_conf_mbus_registers(regs);

	/*
	 * Set LVDS Pads Control Register
	 * wr 0 182F0 FFE00000
	 */
	clrbits_le32(MVEBU_LVDS_PADS_REG, 0x1f << 16);

	/*
	 * Set the LCD_CFG_GRA_START_ADDR0/1 Registers
	 * This is supposed to point to the "physical" memory at memory
	 * end (currently 1GB-64MB but also may be 2GB-64MB).
	 * See also the Window 0 settings!
	 */
	writel(lcd_info->fb_base, regs + MVEBU_LCD_CFG_GRA_START_ADDR0);
	writel(lcd_info->fb_base, regs + MVEBU_LCD_CFG_GRA_START_ADDR1);

	/*
	 * Set the LCD_CFG_GRA_PITCH Register
	 * Bits 31-28: Duty Cycle of Backlight. value/16=High (0x8=Mid Setting)
	 * Bits 25-16: Backlight divider from 32kHz Clock
	 *             (here 16=0x10 for 1kHz)
	 * Bits 15-00: Line Length in Bytes
	 *             240*2 (for RGB1555)=480=0x1E0
	 */
	writel(0x80100000 + 2 * x, regs + MVEBU_LCD_CFG_GRA_PITCH);

	/*
	 * Set the LCD_SPU_GRA_OVSA_HPXL_VLN Register
	 * Bits 31-16: Vertical start of graphical overlay on screen
	 * Bits 15-00: Horizontal start of graphical overlay on screen
	 */
	writel(0x00000000, regs + MVEBU_LCD_SPU_GRA_OVSA_HPXL_VLN);

	/*
	 * Set the LCD_SPU_GRA_HPXL_VLN Register
	 * Bits 31-16: Vertical size of graphical overlay 320=0x140
	 * Bits 15-00: Horizontal size of graphical overlay 240=0xF0
	 * Values before zooming
	 */
	writel((y << 16) | x, regs + MVEBU_LCD_SPU_GRA_HPXL_VLN);

	/*
	 * Set the LCD_SPU_GZM_HPXL_VLN Register
	 * Bits 31-16: Vertical size of graphical overlay 320=0x140
	 * Bits 15-00: Horizontal size of graphical overlay 240=0xF0
	 * Values after zooming
	 */
	writel((y << 16) | x, regs + MVEBU_LCD_SPU_GZM_HPXL_VLN);

	/*
	 * Set the LCD_SPU_HWC_OVSA_HPXL_VLN Register
	 * Bits 31-16: Vertical position of HW Cursor 320=0x140
	 * Bits 15-00: Horizontal position of HW Cursor 240=0xF0
	 */
	writel((y << 16) | x, regs + MVEBU_LCD_SPU_HWC_OVSA_HPXL_VLN);

	/*
	 * Set the LCD_SPU_HWC_OVSA_HPXL_VLN Register
	 * Bits 31-16: Vertical size of HW Cursor
	 * Bits 15-00: Horizontal size of HW Cursor
	 */
	writel(0x00000000, regs + MVEBU_LCD_SPU_HWC_HPXL_VLN);

	/*
	 * Set the LCD_SPU_HWC_OVSA_HPXL_VLN Register
	 * Bits 31-16: Screen total vertical lines:
	 *             VSYNC                = 1
	 *             Vertical Front Porch = 2
	 *             Vertical Lines       = 320
	 *             Vertical Back Porch  = 2
	 *             SUM                  = 325 = 0x0145
	 * Bits 15-00: Screen total horizontal pixels:
	 *             HSYNC                  = 1
	 *             Horizontal Front Porch = 44
	 *             Horizontal Lines       = 240
	 *             Horizontal Back Porch  = 2
	 *             SUM                    = 287 = 0x011F
	 * Note: For the display the backporch is between SYNC and
	 *       the start of the pixels.
	 *       This is not certain for the Marvell (!?)
	 */
	val = ((y + lcd_info->y_fp + lcd_info->y_bp + 1) << 16) |
		(x + lcd_info->x_fp + lcd_info->x_bp + 1);
	writel(val, regs + MVEBU_LCD_SPUT_V_H_TOTAL);

	/*
	 * Set the LCD_SPU_V_H_ACTIVE Register
	 * Bits 31-16: Screen active vertical lines 320=0x140
	 * Bits 15-00: Screen active horizontakl pixels 240=0x00F0
	 */
	writel((y << 16) | x, regs + MVEBU_LCD_SPU_V_H_ACTIVE);

	/*
	 * Set the LCD_SPU_H_PORCH Register
	 * Bits 31-16: Screen horizontal backporch 44=0x2c
	 * Bits 15-00: Screen horizontal frontporch 2=0x02
	 * Note: The terms "front" and "back" for the Marvell seem to be
	 *       exactly opposite to the display.
	 */
	writel((lcd_info->x_fp << 16) | lcd_info->x_bp,
	       regs + MVEBU_LCD_SPU_H_PORCH);

	/*
	 * Set the LCD_SPU_V_PORCH Register
	 * Bits 31-16: Screen vertical backporch  2=0x02
	 * Bits 15-00: Screen vertical frontporch 2=0x02
	 * Note: The terms "front" and "back" for the Marvell seem to be exactly
	 *       opposite to the display.
	 */
	writel((lcd_info->y_fp << 16) | lcd_info->y_bp,
	       regs + MVEBU_LCD_SPU_V_PORCH);

	/*
	 * Set the LCD_SPU_BLANKCOLOR Register
	 * This should be black = 0
	 * For tests this is magenta=00FF00FF
	 */
	writel(0x00FF00FF, regs + MVEBU_LCD_SPU_BLANKCOLOR);

	/*
	 * Registers in the range of 0x0128 to 0x012C are colors for the cursor
	 * Registers in the range of 0x0130 to 0x0138 are colors for video
	 * color keying
	 */

	/*
	 * Set the LCD_SPU_RDREG4F Register
	 * Bits 31-12: Reservd
	 * Bit     11: SRAM Wait
	 * Bit     10: Smart display fast TX (must be 1)
	 * Bit      9: DMA Arbitration Video/Graphics overlay: 0=interleaved
	 * Bit      8: FIFO watermark for DMA: 0=disable
	 * Bits 07-00: Empty 8B FIFO entries to trigger DMA, default=0x80
	 */
	writel(0x00000780, regs + MVEBU_LCD_CFG_RDREG4F);

	/*
	 * Set the LCD_SPU_DMACTRL 0 Register
	 * Bit     31: Disable overlay blending 1=disable
	 * Bit     30: Gamma correction enable, 0=disable
	 * Bit     29: Video Contrast/Saturation/Hue Adjust enable, 0=disable
	 * Bit     28: Color palette enable, 0=disable
	 * Bit     27: DMA AXI Arbiter, 1=default
	 * Bit     26: HW Cursor 1-bit mode
	 * Bit     25: HW Cursor or 1- or 2-bit mode
	 * Bit     24: HW Cursor enabled, 0=disable
	 * Bits 23-20: Graphics Memory Color Format: 0x1=RGB1555
	 * Bits 19-16: Video Memory Color Format:    0x1=RGB1555
	 * Bit     15: Memory Toggle between frame 0 and 1: 0=disable
	 * Bit     14: Graphics horizontal scaling enable: 0=disable
	 * Bit     13: Graphics test mode: 0=disable
	 * Bit     12: Graphics SWAP R and B: 0=disable
	 * Bit     11: Graphics SWAP U and V: 0=disable
	 * Bit     10: Graphics SWAP Y and U/V: 0=disable
	 * Bit     09: Graphic YUV to RGB Conversion: 0=disable
	 * Bit     08: Graphic Transfer: 1=enable
	 * Bit     07: Memory Toggle: 0=disable
	 * Bit     06: Video horizontal scaling enable: 0=disable
	 * Bit     05: Video test mode: 0=disable
	 * Bit     04: Video SWAP R and B: 0=disable
	 * Bit     03: Video SWAP U and V: 0=disable
	 * Bit     02: Video SWAP Y and U/V: 0=disable
	 * Bit     01: Video YUV to RGB Conversion: 0=disable
	 * Bit     00: Video  Transfer: 0=disable
	 */
	writel(0x88111100, regs + MVEBU_LCD_SPU_DMA_CTRL0);

	/*
	 * Set the LCD_SPU_DMA_CTRL1 Register
	 * Bit     31: Manual DMA Trigger = 0
	 * Bits 30-28: DMA Trigger Source: 0x2 VSYNC
	 * Bit     28: VSYNC_INV: 0=Rising Edge, 1=Falling Edge
	 * Bits 26-24: Color Key Mode: 0=disable
	 * Bit     23: Fill low bits: 0=fill with zeroes
	 * Bit     22: Reserved
	 * Bit     21: Gated Clock: 0=disable
	 * Bit     20: Power Save enable: 0=disable
	 * Bits 19-18: Reserved
	 * Bits 17-16: Configure Video/Graphic Path: 0x1: Graphic path alpha.
	 * Bits 15-08: Configure Alpha: 0x00.
	 * Bits 07-00: Reserved.
	 */
	writel(0x20010000, regs + MVEBU_LCD_SPU_DMA_CTRL1);

	/*
	 * Set the LCD_SPU_SRAM_CTRL Register
	 * Reset to default = 0000C000
	 * Bits 15-14: SRAM control: init=0x3, Read=0, Write=2
	 * Bits 11-08: SRAM address ID: 0=gamma_yr, 1=gammy_ug, 2=gamma_vb,
	 *             3=palette, 15=cursor
	 */
	writel(0x0000C000, regs + MVEBU_LCD_SPU_SRAM_CTRL);

	/*
	 * LCD_SPU_SRAM_WRDAT register: 019C
	 * LCD_SPU_SRAM_PARA0 register: 01A0
	 * LCD_SPU_SRAM_PARA1 register: 01A4 - Cursor control/Power settings
	 */
	writel(0x00000000, regs + MVEBU_LCD_SPU_SRAM_PARA1);


	/* Clock settings in the at 01A8 and in the range F0A0 see below */

	/*
	 * Set LCD_SPU_CONTRAST
	 * Bits 31-16: Brightness sign ext. 8-bit value +255 to -255: default=0
	 * Bits 15-00: Contrast sign ext. 8-bit value +255 to -255: default=0
	 */
	writel(0x00000000, regs + MVEBU_LCD_SPU_CONTRAST);

	/*
	 * Set LCD_SPU_SATURATION
	 * Bits 31-16: Multiplier signed 4.12 fixed point value
	 * Bits 15-00: Saturation signed 4.12 fixed point value
	 */
	writel(0x10001000, regs + MVEBU_LCD_SPU_SATURATION);

	/*
	 * Set LCD_SPU_HUE
	 * Bits 31-16: Sine signed 2.14 fixed point value
	 * Bits 15-00: Cosine signed 2.14 fixed point value
	 */
	writel(0x00000000, regs + MVEBU_LCD_SPU_CBSH_HUE);

	/*
	 * Set LCD_SPU_DUMB_CTRL
	 * Bits 31-28: LCD Type: 3=18 bit RGB | 6=24 bit RGB888
	 * Bits 27-12: Reserved
	 * Bit     11: LCD DMA Pipeline Enable: 1=Enable
	 * Bits 10-09: Reserved
	 * Bit      8: LCD GPIO pin (??)
	 * Bit      7: Reverse RGB
	 * Bit      6: Invert composite blank signal DE/EN (??)
	 * Bit      5: Invert composite sync signal
	 * Bit      4: Invert Pixel Valid Enable DE/EN (??)
	 * Bit      3: Invert VSYNC
	 * Bit      2: Invert HSYNC
	 * Bit      1: Invert Pixel Clock
	 * Bit      0: Enable LCD Panel: 1=Enable
	 * Question: Do we have to disable Smart and Dumb LCD
	 * and separately enable LVDS?
	 */
	writel(0x6000080F, regs + MVEBU_LCD_SPU_DUMB_CTRL);

	/*
	 * Set LCD_SPU_IOPAD_CTRL
	 * Bits 31-20: Reserved
	 * Bits 19-18: Vertical Interpolation: 0=Disable
	 * Bits 17-16: Reserved
	 * Bit     15: Graphics Vertical Mirror enable: 0=disable
	 * Bit     14: Reserved
	 * Bit     13: Video Vertical Mirror enable: 0=disable
	 * Bit     12: Reserved
	 * Bit     11: Command Vertical Mirror enable: 0=disable
	 * Bit     10: Reserved
	 * Bits 09-08: YUV to RGB Color space conversion: 0 (Not used)
	 * Bits 07-04: AXI Bus Master: 0x4: no crossing of 4k boundary,
	 *             128 Bytes burst
	 * Bits 03-00: LCD pins: ??? 0=24-bit Dump panel ??
	 */
	writel(0x000000C0, regs + MVEBU_LCD_SPU_IOPAD_CONTROL);

	/*
	 * Set SUP_IRQ_ENA_2: Disable all interrupts
	 */
	writel(0x00000000, regs + MVEBU_LCD_SPU_IRQ_ENA_2);

	/*
	 * Set SUP_IRQ_ENA: Disable all interrupts.
	 */
	writel(0x00000000, regs + MVEBU_LCD_SPU_IRQ_ENA);

	/*
	 * Set up ADDL Control Register
	 * Bits 31-29: 0x0 = Fastest Delay Line (default)
	 *             0x3 = Slowest Delay Line (default)
	 * Bit     28: Calibration done status.
	 * Bit     27: Reserved
	 * Bit     26: Set Pixel Clock to ADDL output
	 * Bit     25: Reduce CAL Enable
	 * Bits 24-22: Manual calibration value.
	 * Bit     21: Manual calibration enable.
	 * Bit     20: Restart Auto Cal
	 * Bits 19-16: Calibration Threshold voltage, default= 0x2
	 * Bite 15-14: Reserved
	 * Bits 13-11: Divisor for ADDL Clock: 0x1=/2, 0x3=/8, 0x5=/16
	 * Bit     10: Power Down ADDL module, default = 1!
	 * Bits 09-08: Test point configuration: 0x2=Bias, 0x3=High-z
	 * Bit     07: Reset ADDL
	 * Bit     06: Invert ADLL Clock
	 * Bits 05-00: Delay taps, 0x3F=Half Cycle, 0x00=No delay
	 * Note: ADLL is used for a VGA interface with DAC - not used here
	 */
	writel(0x00000000, regs + MVEBU_LCD_ADLL_CTRL);

	/*
	 * Set the LCD_CLK_DIS Register:
	 * Bits 3 and 4 must be 1
	 */
	writel(0x00000018, regs + MVEBU_LCD_CLK_DIS);

	/*
	 * Set the LCD_VGA_HSYNC/VSYNC Delay Register:
	 * Bits 03-00: Sets the delay for the HSYNC and VSYNC signals
	 */
	writel(0x00000000, regs + MVEBU_LCD_VGA_HVSYNC_DELAY);

	/*
	 * Clock registers
	 * See page 475 in the functional spec.
	 */

	/* Step 1 and 2: Disable the PLL */

	/*
	 * Disable PLL, see "LCD Clock Configuration 1 Register" below
	 */
	writel(0x8FF40007, regs + MVEBU_LCD_CLK_CFG_1);

	/*
	 * Powerdown, see "LCD Clock Configuration 0 Register" below
	 */
	writel(0x94000174, regs + MVEBU_LCD_CLK_CFG_0);

	/*
	 * Set the LCD_CFG_SCLK_DIV Register
	 * This is set fix to 0x40000001 for the LVDS output:
	 * Bits 31-30: SCLCK Source: 0=AXIBus, 1=AHBus, 2=PLLDivider0
	 * Bits 15-01: Clock Divider: Bypass for LVDS=0x0001
	 * See page 475 in section 28.5.
	 */
	writel(0x80000001, regs + MVEBU_LCD_CFG_SCLK_DIV);

	/*
	 * Set the LCD Clock Configuration 0 Register:
	 * Bit     31: Powerdown: 0=Power up
	 * Bits 30-29: Reserved
	 * Bits 28-26: PLL_KDIV: This encodes K
	 *             K=16 => 0x5
	 * Bits 25-17: PLL_MDIV: This is M-1:
	 *             M=1 => 0x0
	 * Bits 16-13: VCO band: 0x1 for 700-920MHz
	 * Bits 12-04: PLL_NDIV: This is N-1 and corresponds to R1_CTRL!
	 *             N=28=0x1C => 0x1B
	 * Bits 03-00: R1_CTRL (for N=28 => 0x4)
	 */
	writel(0x940021B4, regs + MVEBU_LCD_CLK_CFG_0);

	/*
	 * Set the LCD Clock Configuration 1 Register:
	 * Bits 31-19: Reserved
	 * Bit     18: Select PLL: Core PLL, 1=Dedicated PPL
	 * Bit     17: Clock Output Enable: 0=disable, 1=enable
	 * Bit     16: Select RefClk: 0=RefClk (25MHz), 1=External
	 * Bit     15: Half-Div, Device Clock by DIV+0.5*Half-Dev
	 * Bits 14-13: Reserved
	 * Bits 12-00: PLL Full Divider [Note: Assumed to be the Post-Divider
	 *             M' for LVDS=7!]
	 */
	writel(0x8FF40007, regs + MVEBU_LCD_CLK_CFG_1);

	/*
	 * Set the LVDS Clock Configuration Register:
	 * Bit     31: Clock Gating for the input clock to the LVDS
	 * Bit     30: LVDS Serializer enable: 1=Enabled
	 * Bits 29-11: Reserved
	 * Bit  11-08: LVDS Clock delay: 0x02 (default): by 2 pixel clock/7
	 * Bits 07-02: Reserved
	 * Bit     01: 24bbp Option: 0=Option_1,1=Option2
	 * Bit     00: 1=24bbp Panel: 0=18bpp Panel
	 * Note: Bits 0 and must be verified with the help of the
	 *       Interface/display
	 */
	writel(0xC0000201, regs + MVEBU_LCD_LVDS_CLK_CFG);

	/*
	 * Power up PLL (Clock Config 0)
	 */
	writel(0x140021B4, regs + MVEBU_LCD_CLK_CFG_0);

	/* wait 10 ms */
	mdelay(10);

	/*
	 * Enable PLL (Clock Config 1)
	 */
	writel(0x8FF60007, regs + MVEBU_LCD_CLK_CFG_1);
}

static int mvebu_video_probe(struct udevice *dev)
{
	struct video_uc_plat *plat = dev_get_uclass_plat(dev);
	struct video_priv *uc_priv = dev_get_uclass_priv(dev);
	struct mvebu_video_priv *priv = dev_get_priv(dev);
	struct mvebu_lcd_info lcd_info;
	struct display_timing timings;
	u32 fb_start, fb_end;
	int ret;

	priv->regs = dev_read_addr(dev);
	if (priv->regs == FDT_ADDR_T_NONE) {
		dev_err(dev, "failed to get LCD address\n");
		return -ENXIO;
	}

	ret = ofnode_decode_display_timing(dev_ofnode(dev), 0, &timings);
	if (ret) {
		dev_err(dev, "failed to get any display timings\n");
		return -EINVAL;
	}

	/* Use DT timing (resolution) in internal info struct */
	lcd_info.fb_base = plat->base;
	lcd_info.x_res = timings.hactive.typ;
	lcd_info.x_fp = timings.hfront_porch.typ;
	lcd_info.x_bp = timings.hback_porch.typ;
	lcd_info.y_res = timings.vactive.typ;
	lcd_info.y_fp = timings.vfront_porch.typ;
	lcd_info.y_bp = timings.vback_porch.typ;

	/* Initialize the LCD controller */
	mvebu_lcd_register_init(&lcd_info, priv->regs);

	/* Enable dcache for the frame buffer */
	fb_start = plat->base & ~(MMU_SECTION_SIZE - 1);
	fb_end = plat->base + plat->size;
	fb_end = ALIGN(fb_end, 1 << MMU_SECTION_SHIFT);
	mmu_set_region_dcache_behaviour(fb_start, fb_end - fb_start,
					DCACHE_WRITEBACK);
	video_set_flush_dcache(dev, true);

	uc_priv->xsize = lcd_info.x_res;
	uc_priv->ysize = lcd_info.y_res;
	uc_priv->bpix = VIDEO_BPP16;	/* Uses RGB555 format */

	return 0;
}

static int mvebu_video_bind(struct udevice *dev)
{
	struct video_uc_plat *plat = dev_get_uclass_plat(dev);

	plat->size = LCD_MAX_WIDTH * LCD_MAX_HEIGHT *
		(1 << LCD_MAX_LOG2_BPP) / 8;

	return 0;
}

static const struct udevice_id mvebu_video_ids[] = {
	{ .compatible = "marvell,armada-xp-lcd" },
	{ }
};

U_BOOT_DRIVER(mvebu_video) = {
	.name	= "mvebu_video",
	.id	= UCLASS_VIDEO,
	.of_match = mvebu_video_ids,
	.bind	= mvebu_video_bind,
	.probe	= mvebu_video_probe,
	.priv_auto	= sizeof(struct mvebu_video_priv),
};
