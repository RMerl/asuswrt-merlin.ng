#ifndef _ASM_X86_MICROCODE_INTEL_H
#define _ASM_X86_MICROCODE_INTEL_H

#include <asm/microcode.h>

struct microcode_header_intel {
	unsigned int            hdrver;
	unsigned int            rev;
	unsigned int            date;
	unsigned int            sig;
	unsigned int            cksum;
	unsigned int            ldrver;
	unsigned int            pf;
	unsigned int            datasize;
	unsigned int            totalsize;
	unsigned int            reserved[3];
};

struct microcode_intel {
	struct microcode_header_intel hdr;
	unsigned int            bits[0];
};

/* microcode format is extended from prescott processors */
struct extended_signature {
	unsigned int            sig;
	unsigned int            pf;
	unsigned int            cksum;
};

struct extended_sigtable {
	unsigned int            count;
	unsigned int            cksum;
	unsigned int            reserved[3];
	struct extended_signature sigs[0];
};

#define DEFAULT_UCODE_DATASIZE	(2000)
#define MC_HEADER_SIZE		(sizeof(struct microcode_header_intel))
#define DEFAULT_UCODE_TOTALSIZE (DEFAULT_UCODE_DATASIZE + MC_HEADER_SIZE)
#define EXT_HEADER_SIZE		(sizeof(struct extended_sigtable))
#define EXT_SIGNATURE_SIZE	(sizeof(struct extended_signature))
#define DWSIZE			(sizeof(u32))

#define get_totalsize(mc) \
	(((struct microcode_intel *)mc)->hdr.datasize ? \
	 ((struct microcode_intel *)mc)->hdr.totalsize : \
	 DEFAULT_UCODE_TOTALSIZE)

#define get_datasize(mc) \
	(((struct microcode_intel *)mc)->hdr.datasize ? \
	 ((struct microcode_intel *)mc)->hdr.datasize : DEFAULT_UCODE_DATASIZE)

#define sigmatch(s1, s2, p1, p2) \
	(((s1) == (s2)) && (((p1) & (p2)) || (((p1) == 0) && ((p2) == 0))))

#define exttable_size(et) ((et)->count * EXT_SIGNATURE_SIZE + EXT_HEADER_SIZE)

extern int get_matching_microcode(unsigned int csig, int cpf, int rev, void *mc);
extern int microcode_sanity_check(void *mc, int print_err);
extern int get_matching_sig(unsigned int csig, int cpf, int rev, void *mc);

static inline int
revision_is_newer(struct microcode_header_intel *mc_header, int rev)
{
	return (mc_header->rev <= rev) ? 0 : 1;
}

#ifdef CONFIG_MICROCODE_INTEL_EARLY
extern void __init load_ucode_intel_bsp(void);
extern void load_ucode_intel_ap(void);
extern void show_ucode_info_early(void);
extern int __init save_microcode_in_initrd_intel(void);
void reload_ucode_intel(void);
#else
static inline __init void load_ucode_intel_bsp(void) {}
static inline void load_ucode_intel_ap(void) {}
static inline void show_ucode_info_early(void) {}
static inline int __init save_microcode_in_initrd_intel(void) { return -EINVAL; }
static inline void reload_ucode_intel(void) {}
#endif

#if defined(CONFIG_MICROCODE_INTEL_EARLY) && defined(CONFIG_HOTPLUG_CPU)
extern int save_mc_for_early(u8 *mc);
#else
static inline int save_mc_for_early(u8 *mc)
{
	return 0;
}
#endif

#endif /* _ASM_X86_MICROCODE_INTEL_H */
