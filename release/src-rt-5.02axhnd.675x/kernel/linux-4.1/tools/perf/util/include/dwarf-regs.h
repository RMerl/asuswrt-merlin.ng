#ifndef _PERF_DWARF_REGS_H_
#define _PERF_DWARF_REGS_H_

#if defined(CONFIG_BCM_KF_MIPS_4350) && (defined(CONFIG_BCM96838) || defined(CONFIG_BCM96848))
const char *get_arch_regstr(unsigned int n);
#endif

#ifdef HAVE_DWARF_SUPPORT
const char *get_arch_regstr(unsigned int n);
#endif
#endif

