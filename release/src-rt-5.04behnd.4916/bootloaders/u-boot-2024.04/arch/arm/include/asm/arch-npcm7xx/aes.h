/* SPDX-License-Identifier: GPL-2.0+ */

#ifndef _NPCM_AES_H_
#define _NPCM_AES_H_

#define AES_OP_ENCRYPT          0
#define AES_OP_DECRYPT          1
#define SIZE_AES_BLOCK          (AES128_KEY_LENGTH)

struct npcm_aes_regs {
	unsigned char reserved_0[0x400];    // 0x000
	unsigned int aes_key_0;             // 0x400
	unsigned int aes_key_1;             // 0x404
	unsigned int aes_key_2;             // 0x408
	unsigned int aes_key_3;             // 0x40c
	unsigned char reserved_1[0x30];     // 0x410
	unsigned int aes_iv_0;              // 0x440
	unsigned char reserved_2[0x1c];     // 0x444
	unsigned int aes_ctr_0;             // 0x460
	unsigned char reserved_3[0x0c];     // 0x464
	unsigned int aes_busy;              // 0x470
	unsigned char reserved_4[0x04];     // 0x474
	unsigned int aes_sk;                // 0x478
	unsigned char reserved_5[0x14];     // 0x47c
	unsigned int aes_prev_iv_0;         // 0x490
	unsigned char reserved_6[0x0c];     // 0x494
	unsigned int aes_din_dout;          // 0x4a0
	unsigned char reserved_7[0x1c];     // 0x4a4
	unsigned int aes_control;           // 0x4c0
	unsigned int aes_version;           // 0x4c4
	unsigned int aes_hw_flags;          // 0x4c8
	unsigned char reserved_8[0x28];     // 0x4cc
	unsigned int aes_sw_reset;          // 0x4f4
	unsigned char reserved_9[0x08];     // 0x4f8
	unsigned int aes_fifo_data;         // 0x500
	unsigned char reserved_10[0xfc];    // 0x504
	unsigned int aes_fifo_status;       // 0x600
};

#define AES_BUSY_BIT            BIT(0)
#define SW_RESET_BIT            BIT(0)
#define AES_SK_BIT              BIT(0)

#define DIN_FIFO_FULL           BIT(0)
#define DIN_FIFO_EMPTY          BIT(1)
#define DOUT_FIFO_FULL          BIT(2)
#define DOUT_FIFO_EMPTY         BIT(3)
#define DIN_FIFO_OVERFLOW       BIT(4)
#define DOUT_FIFO_UNDERFLOW     BIT(5)

int npcm_aes_select_key(u8 fkeyind);

#endif
