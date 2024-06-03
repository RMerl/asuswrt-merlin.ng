// SPDX-License-Identifier: GPL-2.0+
/**************************************************************************
Intel Pro 1000 for ppcboot/das-u-boot
Drivers are port from Intel's Linux driver e1000-4.3.15
and from Etherboot pro 1000 driver by mrakes at vivato dot net
tested on both gig copper and gig fiber boards
***************************************************************************/
/*******************************************************************************


  Copyright(c) 1999 - 2002 Intel Corporation. All rights reserved.


  Contact Information:
  Linux NICS <linux.nics@intel.com>
  Intel Corporation, 5200 N.E. Elam Young Parkway, Hillsboro, OR 97124-6497

*******************************************************************************/
/*
 *  Copyright (C) Archway Digital Solutions.
 *
 *  written by Chrsitopher Li <cli at arcyway dot com> or <chrisl at gnuchina dot org>
 *  2/9/2002
 *
 *  Copyright (C) Linux Networx.
 *  Massive upgrade to work with the new intel gigabit NICs.
 *  <ebiederman at lnxi dot com>
 *
 *  Copyright 2011 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <memalign.h>
#include <pci.h>
#include "e1000.h"

#define TOUT_LOOP   100000

#ifdef CONFIG_DM_ETH
#define virt_to_bus(devno, v)	dm_pci_virt_to_mem(devno, (void *) (v))
#define bus_to_phys(devno, a)	dm_pci_mem_to_phys(devno, a)
#else
#define virt_to_bus(devno, v)	pci_virt_to_mem(devno, (void *) (v))
#define bus_to_phys(devno, a)	pci_mem_to_phys(devno, a)
#endif

#define E1000_DEFAULT_PCI_PBA	0x00000030
#define E1000_DEFAULT_PCIE_PBA	0x000a0026

/* NIC specific static variables go here */

/* Intel i210 needs the DMA descriptor rings aligned to 128b */
#define E1000_BUFFER_ALIGN	128

/*
 * TODO(sjg@chromium.org): Even with driver model we share these buffers.
 * Concurrent receiving on multiple active Ethernet devices will not work.
 * Normally U-Boot does not support this anyway. To fix it in this driver,
 * move these buffers and the tx/rx pointers to struct e1000_hw.
 */
DEFINE_ALIGN_BUFFER(struct e1000_tx_desc, tx_base, 16, E1000_BUFFER_ALIGN);
DEFINE_ALIGN_BUFFER(struct e1000_rx_desc, rx_base, 16, E1000_BUFFER_ALIGN);
DEFINE_ALIGN_BUFFER(unsigned char, packet, 4096, E1000_BUFFER_ALIGN);

static int tx_tail;
static int rx_tail, rx_last;
#ifdef CONFIG_DM_ETH
static int num_cards;	/* Number of E1000 devices seen so far */
#endif

static struct pci_device_id e1000_supported[] = {
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82542) },
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82543GC_FIBER) },
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82543GC_COPPER) },
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82544EI_COPPER) },
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82544EI_FIBER) },
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82544GC_COPPER) },
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82544GC_LOM) },
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82540EM) },
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82545EM_COPPER) },
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82545GM_COPPER) },
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82546EB_COPPER) },
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82545EM_FIBER) },
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82546EB_FIBER) },
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82546GB_COPPER) },
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82540EM_LOM) },
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82541ER) },
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82541GI_LF) },
	/* E1000 PCIe card */
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82571EB_COPPER) },
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82571EB_FIBER) },
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82571EB_SERDES) },
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82571EB_QUAD_COPPER) },
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82571PT_QUAD_COPPER) },
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82571EB_QUAD_FIBER) },
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82571EB_QUAD_COPPER_LOWPROFILE) },
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82571EB_SERDES_DUAL) },
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82571EB_SERDES_QUAD) },
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82572EI_COPPER) },
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82572EI_FIBER) },
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82572EI_SERDES) },
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82572EI) },
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82573E) },
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82573E_IAMT) },
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82573L) },
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82574L) },
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82546GB_QUAD_COPPER_KSP3) },
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_80003ES2LAN_COPPER_DPT) },
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_80003ES2LAN_SERDES_DPT) },
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_80003ES2LAN_COPPER_SPT) },
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_80003ES2LAN_SERDES_SPT) },
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_I210_UNPROGRAMMED) },
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_I211_UNPROGRAMMED) },
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_I210_COPPER) },
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_I211_COPPER) },
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_I210_COPPER_FLASHLESS) },
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_I210_SERDES) },
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_I210_SERDES_FLASHLESS) },
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_I210_1000BASEKX) },

	{}
};

/* Function forward declarations */
static int e1000_setup_link(struct e1000_hw *hw);
static int e1000_setup_fiber_link(struct e1000_hw *hw);
static int e1000_setup_copper_link(struct e1000_hw *hw);
static int e1000_phy_setup_autoneg(struct e1000_hw *hw);
static void e1000_config_collision_dist(struct e1000_hw *hw);
static int e1000_config_mac_to_phy(struct e1000_hw *hw);
static int e1000_config_fc_after_link_up(struct e1000_hw *hw);
static int e1000_check_for_link(struct e1000_hw *hw);
static int e1000_wait_autoneg(struct e1000_hw *hw);
static int e1000_get_speed_and_duplex(struct e1000_hw *hw, uint16_t * speed,
				       uint16_t * duplex);
static int e1000_read_phy_reg(struct e1000_hw *hw, uint32_t reg_addr,
			      uint16_t * phy_data);
static int e1000_write_phy_reg(struct e1000_hw *hw, uint32_t reg_addr,
			       uint16_t phy_data);
static int32_t e1000_phy_hw_reset(struct e1000_hw *hw);
static int e1000_phy_reset(struct e1000_hw *hw);
static int e1000_detect_gig_phy(struct e1000_hw *hw);
static void e1000_set_media_type(struct e1000_hw *hw);

static int32_t e1000_swfw_sync_acquire(struct e1000_hw *hw, uint16_t mask);
static void e1000_swfw_sync_release(struct e1000_hw *hw, uint16_t mask);
static int32_t e1000_check_phy_reset_block(struct e1000_hw *hw);

#ifndef CONFIG_E1000_NO_NVM
static void e1000_put_hw_eeprom_semaphore(struct e1000_hw *hw);
static int32_t e1000_get_hw_eeprom_semaphore(struct e1000_hw *hw);
static int32_t e1000_read_eeprom(struct e1000_hw *hw, uint16_t offset,
		uint16_t words,
		uint16_t *data);
/******************************************************************************
 * Raises the EEPROM's clock input.
 *
 * hw - Struct containing variables accessed by shared code
 * eecd - EECD's current value
 *****************************************************************************/
void e1000_raise_ee_clk(struct e1000_hw *hw, uint32_t * eecd)
{
	/* Raise the clock input to the EEPROM (by setting the SK bit), and then
	 * wait 50 microseconds.
	 */
	*eecd = *eecd | E1000_EECD_SK;
	E1000_WRITE_REG(hw, EECD, *eecd);
	E1000_WRITE_FLUSH(hw);
	udelay(50);
}

/******************************************************************************
 * Lowers the EEPROM's clock input.
 *
 * hw - Struct containing variables accessed by shared code
 * eecd - EECD's current value
 *****************************************************************************/
void e1000_lower_ee_clk(struct e1000_hw *hw, uint32_t * eecd)
{
	/* Lower the clock input to the EEPROM (by clearing the SK bit), and then
	 * wait 50 microseconds.
	 */
	*eecd = *eecd & ~E1000_EECD_SK;
	E1000_WRITE_REG(hw, EECD, *eecd);
	E1000_WRITE_FLUSH(hw);
	udelay(50);
}

/******************************************************************************
 * Shift data bits out to the EEPROM.
 *
 * hw - Struct containing variables accessed by shared code
 * data - data to send to the EEPROM
 * count - number of bits to shift out
 *****************************************************************************/
static void
e1000_shift_out_ee_bits(struct e1000_hw *hw, uint16_t data, uint16_t count)
{
	uint32_t eecd;
	uint32_t mask;

	/* We need to shift "count" bits out to the EEPROM. So, value in the
	 * "data" parameter will be shifted out to the EEPROM one bit at a time.
	 * In order to do this, "data" must be broken down into bits.
	 */
	mask = 0x01 << (count - 1);
	eecd = E1000_READ_REG(hw, EECD);
	eecd &= ~(E1000_EECD_DO | E1000_EECD_DI);
	do {
		/* A "1" is shifted out to the EEPROM by setting bit "DI" to a "1",
		 * and then raising and then lowering the clock (the SK bit controls
		 * the clock input to the EEPROM).  A "0" is shifted out to the EEPROM
		 * by setting "DI" to "0" and then raising and then lowering the clock.
		 */
		eecd &= ~E1000_EECD_DI;

		if (data & mask)
			eecd |= E1000_EECD_DI;

		E1000_WRITE_REG(hw, EECD, eecd);
		E1000_WRITE_FLUSH(hw);

		udelay(50);

		e1000_raise_ee_clk(hw, &eecd);
		e1000_lower_ee_clk(hw, &eecd);

		mask = mask >> 1;

	} while (mask);

	/* We leave the "DI" bit set to "0" when we leave this routine. */
	eecd &= ~E1000_EECD_DI;
	E1000_WRITE_REG(hw, EECD, eecd);
}

/******************************************************************************
 * Shift data bits in from the EEPROM
 *
 * hw - Struct containing variables accessed by shared code
 *****************************************************************************/
static uint16_t
e1000_shift_in_ee_bits(struct e1000_hw *hw, uint16_t count)
{
	uint32_t eecd;
	uint32_t i;
	uint16_t data;

	/* In order to read a register from the EEPROM, we need to shift 'count'
	 * bits in from the EEPROM. Bits are "shifted in" by raising the clock
	 * input to the EEPROM (setting the SK bit), and then reading the
	 * value of the "DO" bit.  During this "shifting in" process the
	 * "DI" bit should always be clear.
	 */

	eecd = E1000_READ_REG(hw, EECD);

	eecd &= ~(E1000_EECD_DO | E1000_EECD_DI);
	data = 0;

	for (i = 0; i < count; i++) {
		data = data << 1;
		e1000_raise_ee_clk(hw, &eecd);

		eecd = E1000_READ_REG(hw, EECD);

		eecd &= ~(E1000_EECD_DI);
		if (eecd & E1000_EECD_DO)
			data |= 1;

		e1000_lower_ee_clk(hw, &eecd);
	}

	return data;
}

/******************************************************************************
 * Returns EEPROM to a "standby" state
 *
 * hw - Struct containing variables accessed by shared code
 *****************************************************************************/
void e1000_standby_eeprom(struct e1000_hw *hw)
{
	struct e1000_eeprom_info *eeprom = &hw->eeprom;
	uint32_t eecd;

	eecd = E1000_READ_REG(hw, EECD);

	if (eeprom->type == e1000_eeprom_microwire) {
		eecd &= ~(E1000_EECD_CS | E1000_EECD_SK);
		E1000_WRITE_REG(hw, EECD, eecd);
		E1000_WRITE_FLUSH(hw);
		udelay(eeprom->delay_usec);

		/* Clock high */
		eecd |= E1000_EECD_SK;
		E1000_WRITE_REG(hw, EECD, eecd);
		E1000_WRITE_FLUSH(hw);
		udelay(eeprom->delay_usec);

		/* Select EEPROM */
		eecd |= E1000_EECD_CS;
		E1000_WRITE_REG(hw, EECD, eecd);
		E1000_WRITE_FLUSH(hw);
		udelay(eeprom->delay_usec);

		/* Clock low */
		eecd &= ~E1000_EECD_SK;
		E1000_WRITE_REG(hw, EECD, eecd);
		E1000_WRITE_FLUSH(hw);
		udelay(eeprom->delay_usec);
	} else if (eeprom->type == e1000_eeprom_spi) {
		/* Toggle CS to flush commands */
		eecd |= E1000_EECD_CS;
		E1000_WRITE_REG(hw, EECD, eecd);
		E1000_WRITE_FLUSH(hw);
		udelay(eeprom->delay_usec);
		eecd &= ~E1000_EECD_CS;
		E1000_WRITE_REG(hw, EECD, eecd);
		E1000_WRITE_FLUSH(hw);
		udelay(eeprom->delay_usec);
	}
}

/***************************************************************************
* Description:     Determines if the onboard NVM is FLASH or EEPROM.
*
* hw - Struct containing variables accessed by shared code
****************************************************************************/
static bool e1000_is_onboard_nvm_eeprom(struct e1000_hw *hw)
{
	uint32_t eecd = 0;

	DEBUGFUNC();

	if (hw->mac_type == e1000_ich8lan)
		return false;

	if (hw->mac_type == e1000_82573 || hw->mac_type == e1000_82574) {
		eecd = E1000_READ_REG(hw, EECD);

		/* Isolate bits 15 & 16 */
		eecd = ((eecd >> 15) & 0x03);

		/* If both bits are set, device is Flash type */
		if (eecd == 0x03)
			return false;
	}
	return true;
}

/******************************************************************************
 * Prepares EEPROM for access
 *
 * hw - Struct containing variables accessed by shared code
 *
 * Lowers EEPROM clock. Clears input pin. Sets the chip select pin. This
 * function should be called before issuing a command to the EEPROM.
 *****************************************************************************/
int32_t e1000_acquire_eeprom(struct e1000_hw *hw)
{
	struct e1000_eeprom_info *eeprom = &hw->eeprom;
	uint32_t eecd, i = 0;

	DEBUGFUNC();

	if (e1000_swfw_sync_acquire(hw, E1000_SWFW_EEP_SM))
		return -E1000_ERR_SWFW_SYNC;
	eecd = E1000_READ_REG(hw, EECD);

	if (hw->mac_type != e1000_82573 && hw->mac_type != e1000_82574) {
		/* Request EEPROM Access */
		if (hw->mac_type > e1000_82544) {
			eecd |= E1000_EECD_REQ;
			E1000_WRITE_REG(hw, EECD, eecd);
			eecd = E1000_READ_REG(hw, EECD);
			while ((!(eecd & E1000_EECD_GNT)) &&
				(i < E1000_EEPROM_GRANT_ATTEMPTS)) {
				i++;
				udelay(5);
				eecd = E1000_READ_REG(hw, EECD);
			}
			if (!(eecd & E1000_EECD_GNT)) {
				eecd &= ~E1000_EECD_REQ;
				E1000_WRITE_REG(hw, EECD, eecd);
				DEBUGOUT("Could not acquire EEPROM grant\n");
				return -E1000_ERR_EEPROM;
			}
		}
	}

	/* Setup EEPROM for Read/Write */

	if (eeprom->type == e1000_eeprom_microwire) {
		/* Clear SK and DI */
		eecd &= ~(E1000_EECD_DI | E1000_EECD_SK);
		E1000_WRITE_REG(hw, EECD, eecd);

		/* Set CS */
		eecd |= E1000_EECD_CS;
		E1000_WRITE_REG(hw, EECD, eecd);
	} else if (eeprom->type == e1000_eeprom_spi) {
		/* Clear SK and CS */
		eecd &= ~(E1000_EECD_CS | E1000_EECD_SK);
		E1000_WRITE_REG(hw, EECD, eecd);
		udelay(1);
	}

	return E1000_SUCCESS;
}

/******************************************************************************
 * Sets up eeprom variables in the hw struct.  Must be called after mac_type
 * is configured.  Additionally, if this is ICH8, the flash controller GbE
 * registers must be mapped, or this will crash.
 *
 * hw - Struct containing variables accessed by shared code
 *****************************************************************************/
static int32_t e1000_init_eeprom_params(struct e1000_hw *hw)
{
	struct e1000_eeprom_info *eeprom = &hw->eeprom;
	uint32_t eecd;
	int32_t ret_val = E1000_SUCCESS;
	uint16_t eeprom_size;

	if (hw->mac_type == e1000_igb)
		eecd = E1000_READ_REG(hw, I210_EECD);
	else
		eecd = E1000_READ_REG(hw, EECD);

	DEBUGFUNC();

	switch (hw->mac_type) {
	case e1000_82542_rev2_0:
	case e1000_82542_rev2_1:
	case e1000_82543:
	case e1000_82544:
		eeprom->type = e1000_eeprom_microwire;
		eeprom->word_size = 64;
		eeprom->opcode_bits = 3;
		eeprom->address_bits = 6;
		eeprom->delay_usec = 50;
		eeprom->use_eerd = false;
		eeprom->use_eewr = false;
	break;
	case e1000_82540:
	case e1000_82545:
	case e1000_82545_rev_3:
	case e1000_82546:
	case e1000_82546_rev_3:
		eeprom->type = e1000_eeprom_microwire;
		eeprom->opcode_bits = 3;
		eeprom->delay_usec = 50;
		if (eecd & E1000_EECD_SIZE) {
			eeprom->word_size = 256;
			eeprom->address_bits = 8;
		} else {
			eeprom->word_size = 64;
			eeprom->address_bits = 6;
		}
		eeprom->use_eerd = false;
		eeprom->use_eewr = false;
		break;
	case e1000_82541:
	case e1000_82541_rev_2:
	case e1000_82547:
	case e1000_82547_rev_2:
		if (eecd & E1000_EECD_TYPE) {
			eeprom->type = e1000_eeprom_spi;
			eeprom->opcode_bits = 8;
			eeprom->delay_usec = 1;
			if (eecd & E1000_EECD_ADDR_BITS) {
				eeprom->page_size = 32;
				eeprom->address_bits = 16;
			} else {
				eeprom->page_size = 8;
				eeprom->address_bits = 8;
			}
		} else {
			eeprom->type = e1000_eeprom_microwire;
			eeprom->opcode_bits = 3;
			eeprom->delay_usec = 50;
			if (eecd & E1000_EECD_ADDR_BITS) {
				eeprom->word_size = 256;
				eeprom->address_bits = 8;
			} else {
				eeprom->word_size = 64;
				eeprom->address_bits = 6;
			}
		}
		eeprom->use_eerd = false;
		eeprom->use_eewr = false;
		break;
	case e1000_82571:
	case e1000_82572:
		eeprom->type = e1000_eeprom_spi;
		eeprom->opcode_bits = 8;
		eeprom->delay_usec = 1;
		if (eecd & E1000_EECD_ADDR_BITS) {
			eeprom->page_size = 32;
			eeprom->address_bits = 16;
		} else {
			eeprom->page_size = 8;
			eeprom->address_bits = 8;
		}
		eeprom->use_eerd = false;
		eeprom->use_eewr = false;
		break;
	case e1000_82573:
	case e1000_82574:
		eeprom->type = e1000_eeprom_spi;
		eeprom->opcode_bits = 8;
		eeprom->delay_usec = 1;
		if (eecd & E1000_EECD_ADDR_BITS) {
			eeprom->page_size = 32;
			eeprom->address_bits = 16;
		} else {
			eeprom->page_size = 8;
			eeprom->address_bits = 8;
		}
		if (e1000_is_onboard_nvm_eeprom(hw) == false) {
			eeprom->use_eerd = true;
			eeprom->use_eewr = true;

			eeprom->type = e1000_eeprom_flash;
			eeprom->word_size = 2048;

		/* Ensure that the Autonomous FLASH update bit is cleared due to
		 * Flash update issue on parts which use a FLASH for NVM. */
			eecd &= ~E1000_EECD_AUPDEN;
			E1000_WRITE_REG(hw, EECD, eecd);
		}
		break;
	case e1000_80003es2lan:
		eeprom->type = e1000_eeprom_spi;
		eeprom->opcode_bits = 8;
		eeprom->delay_usec = 1;
		if (eecd & E1000_EECD_ADDR_BITS) {
			eeprom->page_size = 32;
			eeprom->address_bits = 16;
		} else {
			eeprom->page_size = 8;
			eeprom->address_bits = 8;
		}
		eeprom->use_eerd = true;
		eeprom->use_eewr = false;
		break;
	case e1000_igb:
		/* i210 has 4k of iNVM mapped as EEPROM */
		eeprom->type = e1000_eeprom_invm;
		eeprom->opcode_bits = 8;
		eeprom->delay_usec = 1;
		eeprom->page_size = 32;
		eeprom->address_bits = 16;
		eeprom->use_eerd = true;
		eeprom->use_eewr = false;
		break;
	default:
		break;
	}

	if (eeprom->type == e1000_eeprom_spi ||
	    eeprom->type == e1000_eeprom_invm) {
		/* eeprom_size will be an enum [0..8] that maps
		 * to eeprom sizes 128B to
		 * 32KB (incremented by powers of 2).
		 */
		if (hw->mac_type <= e1000_82547_rev_2) {
			/* Set to default value for initial eeprom read. */
			eeprom->word_size = 64;
			ret_val = e1000_read_eeprom(hw, EEPROM_CFG, 1,
					&eeprom_size);
			if (ret_val)
				return ret_val;
			eeprom_size = (eeprom_size & EEPROM_SIZE_MASK)
				>> EEPROM_SIZE_SHIFT;
			/* 256B eeprom size was not supported in earlier
			 * hardware, so we bump eeprom_size up one to
			 * ensure that "1" (which maps to 256B) is never
			 * the result used in the shifting logic below. */
			if (eeprom_size)
				eeprom_size++;
		} else {
			eeprom_size = (uint16_t)((eecd &
				E1000_EECD_SIZE_EX_MASK) >>
				E1000_EECD_SIZE_EX_SHIFT);
		}

		eeprom->word_size = 1 << (eeprom_size + EEPROM_WORD_SIZE_SHIFT);
	}
	return ret_val;
}

/******************************************************************************
 * Polls the status bit (bit 1) of the EERD to determine when the read is done.
 *
 * hw - Struct containing variables accessed by shared code
 *****************************************************************************/
static int32_t
e1000_poll_eerd_eewr_done(struct e1000_hw *hw, int eerd)
{
	uint32_t attempts = 100000;
	uint32_t i, reg = 0;
	int32_t done = E1000_ERR_EEPROM;

	for (i = 0; i < attempts; i++) {
		if (eerd == E1000_EEPROM_POLL_READ) {
			if (hw->mac_type == e1000_igb)
				reg = E1000_READ_REG(hw, I210_EERD);
			else
				reg = E1000_READ_REG(hw, EERD);
		} else {
			if (hw->mac_type == e1000_igb)
				reg = E1000_READ_REG(hw, I210_EEWR);
			else
				reg = E1000_READ_REG(hw, EEWR);
		}

		if (reg & E1000_EEPROM_RW_REG_DONE) {
			done = E1000_SUCCESS;
			break;
		}
		udelay(5);
	}

	return done;
}

/******************************************************************************
 * Reads a 16 bit word from the EEPROM using the EERD register.
 *
 * hw - Struct containing variables accessed by shared code
 * offset - offset of  word in the EEPROM to read
 * data - word read from the EEPROM
 * words - number of words to read
 *****************************************************************************/
static int32_t
e1000_read_eeprom_eerd(struct e1000_hw *hw,
			uint16_t offset,
			uint16_t words,
			uint16_t *data)
{
	uint32_t i, eerd = 0;
	int32_t error = 0;

	for (i = 0; i < words; i++) {
		eerd = ((offset+i) << E1000_EEPROM_RW_ADDR_SHIFT) +
			E1000_EEPROM_RW_REG_START;

		if (hw->mac_type == e1000_igb)
			E1000_WRITE_REG(hw, I210_EERD, eerd);
		else
			E1000_WRITE_REG(hw, EERD, eerd);

		error = e1000_poll_eerd_eewr_done(hw, E1000_EEPROM_POLL_READ);

		if (error)
			break;

		if (hw->mac_type == e1000_igb) {
			data[i] = (E1000_READ_REG(hw, I210_EERD) >>
				E1000_EEPROM_RW_REG_DATA);
		} else {
			data[i] = (E1000_READ_REG(hw, EERD) >>
				E1000_EEPROM_RW_REG_DATA);
		}

	}

	return error;
}

void e1000_release_eeprom(struct e1000_hw *hw)
{
	uint32_t eecd;

	DEBUGFUNC();

	eecd = E1000_READ_REG(hw, EECD);

	if (hw->eeprom.type == e1000_eeprom_spi) {
		eecd |= E1000_EECD_CS;  /* Pull CS high */
		eecd &= ~E1000_EECD_SK; /* Lower SCK */

		E1000_WRITE_REG(hw, EECD, eecd);

		udelay(hw->eeprom.delay_usec);
	} else if (hw->eeprom.type == e1000_eeprom_microwire) {
		/* cleanup eeprom */

		/* CS on Microwire is active-high */
		eecd &= ~(E1000_EECD_CS | E1000_EECD_DI);

		E1000_WRITE_REG(hw, EECD, eecd);

		/* Rising edge of clock */
		eecd |= E1000_EECD_SK;
		E1000_WRITE_REG(hw, EECD, eecd);
		E1000_WRITE_FLUSH(hw);
		udelay(hw->eeprom.delay_usec);

		/* Falling edge of clock */
		eecd &= ~E1000_EECD_SK;
		E1000_WRITE_REG(hw, EECD, eecd);
		E1000_WRITE_FLUSH(hw);
		udelay(hw->eeprom.delay_usec);
	}

	/* Stop requesting EEPROM access */
	if (hw->mac_type > e1000_82544) {
		eecd &= ~E1000_EECD_REQ;
		E1000_WRITE_REG(hw, EECD, eecd);
	}

	e1000_swfw_sync_release(hw, E1000_SWFW_EEP_SM);
}

/******************************************************************************
 * Reads a 16 bit word from the EEPROM.
 *
 * hw - Struct containing variables accessed by shared code
 *****************************************************************************/
static int32_t
e1000_spi_eeprom_ready(struct e1000_hw *hw)
{
	uint16_t retry_count = 0;
	uint8_t spi_stat_reg;

	DEBUGFUNC();

	/* Read "Status Register" repeatedly until the LSB is cleared.  The
	 * EEPROM will signal that the command has been completed by clearing
	 * bit 0 of the internal status register.  If it's not cleared within
	 * 5 milliseconds, then error out.
	 */
	retry_count = 0;
	do {
		e1000_shift_out_ee_bits(hw, EEPROM_RDSR_OPCODE_SPI,
			hw->eeprom.opcode_bits);
		spi_stat_reg = (uint8_t)e1000_shift_in_ee_bits(hw, 8);
		if (!(spi_stat_reg & EEPROM_STATUS_RDY_SPI))
			break;

		udelay(5);
		retry_count += 5;

		e1000_standby_eeprom(hw);
	} while (retry_count < EEPROM_MAX_RETRY_SPI);

	/* ATMEL SPI write time could vary from 0-20mSec on 3.3V devices (and
	 * only 0-5mSec on 5V devices)
	 */
	if (retry_count >= EEPROM_MAX_RETRY_SPI) {
		DEBUGOUT("SPI EEPROM Status error\n");
		return -E1000_ERR_EEPROM;
	}

	return E1000_SUCCESS;
}

/******************************************************************************
 * Reads a 16 bit word from the EEPROM.
 *
 * hw - Struct containing variables accessed by shared code
 * offset - offset of  word in the EEPROM to read
 * data - word read from the EEPROM
 *****************************************************************************/
