#ifndef _LINUX_TYPES_H
#define _LINUX_TYPES_H

//#ifndef _MSC_VER
//#error  _MSC_VER not defined
//#endif

#include <sys/types.h>

typedef unsigned __int8 __u8;
typedef signed __int8 __s8;

typedef	signed   __int16	__s16;
typedef	unsigned __int16	__u16;

typedef	signed   __int32	__s32;
typedef	unsigned __int32	__u32;

typedef	signed   __int64	__s64;
typedef	unsigned __int64	__u64;


//typedef __u32 ino_t;
typedef __u32 dev_t;
typedef __u32 uid_t;
typedef __u32 gid_t;

#include <stdint.h>

#endif /* LINUX_TYPES_H */
