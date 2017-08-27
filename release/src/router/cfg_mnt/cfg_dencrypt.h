#ifndef __CFG_DENCRYPT_H__
#define __CFG_DENCRYPT_H__

extern unsigned char *cm_aesEncryptMsg(unsigned char *key, int pktType, unsigned char *msg, size_t msgLen, size_t *outLen);
extern unsigned char *cm_aesDecryptMsg(unsigned char *key, unsigned char *key1, unsigned char *msg, size_t msgLen);

#endif /* __CFG_DENCRYPT_H__ */
/* End of cfg_dencrypt.h */
