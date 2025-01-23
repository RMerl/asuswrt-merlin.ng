/* hwf-x86.c - Detect hardware features - x86 part
 * Copyright (C) 2007, 2011, 2012  Free Software Foundation, Inc.
 * Copyright (C) 2012  Jussi Kivilinna
 *
 * This file is part of Libgcrypt.
 *
 * Libgcrypt is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * Libgcrypt is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>

#include "g10lib.h"
#include "hwf-common.h"

#if !defined (__i386__) && !defined (__x86_64__)
# error Module build for wrong CPU.
#endif

/* We use the next macro to decide whether we can test for certain
   features.  */
#undef HAS_X86_CPUID

#if defined (__i386__) && SIZEOF_UNSIGNED_LONG == 4 && defined (__GNUC__)
# define HAS_X86_CPUID 1

#if _GCRY_GCC_VERSION >= 40700 /* 4.7 */
# define FORCE_FUNC_FRAME_POINTER \
	__attribute__ ((optimize("no-omit-frame-pointer")))
#else
# define FORCE_FUNC_FRAME_POINTER
#endif

static FORCE_FUNC_FRAME_POINTER int
is_cpuid_available(void)
{
  int has_cpuid = 0;

  /* Detect the CPUID feature by testing some undefined behaviour (16
     vs 32 bit pushf/popf). */
  asm volatile
    ("pushf\n\t"                 /* Copy flags to EAX.  */
     "popl %%eax\n\t"
     "movl %%eax, %%ecx\n\t"     /* Save flags into ECX.  */
     "xorl $0x200000, %%eax\n\t" /* Toggle ID bit and copy it to the flags.  */
     "pushl %%eax\n\t"
     "popf\n\t"
     "pushf\n\t"                 /* Copy changed flags again to EAX.  */
     "popl %%eax\n\t"
     "pushl %%ecx\n\t"           /* Restore flags from ECX.  */
     "popf\n\t"
     "xorl %%eax, %%ecx\n\t"     /* Compare flags against saved flags.  */
     "jz .Lno_cpuid%=\n\t"       /* Toggling did not work, thus no CPUID.  */
     "movl $1, %0\n"             /* Worked. true -> HAS_CPUID.  */
     ".Lno_cpuid%=:\n\t"
     : "+r" (has_cpuid)
     :
     : "%eax", "%ecx", "cc", "memory"
     );

  return has_cpuid;
}

static void
get_cpuid(unsigned int in, unsigned int *eax, unsigned int *ebx,
          unsigned int *ecx, unsigned int *edx)
{
  unsigned int regs[4];

  asm volatile
    ("xchgl %%ebx, %1\n\t"     /* Save GOT register.  */
     "cpuid\n\t"
     "xchgl %%ebx, %1\n\t"     /* Restore GOT register. */
     : "=a" (regs[0]), "=D" (regs[1]), "=c" (regs[2]), "=d" (regs[3])
     : "0" (in), "1" (0), "2" (0), "3" (0)
     : "cc"
     );

  if (eax)
    *eax = regs[0];
  if (ebx)
    *ebx = regs[1];
  if (ecx)
    *ecx = regs[2];
  if (edx)
    *edx = regs[3];
}

#if defined(ENABLE_AVX_SUPPORT) || defined(ENABLE_AVX2_SUPPORT)
static unsigned int
get_xgetbv(void)
{
  unsigned int t_eax, t_edx;

  asm volatile
    ("xgetbv\n\t"
     : "=a" (t_eax), "=d" (t_edx)
     : "c" (0)
    );

  return t_eax;
}
#endif /* ENABLE_AVX_SUPPORT || ENABLE_AVX2_SUPPORT */

#endif /* i386 && GNUC */


#if defined (__x86_64__) && defined (__GNUC__)
# define HAS_X86_CPUID 1

static int
is_cpuid_available(void)
{
  return 1;
}

static void
get_cpuid(unsigned int in, unsigned int *eax, unsigned int *ebx,
          unsigned int *ecx, unsigned int *edx)
{
  unsigned int regs[4];

  asm volatile
    ("cpuid\n\t"
     : "=a" (regs[0]), "=b" (regs[1]), "=c" (regs[2]), "=d" (regs[3])
     : "0" (in), "1" (0), "2" (0), "3" (0)
     : "cc"
     );

  if (eax)
    *eax = regs[0];
  if (ebx)
    *ebx = regs[1];
  if (ecx)
    *ecx = regs[2];
  if (edx)
    *edx = regs[3];
}

