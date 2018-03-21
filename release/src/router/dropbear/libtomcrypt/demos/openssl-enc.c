/* LibTomCrypt, modular cryptographic library -- Tom St Denis
 *
 * LibTomCrypt is a library that provides various cryptographic
 * algorithms in a highly modular and flexible manner.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 */

/*
 * Demo to do the rough equivalent of:
 *
 *    openssl enc -aes-256-cbc -pass pass:foobar -in infile -out outfile -p
 *
 * Compilation:
 *
 *    $(CC) -I /path/to/headers -L .../libs \
 *          -o openssl-enc \
 *          openssl-enc.c -ltomcrypt
 *
 * Usage:
 *
 *    ./openssl-enc <enc|dec> infile outfile "passphrase" [salt]
 *
 * If provided, the salt must be EXACTLY a 16-char hex string.
 *
 * Demo is an example of:
 *
 * - (When decrypting) yanking salt out of the OpenSSL "Salted__..." header
 * - OpenSSL-compatible key derivation (in OpenSSL's modified PKCS#5v1 approach)
 * - Grabbing an Initialization Vector from the key generator
 * - Performing simple block encryption using AES
 * - PKCS#7-type padding (which hopefully can get ripped out of this demo and
 *   made a libtomcrypt thing someday).
 *
 * This program is free for all purposes without any express guarantee it
 * works. If you really want to see a license here, assume the WTFPL :-)
 *
 * BJ Black, bblack@barracuda.com, https://wjblack.com
 *
 * BUGS:
 *       Passing a password on a command line is a HORRIBLE idea.  Don't use
 *       this program for serious work!
 */

#include <tomcrypt.h>

#ifndef LTC_RIJNDAEL
#error Cannot compile this demo; Rijndael (AES) required
#endif
#ifndef LTC_CBC_MODE
#error Cannot compile this demo; CBC mode required
#endif
#ifndef LTC_PKCS_5
#error Cannot compile this demo; PKCS5 required
#endif
#ifndef LTC_RNG_GET_BYTES
#error Cannot compile this demo; random generator required
#endif
#ifndef LTC_MD5
#error Cannot compile this demo; MD5 required
#endif

/* OpenSSL by default only runs one hash round */
#define OPENSSL_ITERATIONS 1
/* Use aes-256-cbc, so 256 bits of key, 128 of IV */
#define KEY_LENGTH (256>>3)
#define IV_LENGTH (128>>3)
/* PKCS#5v1 requires exactly an 8-byte salt */
#define SALT_LENGTH 8
/* The header OpenSSL puts on an encrypted file */
static char salt_header[] = { 'S', 'a', 'l', 't', 'e', 'd', '_', '_' };

#include <errno.h>
#include <stdio.h>
#include <string.h>

/* A simple way to handle the possibility that a block may increase in size
   after padding. */
union paddable {
   unsigned char unpad[1024];
   unsigned char pad[1024+MAXBLOCKSIZE];
};

/*
 * Print usage and exit with a bad status (and perror() if any errno).
 *
 * Input:        argv[0] and the error string
 * Output:       <no return>
 * Side Effects: print messages and barf (does exit(3))
 */
void barf(const char *pname, const char *err)
{
   printf("Usage: %s <enc|dec> infile outfile passphrase [salt]\n", pname);
   printf("\n");
   printf("       # encrypts infile->outfile, random salt\n");
   printf("       %s enc infile outfile \"passphrase\"\n", pname);
   printf("\n");
   printf("       # encrypts infile->outfile, salt from cmdline\n");
   printf("       %s enc infile outfile pass 0123456789abcdef\n", pname);
   printf("\n");
   printf("       # decrypts infile->outfile, pulls salt from infile\n");
   printf("       %s dec infile outfile pass\n", pname);
   printf("\n");
   printf("       # decrypts infile->outfile, salt specified\n");
   printf("       # (don't try to read the salt from infile)\n");
   printf("       %s dec infile outfile pass 0123456789abcdef"
          "\n", pname);
   printf("\n");
   printf("Application Error: %s\n", err);
   if(errno)
      perror("     System Error");
   exit(-1);
}

/*
 * Parse a salt value passed in on the cmdline.
 *
 * Input:        string passed in and a buf to put it in (exactly 8 bytes!)
 * Output:       CRYPT_OK if parsed OK, CRYPT_ERROR if not
 * Side Effects: none
 */
int parse_hex_salt(unsigned char *in, unsigned char *out)
{
   int idx;
   for(idx=0; idx<SALT_LENGTH; idx++)
      if(sscanf((char*)in+idx*2, "%02hhx", out+idx) != 1)
         return CRYPT_ERROR;
   return CRYPT_OK;
}

