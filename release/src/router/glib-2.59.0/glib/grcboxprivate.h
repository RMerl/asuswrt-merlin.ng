/* grcboxprivate.h: Reference counted data
 *
 * Copyright 2018  Emmanuele Bassi
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "gtypes.h"
#include "grcbox.h"

G_BEGIN_DECLS

typedef struct {
  grefcount ref_count;

  gsize mem_size;
  gsize private_offset;

#ifndef G_DISABLE_ASSERT
  /* A "magic" number, used to perform additional integrity
   * checks on the allocated data
   */
  guint32 magic;
#endif
} GRcBox;

typedef struct {
  gatomicrefcount ref_count;

  gsize mem_size;
  gsize private_offset;

#ifndef G_DISABLE_ASSERT
  guint32 magic;
#endif
} GArcBox;

#define G_BOX_MAGIC             0x44ae2bf0

/* Keep the two refcounted boxes identical in size */
G_STATIC_ASSERT (sizeof (GRcBox) == sizeof (GArcBox));

/* This is the default alignment we use when allocating the
 * refcounted memory blocks; it's similar to the alignment
 * guaranteed by the malloc() in GNU's libc and by the GSlice
 * allocator
 */
#define STRUCT_ALIGNMENT (2 * sizeof (gsize))

#define G_RC_BOX_SIZE sizeof (GRcBox)
#define G_ARC_BOX_SIZE sizeof (GArcBox)

gpointer        g_rc_box_alloc_full     (gsize    block_size,
                                         gsize    alignment,
                                         gboolean atomic,
                                         gboolean clear);

G_END_DECLS
