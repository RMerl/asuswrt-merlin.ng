/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright 2019 Google LLC
 */

#ifndef _DT_BINDINGS_SOUND_NHLT_H
#define _DT_BINDINGS_SOUND_NHLT_H

/* See Table 2-1. NHLT Endpoint Descriptor in the NHLT Specification (0.8.1) */
#define NHLT_VID	0x8086
#define NHLT_DID_DMIC	0xae20
#define NHLT_DID_BT	0xae30
#define NHLT_DID_SSP	0xae34

/* Hardware links available to use for codecs */
#define AUDIO_LINK_SSP0		0
#define AUDIO_LINK_SSP1		1
#define AUDIO_LINK_SSP2		2
#define AUDIO_LINK_SSP3		3
#define AUDIO_LINK_SSP4		4
#define AUDIO_LINK_SSP5		5
#define AUDIO_LINK_DMIC		6

#endif /* _DT_BINDINGS_SOUND_NHLT_H */
