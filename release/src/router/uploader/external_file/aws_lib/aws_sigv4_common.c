#include <string.h>
#include <stdarg.h>
#include "aws_sigv4_common.h"

int aws_sigv4_empty_str(aws_sigv4_str_t* str)
{
  return (str == NULL || str->data == NULL || str->len == 0) ? 1 : 0;
}

aws_sigv4_str_t aws_sigv4_string(const unsigned char* cstr)
{
  aws_sigv4_str_t ret = { .data = NULL };
  if (cstr)
  {
    ret.data = (unsigned char*) cstr;
    ret.len  = strlen((char*) cstr);
  }
  return ret;
}

int aws_sigv4_strcmp(aws_sigv4_str_t* str1, aws_sigv4_str_t* str2)
{
  size_t len = str1->len <= str2->len ? str1->len : str2->len;
  return strncmp((char*) str1->data, (char*) str2->data, len);
}

/* reference: http://lxr.nginx.org/source/src/core/ngx_string.c */
static unsigned char* aws_sigv4_vslprintf(unsigned char* buf, unsigned char* last,
                                          const char* fmt, va_list args)
{
  unsigned char*    c_ptr = buf;
  aws_sigv4_str_t*  str;

  while (*fmt && c_ptr < last)
  {
    size_t n_max = last - c_ptr;
    if (*fmt == '%')
    {
      if (*(fmt + 1) == 'V')
      {
        str = va_arg(args, aws_sigv4_str_t *);
        if (aws_sigv4_empty_str(str))
        {
          goto finished;
        }
        size_t cp_len = n_max >= str->len ? str->len : n_max;
        strncpy((char*) c_ptr, (char*) str->data, cp_len);
        c_ptr += cp_len;
        fmt += 2;
      }
      else
      {
        *(c_ptr++) = *(fmt++);
      }
    }
    else
    {
      *(c_ptr++) = *(fmt++);
    }
  }
  *c_ptr = '\0';
finished:
  return c_ptr;
}

unsigned char* aws_sigv4_sprintf(unsigned char* buf, const char* fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  unsigned char* dst = aws_sigv4_vslprintf(buf, (void*) -1, fmt, args);
  va_end(args);
  return dst;
}

unsigned char* aws_sigv4_snprintf(unsigned char* buf, unsigned int n, const char* fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  unsigned char* dst = aws_sigv4_vslprintf(buf, buf + n, fmt, args);
  va_end(args);
  return dst;
}
