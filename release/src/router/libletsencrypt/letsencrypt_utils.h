#ifndef _LETSENCRYPT_UTILS_H
#define _LETSENCRYPT_UTILS_H

int f_diff(const char *path1, const char *path2);
void parse_cert_cn(char *src, char *dst, size_t len);
struct tm * ASN1_TimeToTM(ASN1_TIME* atime, struct tm *t);
u_int16_t pick_random_port(void);

#endif