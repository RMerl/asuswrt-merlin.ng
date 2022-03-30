/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) 2015 Siarhei Siamashka <siarhei.siamashka@gmail.com>
 */

/*
 * Support for the SSD2828 bridge chip, which can take pixel data coming
 * from a parallel LCD interface and translate it on the flight into MIPI DSI
 * interface for driving a MIPI compatible TFT display.
 *
 * Implemented as a utility function. To be used from display drivers, which are
 * responsible for driving parallel LCD hardware in front of the video pipeline.
 */

#ifndef _SSD2828_H
#define _SSD2828_H

struct ctfb_res_modes;

struct ssd2828_config {
	/*********************************************************************/
	/* SSD2828 configuration                                             */
	/*********************************************************************/

	/*
	 * The pins, which are used for SPI communication. This is only used
	 * for configuring SSD2828, so the performance is irrelevant (only
	 * around a hundred of bytes is moved). Also these can be any arbitrary
	 * GPIO pins (not necessarily the pins having hardware SPI function).
	 * Moreover, the 'sdo' pin may be even not wired up in some devices.
	 *
	 * These configuration variables need to be set as pin numbers for
	 * the standard u-boot GPIO interface (gpio_get_value/gpio_set_value
	 * functions). Note that -1 value can be used for the pins, which are
	 * not really wired up.
	 */
	int csx_pin;
	int sck_pin;
	int sdi_pin;
	int sdo_pin;
	/* SSD2828 reset pin (shared with LCD panel reset) */
	int reset_pin;

	/*
	 * The SSD2828 has its own dedicated clock source 'tx_clk' (connected
	 * to TX_CLK_XIO/TX_CLK_XIN pins), which is necessary at least for
	 * clocking SPI after reset. The exact clock speed is not strictly,
	 * defined, but the datasheet says that it must be somewhere in the
	 * 8MHz - 30MHz range (see "TX_CLK Timing" section). It can be also
	 * used as a reference clock for PLL. If the exact clock frequency
	 * is known, then it can be specified here. If it is unknown, or the
	 * information is not trustworthy, then it can be set to 0.
	 *
	 * If unsure, set to 0.
	 */
	int ssd2828_tx_clk_khz;

	/*
	 * This is not a property of the used LCD panel, but more like a
	 * property of the SSD2828 wiring. See the "SSD2828QN4 RGB data
	 * arrangement" table in the datasheet. The SSD2828 pins are arranged
	 * in such a way that 18bpp and 24bpp configurations are completely
	 * incompatible with each other.
	 *
	 * Depending on the color depth, this must be set to 16, 18 or 24.
	 */
	int ssd2828_color_depth;

	/*********************************************************************/
	/* LCD panel configuration                                           */
	/*********************************************************************/

	/*
	 * The number of lanes in the MIPI DSI interface. May vary from 1 to 4.
	 *
	 * This information can be found in the LCD panel datasheet.
	 */
	int mipi_dsi_number_of_data_lanes;

	/*
	 * Data transfer bit rate per lane. Please note that it is expected
	 * to be higher than the pixel clock rate of the used video mode when
	 * multiplied by the number of lanes. This is perfectly normal because
	 * MIPI DSI handles data transfers in periodic bursts, and uses the
	 * idle time between bursts for sending configuration information and
	 * commands. Or just for saving power.
	 *
	 * The necessary Mbps/lane information can be found in the LCD panel
	 * datasheet. Note that the transfer rate can't be always set precisely
	 * and it may be rounded *up* (introducing no more than 10Mbps error).
	 */
	int mipi_dsi_bitrate_per_data_lane_mbps;

	/*
	 * Setting this to 1 enforces packing of 18bpp pixel data in 24bpp
	 * envelope when sending it over the MIPI DSI link.
	 *
	 * If unsure, set to 0.
	 */
	int mipi_dsi_loosely_packed_pixel_format;

	/*
	 * According to the "Example for system sleep in and out" section in
	 * the SSD2828 datasheet, some LCD panel specific delays are necessary
	 * after MIPI DCS commands EXIT_SLEEP_MODE and SET_DISPLAY_ON.
	 *
	 * For example, Allwinner uses 100 milliseconds delay after
	 * EXIT_SLEEP_MODE and 200 milliseconds delay after SET_DISPLAY_ON.
	 */
	int mipi_dsi_delay_after_exit_sleep_mode_ms;
	int mipi_dsi_delay_after_set_display_on_ms;
};

/*
 * Initialize the SSD2828 chip. It needs the 'ssd2828_config' structure
 * and also the video mode timings.
 *
 * The right place to insert this function call is after the parallel LCD
 * interface is initialized and before turning on the backlight. This is
 * advised in the "Example for system sleep in and out" section of the
 * SSD2828 datasheet. And also SS2828 may use 'pclk' as the clock source
 * for PLL, which means that the input signal must be already there.
 */
int ssd2828_init(const struct ssd2828_config *cfg,
		 const struct ctfb_res_modes *mode);

#endif
