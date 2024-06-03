// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2016 The Android Open Source Project
 */

#include "avb_chain_partition_descriptor.h"
#include "avb_util.h"

bool avb_chain_partition_descriptor_validate_and_byteswap(
    const AvbChainPartitionDescriptor* src, AvbChainPartitionDescriptor* dest) {
  uint64_t expected_size;

  avb_memcpy(dest, src, sizeof(AvbChainPartitionDescriptor));

  if (!avb_descriptor_validate_and_byteswap((const AvbDescriptor*)src,
                                            (AvbDescriptor*)dest))
    return false;

  if (dest->parent_descriptor.tag != AVB_DESCRIPTOR_TAG_CHAIN_PARTITION) {
    avb_error("Invalid tag for chain partition descriptor.\n");
    return false;
  }

  dest->rollback_index_location = avb_be32toh(dest->rollback_index_location);
  dest->partition_name_len = avb_be32toh(dest->partition_name_len);
  dest->public_key_len = avb_be32toh(dest->public_key_len);

  if (dest->rollback_index_location < 1) {
    avb_error("Invalid rollback index location value.\n");
    return false;
  }

  /* Check that partition_name and public_key are fully contained. */
  expected_size = sizeof(AvbChainPartitionDescriptor) - sizeof(AvbDescriptor);
  if (!avb_safe_add_to(&expected_size, dest->partition_name_len) ||
      !avb_safe_add_to(&expected_size, dest->public_key_len)) {
    avb_error("Overflow while adding up sizes.\n");
    return false;
  }
  if (expected_size > dest->parent_descriptor.num_bytes_following) {
    avb_error("Descriptor payload size overflow.\n");
    return false;
  }
  return true;
}
