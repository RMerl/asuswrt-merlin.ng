/* testutils.c */

#include "testutils.h"

#include "base16.h"
#include "cbc.h"
#include "cfb.h"
#include "ctr.h"
#include "knuth-lfib.h"
#include "macros.h"
#include "nettle-internal.h"

#include <assert.h>
#include <errno.h>
#include <ctype.h>
#include <sys/time.h>

#if HAVE_VALGRIND_MEMCHECK_H
# include <valgrind/memcheck.h>
# include <valgrind/valgrind.h>
#endif

void
die(const char *format, ...)
{
  va_list args;
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);

  abort ();
}

void *
xalloc(size_t size)
{
  void *p = malloc(size);
  if (size && !p)
    {
      fprintf(stderr, "Virtual memory exhausted.\n");
      abort();
    }

  return p;
}

static struct tstring *tstring_first = NULL;

struct tstring *
tstring_alloc (size_t length)
{
  struct tstring *s = xalloc(sizeof(struct tstring) + length);
  s->length = length;
  s->next = tstring_first;
  /* NUL-terminate, for convenience. */
  s->data[length] = '\0';
  tstring_first = s;
  return s;
}

void
tstring_clear(void)
{
  while (tstring_first)
    {
      struct tstring *s = tstring_first;
      tstring_first = s->next;
      free(s);
    }
}

struct tstring *
tstring_data(size_t length, const uint8_t *data)
{
  struct tstring *s = tstring_alloc (length);
  memcpy (s->data, data, length);
  return s;
}

struct tstring *
tstring_hex(const char *hex)
{
  struct base16_decode_ctx ctx;
  struct tstring *s;
  size_t length = strlen(hex);

  s = tstring_alloc(BASE16_DECODE_LENGTH (length));
  base16_decode_init (&ctx);
  ASSERT (base16_decode_update (&ctx, &s->length, s->data,
				length, hex));
  ASSERT (base16_decode_final (&ctx));

  return s;
}

void
tstring_print_hex(const struct tstring *s)
{
  print_hex (s->length, s->data);
}

void
print_hex(size_t length, const uint8_t *data)
{
  size_t i;
  
  for (i = 0; i < length; i++)
    {
      switch (i % 16)
	{
	default:
	  break;
	case 0:
	  printf("\n");
	  break;
	case 8:
	  printf(" ");
	  break;
	}
      printf("%02x", data[i]);
    }
  printf("\n");
}

int verbose = 0;
int test_side_channel = 0;

#if HAVE_VALGRIND_MEMCHECK_H

void
mark_bytes_undefined (size_t size, const void *p)
{
  if (test_side_channel)
    VALGRIND_MAKE_MEM_UNDEFINED(p, size);
}
void
mark_bytes_defined (size_t size, const void *p)
{
  if (test_side_channel)
    VALGRIND_MAKE_MEM_DEFINED(p, size);
}
#else
void
mark_bytes_undefined (size_t size UNUSED, const void *p UNUSED) {}
void
mark_bytes_defined (size_t size UNUSED, const void *p UNUSED) {}
#endif

int
main(int argc, char **argv)
{
  if (argc > 1)
    {
      if (argc == 2 && !strcmp(argv[1], "-v"))
	verbose = 1;
      else
	{
	  fprintf(stderr, "Invalid argument `%s', only accepted option is `-v'.\n",
		  argv[1]);
	  return 1;
	}
    }

  if (getenv("NETTLE_TEST_SIDE_CHANNEL"))
    {
#if HAVE_VALGRIND_MEMCHECK_H
      if (RUNNING_ON_VALGRIND)
	test_side_channel = 1;
      else
#endif
	SKIP();
    }
  test_main();

  tstring_clear();
  return EXIT_SUCCESS;
}

void
test_cipher(const struct nettle_cipher *cipher,
	    const struct tstring *key,
	    const struct tstring *cleartext,
	    const struct tstring *ciphertext)
{
  void *ctx = xalloc(cipher->context_size);
  uint8_t *data = xalloc(cleartext->length);
  size_t length;
  ASSERT (cleartext->length == ciphertext->length);
  length = cleartext->length;

  ASSERT (key->length == cipher->key_size);
  cipher->set_encrypt_key(ctx, key->data);
  cipher->encrypt(ctx, length, data, cleartext->data);

  if (!MEMEQ(length, data, ciphertext->data))
    {
      fprintf(stderr, "Encrypt failed:\nInput:");
      tstring_print_hex(cleartext);
      fprintf(stderr, "\nOutput: ");
      print_hex(length, data);
      fprintf(stderr, "\nExpected:");
      tstring_print_hex(ciphertext);
      fprintf(stderr, "\n");
      FAIL();
    }
  cipher->set_decrypt_key(ctx, key->data);
  cipher->decrypt(ctx, length, data, data);

  if (!MEMEQ(length, data, cleartext->data))
    {
      fprintf(stderr, "Decrypt failed:\nInput:");
      tstring_print_hex(ciphertext);
      fprintf(stderr, "\nOutput: ");
      print_hex(length, data);
      fprintf(stderr, "\nExpected:");
      tstring_print_hex(cleartext);
      fprintf(stderr, "\n");
      FAIL();
    }

  free(ctx);
  free(data);
}

void
test_cipher_cbc(const struct nettle_cipher *cipher,
		const struct tstring *key,
		const struct tstring *cleartext,
		const struct tstring *ciphertext,
		const struct tstring *iiv)
{
  void *ctx = xalloc(cipher->context_size);
  uint8_t *data;
  uint8_t *iv = xalloc(cipher->block_size);
  size_t length;

  ASSERT (cleartext->length == ciphertext->length);
  length = cleartext->length;

  ASSERT (key->length == cipher->key_size);
  ASSERT (iiv->length == cipher->block_size);

  data = xalloc(length);  
  cipher->set_encrypt_key(ctx, key->data);
  memcpy(iv, iiv->data, cipher->block_size);

  cbc_encrypt(ctx, cipher->encrypt,
	      cipher->block_size, iv,
	      length, data, cleartext->data);

  if (!MEMEQ(length, data, ciphertext->data))
    {
      fprintf(stderr, "CBC encrypt failed:\nInput:");
      tstring_print_hex(cleartext);
      fprintf(stderr, "\nOutput: ");
      print_hex(length, data);
      fprintf(stderr, "\nExpected:");
      tstring_print_hex(ciphertext);
      fprintf(stderr, "\n");
      FAIL();
    }
  cipher->set_decrypt_key(ctx, key->data);
  memcpy(iv, iiv->data, cipher->block_size);

  cbc_decrypt(ctx, cipher->decrypt,
	      cipher->block_size, iv,
	      length, data, data);

  if (!MEMEQ(length, data, cleartext->data))
    {
      fprintf(stderr, "CBC decrypt failed:\nInput:");
      tstring_print_hex(ciphertext);
      fprintf(stderr, "\nOutput: ");
      print_hex(length, data);
      fprintf(stderr, "\nExpected:");
      tstring_print_hex(cleartext);
      fprintf(stderr, "\n");
      FAIL();
    }

  free(ctx);
  free(data);
  free(iv);
}

void
test_cipher_cfb(const struct nettle_cipher *cipher,
		const struct tstring *key,
		const struct tstring *cleartext,
		const struct tstring *ciphertext,
		const struct tstring *iiv)
{
  void *ctx = xalloc(cipher->context_size);
  uint8_t *data, *data2;
  uint8_t *iv = xalloc(cipher->block_size);
  size_t length;

  ASSERT (cleartext->length == ciphertext->length);
  length = cleartext->length;

  ASSERT (key->length == cipher->key_size);
  ASSERT (iiv->length == cipher->block_size);

  data = xalloc(length);
  data2 = xalloc(length);

  cipher->set_encrypt_key(ctx, key->data);
  memcpy(iv, iiv->data, cipher->block_size);

  cfb_encrypt(ctx, cipher->encrypt,
	      cipher->block_size, iv,
	      length, data, cleartext->data);

  if (!MEMEQ(length, data, ciphertext->data))
    {
      fprintf(stderr, "CFB encrypt failed:\nInput:");
      tstring_print_hex(cleartext);
      fprintf(stderr, "\nOutput: ");
      print_hex(length, data);
      fprintf(stderr, "\nExpected:");
      tstring_print_hex(ciphertext);
      fprintf(stderr, "\n");
      FAIL();
    }
  cipher->set_encrypt_key(ctx, key->data);
  memcpy(iv, iiv->data, cipher->block_size);

  cfb_decrypt(ctx, cipher->encrypt,
	      cipher->block_size, iv,
	      length, data2, data);

  if (!MEMEQ(length, data2, cleartext->data))
    {
      fprintf(stderr, "CFB decrypt failed:\nInput:");
      tstring_print_hex(ciphertext);
      fprintf(stderr, "\nOutput: ");
      print_hex(length, data2);
      fprintf(stderr, "\nExpected:");
      tstring_print_hex(cleartext);
      fprintf(stderr, "\n");
      FAIL();
    }
  cipher->set_encrypt_key(ctx, key->data);
  memcpy(iv, iiv->data, cipher->block_size);
  memcpy(data, cleartext->data, length);

  cfb_encrypt(ctx, cipher->encrypt,
	      cipher->block_size, iv,
	      length, data, data);

  if (!MEMEQ(length, data, ciphertext->data))
    {
      fprintf(stderr, "CFB inplace encrypt failed:\nInput:");
      tstring_print_hex(cleartext);
      fprintf(stderr, "\nOutput: ");
      print_hex(length, data);
      fprintf(stderr, "\nExpected:");
      tstring_print_hex(ciphertext);
      fprintf(stderr, "\n");
      FAIL();
    }
  cipher->set_encrypt_key(ctx, key->data);
  memcpy(iv, iiv->data, cipher->block_size);

  cfb_decrypt(ctx, cipher->encrypt,
	      cipher->block_size, iv,
	      length, data, data);

  if (!MEMEQ(length, data, cleartext->data))
    {
      fprintf(stderr, "CFB inplace decrypt failed:\nInput:");
      tstring_print_hex(ciphertext);
      fprintf(stderr, "\nOutput: ");
      print_hex(length, data);
      fprintf(stderr, "\nExpected:");
      tstring_print_hex(cleartext);
      fprintf(stderr, "\n");
      FAIL();
    }

  /* Repeat all tests with incomplete last block */
  length -= 1;

  cipher->set_encrypt_key(ctx, key->data);
  memcpy(iv, iiv->data, cipher->block_size);

  cfb_encrypt(ctx, cipher->encrypt,
	      cipher->block_size, iv,
	      length, data, cleartext->data);

  if (!MEMEQ(length, data, ciphertext->data))
    {
      fprintf(stderr, "CFB encrypt failed:\nInput:");
      print_hex(length, cleartext->data);
      fprintf(stderr, "\nOutput: ");
      print_hex(length, data);
      fprintf(stderr, "\nExpected:");
      print_hex(length, ciphertext->data);
      fprintf(stderr, "\n");
      FAIL();
    }
  cipher->set_encrypt_key(ctx, key->data);
  memcpy(iv, iiv->data, cipher->block_size);

  cfb_decrypt(ctx, cipher->encrypt,
	      cipher->block_size, iv,
	      length, data2, data);

  if (!MEMEQ(length, data2, cleartext->data))
    {
      fprintf(stderr, "CFB decrypt failed:\nInput:");
      print_hex(length, ciphertext->data);
      fprintf(stderr, "\nOutput: ");
      print_hex(length, data2);
      fprintf(stderr, "\nExpected:");
      print_hex(length, cleartext->data);
      fprintf(stderr, "\n");
      FAIL();
    }
  cipher->set_encrypt_key(ctx, key->data);
  memcpy(iv, iiv->data, cipher->block_size);
  memcpy(data, cleartext->data, length);

  cfb_encrypt(ctx, cipher->encrypt,
	      cipher->block_size, iv,
	      length, data, data);

  if (!MEMEQ(length, data, ciphertext->data))
    {
      fprintf(stderr, "CFB inplace encrypt failed:\nInput:");
      print_hex(length, cleartext->data);
      fprintf(stderr, "\nOutput: ");
      print_hex(length, data);
      fprintf(stderr, "\nExpected:");
      print_hex(length, ciphertext->data);
      fprintf(stderr, "\n");
      FAIL();
    }
  cipher->set_encrypt_key(ctx, key->data);
  memcpy(iv, iiv->data, cipher->block_size);

  cfb_decrypt(ctx, cipher->encrypt,
	      cipher->block_size, iv,
	      length, data, data);

  if (!MEMEQ(length, data, cleartext->data))
    {
      fprintf(stderr, "CFB inplace decrypt failed:\nInput:");
      print_hex(length, ciphertext->data);
      fprintf(stderr, "\nOutput: ");
      print_hex(length, data);
      fprintf(stderr, "\nExpected:");
      print_hex(length, cleartext->data);
      fprintf(stderr, "\n");
      FAIL();
    }

  free(ctx);
  free(data);
  free(data2);
  free(iv);
}

