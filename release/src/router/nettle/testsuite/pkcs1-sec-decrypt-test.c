#include "testutils.h"

#include "pkcs1-internal.h"

#if HAVE_VALGRIND_MEMCHECK_H
# include <valgrind/memcheck.h>
static int
pkcs1_decrypt_for_test(size_t msg_len, uint8_t *msg,
                       size_t pad_len, uint8_t *pad)
{
  int ret;

  VALGRIND_MAKE_MEM_UNDEFINED (msg, msg_len);
  VALGRIND_MAKE_MEM_UNDEFINED (pad, pad_len);

  ret = _pkcs1_sec_decrypt (msg_len, msg, pad_len, pad);

  VALGRIND_MAKE_MEM_DEFINED (msg, msg_len);
  VALGRIND_MAKE_MEM_DEFINED (pad, pad_len);
  VALGRIND_MAKE_MEM_DEFINED (&ret, sizeof (ret));

  return ret;
}
#else
#define pkcs1_decrypt_for_test _pkcs1_sec_decrypt
#endif

void
test_main(void)
{
  uint8_t pad[128];
  uint8_t buffer[] =
    "\x00\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f\x10"
    "\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1a\x1b\x1c\x1d\x1e\x1f\x20"
    "\x21\x22\x23\x24\x25\x26\x27\x28\x29\x2a\x2b\x2c\x2d\x2e\x2f\x30"
    "\x31\x32\x33\x34\x35\x36\x37\x38\x39\x3a\x3b\x3c\x3d\x3e\x3f\x40"
    "\x41\x42\x43\x44\x45\x46\x47\x48\x49\x4a\x4b\x4c\x4d\x4e\x4f\x50"
    "\x51\x52\x53\x54\x55\x56\x57\x58\x59\x5a\x5b\x5c\x5d\x5e\x5f\x60"
    "\x61\x62\x63\x64\x65\x66\x67\x68\x69\x6a\x6b\x6c\x6d\x6e\x6f\x70"
    "\x00\x53\x49\x47\x4e\x45\x44\x20\x4d\x45\x53\x53\x41\x47\x45\x2e";
  uint8_t message[15];

  memcpy(pad, buffer, 128);

  memset (message, 'A', 15);
  ASSERT (pkcs1_decrypt_for_test(15, message, 128, pad) == 1);
  ASSERT (memcmp (message, "SIGNED MESSAGE.", 15) == 0);

  /* break format byte 1 */
  memcpy(pad, buffer, 128);
  pad[0] = 1;
  memset (message, 'B', 15);
  ASSERT (pkcs1_decrypt_for_test(15, message, 128, pad) == 0);
  ASSERT (memcmp (message, "BBBBBBBBBBBBBBB", 15) == 0);

  /* break format byte 2 */
  memcpy(pad, buffer, 128);
  pad[1] = 1;
  memset (message, 'C', 15);
  ASSERT (pkcs1_decrypt_for_test(15, message, 128, pad) == 0);
  ASSERT (memcmp (message, "CCCCCCCCCCCCCCC", 15) == 0);

  /* break padding */
  memcpy(pad, buffer, 128);
  pad[24] = 0;
  memset (message, 'D', 15);
  ASSERT (pkcs1_decrypt_for_test(15, message, 128, pad) == 0);
  ASSERT (memcmp (message, "DDDDDDDDDDDDDDD", 15) == 0);

  /* break terminator */
  memcpy(pad, buffer, 128);
  pad[112] = 1;
  memset (message, 'E', 15);
  ASSERT (pkcs1_decrypt_for_test(15, message, 128, pad) == 0);
  ASSERT (memcmp (message, "EEEEEEEEEEEEEEE", 15) == 0);
}
