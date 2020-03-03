
#include <linux/firmware.h>
#include <drm/drmP.h>
#include "ast_drv.h"
MODULE_FIRMWARE("ast_dp501_fw.bin");

int ast_load_dp501_microcode(struct drm_device *dev)
{
	struct ast_private *ast = dev->dev_private;
	static char *fw_name = "ast_dp501_fw.bin";
	int err;
	err = request_firmware(&ast->dp501_fw, fw_name, dev->dev);
	if (err)
		return err;

	return 0;
}

static void send_ack(struct ast_private *ast)
{
	u8 sendack;
	sendack = ast_get_index_reg_mask(ast, AST_IO_CRTC_PORT, 0x9b, 0xff);
	sendack |= 0x80;
	ast_set_index_reg_mask(ast, AST_IO_CRTC_PORT, 0x9b, 0x00, sendack);
}

static void send_nack(struct ast_private *ast)
{
	u8 sendack;
	sendack = ast_get_index_reg_mask(ast, AST_IO_CRTC_PORT, 0x9b, 0xff);
	sendack &= ~0x80;
	ast_set_index_reg_mask(ast, AST_IO_CRTC_PORT, 0x9b, 0x00, sendack);
}

static bool wait_ack(struct ast_private *ast)
{
	u8 waitack;
	u32 retry = 0;
	do {
		waitack = ast_get_index_reg_mask(ast, AST_IO_CRTC_PORT, 0xd2, 0xff);
		waitack &= 0x80;
		udelay(100);
	} while ((!waitack) && (retry++ < 1000));

	if (retry < 1000)
		return true;
	else
		return false;
}

static bool wait_nack(struct ast_private *ast)
{
	u8 waitack;
	u32 retry = 0;
	do {
		waitack = ast_get_index_reg_mask(ast, AST_IO_CRTC_PORT, 0xd2, 0xff);
		waitack &= 0x80;
		udelay(100);
	} while ((waitack) && (retry++ < 1000));

	if (retry < 1000)
		return true;
	else
		return false;
}

static void set_cmd_trigger(struct ast_private *ast)
{
	ast_set_index_reg_mask(ast, AST_IO_CRTC_PORT, 0x9b, ~0x40, 0x40);
}

static void clear_cmd_trigger(struct ast_private *ast)
{
	ast_set_index_reg_mask(ast, AST_IO_CRTC_PORT, 0x9b, ~0x40, 0x00);
}

#if 0
static bool wait_fw_ready(struct ast_private *ast)
{
	u8 waitready;
	u32 retry = 0;
	do {
		waitready = ast_get_index_reg_mask(ast, AST_IO_CRTC_PORT, 0xd2, 0xff);
		waitready &= 0x40;
		udelay(100);
	} while ((!waitready) && (retry++ < 1000));

	if (retry < 1000)
		return true;
	else
		return false;
}
#endif

static bool ast_write_cmd(struct drm_device *dev, u8 data)
{
	struct ast_private *ast = dev->dev_private;
	int retry = 0;
	if (wait_nack(ast)) {
		send_nack(ast);
		ast_set_index_reg_mask(ast, AST_IO_CRTC_PORT, 0x9a, 0x00, data);
		send_ack(ast);
		set_cmd_trigger(ast);
		do {
			if (wait_ack(ast)) {
				clear_cmd_trigger(ast);
				send_nack(ast);
				return true;
			}
		} while (retry++ < 100);
	}
	clear_cmd_trigger(ast);
	send_nack(ast);
	return false;
}

static bool ast_write_data(struct drm_device *dev, u8 data)
{
	struct ast_private *ast = dev->dev_private;

	if (wait_nack(ast)) {
		send_nack(ast);
		ast_set_index_reg_mask(ast, AST_IO_CRTC_PORT, 0x9a, 0x00, data);
		send_ack(ast);
		if (wait_ack(ast)) {
			send_nack(ast);
			return true;
		}
	}
	send_nack(ast);
	return false;
}

#if 0
static bool ast_read_data(struct drm_device *dev, u8 *data)
{
	struct ast_private *ast = dev->dev_private;
	u8 tmp;

	*data = 0;

	if (wait_ack(ast) == false)
		return false;
	tmp = ast_get_index_reg_mask(ast, AST_IO_CRTC_PORT, 0xd3, 0xff);
	*data = tmp;
	if (wait_nack(ast) == false) {
		send_nack(ast);
		return false;
	}
	send_nack(ast);
	return true;
}

static void clear_cmd(struct ast_private *ast)
{
	send_nack(ast);
	ast_set_index_reg_mask(ast, AST_IO_CRTC_PORT, 0x9a, 0x00, 0x00);
}
#endif

