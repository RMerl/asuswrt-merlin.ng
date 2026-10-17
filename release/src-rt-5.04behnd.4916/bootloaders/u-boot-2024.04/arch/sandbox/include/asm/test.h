/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Test-related constants for sandbox
 *
 * Copyright (c) 2014 Google, Inc
 */

#ifndef __ASM_TEST_H
#define __ASM_TEST_H

#include <pci_ids.h>

struct unit_test_state;

/* The sandbox driver always permits an I2C device with this address */
#define SANDBOX_I2C_TEST_ADDR		0x59

#define SANDBOX_PCI_VENDOR_ID		0x1234
#define SANDBOX_PCI_SWAP_CASE_EMUL_ID	0x5678
#define SANDBOX_PCI_PMC_EMUL_ID		0x5677
#define SANDBOX_PCI_P2SB_EMUL_ID	0x5676
#define SANDBOX_PCI_CLASS_CODE		(PCI_CLASS_COMMUNICATION_SERIAL >> 8)
#define SANDBOX_PCI_CLASS_SUB_CODE	(PCI_CLASS_COMMUNICATION_SERIAL & 0xff)

#define PCI_CAP_ID_PM_OFFSET		0x50
#define PCI_CAP_ID_EXP_OFFSET		0x60
#define PCI_CAP_ID_MSIX_OFFSET		0x70
#define PCI_CAP_ID_EA_OFFSET		0x80

#define PCI_EXT_CAP_ID_ERR_OFFSET	0x100
#define PCI_EXT_CAP_ID_VC_OFFSET	0x200
#define PCI_EXT_CAP_ID_DSN_OFFSET	0x300

/* Useful for PCI_VDEVICE() macro */
#define PCI_VENDOR_ID_SANDBOX		SANDBOX_PCI_VENDOR_ID
#define SWAP_CASE_DRV_DATA		0x55aa

#define SANDBOX_CLK_RATE		32768

/* Macros used to test PCI EA capability structure */
#define PCI_CAP_EA_BASE_LO0		0x00100000
#define PCI_CAP_EA_BASE_LO1		0x00110000
#define PCI_CAP_EA_BASE_LO2		0x00120000
#define PCI_CAP_EA_BASE_LO4		0x00140000
#define PCI_CAP_EA_BASE_HI2		0x00020000ULL
#define PCI_CAP_EA_BASE_HI4		0x00040000ULL
#define PCI_CAP_EA_SIZE_LO		0x0000ffff
#define PCI_CAP_EA_SIZE_HI		0x00000010ULL
#define PCI_EA_BAR2_MAGIC		0x72727272
#define PCI_EA_BAR4_MAGIC		0x74747474

enum {
	SANDBOX_IRQN_PEND = 1,	/* Interrupt number for 'pending' test */
};

/* System controller driver data */
enum {
	SYSCON0		= 32,
	SYSCON1,

	SYSCON_COUNT
};

/**
 */
