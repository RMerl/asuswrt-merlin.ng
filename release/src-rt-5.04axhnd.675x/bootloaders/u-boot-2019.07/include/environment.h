/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#ifndef _ENVIRONMENT_H_
#define _ENVIRONMENT_H_

#include <linux/kconfig.h>

/**************************************************************************
 *
 * The "environment" is stored as a list of '\0' terminated
 * "name=value" strings. The end of the list is marked by a double
 * '\0'. New entries are always added at the end. Deleting an entry
 * shifts the remaining entries to the front. Replacing an entry is a
 * combination of deleting the old value and adding the new one.
 *
 * The environment is preceded by a 32 bit CRC over the data part.
 *
 *************************************************************************/

#if defined(CONFIG_ENV_IS_IN_FLASH)
# ifndef	CONFIG_ENV_ADDR
#  define	CONFIG_ENV_ADDR	(CONFIG_SYS_FLASH_BASE + CONFIG_ENV_OFFSET)
# endif
# ifndef	CONFIG_ENV_OFFSET
#  define	CONFIG_ENV_OFFSET (CONFIG_ENV_ADDR - CONFIG_SYS_FLASH_BASE)
# endif
# if !defined(CONFIG_ENV_ADDR_REDUND) && defined(CONFIG_ENV_OFFSET_REDUND)
#  define	CONFIG_ENV_ADDR_REDUND	\
		(CONFIG_SYS_FLASH_BASE + CONFIG_ENV_OFFSET_REDUND)
# endif
# if defined(CONFIG_ENV_SECT_SIZE) || defined(CONFIG_ENV_SIZE)
#  ifndef	CONFIG_ENV_SECT_SIZE
#   define	CONFIG_ENV_SECT_SIZE	CONFIG_ENV_SIZE
#  endif
#  ifndef	CONFIG_ENV_SIZE
#   define	CONFIG_ENV_SIZE	CONFIG_ENV_SECT_SIZE
#  endif
# else
#  error "Both CONFIG_ENV_SECT_SIZE and CONFIG_ENV_SIZE undefined"
# endif
# if defined(CONFIG_ENV_ADDR_REDUND) && !defined(CONFIG_ENV_SIZE_REDUND)
#  define CONFIG_ENV_SIZE_REDUND	CONFIG_ENV_SIZE
# endif
# if	(CONFIG_ENV_ADDR >= CONFIG_SYS_MONITOR_BASE) &&		\
	(CONFIG_ENV_ADDR + CONFIG_ENV_SIZE) <=			\
	(CONFIG_SYS_MONITOR_BASE + CONFIG_SYS_MONITOR_LEN)
#  define ENV_IS_EMBEDDED
# endif
# if defined(CONFIG_ENV_ADDR_REDUND) || defined(CONFIG_ENV_OFFSET_REDUND)
#  define CONFIG_SYS_REDUNDAND_ENVIRONMENT
# endif
# ifdef CONFIG_ENV_IS_EMBEDDED
#  error "do not define CONFIG_ENV_IS_EMBEDDED in your board config"
#  error "it is calculated automatically for you"
# endif
#endif	/* CONFIG_ENV_IS_IN_FLASH */

#if defined(CONFIG_ENV_IS_IN_MMC)
# ifdef CONFIG_ENV_OFFSET_REDUND
#  define CONFIG_SYS_REDUNDAND_ENVIRONMENT
# endif
#endif

#if defined(CONFIG_ENV_IS_IN_NAND)
# if defined(CONFIG_ENV_OFFSET_OOB)
#  ifdef CONFIG_ENV_OFFSET_REDUND
#   error "CONFIG_ENV_OFFSET_REDUND is not supported when CONFIG_ENV_OFFSET_OOB"
#   error "is set"
#  endif
extern unsigned long nand_env_oob_offset;
#  define CONFIG_ENV_OFFSET nand_env_oob_offset
# else
#  ifndef CONFIG_ENV_OFFSET
#   error "Need to define CONFIG_ENV_OFFSET when using CONFIG_ENV_IS_IN_NAND"
#  endif
#  ifdef CONFIG_ENV_OFFSET_REDUND
#   define CONFIG_SYS_REDUNDAND_ENVIRONMENT
#  endif
# endif /* CONFIG_ENV_OFFSET_OOB */
# ifndef CONFIG_ENV_SIZE
#  error "Need to define CONFIG_ENV_SIZE when using CONFIG_ENV_IS_IN_NAND"
# endif
#endif /* CONFIG_ENV_IS_IN_NAND */

