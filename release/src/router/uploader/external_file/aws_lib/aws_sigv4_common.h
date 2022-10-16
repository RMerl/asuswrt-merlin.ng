#ifndef __AWS_SIGV4_COMMON_H
#define __AWS_SIGV4_COMMON_H

typedef struct aws_sigv4_str_s {
  unsigned char*  data;
  unsigned int    len;
} aws_sigv4_str_t;

typedef struct aws_sigv4_kv_s {
  aws_sigv4_str_t key;
  aws_sigv4_str_t value;
} aws_sigv4_kv_t;

aws_sigv4_str_t aws_sigv4_string(const unsigned char* cstr);

int aws_sigv4_strcmp(aws_sigv4_str_t* str1, aws_sigv4_str_t* str2);

int aws_sigv4_empty_str(aws_sigv4_str_t* str);

unsigned char* aws_sigv4_sprintf(unsigned char* buf, const char* fmt, ...);

unsigned char* aws_sigv4_snprintf(unsigned char* buf, unsigned int n, const char* fmt, ...);

#endif /* __AWS_SIGV4_COMMON_H */
