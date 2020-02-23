#ifdef CONFIG_BCM_KF_MIPS_4350
#include <stdlib.h>
#endif

#ifdef CONFIG_BCM_KF_MIPS_4350
#if !defined(__UCLIBC__)
#include <gnu/libc-version.h>
#else
#define XSTR(s) STR(s)
#define STR(s) #s
#endif
#else
#include <gnu/libc-version.h>
#endif

int main(void)
{
#ifdef CONFIG_BCM_KF_MIPS_4350
#if !defined(__UCLIBC__)
 	const char *version = gnu_get_libc_version();
 #else	
	const char *version = XSTR(__GLIBC__) "." XSTR(__GLIBC_MINOR__);
#endif	
#else
	const char *version = gnu_get_libc_version();
#endif 
 	return (long)version;
}
