/* hwf-s390x.c - Detect hardware features - s390x/zSeries part
 * Copyright (C) 2020 Jussi Kivilinna <jussi.kivilinna@iki.fi>
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

#if !defined (__s390x__)
# error Module build for wrong CPU.
#endif

#undef HAVE_STFLE
#ifdef HAVE_GCC_INLINE_ASM_S390X
# define HAVE_STFLE 1
#endif

#ifndef AT_HWCAP
# define AT_HWCAP         16
#endif
#ifndef HWCAP_S390_STFLE
# define HWCAP_S390_STFLE 4
#endif
#ifndef HWCAP_S390_VXRS
# define HWCAP_S390_VXRS  2048
#endif

struct feature_map_s
  {
    unsigned int facilities_bit;
    unsigned int hwcap_flag;
    unsigned int hwf_flag;
  };

static const struct feature_map_s s390x_features[] =
  {
    { 17,  0, HWF_S390X_MSA },
    { 77,  0, HWF_S390X_MSA_4 },
    { 146, 0, HWF_S390X_MSA_8 },
    { 155, 0, HWF_S390X_MSA_9 },
#ifdef HAVE_GCC_INLINE_ASM_S390X_VX
    { 129, HWCAP_S390_VXRS, HWF_S390X_VX },
#endif
  };

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

struct facilities_s
  {
    u64 bits[3];
  };

static int
get_hwcap(unsigned int *hwcap)
{
  struct { unsigned long a_type; unsigned long a_val; } auxv;
  FILE *f;
  int err = -1;
  static int hwcap_initialized = 0;
  static unsigned int stored_hwcap = 0;

  if (hwcap_initialized)
    {
      *hwcap = stored_hwcap;
      return 0;
    }

#if defined(HAVE_SYS_AUXV_H) && defined(HAVE_GETAUXVAL)
  errno = 0;
  auxv.a_val = getauxval (AT_HWCAP);
  if (errno == 0)
    {
      stored_hwcap |= auxv.a_val;
      hwcap_initialized = 1;
    }

  if (hwcap_initialized && stored_hwcap)
    {
      *hwcap = stored_hwcap;
      return 0;
    }
#endif

  f = fopen("/proc/self/auxv", "r");
  if (!f)
    {
      *hwcap = stored_hwcap;
      return -1;
    }

  while (fread(&auxv, sizeof(auxv), 1, f) > 0)
    {
      if (auxv.a_type == AT_HWCAP)
        {
          stored_hwcap |= auxv.a_val;
          hwcap_initialized = 1;
        }
    }

  if (hwcap_initialized)
      err = 0;

  fclose(f);

  *hwcap = stored_hwcap;
  return err;
}
#endif

#ifdef HAVE_STFLE
static void
get_stfle(struct facilities_s *out)
{
  static int stfle_initialized = 0;
  static struct facilities_s stored_facilities;

  if (!stfle_initialized)
    {
      register unsigned long reg0 asm("0") = DIM(stored_facilities.bits) - 1;

      asm ("stfle %1\n\t"
	   : "+d" (reg0),
	     "=Q" (stored_facilities.bits[0])
	   :
	   : "cc", "memory");

      stfle_initialized = 1;
    }

  *out = stored_facilities;
}
#endif

static unsigned int
detect_s390x_features(void)
{
  struct facilities_s facilities = { { 0, } };
  unsigned int hwcap = 0;
  unsigned int features = 0;
  unsigned int i;

#if defined (HAS_SYS_AT_HWCAP)
  if (get_hwcap(&hwcap) < 0)
    return features;
#endif

  if ((hwcap & HWCAP_S390_STFLE) == 0)
    return features;

#ifdef HAVE_STFLE
  get_stfle(&facilities);
#endif

  for (i = 0; i < DIM(s390x_features); i++)
    {
      if (s390x_features[i].hwcap_flag == 0 ||
	  (s390x_features[i].hwcap_flag & hwcap))
	{
	  unsigned int idx = s390x_features[i].facilities_bit;
	  unsigned int u64_idx = idx / 64;
	  unsigned int u64_bit = 63 - (idx % 64);

	  if (facilities.bits[u64_idx] & (U64_C(1) << u64_bit))
	    features |= s390x_features[i].hwf_flag;
	}
    }

  return features;
}

unsigned int
_gcry_hwf_detect_s390x (void)
{
  unsigned int ret = 0;

  ret |= detect_s390x_features ();

  return ret;
}
