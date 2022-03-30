// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2016 The Android Open Source Project
 */

#include "avb_kernel_cmdline_descriptor.h"
#include "avb_util.h"

bool avb_kernel_cmdline_descriptor_validate_and_byteswap(
    const AvbKernelCmdlineDescriptor* src, AvbKernelCmdlineDescriptor* dest) {
  uint64_t expected_size;

  avb_memcpy(dest, src, sizeof(AvbKernelCmdlineDescriptor));

  if (!avb_descriptor_validate_and_byteswap((const AvbDescriptor*)src,
                                            (AvbDescriptor*)dest))
    return false;

  if (dest->parent_descriptor.tag != AVB_DESCRIPTOR_TAG_KERNEL_CMDLINE) {
    avb_error("Invalid tag for kernel cmdline descriptor.\n");
    return false;
  }

  dest->flags = avb_be32toh(dest->flags);
  dest->kernel_cmdline_length = avb_be32toh(dest->kernel_cmdline_length);

  /* Check that kernel_cmdline is fully contained. */
  expected_size = sizeof(AvbKernelCmdlineDescriptor) - sizeof(AvbDescriptor);
  if (!avb_safe_add_to(&expected_size, dest->kernel_cmdline_length)) {
    avb_error("Overflow while adding up sizes.\n");
    return false;
  }
  if (expected_size > dest->parent_descriptor.num_bytes_following) {
    avb_error("Descriptor payload size overflow.\n");
    return false;
  }

  return true;
}
