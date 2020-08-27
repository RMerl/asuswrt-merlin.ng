//Encodes Base64
#include <string.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>
#include <stdint.h>
#include "base64encode.h"

#include <openssl/sha.h>
#include <openssl/hmac.h>


int Base64Encode(const unsigned char* buffer, size_t length, char** b64text) { //Encodes a binary safe base 64 string
	BIO *bio, *b64;
	BUF_MEM *bufferPtr;

	b64 = BIO_new(BIO_f_base64());
	bio = BIO_new(BIO_s_mem());
	bio = BIO_push(b64, bio);
	//b64 = BIO_push(b64, bio);

	BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL); //Ignore newlines - write everything in one line
	BIO_write(bio, buffer, length);
	BIO_flush(bio);
	BIO_get_mem_ptr(bio, &bufferPtr);
	BIO_set_close(bio, BIO_NOCLOSE);
	BIO_free_all(bio);

	char *buff = (char *)malloc(bufferPtr->length+1);
	memset(buff, 0, bufferPtr->length+1);
	memcpy(buff, bufferPtr->data, bufferPtr->length);

	*b64text= buff;
	
	//*b64text=(*bufferPtr).data;

	return (0); //success
}

char *base64encode(const unsigned char *input, int length)
{
  BIO *bmem, *b64;
  BUF_MEM *bptr;

  b64 = BIO_new(BIO_f_base64());
  bmem = BIO_new(BIO_s_mem());
  b64 = BIO_push(b64, bmem);
  

  //BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL); //Ignore newlines - write everything in one line


  BIO_write(b64, input, length);
  BIO_flush(b64);
  BIO_get_mem_ptr(b64, &bptr);

  char *buff = (char *)malloc(bptr->length);
  //memcpy(buff, bptr->data, bptr->length);
  //buff[bptr->length] = 0;

  memcpy(buff, bptr->data, bptr->length-1);
  buff[bptr->length-1] = 0;

  BIO_free_all(b64);

  return buff;
}
