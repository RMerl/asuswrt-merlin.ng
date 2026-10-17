/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2012 Samsung Electronics
 *
 * Author: InKi Dae <inki.dae@samsung.com>
 * Author: Donghwa Lee <dh09.lee@samsung.com>
 */

#ifndef _DSIM_H
#define _DSIM_H

#include <linux/list.h>
#include <linux/fb.h>

#define PANEL_NAME_SIZE		(32)

enum mipi_dsim_interface_type {
	DSIM_COMMAND,
	DSIM_VIDEO
};

enum mipi_dsim_virtual_ch_no {
	DSIM_VIRTUAL_CH_0,
	DSIM_VIRTUAL_CH_1,
	DSIM_VIRTUAL_CH_2,
	DSIM_VIRTUAL_CH_3
};

enum mipi_dsim_burst_mode_type {
	DSIM_NON_BURST_SYNC_EVENT,
	DSIM_BURST_SYNC_EVENT,
	DSIM_NON_BURST_SYNC_PULSE,
	DSIM_BURST,
	DSIM_NON_VIDEO_MODE
};

enum mipi_dsim_no_of_data_lane {
	DSIM_DATA_LANE_1,
	DSIM_DATA_LANE_2,
	DSIM_DATA_LANE_3,
	DSIM_DATA_LANE_4
};

enum mipi_dsim_byte_clk_src {
	DSIM_PLL_OUT_DIV8,
	DSIM_EXT_CLK_DIV8,
	DSIM_EXT_CLK_BYPASS
};

enum mipi_dsim_pixel_format {
	DSIM_CMD_3BPP,
	DSIM_CMD_8BPP,
	DSIM_CMD_12BPP,
	DSIM_CMD_16BPP,
	DSIM_VID_16BPP_565,
	DSIM_VID_18BPP_666PACKED,
	DSIM_18BPP_666LOOSELYPACKED,
	DSIM_24BPP_888
};

/* MIPI DSI Processor-to-Peripheral transaction types */
enum {
	MIPI_DSI_V_SYNC_START				= 0x01,
	MIPI_DSI_V_SYNC_END				= 0x11,
	MIPI_DSI_H_SYNC_START				= 0x21,
	MIPI_DSI_H_SYNC_END				= 0x31,

	MIPI_DSI_COLOR_MODE_OFF				= 0x02,
	MIPI_DSI_COLOR_MODE_ON				= 0x12,
	MIPI_DSI_SHUTDOWN_PERIPHERAL			= 0x22,
	MIPI_DSI_TURN_ON_PERIPHERAL			= 0x32,

	MIPI_DSI_GENERIC_SHORT_WRITE_0_PARAM		= 0x03,
	MIPI_DSI_GENERIC_SHORT_WRITE_1_PARAM		= 0x13,
	MIPI_DSI_GENERIC_SHORT_WRITE_2_PARAM		= 0x23,

	MIPI_DSI_GENERIC_READ_REQUEST_0_PARAM		= 0x04,
	MIPI_DSI_GENERIC_READ_REQUEST_1_PARAM		= 0x14,
	MIPI_DSI_GENERIC_READ_REQUEST_2_PARAM		= 0x24,

	MIPI_DSI_DCS_SHORT_WRITE			= 0x05,
	MIPI_DSI_DCS_SHORT_WRITE_PARAM			= 0x15,

	MIPI_DSI_DCS_READ				= 0x06,

	MIPI_DSI_SET_MAXIMUM_RETURN_PACKET_SIZE		= 0x37,

	MIPI_DSI_END_OF_TRANSMISSION			= 0x08,

	MIPI_DSI_NULL_PACKET				= 0x09,
	MIPI_DSI_BLANKING_PACKET			= 0x19,
	MIPI_DSI_GENERIC_LONG_WRITE			= 0x29,
	MIPI_DSI_DCS_LONG_WRITE				= 0x39,

	MIPI_DSI_LOOSELY_PACKED_PIXEL_STREAM_YCBCR20	= 0x0c,
	MIPI_DSI_PACKED_PIXEL_STREAM_YCBCR24		= 0x1c,
	MIPI_DSI_PACKED_PIXEL_STREAM_YCBCR16		= 0x2c,