void ast_set_dp501_video_output(struct drm_device *dev, u8 mode)
{
	ast_write_cmd(dev, 0x40);
	ast_write_data(dev, mode);

	msleep(10);
}

static u32 get_fw_base(struct ast_private *ast)
{
	return ast_mindwm(ast, 0x1e6e2104) & 0x7fffffff;
}

bool ast_backup_fw(struct drm_device *dev, u8 *addr, u32 size)
{
	struct ast_private *ast = dev->dev_private;
	u32 i, data;
	u32 boot_address;

	data = ast_mindwm(ast, 0x1e6e2100) & 0x01;
	if (data) {
		boot_address = get_fw_base(ast);
		for (i = 0; i < size; i += 4)
			*(u32 *)(addr + i) = ast_mindwm(ast, boot_address + i);
		return true;
	}
	return false;
}

bool ast_launch_m68k(struct drm_device *dev)
{
	struct ast_private *ast = dev->dev_private;
	u32 i, data, len = 0;
	u32 boot_address;
	u8 *fw_addr = NULL;
	u8 jreg;

	data = ast_mindwm(ast, 0x1e6e2100) & 0x01;
	if (!data) {

		if (ast->dp501_fw_addr) {
			fw_addr = ast->dp501_fw_addr;
			len = 32*1024;
		} else if (ast->dp501_fw) {
			fw_addr = (u8 *)ast->dp501_fw->data;
			len = ast->dp501_fw->size;
		}
		/* Get BootAddress */
		ast_moutdwm(ast, 0x1e6e2000, 0x1688a8a8);
		data = ast_mindwm(ast, 0x1e6e0004);
		switch (data & 0x03) {
		case 0:
			boot_address = 0x44000000;
			break;
		default:
		case 1:
			boot_address = 0x48000000;
			break;
		case 2:
			boot_address = 0x50000000;
			break;
		case 3:
			boot_address = 0x60000000;
			break;
		}
		boot_address -= 0x200000; /* -2MB */

		/* copy image to buffer */
		for (i = 0; i < len; i += 4) {
			data = *(u32 *)(fw_addr + i);
			ast_moutdwm(ast, boot_address + i, data);
		}

		/* Init SCU */
		ast_moutdwm(ast, 0x1e6e2000, 0x1688a8a8);

		/* Launch FW */
		ast_moutdwm(ast, 0x1e6e2104, 0x80000000 + boot_address);
		ast_moutdwm(ast, 0x1e6e2100, 1);

		/* Update Scratch */
		data = ast_mindwm(ast, 0x1e6e2040) & 0xfffff1ff;		/* D[11:9] = 100b: UEFI handling */
		data |= 0x800;
		ast_moutdwm(ast, 0x1e6e2040, data);

		jreg = ast_get_index_reg_mask(ast, AST_IO_CRTC_PORT, 0x99, 0xfc); /* D[1:0]: Reserved Video Buffer */
		jreg |= 0x02;
		ast_set_index_reg(ast, AST_IO_CRTC_PORT, 0x99, jreg);
	}
	return true;
}

u8 ast_get_dp501_max_clk(struct drm_device *dev)
{
	struct ast_private *ast = dev->dev_private;
	u32 boot_address, offset, data;
	u8 linkcap[4], linkrate, linklanes, maxclk = 0xff;

	boot_address = get_fw_base(ast);

	/* validate FW version */
	offset = 0xf000;
	data = ast_mindwm(ast, boot_address + offset);
	if ((data & 0xf0) != 0x10) /* version: 1x */
		return maxclk;

	/* Read Link Capability */
	offset  = 0xf014;
	*(u32 *)linkcap = ast_mindwm(ast, boot_address + offset);
	if (linkcap[2] == 0) {
		linkrate = linkcap[0];
		linklanes = linkcap[1];
		data = (linkrate == 0x0a) ? (90 * linklanes) : (54 * linklanes);
		if (data > 0xff)
			data = 0xff;
		maxclk = (u8)data;
	}
	return maxclk;
}

bool ast_dp501_read_edid(struct drm_device *dev, u8 *ediddata)
{
	struct ast_private *ast = dev->dev_private;
	u32 i, boot_address, offset, data;

	boot_address = get_fw_base(ast);

	/* validate FW version */
	offset = 0xf000;
	data = ast_mindwm(ast, boot_address + offset);
	if ((data & 0xf0) != 0x10)
		return false;

	/* validate PnP Monitor */
	offset = 0xf010;
	data = ast_mindwm(ast, boot_address + offset);
	if (!(data & 0x01))
		return false;

	/* Read EDID */
	offset = 0xf020;
	for (i = 0; i < 128; i += 4) {
		data = ast_mindwm(ast, boot_address + offset + i);
		*(u32 *)(ediddata + i) = data;
	}

	return true;
}

