/*
 *	The PCI Utilities -- Common Functions
 *
 *	Copyright (c) 1997--2008 Martin Mares <mj@ucw.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>

#include "pciutils.h"

void NONRET
die(char *msg, ...)
{
  va_list args;

  va_start(args, msg);
  fprintf(stderr, "%s: ", program_name);
  vfprintf(stderr, msg, args);
  fputc('\n', stderr);
  exit(1);
}

void *
xmalloc(unsigned int howmuch)
{
  void *p = malloc(howmuch);
  if (!p)
    die("Unable to allocate %d bytes of memory", howmuch);
  return p;
}

void *
xrealloc(void *ptr, unsigned int howmuch)
{
  void *p = realloc(ptr, howmuch);
  if (!p)
    die("Unable to allocate %d bytes of memory", howmuch);
  return p;
}

char *
xstrdup(char *str)
{
  int len = strlen(str) + 1;
  char *copy = xmalloc(len);
  memcpy(copy, str, len);
  return copy;
}

static void
set_pci_method(struct pci_access *pacc, char *arg)
{
  char *name;
  int i;

  if (!strcmp(arg, "help"))
    {
      printf("Known PCI access methods:\n\n");
      for (i=0; name = pci_get_method_name(i); i++)
	if (name[0])
	  printf("%s\n", name);
      exit(0);
    }
  else
    {
      i = pci_lookup_method(arg);
      if (i < 0)
	die("No such PCI access method: %s (see `-A help' for a list)", arg);
      pacc->method = i;
    }
}

static void
set_pci_option(struct pci_access *pacc, char *arg)
{
  if (!strcmp(arg, "help"))
    {
      struct pci_param *p;
      printf("Known PCI access parameters:\n\n");
      for (p=NULL; p=pci_walk_params(pacc, p);)
	printf("%-20s %s (%s)\n", p->param, p->help, p->value);
      exit(0);
    }
  else
    {
      char *sep = strchr(arg, '=');
      if (!sep)
	die("Invalid PCI access parameter syntax: %s", arg);
      *sep++ = 0;
      if (pci_set_param(pacc, arg, sep) < 0)
	die("Unrecognized PCI access parameter: %s (see `-O help' for a list)", arg);
    }
}

int
parse_generic_option(int i, struct pci_access *pacc, char *optarg)
{
  switch (i)
    {
#ifdef PCI_HAVE_PM_INTEL_CONF
    case 'H':
      if (!strcmp(optarg, "1"))
	pacc->method = PCI_ACCESS_I386_TYPE1;
      else if (!strcmp(optarg, "2"))
	pacc->method = PCI_ACCESS_I386_TYPE2;
      else
	die("Unknown hardware configuration type %s", optarg);
      break;
#endif
#ifdef PCI_HAVE_PM_DUMP
    case 'F':
      pci_set_param(pacc, "dump.name", optarg);
      pacc->method = PCI_ACCESS_DUMP;
      break;
#endif
    case 'A':
      set_pci_method(pacc, optarg);
      break;
    case 'G':
      pacc->debugging++;
      break;
    case 'O':
      set_pci_option(pacc, optarg);
      break;
    default:
      return 0;
    }
  return 1;
}