	MIPI_DSI_PACKED_PIXEL_STREAM_30			= 0x0d,
	MIPI_DSI_PACKED_PIXEL_STREAM_36			= 0x1d,
	MIPI_DSI_PACKED_PIXEL_STREAM_YCBCR12		= 0x3d,

	MIPI_DSI_PACKED_PIXEL_STREAM_16			= 0x0e,
	MIPI_DSI_PACKED_PIXEL_STREAM_18			= 0x1e,
	MIPI_DSI_PIXEL_STREAM_3BYTE_18			= 0x2e,
	MIPI_DSI_PACKED_PIXEL_STREAM_24			= 0x3e,
};

/*
 * struct mipi_dsim_config - interface for configuring mipi-dsi controller.
 *
 * @auto_flush: enable or disable Auto flush of MD FIFO using VSYNC pulse.
 * @eot_disable: enable or disable EoT packet in HS mode.
 * @auto_vertical_cnt: specifies auto vertical count mode.
 *	in Video mode, the vertical line transition uses line counter
 *	configured by VSA, VBP, and Vertical resolution.
 *	If this bit is set to '1', the line counter does not use VSA and VBP
 *	registers.(in command mode, this variable is ignored)
 * @hse: set horizontal sync event mode.
 *	In VSYNC pulse and Vporch area, MIPI DSI master transfers only HSYNC
 *	start packet to MIPI DSI slave at MIPI DSI spec1.1r02.
 *	this bit transfers HSYNC end packet in VSYNC pulse and Vporch area
 *	(in mommand mode, this variable is ignored)
 * @hfp: specifies HFP disable mode.
 *	if this variable is set, DSI master ignores HFP area in VIDEO mode.
 *	(in command mode, this variable is ignored)
 * @hbp: specifies HBP disable mode.
 *	if this variable is set, DSI master ignores HBP area in VIDEO mode.
 *	(in command mode, this variable is ignored)
 * @hsa: specifies HSA disable mode.
 *	if this variable is set, DSI master ignores HSA area in VIDEO mode.
 *	(in command mode, this variable is ignored)
 * @e_interface: specifies interface to be used.(CPU or RGB interface)
 * @e_virtual_ch: specifies virtual channel number that main or
 *	sub diaplsy uses.
 * @e_pixel_format: specifies pixel stream format for main or sub display.
 * @e_burst_mode: selects Burst mode in Video mode.
 *	in Non-burst mode, RGB data area is filled with RGB data and NULL
 *	packets, according to input bandwidth of RGB interface.
 *	In Burst mode, RGB data area is filled with RGB data only.
 * @e_no_data_lane: specifies data lane count to be used by Master.
 * @e_byte_clk: select byte clock source. (it must be DSIM_PLL_OUT_DIV8)
 *	DSIM_EXT_CLK_DIV8 and DSIM_EXT_CLK_BYPASSS are not supported.
 * @pll_stable_time: specifies the PLL Timer for stability of the ganerated
 *	clock(System clock cycle base)
 *	if the timer value goes to 0x00000000, the clock stable bit of status
 *	and interrupt register is set.
 * @esc_clk: specifies escape clock frequency for getting the escape clock
 *	prescaler value.
 * @stop_holding_cnt: specifies the interval value between transmitting
 *	read packet(or write "set_tear_on" command) and BTA request.
 *	after transmitting read packet or write "set_tear_on" command,
 *	BTA requests to D-PHY automatically. this counter value specifies
 *	the interval between them.
 * @bta_timeout: specifies the timer for BTA.
 *	this register specifies time out from BTA request to change
 *	the direction with respect to Tx escape clock.
 * @rx_timeout: specifies the timer for LP Rx mode timeout.
 *	this register specifies time out on how long RxValid deasserts,
 *	after RxLpdt asserts with respect to Tx escape clock.
 *	- RxValid specifies Rx data valid indicator.
 *	- RxLpdt specifies an indicator that D-PHY is under RxLpdt mode.
 *	- RxValid and RxLpdt specifies signal from D-PHY.
 */
