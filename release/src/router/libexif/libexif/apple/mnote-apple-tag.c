/* mnote-apple-tag.c:
 *
 * Copyright (c) 2018 zhanwang-sky <zhanwang_sky@163.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA  02110-1301  USA.
 */

#include <config.h>
#include "mnote-apple-tag.h"

#include <libexif/i18n.h>
#include <libexif/exif-utils.h>

#include <stdlib.h>

static const struct {
    MnoteAppleTag tag;
    const char *name;
    const char *title;
    const char *description;
} table[] = {
#ifndef NO_VERBOSE_TAG_STRINGS
    {MNOTE_APPLE_TAG_HDR, "HDR", N_("HDR Mode"), ""},
    {MNOTE_APPLE_TAG_RUNTIME, "RUNTIME", N_("Runtime"), ""},
    {MNOTE_APPLE_TAG_ACCELERATION_VECTOR, "ACCELERATION_VECTOR", N_("Acceleration Vector"), ""},
    {MNOTE_APPLE_TAG_HDR, "HDR", N_("HDR"), ""},
    {MNOTE_APPLE_TAG_BURST_UUID, "BURST_UUID", N_("Burst UUID"), ""},
    {MNOTE_APPLE_TAG_MEDIA_GROUP_UUID, "MEDIA_GROUP_UUID", N_("Media Group UUID"), ""},
    {MNOTE_APPLE_TAG_IMAGE_UNIQUE_ID, "IMAGE_UNIQUE_ID", N_("Image Unique ID"), ""},
#endif
    {0, NULL, NULL, NULL}
};

const char *
mnote_apple_tag_get_name(MnoteAppleTag t) {
    unsigned int i;

    for (i = 0; i < sizeof (table) / sizeof (table[0]); i++) {
        if (table[i].tag == t) {
            return table[i].name;
        }
    }

    return NULL;
}

const char *
mnote_apple_tag_get_title(MnoteAppleTag t) {
    unsigned int i;

    (void) bindtextdomain(GETTEXT_PACKAGE, LOCALEDIR);
    for (i = 0; i < sizeof (table) / sizeof (table[0]); i++) {
        if (table[i].tag == t) {
            return _(table[i].title);
        }
    }

    return NULL;
}

const char *
mnote_apple_tag_get_description(MnoteAppleTag t) {
    unsigned int i;

    for (i = 0; i < sizeof (table) / sizeof (table[0]); i++) {
        if (table[i].tag == t) {
            if (!table[i].description || !*table[i].description) {
                return "";
            }
            (void) bindtextdomain(GETTEXT_PACKAGE, LOCALEDIR);
            return _(table[i].description);
        }
    }

    return NULL;
}