void
test_cipher_cfb8(const struct nettle_cipher *cipher,
		 const struct tstring *key,
		 const struct tstring *cleartext,
		 const struct tstring *ciphertext,
		 const struct tstring *iiv)
{
  void *ctx = xalloc(cipher->context_size);
  uint8_t *data, *data2;
  uint8_t *iv = xalloc(cipher->block_size);
  size_t length;
  size_t block;

  ASSERT (cleartext->length == ciphertext->length);
  length = cleartext->length;

  ASSERT (key->length == cipher->key_size);
  ASSERT (iiv->length == cipher->block_size);

  data = xalloc(length + 1);
  data2 = xalloc(length + 1);

  for (block = 1; block <= length; block++)
    {
      size_t i;

      cipher->set_encrypt_key(ctx, key->data);
      memcpy(iv, iiv->data, cipher->block_size);

      memset(data, 0x17, length + 1);
      for (i = 0; i + block <= length; i += block)
	{
	  cfb8_encrypt(ctx, cipher->encrypt,
		       cipher->block_size, iv,
		       block, data + i, cleartext->data + i);
	}
      cfb8_encrypt(ctx, cipher->encrypt,
		   cipher->block_size, iv,
		   length - i, data + i, cleartext->data + i);

      if (!MEMEQ(length, data, ciphertext->data))
	{
	  fprintf(stderr, "CFB8 encrypt failed, block size %lu:\nInput:",
		  (unsigned long) block);
	  tstring_print_hex(cleartext);
	  fprintf(stderr, "\nOutput: ");
	  print_hex(length, data);
	  fprintf(stderr, "\nExpected:");
	  tstring_print_hex(ciphertext);
	  fprintf(stderr, "\n");
	  FAIL();
	}
      ASSERT (data[length] == 0x17);

      cipher->set_encrypt_key(ctx, key->data);
      memcpy(iv, iiv->data, cipher->block_size);

      memset(data2, 0x17, length + 1);
      for (i = 0; i + block <= length; i += block)
	{
	  cfb8_decrypt(ctx, cipher->encrypt,
		       cipher->block_size, iv,
		       block, data2 + i, data + i);
	}
      cfb8_decrypt(ctx, cipher->encrypt,
		   cipher->block_size, iv,
		   length - i, data2 + i, data + i);

      if (!MEMEQ(length, data2, cleartext->data))
	{
	  fprintf(stderr, "CFB8 decrypt failed, block size %lu:\nInput:",
		  (unsigned long) block);
	  tstring_print_hex(ciphertext);
	  fprintf(stderr, "\nOutput: ");
	  print_hex(length, data2);
	  fprintf(stderr, "\nExpected:");
	  tstring_print_hex(cleartext);
	  fprintf(stderr, "\n");
	  FAIL();
	}
      ASSERT (data[length] == 0x17);
    }

  cipher->set_encrypt_key(ctx, key->data);
  memcpy(iv, iiv->data, cipher->block_size);
  memcpy(data, cleartext->data, length);

  cfb8_encrypt(ctx, cipher->encrypt,
	      cipher->block_size, iv,
	      length, data, data);

  if (!MEMEQ(length, data, ciphertext->data))
    {
      fprintf(stderr, "CFB8 inplace encrypt failed:\nInput:");
      tstring_print_hex(cleartext);
      fprintf(stderr, "\nOutput: ");
      print_hex(length, data);
      fprintf(stderr, "\nExpected:");
      tstring_print_hex(ciphertext);
      fprintf(stderr, "\n");
      FAIL();
    }
  cipher->set_encrypt_key(ctx, key->data);
  memcpy(iv, iiv->data, cipher->block_size);

  cfb8_decrypt(ctx, cipher->encrypt,
	      cipher->block_size, iv,
	      length, data, data);

  if (!MEMEQ(length, data, cleartext->data))
    {
      fprintf(stderr, "CFB8 inplace decrypt failed:\nInput:");
      tstring_print_hex(ciphertext);
      fprintf(stderr, "\nOutput: ");
      print_hex(length, data);
      fprintf(stderr, "\nExpected:");
      tstring_print_hex(cleartext);
      fprintf(stderr, "\n");
      FAIL();
    }

  /* Repeat all tests with incomplete last block */
  length -= 1;

  cipher->set_encrypt_key(ctx, key->data);
  memcpy(iv, iiv->data, cipher->block_size);

  cfb8_encrypt(ctx, cipher->encrypt,
	      cipher->block_size, iv,
	      length, data, cleartext->data);

  if (!MEMEQ(length, data, ciphertext->data))
    {
      fprintf(stderr, "CFB8 encrypt failed:\nInput:");
      print_hex(length, cleartext->data);
      fprintf(stderr, "\nOutput: ");
      print_hex(length, data);
      fprintf(stderr, "\nExpected:");
      print_hex(length, ciphertext->data);
      fprintf(stderr, "\n");
      FAIL();
    }
  cipher->set_encrypt_key(ctx, key->data);
  memcpy(iv, iiv->data, cipher->block_size);

  cfb8_decrypt(ctx, cipher->encrypt,
	      cipher->block_size, iv,
	      length, data2, data);

  if (!MEMEQ(length, data2, cleartext->data))
    {
      fprintf(stderr, "CFB8 decrypt failed:\nInput:");
      print_hex(length, ciphertext->data);
      fprintf(stderr, "\nOutput: ");
      print_hex(length, data2);
      fprintf(stderr, "\nExpected:");
      print_hex(length, cleartext->data);
      fprintf(stderr, "\n");
      FAIL();
    }
  cipher->set_encrypt_key(ctx, key->data);
  memcpy(iv, iiv->data, cipher->block_size);
  memcpy(data, cleartext->data, length);

  cfb8_encrypt(ctx, cipher->encrypt,
	      cipher->block_size, iv,
	      length, data, data);

  if (!MEMEQ(length, data, ciphertext->data))
    {
      fprintf(stderr, "CFB8 inplace encrypt failed:\nInput:");
      print_hex(length, cleartext->data);
      fprintf(stderr, "\nOutput: ");
      print_hex(length, data);
      fprintf(stderr, "\nExpected:");
      print_hex(length, ciphertext->data);
      fprintf(stderr, "\n");
      FAIL();
    }
  cipher->set_encrypt_key(ctx, key->data);
  memcpy(iv, iiv->data, cipher->block_size);

  cfb8_decrypt(ctx, cipher->encrypt,
	      cipher->block_size, iv,
	      length, data, data);

  if (!MEMEQ(length, data, cleartext->data))
    {
      fprintf(stderr, "CFB8 inplace decrypt failed:\nInput:");
      print_hex(length, ciphertext->data);
      fprintf(stderr, "\nOutput: ");
      print_hex(length, data);
      fprintf(stderr, "\nExpected:");
      print_hex(length, cleartext->data);
      fprintf(stderr, "\n");
      FAIL();
    }

  free(ctx);
  free(data);
  free(data2);
  free(iv);
}

void
test_cipher_ctr(const struct nettle_cipher *cipher,
		const struct tstring *key,
		const struct tstring *cleartext,
		const struct tstring *ciphertext,
		const struct tstring *ictr)
{
  void *ctx = xalloc(cipher->context_size);
  uint8_t *data;
  uint8_t *ctr = xalloc(cipher->block_size);
  uint8_t *octr = xalloc(cipher->block_size);
  size_t length, nblocks;
  unsigned low;
  size_t i;

  ASSERT (cleartext->length == ciphertext->length);
  length = cleartext->length;

  ASSERT (key->length == cipher->key_size);
  ASSERT (ictr->length == cipher->block_size);

  /* Compute expected counter value after the operation. */
  nblocks = (length + cipher->block_size - 1) / cipher->block_size;
  ASSERT (nblocks < 0x100);

  memcpy (octr, ictr->data, cipher->block_size - 1);
  low = ictr->data[cipher->block_size - 1] + nblocks;
  octr[cipher->block_size - 1] = low;

  if (low >= 0x100)
    INCREMENT (cipher->block_size - 1, octr);

  data = xalloc(length);  

  cipher->set_encrypt_key(ctx, key->data);

  for (i = 0; i <= length; i++)
    {
      memcpy(ctr, ictr->data, cipher->block_size);
      memset(data, 17, length);

      ctr_crypt(ctx, cipher->encrypt,
		cipher->block_size, ctr,
		i, data, cleartext->data);

      if (!MEMEQ(i, data, ciphertext->data)
	  || (i < length && data[i] != 17))
	{
	  fprintf(stderr, "CTR encrypt failed (length %d of %d):\nInput:",
		  (int) i, (int) length);
	  tstring_print_hex(cleartext);
	  fprintf(stderr, "\nOutput: ");
	  print_hex(length, data);
	  fprintf(stderr, "\nExpected:");
	  tstring_print_hex(ciphertext);
	  fprintf(stderr, "\n");
	  FAIL();
	}
    }

  ASSERT (MEMEQ (cipher->block_size, ctr, octr));

  memcpy(ctr, ictr->data, cipher->block_size);

  ctr_crypt(ctx, cipher->encrypt,
	    cipher->block_size, ctr,
	    length, data, data);

  if (!MEMEQ(length, data, cleartext->data))
    {
      fprintf(stderr, "CTR decrypt failed:\nInput:");
      tstring_print_hex(ciphertext);
      fprintf(stderr, "\nOutput: ");
      print_hex(length, data);
      fprintf(stderr, "\nExpected:");
      tstring_print_hex(cleartext);
      fprintf(stderr, "\n");
      FAIL();
    }

  ASSERT (MEMEQ (cipher->block_size, ctr, octr));

  free(ctx);
  free(data);
  free(octr);
  free(ctr);
}

#if 0
void
test_cipher_stream(const struct nettle_cipher *cipher,
		   const struct tstring *key,
		   const struct tstring *cleartext,
		   const struct tstring *ciphertext)
{
  size_t block;
  
  void *ctx = xalloc(cipher->context_size);
  uint8_t *data;
  size_t length;

  ASSERT (cleartext->length == ciphertext->length);
  length = cleartext->length;

  data = xalloc(length + 1);

  for (block = 1; block <= length; block++)
    {
      size_t i;

      memset(data, 0x17, length + 1);
      cipher->set_encrypt_key(ctx, key->length, key->data);

      for (i = 0; i + block < length; i += block)
	{
	  cipher->encrypt(ctx, block, data + i, cleartext->data + i);
	  ASSERT (data[i + block] == 0x17);
	}

      cipher->encrypt(ctx, length - i, data + i, cleartext->data + i);
      ASSERT (data[length] == 0x17);
      
      if (!MEMEQ(length, data, ciphertext->data))
	{
	  fprintf(stderr, "Encrypt failed, block size %lu\nInput:",
		  (unsigned long) block);
	  tstring_print_hex(cleartext);
	  fprintf(stderr, "\nOutput: ");
	  print_hex(length, data);
	  fprintf(stderr, "\nExpected:");
	  tstring_print_hex(ciphertext);
	  fprintf(stderr, "\n");
	  FAIL();	    
	}
    }
  
  cipher->set_decrypt_key(ctx, key->length, key->data);
  cipher->decrypt(ctx, length, data, data);

  ASSERT (data[length] == 0x17);

  if (!MEMEQ(length, data, cleartext->data))
    {
      fprintf(stderr, "Decrypt failed\nInput:");
      tstring_print_hex(ciphertext);
      fprintf(stderr, "\nOutput: ");
      print_hex(length, data);
      fprintf(stderr, "\nExpected:");
      tstring_print_hex(cleartext);
      fprintf(stderr, "\n");
      FAIL();	    
    }

  free(ctx);
  free(data);
}
#endif

