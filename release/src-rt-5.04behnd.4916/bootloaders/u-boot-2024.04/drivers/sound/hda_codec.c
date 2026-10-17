// SPDX-License-Identifier: GPL-2.0
/*
 * Implementation of per-board codec beeping
 * Copyright (c) 2011 The Chromium OS Authors.
 * Copyright 2018 Google LLC
 */

#define LOG_CATEGORY	UCLASS_SOUND

#include <common.h>
#include <dm.h>
#include <hda_codec.h>
#include <log.h>
#include <pci.h>
#include <sound.h>
#include <asm/io.h>
#include <dt-bindings/sound/azalia.h>
#include <linux/bitops.h>
#include <linux/delay.h>

/**
 * struct hda_regs - HDA registers
 *
 * https://wiki.osdev.org/Intel_High_Definition_Audio
 * https://www.intel.com/content/www/us/en/standards/high-definition-audio-specification.html
 */
struct hda_regs {
	u16 gcap;
	u8 vmin;
	u8 vmaj;
	u16 outpay;
	u16 inpay;
	u32 gctl;
	u16 wakeen;
	u16 statests;
	u8 reserved[0x50];
	u32 cmd;		/* 0x60 */
	u32 resp;
	u32 icii;
};

enum {
	HDA_ICII_BUSY			= BIT(0),
	HDA_ICII_VALID			= BIT(1),

	/* Common node IDs */
	HDA_ROOT_NODE			= 0x00,

	/* HDA verbs fields */
	HDA_VERB_NID_S			= 20,
	HDA_VERB_VERB_S			= 8,
	HDA_VERB_PARAM_S		= 0,

	HDA_VERB_GET_PARAMS		= 0xf00,
	HDA_VERB_SET_BEEP		= 0x70a,

	/* GET_PARAMS parameter IDs */
	GET_PARAMS_NODE_COUNT		= 0x04,
	GET_PARAMS_AUDIO_GROUP_CAPS	= 0x08,
	GET_PARAMS_AUDIO_WIDGET_CAPS	= 0x09,

	/* Sub-node fields */
	NUM_SUB_NODES_S			= 0,
	NUM_SUB_NODES_M			= 0xff << NUM_SUB_NODES_S,
	FIRST_SUB_NODE_S		= 16,
	FIRST_SUB_NODE_M		= 0xff << FIRST_SUB_NODE_S,

	/* Get Audio Function Group Capabilities fields */
	AUDIO_GROUP_CAPS_BEEP_GEN	= 0x10000,

	/* Get Audio Widget Capabilities fields */
	AUDIO_WIDGET_TYPE_BEEP		= 0x7,
	AUDIO_WIDGET_TYPE_S		= 20,
	AUDIO_WIDGET_TYPE_M		= 0xf << AUDIO_WIDGET_TYPE_S,

	BEEP_FREQ_BASE			= 12000,
};

static inline uint hda_verb(uint nid, uint verb, uint param)
{
	return nid << HDA_VERB_NID_S | verb << HDA_VERB_VERB_S |
		param << HDA_VERB_PARAM_S;
}

int hda_wait_for_ready(struct hda_regs *regs)
{
	int timeout = 1000;	/* Use a 1msec timeout */

	while (timeout--) {
		u32 reg32 = readl(&regs->icii);

		if (!(reg32 & HDA_ICII_BUSY))
			return 0;
		udelay(1);
	}

	return -ETIMEDOUT;
}

static int wait_for_response(struct hda_regs *regs, uint *response)
{
	int timeout = 1000;
	u32 reg32;

	/* Send the verb to the codec */
	setbits_le32(&regs->icii, HDA_ICII_BUSY | HDA_ICII_VALID);

	/* Use a 1msec timeout */
	while (timeout--) {
		reg32 = readl(&regs->icii);
		if ((reg32 & (HDA_ICII_VALID | HDA_ICII_BUSY)) ==
			HDA_ICII_VALID) {
			if (response)
				*response = readl(&regs->resp);
			return 0;
		}
		udelay(1);
	}

	return -ETIMEDOUT;
}

int hda_wait_for_valid(struct hda_regs *regs)
{
	return wait_for_response(regs, NULL);
}

