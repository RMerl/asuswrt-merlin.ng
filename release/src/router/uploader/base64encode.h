//Encodes Base64
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>
#include <stdint.h>


int Base64Encode(const unsigned char* buffer, size_t length, char** b64text);
char *base64encode(const unsigned char *input, int length);
