#include "fuzz.h"

int
LLVMFuzzerTestOneInput (const unsigned char *data, size_t size)
{
  GKeyFile *key = NULL;

  fuzz_set_logging_func ();

  key = g_key_file_new ();
  g_key_file_load_from_data (key, (const gchar*) data, size, G_KEY_FILE_NONE,
                             NULL);

  g_key_file_free (key);
  return 0;
}
