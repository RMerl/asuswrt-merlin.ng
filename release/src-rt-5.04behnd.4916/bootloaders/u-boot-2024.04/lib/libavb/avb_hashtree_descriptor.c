// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2016 The Android Open Source Project
 */

#include "avb_hashtree_descriptor.h"
#include "avb_util.h"

bool avb_hashtree_descriptor_validate_and_byteswap(
    const AvbHashtreeDescriptor* src, AvbHashtreeDescriptor* dest) {
  uint64_t expected_size;

  avb_memcpy(dest, src, sizeof(AvbHashtreeDescriptor));

  if (!avb_descriptor_validate_and_byteswap((const AvbDescriptor*)src,
                                            (AvbDescriptor*)dest))
    return false;

  if (dest->parent_descriptor.tag != AVB_DESCRIPTOR_TAG_HASHTREE) {
    avb_error("Invalid tag for hashtree descriptor.\n");
    return false;
  }

  dest->dm_verity_version = avb_be32toh(dest->dm_verity_version);
  dest->image_size = avb_be64toh(dest->image_size);
  dest->tree_offset = avb_be64toh(dest->tree_offset);
  dest->tree_size = avb_be64toh(dest->tree_size);
  dest->data_block_size = avb_be32toh(dest->data_block_size);
  dest->hash_block_size = avb_be32toh(dest->hash_block_size);
  dest->fec_num_roots = avb_be32toh(dest->fec_num_roots);
  dest->fec_offset = avb_be64toh(dest->fec_offset);
  dest->fec_size = avb_be64toh(dest->fec_size);
  dest->partition_name_len = avb_be32toh(dest->partition_name_len);
  dest->salt_len = avb_be32toh(dest->salt_len);
  dest->root_digest_len = avb_be32toh(dest->root_digest_len);
  dest->flags = avb_be32toh(dest->flags);

  /* Check that partition_name, salt, and root_digest are fully contained. */
  expected_size = sizeof(AvbHashtreeDescriptor) - sizeof(AvbDescriptor);
  if (!avb_safe_add_to(&expected_size, dest->partition_name_len) ||
      !avb_safe_add_to(&expected_size, dest->salt_len) ||
      !avb_safe_add_to(&expected_size, dest->root_digest_len)) {
    avb_error("Overflow while adding up sizes.\n");
    return false;
  }
  if (expected_size > dest->parent_descriptor.num_bytes_following) {
    avb_error("Descriptor payload size overflow.\n");
    return false;
  }
  return true;
}