enum cros_ec_test_t {
	CROSECT_BREAK_HELLO	= BIT(1),
	CROSECT_LID_OPEN	= BIT(2),
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

void sandbox_i2c_eeprom_set_chip_addr_offset_mask(struct udevice *dev,
						  uint mask);

uint sanbox_i2c_eeprom_get_prev_addr(struct udevice *dev);

uint sanbox_i2c_eeprom_get_prev_offset(struct udevice *dev);

/**
 * sandbox_i2c_rtc_set_offset() - set the time offset from system/base time
 *
 * @dev:		RTC device to adjust
 * @use_system_time:	true to use system time, false to use @base_time
 * @offset:		RTC offset from current system/base time (-1 for no
 *			change)
 * Return: old value of RTC offset
 */
long sandbox_i2c_rtc_set_offset(struct udevice *dev, bool use_system_time,
				int offset);

/**
 * sandbox_i2c_rtc_get_set_base_time() - get and set the base time
 *
 * @dev:		RTC device to adjust
 * @base_time:		New base system time (set to -1 for no change)
 * Return: old base time
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
 * Return: 0 if OK, -ENOSPC if the PWM number is invalid
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
 * Return: sum of audio data
 */
int sandbox_get_i2s_sum(struct udevice *dev);

/**
 * sandbox_get_setup_called() - Returns the number of times setup(*) was called
 *
 * This is used in the sound test
 *
 * @dev: Device to check
 * Return: call count for the setup() method
 */
int sandbox_get_setup_called(struct udevice *dev);

/**
 * sandbox_get_sound_active() - Returns whether sound play is in progress
 *
 * Return: true if active, false if not
 */
int sandbox_get_sound_active(struct udevice *dev);

/**
 * sandbox_get_sound_count() - Read back the count of the sound data so far
 *
 * This data is provided to the sandbox driver by the sound play() method.
 *
 * @dev: Device to check
 * Return: count of audio data
 */
int sandbox_get_sound_count(struct udevice *dev);

/**
 * sandbox_get_sound_sum() - Read back the sum of the sound data so far
 *
 * This data is provided to the sandbox driver by the sound play() method.
 *
 * @dev: Device to check
 * Return: sum of audio data
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
 * Return: frequency of beep, if there is an active beep, else 0
 */
int sandbox_get_beep_frequency(struct udevice *dev);

/**
 * sandbox_spi_get_speed() - Get current speed setting of a sandbox spi bus
 *
 * @dev: Device to check
 * Return: current bus speed
 */
uint sandbox_spi_get_speed(struct udevice *dev);

/**
 * sandbox_spi_get_mode() - Get current mode setting of a sandbox spi bus
 *
 * @dev: Device to check
 * Return: current mode
 */
uint sandbox_spi_get_mode(struct udevice *dev);

/**
 * sandbox_get_pch_spi_protect() - Get the PCI SPI protection status
 *
 * @dev: Device to check
 * Return: 0 if not protected, 1 if protected
 */
int sandbox_get_pch_spi_protect(struct udevice *dev);

/**
 * sandbox_get_pci_ep_irq_count() - Get the PCI EP IRQ count
 *
 * @dev: Device to check
 * Return: irq count
 */
int sandbox_get_pci_ep_irq_count(struct udevice *dev);

/**
 * sandbox_pci_read_bar() - Read the BAR value for a read_config operation
 *
 * This is used in PCI emulators to read a base address reset. This has special
 * rules because when the register is set to 0xffffffff it can be used to
 * discover the type and size of the BAR.
 *
 * @barval: Current value of the BAR
 * @type: Type of BAR (PCI_BASE_ADDRESS_SPACE_IO or
 *		PCI_BASE_ADDRESS_MEM_TYPE_32)
 * @size: Size of BAR in bytes
 * Return: BAR value to return from emulator
 */
uint sandbox_pci_read_bar(u32 barval, int type, uint size);

/**
 * sandbox_set_enable_memio() - Enable readl/writel() for sandbox
 *
 * Normally these I/O functions do nothing with sandbox. Certain tests need them
 * to work as for other architectures, so this function can be used to enable
 * them.
 *
 * @enable: true to enable, false to disable
 */
void sandbox_set_enable_memio(bool enable);

/**
 * sandbox_cros_ec_set_test_flags() - Set behaviour for testing purposes
 *
 * @dev: Device to check
 * @flags: Flags to control behaviour (CROSECT_...)
 */
void sandbox_cros_ec_set_test_flags(struct udevice *dev, uint flags);

/**
 * sandbox_cros_ec_get_pwm_duty() - Get EC PWM config for testing purposes
 *
 * @dev: Device to check
 * @index: PWM channel index
 * @duty: Current duty cycle in 0..EC_PWM_MAX_DUTY range.
 * Return: 0 if OK, -ENOSPC if the PWM number is invalid
 */
int sandbox_cros_ec_get_pwm_duty(struct udevice *dev, uint index, uint *duty);

/**
 * sandbox_set_fake_efi_mgr_dev() - Control EFI bootmgr producing valid bootflow
 *
 * This is only used for testing.
 *
 * @dev: efi_mgr bootmeth device
 * @fake_dev: true to produce a valid bootflow when requested, false to produce
 * an error
 */
void sandbox_set_fake_efi_mgr_dev(struct udevice *dev, bool fake_dev);

/**
 * sandbox_load_other_fdt() - load the 'other' FDT into the test state
 *
 * This copies the other.dtb file into the test state, so that a fresh version
 * can be used for a test that is about to run.
 *
 * If @uts->other_fdt is NULL, as it is when first set up, this allocates a
 * buffer for the other FDT and sets @uts->other_fdt_size to its size.
 *
 * In any case, the other FDT is copied from the sandbox state into
 * @uts->other_fdt ready for use.
 *
 * @uts: Unit test state
 * @return 0 if OK, -ve on error
 */
int sandbox_load_other_fdt(void **fdtp, int *sizep);

/**
 * sandbox_set_eth_enable() - Enable / disable Ethernet
 *
 * Allows control of whether Ethernet packets are actually send/received
 *
 * @enable: true to enable Ethernet, false to disable
 */
void sandbox_set_eth_enable(bool enable);

/**
 * sandbox_eth_enabled() - Check if Ethernet is enabled
 *
 * Returns: true if Ethernet is enabled on sandbox, False if not
 */
bool sandbox_eth_enabled(void);

/**
 * sandbox_sf_bootdev_enabled() - Check if SPI flash bootdevs should be bound
 *
 * Returns: true if sandbox should bind bootdevs for SPI flash, false if not
 */
bool sandbox_sf_bootdev_enabled(void);

/**
 * sandbox_sf_set_enable_bootdevs() - Enable / disable the SPI flash bootdevs
 *
 * @enable: true to bind the SPI flash bootdevs, false to skip
 */
void sandbox_sf_set_enable_bootdevs(bool enable);

#endif