/*
 * Parse the Salted__[+8 bytes] from an OpenSSL-compatible file header.
 *
 * Input:        file to read from and a to put the salt in (exactly 8 bytes!)
 * Output:       CRYPT_OK if parsed OK, CRYPT_ERROR if not
 * Side Effects: infile's read pointer += 16
 */
int parse_openssl_header(FILE *in, unsigned char *out)
{
   unsigned char tmp[SALT_LENGTH];
   if(fread(tmp, 1, sizeof(tmp), in) != sizeof(tmp))
      return CRYPT_ERROR;
   if(memcmp(tmp, salt_header, sizeof(tmp)))
      return CRYPT_ERROR;
   if(fread(tmp, 1, sizeof(tmp), in) != sizeof(tmp))
      return CRYPT_ERROR;
   memcpy(out, tmp, sizeof(tmp));
   return CRYPT_OK;
}

/*
 * Dump a hexed stream of bytes (convenience func).
 *
 * Input:        buf to read from, length
 * Output:       none
 * Side Effects: bytes printed as a hex blob, no lf at the end
 */
void dump_bytes(unsigned char *in, unsigned long len)
{
   unsigned long idx;
   for(idx=0; idx<len; idx++)
      printf("%02hhX", *(in+idx));
}

/*
 * Pad or unpad a message using PKCS#7 padding.
 * Padding will add 1-(blocksize) bytes and unpadding will remove that amount.
 * Set is_padding to 1 to pad, 0 to unpad.
 *
 * Input:        paddable buffer, size read, block length of cipher, mode
 * Output:       number of bytes after padding resp. after unpadding
 * Side Effects: none
 */
size_t pkcs7_pad(union paddable *buf, size_t nb, int block_length,
                 int is_padding)
{
   unsigned char padval;
   off_t idx;

   if(is_padding) {
      /* We are PADDING this block (and therefore adding bytes) */
      /* The pad value in PKCS#7 is the number of bytes remaining in
         the block, so for a 16-byte block and 3 bytes left, it's
         0x030303.  In the oddball case where nb is an exact multiple
         multiple of block_length, set the padval to blocksize (i.e.
         add one full block) */
      padval = (unsigned char) (block_length - (nb % block_length));
      padval = padval ? padval : block_length;

      memset(buf->pad+nb, padval, padval);
      return nb+padval;
   } else {
      /* We are UNPADDING this block (and removing bytes)
         We really just need to verify that the pad bytes are correct,
         so start at the end of the string and work backwards. */

      /* Figure out what the padlength should be by looking at the
         last byte */
      idx = nb-1;
      padval = buf->pad[idx];

      /* padval must be nonzero and <= block length */
      if(padval <= 0 || padval > block_length)
         return 0;

      /* First byte's accounted for; do the rest */
      idx--;

      while(idx >= (off_t)(nb-padval))
         if(buf->pad[idx] != padval)
            return 0;
         else
            idx--;

      /* If we got here, the pad checked out, so return a smaller
         number of bytes than nb (basically where we left off+1) */
      return idx+1;
   }
}

/*
 * Perform an encrypt/decrypt operation to/from files using AES+CBC+PKCS7 pad.
 * Set encrypt to 1 to encrypt, 0 to decrypt.
 *
 * Input:        in/out files, key, iv, and mode
 * Output:       CRYPT_OK if no error
 * Side Effects: bytes slurped from infile, pushed to outfile, fds updated.
 */
int do_crypt(FILE *infd, FILE *outfd, unsigned char *key, unsigned char *iv,
             int encrypt)
{
   union paddable inbuf, outbuf;
   int cipher, ret;
   symmetric_CBC cbc;
   size_t nb;

   /* Register your cipher! */
   cipher = register_cipher(&aes_desc);
   if(cipher == -1)
      return CRYPT_INVALID_CIPHER;

   /* Start a CBC session with cipher/key/val params */
   ret = cbc_start(cipher, iv, key, KEY_LENGTH, 0, &cbc);
   if( ret != CRYPT_OK )
      return -1;

   do {
      /* Get bytes from the source */
      nb = fread(inbuf.unpad, 1, sizeof(inbuf.unpad), infd);
      if(!nb)
         return encrypt ? CRYPT_OK : CRYPT_ERROR;

      /* Barf if we got a read error */
      if(ferror(infd))
         return CRYPT_ERROR;

      if(encrypt) {
         /* We're encrypting, so pad first (if at EOF) and then
            crypt */
         if(feof(infd))
            nb = pkcs7_pad(&inbuf, nb,
                           aes_desc.block_length, 1);

         ret = cbc_encrypt(inbuf.pad, outbuf.pad, nb, &cbc);
         if(ret != CRYPT_OK)
            return ret;

      } else {
         /* We're decrypting, so decrypt and then unpad if at
            EOF */
         ret = cbc_decrypt(inbuf.unpad, outbuf.unpad, nb, &cbc);
         if( ret != CRYPT_OK )
            return ret;

         if( feof(infd) )
            nb = pkcs7_pad(&outbuf, nb,
                           aes_desc.block_length, 0);
         if(nb == 0)
            /* The file didn't decrypt correctly */
            return CRYPT_ERROR;

      }

      /* Push bytes to outfile */
      if(fwrite(outbuf.unpad, 1, nb, outfd) != nb)
         return CRYPT_ERROR;

   } while(!feof(infd));

   /* Close up */
   cbc_done(&cbc);

   return CRYPT_OK;
}