static bool ast_init_dvo(struct drm_device *dev)
{
	struct ast_private *ast = dev->dev_private;
	u8 jreg;
	u32 data;
	ast_write32(ast, 0xf004, 0x1e6e0000);
	ast_write32(ast, 0xf000, 0x1);
	ast_write32(ast, 0x12000, 0x1688a8a8);

	jreg = ast_get_index_reg_mask(ast, AST_IO_CRTC_PORT, 0xd0, 0xff);
	if (!(jreg & 0x80)) {
		/* Init SCU DVO Settings */
		data = ast_read32(ast, 0x12008);
		/* delay phase */
		data &= 0xfffff8ff;
		data |= 0x00000500;
		ast_write32(ast, 0x12008, data);

		if (ast->chip == AST2300) {
			data = ast_read32(ast, 0x12084);
			/* multi-pins for DVO single-edge */
			data |= 0xfffe0000;
			ast_write32(ast, 0x12084, data);

			data = ast_read32(ast, 0x12088);
			/* multi-pins for DVO single-edge */
			data |= 0x000fffff;
			ast_write32(ast, 0x12088, data);

			data = ast_read32(ast, 0x12090);
			/* multi-pins for DVO single-edge */
			data &= 0xffffffcf;
			data |= 0x00000020;
			ast_write32(ast, 0x12090, data);
		} else { /* AST2400 */
			data = ast_read32(ast, 0x12088);
			/* multi-pins for DVO single-edge */
			data |= 0x30000000;
			ast_write32(ast, 0x12088, data);

			data = ast_read32(ast, 0x1208c);
			/* multi-pins for DVO single-edge */
			data |= 0x000000cf;
			ast_write32(ast, 0x1208c, data);

			data = ast_read32(ast, 0x120a4);
			/* multi-pins for DVO single-edge */
			data |= 0xffff0000;
			ast_write32(ast, 0x120a4, data);

			data = ast_read32(ast, 0x120a8);
			/* multi-pins for DVO single-edge */
			data |= 0x0000000f;
			ast_write32(ast, 0x120a8, data);

			data = ast_read32(ast, 0x12094);
			/* multi-pins for DVO single-edge */
			data |= 0x00000002;
			ast_write32(ast, 0x12094, data);
		}
	}

	/* Force to DVO */
	data = ast_read32(ast, 0x1202c);
	data &= 0xfffbffff;
	ast_write32(ast, 0x1202c, data);

	/* Init VGA DVO Settings */
	ast_set_index_reg_mask(ast, AST_IO_CRTC_PORT, 0xa3, 0xcf, 0x80);
	return true;
}


static void ast_init_analog(struct drm_device *dev)
{
	struct ast_private *ast = dev->dev_private;
	u32 data;

	/*
	 * Set DAC source to VGA mode in SCU2C via the P2A
	 * bridge. First configure the P2U to target the SCU
	 * in case it isn't at this stage.
	 */
	ast_write32(ast, 0xf004, 0x1e6e0000);
	ast_write32(ast, 0xf000, 0x1);

	/* Then unlock the SCU with the magic password */
	ast_write32(ast, 0x12000, 0x1688a8a8);
	ast_write32(ast, 0x12000, 0x1688a8a8);
	ast_write32(ast, 0x12000, 0x1688a8a8);

	/* Finally, clear bits [17:16] of SCU2c */
	data = ast_read32(ast, 0x1202c);
	data &= 0xfffcffff;
	ast_write32(ast, 0, data);

	/* Disable DVO */
	ast_set_index_reg_mask(ast, AST_IO_CRTC_PORT, 0xa3, 0xcf, 0x00);
}

void ast_init_3rdtx(struct drm_device *dev)
{
	struct ast_private *ast = dev->dev_private;
	u8 jreg;

	if (ast->chip == AST2300 || ast->chip == AST2400) {
		jreg = ast_get_index_reg_mask(ast, AST_IO_CRTC_PORT, 0xd1, 0xff);
		switch (jreg & 0x0e) {
		case 0x04:
			ast_init_dvo(dev);
			break;
		case 0x08:
			ast_launch_m68k(dev);
			break;
		case 0x0c:
			ast_init_dvo(dev);
			break;
		default:
			if (ast->tx_chip_type == AST_TX_SIL164)
				ast_init_dvo(dev);
			else
				ast_init_analog(dev);
		}
	}
}
