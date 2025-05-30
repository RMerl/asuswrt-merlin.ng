/*
   Copyright (c) 2014 Broadcom
   All Rights Reserved

    <:label-BRCM:2014:DUAL/GPL:standard
    
    Unless you and Broadcom execute a separate written software license
    agreement governing use of this software, this software is licensed
    to you under the terms of the GNU General Public License version 2
    (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
    with the following added to such license:
    
       As a special exception, the copyright holders of this software give
       you permission to link this software with independent modules, and
       to copy and distribute the resulting executable under terms of your
       choice, provided that you also meet, for each linked independent
       module, the terms and conditions of the license of that module.
       An independent module is a module which is not derived from this
       software.  The special exception does not apply to any modifications
       of the software.
    
    Not withstanding the above, under no circumstances may you combine
    this software in any way with any other Broadcom software provided
    under a license other than the GPL, without Broadcom's express prior
    written consent.
    
:>
*/

uint16_t fw_predict_5[] = {
0xfffe,
0xe3ff,
0xffff,
0xfe3f,
0xffff,
0xffe3,
0x3fff,
0xfffe,
0xe3ff,
0xffff,
0xfe3f,
0xffff,
0xffe3,
0x3fff,
0x8086,
0xfb5b,
0x2388,
0xc025,
0xa779,
0x18a3,
0x88cd,
0xf424,
0x07c0,
0x8426,
0x725d,
0x5d4e,
0x72e9,
0x9f39,
0x8448,
0x8398,
0x6684,
0x7fc4,
0x8e2c,
0x7797,
0x4221,
0x7630,
0xd84b,
0xc92a,
0x0501,
0x768d,
0xf83b,
0xa0d7,
0xc8d2,
0x29a0,
0xc52e,
0xf182,
0x9718,
0x4e17,
0xb4ae,
0x8336,
0x531a,
0xc400,
0x15a1,
0x8814,
0x5280,
0x52a2,
0x77da,
0xf10b,
0x66c2,
0xd548,
0x682c,
0x000f,
0x7779,
0x6e05,
0x247c,
0xb0b3,
0x4aa7,
0x9bca,
0x2986,
0xebb7,
0x9b81,
0xc882,
0x68c3,
0xc02a,
0x600c,
0xc959,
0x9a72,
0x1463,
0xb05b,
0x2807,
0x4e89,
0x7a51,
0x2fa1,
0xad4f,
0x3405,
0x5b30,
0xcc94,
0xc9a2,
0x5240,
0x2cdb,
0x1080,
0x4465,
0x60cb,
0x0089,
0x728b,
0x831a,
0x348a,
0x2278,
0xb6b6,
0x0331,
0x86bb,
0x9ee4,
0xe4c6,
0x0051,
0xeb19,
0x1f24,
0x9244,
0x230f,
0x07c9,
0x46c3,
0x2d4e,
0x8788,
0xc648,
0x3c46,
0x30fa,
0x53db,
0xe87f,
0xa0b5,
0xd409,
0x5746,
0x50cf,
0x3005,
0x705a,
0x5086,
0xc680,
0x0f71,
0x6029,
0x704f,
0x8a0c,
0x331d,
0x6f21,
0x4bdc,
0x889c,
0x8569,
0x9965,
0xfad1,
0x5bf2,
0xede0,
0x1512,
0x87f6,
0xaefc,
0xad2e,
0x6159,
0x0191,
0x9482,
0x2022,
0xaa4c,
0xf905,
0x5cbf,
0xe827,
0x57ad,
0xc261,
0x9881,
0x6f7a,
0x7606,
0x889e,
0xb159,
0x5ffc,
0x41e6,
0x58e0,
0x81b5,
0xae4c,
0x125f,
0x6de9,
0x9925,
0xe748,
0x425f,
0x3eca,
0x246c,
0x8940,
0xb025,
0x42ef,
0x41d0,
0xa5e8,
0xc902,
0xce64,
0x8832,
0x1017,
0x9090,
0x9227,
0xbdb9,
0x6cc9,
0x31c6,
0x8208,
0x4bdb,
0x308d,
0x001b,
0x3a46,
0xa951,
0xb2bb,
0xe45c,
0x3180,
0x1900,
0xbeeb,
0x4237,
0x1bde,
0xef80,
0xa20d,
0x2982,
0xa257,
0x4a48,
0x4800,
0xa06e,
0x83e1,
0x4209,
0x3f39,
0x0826,
0x4812,
0x027a,
0xaa72,
0x4086,
0x6161,
0x8db6,
0xf267,
0x0b55,
0x39ea,
0xd538,
0xa80e,
0x4004,
0x1293,
0x5a49,
0x31e8,
0x4a0a,
0x1989,
0x8150,
0x3d2a,
0xc2a8,
0x5001,
0xc200,
0xf0d1,
0xd042,
0x2004,
0xbdc0,
0x6a22,
0x992a,
0xf06a,
0x2e6b,
0x8c80,
0xa618,
0xc532,
0x8513,
0x9b95,
0x32c0,
0xe2b3,
0x32e7,
0x1560,
0xada1,
0xd686,
0x9816,
0x4f18,
0xb31c,
0x179c,
0xbb6a,
0xd706,
0x980a,
0xaca9,
0xec30,
0x69e3,
0x106a,
0x08b5,
0x2156,
0x1c11,
0xa0f1,
0x9369,
0x5378,
0xa131,
0x745e,
0x1138,
0x483c,
0x4985,
0x2444,
0x9e04,
0x12b9,
0x868e,
0x8e92,
0x4c10,
0xb4c9,
0x40b3,
0xf09a,
0x4b68,
0x9f1a,
0xa3de,
0x3139,
0x2e38,
0xe1d2,
0xa0b5,
0x17af,
0x9855,
0xa80a,
0x6a9e,
0xd565,
0x9a0e,
0x70d1,
0xa41c,
0x1088,
0xbb0b,
0x737c,
0x1831,
0x4948,
0x01d9,
0x111a,
0x1d36,
0xa6ea,
0x75f1,
0x4ea4,
0x83ba,
0x7082,
0x8086,
0x9de1,
0x0287,
0x10db,
0x4097,
0x6315,
0x00e2,
0x4986,
0x5f0b,
0xdb63,
0x4476,
0xf3a6,
0xcbc5,
0xa563,
0x0c00,
0x6738,
0x7a24,
0x2ebc,
0x3d65,
0x8450,
0x329c,
0x3192,
0x9b1a,
0x8d32,
0xfb88,
0x145c,
0x13c8,
0x0768,
0x99e6,
0x3d46,
0x6eab,
0xd028,
0xa089,
0xf9ae,
0xb340,
0x91d7,
0x7001,
0x83d8,
0x0dd3,
0xca5b,
0xc488,
0x9e8b,
0x6993,
0xf79d,
0x25c3,
0x302c,
0x6106,
0xd7ba,
0xdd1b,
0xf236,
0x2ccc,
0x6c9c,
0xfe0d,
0x6999,
0x7d8a,
0x1123,
0xf902,
0x84e4,
0x7d3a,
0xc204,
0xc08f,
0xf6d8,
0xb105,
0x1493,
0xc927,
0x10c4,
0x76ea,
0x28aa,
0x249e,
0xe901,
0x2b0a,
0x2299,
0xd86e,
0xdffb,
0x9a4c,
0x7b41,
0x1ca3,
0xa6b7,
0x2103,
0x105a,
0x0676,
0xcc53,
0x90ff,
0xd0af,
0x6196,
0x9147,
0x8848,
0xe6bc,
0x373a,
0x0d9f,
0x3d71,
0xde2c,
0x2a12,
0x05b6,
0xf243,
0xbeeb,
0xeed2,
0xec48,
0x6513,
0x98c9,
0x10e1,
0x30d4,
0xd702,
0x78cc,
0x5a72,
0x3305,
0x94bc,
0xa346,
0x4c6a,
0x3673,
0x7054,
0xac83,
0xae67,
0x0a45,
0x0d8a,
0x4a5e,
0x51a3,
0xb83d,
0x0a4d,
0x9635,
0x81f6,
0xc2f1,
0x48bb,
0x3d87,
0x4eef,
0x4d4f,
0x4774,
0xd498,
0x1467,
0x8203,
0x9842,
0x0383,
0x1208,
0xdcef,
0xd6ca,
0x477e,
0xec80,
0x0855,
0x801b,
0x67b7,
0xddb8,
0xcf05,
0xe074,
0x82a6,
0x877e,
0xa8cb,
0x9f9c,
0x927e,
0x3640,
0xb8a6,
0x5930,
0x44b1,
0xb69a,
0xd695,
0x0366,
0x0000,
0x0000,
0x0000,
0x0000,
0x0000,
0x0000,
0x0000,
0x0000,
0x0000,
0x0000,
0x0000,
0x0000,
0x0000,
0x0000,
0x0000,
0x0000,
0x0000,
0x0000,
0x0000,
0x0000,
0x0000,
0x0000,
0x0000,
0x0000,
0x0000,
0x0000,
0x0000,
0x0000,
0x0000,
0x0000,
0x0000,
0x0000,
0x0000,
0x0000,
0x0000,
0x0000,
0x0000,
0x0000,
0x0000,
0x0000,
0x0000,
0x0000,
0x0000,
0x0000,
0x0000,
0x0000,
0x0000,
0x0000,
0x0000,
0x0000,
};
