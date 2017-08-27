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

#include "config.h"

#include "gsettingsschema-internal.h"
#include "gsettings.h"

#include "gvdb/gvdb-reader.h"
#include "strinfo.c"

#include <glibintl.h>
#include <locale.h>
#include <string.h>

/**
 * SECTION:gsettingsschema
 * @short_description: Introspecting and controlling the loading of
 *                     GSettings schemas
 *
 * The #GSettingsSchemaSource and #GSettingsSchema APIs provide a
 * mechanism for advanced control over the loading of schemas and a
 * mechanism for introspecting their content.
 *
 * Plugin loading systems that wish to provide plugins a way to access
 * settings face the problem of how to make the schemas for these
 * settings visible to GSettings.  Typically, a plugin will want to ship
 * the schema along with itself and it won't be installed into the
 * standard system directories for schemas.
 *
 * #GSettingsSchemaSource provides a mechanism for dealing with this by
 * allowing the creation of a new 'schema source' from which schemas can
 * be acquired.  This schema source can then become part of the metadata
 * associated with the plugin and queried whenever the plugin requires
 * access to some settings.
 *
 * Consider the following example:
 *
 * |[
 * typedef struct
 * {
 *    ...
 *    GSettingsSchemaSource *schema_source;
 *    ...
 * } Plugin;
 *
 * Plugin *
 * initialise_plugin (const gchar *dir)
 * {
 *   Plugin *plugin;
 *
 *   ...
 *
 *   plugin->schema_source =
 *     g_settings_new_schema_source_from_directory (dir,
 *       g_settings_schema_source_get_default (), FALSE, NULL);
 *
 *   ...
 *
 *   return plugin;
 * }
 *
 * ...
 *
 * GSettings *
 * plugin_get_settings (Plugin      *plugin,
 *                      const gchar *schema_id)
 * {
 *   GSettingsSchema *schema;
 *
 *   if (schema_id == NULL)
 *     schema_id = plugin->identifier;
 *
 *   schema = g_settings_schema_source_lookup (plugin->schema_source,
 *                                             schema_id, FALSE);
 *
 *   if (schema == NULL)
 *     {
 *       ... disable the plugin or abort, etc ...
 *     }
 *
 *   return g_settings_new_full (schema, NULL, NULL);
 * }
 * ]|
 *
 * The code above shows how hooks should be added to the code that
 * initialises (or enables) the plugin to create the schema source and
 * how an API can be added to the plugin system to provide a convenient
 * way for the plugin to access its settings, using the schemas that it
 * ships.
 *
 * From the standpoint of the plugin, it would need to ensure that it
 * ships a gschemas.compiled file as part of itself, and then simply do
 * the following:
 *
 * |[
 * {
 *   GSettings *settings;
 *   gint some_value;
 *
 *   settings = plugin_get_settings (self, NULL);
 *   some_value = g_settings_get_int (settings, "some-value");
 *   ...
 * }
 * ]|
 *
 * It's also possible that the plugin system expects the schema source
 * files (ie: .gschema.xml files) instead of a gschemas.compiled file.
 * In that case, the plugin loading system must compile the schemas for
 * itself before attempting to create the settings source.
 *
 * Since: 2.32
 **/

/**
 * GSettingsSchema:
 *
 * This is an opaque structure type.  You may not access it directly.
 *
 * Since: 2.32
 **/
struct _GSettingsSchema
{
  const gchar *gettext_domain;
  const gchar *path;
  GQuark *items;
  gint n_items;
  GvdbTable *table;
  gchar *id;

  gint ref_count;
};

/**
 * G_TYPE_SETTINGS_SCHEMA_SOURCE:
 *
 * A boxed #GType corresponding to #GSettingsSchemaSource.
 *
 * Since: 2.32
 **/
G_DEFINE_BOXED_TYPE (GSettingsSchemaSource, g_settings_schema_source, g_settings_schema_source_ref, g_settings_schema_source_unref)

