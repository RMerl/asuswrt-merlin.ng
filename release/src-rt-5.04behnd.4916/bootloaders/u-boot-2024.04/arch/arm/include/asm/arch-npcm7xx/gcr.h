/* SPDX-License-Identifier: GPL-2.0+ */

#ifndef __NPCM750_GCR_H_
#define __NPCM750_GCR_H_

/* On-Chip POLEG NPCM750 VERSIONS */
#define POLEG_Z1                    0x00A92750
#define POLEG_A1                    0x04A92750
#define POLEG_NPCM750				0x00000000
#define POLEG_NPCM730				0x00300395
#define POLEG_NPCM710				0x00200380

#define PWRON_SECEN                    7         /* STRAP8 */
#define NPCM_GCR_BA                  0xF0800000

struct npcm_gcr {
	unsigned int  pdid;
	unsigned int  pwron;
	unsigned char res1[0x4];
	unsigned int  mfsel1;
	unsigned int  mfsel2;
	unsigned int  miscpe;
	unsigned char res2[0x20];
	unsigned int  spswc;
	unsigned int  intcr;
	unsigned int  intsr;
	unsigned char res3[0xc];
	unsigned int  hifcr;
	unsigned int  sd1irv1;
	unsigned int  sd1irv2;
	unsigned char res4[0x4];
	unsigned int  intcr2;
	unsigned int  mfsel3;
	unsigned int  srcnt;
	unsigned int  ressr;
	unsigned int  rlockr1;
	unsigned int  flockr1;
	unsigned int  dscnt;
	unsigned int  mdlr;
	unsigned char res5[0x18];
	unsigned int  davclvlr;
	unsigned int  intcr3;
	unsigned char res6[0xc];
	unsigned int  vsintr;
	unsigned int  mfsel4;
	unsigned int  sd2irv1;
	unsigned int  sd2irv2;
	unsigned char res7[0x8];
	unsigned int  cpbpntr;
	unsigned char res8[0x8];
	unsigned int  cpctl;
	unsigned int  cp2bst;
	unsigned int  b2cpnt;
	unsigned int  cppctl;
	unsigned int  i2csegsel;
	unsigned int  i2csegctl;
	unsigned int  vsrcr;
	unsigned int  mlockr;
	unsigned char res9[0x4c];
	unsigned int  scrpad;
	unsigned int  usb1phyctl;
	unsigned int  usb2phyctl;
};

#endif
