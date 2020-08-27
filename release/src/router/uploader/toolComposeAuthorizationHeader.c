#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
//#include <math.h> 
//#include <uuid/uuid.h>
//#include <sys/time.h>
#include <openssl/sha.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/md5.h>

char* getUUID(void);
void removeAllChars(char* str, char c);
char* getTimeInMillis(void);
char *urlencode(char *pstr);
void hmac_sha1_hex(u_int8_t * digest, u_int8_t * key, u_int32_t keylen, u_int8_t * text, u_int32_t textlen);
int hexHextoDecimal(char* signaturePreA);
int htoi(char chr);
//int Base64Encode(const unsigned char* buffer, size_t length, char** b64text);
char to_hex(char code);

char authorization[512];
unsigned char result[20];
char* uuid_str;
char* timestamp_str;

int tool_composeAuthorizationHeader(char *progKey)
{ 
  char signaturePreA[512];
  char signaturePreB[512];
  char* base64EncodeOutput;
  
  // Step 1, Compose signature string by uuid(nonce) and timestamp
  uuid_str = malloc(sizeof(char) * 37);
  timestamp_str = malloc(sizeof(char) * 128);
  snprintf(signaturePreA,sizeof(signaturePreA),"nonce=%s&signature_method=HMAC-SHA1&timestamp=%s",getUUID(),getTimeInMillis());
  
  // Step 2, Doing urlencode before doing hash
  snprintf(signaturePreB,sizeof(signaturePreB),"%s",urlencode(signaturePreA));
  
  // Step 3, Doing hash signature string by HMAC-SHA1
  hmac_sha1_hex(signaturePreA, progKey, strlen(progKey), signaturePreB, strlen(signaturePreB));
  hexHextoDecimal(signaturePreA);
  
  // Step 4, Doing base64 encoding & doing urlencode again
  Base64Encode(result, sizeof(result), &base64EncodeOutput);  
  snprintf(signaturePreB,sizeof(signaturePreB),"%s",urlencode(base64EncodeOutput));
  free(base64EncodeOutput);
  
  // Final step, Put all parameters to be authorization header string
  // ex: authorization[]="signature_method=\"HMAC-SHA1\",timestamp=\"1438678032743\",nonce=\"b75b53d49eac469dbfdc4c9d0160b247\",signature=\"Yjk1OWQwYTk2M2RmMTAwMTJlMTY2YTI0M2MyYzUwOGMzOWVlOTIzNw%3D%3D\"";
  snprintf(authorization,sizeof(authorization),"Authorization: signature_method=\"HMAC-SHA1\",timestamp=\"%s\",nonce=\"%s\",signature=\"%s\"",timestamp_str,uuid_str,signaturePreB);   
	printf("Authorization: signature_method=\"HMAC-SHA1\",timestamp=\"%s\",nonce=\"%s\",signature=\"%s\"\n",timestamp_str,uuid_str,signaturePreB);  
  free(uuid_str);
  free(timestamp_str);
}

char* getUUID()
{

  //ToDo: bypass to use const value
  /*
  // typedef unsigned char uuid_t[16];
  uuid_t uuid;
  // generate
  uuid_generate_random(uuid);
  // unparse (to string) uuid: "8-4-4-4-12"      
  uuid_unparse(uuid, uuid_str);
  // remove '-'
  removeAllChars(uuid_str,'-');
  */
  
  sprintf(uuid_str,"ad071827ac174952aeddef63c12831d2");  
  return uuid_str;
}

char* getTimeInMillis()
{

/*
struct timeval t_val;
gettimeofday(&t_val, NULL);
printf("start, now, sec=%ld m_sec=%d \n", t_val.tv_sec, t_val.tv_usec);
long sec = t_val.tv_sec;
time_t t_sec = (time_t)sec;
printf("date:%s", ctime(&t_sec));
*/
  //ToDo: bypass to use const value
  
  char *ptr;
  unsigned long long lval = 0;
  struct timeval tp;  
  gettimeofday(&tp, NULL);
  //lval = (unsigned long long)((tp.tv_sec * 1000 * 1000) + (tp.tv_usec)) / 1000; 

  lval = (unsigned long long)(tp.tv_sec * 1000 * 1000 ) + (tp.tv_usec); 
  sprintf(timestamp_str,"%llu",lval);
 
  //sprintf(timestamp_str,"1399463965238");
  return timestamp_str; 
}

void removeAllChars(char* str, char c) {
  char *pr=str, *pw=str;
  while (*pr) {
      *pw = *pr++;
      pw += (*pw != c);
  }
  *pw = '\0';
}

