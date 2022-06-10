#include "lib/net/address.h"
#include "lib/net/socket.h"
#include "lib/cc/ctassert.h"
#include "lib/container/smartlist.h"
#include "lib/ctime/di_ops.h"
#include "lib/log/log.h"
#include "lib/log/escape.h"
#include "lib/malloc/malloc.h"
#include "lib/net/address.h"
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
  tor_addr_t addr_result;
  char *fuzzing_data = tor_memdup_nulterm(data, sz);
  tor_addr_parse_PTR_name(&addr_result, fuzzing_data, AF_UNSPEC, 1);
  tor_free(fuzzing_data);
  return 0;
}
