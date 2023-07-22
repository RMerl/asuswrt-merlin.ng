#include "fuzz.h"

int
LLVMFuzzerTestOneInput (const unsigned char *data, size_t size)
{
  GVariant *variant = NULL, *normal_variant = NULL;

  fuzz_set_logging_func ();

  variant = g_variant_new_from_data (G_VARIANT_TYPE_VARIANT, data, size, FALSE,
                                     NULL, NULL);
  if (variant == NULL)
    return 0;

  normal_variant = g_variant_take_ref (g_variant_get_normal_form (variant));
  g_variant_get_data (variant);

  g_variant_unref (normal_variant);
  g_variant_unref (variant);
  return 0;
}