char to_hex(char code) {
  static char hex[] = "0123456789ABCDEF";
  return hex[code & 15];
}

char *urlencode(char *str) {
  char *pstr = str, *buf = malloc(strlen(str) * 3 + 1), *pbuf = buf;

  while (*pstr) {
    if (isalnum(*pstr) || *pstr == '-' || *pstr == '_' || *pstr == '.' || *pstr == '~') 
      *pbuf++ = *pstr;
    else if (*pstr == ' ') 
      *pbuf++ = '+';
    else 
      *pbuf++ = '%', *pbuf++ = to_hex(*pstr >> 4), *pbuf++ = to_hex(*pstr & 15);
    pstr++;
  }
  *pbuf = '\0';
  return buf;
}

void hmac_sha1_hex(u_int8_t * digest, u_int8_t * key, u_int32_t keylen,
              u_int8_t * text, u_int32_t textlen)
{
    u_int8_t        md[20];
    u_int8_t        mdkey[20];
    u_int8_t        k_ipad[64],k_opad[64];
    unsigned int    i;
    char            s[3];

    if (keylen > 64) {
        SHA_CTX         ctx;
        SHA1_Init(&ctx);
        SHA1_Update(&ctx, key, keylen);
        SHA1_Final(mdkey, &ctx);
        keylen = 20;
        key = mdkey;
    }

    memcpy(k_ipad, key, keylen);
    memcpy(k_opad, key, keylen);
    memset(k_ipad + keylen, 0, 64 - keylen);
    memset(k_opad + keylen, 0, 64 - keylen);

    for (i = 0; i < 64; i++) {
        k_ipad[i] ^= 0x36;
        k_opad[i] ^= 0x5c;
    }

    SHA_CTX         ctx;
    
    SHA1_Init(&ctx);
    SHA1_Update(&ctx, k_ipad, 64);
    SHA1_Update(&ctx, text, textlen);
    SHA1_Final(md, &ctx);

    SHA1_Init(&ctx);
    SHA1_Update(&ctx, k_opad, 64);
    SHA1_Update(&ctx, md, 20);
    SHA1_Final(md, &ctx);
    
    for (i = 0; i < 20; i++) {
        snprintf(s, 3, "%02x", md[i]);
        digest[2 * i] = s[0];
        digest[2 * i + 1] = s[1];
    }    
    digest[40] = '\0';
}

/*
int Base64Encode(const unsigned char* buffer, size_t length, char** b64text) {    
	BIO *bio, *b64;
	BUF_MEM *bufferPtr;

	b64 = BIO_new(BIO_f_base64());
	bio = BIO_new(BIO_s_mem());
	bio = BIO_push(b64, bio);

    //Ignore newlines - write everything in one line
	BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL); 
	BIO_write(bio, buffer, length);
	BIO_flush(bio);
	BIO_get_mem_ptr(bio, &bufferPtr);
	BIO_set_close(bio, BIO_NOCLOSE);
	//BIO_free_all(bio);
  
	//*b64text=(*bufferPtr).data;
	*b64text = (char*) malloc((bufferPtr->length + 1) * sizeof(char));
	memcpy(*b64text, bufferPtr->data, bufferPtr->length);
	(*b64text)[bufferPtr->length] = '\0';

	BIO_free_all(bio);
	
	return (0);
}
*/
int hexHextoDecimal(char* signaturePreA)
{ 
   int hmac_sha1[20];
   int i;
   
   for(i=0;i<20;i++)
   {
     hmac_sha1[i]= (htoi(signaturePreA[i*2]))*16 + htoi(signaturePreA[i*2+1]);
     result[i]=(unsigned char)hmac_sha1[i];
   }
}

int htoi(char chr)
{
    int c;    
    c = chr;
    if ( c >= 'a' && c <= 'f')
        c = c - 'a' + 10;
    else if (c >= 'A' && c <= 'F')
        c = c - 'A' + 10;
    else if (c >= '0' && c <= '9')
        c -= '0';
    return c;
}

/* for the md5(password)of payload */
char *str2md5(const char *str, int length) {
    int n;
    MD5_CTX c;
    unsigned char digest[16];
    char *out = (char*)malloc(33);

    MD5_Init(&c);

    while (length > 0) {
        if (length > 512) {
            MD5_Update(&c, str, 512);
        } else {
            MD5_Update(&c, str, length);
        }
        length -= 512;
        str += 512;
    }

    MD5_Final(digest, &c);

    for (n = 0; n < 16; ++n) {
        snprintf(&(out[n*2]), 16*2, "%02x", (unsigned int)digest[n]);
    }

    return out;
}