/**
 * G_TYPE_SETTINGS_SCHEMA:
 *
 * A boxed #GType corresponding to #GSettingsSchema.
 *
 * Since: 2.32
 **/
G_DEFINE_BOXED_TYPE (GSettingsSchema, g_settings_schema, g_settings_schema_ref, g_settings_schema_unref)

/**
 * GSettingsSchemaSource:
 *
 * This is an opaque structure type.  You may not access it directly.
 *
 * Since: 2.32
 **/
struct _GSettingsSchemaSource
{
  GSettingsSchemaSource *parent;
  GvdbTable *table;

  gint ref_count;
};

static GSettingsSchemaSource *schema_sources;

static void
prepend_schema_table (GvdbTable *table)
{
  GSettingsSchemaSource *source;

  /* we steal the reference from 'schema_sources' for our ->parent */
  source = g_slice_new (GSettingsSchemaSource);
  source->parent = schema_sources;
  source->table = table;
  source->ref_count = 1;

  schema_sources = source;
}

/**
 * g_settings_schema_source_ref:
 * @source: a #GSettingsSchemaSource
 *
 * Increase the reference count of @source, returning a new reference.
 *
 * Returns: a new reference to @source
 *
 * Since: 2.32
 **/
GSettingsSchemaSource *
g_settings_schema_source_ref (GSettingsSchemaSource *source)
{
  g_atomic_int_inc (&source->ref_count);

  return source;
}

/**
 * g_settings_schema_source_unref:
 * @source: a #GSettingsSchemaSource
 *
 * Decrease the reference count of @source, possibly freeing it.
 *
 * Since: 2.32
 **/
void
g_settings_schema_source_unref (GSettingsSchemaSource *source)
{
  if (g_atomic_int_dec_and_test (&source->ref_count))
    {
      if (source == schema_sources)
        g_error ("g_settings_schema_source_unref() called too many times on the default schema source");

      if (source->parent)
        g_settings_schema_source_unref (source->parent);
      gvdb_table_unref (source->table);

      g_slice_free (GSettingsSchemaSource, source);
    }
}

/**
 * g_settings_schema_source_new_from_directory:
 * @directory: the filename of a directory
 * @parent: (allow-none): a #GSettingsSchemaSource, or %NULL
 * @trusted: %TRUE, if the directory is trusted
 * @error: a pointer to a #GError pointer set to %NULL, or %NULL
 *
 * Attempts to create a new schema source corresponding to the contents
 * of the given directory.
 *
 * This function is not required for normal uses of #GSettings but it
 * may be useful to authors of plugin management systems.
 *
 * The directory should contain a file called
 * <filename>gschemas.compiled</filename> as produced by
 * <command>glib-compile-schemas</command>.
 *
 * If @trusted is %TRUE then <filename>gschemas.compiled</filename> is
 * trusted not to be corrupted.  This assumption has a performance
 * advantage, but can result in crashes or inconsistent behaviour in the
 * case of a corrupted file.  Generally, you should set @trusted to
 * %TRUE for files installed by the system and to %FALSE for files in
 * the home directory.
 *
 * If @parent is non-%NULL then there are two effects.
 *
 * First, if g_settings_schema_source_lookup() is called with the
 * @recursive flag set to %TRUE and the schema can not be found in the
 * source, the lookup will recurse to the parent.
 *
 * Second, any references to other schemas specified within this
 * source (ie: <literal>child</literal> or <literal>extends</literal>)
 * references may be resolved from the @parent.
 *
 * For this second reason, except in very unusual situations, the
 * @parent should probably be given as the default schema source, as
 * returned by g_settings_schema_source_get_default().
 *
 * Since: 2.32
 **/
