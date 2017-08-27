/*
   iecdump - dump IEC958 status bits on ALSA
   Copyright (C) 2003 by Takashi Iwai <tiwai@suse.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include <stdio.h>
#include <alsa/asoundlib.h>

struct category_str {
	int val;
	const char *name;
};

static const struct category_str con_category[] = {
	{ IEC958_AES1_CON_GENERAL, "general" },

	{ IEC958_AES1_CON_IEC908_CD, "CD" },
	{ IEC958_AES1_CON_NON_IEC908_CD, "non-IEC908 CD" },
	{ IEC958_AES1_CON_MINI_DISC, "Mini-Disc" },
	{ IEC958_AES1_CON_DVD, "DVD" },

	{ IEC958_AES1_CON_PCM_CODER, "PCM coder" },
	{ IEC958_AES1_CON_MIXER, "digital signal mixer" },
	{ IEC958_AES1_CON_RATE_CONVERTER, "rate converter" },
	{ IEC958_AES1_CON_SAMPLER, "sampler" },
	{ IEC958_AES1_CON_DSP, "digital sound processor" },

	{ IEC958_AES1_CON_DAT, "DAT" },
	{ IEC958_AES1_CON_VCR, "VCR" },
	{ IEC958_AES1_CON_DCC, "DCC" },
	{ IEC958_AES1_CON_MAGNETIC_DISC, "magnetic disc" },

	{ IEC958_AES1_CON_DAB_JAPAN, "digital audio broadcast (Japan)" },
	{ IEC958_AES1_CON_DAB_EUROPE, "digital audio broadcast (Europe)" },
	{ IEC958_AES1_CON_DAB_USA, "digital audio broadcast (USA)" },
	{ IEC958_AES1_CON_SOFTWARE, "software delivery" },

	{ IEC958_AES1_CON_SYNTHESIZER, "synthesizer" },
	{ IEC958_AES1_CON_MICROPHONE, "microphone" },

	{ IEC958_AES1_CON_ADC, "ADC without copyright information" },

	{ IEC958_AES1_CON_ADC_COPYRIGHT, "ADC with copyright information" },

	{ IEC958_AES1_CON_SOLIDMEM_DIGITAL_RECORDER_PLAYER, "flash memory recorder/player" },

	{ IEC958_AES1_CON_EXPERIMENTAL, "experimental" },
};


#define ARRAY_SIZE(x) (int)(sizeof(x)/sizeof(x[0]))

void dump_iec958(snd_aes_iec958_t *iec)
{
	int i;

	if (! (iec->status[0] & IEC958_AES0_PROFESSIONAL)) {
		/* consumer */
		printf("Mode: consumer\n");
		printf("Data: ");
		if (!(iec->status[0] & IEC958_AES0_NONAUDIO)) {
			printf("audio\n");
		} else {
			printf("non-audio\n");
		}
		printf("Rate: ");
		switch (iec->status[3] & IEC958_AES3_CON_FS) {
		case IEC958_AES3_CON_FS_22050:
			printf("22050 Hz\n");
			break;
		case IEC958_AES3_CON_FS_24000:
			printf("24000 Hz\n");
			break;
		case IEC958_AES3_CON_FS_32000:
			printf("32000 Hz\n");
			break;
		case IEC958_AES3_CON_FS_44100:
			printf("44100 Hz\n");
			break;
		case IEC958_AES3_CON_FS_48000:
			printf("48000 Hz\n");
			break;
		case IEC958_AES3_CON_FS_88200:
			printf("88200 Hz\n");
			break;
		case IEC958_AES3_CON_FS_96000:
			printf("96000 Hz\n");
			break;
		case IEC958_AES3_CON_FS_176400:
			printf("176400 Hz\n");
			break;
		case IEC958_AES3_CON_FS_192000:
			printf("192000 Hz\n");
			break;
		case IEC958_AES3_CON_FS_768000:
			printf("768000 Hz\n");
			break;
		case IEC958_AES3_CON_FS_NOTID:
			printf("not indicated\n");
			break;
		default:
			printf("unknown\n");
			break;
		}
		printf("Copyright: ");
		if (iec->status[0] & IEC958_AES0_CON_NOT_COPYRIGHT) {
			printf("permitted\n");
		} else {
			printf("protected\n");
		}
		printf("Emphasis: ");
		if ((iec->status[0] & IEC958_AES0_CON_EMPHASIS) != IEC958_AES0_CON_EMPHASIS_5015) {
			printf("none\n");
		} else {
			printf("50/15us\n");
		}
		printf("Category: ");
		for (i = 0; i < ARRAY_SIZE(con_category); i++) {
			if ((iec->status[1] & IEC958_AES1_CON_CATEGORY) == con_category[i].val) {
				printf("%s\n", con_category[i].name);
				break;
			}
		}
		if (i >= ARRAY_SIZE(con_category)) {
			printf("unknown 0x%x\n", iec->status[1] & IEC958_AES1_CON_CATEGORY);
		}
		printf("Original: ");
		if (iec->status[1] & IEC958_AES1_CON_ORIGINAL) {
			printf("original\n");
		} else {
			printf("1st generation\n");
		}
		printf("Clock: ");
		switch (iec->status[3] & IEC958_AES3_CON_CLOCK) {
		case IEC958_AES3_CON_CLOCK_1000PPM:
			printf("1000 ppm\n");
			break;
		case IEC958_AES3_CON_CLOCK_50PPM:
			printf("50 ppm\n");
			break;
		case IEC958_AES3_CON_CLOCK_VARIABLE:
			printf("variable pitch\n");
			break;
		default:
			printf("unknown\n");
			break;
		}
	} else {
		printf("Mode: professional\n");
		printf("Data: ");
		if (!(iec->status[0] & IEC958_AES0_NONAUDIO)) {
			printf("audio\n");
		} else {
			printf("non-audio\n");
		}
		printf("Rate: ");
		switch (iec->status[0] & IEC958_AES0_PRO_FS) {
		case IEC958_AES0_PRO_FS_44100:
			printf("44100 Hz\n");
			break;
		case IEC958_AES0_PRO_FS_48000:
			printf("48000 Hz\n");
			break;
		case IEC958_AES0_PRO_FS_32000:
			printf("32000 Hz\n");
			break;
		default:
			printf("unknown\n");
			break;
		}
		printf("Rate Locked: ");
		if (iec->status[0] & IEC958_AES0_PRO_FREQ_UNLOCKED)
			printf("no\n");
		else
			printf("yes\n");
		printf("Emphasis: ");
		switch (iec->status[0] & IEC958_AES0_PRO_EMPHASIS) {
		case IEC958_AES0_PRO_EMPHASIS_CCITT:
			printf("CCITT J.17\n");
			break;
		case IEC958_AES0_PRO_EMPHASIS_NONE:
			printf("none\n");
			break;
		case IEC958_AES0_PRO_EMPHASIS_5015:
			printf("50/15us\n");
			break;
		case IEC958_AES0_PRO_EMPHASIS_NOTID:
		default:
			printf("unknown\n");
			break;
		}
		printf("Stereophonic: ");
		if ((iec->status[1] & IEC958_AES1_PRO_MODE) == IEC958_AES1_PRO_MODE_STEREOPHONIC) {
			printf("stereo\n");
		} else {
			printf("not indicated\n");
		}
		printf("Userbits: ");
		switch (iec->status[1] & IEC958_AES1_PRO_USERBITS) {
		case IEC958_AES1_PRO_USERBITS_192:
			printf("192bit\n");
			break;
		case IEC958_AES1_PRO_USERBITS_UDEF:
			printf("user-defined\n");
			break;
		default:
			printf("unkown\n");
			break;
		}
		printf("Sample Bits: ");
		switch (iec->status[2] & IEC958_AES2_PRO_SBITS) {
		case IEC958_AES2_PRO_SBITS_20:
			printf("20 bit\n");
			break;
		case IEC958_AES2_PRO_SBITS_24:
			printf("24 bit\n");
			break;
		case IEC958_AES2_PRO_SBITS_UDEF:
			printf("user defined\n");
			break;
		default:
			printf("unknown\n");
			break;
		}
		printf("Word Length: ");
		switch (iec->status[2] & IEC958_AES2_PRO_WORDLEN) {
		case IEC958_AES2_PRO_WORDLEN_22_18:
			printf("22 bit or 18 bit\n");
			break;
		case IEC958_AES2_PRO_WORDLEN_23_19:
			printf("23 bit or 19 bit\n");
			break;
		case IEC958_AES2_PRO_WORDLEN_24_20:
			printf("24 bit or 20 bit\n");
			break;
		case IEC958_AES2_PRO_WORDLEN_20_16:
			printf("20 bit or 16 bit\n");
			break;
		default:
			printf("unknown\n");
			break;
		}
	}
}
