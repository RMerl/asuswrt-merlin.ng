// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2016 The Android Open Source Project
 */

#include "avb_hash_descriptor.h"
#include "avb_util.h"

bool avb_hash_descriptor_validate_and_byteswap(const AvbHashDescriptor* src,
                                               AvbHashDescriptor* dest) {
  uint64_t expected_size;

  avb_memcpy(dest, src, sizeof(AvbHashDescriptor));

  if (!avb_descriptor_validate_and_byteswap((const AvbDescriptor*)src,
                                            (AvbDescriptor*)dest))
    return false;

  if (dest->parent_descriptor.tag != AVB_DESCRIPTOR_TAG_HASH) {
    avb_error("Invalid tag for hash descriptor.\n");
    return false;
  }

  dest->image_size = avb_be64toh(dest->image_size);
  dest->partition_name_len = avb_be32toh(dest->partition_name_len);
  dest->salt_len = avb_be32toh(dest->salt_len);
  dest->digest_len = avb_be32toh(dest->digest_len);
  dest->flags = avb_be32toh(dest->flags);

  /* Check that partition_name, salt, and digest are fully contained. */
  expected_size = sizeof(AvbHashDescriptor) - sizeof(AvbDescriptor);
  if (!avb_safe_add_to(&expected_size, dest->partition_name_len) ||
      !avb_safe_add_to(&expected_size, dest->salt_len) ||
      !avb_safe_add_to(&expected_size, dest->digest_len)) {
    avb_error("Overflow while adding up sizes.\n");
    return false;
  }
  if (expected_size > dest->parent_descriptor.num_bytes_following) {
    avb_error("Descriptor payload size overflow.\n");
    return false;
  }
  return true;
}
