/* SPDX-License-Identifier: MIT */
/*
 * Copyright (C) 2016 The Android Open Source Project
 */

#if !defined(AVB_INSIDE_LIBAVB_H) && !defined(AVB_COMPILATION)
#error "Never include this file directly, include libavb.h instead."
#endif

#ifndef AVB_KERNEL_CMDLINE_DESCRIPTOR_H_
#define AVB_KERNEL_CMDLINE_DESCRIPTOR_H_

#include "avb_descriptor.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Flags for kernel command-line descriptors.
 *
 * AVB_KERNEL_CMDLINE_FLAGS_USE_ONLY_IF_HASHTREE_NOT_DISABLED: The
 * cmdline will only be applied if hashtree verification is not
 * disabled (cf. AVB_VBMETA_IMAGE_FLAGS_HASHTREE_DISABLED).
 *
 * AVB_KERNEL_CMDLINE_FLAGS_USE_ONLY_IF_HASHTREE_DISABLED: The cmdline
 * will only be applied if hashtree verification is disabled
 * (cf. AVB_VBMETA_IMAGE_FLAGS_HASHTREE_DISABLED).
 */
typedef enum {
  AVB_KERNEL_CMDLINE_FLAGS_USE_ONLY_IF_HASHTREE_NOT_DISABLED = (1 << 0),
  AVB_KERNEL_CMDLINE_FLAGS_USE_ONLY_IF_HASHTREE_DISABLED = (1 << 1)
} AvbKernelCmdlineFlags;

/* A descriptor containing information to be appended to the kernel
 * command-line.
 *
 * The |flags| field contains flags from the AvbKernelCmdlineFlags
 * enumeration.
 *
 * Following this struct are |kernel_cmdline_len| bytes with the
 * kernel command-line (UTF-8 encoded).
 */
typedef struct AvbKernelCmdlineDescriptor {
  AvbDescriptor parent_descriptor;
  uint32_t flags;
  uint32_t kernel_cmdline_length;
} AVB_ATTR_PACKED AvbKernelCmdlineDescriptor;

/* Copies |src| to |dest| and validates, byte-swapping fields in the
 * process if needed. Returns true if valid, false if invalid.
 *
 * Data following the struct is not validated nor copied.
 */
bool avb_kernel_cmdline_descriptor_validate_and_byteswap(
    const AvbKernelCmdlineDescriptor* src,
    AvbKernelCmdlineDescriptor* dest) AVB_ATTR_WARN_UNUSED_RESULT;

#ifdef __cplusplus
}
#endif

#endif /* AVB_KERNEL_CMDLINE_DESCRIPTOR_H_ */