GSettingsSchemaSource *
g_settings_schema_source_new_from_directory (const gchar            *directory,
                                             GSettingsSchemaSource  *parent,
                                             gboolean                trusted,
                                             GError                **error)
{
  GSettingsSchemaSource *source;
  GvdbTable *table;
  gchar *filename;

  filename = g_build_filename (directory, "gschemas.compiled", NULL);
  table = gvdb_table_new (filename, trusted, error);
  g_free (filename);

  if (table == NULL)
    return NULL;

  source = g_slice_new (GSettingsSchemaSource);
  source->parent = parent ? g_settings_schema_source_ref (parent) : NULL;
  source->table = table;
  source->ref_count = 1;

  return source;
}

static void
initialise_schema_sources (void)
{
  static gsize initialised;

  /* need a separate variable because 'schema_sources' may legitimately
   * be null if we have zero valid schema sources
   */
  if G_UNLIKELY (g_once_init_enter (&initialised))
    {
      const gchar * const *dirs;
      const gchar *path;
      gint i;

      /* iterate in reverse: count up, then count down */
      dirs = g_get_system_data_dirs ();
      for (i = 0; dirs[i]; i++);

      while (i--)
        {
          gchar *filename;
          GvdbTable *table;

          filename = g_build_filename (dirs[i], "glib-2.0", "schemas", "gschemas.compiled", NULL);
          table = gvdb_table_new (filename, TRUE, NULL);

          if (table != NULL)
            prepend_schema_table (table);

          g_free (filename);
        }

      if ((path = g_getenv ("GSETTINGS_SCHEMA_DIR")) != NULL)
        {
          gchar *filename;
          GvdbTable *table;

          filename = g_build_filename (path, "gschemas.compiled", NULL);
          table = gvdb_table_new (filename, TRUE, NULL);

          if (table != NULL)
            prepend_schema_table (table);

          g_free (filename);
        }

      g_once_init_leave (&initialised, TRUE);
    }
}

/**
 * g_settings_schema_source_get_default:
 *
 * Gets the default system schema source.
 *
 * This function is not required for normal uses of #GSettings but it
 * may be useful to authors of plugin management systems or to those who
 * want to introspect the content of schemas.
 *
 * If no schemas are installed, %NULL will be returned.
 *
 * The returned source may actually consist of multiple schema sources
 * from different directories, depending on which directories were given
 * in <envar>XDG_DATA_DIRS</envar> and
 * <envar>GSETTINGS_SCHEMA_DIR</envar>.  For this reason, all lookups
 * performed against the default source should probably be done
 * recursively.
 *
 * Returns: (transfer none): the default schema source
 *
 * Since: 2.32
 **/
 GSettingsSchemaSource *
g_settings_schema_source_get_default (void)
{
  initialise_schema_sources ();

  return schema_sources;
}

/**
 * g_settings_schema_source_lookup:
 * @source: a #GSettingsSchemaSource
 * @schema_id: a schema ID
 * @recursive: %TRUE if the lookup should be recursive
 *
 * Looks up a schema with the identifier @schema_id in @source.
 *
 * This function is not required for normal uses of #GSettings but it
 * may be useful to authors of plugin management systems or to those who
 * want to introspect the content of schemas.
 *
 * If the schema isn't found directly in @source and @recursive is %TRUE
 * then the parent sources will also be checked.
 *
 * If the schema isn't found, %NULL is returned.
 *
 * Returns: (transfer full): a new #GSettingsSchema
 *
 * Since: 2.32
 **/
GSettingsSchema *
g_settings_schema_source_lookup (GSettingsSchemaSource *source,
                                 const gchar           *schema_id,
                                 gboolean               recursive)
{
  GSettingsSchema *schema;
  GvdbTable *table;

  g_return_val_if_fail (source != NULL, NULL);
  g_return_val_if_fail (schema_id != NULL, NULL);

  table = gvdb_table_get_table (source->table, schema_id);

  if (table == NULL && recursive)
    for (source = source->parent; source; source = source->parent)
      if ((table = gvdb_table_get_table (source->table, schema_id)))
        break;

  if (table == NULL)
    return NULL;

  schema = g_slice_new0 (GSettingsSchema);
  schema->ref_count = 1;
  schema->id = g_strdup (schema_id);
  schema->table = table;
  schema->path = g_settings_schema_get_string (schema, ".path");
  schema->gettext_domain = g_settings_schema_get_string (schema, ".gettext-domain");

  if (schema->gettext_domain)
    bind_textdomain_codeset (schema->gettext_domain, "UTF-8");

  return schema;
}