#if defined(CONFIG_ENV_IS_IN_UBI)
# ifndef CONFIG_ENV_UBI_PART
#  error "Need to define CONFIG_ENV_UBI_PART when using CONFIG_ENV_IS_IN_UBI"
# endif
# ifndef CONFIG_ENV_UBI_VOLUME
#  error "Need to define CONFIG_ENV_UBI_VOLUME when using CONFIG_ENV_IS_IN_UBI"
# endif
# if defined(CONFIG_ENV_UBI_VOLUME_REDUND)
#  define CONFIG_SYS_REDUNDAND_ENVIRONMENT
# endif
# ifndef CONFIG_ENV_SIZE
#  error "Need to define CONFIG_ENV_SIZE when using CONFIG_ENV_IS_IN_UBI"
# endif
# ifndef CONFIG_CMD_UBI
#  error "Need to define CONFIG_CMD_UBI when using CONFIG_ENV_IS_IN_UBI"
# endif
#endif /* CONFIG_ENV_IS_IN_UBI */

/* Embedded env is only supported for some flash types */
#ifdef CONFIG_ENV_IS_EMBEDDED
# if	!defined(CONFIG_ENV_IS_IN_FLASH)	&& \
	!defined(CONFIG_ENV_IS_IN_NAND)		&& \
	!defined(CONFIG_ENV_IS_IN_ONENAND)	&& \
	!defined(CONFIG_ENV_IS_IN_SPI_FLASH)
#  error "CONFIG_ENV_IS_EMBEDDED not supported for your flash type"
# endif
#endif

/*
 * For the flash types where embedded env is supported, but it cannot be
 * calculated automatically (i.e. NAND), take the board opt-in.
 */
#if defined(CONFIG_ENV_IS_EMBEDDED) && !defined(ENV_IS_EMBEDDED)
# define ENV_IS_EMBEDDED
#endif

/* The build system likes to know if the env is embedded */
#ifdef DO_DEPS_ONLY
# ifdef ENV_IS_EMBEDDED
#  ifndef CONFIG_ENV_IS_EMBEDDED
#   define CONFIG_ENV_IS_EMBEDDED
#  endif
# endif
#endif

#include "compiler.h"

#ifdef CONFIG_SYS_REDUNDAND_ENVIRONMENT
# define ENV_HEADER_SIZE	(sizeof(uint32_t) + 1)

# define ACTIVE_FLAG   1
# define OBSOLETE_FLAG 0
#else
# define ENV_HEADER_SIZE	(sizeof(uint32_t))
#endif

#define ENV_SIZE (CONFIG_ENV_SIZE - ENV_HEADER_SIZE)

typedef struct environment_s {
	uint32_t	crc;		/* CRC32 over data bytes	*/
#ifdef CONFIG_SYS_REDUNDAND_ENVIRONMENT
	unsigned char	flags;		/* active/obsolete flags	*/
#endif
	unsigned char	data[ENV_SIZE]; /* Environment data		*/
} env_t;

#ifdef ENV_IS_EMBEDDED
extern env_t environment;
#endif /* ENV_IS_EMBEDDED */

extern const unsigned char default_environment[];

#if defined(CONFIG_NEEDS_MANUAL_RELOC)
extern void env_reloc(void);
#endif

#ifndef DO_DEPS_ONLY

#include <env_attr.h>
#include <env_callback.h>
#include <env_flags.h>
#include <search.h>

