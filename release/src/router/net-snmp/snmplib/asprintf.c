#include <net-snmp/net-snmp-config.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef HAVE_ASPRINTF

#ifdef va_copy
#elif defined(__va_copy)
#define va_copy __va_copy
#else
#define va_copy(dest, src) memcpy(&dest, &src, sizeof(va_list))
#endif

NETSNMP_IMPORT
int vasprintf(char **strp, const char *fmt, va_list ap)
{
    va_list ap_copy;
    char *str;
    int len;

    va_copy(ap_copy, ap);
    len = vsnprintf(NULL, 0, fmt, ap_copy);
    va_end(ap_copy);

    if (len < 0)
        return len;

    len++;
    str = malloc(len);
    *strp = str;
    return str ? vsnprintf(str, len, fmt, ap) : -1;
}

NETSNMP_IMPORT
int asprintf(char **strp, const char *fmt, ...)
{
    va_list arg;
    int done;

    va_start(arg, fmt);
    done = vasprintf(strp, fmt, arg);
    va_end(arg);

    return done;
}

#endif
