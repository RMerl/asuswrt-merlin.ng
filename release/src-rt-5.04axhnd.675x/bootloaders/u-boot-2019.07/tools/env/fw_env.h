/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2002-2008
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#include <stdint.h>

/*
 * Programs using the library must check which API is available,
 * that varies depending on the U-Boot version.
 * This can be changed in future
 */
#define FW_ENV_API_VERSION	1

struct env_opts {
#ifdef CONFIG_FILE
	char *config_file;
#endif
	char *lockname;
};

/**
 * fw_printenv() - print one or several environment variables
 *
 * @argc: number of variables names to be printed, prints all if 0
 * @argv: array of variable names to be printed, if argc != 0
 * @value_only: do not repeat the variable name, print the bare value,
 *          only one variable allowed with this option, argc must be 1
 * @opts: encryption key, configuration file, defaults are used if NULL
 *
 * Description:
 *  Uses fw_env_open, fw_getenv
 *
 * Return:
 *  0 on success, -1 on failure (modifies errno)
 */
int fw_printenv(int argc, char *argv[], int value_only, struct env_opts *opts);

/**
 * fw_env_set() - adds or removes one variable to the environment
 *
 * @argc: number of strings in argv, argv[0] is variable name,
 *          argc==1 means erase variable, argc > 1 means add a variable
 * @argv: argv[0] is variable name, argv[1..argc-1] are concatenated separated
 *           by single blank and set as the new value of the variable
 * @opts: how to retrieve environment from flash, defaults are used if NULL
 *
 * Description:
 *  Uses fw_env_open, fw_env_write, fw_env_flush
 *
 * Return:
 *  0 on success, -1 on failure (modifies errno)
 *
 * ERRORS:
 *  EROFS - some variables ("ethaddr", "serial#") cannot be modified
 */
int fw_env_set(int argc, char *argv[], struct env_opts *opts);

/**
 * fw_parse_script() - adds or removes multiple variables with a batch script
 *
 * @fname: batch script file name
 * @opts: encryption key, configuration file, defaults are used if NULL
 *
 * Description:
 *  Uses fw_env_open, fw_env_write, fw_env_flush
 *
 * Return:
 *  0 success, -1 on failure (modifies errno)
 *
 * Script Syntax:
 *
 *  key [ [space]+ value]
 *
 *  lines starting with '#' treated as comment
 *
 *  A variable without value will be deleted. Any number of spaces are allowed
 *  between key and value. The value starts with the first non-space character
 *  and ends with newline. No comments allowed on these lines.  Spaces inside
 *  the value are preserved verbatim.
 *
 * Script Example:
 *
 *  netdev         eth0
 *
 *  kernel_addr    400000
 *
 *  foo            spaces           are copied verbatim
 *
 *  # delete variable bar
 *
 *  bar
 */
int fw_parse_script(char *fname, struct env_opts *opts);


/**
 * fw_env_open() - read enviroment from flash into RAM cache
 *
 * @opts: encryption key, configuration file, defaults are used if NULL
 *
 * Return:
 *  0 on success, -1 on failure (modifies errno)
 */
int fw_env_open(struct env_opts *opts);

/**
 * fw_getenv() - lookup variable in the RAM cache
 *
 * @name: variable to be searched
 * Return:
 *  pointer to start of value, NULL if not found
 */
char *fw_getenv(char *name);

/**
 * fw_env_write() - modify a variable held in the RAM cache
 *
 * @name: variable
 * @value: delete variable if NULL, otherwise create or overwrite the variable
 *
 * This is called in sequence to update the environment in RAM without updating
 * the copy in flash after each set
 *
 * Return:
 *  0 on success, -1 on failure (modifies errno)
 *
 * ERRORS:
 *  EROFS - some variables ("ethaddr", "serial#") cannot be modified
 */
int fw_env_write(char *name, char *value);

/**
 * fw_env_flush - write the environment from RAM cache back to flash
 *
 * @opts: encryption key, configuration file, defaults are used if NULL
 *
 * Return:
 *  0 on success, -1 on failure (modifies errno)
 */
int fw_env_flush(struct env_opts *opts);

/**
 * fw_env_close - free allocated structure and close env
 *
 * @opts: encryption key, configuration file, defaults are used if NULL
 *
 * Return:
 *  0 on success, -1 on failure (modifies errno)
 */
int fw_env_close(struct env_opts *opts);


/**
 * fw_env_version - return the current version of the library
 *
 * Return:
 *  version string of the library
 */
char *fw_env_version(void);

unsigned long crc32(unsigned long, const unsigned char *, unsigned);