static int32_t
e1000_read_eeprom(struct e1000_hw *hw, uint16_t offset,
		uint16_t words, uint16_t *data)
{
	struct e1000_eeprom_info *eeprom = &hw->eeprom;
	uint32_t i = 0;

	DEBUGFUNC();

	/* If eeprom is not yet detected, do so now */
	if (eeprom->word_size == 0)
		e1000_init_eeprom_params(hw);

	/* A check for invalid values:  offset too large, too many words,
	 * and not enough words.
	 */
	if ((offset >= eeprom->word_size) ||
		(words > eeprom->word_size - offset) ||
		(words == 0)) {
		DEBUGOUT("\"words\" parameter out of bounds."
			"Words = %d, size = %d\n", offset, eeprom->word_size);
		return -E1000_ERR_EEPROM;
	}

	/* EEPROM's that don't use EERD to read require us to bit-bang the SPI
	 * directly. In this case, we need to acquire the EEPROM so that
	 * FW or other port software does not interrupt.
	 */
	if (e1000_is_onboard_nvm_eeprom(hw) == true &&
		hw->eeprom.use_eerd == false) {

		/* Prepare the EEPROM for bit-bang reading */
		if (e1000_acquire_eeprom(hw) != E1000_SUCCESS)
			return -E1000_ERR_EEPROM;
	}

	/* Eerd register EEPROM access requires no eeprom aquire/release */
	if (eeprom->use_eerd == true)
		return e1000_read_eeprom_eerd(hw, offset, words, data);

	/* Set up the SPI or Microwire EEPROM for bit-bang reading.  We have
	 * acquired the EEPROM at this point, so any returns should relase it */
	if (eeprom->type == e1000_eeprom_spi) {
		uint16_t word_in;
		uint8_t read_opcode = EEPROM_READ_OPCODE_SPI;

		if (e1000_spi_eeprom_ready(hw)) {
			e1000_release_eeprom(hw);
			return -E1000_ERR_EEPROM;
		}

		e1000_standby_eeprom(hw);

		/* Some SPI eeproms use the 8th address bit embedded in
		 * the opcode */
		if ((eeprom->address_bits == 8) && (offset >= 128))
			read_opcode |= EEPROM_A8_OPCODE_SPI;

		/* Send the READ command (opcode + addr)  */
		e1000_shift_out_ee_bits(hw, read_opcode, eeprom->opcode_bits);
		e1000_shift_out_ee_bits(hw, (uint16_t)(offset*2),
				eeprom->address_bits);

		/* Read the data.  The address of the eeprom internally
		 * increments with each byte (spi) being read, saving on the
		 * overhead of eeprom setup and tear-down.  The address
		 * counter will roll over if reading beyond the size of
		 * the eeprom, thus allowing the entire memory to be read
		 * starting from any offset. */
		for (i = 0; i < words; i++) {
			word_in = e1000_shift_in_ee_bits(hw, 16);
			data[i] = (word_in >> 8) | (word_in << 8);
		}
	} else if (eeprom->type == e1000_eeprom_microwire) {
		for (i = 0; i < words; i++) {
			/* Send the READ command (opcode + addr)  */
			e1000_shift_out_ee_bits(hw,
				EEPROM_READ_OPCODE_MICROWIRE,
				eeprom->opcode_bits);
			e1000_shift_out_ee_bits(hw, (uint16_t)(offset + i),
				eeprom->address_bits);

			/* Read the data.  For microwire, each word requires
			 * the overhead of eeprom setup and tear-down. */
			data[i] = e1000_shift_in_ee_bits(hw, 16);
			e1000_standby_eeprom(hw);
		}
	}

	/* End this read operation */
	e1000_release_eeprom(hw);

	return E1000_SUCCESS;
}

#ifndef CONFIG_DM_ETH
/******************************************************************************
 *  e1000_write_eeprom_srwr - Write to Shadow Ram using EEWR
 *  @hw: pointer to the HW structure
 *  @offset: offset within the Shadow Ram to be written to
 *  @words: number of words to write
 *  @data: 16 bit word(s) to be written to the Shadow Ram
 *
 *  Writes data to Shadow Ram at offset using EEWR register.
 *
 *  If e1000_update_eeprom_checksum_i210 is not called after this function, the
 *  Shadow Ram will most likely contain an invalid checksum.
 *****************************************************************************/
static int32_t e1000_write_eeprom_srwr(struct e1000_hw *hw, uint16_t offset,
				       uint16_t words, uint16_t *data)
{
	struct e1000_eeprom_info *eeprom = &hw->eeprom;
	uint32_t i, k, eewr = 0;
	uint32_t attempts = 100000;
	int32_t ret_val = 0;

	/* A check for invalid values:  offset too large, too many words,
	 * too many words for the offset, and not enough words.
	 */
	if ((offset >= eeprom->word_size) ||
	    (words > (eeprom->word_size - offset)) || (words == 0)) {
		DEBUGOUT("nvm parameter(s) out of bounds\n");
		ret_val = -E1000_ERR_EEPROM;
		goto out;
	}

	for (i = 0; i < words; i++) {
		eewr = ((offset + i) << E1000_EEPROM_RW_ADDR_SHIFT)
				| (data[i] << E1000_EEPROM_RW_REG_DATA) |
				E1000_EEPROM_RW_REG_START;

		E1000_WRITE_REG(hw, I210_EEWR, eewr);

		for (k = 0; k < attempts; k++) {
			if (E1000_EEPROM_RW_REG_DONE &
			    E1000_READ_REG(hw, I210_EEWR)) {
				ret_val = 0;
				break;
			}
			udelay(5);
		}

		if (ret_val) {
			DEBUGOUT("Shadow RAM write EEWR timed out\n");
			break;
		}
	}

out:
	return ret_val;
}

/******************************************************************************
 *  e1000_pool_flash_update_done_i210 - Pool FLUDONE status.
 *  @hw: pointer to the HW structure
 *
 *****************************************************************************/
static int32_t e1000_pool_flash_update_done_i210(struct e1000_hw *hw)
{
	int32_t ret_val = -E1000_ERR_EEPROM;
	uint32_t i, reg;

	for (i = 0; i < E1000_FLUDONE_ATTEMPTS; i++) {
		reg = E1000_READ_REG(hw, EECD);
		if (reg & E1000_EECD_FLUDONE_I210) {
			ret_val = 0;
			break;
		}
		udelay(5);
	}

	return ret_val;
}

/******************************************************************************
 *  e1000_update_flash_i210 - Commit EEPROM to the flash
 *  @hw: pointer to the HW structure
 *
 *****************************************************************************/
static int32_t e1000_update_flash_i210(struct e1000_hw *hw)
{
	int32_t ret_val = 0;
	uint32_t flup;

	ret_val = e1000_pool_flash_update_done_i210(hw);
	if (ret_val == -E1000_ERR_EEPROM) {
		DEBUGOUT("Flash update time out\n");
		goto out;
	}

	flup = E1000_READ_REG(hw, EECD) | E1000_EECD_FLUPD_I210;
	E1000_WRITE_REG(hw, EECD, flup);

	ret_val = e1000_pool_flash_update_done_i210(hw);
	if (ret_val)
		DEBUGOUT("Flash update time out\n");
	else
		DEBUGOUT("Flash update complete\n");

out:
	return ret_val;
}

/******************************************************************************
 *  e1000_update_eeprom_checksum_i210 - Update EEPROM checksum
 *  @hw: pointer to the HW structure
 *
 *  Updates the EEPROM checksum by reading/adding each word of the EEPROM
 *  up to the checksum.  Then calculates the EEPROM checksum and writes the
 *  value to the EEPROM. Next commit EEPROM data onto the Flash.
 *****************************************************************************/
static int32_t e1000_update_eeprom_checksum_i210(struct e1000_hw *hw)
{
	int32_t ret_val = 0;
	uint16_t checksum = 0;
	uint16_t i, nvm_data;

	/* Read the first word from the EEPROM. If this times out or fails, do
	 * not continue or we could be in for a very long wait while every
	 * EEPROM read fails
	 */
	ret_val = e1000_read_eeprom_eerd(hw, 0, 1, &nvm_data);
	if (ret_val) {
		DEBUGOUT("EEPROM read failed\n");
		goto out;
	}

	if (!(e1000_get_hw_eeprom_semaphore(hw))) {
		/* Do not use hw->nvm.ops.write, hw->nvm.ops.read
		 * because we do not want to take the synchronization
		 * semaphores twice here.
		 */

		for (i = 0; i < EEPROM_CHECKSUM_REG; i++) {
			ret_val = e1000_read_eeprom_eerd(hw, i, 1, &nvm_data);
			if (ret_val) {
				e1000_put_hw_eeprom_semaphore(hw);
				DEBUGOUT("EEPROM Read Error while updating checksum.\n");
				goto out;
			}
			checksum += nvm_data;
		}
		checksum = (uint16_t)EEPROM_SUM - checksum;
		ret_val = e1000_write_eeprom_srwr(hw, EEPROM_CHECKSUM_REG, 1,
						  &checksum);
		if (ret_val) {
			e1000_put_hw_eeprom_semaphore(hw);
			DEBUGOUT("EEPROM Write Error while updating checksum.\n");
			goto out;
		}

		e1000_put_hw_eeprom_semaphore(hw);

		ret_val = e1000_update_flash_i210(hw);
	} else {
		ret_val = -E1000_ERR_SWFW_SYNC;
	}

out:
	return ret_val;
}
#endif

/******************************************************************************
 * Verifies that the EEPROM has a valid checksum
 *
 * hw - Struct containing variables accessed by shared code
 *
 * Reads the first 64 16 bit words of the EEPROM and sums the values read.
 * If the the sum of the 64 16 bit words is 0xBABA, the EEPROM's checksum is
 * valid.
 *****************************************************************************/
static int e1000_validate_eeprom_checksum(struct e1000_hw *hw)
{
	uint16_t i, checksum, checksum_reg, *buf;

	DEBUGFUNC();

	/* Allocate a temporary buffer */
	buf = malloc(sizeof(buf[0]) * (EEPROM_CHECKSUM_REG + 1));
	if (!buf) {
		E1000_ERR(hw, "Unable to allocate EEPROM buffer!\n");
		return -E1000_ERR_EEPROM;
	}

	/* Read the EEPROM */
	if (e1000_read_eeprom(hw, 0, EEPROM_CHECKSUM_REG + 1, buf) < 0) {
		E1000_ERR(hw, "Unable to read EEPROM!\n");
		return -E1000_ERR_EEPROM;
	}

	/* Compute the checksum */
	checksum = 0;
	for (i = 0; i < EEPROM_CHECKSUM_REG; i++)
		checksum += buf[i];
	checksum = ((uint16_t)EEPROM_SUM) - checksum;
	checksum_reg = buf[i];

	/* Verify it! */
	if (checksum == checksum_reg)
		return 0;

	/* Hrm, verification failed, print an error */
	E1000_ERR(hw, "EEPROM checksum is incorrect!\n");
	E1000_ERR(hw, "  ...register was 0x%04hx, calculated 0x%04hx\n",
		  checksum_reg, checksum);

	return -E1000_ERR_EEPROM;
}
#endif /* CONFIG_E1000_NO_NVM */

/*****************************************************************************
 * Set PHY to class A mode
 * Assumes the following operations will follow to enable the new class mode.
 *  1. Do a PHY soft reset
 *  2. Restart auto-negotiation or force link.
 *
 * hw - Struct containing variables accessed by shared code
 ****************************************************************************/
static int32_t
e1000_set_phy_mode(struct e1000_hw *hw)
{
#ifndef CONFIG_E1000_NO_NVM
	int32_t ret_val;
	uint16_t eeprom_data;

	DEBUGFUNC();

	if ((hw->mac_type == e1000_82545_rev_3) &&
		(hw->media_type == e1000_media_type_copper)) {
		ret_val = e1000_read_eeprom(hw, EEPROM_PHY_CLASS_WORD,
				1, &eeprom_data);
		if (ret_val)
			return ret_val;

		if ((eeprom_data != EEPROM_RESERVED_WORD) &&
			(eeprom_data & EEPROM_PHY_CLASS_A)) {
			ret_val = e1000_write_phy_reg(hw,
					M88E1000_PHY_PAGE_SELECT, 0x000B);
			if (ret_val)
				return ret_val;
			ret_val = e1000_write_phy_reg(hw,
					M88E1000_PHY_GEN_CONTROL, 0x8104);
			if (ret_val)
				return ret_val;

			hw->phy_reset_disable = false;
		}
	}
#endif
	return E1000_SUCCESS;
}

#ifndef CONFIG_E1000_NO_NVM
/***************************************************************************
 *
 * Obtaining software semaphore bit (SMBI) before resetting PHY.
 *
 * hw: Struct containing variables accessed by shared code
 *
 * returns: - E1000_ERR_RESET if fail to obtain semaphore.
 *            E1000_SUCCESS at any other case.
 *
 ***************************************************************************/
static int32_t
e1000_get_software_semaphore(struct e1000_hw *hw)
{
	 int32_t timeout = hw->eeprom.word_size + 1;
	 uint32_t swsm;

	DEBUGFUNC();

	if (hw->mac_type != e1000_80003es2lan && hw->mac_type != e1000_igb)
		return E1000_SUCCESS;

	while (timeout) {
		swsm = E1000_READ_REG(hw, SWSM);
		/* If SMBI bit cleared, it is now set and we hold
		 * the semaphore */
		if (!(swsm & E1000_SWSM_SMBI))
			break;
		mdelay(1);
		timeout--;
	}

	if (!timeout) {
		DEBUGOUT("Driver can't access device - SMBI bit is set.\n");
		return -E1000_ERR_RESET;
	}

	return E1000_SUCCESS;
}
#endif

/***************************************************************************
 * This function clears HW semaphore bits.
 *
 * hw: Struct containing variables accessed by shared code
 *
 * returns: - None.
 *
 ***************************************************************************/
static void
e1000_put_hw_eeprom_semaphore(struct e1000_hw *hw)
{
#ifndef CONFIG_E1000_NO_NVM
	 uint32_t swsm;

	DEBUGFUNC();

	if (!hw->eeprom_semaphore_present)
		return;

	swsm = E1000_READ_REG(hw, SWSM);
	if (hw->mac_type == e1000_80003es2lan || hw->mac_type == e1000_igb) {
		/* Release both semaphores. */
		swsm &= ~(E1000_SWSM_SMBI | E1000_SWSM_SWESMBI);
	} else
		swsm &= ~(E1000_SWSM_SWESMBI);
	E1000_WRITE_REG(hw, SWSM, swsm);
#endif
}

/***************************************************************************
 *
 * Using the combination of SMBI and SWESMBI semaphore bits when resetting
 * adapter or Eeprom access.
 *
 * hw: Struct containing variables accessed by shared code
 *
 * returns: - E1000_ERR_EEPROM if fail to access EEPROM.
 *            E1000_SUCCESS at any other case.
 *
 ***************************************************************************/
static int32_t
e1000_get_hw_eeprom_semaphore(struct e1000_hw *hw)
{
#ifndef CONFIG_E1000_NO_NVM
	int32_t timeout;
	uint32_t swsm;

	DEBUGFUNC();

	if (!hw->eeprom_semaphore_present)
		return E1000_SUCCESS;

	if (hw->mac_type == e1000_80003es2lan || hw->mac_type == e1000_igb) {
		/* Get the SW semaphore. */
		if (e1000_get_software_semaphore(hw) != E1000_SUCCESS)
			return -E1000_ERR_EEPROM;
	}

	/* Get the FW semaphore. */
	timeout = hw->eeprom.word_size + 1;
	while (timeout) {
		swsm = E1000_READ_REG(hw, SWSM);
		swsm |= E1000_SWSM_SWESMBI;
		E1000_WRITE_REG(hw, SWSM, swsm);
		/* if we managed to set the bit we got the semaphore. */
		swsm = E1000_READ_REG(hw, SWSM);
		if (swsm & E1000_SWSM_SWESMBI)
			break;

		udelay(50);
		timeout--;
	}

	if (!timeout) {
		/* Release semaphores */
		e1000_put_hw_eeprom_semaphore(hw);
		DEBUGOUT("Driver can't access the Eeprom - "
				"SWESMBI bit is set.\n");
		return -E1000_ERR_EEPROM;
	}
#endif
	return E1000_SUCCESS;
}

/* Take ownership of the PHY */
static int32_t
e1000_swfw_sync_acquire(struct e1000_hw *hw, uint16_t mask)
{
	uint32_t swfw_sync = 0;
	uint32_t swmask = mask;
	uint32_t fwmask = mask << 16;
	int32_t timeout = 200;

	DEBUGFUNC();
	while (timeout) {
		if (e1000_get_hw_eeprom_semaphore(hw))
			return -E1000_ERR_SWFW_SYNC;

		swfw_sync = E1000_READ_REG(hw, SW_FW_SYNC);
		if (!(swfw_sync & (fwmask | swmask)))
			break;

		/* firmware currently using resource (fwmask) */
		/* or other software thread currently using resource (swmask) */
		e1000_put_hw_eeprom_semaphore(hw);
		mdelay(5);
		timeout--;
	}

	if (!timeout) {
		DEBUGOUT("Driver can't access resource, SW_FW_SYNC timeout.\n");
		return -E1000_ERR_SWFW_SYNC;
	}

	swfw_sync |= swmask;
	E1000_WRITE_REG(hw, SW_FW_SYNC, swfw_sync);

	e1000_put_hw_eeprom_semaphore(hw);
	return E1000_SUCCESS;
}

static void e1000_swfw_sync_release(struct e1000_hw *hw, uint16_t mask)
{
	uint32_t swfw_sync = 0;

	DEBUGFUNC();
	while (e1000_get_hw_eeprom_semaphore(hw))
		; /* Empty */

	swfw_sync = E1000_READ_REG(hw, SW_FW_SYNC);
	swfw_sync &= ~mask;
	E1000_WRITE_REG(hw, SW_FW_SYNC, swfw_sync);

	e1000_put_hw_eeprom_semaphore(hw);
}

static bool e1000_is_second_port(struct e1000_hw *hw)
{
	switch (hw->mac_type) {
	case e1000_80003es2lan:
	case e1000_82546:
	case e1000_82571:
		if (E1000_READ_REG(hw, STATUS) & E1000_STATUS_FUNC_1)
			return true;
		/* Fallthrough */
	default:
		return false;
	}
}

#ifndef CONFIG_E1000_NO_NVM
/******************************************************************************
 * Reads the adapter's MAC address from the EEPROM
 *
 * hw - Struct containing variables accessed by shared code
 * enetaddr - buffering where the MAC address will be stored
 *****************************************************************************/
static int e1000_read_mac_addr_from_eeprom(struct e1000_hw *hw,
					   unsigned char enetaddr[6])
{
	uint16_t offset;
	uint16_t eeprom_data;
	int i;

	for (i = 0; i < NODE_ADDRESS_SIZE; i += 2) {
		offset = i >> 1;
		if (e1000_read_eeprom(hw, offset, 1, &eeprom_data) < 0) {
			DEBUGOUT("EEPROM Read Error\n");
			return -E1000_ERR_EEPROM;
		}
		enetaddr[i] = eeprom_data & 0xff;
		enetaddr[i + 1] = (eeprom_data >> 8) & 0xff;
	}

	return 0;
}

/******************************************************************************
 * Reads the adapter's MAC address from the RAL/RAH registers
 *
 * hw - Struct containing variables accessed by shared code
 * enetaddr - buffering where the MAC address will be stored
 *****************************************************************************/
static int e1000_read_mac_addr_from_regs(struct e1000_hw *hw,
					 unsigned char enetaddr[6])
{
	uint16_t offset, tmp;
	uint32_t reg_data = 0;
	int i;

	if (hw->mac_type != e1000_igb)
		return -E1000_ERR_MAC_TYPE;

	for (i = 0; i < NODE_ADDRESS_SIZE; i += 2) {
		offset = i >> 1;

		if (offset == 0)
			reg_data = E1000_READ_REG_ARRAY(hw, RA, 0);
		else if (offset == 1)
			reg_data >>= 16;
		else if (offset == 2)
			reg_data = E1000_READ_REG_ARRAY(hw, RA, 1);
		tmp = reg_data & 0xffff;

		enetaddr[i] = tmp & 0xff;
		enetaddr[i + 1] = (tmp >> 8) & 0xff;
	}

	return 0;
}

/******************************************************************************
 * Reads the adapter's MAC address from the EEPROM and inverts the LSB for the
 * second function of dual function devices
 *
 * hw - Struct containing variables accessed by shared code
 * enetaddr - buffering where the MAC address will be stored
 *****************************************************************************/
static int e1000_read_mac_addr(struct e1000_hw *hw, unsigned char enetaddr[6])
{
	int ret_val;

	if (hw->mac_type == e1000_igb) {
		/* i210 preloads MAC address into RAL/RAH registers */
		ret_val = e1000_read_mac_addr_from_regs(hw, enetaddr);
	} else {
		ret_val = e1000_read_mac_addr_from_eeprom(hw, enetaddr);
	}
	if (ret_val)
		return ret_val;

	/* Invert the last bit if this is the second device */
	if (e1000_is_second_port(hw))
		enetaddr[5] ^= 1;

	return 0;
}
#endif

/******************************************************************************
 * Initializes receive address filters.
 *
 * hw - Struct containing variables accessed by shared code
 *
 * Places the MAC address in receive address register 0 and clears the rest
 * of the receive addresss registers. Clears the multicast table. Assumes
 * the receiver is in reset when the routine is called.
 *****************************************************************************/
static void
e1000_init_rx_addrs(struct e1000_hw *hw, unsigned char enetaddr[6])
{
	uint32_t i;
	uint32_t addr_low;
	uint32_t addr_high;

	DEBUGFUNC();

	/* Setup the receive address. */
	DEBUGOUT("Programming MAC Address into RAR[0]\n");
	addr_low = (enetaddr[0] |
		    (enetaddr[1] << 8) |
		    (enetaddr[2] << 16) | (enetaddr[3] << 24));

	addr_high = (enetaddr[4] | (enetaddr[5] << 8) | E1000_RAH_AV);

	E1000_WRITE_REG_ARRAY(hw, RA, 0, addr_low);
	E1000_WRITE_REG_ARRAY(hw, RA, 1, addr_high);

	/* Zero out the other 15 receive addresses. */
	DEBUGOUT("Clearing RAR[1-15]\n");
	for (i = 1; i < E1000_RAR_ENTRIES; i++) {
		E1000_WRITE_REG_ARRAY(hw, RA, (i << 1), 0);
		E1000_WRITE_REG_ARRAY(hw, RA, ((i << 1) + 1), 0);
	}
}

/******************************************************************************
 * Clears the VLAN filer table
 *
 * hw - Struct containing variables accessed by shared code
 *****************************************************************************/
static void
e1000_clear_vfta(struct e1000_hw *hw)
{
	uint32_t offset;

	for (offset = 0; offset < E1000_VLAN_FILTER_TBL_SIZE; offset++)
		E1000_WRITE_REG_ARRAY(hw, VFTA, offset, 0);
}

/******************************************************************************
 * Set the mac type member in the hw struct.
 *
 * hw - Struct containing variables accessed by shared code
 *****************************************************************************/
int32_t
e1000_set_mac_type(struct e1000_hw *hw)
{
	DEBUGFUNC();

	switch (hw->device_id) {
	case E1000_DEV_ID_82542:
		switch (hw->revision_id) {
		case E1000_82542_2_0_REV_ID:
			hw->mac_type = e1000_82542_rev2_0;
			break;
		case E1000_82542_2_1_REV_ID:
			hw->mac_type = e1000_82542_rev2_1;
			break;
		default:
			/* Invalid 82542 revision ID */
			return -E1000_ERR_MAC_TYPE;
		}
		break;
	case E1000_DEV_ID_82543GC_FIBER:
	case E1000_DEV_ID_82543GC_COPPER:
		hw->mac_type = e1000_82543;
		break;
	case E1000_DEV_ID_82544EI_COPPER:
	case E1000_DEV_ID_82544EI_FIBER:
	case E1000_DEV_ID_82544GC_COPPER:
	case E1000_DEV_ID_82544GC_LOM:
		hw->mac_type = e1000_82544;
		break;
	case E1000_DEV_ID_82540EM:
	case E1000_DEV_ID_82540EM_LOM:
	case E1000_DEV_ID_82540EP:
	case E1000_DEV_ID_82540EP_LOM:
	case E1000_DEV_ID_82540EP_LP:
		hw->mac_type = e1000_82540;
		break;
	case E1000_DEV_ID_82545EM_COPPER:
	case E1000_DEV_ID_82545EM_FIBER:
		hw->mac_type = e1000_82545;
		break;
	case E1000_DEV_ID_82545GM_COPPER:
	case E1000_DEV_ID_82545GM_FIBER:
	case E1000_DEV_ID_82545GM_SERDES:
		hw->mac_type = e1000_82545_rev_3;
		break;
	case E1000_DEV_ID_82546EB_COPPER:
	case E1000_DEV_ID_82546EB_FIBER:
	case E1000_DEV_ID_82546EB_QUAD_COPPER:
		hw->mac_type = e1000_82546;
		break;
	case E1000_DEV_ID_82546GB_COPPER:
	case E1000_DEV_ID_82546GB_FIBER:
	case E1000_DEV_ID_82546GB_SERDES:
	case E1000_DEV_ID_82546GB_PCIE:
	case E1000_DEV_ID_82546GB_QUAD_COPPER:
	case E1000_DEV_ID_82546GB_QUAD_COPPER_KSP3:
		hw->mac_type = e1000_82546_rev_3;
		break;
	case E1000_DEV_ID_82541EI:
	case E1000_DEV_ID_82541EI_MOBILE:
	case E1000_DEV_ID_82541ER_LOM:
		hw->mac_type = e1000_82541;
		break;
	case E1000_DEV_ID_82541ER:
	case E1000_DEV_ID_82541GI:
	case E1000_DEV_ID_82541GI_LF:
	case E1000_DEV_ID_82541GI_MOBILE:
		hw->mac_type = e1000_82541_rev_2;
		break;
	case E1000_DEV_ID_82547EI:
	case E1000_DEV_ID_82547EI_MOBILE:
		hw->mac_type = e1000_82547;
		break;
	case E1000_DEV_ID_82547GI:
		hw->mac_type = e1000_82547_rev_2;
		break;
	case E1000_DEV_ID_82571EB_COPPER:
	case E1000_DEV_ID_82571EB_FIBER:
	case E1000_DEV_ID_82571EB_SERDES:
	case E1000_DEV_ID_82571EB_SERDES_DUAL:
	case E1000_DEV_ID_82571EB_SERDES_QUAD:
	case E1000_DEV_ID_82571EB_QUAD_COPPER:
	case E1000_DEV_ID_82571PT_QUAD_COPPER:
	case E1000_DEV_ID_82571EB_QUAD_FIBER:
	case E1000_DEV_ID_82571EB_QUAD_COPPER_LOWPROFILE:
		hw->mac_type = e1000_82571;
		break;
	case E1000_DEV_ID_82572EI_COPPER:
	case E1000_DEV_ID_82572EI_FIBER:
	case E1000_DEV_ID_82572EI_SERDES:
	case E1000_DEV_ID_82572EI:
		hw->mac_type = e1000_82572;
		break;
	case E1000_DEV_ID_82573E:
	case E1000_DEV_ID_82573E_IAMT:
	case E1000_DEV_ID_82573L:
		hw->mac_type = e1000_82573;
		break;
	case E1000_DEV_ID_82574L:
		hw->mac_type = e1000_82574;
		break;
	case E1000_DEV_ID_80003ES2LAN_COPPER_SPT:
	case E1000_DEV_ID_80003ES2LAN_SERDES_SPT:
	case E1000_DEV_ID_80003ES2LAN_COPPER_DPT:
	case E1000_DEV_ID_80003ES2LAN_SERDES_DPT:
		hw->mac_type = e1000_80003es2lan;
		break;
	case E1000_DEV_ID_ICH8_IGP_M_AMT:
	case E1000_DEV_ID_ICH8_IGP_AMT:
	case E1000_DEV_ID_ICH8_IGP_C:
	case E1000_DEV_ID_ICH8_IFE:
	case E1000_DEV_ID_ICH8_IFE_GT:
	case E1000_DEV_ID_ICH8_IFE_G:
	case E1000_DEV_ID_ICH8_IGP_M:
		hw->mac_type = e1000_ich8lan;
		break;
	case PCI_DEVICE_ID_INTEL_I210_UNPROGRAMMED:
	case PCI_DEVICE_ID_INTEL_I211_UNPROGRAMMED:
	case PCI_DEVICE_ID_INTEL_I210_COPPER:
	case PCI_DEVICE_ID_INTEL_I211_COPPER:
	case PCI_DEVICE_ID_INTEL_I210_COPPER_FLASHLESS:
	case PCI_DEVICE_ID_INTEL_I210_SERDES:
	case PCI_DEVICE_ID_INTEL_I210_SERDES_FLASHLESS:
	case PCI_DEVICE_ID_INTEL_I210_1000BASEKX:
		hw->mac_type = e1000_igb;
		break;
	default:
		/* Should never have loaded on this device */
		return -E1000_ERR_MAC_TYPE;
	}
	return E1000_SUCCESS;
}

