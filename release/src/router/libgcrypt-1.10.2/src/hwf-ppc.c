/* hwf-ppc.c - Detect hardware features - PPC part
 * Copyright (C) 2013,2019 Jussi Kivilinna <jussi.kivilinna@iki.fi>
 * Copyright (C) 2019 Shawn Landden <shawn@git.icu>
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
#include <errno.h>
#if defined(HAVE_SYS_AUXV_H) && (defined(HAVE_GETAUXVAL) || \
    defined(HAVE_ELF_AUX_INFO))
#include <sys/auxv.h>
#endif

#include "g10lib.h"
#include "hwf-common.h"

#if !defined (__powerpc__) && !defined (__powerpc64__)
# error Module build for wrong CPU.
#endif


#if defined(HAVE_SYS_AUXV_H) && defined(HAVE_ELF_AUX_INFO) && \
    !defined(HAVE_GETAUXVAL) && defined(AT_HWCAP)
#define HAVE_GETAUXVAL
static unsigned long getauxval(unsigned long type)
{
  unsigned long auxval = 0;
  int err;

  /* FreeBSD provides 'elf_aux_info' function that does the same as
   * 'getauxval' on Linux. */

  err = elf_aux_info (type, &auxval, sizeof(auxval));
  if (err)
    {
      errno = err;
      auxval = 0;
    }

  return auxval;
}
#endif


#undef HAS_SYS_AT_HWCAP
#if defined(__linux__) || \
    (defined(HAVE_SYS_AUXV_H) && defined(HAVE_GETAUXVAL))
#define HAS_SYS_AT_HWCAP 1

struct feature_map_s
  {
    unsigned int hwcap_flag;
    unsigned int hwcap2_flag;
    unsigned int hwf_flag;
  };

#if defined(__powerpc__) || defined(__powerpc64__)

/* Note: These macros have same values on Linux and FreeBSD. */
#ifndef AT_HWCAP
# define AT_HWCAP      16
#endif
#ifndef AT_HWCAP2
# define AT_HWCAP2     26
#endif

#ifndef PPC_FEATURE2_ARCH_2_07
# define PPC_FEATURE2_ARCH_2_07     0x80000000
#endif
#ifndef PPC_FEATURE2_VEC_CRYPTO
# define PPC_FEATURE2_VEC_CRYPTO    0x02000000
#endif
#ifndef PPC_FEATURE2_ARCH_3_00
# define PPC_FEATURE2_ARCH_3_00     0x00800000
#endif
#ifndef PPC_FEATURE2_ARCH_3_10
# define PPC_FEATURE2_ARCH_3_10     0x00040000
#endif

static const struct feature_map_s ppc_features[] =
  {
    { 0, PPC_FEATURE2_ARCH_2_07, HWF_PPC_ARCH_2_07 },
#ifdef ENABLE_PPC_CRYPTO_SUPPORT
    { 0, PPC_FEATURE2_VEC_CRYPTO, HWF_PPC_VCRYPTO },
#endif
    { 0, PPC_FEATURE2_ARCH_3_00, HWF_PPC_ARCH_3_00 },
    { 0, PPC_FEATURE2_ARCH_3_10, HWF_PPC_ARCH_3_10 },
  };
#endif

static int
get_hwcap(unsigned int *hwcap, unsigned int *hwcap2)
{
  struct { unsigned long a_type; unsigned long a_val; } auxv;
  FILE *f;
  int err = -1;
  static int hwcap_initialized = 0;
  static unsigned int stored_hwcap = 0;
  static unsigned int stored_hwcap2 = 0;

  if (hwcap_initialized)
    {
      *hwcap = stored_hwcap;
      *hwcap2 = stored_hwcap2;
      return 0;
    }

#if 0 // TODO: configure.ac detection for __builtin_cpu_supports
      // TODO: move to 'detect_ppc_builtin_cpu_supports'
#if defined(__GLIBC__) && defined(__GNUC__) && __GNUC__ >= 6
  /* __builtin_cpu_supports returns 0 if glibc support doesn't exist, so
   * we can only trust positive results. */
#ifdef ENABLE_PPC_CRYPTO_SUPPORT
  if (__builtin_cpu_supports("vcrypto")) /* TODO: Configure.ac */
    {
      stored_hwcap2 |= PPC_FEATURE2_VEC_CRYPTO;
      hwcap_initialized = 1;
    }
#endif

  if (__builtin_cpu_supports("arch_3_00")) /* TODO: Configure.ac */
    {
      stored_hwcap2 |= PPC_FEATURE2_ARCH_3_00;
      hwcap_initialized = 1;
    }
#endif
#endif

#if defined(HAVE_SYS_AUXV_H) && defined(HAVE_GETAUXVAL)
  errno = 0;
  auxv.a_val = getauxval (AT_HWCAP);
  if (errno == 0)
    {
      stored_hwcap |= auxv.a_val;
      hwcap_initialized = 1;
    }

  if (AT_HWCAP2 >= 0)
    {
      errno = 0;
      auxv.a_val = getauxval (AT_HWCAP2);
      if (errno == 0)
	{
	  stored_hwcap2 |= auxv.a_val;
	  hwcap_initialized = 1;
	}
    }

  if (hwcap_initialized && (stored_hwcap || stored_hwcap2))
    {
      *hwcap = stored_hwcap;
      *hwcap2 = stored_hwcap2;
      return 0;
    }
#endif

  f = fopen("/proc/self/auxv", "r");
  if (!f)
    {
      *hwcap = stored_hwcap;
      *hwcap2 = stored_hwcap2;
      return -1;
    }

  while (fread(&auxv, sizeof(auxv), 1, f) > 0)
    {
      if (auxv.a_type == AT_HWCAP)
        {
          stored_hwcap |= auxv.a_val;
          hwcap_initialized = 1;
        }

      if (auxv.a_type == AT_HWCAP2)
        {
          stored_hwcap2 |= auxv.a_val;
          hwcap_initialized = 1;
        }
    }

  if (hwcap_initialized)
      err = 0;

  fclose(f);

  *hwcap = stored_hwcap;
  *hwcap2 = stored_hwcap2;
  return err;
}

static unsigned int
detect_ppc_at_hwcap(void)
{
  unsigned int hwcap;
  unsigned int hwcap2;
  unsigned int features = 0;
  unsigned int i;

  if (get_hwcap(&hwcap, &hwcap2) < 0)
      return features;

  for (i = 0; i < DIM(ppc_features); i++)
    {
      if (hwcap & ppc_features[i].hwcap_flag)
        features |= ppc_features[i].hwf_flag;

      if (hwcap2 & ppc_features[i].hwcap2_flag)
        features |= ppc_features[i].hwf_flag;
    }

  return features;
}

#endif

unsigned int
_gcry_hwf_detect_ppc (void)
{
  unsigned int ret = 0;
  unsigned int broken_hwfs = 0;

#if defined (HAS_SYS_AT_HWCAP)
  ret |= detect_ppc_at_hwcap ();
#endif

  ret &= ~broken_hwfs;

  return ret;
}