#if defined(ENABLE_AVX_SUPPORT) || defined(ENABLE_AVX2_SUPPORT)
static unsigned int
get_xgetbv(void)
{
  unsigned int t_eax, t_edx;

  asm volatile
    ("xgetbv\n\t"
     : "=a" (t_eax), "=d" (t_edx)
     : "c" (0)
    );

  return t_eax;
}
#endif /* ENABLE_AVX_SUPPORT || ENABLE_AVX2_SUPPORT */

#endif /* x86-64 && GNUC */


#ifdef HAS_X86_CPUID
static unsigned int
detect_x86_gnuc (void)
{
  union
  {
    char c[12+1];
    unsigned int ui[3];
  } vendor_id;
  unsigned int features, features2;
  unsigned int os_supports_avx_avx2_registers = 0;
  unsigned int max_cpuid_level;
  unsigned int fms, family, model;
  unsigned int result = 0;
  unsigned int avoid_vpgather = 0;

  (void)os_supports_avx_avx2_registers;

  if (!is_cpuid_available())
    return 0;

  get_cpuid(0, &max_cpuid_level, &vendor_id.ui[0], &vendor_id.ui[2],
            &vendor_id.ui[1]);
  vendor_id.c[12] = 0;

  if (0)
    ; /* Just to make "else if" and ifdef macros look pretty.  */
#ifdef ENABLE_PADLOCK_SUPPORT
  else if (!strcmp (vendor_id.c, "CentaurHauls"))
    {
      /* This is a VIA CPU.  Check what PadLock features we have.  */

      /* Check for extended centaur (EAX).  */
      get_cpuid(0xC0000000, &features, NULL, NULL, NULL);

      /* Has extended centaur features? */
      if (features > 0xC0000000)
        {
           /* Ask for the extended feature flags (EDX). */
           get_cpuid(0xC0000001, NULL, NULL, NULL, &features);

           /* Test bits 2 and 3 to see whether the RNG exists and is enabled. */
           if ((features & 0x0C) == 0x0C)
             result |= HWF_PADLOCK_RNG;

           /* Test bits 6 and 7 to see whether the ACE exists and is enabled. */
           if ((features & 0xC0) == 0xC0)
             result |= HWF_PADLOCK_AES;

           /* Test bits 10 and 11 to see whether the PHE exists and is
              enabled.  */
           if ((features & 0xC00) == 0xC00)
             result |= HWF_PADLOCK_SHA;

           /* Test bits 12 and 13 to see whether the MONTMUL exists and is
              enabled.  */
           if ((features & 0x3000) == 0x3000)
             result |= HWF_PADLOCK_MMUL;
        }
    }
#endif /*ENABLE_PADLOCK_SUPPORT*/
  else if (!strcmp (vendor_id.c, "GenuineIntel"))
    {
      /* This is an Intel CPU.  */
      result |= HWF_INTEL_CPU;
    }
  else if (!strcmp (vendor_id.c, "AuthenticAMD"))
    {
      /* This is an AMD CPU.  */
    }

  /* Detect Intel features, that might also be supported by other
     vendors.  */

  /* Get CPU family/model/stepping (EAX) and Intel feature flags (ECX, EDX).  */
  get_cpuid(1, &fms, NULL, &features, &features2);

  family = ((fms & 0xf00) >> 8) + ((fms & 0xff00000) >> 20);
  model = ((fms & 0xf0) >> 4) + ((fms & 0xf0000) >> 12);

  if ((result & HWF_INTEL_CPU) && family == 6)
    {
      /* These Intel Core processor models have SHLD/SHRD instruction that
       * can do integer rotation faster actual ROL/ROR instructions. */
      switch (model)
	{
	case 0x2A:
	case 0x2D:
	case 0x3A:
	case 0x3C:
	case 0x3F:
	case 0x45:
	case 0x46:
	case 0x3D:
	case 0x4F:
	case 0x56:
	case 0x47:
	case 0x4E:
	case 0x5E:
	case 0x8E:
	case 0x9E:
	case 0x55:
	case 0x66:
	  result |= HWF_INTEL_FAST_SHLD;
	  break;
	}

      /* These Intel Core processors that have AVX2 have slow VPGATHER and
       * should be avoided for table-lookup use. */
      switch (model)
	{
	case 0x3C:
	case 0x3F:
	case 0x45:
	case 0x46:
	  /* Haswell */
	  avoid_vpgather |= 1;
	  break;
	}
    }
  else
    {
      /* Avoid VPGATHER for non-Intel CPUs as testing is needed to
       * make sure it is fast enough. */

      avoid_vpgather |= 1;
    }

#ifdef ENABLE_FORCE_SOFT_HWFEATURES
  /* Soft HW features mark functionality that is available on all systems
   * but not feasible to use because of slow HW implementation. */

  /* SHLD is faster at rotating register than actual ROR/ROL instructions
   * on older Intel systems (~sandy-bridge era). However, SHLD is very
   * slow on almost anything else and later Intel processors have faster
   * ROR/ROL. Therefore in regular build HWF_INTEL_FAST_SHLD is enabled
   * only for those Intel processors that benefit from the SHLD
   * instruction. Enabled here unconditionally as requested. */
  result |= HWF_INTEL_FAST_SHLD;

  /* VPGATHER instructions are used for look-up table based
   * implementations which require VPGATHER to be fast enough to beat
   * regular parallelized look-up table implementations (see Twofish).
   * So far, only Intel processors beginning with skylake have had
   * VPGATHER fast enough to be enabled. AMD Zen3 comes close to
   * being feasible, but not quite (where twofish-avx2 is few percent
   * slower than twofish-3way). Enable VPGATHER here unconditionally
   * as requested. */
  avoid_vpgather = 0;
#endif

#ifdef ENABLE_PCLMUL_SUPPORT
  /* Test bit 1 for PCLMUL.  */
  if (features & 0x00000002)
     result |= HWF_INTEL_PCLMUL;
#endif
  /* Test bit 9 for SSSE3.  */
  if (features & 0x00000200)
     result |= HWF_INTEL_SSSE3;
  /* Test bit 19 for SSE4.1.  */
  if (features & 0x00080000)
     result |= HWF_INTEL_SSE4_1;
#ifdef ENABLE_AESNI_SUPPORT
  /* Test bit 25 for AES-NI.  */
  if (features & 0x02000000)
     result |= HWF_INTEL_AESNI;
#endif /*ENABLE_AESNI_SUPPORT*/
#if defined(ENABLE_AVX_SUPPORT) || defined(ENABLE_AVX2_SUPPORT)
  /* Test bit 27 for OSXSAVE (required for AVX/AVX2).  */
  if (features & 0x08000000)
    {
      /* Check that OS has enabled both XMM and YMM state support.  */
      if ((get_xgetbv() & 0x6) == 0x6)
        os_supports_avx_avx2_registers = 1;
    }
#endif
#ifdef ENABLE_AVX_SUPPORT
  /* Test bit 28 for AVX.  */
  if (features & 0x10000000)
    if (os_supports_avx_avx2_registers)
      result |= HWF_INTEL_AVX;
#endif /*ENABLE_AVX_SUPPORT*/
#ifdef ENABLE_DRNG_SUPPORT
  /* Test bit 30 for RDRAND.  */
  if (features & 0x40000000)
     result |= HWF_INTEL_RDRAND;
#endif /*ENABLE_DRNG_SUPPORT*/

  /* Test bit 4 of EDX for TSC.  */
  if (features2 & 0x00000010)
    result |= HWF_INTEL_RDTSC;

  /* Check additional Intel feature flags.  Early Intel P5 processors report
   * too high max_cpuid_level, so don't check level 7 if processor does not
   * support SSE3 (as cpuid:7 contains only features for newer processors).
   * Source: http://www.sandpile.org/x86/cpuid.htm  */
  if (max_cpuid_level >= 7 && (features & 0x00000001))
    {
      /* Get CPUID:7 contains further Intel feature flags. */
      get_cpuid(7, NULL, &features, &features2, NULL);

      /* Test bit 8 for BMI2.  */
      if (features & 0x00000100)
          result |= HWF_INTEL_BMI2;

#ifdef ENABLE_AVX2_SUPPORT
      /* Test bit 5 for AVX2.  */
      if (features & 0x00000020)
        if (os_supports_avx_avx2_registers)
          result |= HWF_INTEL_AVX2;

      if ((result & HWF_INTEL_AVX2) && !avoid_vpgather)
        result |= HWF_INTEL_FAST_VPGATHER;
#endif /*ENABLE_AVX_SUPPORT*/

      /* Test bit 29 for SHA Extensions. */
      if (features & (1 << 29))
        result |= HWF_INTEL_SHAEXT;

#if defined(ENABLE_AVX2_SUPPORT) && defined(ENABLE_AESNI_SUPPORT) && \
    defined(ENABLE_PCLMUL_SUPPORT)
      /* Test bit 9 for VAES and bit 10 for VPCLMULDQD */
      if ((features2 & 0x00000200) && (features2 & 0x00000400))
        result |= HWF_INTEL_VAES_VPCLMUL;
#endif
    }

  return result;
}
#endif /* HAS_X86_CPUID */


unsigned int
_gcry_hwf_detect_x86 (void)
{
#if defined (HAS_X86_CPUID)
  return detect_x86_gnuc ();
#else
  return 0;
#endif
}
