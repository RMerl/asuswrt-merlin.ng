/*
 *    Copyright IBM Corp. 2007
 *    Author(s): Heiko Carstens <heiko.carstens@de.ibm.com>
 */

#ifndef _ASM_S390_SCLP_H
#define _ASM_S390_SCLP_H

#include <linux/types.h>
#include <asm/chpid.h>
#include <asm/cpu.h>

#define SCLP_CHP_INFO_MASK_SIZE		32

struct sclp_chp_info {
	u8 recognized[SCLP_CHP_INFO_MASK_SIZE];
	u8 standby[SCLP_CHP_INFO_MASK_SIZE];
	u8 configured[SCLP_CHP_INFO_MASK_SIZE];
};

#define LOADPARM_LEN 8

struct sclp_ipl_info {
	int is_valid;
	int has_dump;
	char loadparm[LOADPARM_LEN];
};

struct sclp_cpu_entry {
	u8 core_id;
	u8 reserved0[2];
	u8 : 3;
	u8 siif : 1;
	u8 sigpif : 1;
	u8 : 3;
	u8 reserved2[10];
	u8 type;
	u8 reserved1;
} __attribute__((packed));

struct sclp_cpu_info {
	unsigned int configured;
	unsigned int standby;
	unsigned int combined;
	int has_cpu_type;
	struct sclp_cpu_entry cpu[MAX_CPU_ADDRESS + 1];
};

int sclp_get_cpu_info(struct sclp_cpu_info *info);
int sclp_cpu_configure(u8 cpu);
int sclp_cpu_deconfigure(u8 cpu);
unsigned long long sclp_get_rnmax(void);
unsigned long long sclp_get_rzm(void);
unsigned int sclp_get_max_cpu(void);
unsigned int sclp_get_mtid(u8 cpu_type);
unsigned int sclp_get_mtid_max(void);
unsigned int sclp_get_mtid_prev(void);
int sclp_sdias_blk_count(void);
int sclp_sdias_copy(void *dest, int blk_num, int nr_blks);
int sclp_chp_configure(struct chp_id chpid);
int sclp_chp_deconfigure(struct chp_id chpid);
int sclp_chp_read_info(struct sclp_chp_info *info);
void sclp_get_ipl_info(struct sclp_ipl_info *info);
bool __init sclp_has_linemode(void);
bool __init sclp_has_vt220(void);
bool sclp_has_sprp(void);
int sclp_pci_configure(u32 fid);
int sclp_pci_deconfigure(u32 fid);
int memcpy_hsa(void *dest, unsigned long src, size_t count, int mode);
unsigned long sclp_get_hsa_size(void);
void sclp_early_detect(void);
int sclp_has_siif(void);
int sclp_has_sigpif(void);
unsigned int sclp_get_ibc(void);

long _sclp_print_early(const char *);

#endif /* _ASM_S390_SCLP_H */
