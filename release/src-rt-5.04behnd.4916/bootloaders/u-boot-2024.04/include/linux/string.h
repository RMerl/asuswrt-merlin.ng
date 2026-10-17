#ifndef _LINUX_STRING_H_
#define _LINUX_STRING_H_

#include <linux/types.h>	/* for size_t */
#include <linux/stddef.h>	/* for NULL */

#ifdef __cplusplus
extern "C" {
#endif

extern char * ___strtok;
extern char * strpbrk(const char *,const char *);
extern char * strtok(char *,const char *);
extern char * strsep(char **,const char *);
extern __kernel_size_t strspn(const char *,const char *);


/*
 * Include machine specific inline routines
 */
#include <asm/string.h>

#ifndef __HAVE_ARCH_STRCPY
extern char * strcpy(char *,const char *);
#endif
#ifndef __HAVE_ARCH_STRNCPY
extern char * strncpy(char *,const char *, __kernel_size_t);
#endif
#ifndef __HAVE_ARCH_STRLCPY
size_t strlcpy(char *, const char *, size_t);
#endif
#ifndef __HAVE_ARCH_STRCAT
extern char * strcat(char *, const char *);
#endif
#ifndef __HAVE_ARCH_STRNCAT
extern char * strncat(char *, const char *, __kernel_size_t);
#endif
#ifndef __HAVE_ARCH_STRLCAT
size_t strlcat(char *, const char *, size_t);
#endif
#ifndef __HAVE_ARCH_STRCMP
extern int strcmp(const char *,const char *);
#endif
#ifndef __HAVE_ARCH_STRNCMP
extern int strncmp(const char *,const char *,__kernel_size_t);
#endif
#ifndef __HAVE_ARCH_STRCASECMP
int strcasecmp(const char *s1, const char *s2);
#endif
#ifndef __HAVE_ARCH_STRNCASECMP
extern int strncasecmp(const char *s1, const char *s2, __kernel_size_t len);
#endif
#ifndef __HAVE_ARCH_STRCHR
extern char * strchr(const char *,int);
#endif

/**
 * strchrnul() - return position of a character in the string, or end of string
 *
 * The strchrnul() function is like strchr() except that if c is not found
 * in s, then it returns a pointer to the nul byte at the end of s, rather than
 * NULL
 * @s: string to search
 * @c: character to search for
 * Return: position of @c in @s, or end of @s if not found
 */
const char *strchrnul(const char *s, int c);

#ifndef __HAVE_ARCH_STRRCHR
extern char * strrchr(const char *,int);
#endif
#include <linux/linux_string.h>
#ifndef __HAVE_ARCH_STRSTR
extern char * strstr(const char *,const char *);
#endif
#ifndef __HAVE_ARCH_STRLEN
extern __kernel_size_t strlen(const char *);
#endif
#ifndef __HAVE_ARCH_STRNLEN
extern __kernel_size_t strnlen(const char *,__kernel_size_t);
#endif

#ifndef __HAVE_ARCH_STRCSPN
/**
 * strcspn() - find span of string without given characters
 *
 * Calculates the length of the initial segment of @s which consists entirely
 * of bsytes not in reject.
 *
 * @s: string to search
 * @reject: strings which cause the search to halt
 * Return: number of characters at the start of @s which are not in @reject
 */
size_t strcspn(const char *s, const char *reject);
#endif

#ifdef CONFIG_SANDBOX
# define strdup		sandbox_strdup
# define strndup		sandbox_strndup
#endif

#ifndef __HAVE_ARCH_STRDUP
extern char * strdup(const char *);
extern char * strndup(const char *, size_t);
#endif
#ifndef __HAVE_ARCH_STRSWAB
extern char * strswab(const char *);
#endif

#ifndef __HAVE_ARCH_MEMSET
extern void * memset(void *,int,__kernel_size_t);
#endif
#ifndef __HAVE_ARCH_MEMCPY
extern void * memcpy(void *,const void *,__kernel_size_t);
#endif
#ifndef __HAVE_ARCH_MEMMOVE
extern void * memmove(void *,const void *,__kernel_size_t);
#endif
#ifndef __HAVE_ARCH_MEMSCAN
extern void * memscan(void *,int,__kernel_size_t);
#endif
#ifndef __HAVE_ARCH_MEMCMP
extern int memcmp(const void *,const void *,__kernel_size_t);
#endif
#ifndef __HAVE_ARCH_MEMCHR
extern void * memchr(const void *,int,__kernel_size_t);
#endif
#ifndef __HAVE_ARCH_MEMCHR_INV
void *memchr_inv(const void *, int, size_t);
#endif

/**
 * memdup() - allocate a buffer and copy in the contents
 *
 * Note that this returns a valid pointer even if @len is 0
 *
 * @src: data to copy in
 * @len: number of bytes to copy
 * Return: allocated buffer with the copied contents, or NULL if not enough
 *	memory is available
 *
 */
char *memdup(const void *src, size_t len);

unsigned long ustrtoul(const char *cp, char **endp, unsigned int base);
unsigned long long ustrtoull(const char *cp, char **endp, unsigned int base);

#ifdef __cplusplus
}
#endif

#endif /* _LINUX_STRING_H_ */
