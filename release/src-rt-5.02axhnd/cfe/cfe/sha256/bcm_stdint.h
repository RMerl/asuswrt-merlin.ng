#ifndef BCM_SEC_STDINT_H
#define BCM_SEC_STDINT_H	1

#include <features.h>
#ifdef __UCLIBC_HAS_WCHAR__
#include <bits/wchar.h>
#endif /* __UCLIBC_HAS_WCHAR__ */
#include <bits/wordsize.h>

/* Fast types.  */

/* Signed.  */
typedef signed char		int_fast8_t;
#if __WORDSIZE == 64
typedef long int		int_fast16_t;
typedef long int		int_fast32_t;
typedef long int		int_fast64_t;
#else
typedef int			int_fast16_t;
typedef int			int_fast32_t;
__extension__
typedef long long int		int_fast64_t;
#endif

/* Unsigned.  */
typedef unsigned char		uint_fast8_t;
#if __WORDSIZE == 64
typedef unsigned long int	uint_fast16_t;
typedef unsigned long int	uint_fast32_t;
typedef unsigned long int	uint_fast64_t;
#else
typedef unsigned int		uint_fast16_t;
typedef unsigned int		uint_fast32_t;
__extension__
typedef unsigned long long int	uint_fast64_t;
#endif

#endif /* BCM_SEC_STDINT_H */
