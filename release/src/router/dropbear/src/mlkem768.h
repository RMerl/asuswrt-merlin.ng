#ifndef MLKEM768_H
#define MLKEM768_H
#define crypto_kem_mlkem768_PUBLICKEYBYTES 1184
#define crypto_kem_mlkem768_SECRETKEYBYTES 2400
#define crypto_kem_mlkem768_CIPHERTEXTBYTES 1088
#define crypto_kem_mlkem768_BYTES 32
int crypto_kem_mlkem768_keypair(unsigned char *pk, unsigned char *sk);
int crypto_kem_mlkem768_enc(unsigned char *c, unsigned char *k, const unsigned char *pk);
int crypto_kem_mlkem768_dec(unsigned char *k, const unsigned char *c, const unsigned char *sk);
#endif /* MLKEM768_H */
