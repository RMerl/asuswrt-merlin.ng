// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2016 The Android Open Source Project
 */

#include "avb_descriptor.h"
#include "avb_util.h"
#include "avb_vbmeta_image.h"

bool avb_descriptor_validate_and_byteswap(const AvbDescriptor* src,
                                          AvbDescriptor* dest) {
  dest->tag = avb_be64toh(src->tag);
  dest->num_bytes_following = avb_be64toh(src->num_bytes_following);

  if ((dest->num_bytes_following & 0x07) != 0) {
    avb_error("Descriptor size is not divisible by 8.\n");
    return false;
  }
  return true;
}

bool avb_descriptor_foreach(const uint8_t* image_data,
                            size_t image_size,
                            AvbDescriptorForeachFunc foreach_func,
                            void* user_data) {
  const AvbVBMetaImageHeader* header = NULL;
  bool ret = false;
  const uint8_t* image_end;
  const uint8_t* desc_start;
  const uint8_t* desc_end;
  const uint8_t* p;

  if (image_data == NULL) {
    avb_error("image_data is NULL\n.");
    goto out;
  }

  if (foreach_func == NULL) {
    avb_error("foreach_func is NULL\n.");
    goto out;
  }

  if (image_size < sizeof(AvbVBMetaImageHeader)) {
    avb_error("Length is smaller than header.\n");
    goto out;
  }

  /* Ensure magic is correct. */
  if (avb_memcmp(image_data, AVB_MAGIC, AVB_MAGIC_LEN) != 0) {
    avb_error("Magic is incorrect.\n");
    goto out;
  }

  /* Careful, not byteswapped - also ensure it's aligned properly. */
  avb_assert_aligned(image_data);
  header = (const AvbVBMetaImageHeader*)image_data;
  image_end = image_data + image_size;

  desc_start = image_data + sizeof(AvbVBMetaImageHeader) +
               avb_be64toh(header->authentication_data_block_size) +
               avb_be64toh(header->descriptors_offset);

  desc_end = desc_start + avb_be64toh(header->descriptors_size);

  if (desc_start < image_data || desc_start > image_end ||
      desc_end < image_data || desc_end > image_end || desc_end < desc_start) {
    avb_error("Descriptors not inside passed-in data.\n");
    goto out;
  }

  for (p = desc_start; p < desc_end;) {
    const AvbDescriptor* dh = (const AvbDescriptor*)p;
    avb_assert_aligned(dh);
    uint64_t nb_following = avb_be64toh(dh->num_bytes_following);
    uint64_t nb_total = sizeof(AvbDescriptor) + nb_following;

    if ((nb_total & 7) != 0) {
      avb_error("Invalid descriptor length.\n");
      goto out;
    }

    if (nb_total + p < desc_start || nb_total + p > desc_end) {
      avb_error("Invalid data in descriptors array.\n");
      goto out;
    }

    if (foreach_func(dh, user_data) == 0) {
      goto out;
    }

    p += nb_total;
  }

  ret = true;

out:
  return ret;
}

static bool count_descriptors(const AvbDescriptor* descriptor,
                              void* user_data) {
  size_t* num_descriptors = user_data;
  *num_descriptors += 1;
  return true;
}

typedef struct {
  size_t descriptor_number;
  const AvbDescriptor** descriptors;
} SetDescriptorData;

static bool set_descriptors(const AvbDescriptor* descriptor, void* user_data) {
  SetDescriptorData* data = user_data;
  data->descriptors[data->descriptor_number++] = descriptor;
  return true;
}

const AvbDescriptor** avb_descriptor_get_all(const uint8_t* image_data,
                                             size_t image_size,
                                             size_t* out_num_descriptors) {
  size_t num_descriptors = 0;
  SetDescriptorData data;

  avb_descriptor_foreach(
      image_data, image_size, count_descriptors, &num_descriptors);

  data.descriptor_number = 0;
  data.descriptors =
      avb_calloc(sizeof(const AvbDescriptor*) * (num_descriptors + 1));
  if (data.descriptors == NULL) {
    return NULL;
  }
  avb_descriptor_foreach(image_data, image_size, set_descriptors, &data);
  avb_assert(data.descriptor_number == num_descriptors);

  if (out_num_descriptors != NULL) {
    *out_num_descriptors = num_descriptors;
  }

  return data.descriptors;
}
