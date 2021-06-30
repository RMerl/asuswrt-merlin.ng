#ifndef NETSNMP_LIBRARY_OID_H
#define NETSNMP_LIBRARY_OID_H

#if defined(__CYGWIN__) && defined(__LP64__)
/*
 * The winExtDLL implementation assumes that the size of an OID component is
 * 32 bits. Since on the 64-bit Cygwin environment unsigned long is 64 bits
 * wide, use unsigned int instead for oids. See also the definition of
 * SnmpVarBindList on MSDN
 * (https://msdn.microsoft.com/en-us/library/windows/desktop/aa378929.aspx).
 */
typedef unsigned int oid;
#define MAX_SUBID   0xFFFFFFFFUL
#define NETSNMP_PRIo ""
#else
#ifndef EIGHTBIT_SUBIDS
typedef unsigned long oid;
#define MAX_SUBID   0xFFFFFFFFUL
#define NETSNMP_PRIo "l"
#else
typedef unsigned char oid;
#define MAX_SUBID   0xFF
#define NETSNMP_PRIo ""
#endif
#endif /* __CYGWIN64__ */

#endif /* NETSNMP_LIBRARY_OID_H */
