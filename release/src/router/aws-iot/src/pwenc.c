#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <openssl/aes.h>
#include <openssl/md5.h>
#include <openssl/sha.h>
#include <openssl/hmac.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>
// #include <bcmnvram.h>
#include <shared.h>
// #include <httpd.h>


#define AES_BLOCK_SIZE 16



char * base64Encode(const char *buffer, int length, int newLine)
{
    BIO *bmem = NULL;
    BIO *b64 = NULL;
    BUF_MEM *bptr;

    b64 = BIO_new(BIO_f_base64());
    if (!newLine) {
        BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    }
    bmem = BIO_new(BIO_s_mem());
    b64 = BIO_push(b64, bmem);
    BIO_write(b64, buffer, length);
    BIO_flush(b64);
    BIO_get_mem_ptr(b64, &bptr);
    BIO_set_close(b64, BIO_NOCLOSE);

    char *buff = (char *)malloc(bptr->length + 1);
    memcpy(buff, bptr->data, bptr->length);
    buff[bptr->length] = 0;
    BIO_free_all(b64);

    return buff;
}


unsigned char *AES_BASE64_Encrypt(unsigned char *src, int srcLen, unsigned char *key, int keyLen, int *outLen)
{
    int i=0;
    EVP_CIPHER_CTX *ctx = NULL;
    char * res = NULL;
    int blockCount = 0;
    int quotient = srcLen / AES_BLOCK_SIZE;
    int mod = srcLen % AES_BLOCK_SIZE;
    unsigned char iv[AES_BLOCK_SIZE];
    blockCount = quotient + 1;

    int padding = AES_BLOCK_SIZE - mod;
    char *in = (char *)malloc(AES_BLOCK_SIZE*blockCount);
    memset(in, padding, AES_BLOCK_SIZE*blockCount);
    memcpy(in, src, srcLen);

    for(i=0; i<AES_BLOCK_SIZE; ++i)
        iv[i]=0;

    //out
    char *out = (char *)malloc(AES_BLOCK_SIZE*blockCount);
    memset(out, 0x00, AES_BLOCK_SIZE*blockCount);
    *outLen = AES_BLOCK_SIZE*blockCount;

    do {
        if(!(ctx = EVP_CIPHER_CTX_new())) {
            // handleOpenSSLErrors();
            break;
        }

        if(1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_ecb(), NULL, key, iv)) {
            // handleOpenSSLErrors();
            break;
        }

        if(1 != EVP_EncryptUpdate(ctx, (unsigned char*)out, outLen, (unsigned char *)in, AES_BLOCK_SIZE*blockCount)) {
            // handleOpenSSLErrors();
            break;
        }
        res = base64Encode(out, *outLen, 0);
    }while(0);

    free(in);
    free(out);
    if (ctx != NULL)
        EVP_CIPHER_CTX_free(ctx);

    return (unsigned char*)res;

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