void
test_aead(const struct nettle_aead *aead,
	  nettle_hash_update_func *set_nonce,
	  const struct tstring *key,
	  const struct tstring *authtext,
	  const struct tstring *cleartext,
	  const struct tstring *ciphertext,
	  const struct tstring *nonce,
	  const struct tstring *digest)
{
  void *ctx = xalloc(aead->context_size);
  uint8_t *in, *out;
  uint8_t *buffer;
  unsigned in_align;

  ASSERT (cleartext->length == ciphertext->length);
  ASSERT (key->length == aead->key_size);
  ASSERT(aead->block_size > 0);

  buffer = xalloc(aead->digest_size);
  in = xalloc(cleartext->length + aead->block_size - 1);
  out = xalloc(cleartext->length + aead->block_size - 1);

  for (in_align = 0; in_align < aead->block_size; in_align++)
    {
      /* Different alignment, but don't try all combinations. */
      unsigned out_align = 3*in_align % aead->block_size;
      size_t offset;
      memcpy (in + in_align, cleartext->data, cleartext->length);
      for (offset = 0; offset <= cleartext->length; offset += aead->block_size)
	{
	  /* encryption */
	  aead->set_encrypt_key(ctx, key->data);

	  if (set_nonce)
	      set_nonce (ctx, nonce->length, nonce->data);
	  else
	    {
	      assert (nonce->length == aead->nonce_size);
	      aead->set_nonce(ctx, nonce->data);
	    }
	  if (aead->update)
	    {
	      size_t a_offset = (offset <= authtext->length) ? offset : 0;
	      aead->update(ctx, a_offset, authtext->data);
	      aead->update(ctx, 0, NULL);
	      aead->update(ctx, authtext->length - a_offset, authtext->data + a_offset);
	    }
	  aead->encrypt(ctx, offset, out + out_align, in + in_align);
	  aead->encrypt(ctx, 0, out + out_align, NULL);
	  aead->encrypt(ctx, cleartext->length - offset,
			out + out_align + offset, in + in_align + offset);

	  if (!MEMEQ(cleartext->length, out + out_align, ciphertext->data))
	    {
	      fprintf(stderr, "aead->encrypt failed (offset = %u):\nclear: ",
		      (unsigned) offset);
	      tstring_print_hex(cleartext);
	      fprintf(stderr, "  got: ");
	      print_hex(cleartext->length, out + out_align);
	      fprintf(stderr, "  exp: ");
	      tstring_print_hex(ciphertext);
	      FAIL();
	    }
	  if (digest)
	    {
	      ASSERT (digest->length <= aead->digest_size);
	      memset(buffer, 0, aead->digest_size);
	      aead->digest(ctx, digest->length, buffer);
	      if (!MEMEQ(digest->length, buffer, digest->data))
		{
		  fprintf(stderr, "aead->digest failed (offset = %u):\n  got: ",
			  (unsigned) offset);
		  print_hex(digest->length, buffer);
		  fprintf(stderr, "  exp: ");
		  tstring_print_hex(digest);
		  FAIL();
		}
	    }
	  else
	    ASSERT(!aead->digest);

	  /* decryption */
	  if (aead->set_decrypt_key)
	    {
	      aead->set_decrypt_key(ctx, key->data);

	      if (set_nonce)
		set_nonce (ctx, nonce->length, nonce->data);
	      else
		{
		  assert (nonce->length == aead->nonce_size);
		  aead->set_nonce(ctx, nonce->data);
		}

	      if (aead->update && authtext->length)
		aead->update(ctx, authtext->length, authtext->data);

	      aead->decrypt(ctx, offset, out + out_align, out + out_align);
	      aead->decrypt(ctx, 0, out + out_align, NULL);
	      aead->decrypt(ctx, cleartext->length - offset,
			    out + out_align + offset, out + out_align + offset);

	      ASSERT(MEMEQ(cleartext->length, out + out_align, cleartext->data));

	      if (digest)
		{
		  memset(buffer, 0, aead->digest_size);
		  aead->digest(ctx, digest->length, buffer);
		  ASSERT(MEMEQ(digest->length, buffer, digest->data));
		}
	    }
	}
    }
  free(ctx);
  free(in);
  free(out);
  free(buffer);
}

void
test_aead_message (const struct nettle_aead_message *aead,
		   const struct tstring *key,
		   const struct tstring *nonce,
		   const struct tstring *adata,
		   const struct tstring *clear,
		   const struct tstring *cipher)
{
  void *ctx = xalloc (aead->context_size);
  uint8_t *buf = xalloc (cipher->length + 1);
  uint8_t *copy = xalloc (cipher->length);

  static const uint8_t nul = 0;
  int res;

  ASSERT (key->length == aead->key_size);
  ASSERT (cipher->length > clear->length);
  ASSERT (cipher->length - clear->length == aead->digest_size);

  aead->set_encrypt_key (ctx, key->data);
  buf[cipher->length] = 0xae;
  aead->encrypt (ctx,
		 nonce->length, nonce->data,
		 adata->length, adata->data,
		 cipher->length, buf, clear->data);
  if (!MEMEQ (cipher->length, cipher->data, buf))
    {
      fprintf(stderr, "aead->encrypt (message) failed:\n  got: ");
      print_hex (cipher->length, buf);
      fprintf (stderr, "  exp: ");
      tstring_print_hex (cipher);
      FAIL();
    }
  if (buf[cipher->length] != 0xae)
    {
      fprintf (stderr, "aead->encrypt (message) wrote too much.\n ");
      FAIL();
    }
  aead->set_decrypt_key (ctx, key->data);

  memset (buf, 0xae, clear->length + 1);

  res = aead->decrypt (ctx,
		       nonce->length, nonce->data,
		       adata->length, adata->data,
		       clear->length, buf, cipher->data);
  if (!res)
    {
      fprintf (stderr, "decrypting valid ciphertext failed:\n  ");
      tstring_print_hex (cipher);
    }
  if (!MEMEQ (clear->length, clear->data, buf))
    {
      fprintf(stderr, "aead->decrypt (message) failed:\n  got: ");
      print_hex (clear->length, buf);
      fprintf (stderr, "  exp: ");
      tstring_print_hex (clear);
      FAIL();
    }

  /* Invalid messages */
  if (clear->length > 0
      && aead->decrypt (ctx,
			nonce->length, nonce->data,
			adata->length, adata->data,
			clear->length - 1, buf, cipher->data))
    {
      fprintf (stderr, "Invalid message (truncated) not rejected\n");
      FAIL();
    }
  memcpy (copy, cipher->data, cipher->length);
  copy[0] ^= 4;
  if (aead->decrypt (ctx,
		     nonce->length, nonce->data,
		     adata->length, adata->data,
		     clear->length, buf, copy))
    {
      fprintf (stderr, "Invalid message (first byte modified) not rejected\n");
      FAIL();
    }

  memcpy (copy, cipher->data, cipher->length);
  copy[cipher->length - 1] ^= 4;
  if (aead->decrypt (ctx,
		     nonce->length, nonce->data,
		     adata->length, adata->data,
		     clear->length, buf, copy))
    {
      fprintf (stderr, "Invalid message (last byte modified) not rejected\n");
      FAIL();
    }

  if (aead->decrypt (ctx,
		     nonce->length, nonce->data,
		     adata->length > 0 ? adata->length - 1 : 1,
		     adata->length > 0 ? adata->data : &nul,
		     clear->length, buf, cipher->data))
    {
      fprintf (stderr, "Invalid adata not rejected\n");
      FAIL();
    }

  /* Test in-place operation. NOTE: Not supported for SIV-CMAC. */
  if (aead->supports_inplace)
    {
      aead->set_encrypt_key (ctx, key->data);
      buf[cipher->length] = 0xae;

      memcpy (buf, clear->data, clear->length);
      aead->encrypt (ctx,
		     nonce->length, nonce->data,
		     adata->length, adata->data,
		     cipher->length, buf, buf);
      if (!MEMEQ (cipher->length, cipher->data, buf))
	{
	  fprintf(stderr, "aead->encrypt (in-place message) failed:\n  got: ");
	  print_hex (cipher->length, buf);
	  fprintf (stderr, "  exp: ");
	  tstring_print_hex (cipher);
	  FAIL();
	}
      if (buf[cipher->length] != 0xae)
	{
	  fprintf (stderr, "aead->encrypt (in-place message) wrote too much.\n ");
	  FAIL();
	}

      res = aead->decrypt (ctx,
			   nonce->length, nonce->data,
			   adata->length, adata->data,
			   clear->length, buf, buf);
      if (!res)
	{
	  fprintf (stderr, "in-place decrypting valid ciphertext failed:\n  ");
	  tstring_print_hex (cipher);
	}
      if (!MEMEQ (clear->length, clear->data, buf))
	{
	  fprintf(stderr, "aead->decrypt (in-place message) failed:\n  got: ");
	  print_hex (clear->length, buf);
	  fprintf (stderr, "  exp: ");
	  tstring_print_hex (clear);
	  FAIL();
	}
    }
  free (ctx);
  free (buf);
  free (copy);
}

void
test_hash(const struct nettle_hash *hash,
	  const struct tstring *msg,
	  const struct tstring *digest)
{
  void *ctx = xalloc(hash->context_size);
  uint8_t *buffer = xalloc(digest->length);
  uint8_t *input;
  unsigned offset;

  ASSERT (digest->length == hash->digest_size);

  hash->init(ctx);
  for (offset = 0; offset <= msg->length && offset < 40; offset++)
    {
      hash->update(ctx, offset, msg->data);
      hash->update(ctx, 0, NULL);
      hash->update(ctx, msg->length - offset, msg->data + offset);

      hash->digest(ctx, digest->length, buffer);

      if (MEMEQ(digest->length, digest->data, buffer) == 0)
	{
	  fprintf(stdout, "Offset %u\nGot:\n", offset);
	  print_hex(digest->length, buffer);
	  fprintf(stdout, "\nExpected:\n");
	  print_hex(digest->length, digest->data);
	  abort();
	}
    }

  memset(buffer, 0, digest->length);

  hash->update(ctx, msg->length, msg->data);
  ASSERT(digest->length > 0);
  hash->digest(ctx, digest->length - 1, buffer);

  ASSERT(MEMEQ(digest->length - 1, digest->data, buffer));

  ASSERT(buffer[digest->length - 1] == 0);

  input = xalloc (msg->length + 16);
  for (offset = 0; offset < 16; offset++)
    {
      memset (input, 0, msg->length + 16);
      memcpy (input + offset, msg->data, msg->length);
      hash->update (ctx, msg->length, input + offset);
      hash->digest (ctx, digest->length, buffer);
      if (MEMEQ(digest->length, digest->data, buffer) == 0)
	{
	  fprintf(stdout, "hash input address: %p\nGot:\n", input + offset);
	  print_hex(digest->length, buffer);
	  fprintf(stdout, "\nExpected:\n");
	  print_hex(digest->length, digest->data);
	  abort();
	}      
    }
  free(ctx);
  free(buffer);
  free(input);
}

void
test_xof (const struct nettle_xof *xof,
	  const struct tstring *msg,
	  const struct tstring *digest)
{
  void *ctx = xalloc (xof->context_size);
  uint8_t *buffer = xalloc (digest->length);
  uint8_t *input;
  unsigned offset, size;

  xof->init (ctx);
  for (offset = 0; offset <= msg->length && offset < 40; offset++)
    {
      xof->update (ctx, offset, msg->data);
      xof->update (ctx, 0, NULL);
      xof->update (ctx, msg->length - offset, msg->data + offset);

      xof->digest (ctx, digest->length, buffer);

      if (MEMEQ (digest->length, digest->data, buffer) == 0)
	{
	  fprintf (stdout, "Offset %u\nGot:\n", offset);
	  print_hex (digest->length, buffer);
	  fprintf (stdout, "\nExpected:\n");
	  print_hex (digest->length, digest->data);
	  abort ();
	}
    }

  input = xalloc (msg->length + 16);
  for (offset = 0; offset < 16; offset++)
    {
      memset (input, 0, msg->length + 16);
      memcpy (input + offset, msg->data, msg->length);
      xof->update (ctx, msg->length, input + offset);
      xof->digest (ctx, digest->length, buffer);
      if (MEMEQ(digest->length, digest->data, buffer) == 0)
	{
	  fprintf(stdout, "xof input address: %p\nGot:\n", input + offset);
	  print_hex(digest->length, buffer);
	  fprintf(stdout, "\nExpected:\n");
	  print_hex(digest->length, digest->data);
	  abort();
	}
    }

  /* Test producing output incrementally. */
  for (size = 1; size <= xof->block_size + 2; )
    {
      xof->init (ctx);
      xof->update (ctx, msg->length, msg->data);
      memset (buffer, 0, digest->length);

      for (offset = 0; offset + size < digest->length; offset += size)
	{
	  xof->output (ctx, size, buffer + offset);

	  ASSERT (MEMEQ (offset + size, digest->data, buffer));
	  ASSERT (buffer[offset + size] == 0);
	}
      xof->output (ctx, digest->length - offset, buffer + offset);

      if (MEMEQ(digest->length, digest->data, buffer) == 0)
	{
	  fprintf(stdout, "xof output increment: %u\nGot:\n", size);
	  print_hex(digest->length, buffer);
	  fprintf(stdout, "\nExpected:\n");
	  print_hex(digest->length, digest->data);
	  abort();
	}
      /* Try sizes 1, 2, 4, 7, 11, ..., and sizes around the block size. */
      if (size < xof->block_size - 2)
	{
	  size += 1 + size/2;
	  if (size > xof->block_size - 2)
	    size = xof->block_size - 2;
	}
      else
	size++;
    }
  free(ctx);
  free(buffer);
  free(input);
}

