/* hwf-arm.c - Detect hardware features - ARM part
 * Copyright (C) 2013,2019 Jussi Kivilinna <jussi.kivilinna@iki.fi>
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

#if !defined (__arm__) && !defined (__aarch64__)
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

struct feature_map_s {
  unsigned int hwcap_flag;
  unsigned int hwcap2_flag;
  const char *feature_match;
  unsigned int hwf_flag;
};

#ifdef __arm__

/* Note: These macros have same values on Linux and FreeBSD. */
#ifndef AT_HWCAP
# define AT_HWCAP      16
#endif
#ifndef AT_HWCAP2
# define AT_HWCAP2     26
#endif

#ifndef HWCAP_NEON
# define HWCAP_NEON    4096
#endif

#ifndef HWCAP2_AES
# define HWCAP2_AES    1
#endif
#ifndef HWCAP2_PMULL
# define HWCAP2_PMULL  2
#endif
#ifndef HWCAP2_SHA1
# define HWCAP2_SHA1   4
#endif
#ifndef HWCAP2_SHA2
# define HWCAP2_SHA2   8
#endif

static const struct feature_map_s arm_features[] =
  {
#ifdef ENABLE_NEON_SUPPORT
    { HWCAP_NEON, 0, " neon", HWF_ARM_NEON },
#endif
#ifdef ENABLE_ARM_CRYPTO_SUPPORT
    { 0, HWCAP2_AES, " aes", HWF_ARM_AES },
    { 0, HWCAP2_SHA1," sha1", HWF_ARM_SHA1 },
    { 0, HWCAP2_SHA2, " sha2", HWF_ARM_SHA2 },
    { 0, HWCAP2_PMULL, " pmull", HWF_ARM_PMULL },
#endif
  };

#elif defined(__aarch64__)

/* Note: These macros have same values on Linux and FreeBSD. */
#ifndef AT_HWCAP
# define AT_HWCAP    16
#endif
#ifndef AT_HWCAP2
# define AT_HWCAP2   -1
#endif

#ifndef HWCAP_ASIMD
# define HWCAP_ASIMD 2
#endif
#ifndef HWCAP_AES
# define HWCAP_AES   8
#endif
#ifndef HWCAP_PMULL
# define HWCAP_PMULL 16
#endif
#ifndef HWCAP_SHA1
# define HWCAP_SHA1  32
#endif
#ifndef HWCAP_SHA2
# define HWCAP_SHA2  64
#endif

static const struct feature_map_s arm_features[] =
  {
#ifdef ENABLE_NEON_SUPPORT
    { HWCAP_ASIMD, 0, " asimd", HWF_ARM_NEON },
#endif
#ifdef ENABLE_ARM_CRYPTO_SUPPORT
    { HWCAP_AES, 0, " aes", HWF_ARM_AES },
    { HWCAP_SHA1, 0, " sha1", HWF_ARM_SHA1 },
    { HWCAP_SHA2, 0, " sha2", HWF_ARM_SHA2 },
    { HWCAP_PMULL, 0, " pmull", HWF_ARM_PMULL },
#endif
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
detect_arm_at_hwcap(void)
{
  unsigned int hwcap;
  unsigned int hwcap2;
  unsigned int features = 0;
  unsigned int i;

  if (get_hwcap(&hwcap, &hwcap2) < 0)
    return features;

  for (i = 0; i < DIM(arm_features); i++)
    {
      if (hwcap & arm_features[i].hwcap_flag)
        features |= arm_features[i].hwf_flag;

      if (hwcap2 & arm_features[i].hwcap2_flag)
        features |= arm_features[i].hwf_flag;
    }

  return features;
}

#endif

#undef HAS_PROC_CPUINFO
#ifdef __linux__
#define HAS_PROC_CPUINFO 1

static unsigned int
detect_arm_proc_cpuinfo(unsigned int *broken_hwfs)
{
  char buf[1024]; /* large enough */
  char *str_features, *str_feat;
  int cpu_implementer, cpu_arch, cpu_variant, cpu_part, cpu_revision;
  FILE *f;
  int readlen, i;
  size_t mlen;
  static int cpuinfo_initialized = 0;
  static unsigned int stored_cpuinfo_features;
  static unsigned int stored_broken_hwfs;
  struct {
    const char *name;
    int *value;
  } cpu_entries[5] = {
    { "CPU implementer", &cpu_implementer },
    { "CPU architecture", &cpu_arch },
    { "CPU variant", &cpu_variant },
    { "CPU part", &cpu_part },
    { "CPU revision", &cpu_revision },
  };

  if (cpuinfo_initialized)
    {
      *broken_hwfs |= stored_broken_hwfs;
      return stored_cpuinfo_features;
    }

  f = fopen("/proc/cpuinfo", "r");
  if (!f)
    return 0;

  memset (buf, 0, sizeof(buf));
  readlen = fread (buf, 1, sizeof(buf), f);
  fclose (f);
  if (readlen <= 0 || readlen > sizeof(buf))
    return 0;

  buf[sizeof(buf) - 1] = '\0';

  cpuinfo_initialized = 1;
  stored_cpuinfo_features = 0;
  stored_broken_hwfs = 0;

  /* Find features line. */
  str_features = strstr(buf, "Features");
  if (!str_features)
    return stored_cpuinfo_features;

  /* Find CPU version information. */
  for (i = 0; i < DIM(cpu_entries); i++)
    {
      char *str;

      *cpu_entries[i].value = -1;

      str = strstr(buf, cpu_entries[i].name);
      if (!str)
        continue;

      str = strstr(str, ": ");
      if (!str)
        continue;

      str += 2;
      if (strcmp(cpu_entries[i].name, "CPU architecture") == 0
          && strcmp(str, "AArch64") == 0)
        *cpu_entries[i].value = 8;
      else
        *cpu_entries[i].value = strtoul(str, NULL, 0);
    }

  /* Lines to strings. */
  for (i = 0; i < sizeof(buf); i++)
    if (buf[i] == '\n')
      buf[i] = '\0';

  /* Check features. */
  for (i = 0; i < DIM(arm_features); i++)
    {
      str_feat = strstr(str_features, arm_features[i].feature_match);
      if (str_feat)
        {
          mlen = strlen(arm_features[i].feature_match);
          if (str_feat[mlen] == ' ' || str_feat[mlen] == '\0')
            {
              stored_cpuinfo_features |= arm_features[i].hwf_flag;
            }
        }
    }

  /* Check for CPUs with broken NEON implementation. See
   * https://code.google.com/p/chromium/issues/detail?id=341598
   */
  if (cpu_implementer == 0x51
      && cpu_arch == 7
      && cpu_variant == 1
      && cpu_part == 0x4d
      && cpu_revision == 0)
    {
      stored_broken_hwfs = HWF_ARM_NEON;
    }

  *broken_hwfs |= stored_broken_hwfs;
  return stored_cpuinfo_features;
}

#endif /* __linux__ */

unsigned int
_gcry_hwf_detect_arm (void)
{
  unsigned int ret = 0;
  unsigned int broken_hwfs = 0;

#if defined (HAS_SYS_AT_HWCAP)
  ret |= detect_arm_at_hwcap ();
#endif

#if defined (HAS_PROC_CPUINFO)
  ret |= detect_arm_proc_cpuinfo (&broken_hwfs);
#endif

#if defined(__ARM_NEON) && defined(ENABLE_NEON_SUPPORT)
  ret |= HWF_ARM_NEON;
#endif

  ret &= ~broken_hwfs;

  return ret;
}