struct mipi_dsim_config {
	unsigned char			auto_flush;
	unsigned char			eot_disable;

	unsigned char			auto_vertical_cnt;
	unsigned char			hse;
	unsigned char			hfp;
	unsigned char			hbp;
	unsigned char			hsa;

	enum mipi_dsim_interface_type	e_interface;
	enum mipi_dsim_virtual_ch_no	e_virtual_ch;
	enum mipi_dsim_pixel_format	e_pixel_format;
	enum mipi_dsim_burst_mode_type	e_burst_mode;
	enum mipi_dsim_no_of_data_lane	e_no_data_lane;
	enum mipi_dsim_byte_clk_src	e_byte_clk;

	/*
	 * ===========================================
	 * |    P    |    M    |    S    |    MHz    |
	 * -------------------------------------------
	 * |    3    |   100   |    3    |    100    |
	 * |    3    |   100   |    2    |    200    |
	 * |    3    |    63   |    1    |    252    |
	 * |    4    |   100   |    1    |    300    |
	 * |    4    |   110   |    1    |    330    |
	 * |   12    |   350   |    1    |    350    |
	 * |    3    |   100   |    1    |    400    |
	 * |    4    |   150   |    1    |    450    |
	 * |    6    |   118   |    1    |    472    |
	 * |	3    |   120   |    1    |    480    |
	 * |   12    |   250   |    0    |    500    |
	 * |    4    |   100   |    0    |    600    |
	 * |    3    |    81   |    0    |    648    |
	 * |    3    |    88   |    0    |    704    |
	 * |    3    |    90   |    0    |    720    |
	 * |    3    |   100   |    0    |    800    |
	 * |   12    |   425   |    0    |    850    |
	 * |    4    |   150   |    0    |    900    |
	 * |   12    |   475   |    0    |    950    |
	 * |    6    |   250   |    0    |   1000    |
	 * -------------------------------------------
	 */

	/*
	 * pms could be calculated as the following.
	 * M * 24 / P * 2 ^ S = MHz
	 */
	unsigned char			p;
	unsigned short			m;
	unsigned char			s;

	unsigned int			pll_stable_time;
	unsigned long			esc_clk;

	unsigned short			stop_holding_cnt;
	unsigned char			bta_timeout;
	unsigned short			rx_timeout;
};

/*
 * struct mipi_dsim_device - global interface for mipi-dsi driver.
 *
 * @dsim_config: infomation for configuring mipi-dsi controller.
 * @master_ops: callbacks to mipi-dsi operations.
 * @dsim_lcd_dev: pointer to activated ddi device.
 *	(it would be registered by mipi-dsi driver.)
 * @dsim_lcd_drv: pointer to activated_ddi driver.
 *	(it would be registered by mipi-dsi driver.)
 * @state: specifies status of MIPI-DSI controller.
 *	the status could be RESET, INIT, STOP, HSCLKEN and ULPS.
 * @data_lane: specifiec enabled data lane number.
 *	this variable would be set by driver according to e_no_data_lane
 *	automatically.
 * @e_clk_src: select byte clock source.
 * @pd: pointer to MIPI-DSI driver platform data.
 */
struct mipi_dsim_device {
	struct mipi_dsim_config		*dsim_config;
	struct mipi_dsim_master_ops	*master_ops;
	struct mipi_dsim_lcd_device	*dsim_lcd_dev;
	struct mipi_dsim_lcd_driver	*dsim_lcd_drv;

	unsigned int			state;
	unsigned int			data_lane;
	enum mipi_dsim_byte_clk_src	e_clk_src;

	struct exynos_platform_mipi_dsim	*pd;
};

/*
 * struct exynos_platform_mipi_dsim - interface to platform data
 *	for mipi-dsi driver.
 *
 * @lcd_panel_name: specifies lcd panel name registered to mipi-dsi driver.
 *	lcd panel driver searched would be actived.
 * @dsim_config: pointer of structure for configuring mipi-dsi controller.
 * @lcd_panel_info: pointer for lcd panel specific structure.
 *	this structure specifies width, height, timing and polarity and so on.
 * @lcd_power: callback pointer for enabling or disabling lcd power.
 * @mipi_power: callback pointer for enabling or disabling mipi power.
 * @phy_enable: pointer to a callback controlling D-PHY enable/reset
 */