static gboolean
steal_item (gpointer key,
            gpointer value,
            gpointer user_data)
{
  gchar ***ptr = user_data;

  *(*ptr)++ = (gchar *) key;

  return TRUE;
}

static const gchar * const *non_relocatable_schema_list;
static const gchar * const *relocatable_schema_list;
static gsize schema_lists_initialised;

static void
ensure_schema_lists (void)
{
  if (g_once_init_enter (&schema_lists_initialised))
    {
      GSettingsSchemaSource *source;
      GHashTable *single, *reloc;
      const gchar **ptr;
      gchar **list;
      gint i;

      initialise_schema_sources ();

      /* We use hash tables to avoid duplicate listings for schemas that
       * appear in more than one file.
       */
      single = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);
      reloc = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);

      for (source = schema_sources; source; source = source->parent)
        {
          list = gvdb_table_list (source->table, "");

          /* empty schema cache file? */
          if (list == NULL)
            continue;

          for (i = 0; list[i]; i++)
            {
              if (!g_hash_table_lookup (single, list[i]) &&
                  !g_hash_table_lookup (reloc, list[i]))
                {
                  GvdbTable *table;

                  table = gvdb_table_get_table (source->table, list[i]);
                  g_assert (table != NULL);

                  if (gvdb_table_has_value (table, ".path"))
                    g_hash_table_insert (single, g_strdup (list[i]), NULL);
                  else
                    g_hash_table_insert (reloc, g_strdup (list[i]), NULL);

                  gvdb_table_unref (table);
                }
            }

          g_strfreev (list);
        }

      ptr = g_new (const gchar *, g_hash_table_size (single) + 1);
      non_relocatable_schema_list = ptr;
      g_hash_table_foreach_steal (single, steal_item, &ptr);
      g_hash_table_unref (single);
      *ptr = NULL;

      ptr = g_new (const gchar *, g_hash_table_size (reloc) + 1);
      relocatable_schema_list = ptr;
      g_hash_table_foreach_steal (reloc, steal_item, &ptr);
      g_hash_table_unref (reloc);
      *ptr = NULL;

      g_once_init_leave (&schema_lists_initialised, TRUE);
    }
}

/**
 * g_settings_list_schemas:
 *
 * Gets a list of the #GSettings schemas installed on the system.  The
 * returned list is exactly the list of schemas for which you may call
 * g_settings_new() without adverse effects.
 *
 * This function does not list the schemas that do not provide their own
 * paths (ie: schemas for which you must use
 * g_settings_new_with_path()).  See
 * g_settings_list_relocatable_schemas() for that.
 *
 * Returns: (element-type utf8) (transfer none):  a list of #GSettings
 *   schemas that are available.  The list must not be modified or
 *   freed.
 *
 * Since: 2.26
 **/
const gchar * const *
g_settings_list_schemas (void)
{
  ensure_schema_lists ();

  return non_relocatable_schema_list;
}

/**
 * g_settings_list_relocatable_schemas:
 *
 * Gets a list of the relocatable #GSettings schemas installed on the
 * system.  These are schemas that do not provide their own path.  It is
 * usual to instantiate these schemas directly, but if you want to you
 * can use g_settings_new_with_path() to specify the path.
 *
 * The output of this function, taken together with the output of
 * g_settings_list_schemas() represents the complete list of all
 * installed schemas.
 *
 * Returns: (element-type utf8) (transfer none): a list of relocatable
 *   #GSettings schemas that are available.  The list must not be
 *   modified or freed.
 *
 * Since: 2.28
 **/
