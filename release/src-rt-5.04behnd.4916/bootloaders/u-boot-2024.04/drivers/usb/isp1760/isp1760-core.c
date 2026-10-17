// SPDX-License-Identifier: GPL-2.0
/*
 * Driver for the NXP ISP1760 chip
 *
 * Copyright 2022 Linaro, Rui Miguel Silva <rui.silva@linaro.org>
 *
 * This is based on linux kernel driver, original developed:
 * Copyright 2014 Laurent Pinchart
 * Copyright 2007 Sebastian Siewior
 *
 */

#include <dm.h>
#include <dm/device-internal.h>
#include <dm/device_compat.h>
#include <dm/devres.h>
#include <linux/compat.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <regmap.h>
#include <usb.h>

#include "isp1760-core.h"
#include "isp1760-hcd.h"
#include "isp1760-regs.h"

#define msleep(a) udelay((a) * 1000)

static int isp1760_init_core(struct isp1760_device *isp)
{
	struct isp1760_hcd *hcd = &isp->hcd;

	/*
	 * Reset the host controller, including the CPU interface
	 * configuration.
	 */
	isp1760_field_set(hcd->fields, SW_RESET_RESET_ALL);
	msleep(100);

	/* Setup HW Mode Control: This assumes a level active-low interrupt */
	if ((isp->devflags & ISP1760_FLAG_ANALOG_OC) && hcd->is_isp1763)
		return -EINVAL;

	if (isp->devflags & ISP1760_FLAG_BUS_WIDTH_16)
		isp1760_field_clear(hcd->fields, HW_DATA_BUS_WIDTH);
	if (isp->devflags & ISP1760_FLAG_BUS_WIDTH_8)
		isp1760_field_set(hcd->fields, HW_DATA_BUS_WIDTH);
	if (isp->devflags & ISP1760_FLAG_ANALOG_OC)
		isp1760_field_set(hcd->fields, HW_ANA_DIGI_OC);
	if (isp->devflags & ISP1760_FLAG_DACK_POL_HIGH)
		isp1760_field_set(hcd->fields, HW_DACK_POL_HIGH);
	if (isp->devflags & ISP1760_FLAG_DREQ_POL_HIGH)
		isp1760_field_set(hcd->fields, HW_DREQ_POL_HIGH);
	if (isp->devflags & ISP1760_FLAG_INTR_POL_HIGH)
		isp1760_field_set(hcd->fields, HW_INTR_HIGH_ACT);
	if (isp->devflags & ISP1760_FLAG_INTR_EDGE_TRIG)
		isp1760_field_set(hcd->fields, HW_INTR_EDGE_TRIG);

	/*
	 * The ISP1761 has a dedicated DC IRQ line but supports sharing the HC
	 * IRQ line for both the host and device controllers. Hardcode IRQ
	 * sharing for now and disable the DC interrupts globally to avoid
	 * spurious interrupts during HCD registration.
	 */
	if (isp->devflags & ISP1760_FLAG_ISP1761) {
		isp1760_reg_write(hcd->regs, ISP176x_DC_MODE, 0);
		isp1760_field_set(hcd->fields, HW_COMN_IRQ);
	}

	/*
	 * PORT 1 Control register of the ISP1760 is the OTG control register
	 * on ISP1761.
	 *
	 * TODO: Really support OTG. For now we configure port 1 in device mode
	 */
	if (((isp->devflags & ISP1760_FLAG_ISP1761) ||
	     (isp->devflags & ISP1760_FLAG_ISP1763)) &&
	    (isp->devflags & ISP1760_FLAG_PERIPHERAL_EN)) {
		isp1760_field_set(hcd->fields, HW_DM_PULLDOWN);
		isp1760_field_set(hcd->fields, HW_DP_PULLDOWN);
		isp1760_field_set(hcd->fields, HW_OTG_DISABLE);
	} else {
		isp1760_field_set(hcd->fields, HW_SW_SEL_HC_DC);
		isp1760_field_set(hcd->fields, HW_VBUS_DRV);
		isp1760_field_set(hcd->fields, HW_SEL_CP_EXT);
	}

	printf("%s bus width: %u, oc: %s\n",
	       hcd->is_isp1763 ? "isp1763" : "isp1760",
	       isp->devflags & ISP1760_FLAG_BUS_WIDTH_8 ? 8 :
	       isp->devflags & ISP1760_FLAG_BUS_WIDTH_16 ? 16 : 32,
	       hcd->is_isp1763 ? "not available" :
	       isp->devflags & ISP1760_FLAG_ANALOG_OC ? "analog" : "digital");

	return 0;
}

