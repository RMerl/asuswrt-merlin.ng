/* LibTomCrypt, modular cryptographic library -- Tom St Denis
 *
 * LibTomCrypt is a library that provides various cryptographic
 * algorithms in a highly modular and flexible manner.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 */

/*
 * Written by Daniel Richards <kyhwana@world-net.co.nz> 6/7/2002
 * hash.c: This app uses libtomcrypt to hash either stdin or a file
 * This file is Public Domain. No rights are reserved.
 * Compile with 'gcc hashsum.c -o hashsum -ltomcrypt'
 * This example isn't really big enough to warrent splitting into
 * more functions ;)
*/

#include <tomcrypt.h>

#if defined(_POSIX_C_SOURCE) && _POSIX_C_SOURCE >= 200112L
#include <libgen.h>
#else
#define basename(x) x
#endif

#if !defined(PATH_MAX) && defined(_MSC_VER)
#include <windows.h>
#define PATH_MAX MAX_PATH
#endif

/* thanks http://stackoverflow.com/a/8198009 */
#define _base(x) ((x >= '0' && x <= '9') ? '0' : \
         (x >= 'a' && x <= 'f') ? 'a' - 10 : \
         (x >= 'A' && x <= 'F') ? 'A' - 10 : \
            '\255')
#define HEXOF(x) (x - _base(x))

static char* hashsum;

static void cleanup(void)
{
   free(hashsum);
}

static void die(int status)
{
   unsigned long w, x;
   FILE* o = status == EXIT_SUCCESS ? stdout : stderr;
   fprintf(o, "usage: %s -a algorithm [-c] [file...]\n\n", hashsum);
   fprintf(o, "\t-c\tCheck the hash(es) of the file(s) written in [file].\n");
   fprintf(o, "\t\t(-a not required)\n");
   fprintf(o, "\nAlgorithms:\n\t");
   w = 0;
   for (x = 0; hash_descriptor[x].name != NULL; x++) {
      w += fprintf(o, "%-14s", hash_descriptor[x].name);
      if (w >= 70) {
         fprintf(o, "\n\t");
         w = 0;
      }
   }
   if (w != 0) fprintf(o, "\n");
   exit(status);
}

static void printf_hex(unsigned char* hash_buffer, unsigned long w)
{
   unsigned long x;
   for (x = 0; x < w; x++) {
       printf("%02x",hash_buffer[x]);
   }
}

static void check_file(int argn, int argc, char **argv)
{
   int err, failed, invalid;
   unsigned char is_buffer[MAXBLOCKSIZE], should_buffer[MAXBLOCKSIZE];
   char buf[PATH_MAX + (MAXBLOCKSIZE * 3)];
   /* iterate through all files */
   while(argn < argc) {
      char* s;
      FILE* f = fopen(argv[argn], "rb");
      if(f == NULL) {
         int n = snprintf(buf, sizeof(buf), "%s: %s", hashsum, argv[argn]);
         if (n > 0 && n < (int)sizeof(buf))
            perror(buf);
         else
            perror(argv[argn]);
         exit(EXIT_FAILURE);
      }
      failed = 0;
      invalid = 0;
      /* read the file line by line */
      while((s = fgets(buf, sizeof(buf), f)) != NULL)
      {
         int tries, n;
         unsigned long hash_len, w, x;
         char* space = strstr(s, " ");

         /* skip lines with comments */
         if (buf[0] == '#') continue;

         if (space == NULL) {
            fprintf(stderr, "%s: no properly formatted checksum lines found\n", hashsum);
            goto ERR;
         }

         hash_len = space - s;
         hash_len /= 2;

         if (hash_len > sizeof(should_buffer)) {
            fprintf(stderr, "%s: hash too long\n", hashsum);
            goto ERR;
         }

         /* convert the hex-string back to binary */
         for (x = 0; x < hash_len; ++x) {
            should_buffer[x] = HEXOF(s[x*2]) << 4 | HEXOF(s[x*2 + 1]);
         }

         space++;
         if (*space != '*') {
            fprintf(stderr, "%s: unsupported input mode '%c'\n", hashsum, *space);
            goto ERR;
         }
         space++;

         for (n = 0; n < (buf + sizeof(buf)) - space; ++n) {
            if(iscntrl((int)space[n])) {
               space[n] = '\0';
               break;
            }
         }

         /* try all hash algorithms that have the appropriate hash size */
         tries = 0;
         for (x = 0; hash_descriptor[x].name != NULL; ++x) {
            if (hash_descriptor[x].hashsize == hash_len) {
               tries++;
               w = sizeof(is_buffer);
               if ((err = hash_file(x, space, is_buffer, &w)) != CRYPT_OK) {
                  fprintf(stderr, "%s: File hash error: %s: %s\n", hashsum, space, error_to_string(err));
ERR:
                  fclose(f);
                  exit(EXIT_FAILURE);
               }
               if(XMEMCMP(should_buffer, is_buffer, w) == 0) {
                  printf("%s: OK\n", space);
                  break;
               }
            }
         } /* for */
         if (hash_descriptor[x].name == NULL) {
            if(tries > 0) {
               printf("%s: FAILED\n", space);
               failed++;
            }
            else {
               invalid++;
            }
         }
      } /* while */
      fclose(f);
      if(invalid) {
         fprintf(stderr, "%s: WARNING: %d %s is improperly formatted\n", hashsum, invalid, invalid > 1?"lines":"line");
      }
      if(failed) {
         fprintf(stderr, "%s: WARNING: %d computed %s did NOT match\n", hashsum, failed, failed > 1?"checksums":"checksum");
      }
      argn++;
   }
   exit(EXIT_SUCCESS);
}