void
test_hash_large(const struct nettle_hash *hash,
		size_t count, size_t length,
		uint8_t c,
		const struct tstring *digest)
{
  void *ctx = xalloc(hash->context_size);
  uint8_t *buffer = xalloc(hash->digest_size);
  uint8_t *data = xalloc(length);
  size_t i;

  ASSERT (digest->length == hash->digest_size);

  memset(data, c, length);

  hash->init(ctx);
  for (i = 0; i < count; i++)
    {
      hash->update(ctx, length, data);
      if (i % (count / 50) == 0)
	fprintf (stderr, ".");
    }
  fprintf (stderr, "\n");
  
  hash->digest(ctx, hash->digest_size, buffer);

  print_hex(hash->digest_size, buffer);

  ASSERT (MEMEQ(hash->digest_size, digest->data, buffer));

  free(ctx);
  free(buffer);
  free(data);
}

void
test_mac(const struct nettle_mac *mac,
	 nettle_hash_update_func *set_key,
	 const struct tstring *key,
	 const struct tstring *msg,
	 const struct tstring *digest)
{
  void *ctx = xalloc(mac->context_size);
  uint8_t *hash = xalloc(mac->digest_size);
  unsigned i;

  ASSERT (digest->length <= mac->digest_size);
  if (set_key)
    set_key (ctx, key->length, key->data);
  else
    {
      ASSERT (key->length == mac->key_size);
      mac->set_key (ctx, key->data);
    }
  mac->update (ctx, msg->length, msg->data);
  mac->digest (ctx, digest->length, hash);

  if (!MEMEQ (digest->length, digest->data, hash))
    {
      fprintf (stderr, "test_mac failed, msg: ");
      print_hex (msg->length, msg->data);
      fprintf(stderr, "Output:");
      print_hex (mac->digest_size, hash);
      fprintf(stderr, "Expected:");
      tstring_print_hex(digest);
      fprintf(stderr, "\n");
      FAIL();
    }

  /* attempt to re-use the structure */
  mac->update (ctx, msg->length, msg->data);
  mac->digest (ctx, digest->length, hash);
  if (!MEMEQ (digest->length, digest->data, hash))
    {
      fprintf (stderr, "test_mac: failed on re-use, msg: ");
      print_hex (msg->length, msg->data);
      fprintf(stderr, "Output:");
      print_hex (mac->digest_size, hash);
      fprintf(stderr, "Expected:");
      tstring_print_hex(digest);
      fprintf(stderr, "\n");
      FAIL();
    }

  /* attempt byte-by-byte hashing */
  for (i=0;i<msg->length;i++)
    mac->update (ctx, 1, msg->data+i);
  mac->digest (ctx, digest->length, hash);
  if (!MEMEQ (digest->length, digest->data, hash))
    {
      fprintf (stderr, "test_mac failed on byte-by-byte, msg: ");
      print_hex (msg->length, msg->data);
      fprintf(stderr, "Output:");
      print_hex (16, hash);
      fprintf(stderr, "Expected:");
      tstring_print_hex(digest);
      fprintf(stderr, "\n");
      FAIL();
    }
  free (ctx);
  free (hash);
}

void
test_armor(const struct nettle_armor *armor,
           size_t data_length,
           const uint8_t *data,
           const char *ascii)
{
  size_t ascii_length = strlen(ascii);
  char *buffer = xalloc(1 + ascii_length);
  uint8_t *check = xalloc(1 + armor->decode_length(ascii_length));
  void *encode = xalloc(armor->encode_context_size);
  void *decode = xalloc(armor->decode_context_size);
  size_t done;

  ASSERT(ascii_length
	 <= (armor->encode_length(data_length) + armor->encode_final_length));
  ASSERT(data_length <= armor->decode_length(ascii_length));
  
  memset(buffer, 0x33, 1 + ascii_length);
  memset(check, 0x55, 1 + data_length);

  armor->encode_init(encode);
  
  done = armor->encode_update(encode, buffer, data_length, data);
  done += armor->encode_final(encode, buffer + done);
  ASSERT(done == ascii_length);

  ASSERT (MEMEQ(ascii_length, buffer, ascii));
  ASSERT (0x33 == buffer[strlen(ascii)]);

  armor->decode_init(decode);
  done = armor->decode_length(ascii_length);

  ASSERT(armor->decode_update(decode, &done, check, ascii_length, buffer));
  ASSERT(done == data_length);
  ASSERT(armor->decode_final(decode));
  
  ASSERT (MEMEQ(data_length, check, data));
  ASSERT (0x55 == check[data_length]);

  free(buffer);
  free(check);
  free(encode);
  free(decode);
}

#if WITH_HOGWEED

void
mpn_out_str (FILE *f, int base, const mp_limb_t *xp, mp_size_t xn)
{
  mpz_t x;
  mpz_out_str (f, base, mpz_roinit_n (x, xp, xn));
}

#if NETTLE_USE_MINI_GMP
void
gmp_randinit_default (struct knuth_lfib_ctx *ctx)
{
  knuth_lfib_init (ctx, 17);
}
void
mpz_urandomb (mpz_t r, struct knuth_lfib_ctx *ctx, mp_bitcnt_t bits)
{
  size_t bytes = (bits+7)/8;
  uint8_t *buf = xalloc (bytes);

  knuth_lfib_random (ctx, bytes, buf);
  buf[0] &= 0xff >> (8*bytes - bits);
  nettle_mpz_set_str_256_u (r, bytes, buf);
  free (buf);
}
void
mpz_urandomm (mpz_t r, struct knuth_lfib_ctx *ctx, const mpz_t n)
{
  /* Add some extra bits, to make result almost unbiased. */
  mpz_urandomb(r, ctx, mpz_sizeinbase(n, 2) + 30);
  mpz_mod(r, r, n);
}
#else /* !NETTLE_USE_MINI_GMP */
static void
get_random_seed(mpz_t seed)
{
  struct timeval tv;
  FILE *f;
  f = fopen ("/dev/urandom", "rb");
  if (f)
    {
      uint8_t buf[8];
      size_t res;

      setbuf (f, NULL);
      res = fread (&buf, sizeof(buf), 1, f);
      fclose(f);
      if (res == 1)
	{
	  nettle_mpz_set_str_256_u (seed, sizeof(buf), buf);
	  return;
	}
      fprintf (stderr, "Read of /dev/urandom failed: %s\n",
	       strerror (errno));
    }
  gettimeofday(&tv, NULL);
  mpz_set_ui (seed, tv.tv_sec);
  mpz_mul_ui (seed, seed, 1000000UL);
  mpz_add_ui (seed, seed, tv.tv_usec);
}

int
test_randomize(gmp_randstate_t rands)
{
  const char *nettle_test_seed;

  nettle_test_seed = getenv ("NETTLE_TEST_SEED");
  if (nettle_test_seed && *nettle_test_seed)
    {
      mpz_t seed;
      mpz_init (seed);
      if (mpz_set_str (seed, nettle_test_seed, 0) < 0
	  || mpz_sgn (seed) < 0)
	die ("Invalid NETTLE_TEST_SEED: %s\n",
	     nettle_test_seed);
      if (mpz_sgn (seed) == 0)
	get_random_seed (seed);
      fprintf (stderr, "Using NETTLE_TEST_SEED=");
      mpz_out_str (stderr, 10, seed);
      fprintf (stderr, "\n");

      gmp_randseed (rands, seed);
      mpz_clear (seed);
      return 1;
    }
  else 
    return 0;
}
#endif /* !NETTLE_USE_MINI_GMP */

mp_limb_t *
xalloc_limbs (mp_size_t n)
{
  return xalloc (n * sizeof (mp_limb_t));
}

/* Expects local variables pub, key, rstate, digest, signature */
#define SIGN(hash, msg, expected) do { \
  hash##_update(&hash, LDATA(msg));					\
  ASSERT(rsa_##hash##_sign(key, &hash, signature));			\
  if (verbose)								\
    {									\
      fprintf(stderr, "rsa-%s signature: ", #hash);			\
      mpz_out_str(stderr, 16, signature);				\
      fprintf(stderr, "\n");						\
    }									\
  ASSERT(mpz_cmp (signature, expected) == 0);				\
									\
  hash##_update(&hash, LDATA(msg));					\
  ASSERT(rsa_##hash##_sign_tr(pub, key, &rstate,			\
			      (nettle_random_func *) knuth_lfib_random,	\
			      &hash, signature));			\
  ASSERT(mpz_cmp (signature, expected) == 0);				\
									\
  hash##_update(&hash, LDATA(msg));					\
  hash##_digest(&hash, sizeof(digest), digest);				\
  ASSERT(rsa_##hash##_sign_digest(key, digest, signature));		\
  ASSERT(mpz_cmp (signature, expected) == 0);				\
									\
  ASSERT(rsa_##hash##_sign_digest_tr(pub, key, &rstate,			\
				     (nettle_random_func *)knuth_lfib_random, \
				     digest, signature));		\
  ASSERT(mpz_cmp (signature, expected) == 0);				\
} while(0)

#define VERIFY(key, hash, msg, signature) (	\
  hash##_update(&hash, LDATA(msg)),		\
  rsa_##hash##_verify(key, &hash, signature)	\
)

void
test_rsa_set_key_1(struct rsa_public_key *pub,
		   struct rsa_private_key *key)
{
  /* Initialize key pair for test programs */
  /* 1000-bit key, generated by
   *
   *   lsh-keygen -a rsa -l 1000 -f advanced-hex
   *
   * (private-key (rsa-pkcs1 
   *        (n #69abd505285af665 36ddc7c8f027e6f0 ed435d6748b16088
   *            4fd60842b3a8d7fb bd8a3c98f0cc50ae 4f6a9f7dd73122cc
   *            ec8afa3f77134406 f53721973115fc2d 8cfbba23b145f28d
   *            84f81d3b6ae8ce1e 2850580c026e809b cfbb52566ea3a3b3
   *            df7edf52971872a7 e35c1451b8636d22 279a8fb299368238
   *            e545fbb4cf#)
   *        (e #0db2ad57#)
   *        (d #3240a56f4cd0dcc2 4a413eb4ea545259 5c83d771a1c2ba7b
   *            ec47c5b43eb4b374 09bd2aa1e236dd86 481eb1768811412f
   *            f8d91be3545912af b55c014cb55ceac6 54216af3b85d5c4f
   *            4a32894e3b5dfcde 5b2875aa4dc8d9a8 6afd0ca92ef50d35
   *            bd09f1c47efb4c8d c631e07698d362aa 4a83fd304e66d6c5
   *            468863c307#)
   *        (p #0a66399919be4b4d e5a78c5ea5c85bf9 aba8c013cb4a8732
   *            14557a12bd67711e bb4073fd39ad9a86 f4e80253ad809e5b
   *            f2fad3bc37f6f013 273c9552c9f489#)
   *        (q #0a294f069f118625 f5eae2538db9338c 776a298eae953329
   *            9fd1eed4eba04e82 b2593bc98ba8db27 de034da7daaea795
   *            2d55b07b5f9a5875 d1ca5f6dcab897#)
   *        (a #011b6c48eb592eee e85d1bb35cfb6e07 344ea0b5e5f03a28
   *            5b405396cbc78c5c 868e961db160ba8d 4b984250930cf79a
   *            1bf8a9f28963de53 128aa7d690eb87#)
   *        (b #0409ecf3d2557c88 214f1af5e1f17853 d8b2d63782fa5628
   *            60cf579b0833b7ff 5c0529f2a97c6452 2fa1a8878a9635ab
   *            ce56debf431bdec2 70b308fa5bf387#)
   *        (c #04e103ee925cb5e6 6653949fa5e1a462 c9e65e1adcd60058
   *            e2df9607cee95fa8 daec7a389a7d9afc 8dd21fef9d83805a
   *            40d46f49676a2f6b 2926f70c572c00#)))
   */
  
