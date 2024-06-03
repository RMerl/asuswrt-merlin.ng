/* SPDX-License-Identifier: MIT */
/*
 * Copyright (C) 2017 The Android Open Source Project
 */

#if !defined(AVB_INSIDE_LIBAVB_H) && !defined(AVB_COMPILATION)
#error "Never include this file directly, include libavb.h instead."
#endif

#ifndef AVB_VERSION_H_
#define AVB_VERSION_H_

#include "avb_sysdeps.h"

#ifdef __cplusplus
extern "C" {
#endif

/* The version number of AVB - keep in sync with avbtool. */
#define AVB_VERSION_MAJOR 1
#define AVB_VERSION_MINOR 1
#define AVB_VERSION_SUB 0

/* Returns a NUL-terminated string for the libavb version in use.  The
 * returned string usually looks like "%d.%d.%d". Applications must
 * not make assumptions about the content of this string.
 *
 * Boot loaders should display this string in debug/diagnostics output
 * to aid with debugging.
 *
 * This is similar to the string put in the |release_string| string
 * field in the VBMeta struct by avbtool.
 */
const char* avb_version_string(void);

#ifdef __cplusplus
}
#endif

#endif /* AVB_VERSION_H_ */
