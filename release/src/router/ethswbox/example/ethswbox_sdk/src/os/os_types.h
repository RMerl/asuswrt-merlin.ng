#ifndef _OS_TYPES_H
#define _OS_TYPES_H
/*******************************************************************************

                              Copyright (c) 2009
                            Lantiq Deutschland GmbH
                     Am Campeon 3; 85579 Neubiberg, Germany

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/* Determine the target type */
#if defined(WIN64) || defined (_WIN64) \
   || defined(__x86_64__) \
   || defined(__LP64__) || defined(_LP64) \
   || defined (__amd64) \
   || defined (powerpc64) || defined (__powerpc64__) || defined (__ppc64__) \
   || defined (__64BIT__)
/* X86_64 */
#  ifndef OS_64
#     define OS_64 1
#  endif /* OS_64 */
#elif defined (__ia64__) || defined (__ia64)
/* IA64 */
#  ifndef OS_64
#     define OS_64 1
#  endif /* OS_64 */
#else
/* not 64 bit system */
#endif /* 64 bit determination */

/** \defgroup OS_BASIC_TYPES Basic OS Data Types
    This section describes the Infineon / Lantiq basic data type definitions */
/*@{*/

/** This is the character datatype. */
typedef char                     OS_char_t;
/** This is the integer datatype. */
typedef signed int               OS_int_t;
/** This is the unsigned integer datatype. */
typedef unsigned int             OS_uint_t;
/** This is the unsigned 8-bit datatype. */
typedef unsigned char            OS_uint8_t;
/** This is the signed 8-bit datatype. */
typedef signed char              OS_int8_t;
/** This is the unsigned 16-bit datatype. */
typedef unsigned short           OS_uint16_t;
/** This is the signed 16-bit datatype. */
typedef signed short             OS_int16_t;
/** This is the unsigned 32-bit datatype. */
typedef unsigned int             OS_uint32_t;
/** This is the signed 32-bit datatype. */
typedef signed int               OS_int32_t;
/** This is the float datatype. */
typedef float                    OS_float_t;
/** This is the void datatype.
   It is only a define to be sure, the type
   is "exactly" the same for C++ comatibility! */
#define OS_void_t               void


#if defined(OS_64) && (OS_64 == 1)
   /* NOTE: Most Unix systems use the I32LP64 standard
         which defines a long as 64 bits and Win64 uses
         the IL32LLP64 standard which defines a long as 32 bits.
   */
   #if defined(WIN64)
      /** This is the unsigned 64-bit datatype. */
      typedef unsigned long long int   OS_uint64_t;
      /** This is the signed 64-bit datatype. */
      typedef signed long long int     OS_int64_t;
   #else /* WIN64 */
      /** This is the unsigned 64-bit datatype. */
      typedef unsigned long int        OS_uint64_t;
      /** This is the signed 64-bit datatype. */
      typedef signed long int          OS_int64_t;
   #endif /* WIN64 */

   /** This is the unsigned long datatype.
   On 64 bit systems it is 8 byte wide.
   */
   typedef OS_uint64_t                OS_ulong_t;
   #define HAVE_OS_ULONG_T

   /** This is the signed long datatype.
   On 64 bit systems it is 8 byte wide.
   */
   typedef OS_int64_t                 OS_long_t;
   #define HAVE_OS_LONG_T
#else
   /** This is the unsigned 64-bit datatype. */
   typedef unsigned long long int      OS_uint64_t;
   /** This is the signed 64-bit datatype. */
   typedef signed long long int        OS_int64_t;

   /** This is the unsigned long datatype.
   On 32bit systems it is 4 byte wide.
   */
   typedef unsigned long               OS_ulong_t;
   #define HAVE_OS_ULONG_T

   /** This is the signed long datatype.
   On 32bit systems it is 4 byte wide.
   */
   typedef signed long                 OS_long_t;
   #define HAVE_OS_LONG_T
#endif /* 32/64 bit specific types */


/** This is the size data type (32 or 64 bit) */
typedef OS_ulong_t                    OS_size_t;
#define HAVE_OS_SIZE_T

/** This is the signed size data type (32 or 64 bit) */
typedef OS_long_t                     OS_ssize_t;
#define HAVE_OS_SSIZE_T

/** This is the time data type (32 or 64 bit) */
typedef OS_ulong_t                    OS_time_t;

/* NOTE: (ANSI X3.159-1989)
      While some of these architectures feature uniform pointers
      which are the size of some integer type, maximally portable
      code may not assume any necessary correspondence between
      different pointer types and the integral types.

      Since pointers and integers are now considered incommensurate,
      the only integer that can be safely converted to a pointer
      is the constant 0.
*/
/** Conversion pointer to unsigned values (32 or 64 bit) */
typedef OS_ulong_t           OS_uintptr_t;
#define HAVE_OS_UINTPTR_T
/** Conversion pointer to signed values (32 or 64 bit) */
typedef OS_long_t            OS_intptr_t;
#define HAVE_OS_INTPTR_T

/** This is the volatile unsigned 8-bit datatype. */
typedef volatile OS_uint8_t  OS_vuint8_t;
/** This is the volatile signed 8-bit datatype. */
typedef volatile OS_int8_t   OS_vint8_t;
/** This is the volatile unsigned 16-bit datatype. */
typedef volatile OS_uint16_t OS_vuint16_t;
/** This is the volatile signed 16-bit datatype. */
typedef volatile OS_int16_t  OS_vint16_t;
/** This is the volatile unsigned 32-bit datatype. */
typedef volatile OS_uint32_t OS_vuint32_t;
/** This is the volatile signed 32-bit datatype. */
typedef volatile OS_int32_t  OS_vint32_t;
/** This is the volatile unsigned 64-bit datatype. */
typedef volatile OS_uint64_t OS_vuint64_t;
/** This is the volatile signed 64-bit datatype. */
typedef volatile OS_int64_t  OS_vint64_t;
/** This is the volatile float datatype. */
typedef volatile OS_float_t  OS_vfloat_t;



/** A type for handling boolean issues. */
//typedef enum {
//   /** false */
//   OS_FALSE = 0,
//   /** true */
//   OS_TRUE = 1
//} OS_boolean_t;


/**
   This type is used for parameters that should enable
   and disable a dedicated feature. */
//typedef enum {
//   /** disable */
//   OS_DISABLE = 0,
//   /** enable */
//   OS_ENABLE = 1
//} OS_enDis_t;

/**
   This type is used for parameters that should enable
   and disable a dedicated feature. */
//typedef OS_enDis_t OS_operation_t;

/**
   This type has two states, even and odd.
*/
typedef enum {
   /** even */
   OS_EVEN = 0,
   /** odd */
   OS_ODD = 1
} OS_evenOdd_t;


/**
   This type has two states, high and low.
*/
typedef enum {
    /** low */
   OS_LOW = 0,
   /** high */
   OS_HIGH = 1
} OS_highLow_t;

/**
   This type has two states, success and error
*/
//typedef enum {
// /** operation failed */
//  OS_ERROR   = (-1),
//  /** operation succeeded */
// OS_SUCCESS = 0
//} OS_return_t;

/** NULL pointer */
#ifdef __cplusplus
#define OS_NULL         0
#else
#define OS_NULL         ((OS_void_t *)0)
#endif
/*@}*/ /* OS_BASIC_TYPES */

#endif /* _OS_TYPES_H */
