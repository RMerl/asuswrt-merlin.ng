/* SPDX-License-Identifier: MIT */
/*
 * Copyright (C) 2016 The Android Open Source Project
 */

#if !defined(AVB_INSIDE_LIBAVB_H) && !defined(AVB_COMPILATION)
#error "Never include this file directly, include libavb.h instead."
#endif

#ifndef AVB_FOOTER_H_
#define AVB_FOOTER_H_

#include "avb_sysdeps.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Magic for the footer. */
#define AVB_FOOTER_MAGIC "AVBf"
#define AVB_FOOTER_MAGIC_LEN 4

/* Size of the footer. */
#define AVB_FOOTER_SIZE 64

/* The current footer version used - keep in sync with avbtool. */
#define AVB_FOOTER_VERSION_MAJOR 1
#define AVB_FOOTER_VERSION_MINOR 0

/* The struct used as a footer used on partitions, used to find the
 * AvbVBMetaImageHeader struct. This struct is always stored at the
 * end of a partition.
 */
typedef struct AvbFooter {
  /*   0: Four bytes equal to "AVBf" (AVB_FOOTER_MAGIC). */
  uint8_t magic[AVB_FOOTER_MAGIC_LEN];
  /*   4: The major version of the footer struct. */
  uint32_t version_major;
  /*   8: The minor version of the footer struct. */
  uint32_t version_minor;

  /*  12: The original size of the image on the partition. */
  uint64_t original_image_size;

  /*  20: The offset of the |AvbVBMetaImageHeader| struct. */
  uint64_t vbmeta_offset;

  /*  28: The size of the vbmeta block (header + auth + aux blocks). */
  uint64_t vbmeta_size;

  /*  36: Padding to ensure struct is size AVB_FOOTER_SIZE bytes. This
   * must be set to zeroes.
   */
  uint8_t reserved[28];
} AVB_ATTR_PACKED AvbFooter;

/* Copies |src| to |dest| and validates, byte-swapping fields in the
 * process if needed. Returns true if valid, false if invalid.
 */
bool avb_footer_validate_and_byteswap(const AvbFooter* src, AvbFooter* dest)
    AVB_ATTR_WARN_UNUSED_RESULT;

#ifdef __cplusplus
}
#endif

#endif /* AVB_FOOTER_H_ */