  mpz_set_str(pub->n,
	      "69abd505285af665" "36ddc7c8f027e6f0" "ed435d6748b16088"
	      "4fd60842b3a8d7fb" "bd8a3c98f0cc50ae" "4f6a9f7dd73122cc"
	      "ec8afa3f77134406" "f53721973115fc2d" "8cfbba23b145f28d"
	      "84f81d3b6ae8ce1e" "2850580c026e809b" "cfbb52566ea3a3b3"
	      "df7edf52971872a7" "e35c1451b8636d22" "279a8fb299368238"
	      "e545fbb4cf", 16);
  mpz_set_str(pub->e, "0db2ad57", 16);

  ASSERT (rsa_public_key_prepare(pub));
  
  /* d is not used */
#if 0  
  mpz_set_str(key->d,
	      "3240a56f4cd0dcc2" "4a413eb4ea545259" "5c83d771a1c2ba7b"
	      "ec47c5b43eb4b374" "09bd2aa1e236dd86" "481eb1768811412f"
	      "f8d91be3545912af" "b55c014cb55ceac6" "54216af3b85d5c4f"
	      "4a32894e3b5dfcde" "5b2875aa4dc8d9a8" "6afd0ca92ef50d35"
	      "bd09f1c47efb4c8d" "c631e07698d362aa" "4a83fd304e66d6c5"
	      "468863c307", 16);
#endif
  
  mpz_set_str(key->p,
	      "0a66399919be4b4d" "e5a78c5ea5c85bf9" "aba8c013cb4a8732"
	      "14557a12bd67711e" "bb4073fd39ad9a86" "f4e80253ad809e5b"
	      "f2fad3bc37f6f013" "273c9552c9f489", 16);

  mpz_set_str(key->q,
	      "0a294f069f118625" "f5eae2538db9338c" "776a298eae953329"
	      "9fd1eed4eba04e82" "b2593bc98ba8db27" "de034da7daaea795"
	      "2d55b07b5f9a5875" "d1ca5f6dcab897", 16);
  
  mpz_set_str(key->a,
	      "011b6c48eb592eee" "e85d1bb35cfb6e07" "344ea0b5e5f03a28"
	      "5b405396cbc78c5c" "868e961db160ba8d" "4b984250930cf79a"
	      "1bf8a9f28963de53" "128aa7d690eb87", 16);
  
  mpz_set_str(key->b,
	      "0409ecf3d2557c88" "214f1af5e1f17853" "d8b2d63782fa5628"
	      "60cf579b0833b7ff" "5c0529f2a97c6452" "2fa1a8878a9635ab"
	      "ce56debf431bdec2" "70b308fa5bf387", 16);
  
  mpz_set_str(key->c,
	      "04e103ee925cb5e6" "6653949fa5e1a462" "c9e65e1adcd60058"
	      "e2df9607cee95fa8" "daec7a389a7d9afc" "8dd21fef9d83805a"
	      "40d46f49676a2f6b" "2926f70c572c00", 16);

  ASSERT (rsa_private_key_prepare(key));
  ASSERT (pub->size == key->size);
}

void
test_rsa_set_key_2(struct rsa_public_key *pub,
		   struct rsa_private_key *key)
{
  /* Initialize key pair for test programs */
  /* 2048-bit key, generated by
   *
   *   certtool --generate-privkey --key-type=rsa --bits=2048 --outfile -
   *
   * Public Key Info:
   *         Public Key Algorithm: RSA
   *         Key Security Level: Medium (2048 bits)
   *
   * modulus:
   *         00:dd:ef:1d:66:d5:f0:b3:56:9b:da:2a:59:c7:96:e6
   *         7a:c9:c8:13:61:89:da:f1:5a:01:65:61:ac:fb:53:1a
   *         09:49:41:dd:fd:db:6e:68:c4:3a:8a:61:46:c3:ea:8b
   *         34:5f:e1:f6:60:91:9b:09:ae:8e:8c:ad:fa:99:ef:2a
   *         b5:7d:32:ad:cc:2d:4f:3f:df:21:42:b7:ce:6e:ab:ca
   *         eb:a4:93:97:8f:55:c4:8f:19:de:dd:ea:0a:83:10:3c
   *         8c:5f:f5:ce:01:7f:32:3d:d6:f8:82:f2:2d:3f:8f:20
   *         6a:02:68:a9:8b:0d:12:99:93:67:f9:1c:23:ae:5f:0d
   *         df:ec:2b:5d:78:c8:14:ac:6b:e9:2a:f7:aa:3e:cd:a1
   *         c8:c8:3e:11:24:50:eb:68:34:4c:0c:c1:2a:be:13:95
   *         ee:a0:73:dd:09:f2:38:5c:58:a7:dc:60:30:90:c9:a4
   *         1b:19:ed:51:5c:15:51:54:cd:8a:92:10:76:d4:19:88
   *         f8:93:ff:8d:08:a2:46:0f:59:af:c1:3d:4e:24:b5:e5
   *         8a:8e:44:10:85:74:8d:06:64:72:b2:f7:c5:6c:75:09
   *         fa:bc:9d:73:5d:14:c5:a6:81:f5:2e:8a:2d:a9:aa:c1
   *         43:b9:07:cf:77:90:7a:28:c5:41:53:51:d1:b3:6a:e2
   *         7b:
   *
   * public exponent:
   *         01:00:01:
   *
   * private exponent:
   *         73:2e:6f:66:f8:af:d4:93:a5:8d:63:9f:76:cb:a5:50
   *         a2:ba:b8:fc:4d:4c:99:28:2a:43:50:9f:33:4c:9c:dd
   *         a6:ec:8d:66:fb:e4:60:71:3f:24:a4:79:d2:a2:3e:9e
   *         ef:08:5a:13:22:5e:81:76:db:ba:bd:6c:ab:49:8a:33
   *         e9:07:4d:56:03:49:f7:0f:39:b6:e3:a8:3a:9d:e4:51
   *         c9:f7:63:98:5b:5e:09:1a:d7:24:fb:1b:7b:8c:08:b0
   *         9d:f8:f7:72:a5:6e:10:d4:29:e3:e4:06:81:cf:29:76
   *         7b:4b:90:7a:7f:4d:60:f1:34:eb:ff:a3:b1:12:da:22
   *         9e:87:c0:77:22:38:03:4b:80:ab:0c:8a:07:14:c3:c0
   *         08:9e:2c:13:9f:74:f4:19:86:51:61:67:b0:ce:bc:7b
   *         8f:81:18:8e:3c:8e:e7:e5:d4:e9:ed:49:b8:0b:21:b1
   *         2a:82:0f:7b:4d:8d:e4:f7:bc:8a:6c:c9:20:8c:4b:ba
   *         1e:67:43:9e:58:a4:e4:20:c6:4b:22:05:9d:33:d3:11
   *         92:f2:94:88:d8:fb:c9:53:8f:9c:52:4e:d3:66:f3:2b
   *         8c:3b:56:a7:4c:25:10:44:a0:62:79:4d:44:fb:9d:0e
   *         35:8b:92:b0:49:af:5d:a4:00:36:44:3c:e7:31:9d:f9
   *
   *
   * prime1:
   *         00:ed:6b:29:25:c1:4f:9c:df:08:4b:c1:43:e3:e6:f3
   *         d6:6e:1e:2d:46:d6:eb:97:aa:c7:c9:32:5e:0a:6b:39
   *         ac:be:2a:49:98:f6:c3:c8:d0:08:33:5b:75:62:f7:d9
   *         7e:87:d6:90:2d:9d:ce:94:be:2b:ed:8e:aa:85:29:86
   *         d9:bc:c0:10:2e:95:30:cc:3f:72:75:81:d1:60:f6:89
   *         86:f7:42:dc:29:8f:dc:d7:15:e8:99:1a:1d:12:92:89
   *         88:95:47:3e:25:c5:f4:e5:c4:82:87:32:d3:d7:a8:ce
   *         d9:77:2c:fc:87:bb:d9:f9:e6:a4:ec:1e:c8:a9:7b:0b
   *         35:
   *
   * prime2:
   *         00:ef:4d:b5:f6:53:12:9a:4e:3c:3f:56:ff:25:86:44
   *         0e:b1:b7:81:c4:0e:31:40:90:e7:91:d7:e7:80:8e:8a
   *         cc:99:1a:1b:0b:5d:9b:9b:00:81:fa:36:95:02:6a:44
   *         16:6e:67:64:db:ba:35:9b:5b:1c:e7:96:ac:2a:e3:c8
   *         7a:a9:de:1d:e4:75:54:cf:73:c9:76:92:2f:03:8b:ef
   *         a4:dc:5c:4d:54:2a:cc:f6:c3:cd:a7:0e:84:ce:33:96
   *         9b:99:68:e5:a9:cf:80:5b:df:2b:5a:a4:02:b2:d5:65
   *         a7:b8:d0:33:c8:23:ed:89:d1:44:7b:d1:94:99:9f:3c
   *         ef:
   *
   * coefficient:
   *         5c:ba:5a:d7:98:43:ab:ac:a0:d2:9c:1f:ab:3d:5e:a0
   *         54:c9:6e:54:19:9b:ff:9c:dc:05:75:93:97:3a:88:e9
   *         bb:1f:79:05:03:79:9b:b7:ac:07:71:bf:b0:a7:12:fc
   *         22:6c:38:04:40:2f:b9:0d:2d:0a:f3:b8:2e:88:3e:ab
   *         d8:ee:30:5c:fb:dc:59:0e:d9:9f:96:cd:4a:4a:12:88
   *         aa:c9:f0:03:e2:01:df:5d:0e:ec:e6:ac:0b:f1:81:b8
   *         85:9b:f7:69:b6:c8:e4:54:d8:02:5b:58:b4:c2:45:02
   *         b7:65:d4:5c:dc:aa:b6:47:6e:dd:1f:72:46:6c:27:22
   *
   *
   * exp1:
   *         41:78:03:68:bd:dd:ce:4c:52:65:51:6d:ff:32:78:9a
   *         f0:d2:b1:79:8f:5a:78:00:48:07:5b:34:43:7b:3d:f4
   *         3c:9c:3c:9f:49:ac:c3:7b:5a:47:8f:38:d7:89:b1:18
   *         0b:2d:47:a4:cc:97:62:bc:ee:30:1b:df:39:c9:31:be
   *         69:26:2d:50:2b:23:c1:ae:dd:49:39:fb:1a:d9:e1:22
   *         ae:9c:69:49:ac:ba:21:35:91:66:66:a5:0d:b2:0a:ea
   *         f6:ff:26:4c:14:42:6b:f9:bc:64:bb:c7:5e:f8:d5:d1
   *         71:e3:9d:df:70:15:b3:ab:be:5e:be:3e:67:3d:de:e1
   *
   *
   * exp2:
   *         63:00:4a:5c:5a:e7:e2:50:a5:9a:2a:ba:a9:e2:8f:3b
   *         69:08:9b:35:ea:0d:34:41:fe:9b:96:af:de:be:99:eb
   *         a5:17:68:c2:dd:fa:27:39:21:8c:cb:92:00:0a:c8:9a
   *         63:18:81:60:69:fc:0d:86:b7:41:94:53:2b:f7:4a:94
   *         7c:bc:38:af:b0:5e:e2:e8:6b:1b:93:c4:c1:79:de:2d
   *         dd:40:8e:79:58:af:ad:13:3a:7c:77:84:37:ee:9d:cb
   *         47:bf:5e:ec:4e:bd:32:c4:f4:21:ae:a2:b3:2b:97:bf
   *         b8:b4:e2:07:55:dd:ca:db:79:b2:a3:f5:0f:4d:12:9f
   */