static int set_bits(void *port, u32 mask, u32 val)
{
	u32 reg32;
	int count;

	/* Write (val & mask) to port */
	clrsetbits_le32(port, mask, val);

	/* Wait for readback of register to match what was just written to it */
	count = 50;
	do {
		/* Wait 1ms based on BKDG wait time */
		mdelay(1);
		reg32 = readl(port) & mask;
	} while (reg32 != val && --count);

	/* Timeout occurred */
	if (!count)
		return -ETIMEDOUT;

	return 0;
}

int hda_codec_detect(struct hda_regs *regs)
{
	uint reg8;

	/* Set Bit 0 to 1 to exit reset state (BAR + 0x8)[0] */
	if (set_bits(&regs->gctl, 1, 1))
		goto no_codec;

	/* Write back the value once reset bit is set */
	writew(readw(&regs->gcap), &regs->gcap);

	/* Read in Codec location */
	reg8 = readb(&regs->statests) & 0xf;
	if (!reg8)
		goto no_codec;

	return reg8;

no_codec:
	/* Codec Not found - put HDA back in reset */
	set_bits(&regs->gctl, 1, 0);
	log_debug("No codec\n");

	return 0;
}

static int find_verb_data(struct udevice *dev, uint id, ofnode *nodep)
{
	ofnode parent = dev_read_subnode(dev, "codecs");
	ofnode node;
	u32 vendor_id, device_id;

	ofnode_for_each_subnode(node, parent) {
		if (ofnode_read_u32(node, "vendor-id", &vendor_id) ||
		    ofnode_read_u32(node, "device-id", &device_id)) {
			log_debug("Cannot get IDs for '%s'\n",
				  ofnode_get_name(node));
			return -EINVAL;
		}
		if (id != (vendor_id << 16 | device_id)) {
			log_debug("Skip codec node '%s' for %08x\n",
				  ofnode_get_name(node), id);
			continue;
		}

		log_debug("Found codec node '%s' for %08x\n",
			  ofnode_get_name(node), id);
		*nodep = node;
		return 0;
	}

	return -ENOENT;
}

static int send_verbs(ofnode node, const char *prop_name, struct hda_regs *regs)
{
	int ret, verb_size, i;
	const u32 *verb;

	verb = ofnode_get_property(node, prop_name, &verb_size);
	if (verb_size < 0) {
		log_debug("No verb data\n");
		return -EINVAL;
	}
	log_debug("verb_size: %d\n", verb_size);

	for (i = 0; i < verb_size / sizeof(*verb); i++) {
		ret = hda_wait_for_ready(regs);
		if (ret) {
			log_debug("  codec ready timeout\n");
			return ret;
		}

		writel(fdt32_to_cpu(verb[i]), &regs->cmd);

		ret = hda_wait_for_valid(regs);
		if (ret) {
			log_debug("  codec valid timeout\n");
			return ret;
		}
	}

	return 0;
}

static int codec_init(struct udevice *dev, struct hda_regs *regs, uint addr)
{
	ofnode node;
	uint id;
	int ret;

	log_debug("Initializing codec #%d\n", addr);
	ret = hda_wait_for_ready(regs);
	if (ret) {
		log_debug("  codec not ready\n");
		return ret;
	}

	/* Read the codec's vendor ID */
	writel(addr << AZALIA_CODEC_SHIFT |
	       AZALIA_OPCODE_READ_PARAM << AZALIA_VERB_SHIFT |
	       AZALIA_PARAM_VENDOR_ID, &regs->cmd);
	ret = hda_wait_for_valid(regs);
	if (ret) {
		log_debug("  codec not valid\n");
		return ret;
	}

	id = readl(&regs->resp);
	log_debug("codec vid/did: %08x\n", id);
	ret = find_verb_data(dev, id, &node);
	if (ret) {
		log_debug("No verb (err=%d)\n", ret);
		return ret;
	}
	ret = send_verbs(node, "verbs", regs);
	if (ret) {
		log_debug("failed to send verbs (err=%d)\n", ret);
		return ret;
	}
	log_debug("verb loaded\n");

	return 0;
}