/* Convenience macro for the various barfable places below */
#define BARF(a) { \
   if(infd) fclose(infd); \
   if(outfd) { fclose(outfd); remove(argv[3]); } \
   barf(argv[0], a); \
}
/*
 * The main routine.  Mostly validate cmdline params, open files, run the KDF,
 * and do the crypt.
 */
int main(int argc, char *argv[]) {
   unsigned char salt[SALT_LENGTH];
   FILE *infd = NULL, *outfd = NULL;
   int encrypt = -1;
   int hash = -1;
   int ret;
   unsigned char keyiv[KEY_LENGTH + IV_LENGTH];
   unsigned long keyivlen = (KEY_LENGTH + IV_LENGTH);
   unsigned char *key, *iv;

   /* Check proper number of cmdline args */
   if(argc < 5 || argc > 6)
      BARF("Invalid number of arguments");

   /* Check proper mode of operation */
   if     (!strncmp(argv[1], "enc", 3))
      encrypt = 1;
   else if(!strncmp(argv[1], "dec", 3))
      encrypt = 0;
   else
      BARF("Bad command name");

   /* Check we can open infile/outfile */
   infd = fopen(argv[2], "rb");
   if(infd == NULL)
      BARF("Could not open infile");
   outfd = fopen(argv[3], "wb");
   if(outfd == NULL)
      BARF("Could not open outfile");

   /* Get the salt from wherever */
   if(argc == 6) {
      /* User-provided */
      if(parse_hex_salt((unsigned char*) argv[5], salt) != CRYPT_OK)
         BARF("Bad user-specified salt");
   } else if(!strncmp(argv[1], "enc", 3)) {
      /* Encrypting; get from RNG */
      if(rng_get_bytes(salt, sizeof(salt), NULL) != sizeof(salt))
         BARF("Not enough random data");
   } else {
      /* Parse from infile (decrypt only) */
      if(parse_openssl_header(infd, salt) != CRYPT_OK)
         BARF("Invalid OpenSSL header in infile");
   }

   /* Fetch the MD5 hasher for PKCS#5 */
   hash = register_hash(&md5_desc);
   if(hash == -1)
      BARF("Could not register MD5 hash");

   /* Set things to a sane initial state */
   zeromem(keyiv, sizeof(keyiv));
   key = keyiv + 0;      /* key comes first */
   iv = keyiv + KEY_LENGTH;   /* iv comes next */

   /* Run the key derivation from the provided passphrase.  This gets us
      the key and iv. */
   ret = pkcs_5_alg1_openssl((unsigned char*)argv[4], strlen(argv[4]), salt,
                             OPENSSL_ITERATIONS, hash, keyiv, &keyivlen );
   if(ret != CRYPT_OK)
      BARF("Could not derive key/iv from passphrase");

   /* Display the salt/key/iv like OpenSSL cmdline does when -p */
   printf("salt="); dump_bytes(salt, sizeof(salt)); printf("\n");
   printf("key=");  dump_bytes(key, KEY_LENGTH);    printf("\n");
   printf("iv =");  dump_bytes(iv,  IV_LENGTH );    printf("\n");

   /* If we're encrypting, write the salt header as OpenSSL does */
   if(!strncmp(argv[1], "enc", 3)) {
      if(fwrite(salt_header, 1, sizeof(salt_header), outfd) !=
         sizeof(salt_header) )
         BARF("Error writing salt header to outfile");
      if(fwrite(salt, 1, sizeof(salt), outfd) != sizeof(salt))
         BARF("Error writing salt to outfile");
   }

   /* At this point, the files are open, the salt has been figured out,
      and we're ready to pump data through crypt. */

   /* Do the crypt operation */
   if(do_crypt(infd, outfd, key, iv, encrypt) != CRYPT_OK)
      BARF("Error during crypt operation");

   /* Clean up */
   fclose(infd); fclose(outfd);
   return 0;
}

/* ref:         $Format:%D$ */
/* git commit:  $Format:%H$ */
/* commit time: $Format:%ai$ */
