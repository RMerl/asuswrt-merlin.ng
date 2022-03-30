/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Test-related constants for sandbox
 *
 * Copyright (c) 2014 Google, Inc
 */

#ifndef __ASM_TEST_H
#define __ASM_TEST_H

/* The sandbox driver always permits an I2C device with this address */
#define SANDBOX_I2C_TEST_ADDR		0x59

#define SANDBOX_PCI_VENDOR_ID		0x1234
#define SANDBOX_PCI_DEVICE_ID		0x5678
#define SANDBOX_PCI_CLASS_CODE		PCI_CLASS_CODE_COMM
#define SANDBOX_PCI_CLASS_SUB_CODE	PCI_CLASS_SUB_CODE_COMM_SERIAL

#define PCI_CAP_ID_PM_OFFSET		0x50
#define PCI_CAP_ID_EXP_OFFSET		0x60
#define PCI_CAP_ID_MSIX_OFFSET		0x70

#define PCI_EXT_CAP_ID_ERR_OFFSET	0x100
#define PCI_EXT_CAP_ID_VC_OFFSET	0x200
#define PCI_EXT_CAP_ID_DSN_OFFSET	0x300

/* Useful for PCI_VDEVICE() macro */
#define PCI_VENDOR_ID_SANDBOX		SANDBOX_PCI_VENDOR_ID
#define SWAP_CASE_DRV_DATA		0x55aa

#define SANDBOX_CLK_RATE		32768

/* System controller driver data */
enum {
	SYSCON0		= 32,
	SYSCON1,

	SYSCON_COUNT
};

/**
 * sandbox_i2c_set_test_mode() - set test mode for running unit tests
 *
 * See sandbox_i2c_xfer() for the behaviour changes.
 *
 * @bus:	sandbox I2C bus to adjust
 * @test_mode:	true to select test mode, false to run normally
 */
void sandbox_i2c_set_test_mode(struct udevice *bus, bool test_mode);

enum sandbox_i2c_eeprom_test_mode {
	SIE_TEST_MODE_NONE,
	/* Permits read/write of only one byte per I2C transaction */
	SIE_TEST_MODE_SINGLE_BYTE,
};

void sandbox_i2c_eeprom_set_test_mode(struct udevice *dev,
				      enum sandbox_i2c_eeprom_test_mode mode);

void sandbox_i2c_eeprom_set_offset_len(struct udevice *dev, int offset_len);

/**
 * sandbox_i2c_rtc_set_offset() - set the time offset from system/base time
 *
 * @dev:		RTC device to adjust
 * @use_system_time:	true to use system time, false to use @base_time
 * @offset:		RTC offset from current system/base time (-1 for no
 *			change)
 * @return old value of RTC offset
 */
long sandbox_i2c_rtc_set_offset(struct udevice *dev, bool use_system_time,
				int offset);

/**
 * sandbox_i2c_rtc_get_set_base_time() - get and set the base time
 *
 * @dev:		RTC device to adjust
 * @base_time:		New base system time (set to -1 for no change)
 * @return old base time
 */
long sandbox_i2c_rtc_get_set_base_time(struct udevice *dev, long base_time);

int sandbox_usb_keyb_add_string(struct udevice *dev, const char *str);

/**
 * sandbox_osd_get_mem() - get the internal memory of a sandbox OSD
 *
 * @dev:	OSD device for which to access the internal memory for
 * @buf:	pointer to buffer to receive the OSD memory data
 * @buflen:	length of buffer in bytes
 */
int sandbox_osd_get_mem(struct udevice *dev, u8 *buf, size_t buflen);

/**
 * sandbox_pwm_get_config() - get the PWM config for a channel
 *
 * @dev: Device to check
 * @channel: Channel number to check
 * @period_ns: Period of the PWM in nanoseconds
 * @duty_ns: Current duty cycle of the PWM in nanoseconds
 * @enable: true if the PWM is enabled
 * @polarity: true if the PWM polarity is active high
 * @return 0 if OK, -ENOSPC if the PWM number is invalid
 */
int sandbox_pwm_get_config(struct udevice *dev, uint channel, uint *period_nsp,
			   uint *duty_nsp, bool *enablep, bool *polarityp);

/**
 * sandbox_sf_set_block_protect() - Set the BP bits of the status register
 *
 * @dev: Device to update
 * @bp_mask: BP bits to set (bits 2:0, so a value of 0 to 7)
 */
void sandbox_sf_set_block_protect(struct udevice *dev, int bp_mask);

/**
 * sandbox_get_codec_params() - Read back codec parameters
 *
 * This reads back the parameters set by audio_codec_set_params() for the
 * sandbox audio driver. Arguments are as for that function.
 */
void sandbox_get_codec_params(struct udevice *dev, int *interfacep, int *ratep,
			      int *mclk_freqp, int *bits_per_samplep,
			      uint *channelsp);

/**
 * sandbox_get_i2s_sum() - Read back the sum of the audio data so far
 *
 * This data is provided to the sandbox driver by the I2S tx_data() method.
 *
 * @dev: Device to check
 * @return sum of audio data
 */
int sandbox_get_i2s_sum(struct udevice *dev);

/**
 * sandbox_get_setup_called() - Returns the number of times setup(*) was called
 *
 * This is used in the sound test
 *
 * @dev: Device to check
 * @return call count for the setup() method
 */
int sandbox_get_setup_called(struct udevice *dev);

/**
 * sandbox_get_sound_sum() - Read back the sum of the sound data so far
 *
 * This data is provided to the sandbox driver by the sound play() method.
 *
 * @dev: Device to check
 * @return sum of audio data
 */
int sandbox_get_sound_sum(struct udevice *dev);

/**
 * sandbox_set_allow_beep() - Set whether the 'beep' interface is supported
 *
 * @dev: Device to update
 * @allow: true to allow the start_beep() method, false to disallow it
 */
void sandbox_set_allow_beep(struct udevice *dev, bool allow);

/**
 * sandbox_get_beep_frequency() - Get the frequency of the current beep
 *
 * @dev: Device to check
 * @return frequency of beep, if there is an active beep, else 0
 */
int sandbox_get_beep_frequency(struct udevice *dev);

/**
 * sandbox_get_pch_spi_protect() - Get the PCI SPI protection status
 *
 * @dev: Device to check
 * @return 0 if not protected, 1 if protected
 */
int sandbox_get_pch_spi_protect(struct udevice *dev);

#endif
