#ifndef __PWENC_H__
#define __PWENC_H__


char * base64Encode(const char *buffer, int length, int newLine);
// char * base64Decode(char *input, int length, int newLine);

char *str2md5(const char *str, int length);

#endif