/******************************************************************************
 * Reset the transmit and receive units; mask and clear all interrupts.
 *
 * hw - Struct containing variables accessed by shared code
 *****************************************************************************/
void
e1000_reset_hw(struct e1000_hw *hw)
{
	uint32_t ctrl;
	uint32_t ctrl_ext;
	uint32_t manc;
	uint32_t pba = 0;
	uint32_t reg;

	DEBUGFUNC();

	/* get the correct pba value for both PCI and PCIe*/
	if (hw->mac_type <  e1000_82571)
		pba = E1000_DEFAULT_PCI_PBA;
	else
		pba = E1000_DEFAULT_PCIE_PBA;

	/* For 82542 (rev 2.0), disable MWI before issuing a device reset */
	if (hw->mac_type == e1000_82542_rev2_0) {
		DEBUGOUT("Disabling MWI on 82542 rev 2.0\n");
#ifdef CONFIG_DM_ETH
		dm_pci_write_config16(hw->pdev, PCI_COMMAND,
				hw->pci_cmd_word & ~PCI_COMMAND_INVALIDATE);
#else
		pci_write_config_word(hw->pdev, PCI_COMMAND,
				hw->pci_cmd_word & ~PCI_COMMAND_INVALIDATE);
#endif
	}

	/* Clear interrupt mask to stop board from generating interrupts */
	DEBUGOUT("Masking off all interrupts\n");
	if (hw->mac_type == e1000_igb)
		E1000_WRITE_REG(hw, I210_IAM, 0);
	E1000_WRITE_REG(hw, IMC, 0xffffffff);

	/* Disable the Transmit and Receive units.  Then delay to allow
	 * any pending transactions to complete before we hit the MAC with
	 * the global reset.
	 */
	E1000_WRITE_REG(hw, RCTL, 0);
	E1000_WRITE_REG(hw, TCTL, E1000_TCTL_PSP);
	E1000_WRITE_FLUSH(hw);

	/* The tbi_compatibility_on Flag must be cleared when Rctl is cleared. */
	hw->tbi_compatibility_on = false;

	/* Delay to allow any outstanding PCI transactions to complete before
	 * resetting the device
	 */
	mdelay(10);

	/* Issue a global reset to the MAC.  This will reset the chip's
	 * transmit, receive, DMA, and link units.  It will not effect
	 * the current PCI configuration.  The global reset bit is self-
	 * clearing, and should clear within a microsecond.
	 */
	DEBUGOUT("Issuing a global reset to MAC\n");
	ctrl = E1000_READ_REG(hw, CTRL);

	E1000_WRITE_REG(hw, CTRL, (ctrl | E1000_CTRL_RST));

	/* Force a reload from the EEPROM if necessary */
	if (hw->mac_type == e1000_igb) {
		mdelay(20);
		reg = E1000_READ_REG(hw, STATUS);
		if (reg & E1000_STATUS_PF_RST_DONE)
			DEBUGOUT("PF OK\n");
		reg = E1000_READ_REG(hw, I210_EECD);
		if (reg & E1000_EECD_AUTO_RD)
			DEBUGOUT("EEC OK\n");
	} else if (hw->mac_type < e1000_82540) {
		/* Wait for reset to complete */
		udelay(10);
		ctrl_ext = E1000_READ_REG(hw, CTRL_EXT);
		ctrl_ext |= E1000_CTRL_EXT_EE_RST;
		E1000_WRITE_REG(hw, CTRL_EXT, ctrl_ext);
		E1000_WRITE_FLUSH(hw);
		/* Wait for EEPROM reload */
		mdelay(2);
	} else {
		/* Wait for EEPROM reload (it happens automatically) */
		mdelay(4);
		/* Dissable HW ARPs on ASF enabled adapters */
		manc = E1000_READ_REG(hw, MANC);
		manc &= ~(E1000_MANC_ARP_EN);
		E1000_WRITE_REG(hw, MANC, manc);
	}

	/* Clear interrupt mask to stop board from generating interrupts */
	DEBUGOUT("Masking off all interrupts\n");
	if (hw->mac_type == e1000_igb)
		E1000_WRITE_REG(hw, I210_IAM, 0);
	E1000_WRITE_REG(hw, IMC, 0xffffffff);

	/* Clear any pending interrupt events. */
	E1000_READ_REG(hw, ICR);

	/* If MWI was previously enabled, reenable it. */
	if (hw->mac_type == e1000_82542_rev2_0) {
#ifdef CONFIG_DM_ETH
		dm_pci_write_config16(hw->pdev, PCI_COMMAND, hw->pci_cmd_word);
#else
		pci_write_config_word(hw->pdev, PCI_COMMAND, hw->pci_cmd_word);
#endif
	}
	if (hw->mac_type != e1000_igb)
		E1000_WRITE_REG(hw, PBA, pba);
}

/******************************************************************************
 *
 * Initialize a number of hardware-dependent bits
 *
 * hw: Struct containing variables accessed by shared code
 *
 * This function contains hardware limitation workarounds for PCI-E adapters
 *
 *****************************************************************************/
static void
e1000_initialize_hardware_bits(struct e1000_hw *hw)
{
	if ((hw->mac_type >= e1000_82571) &&
			(!hw->initialize_hw_bits_disable)) {
		/* Settings common to all PCI-express silicon */
		uint32_t reg_ctrl, reg_ctrl_ext;
		uint32_t reg_tarc0, reg_tarc1;
		uint32_t reg_tctl;
		uint32_t reg_txdctl, reg_txdctl1;

		/* link autonegotiation/sync workarounds */
		reg_tarc0 = E1000_READ_REG(hw, TARC0);
		reg_tarc0 &= ~((1 << 30)|(1 << 29)|(1 << 28)|(1 << 27));

		/* Enable not-done TX descriptor counting */
		reg_txdctl = E1000_READ_REG(hw, TXDCTL);
		reg_txdctl |= E1000_TXDCTL_COUNT_DESC;
		E1000_WRITE_REG(hw, TXDCTL, reg_txdctl);

		reg_txdctl1 = E1000_READ_REG(hw, TXDCTL1);
		reg_txdctl1 |= E1000_TXDCTL_COUNT_DESC;
		E1000_WRITE_REG(hw, TXDCTL1, reg_txdctl1);


		switch (hw->mac_type) {
		case e1000_igb:			/* IGB is cool */
			return;
		case e1000_82571:
		case e1000_82572:
			/* Clear PHY TX compatible mode bits */
			reg_tarc1 = E1000_READ_REG(hw, TARC1);
			reg_tarc1 &= ~((1 << 30)|(1 << 29));

			/* link autonegotiation/sync workarounds */
			reg_tarc0 |= ((1 << 26)|(1 << 25)|(1 << 24)|(1 << 23));

			/* TX ring control fixes */
			reg_tarc1 |= ((1 << 26)|(1 << 25)|(1 << 24));

			/* Multiple read bit is reversed polarity */
			reg_tctl = E1000_READ_REG(hw, TCTL);
			if (reg_tctl & E1000_TCTL_MULR)
				reg_tarc1 &= ~(1 << 28);
			else
				reg_tarc1 |= (1 << 28);

			E1000_WRITE_REG(hw, TARC1, reg_tarc1);
			break;
		case e1000_82573:
		case e1000_82574:
			reg_ctrl_ext = E1000_READ_REG(hw, CTRL_EXT);
			reg_ctrl_ext &= ~(1 << 23);
			reg_ctrl_ext |= (1 << 22);

			/* TX byte count fix */
			reg_ctrl = E1000_READ_REG(hw, CTRL);
			reg_ctrl &= ~(1 << 29);

			E1000_WRITE_REG(hw, CTRL_EXT, reg_ctrl_ext);
			E1000_WRITE_REG(hw, CTRL, reg_ctrl);
			break;
		case e1000_80003es2lan:
	/* improve small packet performace for fiber/serdes */
			if ((hw->media_type == e1000_media_type_fiber)
			|| (hw->media_type ==
				e1000_media_type_internal_serdes)) {
				reg_tarc0 &= ~(1 << 20);
			}

		/* Multiple read bit is reversed polarity */
			reg_tctl = E1000_READ_REG(hw, TCTL);
			reg_tarc1 = E1000_READ_REG(hw, TARC1);
			if (reg_tctl & E1000_TCTL_MULR)
				reg_tarc1 &= ~(1 << 28);
			else
				reg_tarc1 |= (1 << 28);

			E1000_WRITE_REG(hw, TARC1, reg_tarc1);
			break;
		case e1000_ich8lan:
			/* Reduce concurrent DMA requests to 3 from 4 */
			if ((hw->revision_id < 3) ||
			((hw->device_id != E1000_DEV_ID_ICH8_IGP_M_AMT) &&
				(hw->device_id != E1000_DEV_ID_ICH8_IGP_M)))
				reg_tarc0 |= ((1 << 29)|(1 << 28));

			reg_ctrl_ext = E1000_READ_REG(hw, CTRL_EXT);
			reg_ctrl_ext |= (1 << 22);
			E1000_WRITE_REG(hw, CTRL_EXT, reg_ctrl_ext);

			/* workaround TX hang with TSO=on */
			reg_tarc0 |= ((1 << 27)|(1 << 26)|(1 << 24)|(1 << 23));

			/* Multiple read bit is reversed polarity */
			reg_tctl = E1000_READ_REG(hw, TCTL);
			reg_tarc1 = E1000_READ_REG(hw, TARC1);
			if (reg_tctl & E1000_TCTL_MULR)
				reg_tarc1 &= ~(1 << 28);
			else
				reg_tarc1 |= (1 << 28);

			/* workaround TX hang with TSO=on */
			reg_tarc1 |= ((1 << 30)|(1 << 26)|(1 << 24));

			E1000_WRITE_REG(hw, TARC1, reg_tarc1);
			break;
		default:
			break;
		}

		E1000_WRITE_REG(hw, TARC0, reg_tarc0);
	}
}

/******************************************************************************
 * Performs basic configuration of the adapter.
 *
 * hw - Struct containing variables accessed by shared code
 *
 * Assumes that the controller has previously been reset and is in a
 * post-reset uninitialized state. Initializes the receive address registers,
 * multicast table, and VLAN filter table. Calls routines to setup link
 * configuration and flow control settings. Clears all on-chip counters. Leaves
 * the transmit and receive units disabled and uninitialized.
 *****************************************************************************/
static int
e1000_init_hw(struct e1000_hw *hw, unsigned char enetaddr[6])
{
	uint32_t ctrl;
	uint32_t i;
	int32_t ret_val;
	uint16_t pcix_cmd_word;
	uint16_t pcix_stat_hi_word;
	uint16_t cmd_mmrbc;
	uint16_t stat_mmrbc;
	uint32_t mta_size;
	uint32_t reg_data;
	uint32_t ctrl_ext;
	DEBUGFUNC();
	/* force full DMA clock frequency for 10/100 on ICH8 A0-B0 */
	if ((hw->mac_type == e1000_ich8lan) &&
		((hw->revision_id < 3) ||
		((hw->device_id != E1000_DEV_ID_ICH8_IGP_M_AMT) &&
		(hw->device_id != E1000_DEV_ID_ICH8_IGP_M)))) {
			reg_data = E1000_READ_REG(hw, STATUS);
			reg_data &= ~0x80000000;
			E1000_WRITE_REG(hw, STATUS, reg_data);
	}
	/* Do not need initialize Identification LED */

	/* Set the media type and TBI compatibility */
	e1000_set_media_type(hw);

	/* Must be called after e1000_set_media_type
	 * because media_type is used */
	e1000_initialize_hardware_bits(hw);

	/* Disabling VLAN filtering. */
	DEBUGOUT("Initializing the IEEE VLAN\n");
	/* VET hardcoded to standard value and VFTA removed in ICH8 LAN */
	if (hw->mac_type != e1000_ich8lan) {
		if (hw->mac_type < e1000_82545_rev_3)
			E1000_WRITE_REG(hw, VET, 0);
		e1000_clear_vfta(hw);
	}

	/* For 82542 (rev 2.0), disable MWI and put the receiver into reset */
	if (hw->mac_type == e1000_82542_rev2_0) {
		DEBUGOUT("Disabling MWI on 82542 rev 2.0\n");
#ifdef CONFIG_DM_ETH
		dm_pci_write_config16(hw->pdev, PCI_COMMAND,
				      hw->
				      pci_cmd_word & ~PCI_COMMAND_INVALIDATE);
#else
		pci_write_config_word(hw->pdev, PCI_COMMAND,
				      hw->
				      pci_cmd_word & ~PCI_COMMAND_INVALIDATE);
#endif
		E1000_WRITE_REG(hw, RCTL, E1000_RCTL_RST);
		E1000_WRITE_FLUSH(hw);
		mdelay(5);
	}

	/* Setup the receive address. This involves initializing all of the Receive
	 * Address Registers (RARs 0 - 15).
	 */
	e1000_init_rx_addrs(hw, enetaddr);

	/* For 82542 (rev 2.0), take the receiver out of reset and enable MWI */
	if (hw->mac_type == e1000_82542_rev2_0) {
		E1000_WRITE_REG(hw, RCTL, 0);
		E1000_WRITE_FLUSH(hw);
		mdelay(1);
#ifdef CONFIG_DM_ETH
		dm_pci_write_config16(hw->pdev, PCI_COMMAND, hw->pci_cmd_word);
#else
		pci_write_config_word(hw->pdev, PCI_COMMAND, hw->pci_cmd_word);
#endif
	}

	/* Zero out the Multicast HASH table */
	DEBUGOUT("Zeroing the MTA\n");
	mta_size = E1000_MC_TBL_SIZE;
	if (hw->mac_type == e1000_ich8lan)
		mta_size = E1000_MC_TBL_SIZE_ICH8LAN;
	for (i = 0; i < mta_size; i++) {
		E1000_WRITE_REG_ARRAY(hw, MTA, i, 0);
		/* use write flush to prevent Memory Write Block (MWB) from
		 * occuring when accessing our register space */
		E1000_WRITE_FLUSH(hw);
	}

	switch (hw->mac_type) {
	case e1000_82545_rev_3:
	case e1000_82546_rev_3:
	case e1000_igb:
		break;
	default:
	/* Workaround for PCI-X problem when BIOS sets MMRBC incorrectly. */
	if (hw->bus_type == e1000_bus_type_pcix) {
#ifdef CONFIG_DM_ETH
		dm_pci_read_config16(hw->pdev, PCIX_COMMAND_REGISTER,
				     &pcix_cmd_word);
		dm_pci_read_config16(hw->pdev, PCIX_STATUS_REGISTER_HI,
				     &pcix_stat_hi_word);
#else
		pci_read_config_word(hw->pdev, PCIX_COMMAND_REGISTER,
				     &pcix_cmd_word);
		pci_read_config_word(hw->pdev, PCIX_STATUS_REGISTER_HI,
				     &pcix_stat_hi_word);
#endif
		cmd_mmrbc =
		    (pcix_cmd_word & PCIX_COMMAND_MMRBC_MASK) >>
		    PCIX_COMMAND_MMRBC_SHIFT;
		stat_mmrbc =
		    (pcix_stat_hi_word & PCIX_STATUS_HI_MMRBC_MASK) >>
		    PCIX_STATUS_HI_MMRBC_SHIFT;
		if (stat_mmrbc == PCIX_STATUS_HI_MMRBC_4K)
			stat_mmrbc = PCIX_STATUS_HI_MMRBC_2K;
		if (cmd_mmrbc > stat_mmrbc) {
			pcix_cmd_word &= ~PCIX_COMMAND_MMRBC_MASK;
			pcix_cmd_word |= stat_mmrbc << PCIX_COMMAND_MMRBC_SHIFT;
#ifdef CONFIG_DM_ETH
			dm_pci_write_config16(hw->pdev, PCIX_COMMAND_REGISTER,
					      pcix_cmd_word);
#else
			pci_write_config_word(hw->pdev, PCIX_COMMAND_REGISTER,
					      pcix_cmd_word);
#endif
		}
	}
		break;
	}

	/* More time needed for PHY to initialize */
	if (hw->mac_type == e1000_ich8lan)
		mdelay(15);
	if (hw->mac_type == e1000_igb)
		mdelay(15);

	/* Call a subroutine to configure the link and setup flow control. */
	ret_val = e1000_setup_link(hw);

	/* Set the transmit descriptor write-back policy */
	if (hw->mac_type > e1000_82544) {
		ctrl = E1000_READ_REG(hw, TXDCTL);
		ctrl =
		    (ctrl & ~E1000_TXDCTL_WTHRESH) |
		    E1000_TXDCTL_FULL_TX_DESC_WB;
		E1000_WRITE_REG(hw, TXDCTL, ctrl);
	}

	/* Set the receive descriptor write back policy */
	if (hw->mac_type >= e1000_82571) {
		ctrl = E1000_READ_REG(hw, RXDCTL);
		ctrl =
		    (ctrl & ~E1000_RXDCTL_WTHRESH) |
		    E1000_RXDCTL_FULL_RX_DESC_WB;
		E1000_WRITE_REG(hw, RXDCTL, ctrl);
	}

	switch (hw->mac_type) {
	default:
		break;
	case e1000_80003es2lan:
		/* Enable retransmit on late collisions */
		reg_data = E1000_READ_REG(hw, TCTL);
		reg_data |= E1000_TCTL_RTLC;
		E1000_WRITE_REG(hw, TCTL, reg_data);

		/* Configure Gigabit Carry Extend Padding */
		reg_data = E1000_READ_REG(hw, TCTL_EXT);
		reg_data &= ~E1000_TCTL_EXT_GCEX_MASK;
		reg_data |= DEFAULT_80003ES2LAN_TCTL_EXT_GCEX;
		E1000_WRITE_REG(hw, TCTL_EXT, reg_data);

		/* Configure Transmit Inter-Packet Gap */
		reg_data = E1000_READ_REG(hw, TIPG);
		reg_data &= ~E1000_TIPG_IPGT_MASK;
		reg_data |= DEFAULT_80003ES2LAN_TIPG_IPGT_1000;
		E1000_WRITE_REG(hw, TIPG, reg_data);

		reg_data = E1000_READ_REG_ARRAY(hw, FFLT, 0x0001);
		reg_data &= ~0x00100000;
		E1000_WRITE_REG_ARRAY(hw, FFLT, 0x0001, reg_data);
		/* Fall through */
	case e1000_82571:
	case e1000_82572:
	case e1000_ich8lan:
		ctrl = E1000_READ_REG(hw, TXDCTL1);
		ctrl = (ctrl & ~E1000_TXDCTL_WTHRESH)
			| E1000_TXDCTL_FULL_TX_DESC_WB;
		E1000_WRITE_REG(hw, TXDCTL1, ctrl);
		break;
	case e1000_82573:
	case e1000_82574:
		reg_data = E1000_READ_REG(hw, GCR);
		reg_data |= E1000_GCR_L1_ACT_WITHOUT_L0S_RX;
		E1000_WRITE_REG(hw, GCR, reg_data);
	case e1000_igb:
		break;
	}

	if (hw->device_id == E1000_DEV_ID_82546GB_QUAD_COPPER ||
		hw->device_id == E1000_DEV_ID_82546GB_QUAD_COPPER_KSP3) {
		ctrl_ext = E1000_READ_REG(hw, CTRL_EXT);
		/* Relaxed ordering must be disabled to avoid a parity
		 * error crash in a PCI slot. */
		ctrl_ext |= E1000_CTRL_EXT_RO_DIS;
		E1000_WRITE_REG(hw, CTRL_EXT, ctrl_ext);
	}

	return ret_val;
}

/******************************************************************************
 * Configures flow control and link settings.
 *
 * hw - Struct containing variables accessed by shared code
 *
 * Determines which flow control settings to use. Calls the apropriate media-
 * specific link configuration function. Configures the flow control settings.
 * Assuming the adapter has a valid link partner, a valid link should be
 * established. Assumes the hardware has previously been reset and the
 * transmitter and receiver are not enabled.
 *****************************************************************************/
static int
e1000_setup_link(struct e1000_hw *hw)
{
	int32_t ret_val;
#ifndef CONFIG_E1000_NO_NVM
	uint32_t ctrl_ext;
	uint16_t eeprom_data;
#endif

	DEBUGFUNC();

	/* In the case of the phy reset being blocked, we already have a link.
	 * We do not have to set it up again. */
	if (e1000_check_phy_reset_block(hw))
		return E1000_SUCCESS;

#ifndef CONFIG_E1000_NO_NVM
	/* Read and store word 0x0F of the EEPROM. This word contains bits
	 * that determine the hardware's default PAUSE (flow control) mode,
	 * a bit that determines whether the HW defaults to enabling or
	 * disabling auto-negotiation, and the direction of the
	 * SW defined pins. If there is no SW over-ride of the flow
	 * control setting, then the variable hw->fc will
	 * be initialized based on a value in the EEPROM.
	 */
	if (e1000_read_eeprom(hw, EEPROM_INIT_CONTROL2_REG, 1,
				&eeprom_data) < 0) {
		DEBUGOUT("EEPROM Read Error\n");
		return -E1000_ERR_EEPROM;
	}
#endif
	if (hw->fc == e1000_fc_default) {
		switch (hw->mac_type) {
		case e1000_ich8lan:
		case e1000_82573:
		case e1000_82574:
		case e1000_igb:
			hw->fc = e1000_fc_full;
			break;
		default:
#ifndef CONFIG_E1000_NO_NVM
			ret_val = e1000_read_eeprom(hw,
				EEPROM_INIT_CONTROL2_REG, 1, &eeprom_data);
			if (ret_val) {
				DEBUGOUT("EEPROM Read Error\n");
				return -E1000_ERR_EEPROM;
			}
			if ((eeprom_data & EEPROM_WORD0F_PAUSE_MASK) == 0)
				hw->fc = e1000_fc_none;
			else if ((eeprom_data & EEPROM_WORD0F_PAUSE_MASK) ==
				    EEPROM_WORD0F_ASM_DIR)
				hw->fc = e1000_fc_tx_pause;
			else
#endif
				hw->fc = e1000_fc_full;
			break;
		}
	}

	/* We want to save off the original Flow Control configuration just
	 * in case we get disconnected and then reconnected into a different
	 * hub or switch with different Flow Control capabilities.
	 */
	if (hw->mac_type == e1000_82542_rev2_0)
		hw->fc &= (~e1000_fc_tx_pause);

	if ((hw->mac_type < e1000_82543) && (hw->report_tx_early == 1))
		hw->fc &= (~e1000_fc_rx_pause);

	hw->original_fc = hw->fc;

	DEBUGOUT("After fix-ups FlowControl is now = %x\n", hw->fc);

#ifndef CONFIG_E1000_NO_NVM
	/* Take the 4 bits from EEPROM word 0x0F that determine the initial
	 * polarity value for the SW controlled pins, and setup the
	 * Extended Device Control reg with that info.
	 * This is needed because one of the SW controlled pins is used for
	 * signal detection.  So this should be done before e1000_setup_pcs_link()
	 * or e1000_phy_setup() is called.
	 */
	if (hw->mac_type == e1000_82543) {
		ctrl_ext = ((eeprom_data & EEPROM_WORD0F_SWPDIO_EXT) <<
			    SWDPIO__EXT_SHIFT);
		E1000_WRITE_REG(hw, CTRL_EXT, ctrl_ext);
	}
#endif

	/* Call the necessary subroutine to configure the link. */
	ret_val = (hw->media_type == e1000_media_type_fiber) ?
	    e1000_setup_fiber_link(hw) : e1000_setup_copper_link(hw);
	if (ret_val < 0) {
		return ret_val;
	}

	/* Initialize the flow control address, type, and PAUSE timer
	 * registers to their default values.  This is done even if flow
	 * control is disabled, because it does not hurt anything to
	 * initialize these registers.
	 */
	DEBUGOUT("Initializing the Flow Control address, type"
			"and timer regs\n");

	/* FCAL/H and FCT are hardcoded to standard values in e1000_ich8lan. */
	if (hw->mac_type != e1000_ich8lan) {
		E1000_WRITE_REG(hw, FCT, FLOW_CONTROL_TYPE);
		E1000_WRITE_REG(hw, FCAH, FLOW_CONTROL_ADDRESS_HIGH);
		E1000_WRITE_REG(hw, FCAL, FLOW_CONTROL_ADDRESS_LOW);
	}

	E1000_WRITE_REG(hw, FCTTV, hw->fc_pause_time);

	/* Set the flow control receive threshold registers.  Normally,
	 * these registers will be set to a default threshold that may be
	 * adjusted later by the driver's runtime code.  However, if the
	 * ability to transmit pause frames in not enabled, then these
	 * registers will be set to 0.
	 */
	if (!(hw->fc & e1000_fc_tx_pause)) {
		E1000_WRITE_REG(hw, FCRTL, 0);
		E1000_WRITE_REG(hw, FCRTH, 0);
	} else {
		/* We need to set up the Receive Threshold high and low water marks
		 * as well as (optionally) enabling the transmission of XON frames.
		 */
		if (hw->fc_send_xon) {
			E1000_WRITE_REG(hw, FCRTL,
					(hw->fc_low_water | E1000_FCRTL_XONE));
			E1000_WRITE_REG(hw, FCRTH, hw->fc_high_water);
		} else {
			E1000_WRITE_REG(hw, FCRTL, hw->fc_low_water);
			E1000_WRITE_REG(hw, FCRTH, hw->fc_high_water);
		}
	}
	return ret_val;
}

