/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Symbol access for symbols set up by binman as part of the build.
 *
 * This allows C code to access the position of a particular part of the image
 * assembled by binman.
 *
 * Copyright (c) 2017 Google, Inc
 */

#ifndef __BINMAN_SYM_H
#define __BINMAN_SYM_H

#define BINMAN_SYM_MISSING	(-1UL)

#ifdef CONFIG_BINMAN

/**
 * binman_symname() - Internal fnuction to get a binman symbol name
 *
 * @entry_name: Name of the entry to look for (e.g. 'u_boot_spl')
 * @_prop_name: Property value to get from that entry (e.g. 'pos')
 * @returns name of the symbol for that entry and property
 */
#define binman_symname(_entry_name, _prop_name) \
	_binman_ ## _entry_name ## _prop_ ## _prop_name

/**
 * binman_sym_declare() - Declare a symbol that will be used at run-time
 *
 * @_type: Type f the symbol (e.g. unsigned long)
 * @entry_name: Name of the entry to look for (e.g. 'u_boot_spl')
 * @_prop_name: Property value to get from that entry (e.g. 'pos')
 */
#define binman_sym_declare(_type, _entry_name, _prop_name) \
	_type binman_symname(_entry_name, _prop_name) \
		__attribute__((aligned(4), unused, section(".binman_sym")))

/**
 * binman_sym_extern() - Declare a extern symbol that will be used at run-time
 *
 * @_type: Type f the symbol (e.g. unsigned long)
 * @entry_name: Name of the entry to look for (e.g. 'u_boot_spl')
 * @_prop_name: Property value to get from that entry (e.g. 'pos')
 */
#define binman_sym_extern(_type, _entry_name, _prop_name) \
	extern _type binman_symname(_entry_name, _prop_name) \
		__attribute__((aligned(4), unused, section(".binman_sym")))

/**
 * binman_sym_declare_optional() - Declare an optional symbol
 *
 * If this symbol cannot be provided by binman, an error will not be generated.
 * Instead the image will be assigned the value BINMAN_SYM_MISSING.
 *
 * @_type: Type f the symbol (e.g. unsigned long)
 * @entry_name: Name of the entry to look for (e.g. 'u_boot_spl')
 * @_prop_name: Property value to get from that entry (e.g. 'pos')
 */
#define binman_sym_declare_optional(_type, _entry_name, _prop_name) \
	_type binman_symname(_entry_name, _prop_name) \
		__attribute__((aligned(4), weak, unused, \
		section(".binman_sym")))

/**
 * binman_sym() - Access a previously declared symbol
 *
 * This is used to get the value of a symbol. E.g.:
 *
 *    ulong address = binman_sym(ulong, u_boot_spl, pos);
 *
 * @_type: Type f the symbol (e.g. unsigned long)
 * @entry_name: Name of the entry to look for (e.g. 'u_boot_spl')
 * @_prop_name: Property value to get from that entry (e.g. 'pos')
 * @returns value of that property (filled in by binman)
 */
#define binman_sym(_type, _entry_name, _prop_name) \
	(*(_type *)&binman_symname(_entry_name, _prop_name))

#else /* !BINMAN */

#define binman_sym_declare(_type, _entry_name, _prop_name)

#define binman_sym_declare_optional(_type, _entry_name, _prop_name)

#define binman_sym_extern(_type, _entry_name, _prop_name)

#define binman_sym(_type, _entry_name, _prop_name) BINMAN_SYM_MISSING

#endif /* BINMAN */

#endif
