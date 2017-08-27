/*
 *	The PCI Utilities -- Declarations
 *
 *	Copyright (c) 1997--2008 Martin Mares <mj@ucw.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#include "lib/pci.h"
#include "lib/sysdep.h"

#ifdef PCI_OS_WINDOWS
#include "compat/getopt.h"
#else
#include <unistd.h>
#endif

#define PCIUTILS_VERSION PCILIB_VERSION

extern const char program_name[];

void die(char *msg, ...) NONRET PCI_PRINTF(1,2);
void *xmalloc(unsigned int howmuch);
void *xrealloc(void *ptr, unsigned int howmuch);
char *xstrdup(char *str);
int parse_generic_option(int i, struct pci_access *pacc, char *optarg);

#ifdef PCI_HAVE_PM_INTEL_CONF
#define GENOPT_INTEL "H:"
#define GENHELP_INTEL "-H <mode>\tUse direct hardware access (<mode> = 1 or 2)\n"
#else
#define GENOPT_INTEL
#define GENHELP_INTEL
#endif
#if defined(PCI_HAVE_PM_DUMP) && !defined(PCIUTILS_SETPCI)
#define GENOPT_DUMP "F:"
#define GENHELP_DUMP "-F <file>\tRead PCI configuration dump from a given file\n"
#else
#define GENOPT_DUMP
#define GENHELP_DUMP
#endif

#define GENERIC_OPTIONS "A:GO:" GENOPT_INTEL GENOPT_DUMP
#define GENERIC_HELP \
	"-A <method>\tUse the specified PCI access method (see `-A help' for a list)\n" \
	"-O <par>=<val>\tSet PCI access parameter (see `-O help' for a list)\n" \
	"-G\t\tEnable PCI access debugging\n" \
	GENHELP_INTEL GENHELP_DUMP