/******************************************************************************
 * Sets up link for a fiber based adapter
 *
 * hw - Struct containing variables accessed by shared code
 *
 * Manipulates Physical Coding Sublayer functions in order to configure
 * link. Assumes the hardware has been previously reset and the transmitter
 * and receiver are not enabled.
 *****************************************************************************/
static int
e1000_setup_fiber_link(struct e1000_hw *hw)
{
	uint32_t ctrl;
	uint32_t status;
	uint32_t txcw = 0;
	uint32_t i;
	uint32_t signal;
	int32_t ret_val;

	DEBUGFUNC();
	/* On adapters with a MAC newer that 82544, SW Defineable pin 1 will be
	 * set when the optics detect a signal. On older adapters, it will be
	 * cleared when there is a signal
	 */
	ctrl = E1000_READ_REG(hw, CTRL);
	if ((hw->mac_type > e1000_82544) && !(ctrl & E1000_CTRL_ILOS))
		signal = E1000_CTRL_SWDPIN1;
	else
		signal = 0;

	printf("signal for %s is %x (ctrl %08x)!!!!\n", hw->name, signal,
	       ctrl);
	/* Take the link out of reset */
	ctrl &= ~(E1000_CTRL_LRST);

	e1000_config_collision_dist(hw);

	/* Check for a software override of the flow control settings, and setup
	 * the device accordingly.  If auto-negotiation is enabled, then software
	 * will have to set the "PAUSE" bits to the correct value in the Tranmsit
	 * Config Word Register (TXCW) and re-start auto-negotiation.  However, if
	 * auto-negotiation is disabled, then software will have to manually
	 * configure the two flow control enable bits in the CTRL register.
	 *
	 * The possible values of the "fc" parameter are:
	 *	0:  Flow control is completely disabled
	 *	1:  Rx flow control is enabled (we can receive pause frames, but
	 *	    not send pause frames).
	 *	2:  Tx flow control is enabled (we can send pause frames but we do
	 *	    not support receiving pause frames).
	 *	3:  Both Rx and TX flow control (symmetric) are enabled.
	 */
	switch (hw->fc) {
	case e1000_fc_none:
		/* Flow control is completely disabled by a software over-ride. */
		txcw = (E1000_TXCW_ANE | E1000_TXCW_FD);
		break;
	case e1000_fc_rx_pause:
		/* RX Flow control is enabled and TX Flow control is disabled by a
		 * software over-ride. Since there really isn't a way to advertise
		 * that we are capable of RX Pause ONLY, we will advertise that we
		 * support both symmetric and asymmetric RX PAUSE. Later, we will
		 *  disable the adapter's ability to send PAUSE frames.
		 */
		txcw = (E1000_TXCW_ANE | E1000_TXCW_FD | E1000_TXCW_PAUSE_MASK);
		break;
	case e1000_fc_tx_pause:
		/* TX Flow control is enabled, and RX Flow control is disabled, by a
		 * software over-ride.
		 */
		txcw = (E1000_TXCW_ANE | E1000_TXCW_FD | E1000_TXCW_ASM_DIR);
		break;
	case e1000_fc_full:
		/* Flow control (both RX and TX) is enabled by a software over-ride. */
		txcw = (E1000_TXCW_ANE | E1000_TXCW_FD | E1000_TXCW_PAUSE_MASK);
		break;
	default:
		DEBUGOUT("Flow control param set incorrectly\n");
		return -E1000_ERR_CONFIG;
		break;
	}

	/* Since auto-negotiation is enabled, take the link out of reset (the link
	 * will be in reset, because we previously reset the chip). This will
	 * restart auto-negotiation.  If auto-neogtiation is successful then the
	 * link-up status bit will be set and the flow control enable bits (RFCE
	 * and TFCE) will be set according to their negotiated value.
	 */
	DEBUGOUT("Auto-negotiation enabled (%#x)\n", txcw);

	E1000_WRITE_REG(hw, TXCW, txcw);
	E1000_WRITE_REG(hw, CTRL, ctrl);
	E1000_WRITE_FLUSH(hw);

	hw->txcw = txcw;
	mdelay(1);

	/* If we have a signal (the cable is plugged in) then poll for a "Link-Up"
	 * indication in the Device Status Register.  Time-out if a link isn't
	 * seen in 500 milliseconds seconds (Auto-negotiation should complete in
	 * less than 500 milliseconds even if the other end is doing it in SW).
	 */
	if ((E1000_READ_REG(hw, CTRL) & E1000_CTRL_SWDPIN1) == signal) {
		DEBUGOUT("Looking for Link\n");
		for (i = 0; i < (LINK_UP_TIMEOUT / 10); i++) {
			mdelay(10);
			status = E1000_READ_REG(hw, STATUS);
			if (status & E1000_STATUS_LU)
				break;
		}
		if (i == (LINK_UP_TIMEOUT / 10)) {
			/* AutoNeg failed to achieve a link, so we'll call
			 * e1000_check_for_link. This routine will force the link up if we
			 * detect a signal. This will allow us to communicate with
			 * non-autonegotiating link partners.
			 */
			DEBUGOUT("Never got a valid link from auto-neg!!!\n");
			hw->autoneg_failed = 1;
			ret_val = e1000_check_for_link(hw);
			if (ret_val < 0) {
				DEBUGOUT("Error while checking for link\n");
				return ret_val;
			}
			hw->autoneg_failed = 0;
		} else {
			hw->autoneg_failed = 0;
			DEBUGOUT("Valid Link Found\n");
		}
	} else {
		DEBUGOUT("No Signal Detected\n");
		return -E1000_ERR_NOLINK;
	}
	return 0;
}

/******************************************************************************
* Make sure we have a valid PHY and change PHY mode before link setup.
*
* hw - Struct containing variables accessed by shared code
******************************************************************************/
static int32_t
e1000_copper_link_preconfig(struct e1000_hw *hw)
{
	uint32_t ctrl;
	int32_t ret_val;
	uint16_t phy_data;

	DEBUGFUNC();

	ctrl = E1000_READ_REG(hw, CTRL);
	/* With 82543, we need to force speed and duplex on the MAC equal to what
	 * the PHY speed and duplex configuration is. In addition, we need to
	 * perform a hardware reset on the PHY to take it out of reset.
	 */
	if (hw->mac_type > e1000_82543) {
		ctrl |= E1000_CTRL_SLU;
		ctrl &= ~(E1000_CTRL_FRCSPD | E1000_CTRL_FRCDPX);
		E1000_WRITE_REG(hw, CTRL, ctrl);
	} else {
		ctrl |= (E1000_CTRL_FRCSPD | E1000_CTRL_FRCDPX
				| E1000_CTRL_SLU);
		E1000_WRITE_REG(hw, CTRL, ctrl);
		ret_val = e1000_phy_hw_reset(hw);
		if (ret_val)
			return ret_val;
	}

	/* Make sure we have a valid PHY */
	ret_val = e1000_detect_gig_phy(hw);
	if (ret_val) {
		DEBUGOUT("Error, did not detect valid phy.\n");
		return ret_val;
	}
	DEBUGOUT("Phy ID = %x\n", hw->phy_id);

	/* Set PHY to class A mode (if necessary) */
	ret_val = e1000_set_phy_mode(hw);
	if (ret_val)
		return ret_val;
	if ((hw->mac_type == e1000_82545_rev_3) ||
		(hw->mac_type == e1000_82546_rev_3)) {
		ret_val = e1000_read_phy_reg(hw, M88E1000_PHY_SPEC_CTRL,
				&phy_data);
		phy_data |= 0x00000008;
		ret_val = e1000_write_phy_reg(hw, M88E1000_PHY_SPEC_CTRL,
				phy_data);
	}

	if (hw->mac_type <= e1000_82543 ||
		hw->mac_type == e1000_82541 || hw->mac_type == e1000_82547 ||
		hw->mac_type == e1000_82541_rev_2
		|| hw->mac_type == e1000_82547_rev_2)
			hw->phy_reset_disable = false;

	return E1000_SUCCESS;
}

/*****************************************************************************
 *
 * This function sets the lplu state according to the active flag.  When
 * activating lplu this function also disables smart speed and vise versa.
 * lplu will not be activated unless the device autonegotiation advertisment
 * meets standards of either 10 or 10/100 or 10/100/1000 at all duplexes.
 * hw: Struct containing variables accessed by shared code
 * active - true to enable lplu false to disable lplu.
 *
 * returns: - E1000_ERR_PHY if fail to read/write the PHY
 *            E1000_SUCCESS at any other case.
 *
 ****************************************************************************/

static int32_t
e1000_set_d3_lplu_state(struct e1000_hw *hw, bool active)
{
	uint32_t phy_ctrl = 0;
	int32_t ret_val;
	uint16_t phy_data;
	DEBUGFUNC();

	if (hw->phy_type != e1000_phy_igp && hw->phy_type != e1000_phy_igp_2
	    && hw->phy_type != e1000_phy_igp_3)
		return E1000_SUCCESS;

	/* During driver activity LPLU should not be used or it will attain link
	 * from the lowest speeds starting from 10Mbps. The capability is used
	 * for Dx transitions and states */
	if (hw->mac_type == e1000_82541_rev_2
			|| hw->mac_type == e1000_82547_rev_2) {
		ret_val = e1000_read_phy_reg(hw, IGP01E1000_GMII_FIFO,
				&phy_data);
		if (ret_val)
			return ret_val;
	} else if (hw->mac_type == e1000_ich8lan) {
		/* MAC writes into PHY register based on the state transition
		 * and start auto-negotiation. SW driver can overwrite the
		 * settings in CSR PHY power control E1000_PHY_CTRL register. */
		phy_ctrl = E1000_READ_REG(hw, PHY_CTRL);
	} else {
		ret_val = e1000_read_phy_reg(hw, IGP02E1000_PHY_POWER_MGMT,
				&phy_data);
		if (ret_val)
			return ret_val;
	}

	if (!active) {
		if (hw->mac_type == e1000_82541_rev_2 ||
			hw->mac_type == e1000_82547_rev_2) {
			phy_data &= ~IGP01E1000_GMII_FLEX_SPD;
			ret_val = e1000_write_phy_reg(hw, IGP01E1000_GMII_FIFO,
					phy_data);
			if (ret_val)
				return ret_val;
		} else {
			if (hw->mac_type == e1000_ich8lan) {
				phy_ctrl &= ~E1000_PHY_CTRL_NOND0A_LPLU;
				E1000_WRITE_REG(hw, PHY_CTRL, phy_ctrl);
			} else {
				phy_data &= ~IGP02E1000_PM_D3_LPLU;
				ret_val = e1000_write_phy_reg(hw,
					IGP02E1000_PHY_POWER_MGMT, phy_data);
				if (ret_val)
					return ret_val;
			}
		}

	/* LPLU and SmartSpeed are mutually exclusive.  LPLU is used during
	 * Dx states where the power conservation is most important.  During
	 * driver activity we should enable SmartSpeed, so performance is
	 * maintained. */
		if (hw->smart_speed == e1000_smart_speed_on) {
			ret_val = e1000_read_phy_reg(hw,
					IGP01E1000_PHY_PORT_CONFIG, &phy_data);
			if (ret_val)
				return ret_val;

			phy_data |= IGP01E1000_PSCFR_SMART_SPEED;
			ret_val = e1000_write_phy_reg(hw,
					IGP01E1000_PHY_PORT_CONFIG, phy_data);
			if (ret_val)
				return ret_val;
		} else if (hw->smart_speed == e1000_smart_speed_off) {
			ret_val = e1000_read_phy_reg(hw,
					IGP01E1000_PHY_PORT_CONFIG, &phy_data);
			if (ret_val)
				return ret_val;

			phy_data &= ~IGP01E1000_PSCFR_SMART_SPEED;
			ret_val = e1000_write_phy_reg(hw,
					IGP01E1000_PHY_PORT_CONFIG, phy_data);
			if (ret_val)
				return ret_val;
		}

	} else if ((hw->autoneg_advertised == AUTONEG_ADVERTISE_SPEED_DEFAULT)
		|| (hw->autoneg_advertised == AUTONEG_ADVERTISE_10_ALL) ||
		(hw->autoneg_advertised == AUTONEG_ADVERTISE_10_100_ALL)) {

		if (hw->mac_type == e1000_82541_rev_2 ||
		    hw->mac_type == e1000_82547_rev_2) {
			phy_data |= IGP01E1000_GMII_FLEX_SPD;
			ret_val = e1000_write_phy_reg(hw,
					IGP01E1000_GMII_FIFO, phy_data);
			if (ret_val)
				return ret_val;
		} else {
			if (hw->mac_type == e1000_ich8lan) {
				phy_ctrl |= E1000_PHY_CTRL_NOND0A_LPLU;
				E1000_WRITE_REG(hw, PHY_CTRL, phy_ctrl);
			} else {
				phy_data |= IGP02E1000_PM_D3_LPLU;
				ret_val = e1000_write_phy_reg(hw,
					IGP02E1000_PHY_POWER_MGMT, phy_data);
				if (ret_val)
					return ret_val;
			}
		}

		/* When LPLU is enabled we should disable SmartSpeed */
		ret_val = e1000_read_phy_reg(hw, IGP01E1000_PHY_PORT_CONFIG,
				&phy_data);
		if (ret_val)
			return ret_val;

		phy_data &= ~IGP01E1000_PSCFR_SMART_SPEED;
		ret_val = e1000_write_phy_reg(hw, IGP01E1000_PHY_PORT_CONFIG,
				phy_data);
		if (ret_val)
			return ret_val;
	}
	return E1000_SUCCESS;
}

/*****************************************************************************
 *
 * This function sets the lplu d0 state according to the active flag.  When
 * activating lplu this function also disables smart speed and vise versa.
 * lplu will not be activated unless the device autonegotiation advertisment
 * meets standards of either 10 or 10/100 or 10/100/1000 at all duplexes.
 * hw: Struct containing variables accessed by shared code
 * active - true to enable lplu false to disable lplu.
 *
 * returns: - E1000_ERR_PHY if fail to read/write the PHY
 *            E1000_SUCCESS at any other case.
 *
 ****************************************************************************/

static int32_t
e1000_set_d0_lplu_state(struct e1000_hw *hw, bool active)
{
	uint32_t phy_ctrl = 0;
	int32_t ret_val;
	uint16_t phy_data;
	DEBUGFUNC();

	if (hw->mac_type <= e1000_82547_rev_2)
		return E1000_SUCCESS;

	if (hw->mac_type == e1000_ich8lan) {
		phy_ctrl = E1000_READ_REG(hw, PHY_CTRL);
	} else if (hw->mac_type == e1000_igb) {
		phy_ctrl = E1000_READ_REG(hw, I210_PHY_CTRL);
	} else {
		ret_val = e1000_read_phy_reg(hw, IGP02E1000_PHY_POWER_MGMT,
				&phy_data);
		if (ret_val)
			return ret_val;
	}

	if (!active) {
		if (hw->mac_type == e1000_ich8lan) {
			phy_ctrl &= ~E1000_PHY_CTRL_D0A_LPLU;
			E1000_WRITE_REG(hw, PHY_CTRL, phy_ctrl);
		} else if (hw->mac_type == e1000_igb) {
			phy_ctrl &= ~E1000_PHY_CTRL_D0A_LPLU;
			E1000_WRITE_REG(hw, I210_PHY_CTRL, phy_ctrl);
		} else {
			phy_data &= ~IGP02E1000_PM_D0_LPLU;
			ret_val = e1000_write_phy_reg(hw,
					IGP02E1000_PHY_POWER_MGMT, phy_data);
			if (ret_val)
				return ret_val;
		}

		if (hw->mac_type == e1000_igb)
			return E1000_SUCCESS;

	/* LPLU and SmartSpeed are mutually exclusive.  LPLU is used during
	 * Dx states where the power conservation is most important.  During
	 * driver activity we should enable SmartSpeed, so performance is
	 * maintained. */
		if (hw->smart_speed == e1000_smart_speed_on) {
			ret_val = e1000_read_phy_reg(hw,
					IGP01E1000_PHY_PORT_CONFIG, &phy_data);
			if (ret_val)
				return ret_val;

			phy_data |= IGP01E1000_PSCFR_SMART_SPEED;
			ret_val = e1000_write_phy_reg(hw,
					IGP01E1000_PHY_PORT_CONFIG, phy_data);
			if (ret_val)
				return ret_val;
		} else if (hw->smart_speed == e1000_smart_speed_off) {
			ret_val = e1000_read_phy_reg(hw,
					IGP01E1000_PHY_PORT_CONFIG, &phy_data);
			if (ret_val)
				return ret_val;

			phy_data &= ~IGP01E1000_PSCFR_SMART_SPEED;
			ret_val = e1000_write_phy_reg(hw,
					IGP01E1000_PHY_PORT_CONFIG, phy_data);
			if (ret_val)
				return ret_val;
		}


	} else {

		if (hw->mac_type == e1000_ich8lan) {
			phy_ctrl |= E1000_PHY_CTRL_D0A_LPLU;
			E1000_WRITE_REG(hw, PHY_CTRL, phy_ctrl);
		} else if (hw->mac_type == e1000_igb) {
			phy_ctrl |= E1000_PHY_CTRL_D0A_LPLU;
			E1000_WRITE_REG(hw, I210_PHY_CTRL, phy_ctrl);
		} else {
			phy_data |= IGP02E1000_PM_D0_LPLU;
			ret_val = e1000_write_phy_reg(hw,
					IGP02E1000_PHY_POWER_MGMT, phy_data);
			if (ret_val)
				return ret_val;
		}

		if (hw->mac_type == e1000_igb)
			return E1000_SUCCESS;

		/* When LPLU is enabled we should disable SmartSpeed */
		ret_val = e1000_read_phy_reg(hw,
				IGP01E1000_PHY_PORT_CONFIG, &phy_data);
		if (ret_val)
			return ret_val;

		phy_data &= ~IGP01E1000_PSCFR_SMART_SPEED;
		ret_val = e1000_write_phy_reg(hw,
				IGP01E1000_PHY_PORT_CONFIG, phy_data);
		if (ret_val)
			return ret_val;

	}
	return E1000_SUCCESS;
}

/********************************************************************
* Copper link setup for e1000_phy_igp series.
*
* hw - Struct containing variables accessed by shared code
*********************************************************************/
static int32_t
e1000_copper_link_igp_setup(struct e1000_hw *hw)
{
	uint32_t led_ctrl;
	int32_t ret_val;
	uint16_t phy_data;

	DEBUGFUNC();

	if (hw->phy_reset_disable)
		return E1000_SUCCESS;

	ret_val = e1000_phy_reset(hw);
	if (ret_val) {
		DEBUGOUT("Error Resetting the PHY\n");
		return ret_val;
	}

	/* Wait 15ms for MAC to configure PHY from eeprom settings */
	mdelay(15);
	if (hw->mac_type != e1000_ich8lan) {
		/* Configure activity LED after PHY reset */
		led_ctrl = E1000_READ_REG(hw, LEDCTL);
		led_ctrl &= IGP_ACTIVITY_LED_MASK;
		led_ctrl |= (IGP_ACTIVITY_LED_ENABLE | IGP_LED3_MODE);
		E1000_WRITE_REG(hw, LEDCTL, led_ctrl);
	}

	/* The NVM settings will configure LPLU in D3 for IGP2 and IGP3 PHYs */
	if (hw->phy_type == e1000_phy_igp) {
		/* disable lplu d3 during driver init */
		ret_val = e1000_set_d3_lplu_state(hw, false);
		if (ret_val) {
			DEBUGOUT("Error Disabling LPLU D3\n");
			return ret_val;
		}
	}

	/* disable lplu d0 during driver init */
	ret_val = e1000_set_d0_lplu_state(hw, false);
	if (ret_val) {
		DEBUGOUT("Error Disabling LPLU D0\n");
		return ret_val;
	}
	/* Configure mdi-mdix settings */
	ret_val = e1000_read_phy_reg(hw, IGP01E1000_PHY_PORT_CTRL, &phy_data);
	if (ret_val)
		return ret_val;

	if ((hw->mac_type == e1000_82541) || (hw->mac_type == e1000_82547)) {
		hw->dsp_config_state = e1000_dsp_config_disabled;
		/* Force MDI for earlier revs of the IGP PHY */
		phy_data &= ~(IGP01E1000_PSCR_AUTO_MDIX
				| IGP01E1000_PSCR_FORCE_MDI_MDIX);
		hw->mdix = 1;

	} else {
		hw->dsp_config_state = e1000_dsp_config_enabled;
		phy_data &= ~IGP01E1000_PSCR_AUTO_MDIX;

		switch (hw->mdix) {
		case 1:
			phy_data &= ~IGP01E1000_PSCR_FORCE_MDI_MDIX;
			break;
		case 2:
			phy_data |= IGP01E1000_PSCR_FORCE_MDI_MDIX;
			break;
		case 0:
		default:
			phy_data |= IGP01E1000_PSCR_AUTO_MDIX;
			break;
		}
	}
	ret_val = e1000_write_phy_reg(hw, IGP01E1000_PHY_PORT_CTRL, phy_data);
	if (ret_val)
		return ret_val;

	/* set auto-master slave resolution settings */
	if (hw->autoneg) {
		e1000_ms_type phy_ms_setting = hw->master_slave;

		if (hw->ffe_config_state == e1000_ffe_config_active)
			hw->ffe_config_state = e1000_ffe_config_enabled;

		if (hw->dsp_config_state == e1000_dsp_config_activated)
			hw->dsp_config_state = e1000_dsp_config_enabled;

		/* when autonegotiation advertisment is only 1000Mbps then we
		  * should disable SmartSpeed and enable Auto MasterSlave
		  * resolution as hardware default. */
		if (hw->autoneg_advertised == ADVERTISE_1000_FULL) {
			/* Disable SmartSpeed */
			ret_val = e1000_read_phy_reg(hw,
					IGP01E1000_PHY_PORT_CONFIG, &phy_data);
			if (ret_val)
				return ret_val;
			phy_data &= ~IGP01E1000_PSCFR_SMART_SPEED;
			ret_val = e1000_write_phy_reg(hw,
					IGP01E1000_PHY_PORT_CONFIG, phy_data);
			if (ret_val)
				return ret_val;
			/* Set auto Master/Slave resolution process */
			ret_val = e1000_read_phy_reg(hw, PHY_1000T_CTRL,
					&phy_data);
			if (ret_val)
				return ret_val;
			phy_data &= ~CR_1000T_MS_ENABLE;
			ret_val = e1000_write_phy_reg(hw, PHY_1000T_CTRL,
					phy_data);
			if (ret_val)
				return ret_val;
		}

		ret_val = e1000_read_phy_reg(hw, PHY_1000T_CTRL, &phy_data);
		if (ret_val)
			return ret_val;

		/* load defaults for future use */
		hw->original_master_slave = (phy_data & CR_1000T_MS_ENABLE) ?
				((phy_data & CR_1000T_MS_VALUE) ?
				e1000_ms_force_master :
				e1000_ms_force_slave) :
				e1000_ms_auto;

		switch (phy_ms_setting) {
		case e1000_ms_force_master:
			phy_data |= (CR_1000T_MS_ENABLE | CR_1000T_MS_VALUE);
			break;
		case e1000_ms_force_slave:
			phy_data |= CR_1000T_MS_ENABLE;
			phy_data &= ~(CR_1000T_MS_VALUE);
			break;
		case e1000_ms_auto:
			phy_data &= ~CR_1000T_MS_ENABLE;
		default:
			break;
		}
		ret_val = e1000_write_phy_reg(hw, PHY_1000T_CTRL, phy_data);
		if (ret_val)
			return ret_val;
	}

	return E1000_SUCCESS;
}

/*****************************************************************************
 * This function checks the mode of the firmware.
 *
 * returns  - true when the mode is IAMT or false.
 ****************************************************************************/
bool
e1000_check_mng_mode(struct e1000_hw *hw)
{
	uint32_t fwsm;
	DEBUGFUNC();

	fwsm = E1000_READ_REG(hw, FWSM);

	if (hw->mac_type == e1000_ich8lan) {
		if ((fwsm & E1000_FWSM_MODE_MASK) ==
		    (E1000_MNG_ICH_IAMT_MODE << E1000_FWSM_MODE_SHIFT))
			return true;
	} else if ((fwsm & E1000_FWSM_MODE_MASK) ==
		       (E1000_MNG_IAMT_MODE << E1000_FWSM_MODE_SHIFT))
			return true;

	return false;
}

static int32_t
e1000_write_kmrn_reg(struct e1000_hw *hw, uint32_t reg_addr, uint16_t data)
{
	uint16_t swfw = E1000_SWFW_PHY0_SM;
	uint32_t reg_val;
	DEBUGFUNC();

	if (e1000_is_second_port(hw))
		swfw = E1000_SWFW_PHY1_SM;

	if (e1000_swfw_sync_acquire(hw, swfw))
		return -E1000_ERR_SWFW_SYNC;

	reg_val = ((reg_addr << E1000_KUMCTRLSTA_OFFSET_SHIFT)
			& E1000_KUMCTRLSTA_OFFSET) | data;
	E1000_WRITE_REG(hw, KUMCTRLSTA, reg_val);
	udelay(2);

	return E1000_SUCCESS;
}

static int32_t
e1000_read_kmrn_reg(struct e1000_hw *hw, uint32_t reg_addr, uint16_t *data)
{
	uint16_t swfw = E1000_SWFW_PHY0_SM;
	uint32_t reg_val;
	DEBUGFUNC();

	if (e1000_is_second_port(hw))
		swfw = E1000_SWFW_PHY1_SM;

	if (e1000_swfw_sync_acquire(hw, swfw)) {
		debug("%s[%i]\n", __func__, __LINE__);
		return -E1000_ERR_SWFW_SYNC;
	}

	/* Write register address */
	reg_val = ((reg_addr << E1000_KUMCTRLSTA_OFFSET_SHIFT) &
			E1000_KUMCTRLSTA_OFFSET) | E1000_KUMCTRLSTA_REN;
	E1000_WRITE_REG(hw, KUMCTRLSTA, reg_val);
	udelay(2);

	/* Read the data returned */
	reg_val = E1000_READ_REG(hw, KUMCTRLSTA);
	*data = (uint16_t)reg_val;

	return E1000_SUCCESS;
}

