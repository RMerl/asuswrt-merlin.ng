#include "fuzz.h"

int
LLVMFuzzerTestOneInput (const unsigned char *data, size_t size)
{
  const gchar *gdata = (const gchar*) data;
  GVariant *variant = NULL;
  gchar *text = NULL;

  fuzz_set_logging_func ();

  variant = g_variant_parse (NULL, gdata, gdata + size, NULL, NULL);
  if (variant == NULL)
    return 0;

  text = g_variant_print (variant, TRUE);

  g_free (text);
  g_variant_unref (variant);
  return 0;
}