int hda_codecs_init(struct udevice *dev, struct hda_regs *regs, u32 codec_mask)
{
	int ret;
	int i;

	for (i = 3; i >= 0; i--) {
		if (codec_mask & (1 << i)) {
			ret = codec_init(dev, regs, i);
			if (ret)
				return ret;
		}
	}

	ret = send_verbs(dev_ofnode(dev), "beep-verbs", regs);
	if (ret) {
		log_debug("failed to send beep verbs (err=%d)\n", ret);
		return ret;
	}
	log_debug("beep verbs loaded\n");

	return 0;
}

/**
 * exec_verb() - Write a verb to the codec
 *
 * @regs: HDA registers
 * @val: Command to write
 * @response: Set to response from codec
 * Return: 0 if OK, -ve on error
 */
static int exec_verb(struct hda_regs *regs, uint val, uint *response)
{
	int ret;

	ret = hda_wait_for_ready(regs);
	if (ret)
		return ret;

	writel(val, &regs->cmd);

	return wait_for_response(regs, response);
}

/**
 * get_subnode_info() - Get subnode information
 *
 * @regs: HDA registers
 * @nid: Parent node ID to check
 * @num_sub_nodesp: Returns number of subnodes
 * @start_sub_node_nidp: Returns start subnode number
 * Return: 0 if OK, -ve on error
 */
static int get_subnode_info(struct hda_regs *regs, uint nid,
			    uint *num_sub_nodesp, uint *start_sub_node_nidp)
{
	uint response;
	int ret;

	ret = exec_verb(regs, hda_verb(nid, HDA_VERB_GET_PARAMS,
				       GET_PARAMS_NODE_COUNT),
			&response);
	if (ret < 0) {
		printf("Audio: Error reading sub-node info %d\n", nid);
		return ret;
	}

	*num_sub_nodesp = (response & NUM_SUB_NODES_M) >> NUM_SUB_NODES_S;
	*start_sub_node_nidp = (response & FIRST_SUB_NODE_M) >>
			 FIRST_SUB_NODE_S;

	return 0;
}

/**
 * find_beep_node_in_group() - Finds the beeping node
 *
 * Searches the audio group for a node that supports beeping
 *
 * @regs: HDA registers
 * @group_nid: Group node ID to check
 * Return: 0 if OK, -ve on error
 */
static uint find_beep_node_in_group(struct hda_regs *regs, uint group_nid)
{
	uint node_count = 0;
	uint current_nid = 0;
	uint response;
	uint end_nid;
	int ret;

	ret = get_subnode_info(regs, group_nid, &node_count, &current_nid);
	if (ret < 0)
		return 0;

	end_nid = current_nid + node_count;
	while (current_nid < end_nid) {
		ret = exec_verb(regs,
				hda_verb(current_nid, HDA_VERB_GET_PARAMS,
					 GET_PARAMS_AUDIO_WIDGET_CAPS),
				&response);
		if (ret < 0) {
			printf("Audio: Error reading widget caps\n");
			return 0;
		}

		if ((response & AUDIO_WIDGET_TYPE_M) >> AUDIO_WIDGET_TYPE_S ==
		    AUDIO_WIDGET_TYPE_BEEP)
			return current_nid;

		current_nid++;
	}

	return 0; /* no beep node found */
}

/**
 * audio_group_has_beep_node() - Check if group has a beep node
 *
 * Checks if the given audio group contains a beep generator
 * @regs: HDA registers
 * @nid: Node ID to check
 * Return: 0 if OK, -ve on error
 */
static int audio_group_has_beep_node(struct hda_regs *regs, uint nid)
{
	uint response;
	int ret;

	ret = exec_verb(regs, hda_verb(nid, HDA_VERB_GET_PARAMS,
				       GET_PARAMS_AUDIO_GROUP_CAPS),
			&response);
	if (ret < 0) {
		printf("Audio: Error reading audio group caps %d\n", nid);
		return 0;
	}

	return !!(response & AUDIO_GROUP_CAPS_BEEP_GEN);
}

/**
 * get_hda_beep_nid() - Finds the node ID of the beep node
 *
 * Finds the nid of the beep node if it exists. Starts at the root node, for
 * each sub-node checks if the group contains a beep node.  If the group
 * contains a beep node, polls each node in the group until it is found.
 *
 * If the device has a intel,beep-nid property, the value of that is used
 * instead.
 *
 * @dev: Sound device
 * Return: Node ID >0 if found, -ve error code otherwise
 */