/********************************************************************
* Copper link setup for e1000_phy_gg82563 series.
*
* hw - Struct containing variables accessed by shared code
*********************************************************************/
static int32_t
e1000_copper_link_ggp_setup(struct e1000_hw *hw)
{
	int32_t ret_val;
	uint16_t phy_data;
	uint32_t reg_data;

	DEBUGFUNC();

	if (!hw->phy_reset_disable) {
		/* Enable CRS on TX for half-duplex operation. */
		ret_val = e1000_read_phy_reg(hw,
				GG82563_PHY_MAC_SPEC_CTRL, &phy_data);
		if (ret_val)
			return ret_val;

		phy_data |= GG82563_MSCR_ASSERT_CRS_ON_TX;
		/* Use 25MHz for both link down and 1000BASE-T for Tx clock */
		phy_data |= GG82563_MSCR_TX_CLK_1000MBPS_25MHZ;

		ret_val = e1000_write_phy_reg(hw,
				GG82563_PHY_MAC_SPEC_CTRL, phy_data);
		if (ret_val)
			return ret_val;

		/* Options:
		 *   MDI/MDI-X = 0 (default)
		 *   0 - Auto for all speeds
		 *   1 - MDI mode
		 *   2 - MDI-X mode
		 *   3 - Auto for 1000Base-T only (MDI-X for 10/100Base-T modes)
		 */
		ret_val = e1000_read_phy_reg(hw,
				GG82563_PHY_SPEC_CTRL, &phy_data);
		if (ret_val)
			return ret_val;

		phy_data &= ~GG82563_PSCR_CROSSOVER_MODE_MASK;

		switch (hw->mdix) {
		case 1:
			phy_data |= GG82563_PSCR_CROSSOVER_MODE_MDI;
			break;
		case 2:
			phy_data |= GG82563_PSCR_CROSSOVER_MODE_MDIX;
			break;
		case 0:
		default:
			phy_data |= GG82563_PSCR_CROSSOVER_MODE_AUTO;
			break;
		}

		/* Options:
		 *   disable_polarity_correction = 0 (default)
		 *       Automatic Correction for Reversed Cable Polarity
		 *   0 - Disabled
		 *   1 - Enabled
		 */
		phy_data &= ~GG82563_PSCR_POLARITY_REVERSAL_DISABLE;
		ret_val = e1000_write_phy_reg(hw,
				GG82563_PHY_SPEC_CTRL, phy_data);

		if (ret_val)
			return ret_val;

		/* SW Reset the PHY so all changes take effect */
		ret_val = e1000_phy_reset(hw);
		if (ret_val) {
			DEBUGOUT("Error Resetting the PHY\n");
			return ret_val;
		}
	} /* phy_reset_disable */

	if (hw->mac_type == e1000_80003es2lan) {
		/* Bypass RX and TX FIFO's */
		ret_val = e1000_write_kmrn_reg(hw,
				E1000_KUMCTRLSTA_OFFSET_FIFO_CTRL,
				E1000_KUMCTRLSTA_FIFO_CTRL_RX_BYPASS
				| E1000_KUMCTRLSTA_FIFO_CTRL_TX_BYPASS);
		if (ret_val)
			return ret_val;

		ret_val = e1000_read_phy_reg(hw,
				GG82563_PHY_SPEC_CTRL_2, &phy_data);
		if (ret_val)
			return ret_val;

		phy_data &= ~GG82563_PSCR2_REVERSE_AUTO_NEG;
		ret_val = e1000_write_phy_reg(hw,
				GG82563_PHY_SPEC_CTRL_2, phy_data);

		if (ret_val)
			return ret_val;

		reg_data = E1000_READ_REG(hw, CTRL_EXT);
		reg_data &= ~(E1000_CTRL_EXT_LINK_MODE_MASK);
		E1000_WRITE_REG(hw, CTRL_EXT, reg_data);

		ret_val = e1000_read_phy_reg(hw,
				GG82563_PHY_PWR_MGMT_CTRL, &phy_data);
		if (ret_val)
			return ret_val;

	/* Do not init these registers when the HW is in IAMT mode, since the
	 * firmware will have already initialized them.  We only initialize
	 * them if the HW is not in IAMT mode.
	 */
		if (e1000_check_mng_mode(hw) == false) {
			/* Enable Electrical Idle on the PHY */
			phy_data |= GG82563_PMCR_ENABLE_ELECTRICAL_IDLE;
			ret_val = e1000_write_phy_reg(hw,
					GG82563_PHY_PWR_MGMT_CTRL, phy_data);
			if (ret_val)
				return ret_val;

			ret_val = e1000_read_phy_reg(hw,
					GG82563_PHY_KMRN_MODE_CTRL, &phy_data);
			if (ret_val)
				return ret_val;

			phy_data &= ~GG82563_KMCR_PASS_FALSE_CARRIER;
			ret_val = e1000_write_phy_reg(hw,
					GG82563_PHY_KMRN_MODE_CTRL, phy_data);

			if (ret_val)
				return ret_val;
		}

		/* Workaround: Disable padding in Kumeran interface in the MAC
		 * and in the PHY to avoid CRC errors.
		 */
		ret_val = e1000_read_phy_reg(hw,
				GG82563_PHY_INBAND_CTRL, &phy_data);
		if (ret_val)
			return ret_val;
		phy_data |= GG82563_ICR_DIS_PADDING;
		ret_val = e1000_write_phy_reg(hw,
				GG82563_PHY_INBAND_CTRL, phy_data);
		if (ret_val)
			return ret_val;
	}
	return E1000_SUCCESS;
}

/********************************************************************
* Copper link setup for e1000_phy_m88 series.
*
* hw - Struct containing variables accessed by shared code
*********************************************************************/
static int32_t
e1000_copper_link_mgp_setup(struct e1000_hw *hw)
{
	int32_t ret_val;
	uint16_t phy_data;

	DEBUGFUNC();

	if (hw->phy_reset_disable)
		return E1000_SUCCESS;

	/* Enable CRS on TX. This must be set for half-duplex operation. */
	ret_val = e1000_read_phy_reg(hw, M88E1000_PHY_SPEC_CTRL, &phy_data);
	if (ret_val)
		return ret_val;

	phy_data |= M88E1000_PSCR_ASSERT_CRS_ON_TX;

	/* Options:
	 *   MDI/MDI-X = 0 (default)
	 *   0 - Auto for all speeds
	 *   1 - MDI mode
	 *   2 - MDI-X mode
	 *   3 - Auto for 1000Base-T only (MDI-X for 10/100Base-T modes)
	 */
	phy_data &= ~M88E1000_PSCR_AUTO_X_MODE;

	switch (hw->mdix) {
	case 1:
		phy_data |= M88E1000_PSCR_MDI_MANUAL_MODE;
		break;
	case 2:
		phy_data |= M88E1000_PSCR_MDIX_MANUAL_MODE;
		break;
	case 3:
		phy_data |= M88E1000_PSCR_AUTO_X_1000T;
		break;
	case 0:
	default:
		phy_data |= M88E1000_PSCR_AUTO_X_MODE;
		break;
	}

	/* Options:
	 *   disable_polarity_correction = 0 (default)
	 *       Automatic Correction for Reversed Cable Polarity
	 *   0 - Disabled
	 *   1 - Enabled
	 */
	phy_data &= ~M88E1000_PSCR_POLARITY_REVERSAL;
	ret_val = e1000_write_phy_reg(hw, M88E1000_PHY_SPEC_CTRL, phy_data);
	if (ret_val)
		return ret_val;

	if (hw->phy_revision < M88E1011_I_REV_4) {
		/* Force TX_CLK in the Extended PHY Specific Control Register
		 * to 25MHz clock.
		 */
		ret_val = e1000_read_phy_reg(hw,
				M88E1000_EXT_PHY_SPEC_CTRL, &phy_data);
		if (ret_val)
			return ret_val;

		phy_data |= M88E1000_EPSCR_TX_CLK_25;

		if ((hw->phy_revision == E1000_REVISION_2) &&
			(hw->phy_id == M88E1111_I_PHY_ID)) {
			/* Vidalia Phy, set the downshift counter to 5x */
			phy_data &= ~(M88EC018_EPSCR_DOWNSHIFT_COUNTER_MASK);
			phy_data |= M88EC018_EPSCR_DOWNSHIFT_COUNTER_5X;
			ret_val = e1000_write_phy_reg(hw,
					M88E1000_EXT_PHY_SPEC_CTRL, phy_data);
			if (ret_val)
				return ret_val;
		} else {
			/* Configure Master and Slave downshift values */
			phy_data &= ~(M88E1000_EPSCR_MASTER_DOWNSHIFT_MASK
					| M88E1000_EPSCR_SLAVE_DOWNSHIFT_MASK);
			phy_data |= (M88E1000_EPSCR_MASTER_DOWNSHIFT_1X
					| M88E1000_EPSCR_SLAVE_DOWNSHIFT_1X);
			ret_val = e1000_write_phy_reg(hw,
					M88E1000_EXT_PHY_SPEC_CTRL, phy_data);
			if (ret_val)
				return ret_val;
		}
	}

	/* SW Reset the PHY so all changes take effect */
	ret_val = e1000_phy_reset(hw);
	if (ret_val) {
		DEBUGOUT("Error Resetting the PHY\n");
		return ret_val;
	}

	return E1000_SUCCESS;
}

/********************************************************************
* Setup auto-negotiation and flow control advertisements,
* and then perform auto-negotiation.
*
* hw - Struct containing variables accessed by shared code
*********************************************************************/
static int32_t
e1000_copper_link_autoneg(struct e1000_hw *hw)
{
	int32_t ret_val;
	uint16_t phy_data;

	DEBUGFUNC();

	/* Perform some bounds checking on the hw->autoneg_advertised
	 * parameter.  If this variable is zero, then set it to the default.
	 */
	hw->autoneg_advertised &= AUTONEG_ADVERTISE_SPEED_DEFAULT;

	/* If autoneg_advertised is zero, we assume it was not defaulted
	 * by the calling code so we set to advertise full capability.
	 */
	if (hw->autoneg_advertised == 0)
		hw->autoneg_advertised = AUTONEG_ADVERTISE_SPEED_DEFAULT;

	/* IFE phy only supports 10/100 */
	if (hw->phy_type == e1000_phy_ife)
		hw->autoneg_advertised &= AUTONEG_ADVERTISE_10_100_ALL;

	DEBUGOUT("Reconfiguring auto-neg advertisement params\n");
	ret_val = e1000_phy_setup_autoneg(hw);
	if (ret_val) {
		DEBUGOUT("Error Setting up Auto-Negotiation\n");
		return ret_val;
	}
	DEBUGOUT("Restarting Auto-Neg\n");

	/* Restart auto-negotiation by setting the Auto Neg Enable bit and
	 * the Auto Neg Restart bit in the PHY control register.
	 */
	ret_val = e1000_read_phy_reg(hw, PHY_CTRL, &phy_data);
	if (ret_val)
		return ret_val;

	phy_data |= (MII_CR_AUTO_NEG_EN | MII_CR_RESTART_AUTO_NEG);
	ret_val = e1000_write_phy_reg(hw, PHY_CTRL, phy_data);
	if (ret_val)
		return ret_val;

	/* Does the user want to wait for Auto-Neg to complete here, or
	 * check at a later time (for example, callback routine).
	 */
	/* If we do not wait for autonegtation to complete I
	 * do not see a valid link status.
	 * wait_autoneg_complete = 1 .
	 */
	if (hw->wait_autoneg_complete) {
		ret_val = e1000_wait_autoneg(hw);
		if (ret_val) {
			DEBUGOUT("Error while waiting for autoneg"
					"to complete\n");
			return ret_val;
		}
	}

	hw->get_link_status = true;

	return E1000_SUCCESS;
}

/******************************************************************************
* Config the MAC and the PHY after link is up.
*   1) Set up the MAC to the current PHY speed/duplex
*      if we are on 82543.  If we
*      are on newer silicon, we only need to configure
*      collision distance in the Transmit Control Register.
*   2) Set up flow control on the MAC to that established with
*      the link partner.
*   3) Config DSP to improve Gigabit link quality for some PHY revisions.
*
* hw - Struct containing variables accessed by shared code
******************************************************************************/
static int32_t
e1000_copper_link_postconfig(struct e1000_hw *hw)
{
	int32_t ret_val;
	DEBUGFUNC();

	if (hw->mac_type >= e1000_82544) {
		e1000_config_collision_dist(hw);
	} else {
		ret_val = e1000_config_mac_to_phy(hw);
		if (ret_val) {
			DEBUGOUT("Error configuring MAC to PHY settings\n");
			return ret_val;
		}
	}
	ret_val = e1000_config_fc_after_link_up(hw);
	if (ret_val) {
		DEBUGOUT("Error Configuring Flow Control\n");
		return ret_val;
	}
	return E1000_SUCCESS;
}

/******************************************************************************
* Detects which PHY is present and setup the speed and duplex
*
* hw - Struct containing variables accessed by shared code
******************************************************************************/
static int
e1000_setup_copper_link(struct e1000_hw *hw)
{
	int32_t ret_val;
	uint16_t i;
	uint16_t phy_data;
	uint16_t reg_data;

	DEBUGFUNC();

	switch (hw->mac_type) {
	case e1000_80003es2lan:
	case e1000_ich8lan:
		/* Set the mac to wait the maximum time between each
		 * iteration and increase the max iterations when
		 * polling the phy; this fixes erroneous timeouts at 10Mbps. */
		ret_val = e1000_write_kmrn_reg(hw,
				GG82563_REG(0x34, 4), 0xFFFF);
		if (ret_val)
			return ret_val;
		ret_val = e1000_read_kmrn_reg(hw,
				GG82563_REG(0x34, 9), &reg_data);
		if (ret_val)
			return ret_val;
		reg_data |= 0x3F;
		ret_val = e1000_write_kmrn_reg(hw,
				GG82563_REG(0x34, 9), reg_data);
		if (ret_val)
			return ret_val;
	default:
		break;
	}

	/* Check if it is a valid PHY and set PHY mode if necessary. */
	ret_val = e1000_copper_link_preconfig(hw);
	if (ret_val)
		return ret_val;
	switch (hw->mac_type) {
	case e1000_80003es2lan:
		/* Kumeran registers are written-only */
		reg_data =
		E1000_KUMCTRLSTA_INB_CTRL_LINK_STATUS_TX_TIMEOUT_DEFAULT;
		reg_data |= E1000_KUMCTRLSTA_INB_CTRL_DIS_PADDING;
		ret_val = e1000_write_kmrn_reg(hw,
				E1000_KUMCTRLSTA_OFFSET_INB_CTRL, reg_data);
		if (ret_val)
			return ret_val;
		break;
	default:
		break;
	}

	if (hw->phy_type == e1000_phy_igp ||
		hw->phy_type == e1000_phy_igp_3 ||
		hw->phy_type == e1000_phy_igp_2) {
		ret_val = e1000_copper_link_igp_setup(hw);
		if (ret_val)
			return ret_val;
	} else if (hw->phy_type == e1000_phy_m88 ||
		hw->phy_type == e1000_phy_igb) {
		ret_val = e1000_copper_link_mgp_setup(hw);
		if (ret_val)
			return ret_val;
	} else if (hw->phy_type == e1000_phy_gg82563) {
		ret_val = e1000_copper_link_ggp_setup(hw);
		if (ret_val)
			return ret_val;
	}

	/* always auto */
	/* Setup autoneg and flow control advertisement
	  * and perform autonegotiation */
	ret_val = e1000_copper_link_autoneg(hw);
	if (ret_val)
		return ret_val;

	/* Check link status. Wait up to 100 microseconds for link to become
	 * valid.
	 */
	for (i = 0; i < 10; i++) {
		ret_val = e1000_read_phy_reg(hw, PHY_STATUS, &phy_data);
		if (ret_val)
			return ret_val;
		ret_val = e1000_read_phy_reg(hw, PHY_STATUS, &phy_data);
		if (ret_val)
			return ret_val;

		if (phy_data & MII_SR_LINK_STATUS) {
			/* Config the MAC and PHY after link is up */
			ret_val = e1000_copper_link_postconfig(hw);
			if (ret_val)
				return ret_val;

			DEBUGOUT("Valid link established!!!\n");
			return E1000_SUCCESS;
		}
		udelay(10);
	}

	DEBUGOUT("Unable to establish link!!!\n");
	return E1000_SUCCESS;
}

/******************************************************************************
* Configures PHY autoneg and flow control advertisement settings
*
* hw - Struct containing variables accessed by shared code
******************************************************************************/
int32_t
e1000_phy_setup_autoneg(struct e1000_hw *hw)
{
	int32_t ret_val;
	uint16_t mii_autoneg_adv_reg;
	uint16_t mii_1000t_ctrl_reg;

	DEBUGFUNC();

	/* Read the MII Auto-Neg Advertisement Register (Address 4). */
	ret_val = e1000_read_phy_reg(hw, PHY_AUTONEG_ADV, &mii_autoneg_adv_reg);
	if (ret_val)
		return ret_val;

	if (hw->phy_type != e1000_phy_ife) {
		/* Read the MII 1000Base-T Control Register (Address 9). */
		ret_val = e1000_read_phy_reg(hw, PHY_1000T_CTRL,
				&mii_1000t_ctrl_reg);
		if (ret_val)
			return ret_val;
	} else
		mii_1000t_ctrl_reg = 0;

	/* Need to parse both autoneg_advertised and fc and set up
	 * the appropriate PHY registers.  First we will parse for
	 * autoneg_advertised software override.  Since we can advertise
	 * a plethora of combinations, we need to check each bit
	 * individually.
	 */

	/* First we clear all the 10/100 mb speed bits in the Auto-Neg
	 * Advertisement Register (Address 4) and the 1000 mb speed bits in
	 * the  1000Base-T Control Register (Address 9).
	 */
	mii_autoneg_adv_reg &= ~REG4_SPEED_MASK;
	mii_1000t_ctrl_reg &= ~REG9_SPEED_MASK;

	DEBUGOUT("autoneg_advertised %x\n", hw->autoneg_advertised);

	/* Do we want to advertise 10 Mb Half Duplex? */
	if (hw->autoneg_advertised & ADVERTISE_10_HALF) {
		DEBUGOUT("Advertise 10mb Half duplex\n");
		mii_autoneg_adv_reg |= NWAY_AR_10T_HD_CAPS;
	}

	/* Do we want to advertise 10 Mb Full Duplex? */
	if (hw->autoneg_advertised & ADVERTISE_10_FULL) {
		DEBUGOUT("Advertise 10mb Full duplex\n");
		mii_autoneg_adv_reg |= NWAY_AR_10T_FD_CAPS;
	}

	/* Do we want to advertise 100 Mb Half Duplex? */
	if (hw->autoneg_advertised & ADVERTISE_100_HALF) {
		DEBUGOUT("Advertise 100mb Half duplex\n");
		mii_autoneg_adv_reg |= NWAY_AR_100TX_HD_CAPS;
	}

	/* Do we want to advertise 100 Mb Full Duplex? */
	if (hw->autoneg_advertised & ADVERTISE_100_FULL) {
		DEBUGOUT("Advertise 100mb Full duplex\n");
		mii_autoneg_adv_reg |= NWAY_AR_100TX_FD_CAPS;
	}

	/* We do not allow the Phy to advertise 1000 Mb Half Duplex */
	if (hw->autoneg_advertised & ADVERTISE_1000_HALF) {
		DEBUGOUT
		    ("Advertise 1000mb Half duplex requested, request denied!\n");
	}

	/* Do we want to advertise 1000 Mb Full Duplex? */
	if (hw->autoneg_advertised & ADVERTISE_1000_FULL) {
		DEBUGOUT("Advertise 1000mb Full duplex\n");
		mii_1000t_ctrl_reg |= CR_1000T_FD_CAPS;
	}

	/* Check for a software override of the flow control settings, and
	 * setup the PHY advertisement registers accordingly.  If
	 * auto-negotiation is enabled, then software will have to set the
	 * "PAUSE" bits to the correct value in the Auto-Negotiation
	 * Advertisement Register (PHY_AUTONEG_ADV) and re-start auto-negotiation.
	 *
	 * The possible values of the "fc" parameter are:
	 *	0:  Flow control is completely disabled
	 *	1:  Rx flow control is enabled (we can receive pause frames
	 *	    but not send pause frames).
	 *	2:  Tx flow control is enabled (we can send pause frames
	 *	    but we do not support receiving pause frames).
	 *	3:  Both Rx and TX flow control (symmetric) are enabled.
	 *  other:  No software override.  The flow control configuration
	 *	    in the EEPROM is used.
	 */
	switch (hw->fc) {
	case e1000_fc_none:	/* 0 */
		/* Flow control (RX & TX) is completely disabled by a
		 * software over-ride.
		 */
		mii_autoneg_adv_reg &= ~(NWAY_AR_ASM_DIR | NWAY_AR_PAUSE);
		break;
	case e1000_fc_rx_pause:	/* 1 */
		/* RX Flow control is enabled, and TX Flow control is
		 * disabled, by a software over-ride.
		 */
		/* Since there really isn't a way to advertise that we are
		 * capable of RX Pause ONLY, we will advertise that we
		 * support both symmetric and asymmetric RX PAUSE.  Later
		 * (in e1000_config_fc_after_link_up) we will disable the
		 *hw's ability to send PAUSE frames.
		 */
		mii_autoneg_adv_reg |= (NWAY_AR_ASM_DIR | NWAY_AR_PAUSE);
		break;
	case e1000_fc_tx_pause:	/* 2 */
		/* TX Flow control is enabled, and RX Flow control is
		 * disabled, by a software over-ride.
		 */
		mii_autoneg_adv_reg |= NWAY_AR_ASM_DIR;
		mii_autoneg_adv_reg &= ~NWAY_AR_PAUSE;
		break;
	case e1000_fc_full:	/* 3 */
		/* Flow control (both RX and TX) is enabled by a software
		 * over-ride.
		 */
		mii_autoneg_adv_reg |= (NWAY_AR_ASM_DIR | NWAY_AR_PAUSE);
		break;
	default:
		DEBUGOUT("Flow control param set incorrectly\n");
		return -E1000_ERR_CONFIG;
	}

	ret_val = e1000_write_phy_reg(hw, PHY_AUTONEG_ADV, mii_autoneg_adv_reg);
	if (ret_val)
		return ret_val;

	DEBUGOUT("Auto-Neg Advertising %x\n", mii_autoneg_adv_reg);

	if (hw->phy_type != e1000_phy_ife) {
		ret_val = e1000_write_phy_reg(hw, PHY_1000T_CTRL,
				mii_1000t_ctrl_reg);
		if (ret_val)
			return ret_val;
	}

	return E1000_SUCCESS;
}

/******************************************************************************
* Sets the collision distance in the Transmit Control register
*
* hw - Struct containing variables accessed by shared code
*
* Link should have been established previously. Reads the speed and duplex
* information from the Device Status register.
******************************************************************************/
static void
e1000_config_collision_dist(struct e1000_hw *hw)
{
	uint32_t tctl, coll_dist;

	DEBUGFUNC();

	if (hw->mac_type < e1000_82543)
		coll_dist = E1000_COLLISION_DISTANCE_82542;
	else
		coll_dist = E1000_COLLISION_DISTANCE;

	tctl = E1000_READ_REG(hw, TCTL);

	tctl &= ~E1000_TCTL_COLD;
	tctl |= coll_dist << E1000_COLD_SHIFT;

	E1000_WRITE_REG(hw, TCTL, tctl);
	E1000_WRITE_FLUSH(hw);
}

/******************************************************************************
* Sets MAC speed and duplex settings to reflect the those in the PHY
*
* hw - Struct containing variables accessed by shared code
* mii_reg - data to write to the MII control register
*
* The contents of the PHY register containing the needed information need to
* be passed in.
******************************************************************************/
static int
e1000_config_mac_to_phy(struct e1000_hw *hw)
{
	uint32_t ctrl;
	uint16_t phy_data;

	DEBUGFUNC();

	/* Read the Device Control Register and set the bits to Force Speed
	 * and Duplex.
	 */
	ctrl = E1000_READ_REG(hw, CTRL);
	ctrl |= (E1000_CTRL_FRCSPD | E1000_CTRL_FRCDPX);
	ctrl &= ~(E1000_CTRL_ILOS);
	ctrl |= (E1000_CTRL_SPD_SEL);

	/* Set up duplex in the Device Control and Transmit Control
	 * registers depending on negotiated values.
	 */
	if (e1000_read_phy_reg(hw, M88E1000_PHY_SPEC_STATUS, &phy_data) < 0) {
		DEBUGOUT("PHY Read Error\n");
		return -E1000_ERR_PHY;
	}
	if (phy_data & M88E1000_PSSR_DPLX)
		ctrl |= E1000_CTRL_FD;
	else
		ctrl &= ~E1000_CTRL_FD;

	e1000_config_collision_dist(hw);

	/* Set up speed in the Device Control register depending on
	 * negotiated values.
	 */
	if ((phy_data & M88E1000_PSSR_SPEED) == M88E1000_PSSR_1000MBS)
		ctrl |= E1000_CTRL_SPD_1000;
	else if ((phy_data & M88E1000_PSSR_SPEED) == M88E1000_PSSR_100MBS)
		ctrl |= E1000_CTRL_SPD_100;
	/* Write the configured values back to the Device Control Reg. */
	E1000_WRITE_REG(hw, CTRL, ctrl);
	return 0;
}

/******************************************************************************
 * Forces the MAC's flow control settings.
 *
 * hw - Struct containing variables accessed by shared code
 *
 * Sets the TFCE and RFCE bits in the device control register to reflect
 * the adapter settings. TFCE and RFCE need to be explicitly set by
 * software when a Copper PHY is used because autonegotiation is managed
 * by the PHY rather than the MAC. Software must also configure these
 * bits when link is forced on a fiber connection.
 *****************************************************************************/
static int
e1000_force_mac_fc(struct e1000_hw *hw)
{
	uint32_t ctrl;

	DEBUGFUNC();

	/* Get the current configuration of the Device Control Register */
	ctrl = E1000_READ_REG(hw, CTRL);

	/* Because we didn't get link via the internal auto-negotiation
	 * mechanism (we either forced link or we got link via PHY
	 * auto-neg), we have to manually enable/disable transmit an
	 * receive flow control.
	 *
	 * The "Case" statement below enables/disable flow control
	 * according to the "hw->fc" parameter.
	 *
	 * The possible values of the "fc" parameter are:
	 *	0:  Flow control is completely disabled
	 *	1:  Rx flow control is enabled (we can receive pause
	 *	    frames but not send pause frames).
	 *	2:  Tx flow control is enabled (we can send pause frames
	 *	    frames but we do not receive pause frames).
	 *	3:  Both Rx and TX flow control (symmetric) is enabled.
	 *  other:  No other values should be possible at this point.
	 */

	switch (hw->fc) {
	case e1000_fc_none:
		ctrl &= (~(E1000_CTRL_TFCE | E1000_CTRL_RFCE));
		break;
	case e1000_fc_rx_pause:
		ctrl &= (~E1000_CTRL_TFCE);
		ctrl |= E1000_CTRL_RFCE;
		break;
	case e1000_fc_tx_pause:
		ctrl &= (~E1000_CTRL_RFCE);
		ctrl |= E1000_CTRL_TFCE;
		break;
	case e1000_fc_full:
		ctrl |= (E1000_CTRL_TFCE | E1000_CTRL_RFCE);
		break;
	default:
		DEBUGOUT("Flow control param set incorrectly\n");
		return -E1000_ERR_CONFIG;
	}

	/* Disable TX Flow Control for 82542 (rev 2.0) */
	if (hw->mac_type == e1000_82542_rev2_0)
		ctrl &= (~E1000_CTRL_TFCE);

	E1000_WRITE_REG(hw, CTRL, ctrl);
	return 0;
}

/******************************************************************************
 * Configures flow control settings after link is established
 *
 * hw - Struct containing variables accessed by shared code
 *
 * Should be called immediately after a valid link has been established.
 * Forces MAC flow control settings if link was forced. When in MII/GMII mode
 * and autonegotiation is enabled, the MAC flow control settings will be set
 * based on the flow control negotiated by the PHY. In TBI mode, the TFCE
 * and RFCE bits will be automaticaly set to the negotiated flow control mode.
 *****************************************************************************/
