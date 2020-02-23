#ifndef _BCM_PREFETCH_H
#define _BCM_PREFETCH_H

#if (defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148) || defined(CONFIG_BCM96846) || defined(CONFIG_BCM947189) || defined(CONFIG_BCM963178) || defined(CONFIG_BCM947622) || defined(CONFIG_BCM96878))
static inline void _bcm_prefetch(const void * addr)
{
	__asm__ __volatile__("pld\t%a0" : : "p"(addr) : "cc");
}
static inline void bcm_prefetch_ld(const void * addr) { _bcm_prefetch(addr); }
static inline void bcm_prefetch_st(const void * addr) { _bcm_prefetch(addr); }
static inline void bcm_prefetch(const void * addr)    { _bcm_prefetch(addr); }
static inline void bcm_prefetch_multiple(const void * addr, const int cachelines)
{
	switch (cachelines) {
	default:
	case 4:
		__asm__ __volatile__("pld\t%a0" : : "p"(addr + (L1_CACHE_BYTES * 3)) : "cc");
	case 3:
		__asm__ __volatile__("pld\t%a0" : : "p"(addr + (L1_CACHE_BYTES * 2)) : "cc");
	case 2:
		__asm__ __volatile__("pld\t%a0" : : "p"(addr + (L1_CACHE_BYTES * 1)) : "cc");
	case 1:
		__asm__ __volatile__("pld\t%a0" : : "p"(addr + (L1_CACHE_BYTES * 0)) : "cc");
	}
	return;
}
#elif (defined(CONFIG_BCM94908) || defined(CONFIG_BCM963158) || defined(CONFIG_BCM96858) || defined(CONFIG_BCM96856))
static inline void bcm_prefetch_ld(const void * addr)
{
	__asm__ __volatile__("prfm pldl1keep, %a0" : : "p"(addr) : "cc");
}
static inline void bcm_prefetch_st(const void * addr)
{
	__asm__ __volatile__("prfm pstl1keep, %a0" : : "p"(addr) : "cc");
}
static inline void bcm_prefetch(const void * addr) { bcm_prefetch_ld(addr); }
static inline void bcm_prefetch_multiple(const void * addr, const int cachelines)
{
	switch (cachelines) {
	default:
	case 4:
		__asm__ __volatile__("prfm pldl1keep, %a0" : : "p"(addr + (L1_CACHE_BYTES * 3)) : "cc");
	case 3:
		__asm__ __volatile__("prfm pldl1keep, %a0" : : "p"(addr + (L1_CACHE_BYTES * 2)) : "cc");
	case 2:
		__asm__ __volatile__("prfm pldl1keep, %a0" : : "p"(addr + (L1_CACHE_BYTES * 1)) : "cc");
	case 1:
		__asm__ __volatile__("prfm pldl1keep, %a0" : : "p"(addr + (L1_CACHE_BYTES * 0)) : "cc");
	}
	return;
}

#elif defined(CONFIG_MIPS_BCM963XX)
static inline void _bcm_prefetch(const void * addr)
{
    __asm__ __volatile__("pref %0, (%1)" :: "i"(0), "r"(addr));
}
static inline void bcm_prefetch_ld(const void * addr) { _bcm_prefetch(addr); }
static inline void bcm_prefetch_st(const void * addr) { _bcm_prefetch(addr); }
static inline void bcm_prefetch(const void * addr)    { _bcm_prefetch(addr); }
#else
static inline void bcm_prefetch_ld(const void * addr) { }
static inline void bcm_prefetch_st(const void * addr) { }
static inline void bcm_prefetch(const void * addr) { }
static inline void bcm_prefetch_multiple(const void * addr, const int cachelines) { }
#endif
#endif /* _BCM_PREFETCH_H */
