/* -*- mode: c; c-basic-offset: 2 -*- */
/* 
 * Copyright (C) 2003, 2004, 2005 Mondru AB.
 * Copyright (C) 2007-2012 David Bird (Coova Technologies) <support@coova.com>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * 
 */

#define MAIN_FILE

#include "chilli.h"

#define MD5LEN 16

struct options_t _options;

static int usage(char *program) {
  fprintf(stderr, "Usage: %s <challenge> <uamsecret> <password>\n", program);
  fprintf(stderr, "       %s -pap <challenge> <uamsecret> <password>\n", program);
  fprintf(stderr, "       %s -nt <challenge> <uamsecret> <username> <password>\n", program);
  return 1;
}

static int hextochar(char *src, unsigned char * dst, int len) {
  char x[3];
  int n;
  int y;

  for (n=0; n < len; n++) {
    x[0] = src[n*2+0];
    x[1] = src[n*2+1];
    x[2] = 0;

    if (sscanf(x, "%2x", &y) != 1)
      return -1;

    dst[n] = (unsigned char) y;
  }

  return 0;
}

static int chartohex(unsigned char *src, char *dst, int len) {
  char x[3];
  int n;
  
  for (n=0; n < len; n++) {
    safe_snprintf(x, sizeof(x), "%.2x", src[n]);
    dst[n*2+0] = x[0];
    dst[n*2+1] = x[1];
  }
  dst[len*2] = 0;
  return 0;
}

int main(int argc, char **argv) {
  uint8_t chap_ident = 0;
  uint8_t challenge[32];
  char buffer[512];
  MD5_CTX context;

  int idx = 0;
  int usent = 0;
  int usepap = 0;

  if (argc < 2)
    return usage(argv[0]);

  if (!strcmp(argv[1],"-nt")) {
    usent = 1;
    argc--;
    idx++;
  }

  if (!strcmp(argv[1],"-pap")) {
    usepap = 1;
    argc--;
    idx++;
  }

  if (usent && usepap)
    return usage(argv[0]);

  if (argc < 4)
    return usage(argv[0]);

  if (argc == 5) 
    chap_ident = atoi(argv[idx+4]);

  /* challenge - argv 1 */
  memset(buffer, 0, sizeof(buffer));
  strcpy(buffer, argv[idx+1]);
  hextochar(buffer, challenge, MD5LEN);

  /* uamsecret - argv 2 */
  MD5Init(&context);
  MD5Update(&context, challenge, MD5LEN);
  MD5Update(&context, (uint8_t*)argv[idx+2], strlen(argv[idx+2]));
  MD5Final(challenge, &context);

  if (usepap) {
    uint8_t user_password[RADIUS_PWSIZE + 1];
    uint8_t p[RADIUS_PWSIZE + 1];
    int m, n, plen = strlen(argv[idx+3]);
    
    memset(p, 0, sizeof(p));
    safe_strncpy((char *)p, argv[idx+3], RADIUS_PWSIZE);
    
    for (m=0; m < plen;) {
      for (n=0; n < REDIR_MD5LEN; m++, n++) {
	user_password[m] = p[m] ^ challenge[n];
      }
    }
    
    chartohex(user_password, buffer, plen);
    printf("%s\n", buffer);
    
  } else if (usent) {
    
#ifdef HAVE_OPENSSL
    uint8_t ntresponse[24];

    if (argc < 5)
      return usage(argv[0]);

    GenerateNTResponse(challenge, challenge,
		       (uint8_t*)argv[idx+3], strlen(argv[idx+3]),
		       (uint8_t*)argv[idx+4], strlen(argv[idx+4]),
		       ntresponse);
    chartohex(ntresponse, buffer, 24);
    printf("%s\n", buffer);

#else

    printf("Requires OpenSSL Support\n");

#endif

  } else {
    uint8_t response[32];

    /* password - argv 3 */
    MD5Init(&context);
    MD5Update(&context, (uint8_t*)&chap_ident, 1);	  
    MD5Update(&context, (uint8_t*)argv[idx+3], strlen(argv[idx+3]));
    MD5Update(&context, challenge, MD5LEN);
    MD5Final(response, &context);
    
    chartohex(response, buffer, MD5LEN);
    printf("%s\n", buffer);
  }

  return 0;
}