static int32_t
e1000_config_fc_after_link_up(struct e1000_hw *hw)
{
	int32_t ret_val;
	uint16_t mii_status_reg;
	uint16_t mii_nway_adv_reg;
	uint16_t mii_nway_lp_ability_reg;
	uint16_t speed;
	uint16_t duplex;

	DEBUGFUNC();

	/* Check for the case where we have fiber media and auto-neg failed
	 * so we had to force link.  In this case, we need to force the
	 * configuration of the MAC to match the "fc" parameter.
	 */
	if (((hw->media_type == e1000_media_type_fiber) && (hw->autoneg_failed))
		|| ((hw->media_type == e1000_media_type_internal_serdes)
		&& (hw->autoneg_failed))
		|| ((hw->media_type == e1000_media_type_copper)
		&& (!hw->autoneg))) {
		ret_val = e1000_force_mac_fc(hw);
		if (ret_val < 0) {
			DEBUGOUT("Error forcing flow control settings\n");
			return ret_val;
		}
	}

	/* Check for the case where we have copper media and auto-neg is
	 * enabled.  In this case, we need to check and see if Auto-Neg
	 * has completed, and if so, how the PHY and link partner has
	 * flow control configured.
	 */
	if (hw->media_type == e1000_media_type_copper) {
		/* Read the MII Status Register and check to see if AutoNeg
		 * has completed.  We read this twice because this reg has
		 * some "sticky" (latched) bits.
		 */
		if (e1000_read_phy_reg(hw, PHY_STATUS, &mii_status_reg) < 0) {
			DEBUGOUT("PHY Read Error\n");
			return -E1000_ERR_PHY;
		}
		if (e1000_read_phy_reg(hw, PHY_STATUS, &mii_status_reg) < 0) {
			DEBUGOUT("PHY Read Error\n");
			return -E1000_ERR_PHY;
		}

		if (mii_status_reg & MII_SR_AUTONEG_COMPLETE) {
			/* The AutoNeg process has completed, so we now need to
			 * read both the Auto Negotiation Advertisement Register
			 * (Address 4) and the Auto_Negotiation Base Page Ability
			 * Register (Address 5) to determine how flow control was
			 * negotiated.
			 */
			if (e1000_read_phy_reg
			    (hw, PHY_AUTONEG_ADV, &mii_nway_adv_reg) < 0) {
				DEBUGOUT("PHY Read Error\n");
				return -E1000_ERR_PHY;
			}
			if (e1000_read_phy_reg
			    (hw, PHY_LP_ABILITY,
			     &mii_nway_lp_ability_reg) < 0) {
				DEBUGOUT("PHY Read Error\n");
				return -E1000_ERR_PHY;
			}

			/* Two bits in the Auto Negotiation Advertisement Register
			 * (Address 4) and two bits in the Auto Negotiation Base
			 * Page Ability Register (Address 5) determine flow control
			 * for both the PHY and the link partner.  The following
			 * table, taken out of the IEEE 802.3ab/D6.0 dated March 25,
			 * 1999, describes these PAUSE resolution bits and how flow
			 * control is determined based upon these settings.
			 * NOTE:  DC = Don't Care
			 *
			 *   LOCAL DEVICE  |   LINK PARTNER
			 * PAUSE | ASM_DIR | PAUSE | ASM_DIR | NIC Resolution
			 *-------|---------|-------|---------|--------------------
			 *   0	 |    0    |  DC   |   DC    | e1000_fc_none
			 *   0	 |    1    |   0   |   DC    | e1000_fc_none
			 *   0	 |    1    |   1   |	0    | e1000_fc_none
			 *   0	 |    1    |   1   |	1    | e1000_fc_tx_pause
			 *   1	 |    0    |   0   |   DC    | e1000_fc_none
			 *   1	 |   DC    |   1   |   DC    | e1000_fc_full
			 *   1	 |    1    |   0   |	0    | e1000_fc_none
			 *   1	 |    1    |   0   |	1    | e1000_fc_rx_pause
			 *
			 */
			/* Are both PAUSE bits set to 1?  If so, this implies
			 * Symmetric Flow Control is enabled at both ends.  The
			 * ASM_DIR bits are irrelevant per the spec.
			 *
			 * For Symmetric Flow Control:
			 *
			 *   LOCAL DEVICE  |   LINK PARTNER
			 * PAUSE | ASM_DIR | PAUSE | ASM_DIR | Result
			 *-------|---------|-------|---------|--------------------
			 *   1	 |   DC    |   1   |   DC    | e1000_fc_full
			 *
			 */
			if ((mii_nway_adv_reg & NWAY_AR_PAUSE) &&
			    (mii_nway_lp_ability_reg & NWAY_LPAR_PAUSE)) {
				/* Now we need to check if the user selected RX ONLY
				 * of pause frames.  In this case, we had to advertise
				 * FULL flow control because we could not advertise RX
				 * ONLY. Hence, we must now check to see if we need to
				 * turn OFF  the TRANSMISSION of PAUSE frames.
				 */
				if (hw->original_fc == e1000_fc_full) {
					hw->fc = e1000_fc_full;
					DEBUGOUT("Flow Control = FULL.\r\n");
				} else {
					hw->fc = e1000_fc_rx_pause;
					DEBUGOUT
					    ("Flow Control = RX PAUSE frames only.\r\n");
				}
			}
			/* For receiving PAUSE frames ONLY.
			 *
			 *   LOCAL DEVICE  |   LINK PARTNER
			 * PAUSE | ASM_DIR | PAUSE | ASM_DIR | Result
			 *-------|---------|-------|---------|--------------------
			 *   0	 |    1    |   1   |	1    | e1000_fc_tx_pause
			 *
			 */
			else if (!(mii_nway_adv_reg & NWAY_AR_PAUSE) &&
				 (mii_nway_adv_reg & NWAY_AR_ASM_DIR) &&
				 (mii_nway_lp_ability_reg & NWAY_LPAR_PAUSE) &&
				 (mii_nway_lp_ability_reg & NWAY_LPAR_ASM_DIR))
			{
				hw->fc = e1000_fc_tx_pause;
				DEBUGOUT
				    ("Flow Control = TX PAUSE frames only.\r\n");
			}
			/* For transmitting PAUSE frames ONLY.
			 *
			 *   LOCAL DEVICE  |   LINK PARTNER
			 * PAUSE | ASM_DIR | PAUSE | ASM_DIR | Result
			 *-------|---------|-------|---------|--------------------
			 *   1	 |    1    |   0   |	1    | e1000_fc_rx_pause
			 *
			 */
			else if ((mii_nway_adv_reg & NWAY_AR_PAUSE) &&
				 (mii_nway_adv_reg & NWAY_AR_ASM_DIR) &&
				 !(mii_nway_lp_ability_reg & NWAY_LPAR_PAUSE) &&
				 (mii_nway_lp_ability_reg & NWAY_LPAR_ASM_DIR))
			{
				hw->fc = e1000_fc_rx_pause;
				DEBUGOUT
				    ("Flow Control = RX PAUSE frames only.\r\n");
			}
			/* Per the IEEE spec, at this point flow control should be
			 * disabled.  However, we want to consider that we could
			 * be connected to a legacy switch that doesn't advertise
			 * desired flow control, but can be forced on the link
			 * partner.  So if we advertised no flow control, that is
			 * what we will resolve to.  If we advertised some kind of
			 * receive capability (Rx Pause Only or Full Flow Control)
			 * and the link partner advertised none, we will configure
			 * ourselves to enable Rx Flow Control only.  We can do
			 * this safely for two reasons:  If the link partner really
			 * didn't want flow control enabled, and we enable Rx, no
			 * harm done since we won't be receiving any PAUSE frames
			 * anyway.  If the intent on the link partner was to have
			 * flow control enabled, then by us enabling RX only, we
			 * can at least receive pause frames and process them.
			 * This is a good idea because in most cases, since we are
			 * predominantly a server NIC, more times than not we will
			 * be asked to delay transmission of packets than asking
			 * our link partner to pause transmission of frames.
			 */
			else if (hw->original_fc == e1000_fc_none ||
				 hw->original_fc == e1000_fc_tx_pause) {
				hw->fc = e1000_fc_none;
				DEBUGOUT("Flow Control = NONE.\r\n");
			} else {
				hw->fc = e1000_fc_rx_pause;
				DEBUGOUT
				    ("Flow Control = RX PAUSE frames only.\r\n");
			}

			/* Now we need to do one last check...	If we auto-
			 * negotiated to HALF DUPLEX, flow control should not be
			 * enabled per IEEE 802.3 spec.
			 */
			e1000_get_speed_and_duplex(hw, &speed, &duplex);

			if (duplex == HALF_DUPLEX)
				hw->fc = e1000_fc_none;

			/* Now we call a subroutine to actually force the MAC
			 * controller to use the correct flow control settings.
			 */
			ret_val = e1000_force_mac_fc(hw);
			if (ret_val < 0) {
				DEBUGOUT
				    ("Error forcing flow control settings\n");
				return ret_val;
			}
		} else {
			DEBUGOUT
			    ("Copper PHY and Auto Neg has not completed.\r\n");
		}
	}
	return E1000_SUCCESS;
}

/******************************************************************************
 * Checks to see if the link status of the hardware has changed.
 *
 * hw - Struct containing variables accessed by shared code
 *
 * Called by any function that needs to check the link status of the adapter.
 *****************************************************************************/
static int
e1000_check_for_link(struct e1000_hw *hw)
{
	uint32_t rxcw;
	uint32_t ctrl;
	uint32_t status;
	uint32_t rctl;
	uint32_t signal;
	int32_t ret_val;
	uint16_t phy_data;
	uint16_t lp_capability;

	DEBUGFUNC();

	/* On adapters with a MAC newer that 82544, SW Defineable pin 1 will be
	 * set when the optics detect a signal. On older adapters, it will be
	 * cleared when there is a signal
	 */
	ctrl = E1000_READ_REG(hw, CTRL);
	if ((hw->mac_type > e1000_82544) && !(ctrl & E1000_CTRL_ILOS))
		signal = E1000_CTRL_SWDPIN1;
	else
		signal = 0;

	status = E1000_READ_REG(hw, STATUS);
	rxcw = E1000_READ_REG(hw, RXCW);
	DEBUGOUT("ctrl: %#08x status %#08x rxcw %#08x\n", ctrl, status, rxcw);

	/* If we have a copper PHY then we only want to go out to the PHY
	 * registers to see if Auto-Neg has completed and/or if our link
	 * status has changed.	The get_link_status flag will be set if we
	 * receive a Link Status Change interrupt or we have Rx Sequence
	 * Errors.
	 */
	if ((hw->media_type == e1000_media_type_copper) && hw->get_link_status) {
		/* First we want to see if the MII Status Register reports
		 * link.  If so, then we want to get the current speed/duplex
		 * of the PHY.
		 * Read the register twice since the link bit is sticky.
		 */
		if (e1000_read_phy_reg(hw, PHY_STATUS, &phy_data) < 0) {
			DEBUGOUT("PHY Read Error\n");
			return -E1000_ERR_PHY;
		}
		if (e1000_read_phy_reg(hw, PHY_STATUS, &phy_data) < 0) {
			DEBUGOUT("PHY Read Error\n");
			return -E1000_ERR_PHY;
		}

		if (phy_data & MII_SR_LINK_STATUS) {
			hw->get_link_status = false;
		} else {
			/* No link detected */
			return -E1000_ERR_NOLINK;
		}

		/* We have a M88E1000 PHY and Auto-Neg is enabled.  If we
		 * have Si on board that is 82544 or newer, Auto
		 * Speed Detection takes care of MAC speed/duplex
		 * configuration.  So we only need to configure Collision
		 * Distance in the MAC.  Otherwise, we need to force
		 * speed/duplex on the MAC to the current PHY speed/duplex
		 * settings.
		 */
		if (hw->mac_type >= e1000_82544)
			e1000_config_collision_dist(hw);
		else {
			ret_val = e1000_config_mac_to_phy(hw);
			if (ret_val < 0) {
				DEBUGOUT
				    ("Error configuring MAC to PHY settings\n");
				return ret_val;
			}
		}

		/* Configure Flow Control now that Auto-Neg has completed. First, we
		 * need to restore the desired flow control settings because we may
		 * have had to re-autoneg with a different link partner.
		 */
		ret_val = e1000_config_fc_after_link_up(hw);
		if (ret_val < 0) {
			DEBUGOUT("Error configuring flow control\n");
			return ret_val;
		}

		/* At this point we know that we are on copper and we have
		 * auto-negotiated link.  These are conditions for checking the link
		 * parter capability register.	We use the link partner capability to
		 * determine if TBI Compatibility needs to be turned on or off.  If
		 * the link partner advertises any speed in addition to Gigabit, then
		 * we assume that they are GMII-based, and TBI compatibility is not
		 * needed. If no other speeds are advertised, we assume the link
		 * partner is TBI-based, and we turn on TBI Compatibility.
		 */
		if (hw->tbi_compatibility_en) {
			if (e1000_read_phy_reg
			    (hw, PHY_LP_ABILITY, &lp_capability) < 0) {
				DEBUGOUT("PHY Read Error\n");
				return -E1000_ERR_PHY;
			}
			if (lp_capability & (NWAY_LPAR_10T_HD_CAPS |
					     NWAY_LPAR_10T_FD_CAPS |
					     NWAY_LPAR_100TX_HD_CAPS |
					     NWAY_LPAR_100TX_FD_CAPS |
					     NWAY_LPAR_100T4_CAPS)) {
				/* If our link partner advertises anything in addition to
				 * gigabit, we do not need to enable TBI compatibility.
				 */
				if (hw->tbi_compatibility_on) {
					/* If we previously were in the mode, turn it off. */
					rctl = E1000_READ_REG(hw, RCTL);
					rctl &= ~E1000_RCTL_SBP;
					E1000_WRITE_REG(hw, RCTL, rctl);
					hw->tbi_compatibility_on = false;
				}
			} else {
				/* If TBI compatibility is was previously off, turn it on. For
				 * compatibility with a TBI link partner, we will store bad
				 * packets. Some frames have an additional byte on the end and
				 * will look like CRC errors to to the hardware.
				 */
				if (!hw->tbi_compatibility_on) {
					hw->tbi_compatibility_on = true;
					rctl = E1000_READ_REG(hw, RCTL);
					rctl |= E1000_RCTL_SBP;
					E1000_WRITE_REG(hw, RCTL, rctl);
				}
			}
		}
	}
	/* If we don't have link (auto-negotiation failed or link partner cannot
	 * auto-negotiate), the cable is plugged in (we have signal), and our
	 * link partner is not trying to auto-negotiate with us (we are receiving
	 * idles or data), we need to force link up. We also need to give
	 * auto-negotiation time to complete, in case the cable was just plugged
	 * in. The autoneg_failed flag does this.
	 */
	else if ((hw->media_type == e1000_media_type_fiber) &&
		 (!(status & E1000_STATUS_LU)) &&
		 ((ctrl & E1000_CTRL_SWDPIN1) == signal) &&
		 (!(rxcw & E1000_RXCW_C))) {
		if (hw->autoneg_failed == 0) {
			hw->autoneg_failed = 1;
			return 0;
		}
		DEBUGOUT("NOT RXing /C/, disable AutoNeg and force link.\r\n");

		/* Disable auto-negotiation in the TXCW register */
		E1000_WRITE_REG(hw, TXCW, (hw->txcw & ~E1000_TXCW_ANE));

		/* Force link-up and also force full-duplex. */
		ctrl = E1000_READ_REG(hw, CTRL);
		ctrl |= (E1000_CTRL_SLU | E1000_CTRL_FD);
		E1000_WRITE_REG(hw, CTRL, ctrl);

		/* Configure Flow Control after forcing link up. */
		ret_val = e1000_config_fc_after_link_up(hw);
		if (ret_val < 0) {
			DEBUGOUT("Error configuring flow control\n");
			return ret_val;
		}
	}
	/* If we are forcing link and we are receiving /C/ ordered sets, re-enable
	 * auto-negotiation in the TXCW register and disable forced link in the
	 * Device Control register in an attempt to auto-negotiate with our link
	 * partner.
	 */
	else if ((hw->media_type == e1000_media_type_fiber) &&
		 (ctrl & E1000_CTRL_SLU) && (rxcw & E1000_RXCW_C)) {
		DEBUGOUT
		    ("RXing /C/, enable AutoNeg and stop forcing link.\r\n");
		E1000_WRITE_REG(hw, TXCW, hw->txcw);
		E1000_WRITE_REG(hw, CTRL, (ctrl & ~E1000_CTRL_SLU));
	}
	return 0;
}

/******************************************************************************
* Configure the MAC-to-PHY interface for 10/100Mbps
*
* hw - Struct containing variables accessed by shared code
******************************************************************************/
static int32_t
e1000_configure_kmrn_for_10_100(struct e1000_hw *hw, uint16_t duplex)
{
	int32_t ret_val = E1000_SUCCESS;
	uint32_t tipg;
	uint16_t reg_data;

	DEBUGFUNC();

	reg_data = E1000_KUMCTRLSTA_HD_CTRL_10_100_DEFAULT;
	ret_val = e1000_write_kmrn_reg(hw,
			E1000_KUMCTRLSTA_OFFSET_HD_CTRL, reg_data);
	if (ret_val)
		return ret_val;

	/* Configure Transmit Inter-Packet Gap */
	tipg = E1000_READ_REG(hw, TIPG);
	tipg &= ~E1000_TIPG_IPGT_MASK;
	tipg |= DEFAULT_80003ES2LAN_TIPG_IPGT_10_100;
	E1000_WRITE_REG(hw, TIPG, tipg);

	ret_val = e1000_read_phy_reg(hw, GG82563_PHY_KMRN_MODE_CTRL, &reg_data);

	if (ret_val)
		return ret_val;

	if (duplex == HALF_DUPLEX)
		reg_data |= GG82563_KMCR_PASS_FALSE_CARRIER;
	else
		reg_data &= ~GG82563_KMCR_PASS_FALSE_CARRIER;

	ret_val = e1000_write_phy_reg(hw, GG82563_PHY_KMRN_MODE_CTRL, reg_data);

	return ret_val;
}

static int32_t
e1000_configure_kmrn_for_1000(struct e1000_hw *hw)
{
	int32_t ret_val = E1000_SUCCESS;
	uint16_t reg_data;
	uint32_t tipg;

	DEBUGFUNC();

	reg_data = E1000_KUMCTRLSTA_HD_CTRL_1000_DEFAULT;
	ret_val = e1000_write_kmrn_reg(hw,
			E1000_KUMCTRLSTA_OFFSET_HD_CTRL, reg_data);
	if (ret_val)
		return ret_val;

	/* Configure Transmit Inter-Packet Gap */
	tipg = E1000_READ_REG(hw, TIPG);
	tipg &= ~E1000_TIPG_IPGT_MASK;
	tipg |= DEFAULT_80003ES2LAN_TIPG_IPGT_1000;
	E1000_WRITE_REG(hw, TIPG, tipg);

	ret_val = e1000_read_phy_reg(hw, GG82563_PHY_KMRN_MODE_CTRL, &reg_data);

	if (ret_val)
		return ret_val;

	reg_data &= ~GG82563_KMCR_PASS_FALSE_CARRIER;
	ret_val = e1000_write_phy_reg(hw, GG82563_PHY_KMRN_MODE_CTRL, reg_data);

	return ret_val;
}

/******************************************************************************
 * Detects the current speed and duplex settings of the hardware.
 *
 * hw - Struct containing variables accessed by shared code
 * speed - Speed of the connection
 * duplex - Duplex setting of the connection
 *****************************************************************************/
static int
e1000_get_speed_and_duplex(struct e1000_hw *hw, uint16_t *speed,
		uint16_t *duplex)
{
	uint32_t status;
	int32_t ret_val;
	uint16_t phy_data;

	DEBUGFUNC();

	if (hw->mac_type >= e1000_82543) {
		status = E1000_READ_REG(hw, STATUS);
		if (status & E1000_STATUS_SPEED_1000) {
			*speed = SPEED_1000;
			DEBUGOUT("1000 Mbs, ");
		} else if (status & E1000_STATUS_SPEED_100) {
			*speed = SPEED_100;
			DEBUGOUT("100 Mbs, ");
		} else {
			*speed = SPEED_10;
			DEBUGOUT("10 Mbs, ");
		}

		if (status & E1000_STATUS_FD) {
			*duplex = FULL_DUPLEX;
			DEBUGOUT("Full Duplex\r\n");
		} else {
			*duplex = HALF_DUPLEX;
			DEBUGOUT(" Half Duplex\r\n");
		}
	} else {
		DEBUGOUT("1000 Mbs, Full Duplex\r\n");
		*speed = SPEED_1000;
		*duplex = FULL_DUPLEX;
	}

	/* IGP01 PHY may advertise full duplex operation after speed downgrade
	 * even if it is operating at half duplex.  Here we set the duplex
	 * settings to match the duplex in the link partner's capabilities.
	 */
	if (hw->phy_type == e1000_phy_igp && hw->speed_downgraded) {
		ret_val = e1000_read_phy_reg(hw, PHY_AUTONEG_EXP, &phy_data);
		if (ret_val)
			return ret_val;

		if (!(phy_data & NWAY_ER_LP_NWAY_CAPS))
			*duplex = HALF_DUPLEX;
		else {
			ret_val = e1000_read_phy_reg(hw,
					PHY_LP_ABILITY, &phy_data);
			if (ret_val)
				return ret_val;
			if ((*speed == SPEED_100 &&
				!(phy_data & NWAY_LPAR_100TX_FD_CAPS))
				|| (*speed == SPEED_10
				&& !(phy_data & NWAY_LPAR_10T_FD_CAPS)))
				*duplex = HALF_DUPLEX;
		}
	}

	if ((hw->mac_type == e1000_80003es2lan) &&
		(hw->media_type == e1000_media_type_copper)) {
		if (*speed == SPEED_1000)
			ret_val = e1000_configure_kmrn_for_1000(hw);
		else
			ret_val = e1000_configure_kmrn_for_10_100(hw, *duplex);
		if (ret_val)
			return ret_val;
	}
	return E1000_SUCCESS;
}

/******************************************************************************
* Blocks until autoneg completes or times out (~4.5 seconds)
*
* hw - Struct containing variables accessed by shared code
******************************************************************************/
static int
e1000_wait_autoneg(struct e1000_hw *hw)
{
	uint16_t i;
	uint16_t phy_data;

	DEBUGFUNC();
	DEBUGOUT("Waiting for Auto-Neg to complete.\n");

	/* We will wait for autoneg to complete or timeout to expire. */
	for (i = PHY_AUTO_NEG_TIME; i > 0; i--) {
		/* Read the MII Status Register and wait for Auto-Neg
		 * Complete bit to be set.
		 */
		if (e1000_read_phy_reg(hw, PHY_STATUS, &phy_data) < 0) {
			DEBUGOUT("PHY Read Error\n");
			return -E1000_ERR_PHY;
		}
		if (e1000_read_phy_reg(hw, PHY_STATUS, &phy_data) < 0) {
			DEBUGOUT("PHY Read Error\n");
			return -E1000_ERR_PHY;
		}
		if (phy_data & MII_SR_AUTONEG_COMPLETE) {
			DEBUGOUT("Auto-Neg complete.\n");
			return 0;
		}
		mdelay(100);
	}
	DEBUGOUT("Auto-Neg timedout.\n");
	return -E1000_ERR_TIMEOUT;
}

/******************************************************************************
* Raises the Management Data Clock
*
* hw - Struct containing variables accessed by shared code
* ctrl - Device control register's current value
******************************************************************************/
static void
e1000_raise_mdi_clk(struct e1000_hw *hw, uint32_t * ctrl)
{
	/* Raise the clock input to the Management Data Clock (by setting the MDC
	 * bit), and then delay 2 microseconds.
	 */
	E1000_WRITE_REG(hw, CTRL, (*ctrl | E1000_CTRL_MDC));
	E1000_WRITE_FLUSH(hw);
	udelay(2);
}

/******************************************************************************
* Lowers the Management Data Clock
*
* hw - Struct containing variables accessed by shared code
* ctrl - Device control register's current value
******************************************************************************/
static void
e1000_lower_mdi_clk(struct e1000_hw *hw, uint32_t * ctrl)
{
	/* Lower the clock input to the Management Data Clock (by clearing the MDC
	 * bit), and then delay 2 microseconds.
	 */
	E1000_WRITE_REG(hw, CTRL, (*ctrl & ~E1000_CTRL_MDC));
	E1000_WRITE_FLUSH(hw);
	udelay(2);
}

/******************************************************************************
* Shifts data bits out to the PHY
*
* hw - Struct containing variables accessed by shared code
* data - Data to send out to the PHY
* count - Number of bits to shift out
*
* Bits are shifted out in MSB to LSB order.
******************************************************************************/
static void
e1000_shift_out_mdi_bits(struct e1000_hw *hw, uint32_t data, uint16_t count)
{
	uint32_t ctrl;
	uint32_t mask;

	/* We need to shift "count" number of bits out to the PHY. So, the value
	 * in the "data" parameter will be shifted out to the PHY one bit at a
	 * time. In order to do this, "data" must be broken down into bits.
	 */
	mask = 0x01;
	mask <<= (count - 1);

	ctrl = E1000_READ_REG(hw, CTRL);

	/* Set MDIO_DIR and MDC_DIR direction bits to be used as output pins. */
	ctrl |= (E1000_CTRL_MDIO_DIR | E1000_CTRL_MDC_DIR);

	while (mask) {
		/* A "1" is shifted out to the PHY by setting the MDIO bit to "1" and
		 * then raising and lowering the Management Data Clock. A "0" is
		 * shifted out to the PHY by setting the MDIO bit to "0" and then
		 * raising and lowering the clock.
		 */
		if (data & mask)
			ctrl |= E1000_CTRL_MDIO;
		else
			ctrl &= ~E1000_CTRL_MDIO;

		E1000_WRITE_REG(hw, CTRL, ctrl);
		E1000_WRITE_FLUSH(hw);

		udelay(2);

		e1000_raise_mdi_clk(hw, &ctrl);
		e1000_lower_mdi_clk(hw, &ctrl);

		mask = mask >> 1;
	}
}

/******************************************************************************
* Shifts data bits in from the PHY
*
* hw - Struct containing variables accessed by shared code
*
* Bits are shifted in in MSB to LSB order.
******************************************************************************/
static uint16_t
e1000_shift_in_mdi_bits(struct e1000_hw *hw)
{
	uint32_t ctrl;
	uint16_t data = 0;
	uint8_t i;

	/* In order to read a register from the PHY, we need to shift in a total
	 * of 18 bits from the PHY. The first two bit (turnaround) times are used
	 * to avoid contention on the MDIO pin when a read operation is performed.
	 * These two bits are ignored by us and thrown away. Bits are "shifted in"
	 * by raising the input to the Management Data Clock (setting the MDC bit),
	 * and then reading the value of the MDIO bit.
	 */
	ctrl = E1000_READ_REG(hw, CTRL);

	/* Clear MDIO_DIR (SWDPIO1) to indicate this bit is to be used as input. */
	ctrl &= ~E1000_CTRL_MDIO_DIR;
	ctrl &= ~E1000_CTRL_MDIO;

	E1000_WRITE_REG(hw, CTRL, ctrl);
	E1000_WRITE_FLUSH(hw);

	/* Raise and Lower the clock before reading in the data. This accounts for
	 * the turnaround bits. The first clock occurred when we clocked out the
	 * last bit of the Register Address.
	 */
	e1000_raise_mdi_clk(hw, &ctrl);
	e1000_lower_mdi_clk(hw, &ctrl);

	for (data = 0, i = 0; i < 16; i++) {
		data = data << 1;
		e1000_raise_mdi_clk(hw, &ctrl);
		ctrl = E1000_READ_REG(hw, CTRL);
		/* Check to see if we shifted in a "1". */
		if (ctrl & E1000_CTRL_MDIO)
			data |= 1;
		e1000_lower_mdi_clk(hw, &ctrl);
	}

	e1000_raise_mdi_clk(hw, &ctrl);
	e1000_lower_mdi_clk(hw, &ctrl);

	return data;
}