const gchar * const *
g_settings_list_relocatable_schemas (void)
{
  ensure_schema_lists ();

  return relocatable_schema_list;
}

/**
 * g_settings_schema_ref:
 * @schema: a #GSettingsSchema
 *
 * Increase the reference count of @schema, returning a new reference.
 *
 * Returns: a new reference to @schema
 *
 * Since: 2.32
 **/
GSettingsSchema *
g_settings_schema_ref (GSettingsSchema *schema)
{
  g_atomic_int_inc (&schema->ref_count);

  return schema;
}

/**
 * g_settings_schema_unref:
 * @schema: a #GSettingsSchema
 *
 * Decrease the reference count of @schema, possibly freeing it.
 *
 * Since: 2.32
 **/
void
g_settings_schema_unref (GSettingsSchema *schema)
{
  if (g_atomic_int_dec_and_test (&schema->ref_count))
    {
      gvdb_table_unref (schema->table);
      g_free (schema->items);
      g_free (schema->id);

      g_slice_free (GSettingsSchema, schema);
    }
}

const gchar *
g_settings_schema_get_string (GSettingsSchema *schema,
                              const gchar     *key)
{
  const gchar *result = NULL;
  GVariant *value;

  if ((value = gvdb_table_get_raw_value (schema->table, key)))
    {
      result = g_variant_get_string (value, NULL);
      g_variant_unref (value);
    }

  return result;
}

GVariantIter *
g_settings_schema_get_value (GSettingsSchema *schema,
                             const gchar     *key)
{
  GVariantIter *iter;
  GVariant *value;

  value = gvdb_table_get_raw_value (schema->table, key);

  if G_UNLIKELY (value == NULL || !g_variant_is_of_type (value, G_VARIANT_TYPE_TUPLE))
    g_error ("Settings schema '%s' does not contain a key named '%s'", schema->id, key);

  iter = g_variant_iter_new (value);
  g_variant_unref (value);

  return iter;
}

/**
 * g_settings_schema_get_path:
 * @schema: a #GSettingsSchema
 *
 * Gets the path associated with @schema, or %NULL.
 *
 * Schemas may be single-instance or relocatable.  Single-instance
 * schemas correspond to exactly one set of keys in the backend
 * database: those located at the path returned by this function.
 *
 * Relocatable schemas can be referenced by other schemas and can
 * threfore describe multiple sets of keys at different locations.  For
 * relocatable schemas, this function will return %NULL.
 *
 * Returns: (transfer none): the path of the schema, or %NULL
 *
 * Since: 2.32
 **/
const gchar *
g_settings_schema_get_path (GSettingsSchema *schema)
{
  return schema->path;
}

const gchar *
g_settings_schema_get_gettext_domain (GSettingsSchema *schema)
{
  return schema->gettext_domain;
}

gboolean
g_settings_schema_has_key (GSettingsSchema *schema,
                           const gchar     *key)
{
  return gvdb_table_has_value (schema->table, key);
}

const GQuark *
g_settings_schema_list (GSettingsSchema *schema,
                        gint            *n_items)
{
  gint i, j;

  if (schema->items == NULL)
    {
      gchar **list;
      gint len;

      list = gvdb_table_list (schema->table, "");
      len = list ? g_strv_length (list) : 0;

      schema->items = g_new (GQuark, len);
      j = 0;

      for (i = 0; i < len; i++)
        if (list[i][0] != '.')
          schema->items[j++] = g_quark_from_string (list[i]);
      schema->n_items = j;

      g_strfreev (list);
    }

  *n_items = schema->n_items;
  return schema->items;
}

/**
 * g_settings_schema_get_id:
 * @schema: a #GSettingsSchema
 *
 * Get the ID of @schema.
 *
 * Returns: (transfer none): the ID
 **/
const gchar *
g_settings_schema_get_id (GSettingsSchema *schema)
{
  return schema->id;
}