void isp1760_set_pullup(struct isp1760_device *isp, bool enable)
{
	struct isp1760_hcd *hcd = &isp->hcd;

	if (enable)
		isp1760_field_set(hcd->fields, HW_DP_PULLUP);
	else
		isp1760_field_set(hcd->fields, HW_DP_PULLUP_CLEAR);
}

/*
 * ISP1760/61:
 *
 * 60kb divided in:
 * - 32 blocks @ 256  bytes
 * - 20 blocks @ 1024 bytes
 * -  4 blocks @ 8192 bytes
 */
static const struct isp1760_memory_layout isp176x_memory_conf = {
	.blocks[0]		= 32,
	.blocks_size[0]		= 256,
	.blocks[1]		= 20,
	.blocks_size[1]		= 1024,
	.blocks[2]		= 4,
	.blocks_size[2]		= 8192,

	.slot_num		= 32,
	.payload_blocks		= 32 + 20 + 4,
	.payload_area_size	= 0xf000,
};

/*
 * ISP1763:
 *
 * 20kb divided in:
 * - 8 blocks @ 256  bytes
 * - 2 blocks @ 1024 bytes
 * - 4 blocks @ 4096 bytes
 */
static const struct isp1760_memory_layout isp1763_memory_conf = {
	.blocks[0]		= 8,
	.blocks_size[0]		= 256,
	.blocks[1]		= 2,
	.blocks_size[1]		= 1024,
	.blocks[2]		= 4,
	.blocks_size[2]		= 4096,

	.slot_num		= 16,
	.payload_blocks		= 8 + 2 + 4,
	.payload_area_size	= 0x5000,
};

static const struct regmap_config isp1760_hc_regmap_conf = {
	.width = REGMAP_SIZE_16,
};

