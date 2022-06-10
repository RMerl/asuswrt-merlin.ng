#include "lib/net/address.h"
#include "lib/malloc/malloc.h"

#include "test/fuzz/fuzzing.h"

int
fuzz_init(void)
{
  return 0;
}

int
fuzz_cleanup(void)
{
  return 0;
}

int
fuzz_main(const uint8_t *data, size_t sz)
{
  tor_addr_t addr;
  char *fuzzing_data = tor_memdup_nulterm(data, sz);
  tor_addr_parse(&addr, fuzzing_data);
  tor_free(fuzzing_data);
  return 0;
}