/*****************************************************************************
* Reads the value from a PHY register
*
* hw - Struct containing variables accessed by shared code
* reg_addr - address of the PHY register to read
******************************************************************************/
static int
e1000_read_phy_reg(struct e1000_hw *hw, uint32_t reg_addr, uint16_t * phy_data)
{
	uint32_t i;
	uint32_t mdic = 0;
	const uint32_t phy_addr = 1;

	if (reg_addr > MAX_PHY_REG_ADDRESS) {
		DEBUGOUT("PHY Address %d is out of range\n", reg_addr);
		return -E1000_ERR_PARAM;
	}

	if (hw->mac_type > e1000_82543) {
		/* Set up Op-code, Phy Address, and register address in the MDI
		 * Control register.  The MAC will take care of interfacing with the
		 * PHY to retrieve the desired data.
		 */
		mdic = ((reg_addr << E1000_MDIC_REG_SHIFT) |
			(phy_addr << E1000_MDIC_PHY_SHIFT) |
			(E1000_MDIC_OP_READ));

		E1000_WRITE_REG(hw, MDIC, mdic);

		/* Poll the ready bit to see if the MDI read completed */
		for (i = 0; i < 64; i++) {
			udelay(10);
			mdic = E1000_READ_REG(hw, MDIC);
			if (mdic & E1000_MDIC_READY)
				break;
		}
		if (!(mdic & E1000_MDIC_READY)) {
			DEBUGOUT("MDI Read did not complete\n");
			return -E1000_ERR_PHY;
		}
		if (mdic & E1000_MDIC_ERROR) {
			DEBUGOUT("MDI Error\n");
			return -E1000_ERR_PHY;
		}
		*phy_data = (uint16_t) mdic;
	} else {
		/* We must first send a preamble through the MDIO pin to signal the
		 * beginning of an MII instruction.  This is done by sending 32
		 * consecutive "1" bits.
		 */
		e1000_shift_out_mdi_bits(hw, PHY_PREAMBLE, PHY_PREAMBLE_SIZE);

		/* Now combine the next few fields that are required for a read
		 * operation.  We use this method instead of calling the
		 * e1000_shift_out_mdi_bits routine five different times. The format of
		 * a MII read instruction consists of a shift out of 14 bits and is
		 * defined as follows:
		 *    <Preamble><SOF><Op Code><Phy Addr><Reg Addr>
		 * followed by a shift in of 18 bits.  This first two bits shifted in
		 * are TurnAround bits used to avoid contention on the MDIO pin when a
		 * READ operation is performed.  These two bits are thrown away
		 * followed by a shift in of 16 bits which contains the desired data.
		 */
		mdic = ((reg_addr) | (phy_addr << 5) |
			(PHY_OP_READ << 10) | (PHY_SOF << 12));

		e1000_shift_out_mdi_bits(hw, mdic, 14);

		/* Now that we've shifted out the read command to the MII, we need to
		 * "shift in" the 16-bit value (18 total bits) of the requested PHY
		 * register address.
		 */
		*phy_data = e1000_shift_in_mdi_bits(hw);
	}
	return 0;
}

/******************************************************************************
* Writes a value to a PHY register
*
* hw - Struct containing variables accessed by shared code
* reg_addr - address of the PHY register to write
* data - data to write to the PHY
******************************************************************************/
static int
e1000_write_phy_reg(struct e1000_hw *hw, uint32_t reg_addr, uint16_t phy_data)
{
	uint32_t i;
	uint32_t mdic = 0;
	const uint32_t phy_addr = 1;

	if (reg_addr > MAX_PHY_REG_ADDRESS) {
		DEBUGOUT("PHY Address %d is out of range\n", reg_addr);
		return -E1000_ERR_PARAM;
	}

	if (hw->mac_type > e1000_82543) {
		/* Set up Op-code, Phy Address, register address, and data intended
		 * for the PHY register in the MDI Control register.  The MAC will take
		 * care of interfacing with the PHY to send the desired data.
		 */
		mdic = (((uint32_t) phy_data) |
			(reg_addr << E1000_MDIC_REG_SHIFT) |
			(phy_addr << E1000_MDIC_PHY_SHIFT) |
			(E1000_MDIC_OP_WRITE));

		E1000_WRITE_REG(hw, MDIC, mdic);

		/* Poll the ready bit to see if the MDI read completed */
		for (i = 0; i < 64; i++) {
			udelay(10);
			mdic = E1000_READ_REG(hw, MDIC);
			if (mdic & E1000_MDIC_READY)
				break;
		}
		if (!(mdic & E1000_MDIC_READY)) {
			DEBUGOUT("MDI Write did not complete\n");
			return -E1000_ERR_PHY;
		}
	} else {
		/* We'll need to use the SW defined pins to shift the write command
		 * out to the PHY. We first send a preamble to the PHY to signal the
		 * beginning of the MII instruction.  This is done by sending 32
		 * consecutive "1" bits.
		 */
		e1000_shift_out_mdi_bits(hw, PHY_PREAMBLE, PHY_PREAMBLE_SIZE);

		/* Now combine the remaining required fields that will indicate a
		 * write operation. We use this method instead of calling the
		 * e1000_shift_out_mdi_bits routine for each field in the command. The
		 * format of a MII write instruction is as follows:
		 * <Preamble><SOF><Op Code><Phy Addr><Reg Addr><Turnaround><Data>.
		 */
		mdic = ((PHY_TURNAROUND) | (reg_addr << 2) | (phy_addr << 7) |
			(PHY_OP_WRITE << 12) | (PHY_SOF << 14));
		mdic <<= 16;
		mdic |= (uint32_t) phy_data;

		e1000_shift_out_mdi_bits(hw, mdic, 32);
	}
	return 0;
}

/******************************************************************************
 * Checks if PHY reset is blocked due to SOL/IDER session, for example.
 * Returning E1000_BLK_PHY_RESET isn't necessarily an error.  But it's up to
 * the caller to figure out how to deal with it.
 *
 * hw - Struct containing variables accessed by shared code
 *
 * returns: - E1000_BLK_PHY_RESET
 *            E1000_SUCCESS
 *
 *****************************************************************************/
int32_t
e1000_check_phy_reset_block(struct e1000_hw *hw)
{
	uint32_t manc = 0;
	uint32_t fwsm = 0;

	if (hw->mac_type == e1000_ich8lan) {
		fwsm = E1000_READ_REG(hw, FWSM);
		return (fwsm & E1000_FWSM_RSPCIPHY) ? E1000_SUCCESS
						: E1000_BLK_PHY_RESET;
	}

	if (hw->mac_type > e1000_82547_rev_2)
		manc = E1000_READ_REG(hw, MANC);
	return (manc & E1000_MANC_BLK_PHY_RST_ON_IDE) ?
		E1000_BLK_PHY_RESET : E1000_SUCCESS;
}

/***************************************************************************
 * Checks if the PHY configuration is done
 *
 * hw: Struct containing variables accessed by shared code
 *
 * returns: - E1000_ERR_RESET if fail to reset MAC
 *            E1000_SUCCESS at any other case.
 *
 ***************************************************************************/
static int32_t
e1000_get_phy_cfg_done(struct e1000_hw *hw)
{
	int32_t timeout = PHY_CFG_TIMEOUT;
	uint32_t cfg_mask = E1000_EEPROM_CFG_DONE;

	DEBUGFUNC();

	switch (hw->mac_type) {
	default:
		mdelay(10);
		break;

	case e1000_80003es2lan:
		/* Separate *_CFG_DONE_* bit for each port */
		if (e1000_is_second_port(hw))
			cfg_mask = E1000_EEPROM_CFG_DONE_PORT_1;
		/* Fall Through */

	case e1000_82571:
	case e1000_82572:
	case e1000_igb:
		while (timeout) {
			if (hw->mac_type == e1000_igb) {
				if (E1000_READ_REG(hw, I210_EEMNGCTL) & cfg_mask)
					break;
			} else {
				if (E1000_READ_REG(hw, EEMNGCTL) & cfg_mask)
					break;
			}
			mdelay(1);
			timeout--;
		}
		if (!timeout) {
			DEBUGOUT("MNG configuration cycle has not "
					"completed.\n");
			return -E1000_ERR_RESET;
		}
		break;
	}

	return E1000_SUCCESS;
}

/******************************************************************************
* Returns the PHY to the power-on reset state
*
* hw - Struct containing variables accessed by shared code
******************************************************************************/
int32_t
e1000_phy_hw_reset(struct e1000_hw *hw)
{
	uint16_t swfw = E1000_SWFW_PHY0_SM;
	uint32_t ctrl, ctrl_ext;
	uint32_t led_ctrl;
	int32_t ret_val;

	DEBUGFUNC();

	/* In the case of the phy reset being blocked, it's not an error, we
	 * simply return success without performing the reset. */
	ret_val = e1000_check_phy_reset_block(hw);
	if (ret_val)
		return E1000_SUCCESS;

	DEBUGOUT("Resetting Phy...\n");

	if (hw->mac_type > e1000_82543) {
		if (e1000_is_second_port(hw))
			swfw = E1000_SWFW_PHY1_SM;

		if (e1000_swfw_sync_acquire(hw, swfw)) {
			DEBUGOUT("Unable to acquire swfw sync\n");
			return -E1000_ERR_SWFW_SYNC;
		}

		/* Read the device control register and assert the E1000_CTRL_PHY_RST
		 * bit. Then, take it out of reset.
		 */
		ctrl = E1000_READ_REG(hw, CTRL);
		E1000_WRITE_REG(hw, CTRL, ctrl | E1000_CTRL_PHY_RST);
		E1000_WRITE_FLUSH(hw);

		if (hw->mac_type < e1000_82571)
			udelay(10);
		else
			udelay(100);

		E1000_WRITE_REG(hw, CTRL, ctrl);
		E1000_WRITE_FLUSH(hw);

		if (hw->mac_type >= e1000_82571)
			mdelay(10);

	} else {
		/* Read the Extended Device Control Register, assert the PHY_RESET_DIR
		 * bit to put the PHY into reset. Then, take it out of reset.
		 */
		ctrl_ext = E1000_READ_REG(hw, CTRL_EXT);
		ctrl_ext |= E1000_CTRL_EXT_SDP4_DIR;
		ctrl_ext &= ~E1000_CTRL_EXT_SDP4_DATA;
		E1000_WRITE_REG(hw, CTRL_EXT, ctrl_ext);
		E1000_WRITE_FLUSH(hw);
		mdelay(10);
		ctrl_ext |= E1000_CTRL_EXT_SDP4_DATA;
		E1000_WRITE_REG(hw, CTRL_EXT, ctrl_ext);
		E1000_WRITE_FLUSH(hw);
	}
	udelay(150);

	if ((hw->mac_type == e1000_82541) || (hw->mac_type == e1000_82547)) {
		/* Configure activity LED after PHY reset */
		led_ctrl = E1000_READ_REG(hw, LEDCTL);
		led_ctrl &= IGP_ACTIVITY_LED_MASK;
		led_ctrl |= (IGP_ACTIVITY_LED_ENABLE | IGP_LED3_MODE);
		E1000_WRITE_REG(hw, LEDCTL, led_ctrl);
	}

	e1000_swfw_sync_release(hw, swfw);

	/* Wait for FW to finish PHY configuration. */
	ret_val = e1000_get_phy_cfg_done(hw);
	if (ret_val != E1000_SUCCESS)
		return ret_val;

	return ret_val;
}

/******************************************************************************
 * IGP phy init script - initializes the GbE PHY
 *
 * hw - Struct containing variables accessed by shared code
 *****************************************************************************/
static void
e1000_phy_init_script(struct e1000_hw *hw)
{
	uint32_t ret_val;
	uint16_t phy_saved_data;
	DEBUGFUNC();

	if (hw->phy_init_script) {
		mdelay(20);

		/* Save off the current value of register 0x2F5B to be
		 * restored at the end of this routine. */
		ret_val = e1000_read_phy_reg(hw, 0x2F5B, &phy_saved_data);

		/* Disabled the PHY transmitter */
		e1000_write_phy_reg(hw, 0x2F5B, 0x0003);

		mdelay(20);

		e1000_write_phy_reg(hw, 0x0000, 0x0140);

		mdelay(5);

		switch (hw->mac_type) {
		case e1000_82541:
		case e1000_82547:
			e1000_write_phy_reg(hw, 0x1F95, 0x0001);

			e1000_write_phy_reg(hw, 0x1F71, 0xBD21);

			e1000_write_phy_reg(hw, 0x1F79, 0x0018);

			e1000_write_phy_reg(hw, 0x1F30, 0x1600);

			e1000_write_phy_reg(hw, 0x1F31, 0x0014);

			e1000_write_phy_reg(hw, 0x1F32, 0x161C);

			e1000_write_phy_reg(hw, 0x1F94, 0x0003);

			e1000_write_phy_reg(hw, 0x1F96, 0x003F);

			e1000_write_phy_reg(hw, 0x2010, 0x0008);
			break;

		case e1000_82541_rev_2:
		case e1000_82547_rev_2:
			e1000_write_phy_reg(hw, 0x1F73, 0x0099);
			break;
		default:
			break;
		}

		e1000_write_phy_reg(hw, 0x0000, 0x3300);

		mdelay(20);

		/* Now enable the transmitter */
		if (!ret_val)
			e1000_write_phy_reg(hw, 0x2F5B, phy_saved_data);

		if (hw->mac_type == e1000_82547) {
			uint16_t fused, fine, coarse;

			/* Move to analog registers page */
			e1000_read_phy_reg(hw,
				IGP01E1000_ANALOG_SPARE_FUSE_STATUS, &fused);

			if (!(fused & IGP01E1000_ANALOG_SPARE_FUSE_ENABLED)) {
				e1000_read_phy_reg(hw,
					IGP01E1000_ANALOG_FUSE_STATUS, &fused);

				fine = fused & IGP01E1000_ANALOG_FUSE_FINE_MASK;
				coarse = fused
					& IGP01E1000_ANALOG_FUSE_COARSE_MASK;

				if (coarse >
					IGP01E1000_ANALOG_FUSE_COARSE_THRESH) {
					coarse -=
					IGP01E1000_ANALOG_FUSE_COARSE_10;
					fine -= IGP01E1000_ANALOG_FUSE_FINE_1;
				} else if (coarse
					== IGP01E1000_ANALOG_FUSE_COARSE_THRESH)
					fine -= IGP01E1000_ANALOG_FUSE_FINE_10;

				fused = (fused
					& IGP01E1000_ANALOG_FUSE_POLY_MASK) |
					(fine
					& IGP01E1000_ANALOG_FUSE_FINE_MASK) |
					(coarse
					& IGP01E1000_ANALOG_FUSE_COARSE_MASK);

				e1000_write_phy_reg(hw,
					IGP01E1000_ANALOG_FUSE_CONTROL, fused);
				e1000_write_phy_reg(hw,
					IGP01E1000_ANALOG_FUSE_BYPASS,
				IGP01E1000_ANALOG_FUSE_ENABLE_SW_CONTROL);
			}
		}
	}
}

/******************************************************************************
* Resets the PHY
*
* hw - Struct containing variables accessed by shared code
*
* Sets bit 15 of the MII Control register
******************************************************************************/
int32_t
e1000_phy_reset(struct e1000_hw *hw)
{
	int32_t ret_val;
	uint16_t phy_data;

	DEBUGFUNC();

	/* In the case of the phy reset being blocked, it's not an error, we
	 * simply return success without performing the reset. */
	ret_val = e1000_check_phy_reset_block(hw);
	if (ret_val)
		return E1000_SUCCESS;

	switch (hw->phy_type) {
	case e1000_phy_igp:
	case e1000_phy_igp_2:
	case e1000_phy_igp_3:
	case e1000_phy_ife:
	case e1000_phy_igb:
		ret_val = e1000_phy_hw_reset(hw);
		if (ret_val)
			return ret_val;
		break;
	default:
		ret_val = e1000_read_phy_reg(hw, PHY_CTRL, &phy_data);
		if (ret_val)
			return ret_val;

		phy_data |= MII_CR_RESET;
		ret_val = e1000_write_phy_reg(hw, PHY_CTRL, phy_data);
		if (ret_val)
			return ret_val;

		udelay(1);
		break;
	}

	if (hw->phy_type == e1000_phy_igp || hw->phy_type == e1000_phy_igp_2)
		e1000_phy_init_script(hw);

	return E1000_SUCCESS;
}

static int e1000_set_phy_type (struct e1000_hw *hw)
{
	DEBUGFUNC ();

	if (hw->mac_type == e1000_undefined)
		return -E1000_ERR_PHY_TYPE;

	switch (hw->phy_id) {
	case M88E1000_E_PHY_ID:
	case M88E1000_I_PHY_ID:
	case M88E1011_I_PHY_ID:
	case M88E1111_I_PHY_ID:
		hw->phy_type = e1000_phy_m88;
		break;
	case IGP01E1000_I_PHY_ID:
		if (hw->mac_type == e1000_82541 ||
			hw->mac_type == e1000_82541_rev_2 ||
			hw->mac_type == e1000_82547 ||
			hw->mac_type == e1000_82547_rev_2) {
			hw->phy_type = e1000_phy_igp;
			break;
		}
	case IGP03E1000_E_PHY_ID:
		hw->phy_type = e1000_phy_igp_3;
		break;
	case IFE_E_PHY_ID:
	case IFE_PLUS_E_PHY_ID:
	case IFE_C_E_PHY_ID:
		hw->phy_type = e1000_phy_ife;
		break;
	case GG82563_E_PHY_ID:
		if (hw->mac_type == e1000_80003es2lan) {
			hw->phy_type = e1000_phy_gg82563;
			break;
		}
	case BME1000_E_PHY_ID:
		hw->phy_type = e1000_phy_bm;
		break;
	case I210_I_PHY_ID:
		hw->phy_type = e1000_phy_igb;
		break;
		/* Fall Through */
	default:
		/* Should never have loaded on this device */
		hw->phy_type = e1000_phy_undefined;
		return -E1000_ERR_PHY_TYPE;
	}

	return E1000_SUCCESS;
}

/******************************************************************************
* Probes the expected PHY address for known PHY IDs
*
* hw - Struct containing variables accessed by shared code
******************************************************************************/
static int32_t
e1000_detect_gig_phy(struct e1000_hw *hw)
{
	int32_t phy_init_status, ret_val;
	uint16_t phy_id_high, phy_id_low;
	bool match = false;

	DEBUGFUNC();

	/* The 82571 firmware may still be configuring the PHY.  In this
	 * case, we cannot access the PHY until the configuration is done.  So
	 * we explicitly set the PHY values. */
	if (hw->mac_type == e1000_82571 ||
		hw->mac_type == e1000_82572) {
		hw->phy_id = IGP01E1000_I_PHY_ID;
		hw->phy_type = e1000_phy_igp_2;
		return E1000_SUCCESS;
	}

	/* ESB-2 PHY reads require e1000_phy_gg82563 to be set because of a
	 * work- around that forces PHY page 0 to be set or the reads fail.
	 * The rest of the code in this routine uses e1000_read_phy_reg to
	 * read the PHY ID.  So for ESB-2 we need to have this set so our
	 * reads won't fail.  If the attached PHY is not a e1000_phy_gg82563,
	 * the routines below will figure this out as well. */
	if (hw->mac_type == e1000_80003es2lan)
		hw->phy_type = e1000_phy_gg82563;

	/* Read the PHY ID Registers to identify which PHY is onboard. */
	ret_val = e1000_read_phy_reg(hw, PHY_ID1, &phy_id_high);
	if (ret_val)
		return ret_val;

	hw->phy_id = (uint32_t) (phy_id_high << 16);
	udelay(20);
	ret_val = e1000_read_phy_reg(hw, PHY_ID2, &phy_id_low);
	if (ret_val)
		return ret_val;

	hw->phy_id |= (uint32_t) (phy_id_low & PHY_REVISION_MASK);
	hw->phy_revision = (uint32_t) phy_id_low & ~PHY_REVISION_MASK;

	switch (hw->mac_type) {
	case e1000_82543:
		if (hw->phy_id == M88E1000_E_PHY_ID)
			match = true;
		break;
	case e1000_82544:
		if (hw->phy_id == M88E1000_I_PHY_ID)
			match = true;
		break;
	case e1000_82540:
	case e1000_82545:
	case e1000_82545_rev_3:
	case e1000_82546:
	case e1000_82546_rev_3:
		if (hw->phy_id == M88E1011_I_PHY_ID)
			match = true;
		break;
	case e1000_82541:
	case e1000_82541_rev_2:
	case e1000_82547:
	case e1000_82547_rev_2:
		if(hw->phy_id == IGP01E1000_I_PHY_ID)
			match = true;

		break;
	case e1000_82573:
		if (hw->phy_id == M88E1111_I_PHY_ID)
			match = true;
		break;
	case e1000_82574:
		if (hw->phy_id == BME1000_E_PHY_ID)
			match = true;
		break;
	case e1000_80003es2lan:
		if (hw->phy_id == GG82563_E_PHY_ID)
			match = true;
		break;
	case e1000_ich8lan:
		if (hw->phy_id == IGP03E1000_E_PHY_ID)
			match = true;
		if (hw->phy_id == IFE_E_PHY_ID)
			match = true;
		if (hw->phy_id == IFE_PLUS_E_PHY_ID)
			match = true;
		if (hw->phy_id == IFE_C_E_PHY_ID)
			match = true;
		break;
	case e1000_igb:
		if (hw->phy_id == I210_I_PHY_ID)
			match = true;
		break;
	default:
		DEBUGOUT("Invalid MAC type %d\n", hw->mac_type);
		return -E1000_ERR_CONFIG;
	}

	phy_init_status = e1000_set_phy_type(hw);

	if ((match) && (phy_init_status == E1000_SUCCESS)) {
		DEBUGOUT("PHY ID 0x%X detected\n", hw->phy_id);
		return 0;
	}
	DEBUGOUT("Invalid PHY ID 0x%X\n", hw->phy_id);
	return -E1000_ERR_PHY;
}

/*****************************************************************************
 * Set media type and TBI compatibility.
 *
 * hw - Struct containing variables accessed by shared code
 * **************************************************************************/
void
e1000_set_media_type(struct e1000_hw *hw)
{
	uint32_t status;

	DEBUGFUNC();

	if (hw->mac_type != e1000_82543) {
		/* tbi_compatibility is only valid on 82543 */
		hw->tbi_compatibility_en = false;
	}

	switch (hw->device_id) {
	case E1000_DEV_ID_82545GM_SERDES:
	case E1000_DEV_ID_82546GB_SERDES:
	case E1000_DEV_ID_82571EB_SERDES:
	case E1000_DEV_ID_82571EB_SERDES_DUAL:
	case E1000_DEV_ID_82571EB_SERDES_QUAD:
	case E1000_DEV_ID_82572EI_SERDES:
	case E1000_DEV_ID_80003ES2LAN_SERDES_DPT:
		hw->media_type = e1000_media_type_internal_serdes;
		break;
	default:
		switch (hw->mac_type) {
		case e1000_82542_rev2_0:
		case e1000_82542_rev2_1:
			hw->media_type = e1000_media_type_fiber;
			break;
		case e1000_ich8lan:
		case e1000_82573:
		case e1000_82574:
		case e1000_igb:
			/* The STATUS_TBIMODE bit is reserved or reused
			 * for the this device.
			 */
			hw->media_type = e1000_media_type_copper;
			break;
		default:
			status = E1000_READ_REG(hw, STATUS);
			if (status & E1000_STATUS_TBIMODE) {
				hw->media_type = e1000_media_type_fiber;
				/* tbi_compatibility not valid on fiber */
				hw->tbi_compatibility_en = false;
			} else {
				hw->media_type = e1000_media_type_copper;
			}
			break;
		}
	}
}

/**
 * e1000_sw_init - Initialize general software structures (struct e1000_adapter)
 *
 * e1000_sw_init initializes the Adapter private data structure.
 * Fields are initialized based on PCI device information and
 * OS network device settings (MTU size).
 **/

static int
e1000_sw_init(struct e1000_hw *hw)
{
	int result;

	/* PCI config space info */
#ifdef CONFIG_DM_ETH
	dm_pci_read_config16(hw->pdev, PCI_VENDOR_ID, &hw->vendor_id);
	dm_pci_read_config16(hw->pdev, PCI_DEVICE_ID, &hw->device_id);
	dm_pci_read_config16(hw->pdev, PCI_SUBSYSTEM_VENDOR_ID,
			     &hw->subsystem_vendor_id);
	dm_pci_read_config16(hw->pdev, PCI_SUBSYSTEM_ID, &hw->subsystem_id);

	dm_pci_read_config8(hw->pdev, PCI_REVISION_ID, &hw->revision_id);
	dm_pci_read_config16(hw->pdev, PCI_COMMAND, &hw->pci_cmd_word);
#else
	pci_read_config_word(hw->pdev, PCI_VENDOR_ID, &hw->vendor_id);
	pci_read_config_word(hw->pdev, PCI_DEVICE_ID, &hw->device_id);
	pci_read_config_word(hw->pdev, PCI_SUBSYSTEM_VENDOR_ID,
			     &hw->subsystem_vendor_id);
	pci_read_config_word(hw->pdev, PCI_SUBSYSTEM_ID, &hw->subsystem_id);

	pci_read_config_byte(hw->pdev, PCI_REVISION_ID, &hw->revision_id);
	pci_read_config_word(hw->pdev, PCI_COMMAND, &hw->pci_cmd_word);
#endif

	/* identify the MAC */
	result = e1000_set_mac_type(hw);
	if (result) {
		E1000_ERR(hw, "Unknown MAC Type\n");
		return result;
	}

	switch (hw->mac_type) {
	default:
		break;
	case e1000_82541:
	case e1000_82547:
	case e1000_82541_rev_2:
	case e1000_82547_rev_2:
		hw->phy_init_script = 1;
		break;
	}

	/* flow control settings */
	hw->fc_high_water = E1000_FC_HIGH_THRESH;
	hw->fc_low_water = E1000_FC_LOW_THRESH;
	hw->fc_pause_time = E1000_FC_PAUSE_TIME;
	hw->fc_send_xon = 1;

	/* Media type - copper or fiber */
	hw->tbi_compatibility_en = true;
	e1000_set_media_type(hw);

	if (hw->mac_type >= e1000_82543) {
		uint32_t status = E1000_READ_REG(hw, STATUS);

		if (status & E1000_STATUS_TBIMODE) {
			DEBUGOUT("fiber interface\n");
			hw->media_type = e1000_media_type_fiber;
		} else {
			DEBUGOUT("copper interface\n");
			hw->media_type = e1000_media_type_copper;
		}
	} else {
		hw->media_type = e1000_media_type_fiber;
	}

	hw->wait_autoneg_complete = true;
	if (hw->mac_type < e1000_82543)
		hw->report_tx_early = 0;
	else
		hw->report_tx_early = 1;

	return E1000_SUCCESS;
}

