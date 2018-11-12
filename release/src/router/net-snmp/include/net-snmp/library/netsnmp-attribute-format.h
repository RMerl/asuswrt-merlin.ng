#ifndef _NETSNMP_ATTRIBUTE_FORMAT_H_
#define _NETSNMP_ATTRIBUTE_FORMAT_H_

#if !defined(__GNUC__) || __GNUC__ < 2 || (__GNUC__ == 2 && __GNUC_MINOR__ < 8)
#define NETSNMP_ATTRIBUTE_FORMAT(type, formatArg, firstArg)
#else
#define NETSNMP_ATTRIBUTE_FORMAT(type, formatArg, firstArg) \
  __attribute__((__format__( __ ## type ## __, formatArg, firstArg )))
#endif

#endif /* _NETSNMP_ATTRIBUTE_FORMAT_H_ */