static inline void
endian_fixup (GVariant **value)
{
#if G_BYTE_ORDER == G_BIG_ENDIAN
  GVariant *tmp;

  tmp = g_variant_byteswap (*value);
  g_variant_unref (*value);
  *value = tmp;
#endif
}

void
g_settings_schema_key_init (GSettingsSchemaKey *key,
                            GSettingsSchema    *schema,
                            const gchar        *name)
{
  GVariantIter *iter;
  GVariant *data;
  guchar code;

  memset (key, 0, sizeof *key);

  iter = g_settings_schema_get_value (schema, name);

  key->schema = g_settings_schema_ref (schema);
  key->default_value = g_variant_iter_next_value (iter);
  endian_fixup (&key->default_value);
  key->type = g_variant_get_type (key->default_value);
  key->name = g_intern_string (name);

  while (g_variant_iter_next (iter, "(y*)", &code, &data))
    {
      switch (code)
        {
        case 'l':
          /* translation requested */
          g_variant_get (data, "(y&s)", &key->lc_char, &key->unparsed);
          break;

        case 'e':
          /* enumerated types... */
          key->is_enum = TRUE;
          goto choice;

        case 'f':
          /* flags... */
          key->is_flags = TRUE;
          goto choice;

        choice: case 'c':
          /* ..., choices, aliases */
          key->strinfo = g_variant_get_fixed_array (data, &key->strinfo_length, sizeof (guint32));
          break;

        case 'r':
          g_variant_get (data, "(**)", &key->minimum, &key->maximum);
          endian_fixup (&key->minimum);
          endian_fixup (&key->maximum);
          break;

        default:
          g_warning ("unknown schema extension '%c'", code);
          break;
        }

      g_variant_unref (data);
    }

  g_variant_iter_free (iter);
}

void
g_settings_schema_key_clear (GSettingsSchemaKey *key)
{
  if (key->minimum)
    g_variant_unref (key->minimum);

  if (key->maximum)
    g_variant_unref (key->maximum);

  g_variant_unref (key->default_value);

  g_settings_schema_unref (key->schema);
}

gboolean
g_settings_schema_key_type_check (GSettingsSchemaKey *key,
                                  GVariant           *value)
{
  g_return_val_if_fail (value != NULL, FALSE);

  return g_variant_is_of_type (value, key->type);
}

gboolean
g_settings_schema_key_range_check (GSettingsSchemaKey *key,
                                   GVariant           *value)
{
  if (key->minimum == NULL && key->strinfo == NULL)
    return TRUE;

  if (g_variant_is_container (value))
    {
      gboolean ok = TRUE;
      GVariantIter iter;
      GVariant *child;

      g_variant_iter_init (&iter, value);
      while (ok && (child = g_variant_iter_next_value (&iter)))
        {
          ok = g_settings_schema_key_range_check (key, child);
          g_variant_unref (child);
        }

      return ok;
    }

  if (key->minimum)
    {
      return g_variant_compare (key->minimum, value) <= 0 &&
             g_variant_compare (value, key->maximum) <= 0;
    }

  return strinfo_is_string_valid (key->strinfo, key->strinfo_length,
                                  g_variant_get_string (value, NULL));
}

GVariant *
g_settings_schema_key_range_fixup (GSettingsSchemaKey *key,
                                   GVariant           *value)
{
  const gchar *target;

  if (g_settings_schema_key_range_check (key, value))
    return g_variant_ref (value);

  if (key->strinfo == NULL)
    return NULL;

  if (g_variant_is_container (value))
    {
      GVariantBuilder builder;
      GVariantIter iter;
      GVariant *child;

      g_variant_iter_init (&iter, value);
      g_variant_builder_init (&builder, g_variant_get_type (value));

      while ((child = g_variant_iter_next_value (&iter)))
        {
          GVariant *fixed;

          fixed = g_settings_schema_key_range_fixup (key, child);
          g_variant_unref (child);

          if (fixed == NULL)
            {
              g_variant_builder_clear (&builder);
              return NULL;
            }

          g_variant_builder_add_value (&builder, fixed);
          g_variant_unref (fixed);
        }

      return g_variant_ref_sink (g_variant_builder_end (&builder));
    }

  target = strinfo_string_from_alias (key->strinfo, key->strinfo_length,
                                      g_variant_get_string (value, NULL));
  return target ? g_variant_ref_sink (g_variant_new_string (target)) : NULL;
}