  mpz_set_str(pub->n,
	      "00ddef1d66d5f0b3" "569bda2a59c796e6" "7ac9c8136189daf1"
	      "5a016561acfb531a" "094941ddfddb6e68" "c43a8a6146c3ea8b"
	      "345fe1f660919b09" "ae8e8cadfa99ef2a" "b57d32adcc2d4f3f"
	      "df2142b7ce6eabca" "eba493978f55c48f" "19deddea0a83103c"
	      "8c5ff5ce017f323d" "d6f882f22d3f8f20" "6a0268a98b0d1299"
	      "9367f91c23ae5f0d" "dfec2b5d78c814ac" "6be92af7aa3ecda1"
	      "c8c83e112450eb68" "344c0cc12abe1395" "eea073dd09f2385c"
	      "58a7dc603090c9a4" "1b19ed515c155154" "cd8a921076d41988"
	      "f893ff8d08a2460f" "59afc13d4e24b5e5" "8a8e441085748d06"
	      "6472b2f7c56c7509" "fabc9d735d14c5a6" "81f52e8a2da9aac1"
	      "43b907cf77907a28" "c5415351d1b36ae2" "7b", 16);
  mpz_set_str(pub->e, "010001", 16);

  ASSERT (rsa_public_key_prepare(pub));

  /* d is not used */
#if 0
  mpz_set_str(key->d,
	      "732e6f66f8afd493" "a58d639f76cba550" "a2bab8fc4d4c9928"
	      "2a43509f334c9cdd" "a6ec8d66fbe46071" "3f24a479d2a23e9e"
	      "ef085a13225e8176" "dbbabd6cab498a33" "e9074d560349f70f"
	      "39b6e3a83a9de451" "c9f763985b5e091a" "d724fb1b7b8c08b0"
	      "9df8f772a56e10d4" "29e3e40681cf2976" "7b4b907a7f4d60f1"
	      "34ebffa3b112da22" "9e87c0772238034b" "80ab0c8a0714c3c0"
	      "089e2c139f74f419" "86516167b0cebc7b" "8f81188e3c8ee7e5"
	      "d4e9ed49b80b21b1" "2a820f7b4d8de4f7" "bc8a6cc9208c4bba"
	      "1e67439e58a4e420" "c64b22059d33d311" "92f29488d8fbc953"
	      "8f9c524ed366f32b" "8c3b56a74c251044" "a062794d44fb9d0e"
	      "358b92b049af5da4" "0036443ce7319df9" , 16);
#endif

  mpz_set_str(key->p,
	      "00ed6b2925c14f9c" "df084bc143e3e6f3" "d66e1e2d46d6eb97"
	      "aac7c9325e0a6b39" "acbe2a4998f6c3c8" "d008335b7562f7d9"
	      "7e87d6902d9dce94" "be2bed8eaa852986" "d9bcc0102e9530cc"
	      "3f727581d160f689" "86f742dc298fdcd7" "15e8991a1d129289"
	      "8895473e25c5f4e5" "c4828732d3d7a8ce" "d9772cfc87bbd9f9"
	      "e6a4ec1ec8a97b0b" "35", 16);

  mpz_set_str(key->q,
	      "00ef4db5f653129a" "4e3c3f56ff258644" "0eb1b781c40e3140"
	      "90e791d7e7808e8a" "cc991a1b0b5d9b9b" "0081fa3695026a44"
	      "166e6764dbba359b" "5b1ce796ac2ae3c8" "7aa9de1de47554cf"
	      "73c976922f038bef" "a4dc5c4d542accf6" "c3cda70e84ce3396"
	      "9b9968e5a9cf805b" "df2b5aa402b2d565" "a7b8d033c823ed89"
	      "d1447bd194999f3c" "ef", 16);

  mpz_set_str(key->a,
	      "41780368bdddce4c" "5265516dff32789a" "f0d2b1798f5a7800"
	      "48075b34437b3df4" "3c9c3c9f49acc37b" "5a478f38d789b118"
	      "0b2d47a4cc9762bc" "ee301bdf39c931be" "69262d502b23c1ae"
	      "dd4939fb1ad9e122" "ae9c6949acba2135" "916666a50db20aea"
	      "f6ff264c14426bf9" "bc64bbc75ef8d5d1" "71e39ddf7015b3ab"
	      "be5ebe3e673ddee1", 16);

  mpz_set_str(key->b,
	      "63004a5c5ae7e250" "a59a2abaa9e28f3b" "69089b35ea0d3441"
	      "fe9b96afdebe99eb" "a51768c2ddfa2739" "218ccb92000ac89a"
	      "6318816069fc0d86" "b74194532bf74a94" "7cbc38afb05ee2e8"
	      "6b1b93c4c179de2d" "dd408e7958afad13" "3a7c778437ee9dcb"
	      "47bf5eec4ebd32c4" "f421aea2b32b97bf" "b8b4e20755ddcadb"
	      "79b2a3f50f4d129f", 16);

  mpz_set_str(key->c,
	      "5cba5ad79843abac" "a0d29c1fab3d5ea0" "54c96e54199bff9c"
	      "dc057593973a88e9" "bb1f790503799bb7" "ac0771bfb0a712fc"
	      "226c3804402fb90d" "2d0af3b82e883eab" "d8ee305cfbdc590e"
	      "d99f96cd4a4a1288" "aac9f003e201df5d" "0eece6ac0bf181b8"
	      "859bf769b6c8e454" "d8025b58b4c24502" "b765d45cdcaab647"
	      "6edd1f72466c2722", 16);

  ASSERT (rsa_private_key_prepare(key));
  ASSERT (pub->size == key->size);
}

void
test_rsa_md5(struct rsa_public_key *pub,
	     struct rsa_private_key *key,
	     mpz_t expected)
{
  struct md5_ctx md5;
  struct knuth_lfib_ctx rstate;
  uint8_t digest[MD5_DIGEST_SIZE];
  mpz_t signature;

  md5_init(&md5);
  mpz_init(signature);
  knuth_lfib_init (&rstate, 15);

  SIGN(md5, "The magic words are squeamish ossifrage", expected);

  /* Try bad data */
  ASSERT (!VERIFY(pub, md5,
		  "The magick words are squeamish ossifrage", signature));

  /* Try correct data */
  ASSERT (VERIFY(pub, md5,
		 "The magic words are squeamish ossifrage", signature));

  /* Try bad signature */
  mpz_combit(signature, 17);
  ASSERT (!VERIFY(pub, md5,
		  "The magic words are squeamish ossifrage", signature));

  mpz_clear(signature);
}

void
test_rsa_sha1(struct rsa_public_key *pub,
	      struct rsa_private_key *key,
	      mpz_t expected)
{
  struct sha1_ctx sha1;
  struct knuth_lfib_ctx rstate;
  uint8_t digest[SHA1_DIGEST_SIZE];
  mpz_t signature;

  sha1_init(&sha1);
  mpz_init(signature);
  knuth_lfib_init (&rstate, 16);

  SIGN(sha1, "The magic words are squeamish ossifrage", expected);

  /* Try bad data */
  ASSERT (!VERIFY(pub, sha1,
		  "The magick words are squeamish ossifrage", signature));

  /* Try correct data */
  ASSERT (VERIFY(pub, sha1,
		 "The magic words are squeamish ossifrage", signature));

  /* Try bad signature */
  mpz_combit(signature, 17);
  ASSERT (!VERIFY(pub, sha1,
		  "The magic words are squeamish ossifrage", signature));

  mpz_clear(signature);
}

void
test_rsa_sha256(struct rsa_public_key *pub,
		struct rsa_private_key *key,
		mpz_t expected)
{
  struct sha256_ctx sha256;
  struct knuth_lfib_ctx rstate;
  uint8_t digest[SHA256_DIGEST_SIZE];
  mpz_t signature;

  sha256_init(&sha256);
  mpz_init(signature);
  knuth_lfib_init (&rstate, 17);

  SIGN(sha256, "The magic words are squeamish ossifrage", expected);

  /* Try bad data */
  ASSERT (!VERIFY(pub, sha256,
		  "The magick words are squeamish ossifrage", signature));

  /* Try correct data */
  ASSERT (VERIFY(pub, sha256,
		 "The magic words are squeamish ossifrage", signature));

  /* Try bad signature */
  mpz_combit(signature, 17);
  ASSERT (!VERIFY(pub, sha256,
		  "The magic words are squeamish ossifrage", signature));

  mpz_clear(signature);
}

void
test_rsa_sha512(struct rsa_public_key *pub,
		struct rsa_private_key *key,
		mpz_t expected)
{
  struct sha512_ctx sha512;
  struct knuth_lfib_ctx rstate;
  uint8_t digest[SHA512_DIGEST_SIZE];
  mpz_t signature;

  sha512_init(&sha512);
  mpz_init(signature);
  knuth_lfib_init (&rstate, 18);

  SIGN(sha512, "The magic words are squeamish ossifrage", expected);

  /* Try bad data */
  ASSERT (!VERIFY(pub, sha512,
		  "The magick words are squeamish ossifrage", signature));

  /* Try correct data */
  ASSERT (VERIFY(pub, sha512,
		 "The magic words are squeamish ossifrage", signature));

  /* Try bad signature */
  mpz_combit(signature, 17);
  ASSERT (!VERIFY(pub, sha512,
		  "The magic words are squeamish ossifrage", signature));

  mpz_clear(signature);
}

#undef SIGN
#undef VERIFY

void
test_rsa_key(struct rsa_public_key *pub,
	     struct rsa_private_key *key)
{
  mpz_t tmp;
  mpz_t phi;
  
  mpz_init(tmp); mpz_init(phi);
  
  if (verbose)
    {
      fprintf(stderr, "Public key: n=");
      mpz_out_str(stderr, 16, pub->n);
      fprintf(stderr, "\n    e=");
      mpz_out_str(stderr, 16, pub->e);

      fprintf(stderr, "\n\nPrivate key: d=");
      mpz_out_str(stderr, 16, key->d);
      fprintf(stderr, "\n    p=");
      mpz_out_str(stderr, 16, key->p);
      fprintf(stderr, "\n    q=");
      mpz_out_str(stderr, 16, key->q);
      fprintf(stderr, "\n    a=");
      mpz_out_str(stderr, 16, key->a);
      fprintf(stderr, "\n    b=");
      mpz_out_str(stderr, 16, key->b);
      fprintf(stderr, "\n    c=");
      mpz_out_str(stderr, 16, key->c);
      fprintf(stderr, "\n\n");
    }

  /* Check n = p q */
  mpz_mul(tmp, key->p, key->q);
  ASSERT (mpz_cmp(tmp, pub->n)== 0);

  /* Check c q = 1 mod p */
  mpz_mul(tmp, key->c, key->q);
  mpz_fdiv_r(tmp, tmp, key->p);
  ASSERT (mpz_cmp_ui(tmp, 1) == 0);

  /* Check ed = 1 (mod phi) */
  mpz_sub_ui(phi, key->p, 1);
  mpz_sub_ui(tmp, key->q, 1);

  mpz_mul(phi, phi, tmp);

  mpz_mul(tmp, pub->e, key->d);
  mpz_fdiv_r(tmp, tmp, phi);
  ASSERT (mpz_cmp_ui(tmp, 1) == 0);

  /* Check a e = 1 (mod (p-1) ) */
  mpz_sub_ui(phi, key->p, 1);
  mpz_mul(tmp, pub->e, key->a);
  mpz_fdiv_r(tmp, tmp, phi);
  ASSERT (mpz_cmp_ui(tmp, 1) == 0);
  
  /* Check b e = 1 (mod (q-1) ) */
  mpz_sub_ui(phi, key->q, 1);
  mpz_mul(tmp, pub->e, key->b);
  mpz_fdiv_r(tmp, tmp, phi);
  ASSERT (mpz_cmp_ui(tmp, 1) == 0);
  
  mpz_clear(tmp); mpz_clear(phi);
}