void
fill_rx(struct e1000_hw *hw)
{
	struct e1000_rx_desc *rd;
	unsigned long flush_start, flush_end;

	rx_last = rx_tail;
	rd = rx_base + rx_tail;
	rx_tail = (rx_tail + 1) % 8;
	memset(rd, 0, 16);
	rd->buffer_addr = cpu_to_le64((unsigned long)packet);

	/*
	 * Make sure there are no stale data in WB over this area, which
	 * might get written into the memory while the e1000 also writes
	 * into the same memory area.
	 */
	invalidate_dcache_range((unsigned long)packet,
				(unsigned long)packet + 4096);
	/* Dump the DMA descriptor into RAM. */
	flush_start = ((unsigned long)rd) & ~(ARCH_DMA_MINALIGN - 1);
	flush_end = flush_start + roundup(sizeof(*rd), ARCH_DMA_MINALIGN);
	flush_dcache_range(flush_start, flush_end);

	E1000_WRITE_REG(hw, RDT, rx_tail);
}

/**
 * e1000_configure_tx - Configure 8254x Transmit Unit after Reset
 * @adapter: board private structure
 *
 * Configure the Tx unit of the MAC after a reset.
 **/

static void
e1000_configure_tx(struct e1000_hw *hw)
{
	unsigned long tctl;
	unsigned long tipg, tarc;
	uint32_t ipgr1, ipgr2;

	E1000_WRITE_REG(hw, TDBAL, lower_32_bits((unsigned long)tx_base));
	E1000_WRITE_REG(hw, TDBAH, upper_32_bits((unsigned long)tx_base));

	E1000_WRITE_REG(hw, TDLEN, 128);

	/* Setup the HW Tx Head and Tail descriptor pointers */
	E1000_WRITE_REG(hw, TDH, 0);
	E1000_WRITE_REG(hw, TDT, 0);
	tx_tail = 0;

	/* Set the default values for the Tx Inter Packet Gap timer */
	if (hw->mac_type <= e1000_82547_rev_2 &&
	    (hw->media_type == e1000_media_type_fiber ||
	     hw->media_type == e1000_media_type_internal_serdes))
		tipg = DEFAULT_82543_TIPG_IPGT_FIBER;
	else
		tipg = DEFAULT_82543_TIPG_IPGT_COPPER;

	/* Set the default values for the Tx Inter Packet Gap timer */
	switch (hw->mac_type) {
	case e1000_82542_rev2_0:
	case e1000_82542_rev2_1:
		tipg = DEFAULT_82542_TIPG_IPGT;
		ipgr1 = DEFAULT_82542_TIPG_IPGR1;
		ipgr2 = DEFAULT_82542_TIPG_IPGR2;
		break;
	case e1000_80003es2lan:
		ipgr1 = DEFAULT_82543_TIPG_IPGR1;
		ipgr2 = DEFAULT_80003ES2LAN_TIPG_IPGR2;
		break;
	default:
		ipgr1 = DEFAULT_82543_TIPG_IPGR1;
		ipgr2 = DEFAULT_82543_TIPG_IPGR2;
		break;
	}
	tipg |= ipgr1 << E1000_TIPG_IPGR1_SHIFT;
	tipg |= ipgr2 << E1000_TIPG_IPGR2_SHIFT;
	E1000_WRITE_REG(hw, TIPG, tipg);
	/* Program the Transmit Control Register */
	tctl = E1000_READ_REG(hw, TCTL);
	tctl &= ~E1000_TCTL_CT;
	tctl |= E1000_TCTL_EN | E1000_TCTL_PSP |
	    (E1000_COLLISION_THRESHOLD << E1000_CT_SHIFT);

	if (hw->mac_type == e1000_82571 || hw->mac_type == e1000_82572) {
		tarc = E1000_READ_REG(hw, TARC0);
		/* set the speed mode bit, we'll clear it if we're not at
		 * gigabit link later */
		/* git bit can be set to 1*/
	} else if (hw->mac_type == e1000_80003es2lan) {
		tarc = E1000_READ_REG(hw, TARC0);
		tarc |= 1;
		E1000_WRITE_REG(hw, TARC0, tarc);
		tarc = E1000_READ_REG(hw, TARC1);
		tarc |= 1;
		E1000_WRITE_REG(hw, TARC1, tarc);
	}


	e1000_config_collision_dist(hw);
	/* Setup Transmit Descriptor Settings for eop descriptor */
	hw->txd_cmd = E1000_TXD_CMD_EOP | E1000_TXD_CMD_IFCS;

	/* Need to set up RS bit */
	if (hw->mac_type < e1000_82543)
		hw->txd_cmd |= E1000_TXD_CMD_RPS;
	else
		hw->txd_cmd |= E1000_TXD_CMD_RS;


	if (hw->mac_type == e1000_igb) {
		E1000_WRITE_REG(hw, TCTL_EXT, 0x42 << 10);

		uint32_t reg_txdctl = E1000_READ_REG(hw, TXDCTL);
		reg_txdctl |= 1 << 25;
		E1000_WRITE_REG(hw, TXDCTL, reg_txdctl);
		mdelay(20);
	}



	E1000_WRITE_REG(hw, TCTL, tctl);


}

/**
 * e1000_setup_rctl - configure the receive control register
 * @adapter: Board private structure
 **/
static void
e1000_setup_rctl(struct e1000_hw *hw)
{
	uint32_t rctl;

	rctl = E1000_READ_REG(hw, RCTL);

	rctl &= ~(3 << E1000_RCTL_MO_SHIFT);

	rctl |= E1000_RCTL_EN | E1000_RCTL_BAM | E1000_RCTL_LBM_NO
		| E1000_RCTL_RDMTS_HALF;	/* |
			(hw.mc_filter_type << E1000_RCTL_MO_SHIFT); */

	if (hw->tbi_compatibility_on == 1)
		rctl |= E1000_RCTL_SBP;
	else
		rctl &= ~E1000_RCTL_SBP;

	rctl &= ~(E1000_RCTL_SZ_4096);
		rctl |= E1000_RCTL_SZ_2048;
		rctl &= ~(E1000_RCTL_BSEX | E1000_RCTL_LPE);
	E1000_WRITE_REG(hw, RCTL, rctl);
}

/**
 * e1000_configure_rx - Configure 8254x Receive Unit after Reset
 * @adapter: board private structure
 *
 * Configure the Rx unit of the MAC after a reset.
 **/
static void
e1000_configure_rx(struct e1000_hw *hw)
{
	unsigned long rctl, ctrl_ext;
	rx_tail = 0;

	/* make sure receives are disabled while setting up the descriptors */
	rctl = E1000_READ_REG(hw, RCTL);
	E1000_WRITE_REG(hw, RCTL, rctl & ~E1000_RCTL_EN);
	if (hw->mac_type >= e1000_82540) {
		/* Set the interrupt throttling rate.  Value is calculated
		 * as DEFAULT_ITR = 1/(MAX_INTS_PER_SEC * 256ns) */
#define MAX_INTS_PER_SEC	8000
#define DEFAULT_ITR		1000000000/(MAX_INTS_PER_SEC * 256)
		E1000_WRITE_REG(hw, ITR, DEFAULT_ITR);
	}

	if (hw->mac_type >= e1000_82571) {
		ctrl_ext = E1000_READ_REG(hw, CTRL_EXT);
		/* Reset delay timers after every interrupt */
		ctrl_ext |= E1000_CTRL_EXT_INT_TIMER_CLR;
		E1000_WRITE_REG(hw, CTRL_EXT, ctrl_ext);
		E1000_WRITE_FLUSH(hw);
	}
	/* Setup the Base and Length of the Rx Descriptor Ring */
	E1000_WRITE_REG(hw, RDBAL, lower_32_bits((unsigned long)rx_base));
	E1000_WRITE_REG(hw, RDBAH, upper_32_bits((unsigned long)rx_base));

	E1000_WRITE_REG(hw, RDLEN, 128);

	/* Setup the HW Rx Head and Tail Descriptor Pointers */
	E1000_WRITE_REG(hw, RDH, 0);
	E1000_WRITE_REG(hw, RDT, 0);
	/* Enable Receives */

	if (hw->mac_type == e1000_igb) {

		uint32_t reg_rxdctl = E1000_READ_REG(hw, RXDCTL);
		reg_rxdctl |= 1 << 25;
		E1000_WRITE_REG(hw, RXDCTL, reg_rxdctl);
		mdelay(20);
	}

	E1000_WRITE_REG(hw, RCTL, rctl);

	fill_rx(hw);
}

/**************************************************************************
POLL - Wait for a frame
***************************************************************************/
static int
_e1000_poll(struct e1000_hw *hw)
{
	struct e1000_rx_desc *rd;
	unsigned long inval_start, inval_end;
	uint32_t len;

	/* return true if there's an ethernet packet ready to read */
	rd = rx_base + rx_last;

	/* Re-load the descriptor from RAM. */
	inval_start = ((unsigned long)rd) & ~(ARCH_DMA_MINALIGN - 1);
	inval_end = inval_start + roundup(sizeof(*rd), ARCH_DMA_MINALIGN);
	invalidate_dcache_range(inval_start, inval_end);

	if (!(rd->status & E1000_RXD_STAT_DD))
		return 0;
	/* DEBUGOUT("recv: packet len=%d\n", rd->length); */
	/* Packet received, make sure the data are re-loaded from RAM. */
	len = le16_to_cpu(rd->length);
	invalidate_dcache_range((unsigned long)packet,
				(unsigned long)packet +
				roundup(len, ARCH_DMA_MINALIGN));
	return len;
}

static int _e1000_transmit(struct e1000_hw *hw, void *txpacket, int length)
{
	void *nv_packet = (void *)txpacket;
	struct e1000_tx_desc *txp;
	int i = 0;
	unsigned long flush_start, flush_end;

	txp = tx_base + tx_tail;
	tx_tail = (tx_tail + 1) % 8;

	txp->buffer_addr = cpu_to_le64(virt_to_bus(hw->pdev, nv_packet));
	txp->lower.data = cpu_to_le32(hw->txd_cmd | length);
	txp->upper.data = 0;

	/* Dump the packet into RAM so e1000 can pick them. */
	flush_dcache_range((unsigned long)nv_packet,
			   (unsigned long)nv_packet +
			   roundup(length, ARCH_DMA_MINALIGN));
	/* Dump the descriptor into RAM as well. */
	flush_start = ((unsigned long)txp) & ~(ARCH_DMA_MINALIGN - 1);
	flush_end = flush_start + roundup(sizeof(*txp), ARCH_DMA_MINALIGN);
	flush_dcache_range(flush_start, flush_end);

	E1000_WRITE_REG(hw, TDT, tx_tail);

	E1000_WRITE_FLUSH(hw);
	while (1) {
		invalidate_dcache_range(flush_start, flush_end);
		if (le32_to_cpu(txp->upper.data) & E1000_TXD_STAT_DD)
			break;
		if (i++ > TOUT_LOOP) {
			DEBUGOUT("e1000: tx timeout\n");
			return 0;
		}
		udelay(10);	/* give the nic a chance to write to the register */
	}
	return 1;
}

static void
_e1000_disable(struct e1000_hw *hw)
{
	/* Turn off the ethernet interface */
	E1000_WRITE_REG(hw, RCTL, 0);
	E1000_WRITE_REG(hw, TCTL, 0);

	/* Clear the transmit ring */
	E1000_WRITE_REG(hw, TDH, 0);
	E1000_WRITE_REG(hw, TDT, 0);

	/* Clear the receive ring */
	E1000_WRITE_REG(hw, RDH, 0);
	E1000_WRITE_REG(hw, RDT, 0);

	mdelay(10);
}

/*reset function*/
static inline int
e1000_reset(struct e1000_hw *hw, unsigned char enetaddr[6])
{
	e1000_reset_hw(hw);
	if (hw->mac_type >= e1000_82544)
		E1000_WRITE_REG(hw, WUC, 0);

	return e1000_init_hw(hw, enetaddr);
}

static int
_e1000_init(struct e1000_hw *hw, unsigned char enetaddr[6])
{
	int ret_val = 0;

	ret_val = e1000_reset(hw, enetaddr);
	if (ret_val < 0) {
		if ((ret_val == -E1000_ERR_NOLINK) ||
		    (ret_val == -E1000_ERR_TIMEOUT)) {
			E1000_ERR(hw, "Valid Link not detected: %d\n", ret_val);
		} else {
			E1000_ERR(hw, "Hardware Initialization Failed\n");
		}
		return ret_val;
	}
	e1000_configure_tx(hw);
	e1000_setup_rctl(hw);
	e1000_configure_rx(hw);
	return 0;
}

/******************************************************************************
 * Gets the current PCI bus type of hardware
 *
 * hw - Struct containing variables accessed by shared code
 *****************************************************************************/
void e1000_get_bus_type(struct e1000_hw *hw)
{
	uint32_t status;

	switch (hw->mac_type) {
	case e1000_82542_rev2_0:
	case e1000_82542_rev2_1:
		hw->bus_type = e1000_bus_type_pci;
		break;
	case e1000_82571:
	case e1000_82572:
	case e1000_82573:
	case e1000_82574:
	case e1000_80003es2lan:
	case e1000_ich8lan:
	case e1000_igb:
		hw->bus_type = e1000_bus_type_pci_express;
		break;
	default:
		status = E1000_READ_REG(hw, STATUS);
		hw->bus_type = (status & E1000_STATUS_PCIX_MODE) ?
				e1000_bus_type_pcix : e1000_bus_type_pci;
		break;
	}
}

#ifndef CONFIG_DM_ETH
/* A list of all registered e1000 devices */
static LIST_HEAD(e1000_hw_list);
#endif

#ifdef CONFIG_DM_ETH
static int e1000_init_one(struct e1000_hw *hw, int cardnum,
			  struct udevice *devno, unsigned char enetaddr[6])
#else
static int e1000_init_one(struct e1000_hw *hw, int cardnum, pci_dev_t devno,
			  unsigned char enetaddr[6])
#endif
{
	u32 val;

	/* Assign the passed-in values */
#ifdef CONFIG_DM_ETH
	hw->pdev = devno;
#else
	hw->pdev = devno;
#endif
	hw->cardnum = cardnum;

	/* Print a debug message with the IO base address */
#ifdef CONFIG_DM_ETH
	dm_pci_read_config32(devno, PCI_BASE_ADDRESS_0, &val);
#else
	pci_read_config_dword(devno, PCI_BASE_ADDRESS_0, &val);
#endif
	E1000_DBG(hw, "iobase 0x%08x\n", val & 0xfffffff0);

	/* Try to enable I/O accesses and bus-mastering */
	val = PCI_COMMAND_MEMORY | PCI_COMMAND_MASTER;
#ifdef CONFIG_DM_ETH
	dm_pci_write_config32(devno, PCI_COMMAND, val);
#else
	pci_write_config_dword(devno, PCI_COMMAND, val);
#endif

	/* Make sure it worked */
#ifdef CONFIG_DM_ETH
	dm_pci_read_config32(devno, PCI_COMMAND, &val);
#else
	pci_read_config_dword(devno, PCI_COMMAND, &val);
#endif
	if (!(val & PCI_COMMAND_MEMORY)) {
		E1000_ERR(hw, "Can't enable I/O memory\n");
		return -ENOSPC;
	}
	if (!(val & PCI_COMMAND_MASTER)) {
		E1000_ERR(hw, "Can't enable bus-mastering\n");
		return -EPERM;
	}

	/* Are these variables needed? */
	hw->fc = e1000_fc_default;
	hw->original_fc = e1000_fc_default;
	hw->autoneg_failed = 0;
	hw->autoneg = 1;
	hw->get_link_status = true;
#ifndef CONFIG_E1000_NO_NVM
	hw->eeprom_semaphore_present = true;
#endif
#ifdef CONFIG_DM_ETH
	hw->hw_addr = dm_pci_map_bar(devno,	PCI_BASE_ADDRESS_0,
						PCI_REGION_MEM);
#else
	hw->hw_addr = pci_map_bar(devno,	PCI_BASE_ADDRESS_0,
						PCI_REGION_MEM);
#endif
	hw->mac_type = e1000_undefined;

	/* MAC and Phy settings */
	if (e1000_sw_init(hw) < 0) {
		E1000_ERR(hw, "Software init failed\n");
		return -EIO;
	}
	if (e1000_check_phy_reset_block(hw))
		E1000_ERR(hw, "PHY Reset is blocked!\n");

	/* Basic init was OK, reset the hardware and allow SPI access */
	e1000_reset_hw(hw);

#ifndef CONFIG_E1000_NO_NVM
	/* Validate the EEPROM and get chipset information */
	if (e1000_init_eeprom_params(hw)) {
		E1000_ERR(hw, "EEPROM is invalid!\n");
		return -EINVAL;
	}
	if ((E1000_READ_REG(hw, I210_EECD) & E1000_EECD_FLUPD) &&
	    e1000_validate_eeprom_checksum(hw))
		return -ENXIO;
	e1000_read_mac_addr(hw, enetaddr);
#endif
	e1000_get_bus_type(hw);

#ifndef CONFIG_E1000_NO_NVM
	printf("e1000: %02x:%02x:%02x:%02x:%02x:%02x\n       ",
	       enetaddr[0], enetaddr[1], enetaddr[2],
	       enetaddr[3], enetaddr[4], enetaddr[5]);
#else
	memset(enetaddr, 0, 6);
	printf("e1000: no NVM\n");
#endif

	return 0;
}

/* Put the name of a device in a string */
static void e1000_name(char *str, int cardnum)
{
	sprintf(str, "e1000#%u", cardnum);
}

#ifndef CONFIG_DM_ETH
/**************************************************************************
TRANSMIT - Transmit a frame
***************************************************************************/
static int e1000_transmit(struct eth_device *nic, void *txpacket, int length)
{
	struct e1000_hw *hw = nic->priv;

	return _e1000_transmit(hw, txpacket, length);
}

/**************************************************************************
DISABLE - Turn off ethernet interface
***************************************************************************/
static void
e1000_disable(struct eth_device *nic)
{
	struct e1000_hw *hw = nic->priv;

	_e1000_disable(hw);
}

/**************************************************************************
INIT - set up ethernet interface(s)
***************************************************************************/
static int
e1000_init(struct eth_device *nic, bd_t *bis)
{
	struct e1000_hw *hw = nic->priv;

	return _e1000_init(hw, nic->enetaddr);
}

static int
e1000_poll(struct eth_device *nic)
{
	struct e1000_hw *hw = nic->priv;
	int len;

	len = _e1000_poll(hw);
	if (len) {
		net_process_received_packet((uchar *)packet, len);
		fill_rx(hw);
	}

	return len ? 1 : 0;
}

static int e1000_write_hwaddr(struct eth_device *dev)
{
#ifndef CONFIG_E1000_NO_NVM
	unsigned char *mac = dev->enetaddr;
	unsigned char current_mac[6];
	struct e1000_hw *hw = dev->priv;
	uint16_t data[3];
	int ret_val, i;

	DEBUGOUT("%s: mac=%pM\n", __func__, mac);

	memset(current_mac, 0, 6);

	/* Read from EEPROM, not from registers, to make sure
	 * the address is persistently configured
	 */
	ret_val = e1000_read_mac_addr_from_eeprom(hw, current_mac);
	DEBUGOUT("%s: current mac=%pM\n", __func__, current_mac);

	/* Only write to EEPROM if the given address is different or
	 * reading the current address failed
	 */
	if (!ret_val && memcmp(current_mac, mac, 6) == 0)
		return 0;

	for (i = 0; i < 3; ++i)
		data[i] = mac[i * 2 + 1] << 8 | mac[i * 2];

	ret_val = e1000_write_eeprom_srwr(hw, 0x0, 3, data);

	if (!ret_val)
		ret_val = e1000_update_eeprom_checksum_i210(hw);

	return ret_val;
#else
	return 0;
#endif
}

/**************************************************************************
PROBE - Look for an adapter, this routine's visible to the outside
You should omit the last argument struct pci_device * for a non-PCI NIC
***************************************************************************/
int
e1000_initialize(bd_t * bis)
{
	unsigned int i;
	pci_dev_t devno;
	int ret;

	DEBUGFUNC();

	/* Find and probe all the matching PCI devices */
	for (i = 0; (devno = pci_find_devices(e1000_supported, i)) >= 0; i++) {
		/*
		 * These will never get freed due to errors, this allows us to
		 * perform SPI EEPROM programming from U-Boot, for example.
		 */
		struct eth_device *nic = malloc(sizeof(*nic));
		struct e1000_hw *hw = malloc(sizeof(*hw));
		if (!nic || !hw) {
			printf("e1000#%u: Out of Memory!\n", i);
			free(nic);
			free(hw);
			continue;
		}

		/* Make sure all of the fields are initially zeroed */
		memset(nic, 0, sizeof(*nic));
		memset(hw, 0, sizeof(*hw));
		nic->priv = hw;

		/* Generate a card name */
		e1000_name(nic->name, i);
		hw->name = nic->name;

		ret = e1000_init_one(hw, i, devno, nic->enetaddr);
		if (ret)
			continue;
		list_add_tail(&hw->list_node, &e1000_hw_list);

		hw->nic = nic;

		/* Set up the function pointers and register the device */
		nic->init = e1000_init;
		nic->recv = e1000_poll;
		nic->send = e1000_transmit;
		nic->halt = e1000_disable;
		nic->write_hwaddr = e1000_write_hwaddr;
		eth_register(nic);
	}

	return i;
}

struct e1000_hw *e1000_find_card(unsigned int cardnum)
{
	struct e1000_hw *hw;

	list_for_each_entry(hw, &e1000_hw_list, list_node)
		if (hw->cardnum == cardnum)
			return hw;

	return NULL;
}
#endif /* !CONFIG_DM_ETH */

#ifdef CONFIG_CMD_E1000
static int do_e1000(cmd_tbl_t *cmdtp, int flag,
		int argc, char * const argv[])
{
	unsigned char *mac = NULL;
#ifdef CONFIG_DM_ETH
	struct eth_pdata *plat;
	struct udevice *dev;
	char name[30];
	int ret;
#endif
#if !defined(CONFIG_DM_ETH) || defined(CONFIG_E1000_SPI)
	struct e1000_hw *hw;
#endif
	int cardnum;

	if (argc < 3) {
		cmd_usage(cmdtp);
		return 1;
	}

	/* Make sure we can find the requested e1000 card */
	cardnum = simple_strtoul(argv[1], NULL, 10);
#ifdef CONFIG_DM_ETH
	e1000_name(name, cardnum);
	ret = uclass_get_device_by_name(UCLASS_ETH, name, &dev);
	if (!ret) {
		plat = dev_get_platdata(dev);
		mac = plat->enetaddr;
	}
#else
	hw = e1000_find_card(cardnum);
	if (hw)
		mac = hw->nic->enetaddr;
#endif
	if (!mac) {
		printf("e1000: ERROR: No such device: e1000#%s\n", argv[1]);
		return 1;
	}

	if (!strcmp(argv[2], "print-mac-address")) {
		printf("%02x:%02x:%02x:%02x:%02x:%02x\n",
			mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
		return 0;
	}

#ifdef CONFIG_E1000_SPI
#ifdef CONFIG_DM_ETH
	hw = dev_get_priv(dev);
#endif
	/* Handle the "SPI" subcommand */
	if (!strcmp(argv[2], "spi"))
		return do_e1000_spi(cmdtp, hw, argc - 3, argv + 3);
#endif

	cmd_usage(cmdtp);
	return 1;
}

U_BOOT_CMD(
	e1000, 7, 0, do_e1000,
	"Intel e1000 controller management",
	/*  */"<card#> print-mac-address\n"
#ifdef CONFIG_E1000_SPI
	"e1000 <card#> spi show [<offset> [<length>]]\n"
	"e1000 <card#> spi dump <addr> <offset> <length>\n"
	"e1000 <card#> spi program <addr> <offset> <length>\n"
	"e1000 <card#> spi checksum [update]\n"
#endif
	"       - Manage the Intel E1000 PCI device"
);
#endif /* not CONFIG_CMD_E1000 */

#ifdef CONFIG_DM_ETH
static int e1000_eth_start(struct udevice *dev)
{
	struct eth_pdata *plat = dev_get_platdata(dev);
	struct e1000_hw *hw = dev_get_priv(dev);

	return _e1000_init(hw, plat->enetaddr);
}

static void e1000_eth_stop(struct udevice *dev)
{
	struct e1000_hw *hw = dev_get_priv(dev);

	_e1000_disable(hw);
}

static int e1000_eth_send(struct udevice *dev, void *packet, int length)
{
	struct e1000_hw *hw = dev_get_priv(dev);
	int ret;

	ret = _e1000_transmit(hw, packet, length);

	return ret ? 0 : -ETIMEDOUT;
}

static int e1000_eth_recv(struct udevice *dev, int flags, uchar **packetp)
{
	struct e1000_hw *hw = dev_get_priv(dev);
	int len;

	len = _e1000_poll(hw);
	if (len)
		*packetp = packet;

	return len ? len : -EAGAIN;
}

static int e1000_free_pkt(struct udevice *dev, uchar *packet, int length)
{
	struct e1000_hw *hw = dev_get_priv(dev);

	fill_rx(hw);

	return 0;
}

static int e1000_eth_probe(struct udevice *dev)
{
	struct eth_pdata *plat = dev_get_platdata(dev);
	struct e1000_hw *hw = dev_get_priv(dev);
	int ret;

	hw->name = dev->name;
	ret = e1000_init_one(hw, trailing_strtol(dev->name),
			     dev, plat->enetaddr);
	if (ret < 0) {
		printf(pr_fmt("failed to initialize card: %d\n"), ret);
		return ret;
	}

	return 0;
}

static int e1000_eth_bind(struct udevice *dev)
{
	char name[20];

	/*
	 * A simple way to number the devices. When device tree is used this
	 * is unnecessary, but when the device is just discovered on the PCI
	 * bus we need a name. We could instead have the uclass figure out
	 * which devices are different and number them.
	 */
	e1000_name(name, num_cards++);

	return device_set_name(dev, name);
}

static const struct eth_ops e1000_eth_ops = {
	.start	= e1000_eth_start,
	.send	= e1000_eth_send,
	.recv	= e1000_eth_recv,
	.stop	= e1000_eth_stop,
	.free_pkt = e1000_free_pkt,
};

static const struct udevice_id e1000_eth_ids[] = {
	{ .compatible = "intel,e1000" },
	{ }
};

U_BOOT_DRIVER(eth_e1000) = {
	.name	= "eth_e1000",
	.id	= UCLASS_ETH,
	.of_match = e1000_eth_ids,
	.bind	= e1000_eth_bind,
	.probe	= e1000_eth_probe,
	.ops	= &e1000_eth_ops,
	.priv_auto_alloc_size = sizeof(struct e1000_hw),
	.platdata_auto_alloc_size = sizeof(struct eth_pdata),
};

U_BOOT_PCI_DEVICE(eth_e1000, e1000_supported);
#endif