static const struct reg_field isp1760_hc_reg_fields[] = {
	[HCS_PPC]		= REG_FIELD(ISP176x_HC_HCSPARAMS, 4, 4),
	[HCS_N_PORTS]		= REG_FIELD(ISP176x_HC_HCSPARAMS, 0, 3),
	[HCC_ISOC_CACHE]	= REG_FIELD(ISP176x_HC_HCCPARAMS, 7, 7),
	[HCC_ISOC_THRES]	= REG_FIELD(ISP176x_HC_HCCPARAMS, 4, 6),
	[CMD_LRESET]		= REG_FIELD(ISP176x_HC_USBCMD, 7, 7),
	[CMD_RESET]		= REG_FIELD(ISP176x_HC_USBCMD, 1, 1),
	[CMD_RUN]		= REG_FIELD(ISP176x_HC_USBCMD, 0, 0),
	[STS_PCD]		= REG_FIELD(ISP176x_HC_USBSTS, 2, 2),
	[HC_FRINDEX]		= REG_FIELD(ISP176x_HC_FRINDEX, 0, 13),
	[FLAG_CF]		= REG_FIELD(ISP176x_HC_CONFIGFLAG, 0, 0),
	[HC_ISO_PTD_DONEMAP]	= REG_FIELD(ISP176x_HC_ISO_PTD_DONEMAP, 0, 31),
	[HC_ISO_PTD_SKIPMAP]	= REG_FIELD(ISP176x_HC_ISO_PTD_SKIPMAP, 0, 31),
	[HC_ISO_PTD_LASTPTD]	= REG_FIELD(ISP176x_HC_ISO_PTD_LASTPTD, 0, 31),
	[HC_INT_PTD_DONEMAP]	= REG_FIELD(ISP176x_HC_INT_PTD_DONEMAP, 0, 31),
	[HC_INT_PTD_SKIPMAP]	= REG_FIELD(ISP176x_HC_INT_PTD_SKIPMAP, 0, 31),
	[HC_INT_PTD_LASTPTD]	= REG_FIELD(ISP176x_HC_INT_PTD_LASTPTD, 0, 31),
	[HC_ATL_PTD_DONEMAP]	= REG_FIELD(ISP176x_HC_ATL_PTD_DONEMAP, 0, 31),
	[HC_ATL_PTD_SKIPMAP]	= REG_FIELD(ISP176x_HC_ATL_PTD_SKIPMAP, 0, 31),
	[HC_ATL_PTD_LASTPTD]	= REG_FIELD(ISP176x_HC_ATL_PTD_LASTPTD, 0, 31),
	[PORT_OWNER]		= REG_FIELD(ISP176x_HC_PORTSC1, 13, 13),
	[PORT_POWER]		= REG_FIELD(ISP176x_HC_PORTSC1, 12, 12),
	[PORT_LSTATUS]		= REG_FIELD(ISP176x_HC_PORTSC1, 10, 11),
	[PORT_RESET]		= REG_FIELD(ISP176x_HC_PORTSC1, 8, 8),
	[PORT_SUSPEND]		= REG_FIELD(ISP176x_HC_PORTSC1, 7, 7),
	[PORT_RESUME]		= REG_FIELD(ISP176x_HC_PORTSC1, 6, 6),
	[PORT_PE]		= REG_FIELD(ISP176x_HC_PORTSC1, 2, 2),
	[PORT_CSC]		= REG_FIELD(ISP176x_HC_PORTSC1, 1, 1),
	[PORT_CONNECT]		= REG_FIELD(ISP176x_HC_PORTSC1, 0, 0),
	[ALL_ATX_RESET]		= REG_FIELD(ISP176x_HC_HW_MODE_CTRL, 31, 31),
	[HW_ANA_DIGI_OC]	= REG_FIELD(ISP176x_HC_HW_MODE_CTRL, 15, 15),
	[HW_COMN_IRQ]		= REG_FIELD(ISP176x_HC_HW_MODE_CTRL, 10, 10),
	[HW_DATA_BUS_WIDTH]	= REG_FIELD(ISP176x_HC_HW_MODE_CTRL, 8, 8),
	[HW_DACK_POL_HIGH]	= REG_FIELD(ISP176x_HC_HW_MODE_CTRL, 6, 6),
	[HW_DREQ_POL_HIGH]	= REG_FIELD(ISP176x_HC_HW_MODE_CTRL, 5, 5),
	[HW_INTR_HIGH_ACT]	= REG_FIELD(ISP176x_HC_HW_MODE_CTRL, 2, 2),
	[HW_INTR_EDGE_TRIG]	= REG_FIELD(ISP176x_HC_HW_MODE_CTRL, 1, 1),
	[HW_GLOBAL_INTR_EN]	= REG_FIELD(ISP176x_HC_HW_MODE_CTRL, 0, 0),
	[HC_CHIP_REV]		= REG_FIELD(ISP176x_HC_CHIP_ID, 16, 31),
	[HC_CHIP_ID_HIGH]	= REG_FIELD(ISP176x_HC_CHIP_ID, 8, 15),
	[HC_CHIP_ID_LOW]	= REG_FIELD(ISP176x_HC_CHIP_ID, 0, 7),
	[HC_SCRATCH]		= REG_FIELD(ISP176x_HC_SCRATCH, 0, 31),
	[SW_RESET_RESET_ALL]	= REG_FIELD(ISP176x_HC_RESET, 0, 0),
	[ISO_BUF_FILL]		= REG_FIELD(ISP176x_HC_BUFFER_STATUS, 2, 2),
	[INT_BUF_FILL]		= REG_FIELD(ISP176x_HC_BUFFER_STATUS, 1, 1),
	[ATL_BUF_FILL]		= REG_FIELD(ISP176x_HC_BUFFER_STATUS, 0, 0),
	[MEM_BANK_SEL]		= REG_FIELD(ISP176x_HC_MEMORY, 16, 17),
	[MEM_START_ADDR]	= REG_FIELD(ISP176x_HC_MEMORY, 0, 15),
	[HC_INTERRUPT]		= REG_FIELD(ISP176x_HC_INTERRUPT, 0, 9),
	[HC_ATL_IRQ_ENABLE]	= REG_FIELD(ISP176x_HC_INTERRUPT_ENABLE, 8, 8),
	[HC_INT_IRQ_ENABLE]	= REG_FIELD(ISP176x_HC_INTERRUPT_ENABLE, 7, 7),
	[HC_ISO_IRQ_MASK_OR]	= REG_FIELD(ISP176x_HC_ISO_IRQ_MASK_OR, 0, 31),
	[HC_INT_IRQ_MASK_OR]	= REG_FIELD(ISP176x_HC_INT_IRQ_MASK_OR, 0, 31),
	[HC_ATL_IRQ_MASK_OR]	= REG_FIELD(ISP176x_HC_ATL_IRQ_MASK_OR, 0, 31),
	[HC_ISO_IRQ_MASK_AND]	= REG_FIELD(ISP176x_HC_ISO_IRQ_MASK_AND, 0, 31),
	[HC_INT_IRQ_MASK_AND]	= REG_FIELD(ISP176x_HC_INT_IRQ_MASK_AND, 0, 31),
	[HC_ATL_IRQ_MASK_AND]	= REG_FIELD(ISP176x_HC_ATL_IRQ_MASK_AND, 0, 31),
	[HW_OTG_DISABLE]	= REG_FIELD(ISP176x_HC_OTG_CTRL_SET, 10, 10),
	[HW_SW_SEL_HC_DC]	= REG_FIELD(ISP176x_HC_OTG_CTRL_SET, 7, 7),
	[HW_VBUS_DRV]		= REG_FIELD(ISP176x_HC_OTG_CTRL_SET, 4, 4),
	[HW_SEL_CP_EXT]		= REG_FIELD(ISP176x_HC_OTG_CTRL_SET, 3, 3),
	[HW_DM_PULLDOWN]	= REG_FIELD(ISP176x_HC_OTG_CTRL_SET, 2, 2),
	[HW_DP_PULLDOWN]	= REG_FIELD(ISP176x_HC_OTG_CTRL_SET, 1, 1),
	[HW_DP_PULLUP]		= REG_FIELD(ISP176x_HC_OTG_CTRL_SET, 0, 0),
	[HW_OTG_DISABLE_CLEAR]	= REG_FIELD(ISP176x_HC_OTG_CTRL_CLEAR, 10, 10),
	[HW_SW_SEL_HC_DC_CLEAR]	= REG_FIELD(ISP176x_HC_OTG_CTRL_CLEAR, 7, 7),
	[HW_VBUS_DRV_CLEAR]	= REG_FIELD(ISP176x_HC_OTG_CTRL_CLEAR, 4, 4),
	[HW_SEL_CP_EXT_CLEAR]	= REG_FIELD(ISP176x_HC_OTG_CTRL_CLEAR, 3, 3),
	[HW_DM_PULLDOWN_CLEAR]	= REG_FIELD(ISP176x_HC_OTG_CTRL_CLEAR, 2, 2),
	[HW_DP_PULLDOWN_CLEAR]	= REG_FIELD(ISP176x_HC_OTG_CTRL_CLEAR, 1, 1),
	[HW_DP_PULLUP_CLEAR]	= REG_FIELD(ISP176x_HC_OTG_CTRL_CLEAR, 0, 0),
	/* Make sure the array is sized properly during compilation */
	[HC_FIELD_MAX]		= {},
};

