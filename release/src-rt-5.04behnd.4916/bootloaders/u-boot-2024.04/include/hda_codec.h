/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Support for Intel High-Definition Audio codec
 *
 * Copyright 2018 Google LLC
 *
 * Taken from coreboot file of the same name
 */

#ifndef __HDA_CODEC_H_
#define __HDA_CODEC_H_

struct hda_regs;

/**
 * struct hda_codec_priv - Private data required by the HDA codec
 *
 * @regs: HDA registers
 * @beep_nid: Node ID of beep node (>0)
 */
struct hda_codec_priv {
	struct hda_regs *regs;
	uint beep_nid;
};

/**
 *  hda_wait_for_ready() - Wait for the codec to indicate it is ready
 *
 * @regs: HDA registers
 * Return: 0 if OK -ETIMEDOUT if codec did not respond in time
 */
int hda_wait_for_ready(struct hda_regs *regs);

/**
 *  hda_wait_for_valid() - Wait for the codec to accept the last command
 *
 * @regs: HDA registers
 * Return: 0 if OK -ETIMEDOUT if codec did not respond in time
 */
int hda_wait_for_valid(struct hda_regs *regs);

/**
 * hda_codec_detect() - Detect which codecs are present
 *
 * @regs: HDA registers
 * Return: bit mask of active codecs (0 if none)
 * Return: 0 if OK, -ve on error
 */
int hda_codec_detect(struct hda_regs *regs);

/**
 * hda_codecs_init() - Init all codecs
 *
 * @dev: Sound device
 * @regs: HDA registers
 * @codec_mask: Mask of codecs to init (bits 3:0)
 * Return: 0 if OK, -ve on error
 */
int hda_codecs_init(struct udevice *dev, struct hda_regs *regs, u32 codec_mask);

/**
 * hda_codec_start_beep() - Start beeping
 *
 * This tells the sound hardware to start a beep. It will continue until stopped
 * by sound_stop_beep().
 *
 * @dev: Sound device
 * @frequency_hz: Beep frequency in hertz
 * Return: if OK, -ve on error
 */
int hda_codec_start_beep(struct udevice *dev, int frequency_hz);

/**
 * hda_codec_stop_beep() - Stop beeping
 *
 * This tells the sound hardware to stop a previously started beep.
 *
 * @dev: Sound device
 * Return: if OK, -ve on error
 */
int hda_codec_stop_beep(struct udevice *dev);

/**
 * hda_codec_init() - Set up the HDA codec base address
 *
 * This should be called at the start of the probe() method.
 *
 * @dev: Sound device
 * Return: 0 if OK, -ve on error
 */
int hda_codec_init(struct udevice *dev);

/**
 * hda_codec_finish_init() - Finish setting up the HDA codec base address
 *
 * This should be called at the end of the probe() method.
 *
 * @dev: Sound device
 * Return: 0 if OK, -ve on error
 */
int hda_codec_finish_init(struct udevice *dev);

#endif /* __HDA_CODEC_H_ */