/* Requires that the context is named like the hash algorithm. */
#define DSA_VERIFY(key, hash, msg, signature)	\
  (hash##_update(&hash, LDATA(msg)),		\
   dsa_##hash##_verify(key, &hash, signature))

void
test_dsa160(const struct dsa_public_key *pub,
	    const struct dsa_private_key *key,
	    const struct dsa_signature *expected)
{
  struct sha1_ctx sha1;
  struct dsa_signature signature;
  struct knuth_lfib_ctx lfib;
  
  sha1_init(&sha1);
  dsa_signature_init(&signature);
  knuth_lfib_init(&lfib, 1111);
  
  sha1_update(&sha1, LDATA("The magic words are squeamish ossifrage"));
  ASSERT (dsa_sha1_sign(pub, key,
			&lfib, (nettle_random_func *) knuth_lfib_random,
			&sha1, &signature));

  if (verbose)
    {
      fprintf(stderr, "dsa160 signature: ");
      mpz_out_str(stderr, 16, signature.r);
      fprintf(stderr, ", ");
      mpz_out_str(stderr, 16, signature.s);
      fprintf(stderr, "\n");
    }

  if (expected)
    ASSERT (mpz_cmp (signature.r, expected->r) == 0
	    && mpz_cmp (signature.s, expected->s) == 0);
  
  /* Try bad data */
  ASSERT (!DSA_VERIFY(pub, sha1,
		      "The magick words are squeamish ossifrage",
		      &signature));

  /* Try correct data */
  ASSERT (DSA_VERIFY(pub, sha1,
		     "The magic words are squeamish ossifrage",
		     &signature));

  /* Try bad signature */
  mpz_combit(signature.r, 17);
  ASSERT (!DSA_VERIFY(pub, sha1,
		      "The magic words are squeamish ossifrage",
		      &signature));

  dsa_signature_clear(&signature);
}

void
test_dsa256(const struct dsa_public_key *pub,
	    const struct dsa_private_key *key,
	    const struct dsa_signature *expected)
{
  struct sha256_ctx sha256;
  struct dsa_signature signature;
  struct knuth_lfib_ctx lfib;
  
  sha256_init(&sha256);
  dsa_signature_init(&signature);
  knuth_lfib_init(&lfib, 1111);
  
  sha256_update(&sha256, LDATA("The magic words are squeamish ossifrage"));
  ASSERT (dsa_sha256_sign(pub, key,
			&lfib, (nettle_random_func *) knuth_lfib_random,
			&sha256, &signature));
  
  if (verbose)
    {
      fprintf(stderr, "dsa256 signature: ");
      mpz_out_str(stderr, 16, signature.r);
      fprintf(stderr, ", ");
      mpz_out_str(stderr, 16, signature.s);
      fprintf(stderr, "\n");
    }

  if (expected)
    ASSERT (mpz_cmp (signature.r, expected->r) == 0
	    && mpz_cmp (signature.s, expected->s) == 0);
  
  /* Try bad data */
  ASSERT (!DSA_VERIFY(pub, sha256,
		      "The magick words are squeamish ossifrage",
		      &signature));

  /* Try correct data */
  ASSERT (DSA_VERIFY(pub, sha256,
		     "The magic words are squeamish ossifrage",
		     &signature));

  /* Try bad signature */
  mpz_combit(signature.r, 17);
  ASSERT (!DSA_VERIFY(pub, sha256,
		      "The magic words are squeamish ossifrage",
		      &signature));

  dsa_signature_clear(&signature);
}

#if 0
void
test_dsa_sign(const struct dsa_public_key *pub,
	      const struct dsa_private_key *key,
	      const struct nettle_hash *hash,
	      const struct dsa_signature *expected)
{
  void *ctx = xalloc (hash->context_size);
  uint8_t *digest = xalloc (hash->digest_size);
  uint8_t *bad_digest = xalloc (hash->digest_size);
  struct dsa_signature signature;
  struct knuth_lfib_ctx lfib;
  
  dsa_signature_init(&signature);
  knuth_lfib_init(&lfib, 1111);

  hash->init(ctx);
  
  hash->update(ctx, LDATA("The magic words are squeamish ossifrage"));
  hash->digest(ctx, hash->digest_size, digest);
  ASSERT (dsa_sign(pub, key,
		   &lfib, (nettle_random_func *) knuth_lfib_random,
		   hash->digest_size, digest, &signature));
  
  if (verbose)
    {
      fprintf(stderr, "dsa-%s signature: ", hash->name);
      mpz_out_str(stderr, 16, signature.r);
      fprintf(stderr, ", ");
      mpz_out_str(stderr, 16, signature.s);
      fprintf(stderr, "\n");
    }

  if (expected)
    ASSERT (mpz_cmp (signature.r, expected->r) == 0
	    && mpz_cmp (signature.s, expected->s) == 0);
  
  /* Try correct data */
  ASSERT (dsa_verify(pub, hash->digest_size, digest,
		     &signature));
  /* Try bad data */
  hash->update(ctx, LDATA("The magick words are squeamish ossifrage"));
  hash->digest(ctx, hash->digest_size, bad_digest);
  
  ASSERT (!dsa_verify(pub, hash->digest_size, bad_digest,
		      &signature));

  /* Try bad signature */
  mpz_combit(signature.r, 17);
  ASSERT (!dsa_verify(pub, hash->digest_size, digest,
		      &signature));

  free (ctx);
  free (digest);
  free (bad_digest);
  dsa_signature_clear(&signature);
}
#endif

void
test_dsa_verify(const struct dsa_params *params,
		const mpz_t pub,
		const struct nettle_hash *hash,
		struct tstring *msg,
		const struct dsa_signature *ref)
{
  void *ctx = xalloc (hash->context_size);
  uint8_t *digest = xalloc (hash->digest_size);
  struct dsa_signature signature;

  dsa_signature_init (&signature);

  hash->init(ctx);
  
  hash->update (ctx, msg->length, msg->data);
  hash->digest (ctx, hash->digest_size, digest);

  mpz_set (signature.r, ref->r);
  mpz_set (signature.s, ref->s);

  ASSERT (dsa_verify (params, pub,
		       hash->digest_size, digest,
		       &signature));

  /* Try bad signature */
  mpz_combit(signature.r, 17);
  ASSERT (!dsa_verify (params, pub,
		       hash->digest_size, digest,
		       &signature));
  
  /* Try bad data */
  digest[hash->digest_size / 2-1] ^= 8;
  ASSERT (!dsa_verify (params, pub,
		       hash->digest_size, digest,
		       ref));

  free (ctx);
  free (digest);
  dsa_signature_clear(&signature);  
}

void
test_dsa_key(const struct dsa_params *params,
	     const mpz_t pub,
	     const mpz_t key,
	     unsigned q_size)
{
  mpz_t t;

  mpz_init(t);

  ASSERT(mpz_sizeinbase(params->q, 2) == q_size);
  ASSERT(mpz_sizeinbase(params->p, 2) >= DSA_SHA1_MIN_P_BITS);
  
  ASSERT(mpz_probab_prime_p(params->p, 10));

  ASSERT(mpz_probab_prime_p(params->q, 10));

  mpz_fdiv_r(t, params->p, params->q);

  ASSERT(0 == mpz_cmp_ui(t, 1));

  ASSERT(mpz_cmp_ui(params->g, 1) > 0);
  
  mpz_powm(t, params->g, params->q, params->p);
  ASSERT(0 == mpz_cmp_ui(t, 1));
  
  mpz_powm(t, params->g, key, params->p);
  ASSERT(0 == mpz_cmp(t, pub));

  mpz_clear(t);
}

const struct ecc_curve * const ecc_curves[] = {
  &_nettle_secp_192r1,
  &_nettle_secp_224r1,
  &_nettle_secp_256r1,
  &_nettle_secp_384r1,
  &_nettle_secp_521r1,
  &_nettle_curve25519,
  &_nettle_curve448,
  &_nettle_gost_gc256b,
  &_nettle_gost_gc512a,
  NULL
};

int
test_ecc_point_valid_p (struct ecc_point *pub)
{
  mpz_t t, x, y;
  mpz_t lhs, rhs;
  int res;
  mp_size_t size;

  size = pub->ecc->p.size;

  /* First check range */
  if (mpn_cmp (pub->p, pub->ecc->p.m, size) >= 0
      || mpn_cmp (pub->p + size, pub->ecc->p.m, size) >= 0)
    return 0;

  mpz_init (lhs);
  mpz_init (rhs);

  mpz_roinit_n (x, pub->p, size);
  mpz_roinit_n (y, pub->p + size, size);

  mpz_mul (lhs, y, y);

  if (pub->ecc->p.bit_size == 255)
    {
      /* Check that
	 121666 (1 + x^2 - y^2) = 121665 x^2 y^2 */
      mpz_t x2;
      mpz_init (x2);
      mpz_mul (x2, x, x); /* x^2 */
      mpz_mul (rhs, x2, lhs); /* x^2 y^2 */
      mpz_sub (lhs, x2, lhs); /* x^2 - y^2 */
      mpz_add_ui (lhs, lhs, 1); /* 1 + x^2 - y^2 */
      mpz_mul_ui (lhs, lhs, 121666);
      mpz_mul_ui (rhs, rhs, 121665);

      mpz_clear (x2);
    }
  else if (pub->ecc->p.bit_size == 448)
    {
      /* Check that
	 x^2 + y^2 = 1 - 39081 x^2 y^2 */
      mpz_t x2, d;
      mpz_init (x2);
      mpz_init_set_ui (d, 39081);
      mpz_mul (x2, x, x); /* x^2 */
      mpz_mul (d, d, x2); /* 39081 x^2 */
      mpz_set_ui (rhs, 1);
      mpz_submul (rhs, d, lhs); /* 1 - 39081 x^2 y^2 */
      mpz_add (lhs, x2, lhs);	/* x^2 + y^2 */

      mpz_clear (d);
      mpz_clear (x2);
    }
  else
    {
      /* Check y^2 = x^3 - 3 x + b */
      mpz_mul (rhs, x, x);
      mpz_sub_ui (rhs, rhs, 3);
      mpz_mul (rhs, rhs, x);
      mpz_add (rhs, rhs, mpz_roinit_n (t, pub->ecc->b, size));
    }
  res = mpz_congruent_p (lhs, rhs, mpz_roinit_n (t, pub->ecc->p.m, size));

  mpz_clear (lhs);
  mpz_clear (rhs);

  return res;
}

static int
test_mpn (const char *ref, const mp_limb_t *xp, mp_size_t n)
{
  mpz_t r, x;
  int res;

  mpz_init_set_str (r, ref, 16);
  
  res = (mpz_cmp (r, mpz_roinit_n (x, xp, n)) == 0);
  mpz_clear (r);
  return res;
}

void
write_mpn (FILE *f, int base, const mp_limb_t *xp, mp_size_t n)
{
  mpz_t t;
  mpz_out_str (f, base, mpz_roinit_n (t,xp, n));
}

void
test_ecc_point (const struct ecc_curve *ecc,
		const struct ecc_ref_point *ref,
		const mp_limb_t *p)
{
  if (! (test_mpn (ref->x, p, ecc->p.size)
	 && test_mpn (ref->y, p + ecc->p.size, ecc->p.size) ))
    {
      fprintf (stderr, "Incorrect point, curve bits %d!\n"
	       "got: x = ", ecc->p.bit_size);
      write_mpn (stderr, 16, p, ecc->p.size);
      fprintf (stderr, "\n"
	       "     y = ");
      write_mpn (stderr, 16, p + ecc->p.size, ecc->p.size);
      fprintf (stderr, "\n"
	       "ref: x = %s\n"
	       "     y = %s\n",
	       ref->x, ref->y);
      abort();
    }
}

/* For each curve, the points g, 2 g, 3 g and 4 g */
static const struct ecc_ref_point ecc_ref[9][4] = {
  { { "188da80eb03090f67cbf20eb43a18800f4ff0afd82ff1012",
      "07192b95ffc8da78631011ed6b24cdd573f977a11e794811" },
    { "dafebf5828783f2ad35534631588a3f629a70fb16982a888",
	"dd6bda0d993da0fa46b27bbc141b868f59331afa5c7e93ab" },
    { "76e32a2557599e6edcd283201fb2b9aadfd0d359cbb263da",
	"782c37e372ba4520aa62e0fed121d49ef3b543660cfd05fd" },
    { "35433907297cc378b0015703374729d7a4fe46647084e4ba",
	"a2649984f2135c301ea3acb0776cd4f125389b311db3be32" }
  },
  { { "b70e0cbd6bb4bf7f321390b94a03c1d356c21122343280d6115c1d21",
	"bd376388b5f723fb4c22dfe6cd4375a05a07476444d5819985007e34" },
    { "706a46dc76dcb76798e60e6d89474788d16dc18032d268fd1a704fa6",
	"1c2b76a7bc25e7702a704fa986892849fca629487acf3709d2e4e8bb" },
    { "df1b1d66a551d0d31eff822558b9d2cc75c2180279fe0d08fd896d04",
	"a3f7f03cadd0be444c0aa56830130ddf77d317344e1af3591981a925" },
    { "ae99feebb5d26945b54892092a8aee02912930fa41cd114e40447301",
	"482580a0ec5bc47e88bc8c378632cd196cb3fa058a7114eb03054c9" },
  },
  { { "6b17d1f2e12c4247f8bce6e563a440f277037d812deb33a0f4a13945d898c296",
	"4fe342e2fe1a7f9b8ee7eb4a7c0f9e162bce33576b315ececbb6406837bf51f5" },
    { "7cf27b188d034f7e8a52380304b51ac3c08969e277f21b35a60b48fc47669978",
	"7775510db8ed040293d9ac69f7430dbba7dade63ce982299e04b79d227873d1" },
    { "5ecbe4d1a6330a44c8f7ef951d4bf165e6c6b721efada985fb41661bc6e7fd6c",
	"8734640c4998ff7e374b06ce1a64a2ecd82ab036384fb83d9a79b127a27d5032" },
    { "e2534a3532d08fbba02dde659ee62bd0031fe2db785596ef509302446b030852",
	"e0f1575a4c633cc719dfee5fda862d764efc96c3f30ee0055c42c23f184ed8c6" },
  },
  { { "aa87ca22be8b05378eb1c71ef320ad746e1d3b628ba79b9859f741e082542a38"
	"5502f25dbf55296c3a545e3872760ab7",
	"3617de4a96262c6f5d9e98bf9292dc29f8f41dbd289a147ce9da3113b5f0b8c0"
	"0a60b1ce1d7e819d7a431d7c90ea0e5f" },
    { "8d999057ba3d2d969260045c55b97f089025959a6f434d651d207d19fb96e9e"
	"4fe0e86ebe0e64f85b96a9c75295df61",
	"8e80f1fa5b1b3cedb7bfe8dffd6dba74b275d875bc6cc43e904e505f256ab425"
	"5ffd43e94d39e22d61501e700a940e80" },
    { "77a41d4606ffa1464793c7e5fdc7d98cb9d3910202dcd06bea4f240d3566da6"
	"b408bbae5026580d02d7e5c70500c831",
	"c995f7ca0b0c42837d0bbe9602a9fc998520b41c85115aa5f7684c0edc111eac"
	"c24abd6be4b5d298b65f28600a2f1df1" },
    { "138251cd52ac9298c1c8aad977321deb97e709bd0b4ca0aca55dc8ad51dcfc9d"
	"1589a1597e3a5120e1efd631c63e1835",
	"cacae29869a62e1631e8a28181ab56616dc45d918abc09f3ab0e63cf792aa4dc"
	"ed7387be37bba569549f1c02b270ed67" },
  },
  { { "c6"
	"858e06b70404e9cd9e3ecb662395b4429c648139053fb521f828af606b4d3dba"
	"a14b5e77efe75928fe1dc127a2ffa8de3348b3c1856a429bf97e7e31c2e5bd66",
	"118"
	"39296a789a3bc0045c8a5fb42c7d1bd998f54449579b446817afbd17273e662c"
	"97ee72995ef42640c550b9013fad0761353c7086a272c24088be94769fd16650" },
    { "43"
	"3c219024277e7e682fcb288148c282747403279b1ccc06352c6e5505d769be97"
	"b3b204da6ef55507aa104a3a35c5af41cf2fa364d60fd967f43e3933ba6d783d",
	"f4"
	"bb8cc7f86db26700a7f3eceeeed3f0b5c6b5107c4da97740ab21a29906c42dbb"
	"b3e377de9f251f6b93937fa99a3248f4eafcbe95edc0f4f71be356d661f41b02"
    },
    { "1a7"
	"3d352443de29195dd91d6a64b5959479b52a6e5b123d9ab9e5ad7a112d7a8dd1"
	"ad3f164a3a4832051da6bd16b59fe21baeb490862c32ea05a5919d2ede37ad7d",
	"13e"
	"9b03b97dfa62ddd9979f86c6cab814f2f1557fa82a9d0317d2f8ab1fa355ceec"
	"2e2dd4cf8dc575b02d5aced1dec3c70cf105c9bc93a590425f588ca1ee86c0e5" },
    { "35"
	"b5df64ae2ac204c354b483487c9070cdc61c891c5ff39afc06c5d55541d3ceac"
	"8659e24afe3d0750e8b88e9f078af066a1d5025b08e5a5e2fbc87412871902f3",
	"82"
	"096f84261279d2b673e0178eb0b4abb65521aef6e6e32e1b5ae63fe2f19907f2"
	"79f283e54ba385405224f750a95b85eebb7faef04699d1d9e21f47fc346e4d0d" },
  },
  { { "216936d3cd6e53fec0a4e231fdd6dc5c692cc7609525a7b2c9562d608f25d51a",
      "6666666666666666666666666666666666666666666666666666666666666658" },
    { "36ab384c9f5a046c3d043b7d1833e7ac080d8e4515d7a45f83c5a14e2843ce0e",
	"2260cdf3092329c21da25ee8c9a21f5697390f51643851560e5f46ae6af8a3c9" },
    { "67ae9c4a22928f491ff4ae743edac83a6343981981624886ac62485fd3f8e25c",
	"1267b1d177ee69aba126a18e60269ef79f16ec176724030402c3684878f5b4d4" },
    { "203da8db56cff1468325d4b87a3520f91a739ec193ce1547493aa657c4c9f870",
	"47d0e827cb1595e1470eb88580d5716c4cf22832ea2f0ff0df38ab61ca32112f" },
  },
  { { "4f1970c66bed0ded221d15a622bf36da9e146570470f1767ea6de324a3d3a46412ae1af72ab66511433b80e18b00938e2626a82bc70cc05e",
	"693f46716eb6bc248876203756c9c7624bea73736ca3984087789c1e05a0c2d73ad3ff1ce67c39c4fdbd132c4ed7c8ad9808795bf230fa14" },
    { "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa955555555555555555555555555555555555555555555555555555555",
	"ae05e9634ad7048db359d6205086c2b0036ed7a035884dd7b7e36d728ad8c4b80d6565833a2a3098bbbcb2bed1cda06bdaeafbcdea9386ed" },
    { "865886b9108af6455bd64316cb6943332241b8b8cda82c7e2ba077a4a3fcfe8daa9cbf7f6271fd6e862b769465da8575728173286ff2f8f",
	"e005a8dbd5125cf706cbda7ad43aa6449a4a8d952356c3b9fce43c82ec4e1d58bb3a331bdb6767f0bffa9a68fed02dafb822ac13588ed6fc" },
    { "49dcbc5c6c0cce2c1419a17226f929ea255a09cf4e0891c693fda4be70c74cc301b7bdf1515dd8ba21aee1798949e120e2ce42ac48ba7f30",
	"d49077e4accde527164b33a5de021b979cb7c02f0457d845c90dc3227b8a5bc1c0d8f97ea1ca9472b5d444285d0d4f5b32e236f86de51839" },
  },
  { { "0000000000000000000000000000000000000000000000000000000000000001",
      "8d91e471e0989cda27df505a453f2b7635294f2ddf23e3b122acc99c9e9f1e14" },
    { "fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffd95",
      "726e1b8e1f676325d820afa5bac0d489cad6b0d220dc1c4edd5336636160df83" },
    { "8e38e38e38e38e38e38e38e38e38e38e38e38e38e38e38e38e38e38e38e38d2c",
      "76bcd1ca9a23b041d4d9baf507a6cd821267a94c838768e8486117796b788a51" },
    { "f7063e7063e7063e7063e7063e7063e7063e7063e7063e7063e7063e7063e4b7",
      "83ccf17ba6706d73625cc3534c7a2b9d6ec1ee6a9a7e07c10d84b388de59f741" },
  },
  { { "0000000000000000000000000000000000000000000000000000000000000000"
      "0000000000000000000000000000000000000000000000000000000000000003",
      "7503cfe87a836ae3a61b8816e25450e6ce5e1c93acf1abc1778064fdcbefa921"
      "df1626be4fd036e93d75e6a50e3a41e98028fe5fc235f5b889a589cb5215f2a4" },
    { "3b89dcfc622996ab97a5869dbff15cf51db00954f43a58a5e5f6b0470a132b2f"
      "4434bbcd405d2a9516151d2a6a04f2e4375bf48de1fdb21fb982afd9d2ea137c",
      "c813c4e2e2e0a8a391774c7903da7a6f14686e98e183e670ee6fb784809a3e92"
      "ca209dc631d85b1c7534ed3b37fddf64d854d7e01f91f18bb3fd307591afc051" },
    { "a1ff1ab2712a267eb53935ddb5a567f84db156cc096168a1174291d5f488fba5"
      "43d2840b4d2dd35d764b2f57b308907aec55cfba10544e8416e134687ccb87c3",
      "3cb5c4417ec4637f30374f189bb5b984c41e3a48d7f84fbfa3819e3f333f7eb3"
      "11d3af7e67c4c16eeacfac2fe94c6dd4c6366f711a4fb6c7125cd7ec518d90d6" },
    { "b7bfb80956c8670031ba191929f64e301d681634236d47a60e571a4bedc0ef25"
      "7452ef78b5b98dbb3d9f3129d9349433ce2a3a35cb519c91e2d633d7b373ae16",
      "3bee95e29eecc5d5ad2beba941abcbf9f1cad478df0fecf614f63aeebef77850"
      "da7efdb93de8f3df80bc25eac09239c14175f5c29704ce9a3e383f1b3ec0e929" },
  }
};

void
test_ecc_ga (unsigned curve, const mp_limb_t *p)
{
  return test_ecc_point (ecc_curves[curve], &ecc_ref[curve][0], p);
}

void
test_ecc_mul_a (unsigned curve, unsigned n, const mp_limb_t *p)
{
  assert (curve < 9);
  assert (n <= 4);
  if (n == 0)
    {
      /* Makes sense for curve25519 only */
      const struct ecc_curve *ecc = ecc_curves[curve];
      assert (ecc->p.bit_size == 255 || ecc->p.bit_size == 448);
      if (!mpn_zero_p (p, ecc->p.size)
	  || mpn_cmp (p + ecc->p.size, ecc->unit, ecc->p.size) != 0)
	{
	  fprintf (stderr, "Incorrect point (expected (0, 1))!\n"
		   "got: x = ");
	  write_mpn (stderr, 16, p, ecc->p.size);
	  fprintf (stderr, "\n"
		   "     y = ");
	  write_mpn (stderr, 16, p + ecc->p.size, ecc->p.size);
	  fprintf (stderr, "\n");
	  abort();
	}
    }
  else
    test_ecc_point (ecc_curves[curve], &ecc_ref[curve][n-1], p);
}

void
test_ecc_mul_h (unsigned curve, unsigned n, const mp_limb_t *p)
{
  const struct ecc_curve *ecc = ecc_curves[curve];
  mp_limb_t *np = xalloc_limbs (ecc_size_a (ecc));
  mp_limb_t *scratch = xalloc_limbs (ecc->h_to_a_itch);
  ecc->h_to_a (ecc, 0, np, p, scratch);

  test_ecc_mul_a (curve, n, np);

  free (np);
  free (scratch);
}

void
test_ecc_get_g (unsigned curve, mp_limb_t *rp)
{
  const struct ecc_curve *ecc = ecc_curves[curve];
  mpz_t x;
  mpz_t y;
  mpz_init_set_str (x, ecc_ref[curve][0].x, 16);
  mpz_init_set_str (y, ecc_ref[curve][0].y, 16);

  if (ecc->use_redc)
    {
      mpz_t t;
      mpz_mul_2exp (x, x, ecc->p.size * GMP_NUMB_BITS);
      mpz_mod (x, x, mpz_roinit_n (t, ecc->p.m, ecc->p.size));
      mpz_mul_2exp (y, y, ecc->p.size * GMP_NUMB_BITS);
      mpz_mod (y, y, mpz_roinit_n (t, ecc->p.m, ecc->p.size));
    }
  mpz_limbs_copy (rp, x, ecc->p.size);
  mpz_limbs_copy (rp + ecc->p.size, y, ecc->p.size);
  mpn_copyi (rp + 2*ecc->p.size, ecc->unit, ecc->p.size);

  mpz_clear (x);
  mpz_clear (y);
}

void
test_ecc_get_ga (unsigned curve, mp_limb_t *rp)
{
  const struct ecc_curve *ecc = ecc_curves[curve];
  mpz_t x;
  mpz_t y;
  mpz_init_set_str (x, ecc_ref[curve][0].x, 16);
  mpz_init_set_str (y, ecc_ref[curve][0].y, 16);

  mpz_limbs_copy (rp, x, ecc->p.size);
  mpz_limbs_copy (rp + ecc->p.size, y, ecc->p.size);

  mpz_clear (x);
  mpz_clear (y);
}

#else /* !WITH_HOGWEED */
/* Make sure either gmp or mini-gmp is available for tests. */
#include "mini-gmp.c"
#endif /* !WITH_HOGWEED */