static const struct regmap_config isp1763_hc_regmap_conf = {
	.width = REGMAP_SIZE_16,
};

static const struct reg_field isp1763_hc_reg_fields[] = {
	[CMD_LRESET]		= REG_FIELD(ISP1763_HC_USBCMD, 7, 7),
	[CMD_RESET]		= REG_FIELD(ISP1763_HC_USBCMD, 1, 1),
	[CMD_RUN]		= REG_FIELD(ISP1763_HC_USBCMD, 0, 0),
	[STS_PCD]		= REG_FIELD(ISP1763_HC_USBSTS, 2, 2),
	[HC_FRINDEX]		= REG_FIELD(ISP1763_HC_FRINDEX, 0, 13),
	[FLAG_CF]		= REG_FIELD(ISP1763_HC_CONFIGFLAG, 0, 0),
	[HC_ISO_PTD_DONEMAP]	= REG_FIELD(ISP1763_HC_ISO_PTD_DONEMAP, 0, 15),
	[HC_ISO_PTD_SKIPMAP]	= REG_FIELD(ISP1763_HC_ISO_PTD_SKIPMAP, 0, 15),
	[HC_ISO_PTD_LASTPTD]	= REG_FIELD(ISP1763_HC_ISO_PTD_LASTPTD, 0, 15),
	[HC_INT_PTD_DONEMAP]	= REG_FIELD(ISP1763_HC_INT_PTD_DONEMAP, 0, 15),
	[HC_INT_PTD_SKIPMAP]	= REG_FIELD(ISP1763_HC_INT_PTD_SKIPMAP, 0, 15),
	[HC_INT_PTD_LASTPTD]	= REG_FIELD(ISP1763_HC_INT_PTD_LASTPTD, 0, 15),
	[HC_ATL_PTD_DONEMAP]	= REG_FIELD(ISP1763_HC_ATL_PTD_DONEMAP, 0, 15),
	[HC_ATL_PTD_SKIPMAP]	= REG_FIELD(ISP1763_HC_ATL_PTD_SKIPMAP, 0, 15),
	[HC_ATL_PTD_LASTPTD]	= REG_FIELD(ISP1763_HC_ATL_PTD_LASTPTD, 0, 15),
	[PORT_OWNER]		= REG_FIELD(ISP1763_HC_PORTSC1, 13, 13),
	[PORT_POWER]		= REG_FIELD(ISP1763_HC_PORTSC1, 12, 12),
	[PORT_LSTATUS]		= REG_FIELD(ISP1763_HC_PORTSC1, 10, 11),
	[PORT_RESET]		= REG_FIELD(ISP1763_HC_PORTSC1, 8, 8),
	[PORT_SUSPEND]		= REG_FIELD(ISP1763_HC_PORTSC1, 7, 7),
	[PORT_RESUME]		= REG_FIELD(ISP1763_HC_PORTSC1, 6, 6),
	[PORT_PE]		= REG_FIELD(ISP1763_HC_PORTSC1, 2, 2),
	[PORT_CSC]		= REG_FIELD(ISP1763_HC_PORTSC1, 1, 1),
	[PORT_CONNECT]		= REG_FIELD(ISP1763_HC_PORTSC1, 0, 0),
	[HW_DATA_BUS_WIDTH]	= REG_FIELD(ISP1763_HC_HW_MODE_CTRL, 4, 4),
	[HW_DACK_POL_HIGH]	= REG_FIELD(ISP1763_HC_HW_MODE_CTRL, 6, 6),
	[HW_DREQ_POL_HIGH]	= REG_FIELD(ISP1763_HC_HW_MODE_CTRL, 5, 5),
	[HW_INTF_LOCK]		= REG_FIELD(ISP1763_HC_HW_MODE_CTRL, 3, 3),
	[HW_INTR_HIGH_ACT]	= REG_FIELD(ISP1763_HC_HW_MODE_CTRL, 2, 2),
	[HW_INTR_EDGE_TRIG]	= REG_FIELD(ISP1763_HC_HW_MODE_CTRL, 1, 1),
	[HW_GLOBAL_INTR_EN]	= REG_FIELD(ISP1763_HC_HW_MODE_CTRL, 0, 0),
	[SW_RESET_RESET_ATX]	= REG_FIELD(ISP1763_HC_RESET, 3, 3),
	[SW_RESET_RESET_ALL]	= REG_FIELD(ISP1763_HC_RESET, 0, 0),
	[HC_CHIP_ID_HIGH]	= REG_FIELD(ISP1763_HC_CHIP_ID, 0, 15),
	[HC_CHIP_ID_LOW]	= REG_FIELD(ISP1763_HC_CHIP_REV, 8, 15),
	[HC_CHIP_REV]		= REG_FIELD(ISP1763_HC_CHIP_REV, 0, 7),
	[HC_SCRATCH]		= REG_FIELD(ISP1763_HC_SCRATCH, 0, 15),
	[ISO_BUF_FILL]		= REG_FIELD(ISP1763_HC_BUFFER_STATUS, 2, 2),
	[INT_BUF_FILL]		= REG_FIELD(ISP1763_HC_BUFFER_STATUS, 1, 1),
	[ATL_BUF_FILL]		= REG_FIELD(ISP1763_HC_BUFFER_STATUS, 0, 0),
	[MEM_START_ADDR]	= REG_FIELD(ISP1763_HC_MEMORY, 0, 15),
	[HC_DATA]		= REG_FIELD(ISP1763_HC_DATA, 0, 15),
	[HC_INTERRUPT]		= REG_FIELD(ISP1763_HC_INTERRUPT, 0, 10),
	[HC_ATL_IRQ_ENABLE]	= REG_FIELD(ISP1763_HC_INTERRUPT_ENABLE, 8, 8),
	[HC_INT_IRQ_ENABLE]	= REG_FIELD(ISP1763_HC_INTERRUPT_ENABLE, 7, 7),
	[HC_ISO_IRQ_MASK_OR]	= REG_FIELD(ISP1763_HC_ISO_IRQ_MASK_OR, 0, 15),
	[HC_INT_IRQ_MASK_OR]	= REG_FIELD(ISP1763_HC_INT_IRQ_MASK_OR, 0, 15),
	[HC_ATL_IRQ_MASK_OR]	= REG_FIELD(ISP1763_HC_ATL_IRQ_MASK_OR, 0, 15),
	[HC_ISO_IRQ_MASK_AND]	= REG_FIELD(ISP1763_HC_ISO_IRQ_MASK_AND, 0, 15),
	[HC_INT_IRQ_MASK_AND]	= REG_FIELD(ISP1763_HC_INT_IRQ_MASK_AND, 0, 15),
	[HC_ATL_IRQ_MASK_AND]	= REG_FIELD(ISP1763_HC_ATL_IRQ_MASK_AND, 0, 15),
	[HW_HC_2_DIS]		= REG_FIELD(ISP1763_HC_OTG_CTRL_SET, 15, 15),
	[HW_OTG_DISABLE]	= REG_FIELD(ISP1763_HC_OTG_CTRL_SET, 10, 10),
	[HW_SW_SEL_HC_DC]	= REG_FIELD(ISP1763_HC_OTG_CTRL_SET, 7, 7),
	[HW_VBUS_DRV]		= REG_FIELD(ISP1763_HC_OTG_CTRL_SET, 4, 4),
	[HW_SEL_CP_EXT]		= REG_FIELD(ISP1763_HC_OTG_CTRL_SET, 3, 3),
	[HW_DM_PULLDOWN]	= REG_FIELD(ISP1763_HC_OTG_CTRL_SET, 2, 2),
	[HW_DP_PULLDOWN]	= REG_FIELD(ISP1763_HC_OTG_CTRL_SET, 1, 1),
	[HW_DP_PULLUP]		= REG_FIELD(ISP1763_HC_OTG_CTRL_SET, 0, 0),
	[HW_HC_2_DIS_CLEAR]	= REG_FIELD(ISP1763_HC_OTG_CTRL_CLEAR, 15, 15),
	[HW_OTG_DISABLE_CLEAR]	= REG_FIELD(ISP1763_HC_OTG_CTRL_CLEAR, 10, 10),
	[HW_SW_SEL_HC_DC_CLEAR]	= REG_FIELD(ISP1763_HC_OTG_CTRL_CLEAR, 7, 7),
	[HW_VBUS_DRV_CLEAR]	= REG_FIELD(ISP1763_HC_OTG_CTRL_CLEAR, 4, 4),
	[HW_SEL_CP_EXT_CLEAR]	= REG_FIELD(ISP1763_HC_OTG_CTRL_CLEAR, 3, 3),
	[HW_DM_PULLDOWN_CLEAR]	= REG_FIELD(ISP1763_HC_OTG_CTRL_CLEAR, 2, 2),
	[HW_DP_PULLDOWN_CLEAR]	= REG_FIELD(ISP1763_HC_OTG_CTRL_CLEAR, 1, 1),
	[HW_DP_PULLUP_CLEAR]	= REG_FIELD(ISP1763_HC_OTG_CTRL_CLEAR, 0, 0),
	/* Make sure the array is sized properly during compilation */
	[HC_FIELD_MAX]		= {},
};