int main(int argc, char **argv)
{
   int idxs[TAB_SIZE], idx, check, y, z, err, argn;
   unsigned long w, x;
   unsigned char hash_buffer[MAXBLOCKSIZE];

   hashsum = strdup(basename(argv[0]));
   atexit(cleanup);

   /* You need to register algorithms before using them */
   register_all_ciphers();
   register_all_hashes();
   if (argc > 1 && (strcmp("-h", argv[1]) == 0 || strcmp("--help", argv[1]) == 0)) {
      die(EXIT_SUCCESS);
   }
   if (argc < 3) {
      die(EXIT_FAILURE);
   }

   for (x = 0; x < sizeof(idxs)/sizeof(idxs[0]); ++x) {
      idxs[x] = -2;
   }
   argn = 1;
   check = 0;
   idx = 0;

   while(argn < argc){
      if(strcmp("-a", argv[argn]) == 0) {
         argn++;
         if(argn < argc) {
            idxs[idx] = find_hash(argv[argn]);
            if (idxs[idx] == -1) {
               struct {
                  const char* is;
                  const char* should;
               } shasum_compat[] =
                     {
#ifdef LTC_SHA1
                           { "1",        sha1_desc.name },
#endif
#ifdef LTC_SHA224
                           { "224",      sha224_desc.name  },
#endif
#ifdef LTC_SHA256
                           { "256",      sha256_desc.name  },
#endif
#ifdef LTC_SHA384
                           { "384",      sha384_desc.name  },
#endif
#ifdef LTC_SHA512
                           { "512",      sha512_desc.name  },
#endif
#ifdef LTC_SHA512_224
                           { "512224",   sha512_224_desc.name  },
#endif
#ifdef LTC_SHA512_256
                           { "512256",   sha512_256_desc.name  },
#endif
                           { NULL, NULL }
                     };
               for (x = 0; shasum_compat[x].is != NULL; ++x) {
                  if(XSTRCMP(shasum_compat[x].is, argv[argn]) == 0) {
                     idxs[idx] = find_hash(shasum_compat[x].should);
                     break;
                  }
               }
            }
            if (idxs[idx] == -1) {
               fprintf(stderr, "%s: Unrecognized algorithm\n", hashsum);
               die(EXIT_FAILURE);
            }
            idx++;
            if ((size_t)idx >= sizeof(idxs)/sizeof(idxs[0])) {
               fprintf(stderr, "%s: Too many '-a' options chosen\n", hashsum);
               die(EXIT_FAILURE);
            }
            argn++;
            continue;
         }
         else {
            die(EXIT_FAILURE);
         }
      }
      if(strcmp("-c", argv[argn]) == 0) {
         check = 1;
         argn++;
         continue;
      }
      break;
   }

   if (check == 1) {
      check_file(argn, argc, argv);
   }

   if (argc == argn) {
      w = sizeof(hash_buffer);
      if ((err = hash_filehandle(idxs[0], stdin, hash_buffer, &w)) != CRYPT_OK) {
         fprintf(stderr, "%s: File hash error: %s\n", hashsum, error_to_string(err));
         return EXIT_FAILURE;
      } else {
          for (x = 0; x < w; x++) {
              printf("%02x",hash_buffer[x]);
          }
          printf(" *-\n");
      }
   } else {
      for (z = argn; z < argc; z++) {
         for (y = 0; y < idx; ++y) {
            w = sizeof(hash_buffer);
            if ((err = hash_file(idxs[y],argv[z],hash_buffer,&w)) != CRYPT_OK) {
               fprintf(stderr, "%s: File hash error: %s\n", hashsum, error_to_string(err));
               return EXIT_FAILURE;
            } else {
                printf_hex(hash_buffer, w);
                printf(" *%s\n", argv[z]);
            }
         }
      }
   }
   return EXIT_SUCCESS;
}

/* ref:         $Format:%D$ */
/* git commit:  $Format:%H$ */
/* commit time: $Format:%ai$ */