static int get_hda_beep_nid(struct udevice *dev)
{
	struct hda_codec_priv *priv = dev_get_priv(dev);
	uint current_nid = 0;
	uint node_count = 0;
	uint end_nid;
	int ret;

	/* If the field exists, use the beep nid set in the fdt */
	ret = dev_read_u32(dev, "intel,beep-nid", &current_nid);
	if (!ret)
		return current_nid;

	ret = get_subnode_info(priv->regs, HDA_ROOT_NODE, &node_count,
			       &current_nid);
	if (ret < 0)
		return ret;

	end_nid = current_nid + node_count;
	while (current_nid < end_nid) {
		if (audio_group_has_beep_node(priv->regs, current_nid))
			return find_beep_node_in_group(priv->regs,
						       current_nid);
		current_nid++;
	}
	 /* no beep node found */

	return -ENOENT;
}

/**
 * set_beep_divisor() - Sets the beep divisor to set the pitch
 *
 * @priv: Device's private data
 * @divider: Divider value (0 to disable the beep)
 * Return: 0 if OK, -ve on error
 */
static int set_beep_divisor(struct hda_codec_priv *priv, uint divider)
{
	return exec_verb(priv->regs,
			 hda_verb(priv->beep_nid, HDA_VERB_SET_BEEP, divider),
			 NULL);
}

int hda_codec_init(struct udevice *dev)
{
	struct hda_codec_priv *priv = dev_get_priv(dev);
	ulong base_addr;

	base_addr = dm_pci_read_bar32(dev, 0);
	log_debug("base = %08lx\n", base_addr);
	if (!base_addr)
		return -EINVAL;

	priv->regs = (struct hda_regs *)base_addr;

	return 0;
}

int hda_codec_finish_init(struct udevice *dev)
{
	struct hda_codec_priv *priv = dev_get_priv(dev);
	int ret;

	ret = get_hda_beep_nid(dev);
	if (ret <= 0) {
		log_warning("Could not find beep NID (err=%d)\n", ret);
		return ret ? ret : -ENOENT;
	}
	priv->beep_nid = ret;

	return 0;
}

int hda_codec_start_beep(struct udevice *dev, int frequency_hz)
{
	struct hda_codec_priv *priv = dev_get_priv(dev);
	uint divider_val;

	if (!priv->beep_nid) {
		log_err("Failed to find a beep-capable node\n");
		return -ENOENT;
	}

	if (!frequency_hz)
		divider_val = 0;	/* off */
	else if (frequency_hz > BEEP_FREQ_BASE)
		divider_val = 1;
	else if (frequency_hz < BEEP_FREQ_BASE / 0xff)
		divider_val = 0xff;
	else
		divider_val = 0xff & (BEEP_FREQ_BASE / frequency_hz);

	return set_beep_divisor(priv, divider_val);
}

int hda_codec_stop_beep(struct udevice *dev)
{
	struct hda_codec_priv *priv = dev_get_priv(dev);

	return set_beep_divisor(priv, 0);
}

static const struct sound_ops hda_codec_ops = {
	.setup		= hda_codec_finish_init,
	.start_beep	= hda_codec_start_beep,
	.stop_beep	= hda_codec_stop_beep,
};

U_BOOT_DRIVER(hda_codec) = {
	.name		= "hda_codec",
	.id		= UCLASS_SOUND,
	.ops		= &hda_codec_ops,
	.priv_auto	= sizeof(struct hda_codec_priv),
	.probe		= hda_codec_init,
};

static struct pci_device_id hda_supported[] = {
	{ PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_COUGARPOINT_HDA},
	{ PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_PANTHERPOINT_HDA},
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL,
		PCI_DEVICE_ID_INTEL_WILDCATPOINT_HDA) },

	/*
	 * Note this driver is not necessarily generic, but it attempts to
	 * support any codec in the hd-audio class
	 */
	{ PCI_DEVICE_CLASS(PCI_CLASS_MULTIMEDIA_HD_AUDIO, 0xffffff) },
};

U_BOOT_PCI_DEVICE(hda_codec, hda_supported);