struct exynos_platform_mipi_dsim {
	char				lcd_panel_name[PANEL_NAME_SIZE];

	struct mipi_dsim_config		*dsim_config;
	void				*lcd_panel_info;

	int (*lcd_power)(void);
	int (*mipi_power)(void);
	void (*phy_enable)(unsigned int dev_index, unsigned int enable);
};

/*
 * struct mipi_dsim_master_ops - callbacks to mipi-dsi operations.
 *
 * @cmd_write: transfer command to lcd panel at LP mode.
 * @cmd_read: read command from rx register.
 * @get_dsim_frame_done: get the status that all screen data have been
 *	transferred to mipi-dsi.
 * @clear_dsim_frame_done: clear frame done status.
 * @get_fb_frame_done: get frame done status of display controller.
 * @trigger: trigger display controller.
 *	- this one would be used only in case of CPU mode.
 */
struct mipi_dsim_master_ops {
	int (*cmd_write)(struct mipi_dsim_device *dsim, unsigned int data_id,
		const unsigned char *data0, unsigned int data1);
	int (*cmd_read)(struct mipi_dsim_device *dsim, unsigned int data_id,
		unsigned int data0, unsigned int data1);
	int (*get_dsim_frame_done)(struct mipi_dsim_device *dsim);
	int (*clear_dsim_frame_done)(struct mipi_dsim_device *dsim);

	int (*get_fb_frame_done)(void);
	void (*trigger)(struct fb_info *info);
};

/*
 * device structure for mipi-dsi based lcd panel.
 *
 * @name: name of the device to use with this device, or an
 *	alias for that name.
 * @id: id of device to be registered.
 * @bus_id: bus id for identifing connected bus
 *	and this bus id should be same as id of mipi_dsim_device.
 * @master: pointer to mipi-dsi master device object.
 * @platform_data: lcd panel specific platform data.
 */
struct mipi_dsim_lcd_device {
	char			*name;
	int			id;
	int			bus_id;
	int			reverse_panel;

	struct mipi_dsim_device *master;
	struct exynos_platform_mipi_dsim *platform_data;
};

/*
 * driver structure for mipi-dsi based lcd panel.
 *
 * this structure should be registered by lcd panel driver.
 * mipi-dsi driver seeks lcd panel registered through name field
 * and calls these callback functions in appropriate time.
 *
 * @name: name of the driver to use with this device, or an
 *	alias for that name.
 * @id: id of driver to be registered.
 *	this id would be used for finding device object registered.
 * @mipi_panel_init: callback pointer for initializing lcd panel based on mipi
 *	dsi interface.
 * @mipi_display_on: callback pointer for lcd panel display on.
 */
struct mipi_dsim_lcd_driver {
	char			*name;
	int			id;

	int	(*mipi_panel_init)(struct mipi_dsim_device *dsim_dev);
	void	(*mipi_display_on)(struct mipi_dsim_device *dsim_dev);
};

#ifdef CONFIG_EXYNOS_MIPI_DSIM
int exynos_mipi_dsi_init(struct exynos_platform_mipi_dsim *dsim_pd);
#else
static inline int exynos_mipi_dsi_init(
			struct exynos_platform_mipi_dsim *dsim_pd)
{
	return 0;
}
#endif

/*
 * register mipi_dsim_lcd_driver object defined by lcd panel driver
 * to mipi-dsi driver.
 */
int exynos_mipi_dsi_register_lcd_driver(struct mipi_dsim_lcd_driver
						*lcd_drv);

/*
 * register mipi_dsim_lcd_device to mipi-dsi master.
 */
int exynos_mipi_dsi_register_lcd_device(struct mipi_dsim_lcd_device
						*lcd_dev);

void exynos_set_dsim_platform_data(struct exynos_platform_mipi_dsim *pd);
struct vidinfo;
void exynos_init_dsim_platform_data(struct vidinfo *vid);

/* panel driver init based on mipi dsi interface */
void s6e8ax0_init(void);

extern int mipi_power(void);
#endif /* _DSIM_H */
