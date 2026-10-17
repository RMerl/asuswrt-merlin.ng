/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Intel HDA audio codec config. This is a mechanicm to configure codecs when
 * using Intel HDA audio.
 *
 * Copyright 2018 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#ifndef __AZALIA_H
#define __AZALIA_H

#define AZALIA_CODEC_SHIFT		28
#define AZALIA_NID_SHIFT		20
#define AZALIA_VERB_SHIFT		8

/* Supported opcodes */
#define AZALIA_OPCODE_CONFIG_DEFAULT	0x71c
#define AZALIA_OPCODE_IMPL_ID		0x720
#define AZALIA_OPCODE_READ_PARAM	0xf00

#define AZALIA_PARAM_VENDOR_ID		0

/* Generate the register value to write a particular byte of a 32-bit value */
#define AZALIA_SET_BYTE(codec, nid, opcode, val, byte)		\
	((codec) << AZALIA_CODEC_SHIFT |			\
	 (nid) << AZALIA_NID_SHIFT |			\
	 ((opcode) + (byte)) << AZALIA_VERB_SHIFT |		\
	 (((val) >> ((byte) * 8)) & 0xff))

/* Generate the register value to write all bytes of a 32-bit value */
#define AZALIA_WORD(codec, nid, opcode, val)			\
	 (AZALIA_SET_BYTE(codec, nid, opcode, val, 0) |	\
	  AZALIA_SET_BYTE(codec, nid, opcode, val, 1) |		\
	  AZALIA_SET_BYTE(codec, nid, opcode, val, 2) |		\
	  AZALIA_SET_BYTE(codec, nid, opcode, val, 3))

#define AZALIA_PIN_CFG(codec, nid, val)				\
	 AZALIA_WORD(codec, nid, AZALIA_OPCODE_CONFIG_DEFAULT, val)

#define AZALIA_SUBVENDOR(codec, val)				\
	 AZALIA_WORD(codec, 1, AZALIA_OPCODE_IMPL_ID, val)

#endif /* __AZALIA_H */
