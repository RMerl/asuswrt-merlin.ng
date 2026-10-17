/* SPDX-License-Identifier: GPL-2.0+ */

#ifndef _NPCM_OTP_H_
#define _NPCM_OTP_H_

#ifdef CONFIG_ARCH_NPCM8XX
enum {
	NPCM_KEY_SA    = 0,
	NPCM_FUSE_SA   = 0,
	NPCM_NUM_OF_SA   = 1
};
#else
enum {
	NPCM_KEY_SA    = 0,
	NPCM_FUSE_SA   = 1,
	NPCM_NUM_OF_SA = 2
};
#endif

struct npcm_otp_regs {
	unsigned int fst;
	unsigned int faddr;
	unsigned int fdata;
	unsigned int fcfg;
	unsigned int fustrap_fkeyind;
	unsigned int fctl;
};

#define FST_RDY                 BIT(0)
#define FST_RDST                BIT(1)
#define FST_RIEN                BIT(2)

#ifdef CONFIG_ARCH_NPCM8XX
#define FADDR_BYTEADDR(addr)        ((addr) << 3)
#define FADDR_BITPOS(pos)           ((pos) << 0)
#define FADDR_VAL(addr, pos)        (FADDR_BITPOS(pos) | FADDR_BYTEADDR(addr))
#define FADDR_IN_PROG               BIT(16)
#else
#define FADDR_BYTEADDR(addr)    ((addr) << 0)
#define FADDR_BITPOS(pos)       ((pos) << 10)
#define FADDR_VAL(addr, pos)    (FADDR_BYTEADDR(addr) | FADDR_BITPOS(pos))
#define FADDR_IN_PROG               BIT(16)
#endif

#define FDATA_MASK              (0xff)

#define FUSTRAP_O_SECBOOT       BIT(23)

#define FCFG_FDIS               BIT(31)

#define FKEYIND_KVAL            BIT(0)
#define FKEYIND_KSIZE_MASK      (0x00000070)
#define FKEYIND_KSIZE_128       (0x40)
#define FKEYIND_KSIZE_192       (0x50)
#define FKEYIND_KSIZE_256       (0x60)
#define FKEYIND_KIND_MASK       (0x000c0000)
#define FKEYIND_KIND_KEY(indx)  ((indx) << 18)

// Program cycle initiation values (sequence of two adjacent writes)
#define PROGRAM_ARM             0x1
#define PROGRAM_INIT            0xBF79E5D0

#define OTP2_BASE               0xF018A000
#define FUSTRAP                 (OTP2_BASE + 0x10)

// Read cycle initiation value
#define READ_INIT               0x02

// Value to clean FDATA contents
#define FDATA_CLEAN_VALUE       0x01

#ifdef CONFIG_ARCH_NPCM8XX
#define NPCM_OTP_ARR_BYTE_SIZE        8192
#else
#define NPCM_OTP_ARR_BYTE_SIZE        1024
#endif

#define MIN_PROGRAM_PULSES               4
#define MAX_PROGRAM_PULSES               20
#define NPCM_OTP_ARR_BYTE_SIZE        1024

int fuse_prog_image(u32 bank, uintptr_t address);
int  fuse_program_data(u32 bank, u32 word, u8 *data, u32 size);
int  npcm_otp_select_key(u8 key_index);
bool npcm_otp_is_fuse_array_disabled(u32 arr);
void npcm_otp_nibble_parity_ecc_encode(u8 *datain, u8 *dataout, u32 size);
void npcm_otp_majority_rule_ecc_encode(u8 *datain, u8 *dataout, u32 size);
void npcm_arch_preboot_os(void);

#endif