/* Value for environment validity */
enum env_valid {
	ENV_INVALID,	/* No valid environment */
	ENV_VALID,	/* First or only environment is valid */
	ENV_REDUND,	/* Redundant environment is valid */
};

enum env_location {
	ENVL_UNKNOWN,
	ENVL_EEPROM,
	ENVL_EXT4,
	ENVL_FAT,
	ENVL_FLASH,
	ENVL_MMC,
	ENVL_NAND,
	ENVL_NVRAM,
	ENVL_ONENAND,
	ENVL_REMOTE,
	ENVL_SPI_FLASH,
	ENVL_UBI,
	ENVL_BOOT_MAGIC,
	ENVL_NOWHERE,

	ENVL_COUNT,
};

/* value for the various operations we want to perform on the env */
enum env_operation {
	ENVOP_GET_CHAR,	/* we want to call the get_char function */
	ENVOP_INIT,	/* we want to call the init function */
	ENVOP_LOAD,	/* we want to call the load function */
	ENVOP_SAVE,	/* we want to call the save function */
};

struct env_driver {
	const char *name;
	enum env_location location;

	/**
	 * load() - Load the environment from storage
	 *
	 * This method is optional. If not provided, no environment will be
	 * loaded.
	 *
	 * @return 0 if OK, -ve on error
	 */
	int (*load)(void);

	/**
	 * save() - Save the environment to storage
	 *
	 * This method is required for 'saveenv' to work.
	 *
	 * @return 0 if OK, -ve on error
	 */
	int (*save)(void);

	/**
	 * init() - Set up the initial pre-relocation environment
	 *
	 * This method is optional.
	 *
	 * @return 0 if OK, -ENOENT if no initial environment could be found,
	 * other -ve on error
	 */
	int (*init)(void);
};

/* Declare a new environment location driver */
#define U_BOOT_ENV_LOCATION(__name)					\
	ll_entry_declare(struct env_driver, __name, env_driver)

/* Declare the name of a location */
#ifdef CONFIG_CMD_SAVEENV
#define ENV_NAME(_name) .name = _name,
#else
#define ENV_NAME(_name)
#endif

#ifdef CONFIG_CMD_SAVEENV
#define env_save_ptr(x) x
#else
#define env_save_ptr(x) NULL
#endif

extern struct hsearch_data env_htab;

/* Function that updates CRC of the enironment */
void env_crc_update(void);

/* Look up the variable from the default environment */
char *env_get_default(const char *name);

/* [re]set to the default environment */
void set_default_env(const char *s, int flags);

/* [re]set individual variables to their value in the default environment */
int set_default_vars(int nvars, char * const vars[], int flags);

/* Import from binary representation into hash table */
int env_import(const char *buf, int check);

/* Export from hash table into binary representation */
int env_export(env_t *env_out);

#ifdef CONFIG_SYS_REDUNDAND_ENVIRONMENT
/* Select and import one of two redundant environments */
int env_import_redund(const char *buf1, int buf1_status,
		      const char *buf2, int buf2_status);
#endif

/**
 * env_get_char() - Get a character from the early environment
 *
 * This reads from the pre-relocation environment
 *
 * @index: Index of character to read (0 = first)
 * @return character read, or -ve on error
 */
int env_get_char(int index);

/**
 * env_load() - Load the environment from storage
 *
 * @return 0 if OK, -ve on error
 */
int env_load(void);

/**
 * env_save() - Save the environment to storage
 *
 * @return 0 if OK, -ve on error
 */
int env_save(void);

/**
 * env_fix_drivers() - Updates envdriver as per relocation
 */
void env_fix_drivers(void);

void eth_parse_enetaddr(const char *addr, uint8_t *enetaddr);
int eth_env_get_enetaddr(const char *name, uint8_t *enetaddr);
int eth_env_set_enetaddr(const char *name, const uint8_t *enetaddr);

#endif /* DO_DEPS_ONLY */

#endif /* _ENVIRONMENT_H_ */