int isp1760_register(struct isp1760_device *isp, struct resource *mem, int irq,
		     unsigned long irqflags)
{
	const struct regmap_config *hc_regmap;
	const struct reg_field *hc_reg_fields;
	struct isp1760_hcd *hcd;
	struct regmap_field *f;
	unsigned int devflags;
	struct udevice *dev;
	int ret;
	int i;

	hcd = &isp->hcd;
	devflags = isp->devflags;
	dev = isp->dev;

	hcd->is_isp1763 = !!(devflags & ISP1760_FLAG_ISP1763);

	if (!hcd->is_isp1763 && (devflags & ISP1760_FLAG_BUS_WIDTH_8)) {
		dev_err(dev, "isp1760/61 do not support data width 8\n");
		return -EINVAL;
	}

	if (hcd->is_isp1763) {
		hc_regmap = &isp1763_hc_regmap_conf;
		hc_reg_fields = &isp1763_hc_reg_fields[0];
	} else {
		hc_regmap = &isp1760_hc_regmap_conf;
		hc_reg_fields = &isp1760_hc_reg_fields[0];
	}

	hcd->base = devm_ioremap(dev, mem->start, resource_size(mem));
	if (IS_ERR(hcd->base))
		return PTR_ERR(hcd->base);

	hcd->regs = devm_regmap_init(dev, NULL, NULL, hc_regmap);
	if (IS_ERR(hcd->regs))
		return PTR_ERR(hcd->regs);

	for (i = 0; i < HC_FIELD_MAX; i++) {
		f = devm_regmap_field_alloc(dev, hcd->regs, hc_reg_fields[i]);
		if (IS_ERR(f))
			return PTR_ERR(f);

		hcd->fields[i] = f;
	}

	if (hcd->is_isp1763)
		hcd->memory_layout = &isp1763_memory_conf;
	else
		hcd->memory_layout = &isp176x_memory_conf;

	ret = isp1760_init_core(isp);
	if (ret < 0)
		return ret;

	hcd->dev = dev;

	ret = isp1760_hcd_register(hcd, mem, irq, irqflags, dev);
	if (ret < 0)
		return ret;

	ret = isp1760_hcd_lowlevel_init(hcd);
	if (ret < 0)
		return ret;

	dev_set_drvdata(dev, isp);

	return 0;
}

void isp1760_unregister(struct isp1760_device *isp)
{
	isp1760_hcd_unregister(&isp->hcd);
}
