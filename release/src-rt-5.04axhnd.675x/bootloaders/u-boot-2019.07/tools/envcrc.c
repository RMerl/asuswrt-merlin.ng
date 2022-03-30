// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2001
 * Paolo Scaffardi, AIRVENT SAM s.p.a - RIMINI(ITALY), arsenio@tin.it
 */

#include <errno.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <linux/kconfig.h>

#ifndef __ASSEMBLY__
#define	__ASSEMBLY__			/* Dirty trick to get only #defines	*/
#endif
#define	__ASM_STUB_PROCESSOR_H__	/* don't include asm/processor.		*/
#include <config.h>
#undef	__ASSEMBLY__

#if defined(CONFIG_ENV_IS_IN_FLASH)
# ifndef  CONFIG_ENV_ADDR
#  define CONFIG_ENV_ADDR	(CONFIG_SYS_FLASH_BASE + CONFIG_ENV_OFFSET)
# endif
# ifndef  CONFIG_ENV_OFFSET
#  define CONFIG_ENV_OFFSET (CONFIG_ENV_ADDR - CONFIG_SYS_FLASH_BASE)
# endif
# if !defined(CONFIG_ENV_ADDR_REDUND) && defined(CONFIG_ENV_OFFSET_REDUND)
#  define CONFIG_ENV_ADDR_REDUND	(CONFIG_SYS_FLASH_BASE + CONFIG_ENV_OFFSET_REDUND)
# endif
# ifndef  CONFIG_ENV_SIZE
#  define CONFIG_ENV_SIZE	CONFIG_ENV_SECT_SIZE
# endif
# if defined(CONFIG_ENV_ADDR_REDUND) && !defined(CONFIG_ENV_SIZE_REDUND)
#  define CONFIG_ENV_SIZE_REDUND	CONFIG_ENV_SIZE
# endif
# if (CONFIG_ENV_ADDR >= CONFIG_SYS_MONITOR_BASE) && \
     ((CONFIG_ENV_ADDR + CONFIG_ENV_SIZE) <= (CONFIG_SYS_MONITOR_BASE + CONFIG_SYS_MONITOR_LEN))
#  define ENV_IS_EMBEDDED
# endif
# if defined(CONFIG_ENV_ADDR_REDUND) || defined(CONFIG_ENV_OFFSET_REDUND)
#  define CONFIG_SYS_REDUNDAND_ENVIRONMENT
# endif
#endif	/* CONFIG_ENV_IS_IN_FLASH */

#if defined(ENV_IS_EMBEDDED) && !defined(CONFIG_BUILD_ENVCRC)
# define CONFIG_BUILD_ENVCRC
#endif

#ifdef CONFIG_SYS_REDUNDAND_ENVIRONMENT
# define ENV_HEADER_SIZE	(sizeof(uint32_t) + 1)
#else
# define ENV_HEADER_SIZE	(sizeof(uint32_t))
#endif

#define ENV_SIZE (CONFIG_ENV_SIZE - ENV_HEADER_SIZE)


#ifdef CONFIG_BUILD_ENVCRC
# include <environment.h>
extern unsigned int env_size;
extern env_t environment;
#endif	/* CONFIG_BUILD_ENVCRC */

extern uint32_t crc32 (uint32_t, const unsigned char *, unsigned int);

int main (int argc, char **argv)
{
#ifdef CONFIG_BUILD_ENVCRC
	unsigned char pad = 0x00;
	uint32_t crc;
	unsigned char *envptr = (unsigned char *)&environment,
		*dataptr = envptr + ENV_HEADER_SIZE;
	unsigned int datasize = ENV_SIZE;
	unsigned int eoe;

	if (argv[1] && !strncmp(argv[1], "--binary", 8)) {
		int ipad = 0xff;
		if (argv[1][8] == '=')
			sscanf(argv[1] + 9, "%i", &ipad);
		pad = ipad;
	}

	if (pad) {
		/* find the end of env */
		for (eoe = 0; eoe < datasize - 1; ++eoe)
			if (!dataptr[eoe] && !dataptr[eoe+1]) {
				eoe += 2;
				break;
			}
		if (eoe < datasize - 1)
			memset(dataptr + eoe, pad, datasize - eoe);
	}

	crc = crc32 (0, dataptr, datasize);

	/* Check if verbose mode is activated passing a parameter to the program */
	if (argc > 1) {
		if (!strncmp(argv[1], "--binary", 8)) {
			int le = (argc > 2 ? !strcmp(argv[2], "le") : 1);
			size_t i, start, end, step;
			if (le) {
				start = 0;
				end = ENV_HEADER_SIZE;
				step = 1;
			} else {
				start = ENV_HEADER_SIZE - 1;
				end = -1;
				step = -1;
			}
			for (i = start; i != end; i += step)
				printf("%c", (crc & (0xFF << (i * 8))) >> (i * 8));
			if (fwrite(dataptr, 1, datasize, stdout) != datasize)
				fprintf(stderr, "fwrite() failed: %s\n", strerror(errno));
		} else {
			printf("CRC32 from offset %08X to %08X of environment = %08X\n",
				(unsigned int) (dataptr - envptr),
				(unsigned int) (dataptr - envptr) + datasize,
				crc);
		}
	} else {
		printf ("0x%08X\n", crc);
	}
#else
	printf ("0\n");
#endif
	return EXIT_SUCCESS;
}
