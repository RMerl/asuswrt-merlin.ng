/*
 * Copyright © 2010 Codethink Limited
 * Copyright © 2011 Canonical Limited
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the licence, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef __G_SETTINGS_SCHEMA_H__
#define __G_SETTINGS_SCHEMA_H__

#include <glib-object.h>

G_BEGIN_DECLS

typedef struct _GSettingsSchemaSource                       GSettingsSchemaSource;
typedef struct _GSettingsSchema                             GSettingsSchema;

#define                 G_TYPE_SETTINGS_SCHEMA_SOURCE                   (g_settings_schema_source_get_type ())
GLIB_AVAILABLE_IN_2_32
GType                   g_settings_schema_source_get_type               (void) G_GNUC_CONST;

GLIB_AVAILABLE_IN_2_32
GSettingsSchemaSource * g_settings_schema_source_get_default            (void);
GLIB_AVAILABLE_IN_2_32
GSettingsSchemaSource * g_settings_schema_source_ref                    (GSettingsSchemaSource  *source);
GLIB_AVAILABLE_IN_2_32
void                    g_settings_schema_source_unref                  (GSettingsSchemaSource  *source);

GLIB_AVAILABLE_IN_2_32
GSettingsSchemaSource * g_settings_schema_source_new_from_directory     (const gchar            *directory,
                                                                         GSettingsSchemaSource  *parent,
                                                                         gboolean                trusted,
                                                                         GError                **error);

GLIB_AVAILABLE_IN_2_32
GSettingsSchema *       g_settings_schema_source_lookup                 (GSettingsSchemaSource  *source,
                                                                         const gchar            *schema_id,
                                                                         gboolean                recursive);

#define                 G_TYPE_SETTINGS_SCHEMA                          (g_settings_schema_get_type ())
GLIB_AVAILABLE_IN_2_32
GType                   g_settings_schema_get_type                      (void) G_GNUC_CONST;

GLIB_AVAILABLE_IN_2_32
GSettingsSchema *       g_settings_schema_ref                           (GSettingsSchema        *schema);
GLIB_AVAILABLE_IN_2_32
void                    g_settings_schema_unref                         (GSettingsSchema        *schema);

GLIB_AVAILABLE_IN_2_32
const gchar *           g_settings_schema_get_id                        (GSettingsSchema        *schema);
GLIB_AVAILABLE_IN_2_32
const gchar *           g_settings_schema_get_path                      (GSettingsSchema        *schema);

G_END_DECLS

#endif /* __G_SETTINGS_SCHEMA_H__ */
