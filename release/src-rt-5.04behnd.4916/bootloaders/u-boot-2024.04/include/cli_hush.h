/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2001
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#ifndef _CLI_HUSH_H_
#define _CLI_HUSH_H_

#define FLAG_EXIT_FROM_LOOP 1
#define FLAG_PARSE_SEMICOLON (1 << 1)	  /* symbol ';' is special for parser */
#define FLAG_REPARSING       (1 << 2)	  /* >=2nd pass */
#define FLAG_CONT_ON_NEWLINE (1 << 3)	  /* continue when we see \n */

#if CONFIG_IS_ENABLED(HUSH_OLD_PARSER)
extern int u_boot_hush_start(void);
extern int parse_string_outer(const char *str, int flag);
extern int parse_file_outer(void);
int set_local_var(const char *s, int flg_export);
#else
static inline int u_boot_hush_start(void)
{
	return 0;
}

static inline int parse_string_outer(const char *str, int flag)
{
	return 1;
}

static inline int parse_file_outer(void)
{
	return 0;
}

static inline int set_local_var(const char *s, int flg_export)
{
	return 0;
}
#endif
#if CONFIG_IS_ENABLED(HUSH_MODERN_PARSER)
extern int u_boot_hush_start_modern(void);
extern int parse_string_outer_modern(const char *str, int flag);
extern void parse_and_run_file(void);
int set_local_var_modern(char *s, int flg_export);
#else
static inline int u_boot_hush_start_modern(void)
{
	return 0;
}

static inline int parse_string_outer_modern(const char *str, int flag)
{
	return 1;
}

static inline void parse_and_run_file(void)
{
}

static inline int set_local_var_modern(char *s, int flg_export)
{
	return 0;
}
#endif

void unset_local_var(const char *name);
char *get_local_var(const char *s);

#if defined(CONFIG_HUSH_INIT_VAR)
extern int hush_init_var (void);
#endif
#endif