GVariant *
g_settings_schema_key_get_translated_default (GSettingsSchemaKey *key)
{
  const gchar *translated;
  GError *error = NULL;
  const gchar *domain;
  GVariant *value;

  domain = g_settings_schema_get_gettext_domain (key->schema);

  if (key->lc_char == '\0')
    /* translation not requested for this key */
    return NULL;

  if (key->lc_char == 't')
    translated = g_dcgettext (domain, key->unparsed, LC_TIME);
  else
    translated = g_dgettext (domain, key->unparsed);

  if (translated == key->unparsed)
    /* the default value was not translated */
    return NULL;

  /* try to parse the translation of the unparsed default */
  value = g_variant_parse (key->type, translated, NULL, NULL, &error);

  if (value == NULL)
    {
      g_warning ("Failed to parse translated string '%s' for "
                 "key '%s' in schema '%s': %s", key->unparsed, key->name,
                 g_settings_schema_get_id (key->schema), error->message);
      g_warning ("Using untranslated default instead.");
      g_error_free (error);
    }

  else if (!g_settings_schema_key_range_check (key, value))
    {
      g_warning ("Translated default '%s' for key '%s' in schema '%s' "
                 "is outside of valid range", key->unparsed, key->name,
                 g_settings_schema_get_id (key->schema));
      g_variant_unref (value);
      value = NULL;
    }

  return value;
}

gint
g_settings_schema_key_to_enum (GSettingsSchemaKey *key,
                               GVariant           *value)
{
  gboolean it_worked;
  guint result;

  it_worked = strinfo_enum_from_string (key->strinfo, key->strinfo_length,
                                        g_variant_get_string (value, NULL),
                                        &result);

  /* 'value' can only come from the backend after being filtered for validity,
   * from the translation after being filtered for validity, or from the schema
   * itself (which the schema compiler checks for validity).  If this assertion
   * fails then it's really a bug in GSettings or the schema compiler...
   */
  g_assert (it_worked);

  return result;
}

GVariant *
g_settings_schema_key_from_enum (GSettingsSchemaKey *key,
                                 gint                value)
{
  const gchar *string;

  string = strinfo_string_from_enum (key->strinfo, key->strinfo_length, value);

  if (string == NULL)
    return NULL;

  return g_variant_new_string (string);
}

guint
g_settings_schema_key_to_flags (GSettingsSchemaKey *key,
                                GVariant           *value)
{
  GVariantIter iter;
  const gchar *flag;
  guint result;

  result = 0;
  g_variant_iter_init (&iter, value);
  while (g_variant_iter_next (&iter, "&s", &flag))
    {
      gboolean it_worked;
      guint flag_value;

      it_worked = strinfo_enum_from_string (key->strinfo, key->strinfo_length, flag, &flag_value);
      /* as in g_settings_to_enum() */
      g_assert (it_worked);

      result |= flag_value;
    }

  return result;
}

GVariant *
g_settings_schema_key_from_flags (GSettingsSchemaKey *key,
                                  guint               value)
{
  GVariantBuilder builder;
  gint i;

  g_variant_builder_init (&builder, G_VARIANT_TYPE ("as"));

  for (i = 0; i < 32; i++)
    if (value & (1u << i))
      {
        const gchar *string;

        string = strinfo_string_from_enum (key->strinfo, key->strinfo_length, 1u << i);

        if (string == NULL)
          {
            g_variant_builder_clear (&builder);
            return NULL;
          }

        g_variant_builder_add (&builder, "s", string);
      }

  return g_variant_builder_end (&builder);
}
