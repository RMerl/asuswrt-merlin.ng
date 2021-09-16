#include "testutils.h"
#include "blowfish.h"

static void
test_bcrypt(int succeed, const struct tstring *key,
	    const struct tstring *hash)
{
  if (succeed != blowfish_bcrypt_verify(key->length, key->data,
                                       hash->length, hash->data))
    {
      fprintf(stderr, "blowfish_bcrypt_verify failed:\nKey:");
      tstring_print_hex(key);
      fprintf(stderr, "\nHash: ");
      tstring_print_hex(hash);
      fprintf(stderr, "\n");
      FAIL();
    }
}

void
test_main(void)
{
  /* Tests for BSD-style bcrypt.
     From John the Ripper 1.7.9 via Phpass */
  test_bcrypt(1, SDATA("U*U"), SDATA("$2a$05$CCCCCCCCCCCCCCCCCCCCC.E5YPO9kmyuRGyh0XouQYb4YMJKvyOeW"));
  test_bcrypt(1, SDATA("U*U*"), SDATA("$2a$05$CCCCCCCCCCCCCCCCCCCCC.VGOzA784oUp/Z0DY336zx7pLYAy0lwK"));
  test_bcrypt(1, SDATA("U*U*U"), SDATA("$2a$05$XXXXXXXXXXXXXXXXXXXXXOAcXxm9kjPGEMsLznoKqmqw7tc8WCx4a"));
  test_bcrypt(1, SDATA(""), SDATA("$2a$05$CCCCCCCCCCCCCCCCCCCCC.7uG0VCzI2bS7j6ymqJi9CdcdxiRTWNy"));
  test_bcrypt(1, SDATA("0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789chars after 72 are ignored"), SDATA("$2a$05$abcdefghijklmnopqrstuu5s2v8.iXieOjg/.AySBTTZIIVFJeBui"));
  test_bcrypt(1, SDATA("\xa3"), SDATA("$2x$05$/OK.fbVrR/bpIqNJ5ianF.CE5elHaaO4EbggVDjb8P19RukzXSM3e"));
  test_bcrypt(1, SDATA("\xa3"), SDATA("$2y$05$/OK.fbVrR/bpIqNJ5ianF.Sa7shbm4.OzKpvFnX1pQLmQW96oUlCq"));
  test_bcrypt(1, SDATA("\xd1\x91"), SDATA("$2x$05$6bNw2HLQYeqHYyBfLMsv/OiwqTymGIGzFsA4hOTWebfehXHNprcAS"));
  test_bcrypt(1, SDATA("\xd0\xc1\xd2\xcf\xcc\xd8"), SDATA("$2x$05$6bNw2HLQYeqHYyBfLMsv/O9LIGgn8OMzuDoHfof8AQimSGfcSWxnS"));
  test_bcrypt(1, SDATA("\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa""chars after 72 are ignored as usual"), SDATA("$2a$05$/OK.fbVrR/bpIqNJ5ianF.swQOIzjOiJ9GHEPuhEkvqrUyvWhEMx6"));
  test_bcrypt(1, SDATA("\xaa\x55\xaa\x55\xaa\x55\xaa\x55\xaa\x55\xaa\x55\xaa\x55\xaa\x55\xaa\x55\xaa\x55\xaa\x55\xaa\x55\xaa\x55\xaa\x55\xaa\x55\xaa\x55\xaa\x55\xaa\x55\xaa\x55\xaa\x55\xaa\x55\xaa\x55\xaa\x55\xaa\x55\xaa\x55\xaa\x55\xaa\x55\xaa\x55\xaa\x55\xaa\x55\xaa\x55\xaa\x55\xaa\x55\xaa\x55\xaa\x55\xaa\x55"), SDATA("$2a$05$/OK.fbVrR/bpIqNJ5ianF.R9xrDjiycxMbQE2bp.vgqlYpW5wx2yy"));
  test_bcrypt(1, SDATA(""), SDATA("$2a$05$CCCCCCCCCCCCCCCCCCCCC.7uG0VCzI2bS7j6ymqJi9CdcdxiRTWNy"));
  test_bcrypt(1, SDATA("\x55\xaa\xff\x55\xaa\xff\x55\xaa\xff\x55\xaa\xff\x55\xaa\xff\x55\xaa\xff\x55\xaa\xff\x55\xaa\xff\x55\xaa\xff\x55\xaa\xff\x55\xaa\xff\x55\xaa\xff\x55\xaa\xff\x55\xaa\xff\x55\xaa\xff\x55\xaa\xff\x55\xaa\xff\x55\xaa\xff\x55\xaa\xff\x55\xaa\xff\x55\xaa\xff\x55\xaa\xff\x55\xaa\xff\x55\xaa\xff"), SDATA("$2a$05$/OK.fbVrR/bpIqNJ5ianF.9tQZzcJfm3uj2NvJ/n5xkhpqLrMpWCe"));
  /* From Openwall's crypt v1.2 via Phpass */
  test_bcrypt(0, SDATA(""), SDATA("$2a$03$CCCCCCCCCCCCCCCCCCCCC."));
  test_bcrypt(0, SDATA(""), SDATA("$2a$32$CCCCCCCCCCCCCCCCCCCCC."));
  test_bcrypt(0, SDATA(""), SDATA("$2z$05$CCCCCCCCCCCCCCCCCCCCC."));
  test_bcrypt(0, SDATA(""), SDATA("$2`$05$CCCCCCCCCCCCCCCCCCCCC."));
  test_bcrypt(0, SDATA(""), SDATA("$2{$05$CCCCCCCCCCCCCCCCCCCCC."));
  /* Stephen's personal tests */
  test_bcrypt(1, SDATA("yawinpassword"),
     SDATA("$2a$04$MzVXtd4o0y4DOlyHMMLMDeE4/eezrsT5Xad.2lmGr/NkCpwBgvn3e"));
  test_bcrypt(0, SDATA("xawinpassword"),
     SDATA("$2a$04$MzVXtd4o0y4DOlyHMMLMDeE4/eezrsT5Xad.2lmGr/NkCpwBgvn3e"));
  test_bcrypt(1, SDATA("Bootq9sH5"),
     SDATA("$2y$10$1b2lPgo4XumibnJGN3r3sOsXFfVVYlebFjlw47qpaslC4KIwu9dAK"));
  test_bcrypt(0, SDATA("Bootq9sH6"),
     SDATA("$2y$10$1b2lPgo4XumibnJGN3r3sOsXFfVVYlebFjlw47qpaslC4KIwu9dAK"));
  test_bcrypt(0, SDATA("1234"), SDATA("$2y$"));
}
