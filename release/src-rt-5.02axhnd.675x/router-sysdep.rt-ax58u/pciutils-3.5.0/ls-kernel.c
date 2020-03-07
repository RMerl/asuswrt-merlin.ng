/*
 *	The PCI Utilities -- Show Kernel Drivers
 *
 *	Copyright (c) 1997--2013 Martin Mares <mj@ucw.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "lspci.h"

#ifdef PCI_OS_LINUX

#include <sys/utsname.h>

#ifdef PCI_USE_LIBKMOD

#include <libkmod.h>

static struct kmod_ctx *kmod_ctx;

static int
show_kernel_init(void)
{
  static int show_kernel_inited = -1;
  if (show_kernel_inited >= 0)
    return show_kernel_inited;

  struct utsname uts;
  if (uname(&uts) < 0)
    die("uname() failed: %m");
  char *name = alloca(64 + strlen(uts.release));
  sprintf(name, "/lib/modules/%s", uts.release);

  kmod_ctx = kmod_new(name, NULL);
  if (!kmod_ctx)
    {
      fprintf(stderr, "lspci: Unable to initialize libkmod context\n");
      goto failed;
    }

  int err;
  if ((err = kmod_load_resources(kmod_ctx)) < 0)
    {
      fprintf(stderr, "lspci: Unable to load libkmod resources: error %d\n", err);
      goto failed;
    }

  show_kernel_inited = 1;
  return 1;

failed:
  show_kernel_inited = 0;
  return 0;
}

void
show_kernel_cleanup(void)
{
  if (kmod_ctx)
    kmod_unref(kmod_ctx);
}

static const char *next_module(struct device *d)
{
  static struct kmod_list *klist, *kcurrent;
  static struct kmod_module *kmodule;

  if (kmodule)
    {
      kmod_module_unref(kmodule);
      kmodule = NULL;
    }

  if (!klist)
    {
      pci_fill_info(d->dev, PCI_FILL_MODULE_ALIAS);
      if (!d->dev->module_alias)
	return NULL;
      int err = kmod_module_new_from_lookup(kmod_ctx, d->dev->module_alias, &klist);
      if (err < 0)
	{
	  fprintf(stderr, "lspci: libkmod lookup failed: error %d\n", err);
	  return NULL;
	}
      kcurrent = klist;
    }
  else
    kcurrent = kmod_list_next(klist, kcurrent);

  if (kcurrent)
    {
      kmodule = kmod_module_get_module(kcurrent);
      return kmod_module_get_name(kmodule);
    }

  kmod_module_unref_list(klist);
  klist = NULL;
  return NULL;
}

#else

struct pcimap_entry {
  struct pcimap_entry *next;
  unsigned int vendor, device;
  unsigned int subvendor, subdevice;
  unsigned int class, class_mask;
  char module[1];
};

static struct pcimap_entry *pcimap_head;

static int
show_kernel_init(void)
{
  static int tried_pcimap;
  struct utsname uts;
  char *name, line[1024];
  FILE *f;

  if (tried_pcimap)
    return 1;
  tried_pcimap = 1;

  if (name = opt_pcimap)
    {
      f = fopen(name, "r");
      if (!f)
	die("Cannot open pcimap file %s: %m", name);
    }
  else
    {
      if (uname(&uts) < 0)
	die("uname() failed: %m");
      name = alloca(64 + strlen(uts.release));
      sprintf(name, "/lib/modules/%s/modules.pcimap", uts.release);
      f = fopen(name, "r");
      if (!f)
	return 1;
    }

  while (fgets(line, sizeof(line), f))
    {
      char *c = strchr(line, '\n');
      struct pcimap_entry *e;

      if (!c)
	die("Unterminated or too long line in %s", name);
      *c = 0;
      if (!line[0] || line[0] == '#')
	continue;

      c = line;
      while (*c && *c != ' ' && *c != '\t')
	c++;
      if (!*c)
	continue;	/* FIXME: Emit warnings! */
      *c++ = 0;

      e = xmalloc(sizeof(*e) + strlen(line));
      if (sscanf(c, "%i%i%i%i%i%i",
		 &e->vendor, &e->device,
		 &e->subvendor, &e->subdevice,
		 &e->class, &e->class_mask) != 6)
	continue;
      e->next = pcimap_head;
      pcimap_head = e;
      strcpy(e->module, line);
    }
  fclose(f);

  return 1;
}

static int
match_pcimap(struct device *d, struct pcimap_entry *e)
{
  struct pci_dev *dev = d->dev;
  unsigned int class = get_conf_long(d, PCI_REVISION_ID) >> 8;
  word subv, subd;

#define MATCH(x, y) ((y) > 0xffff || (x) == (y))
  get_subid(d, &subv, &subd);
  return
    MATCH(dev->vendor_id, e->vendor) &&
    MATCH(dev->device_id, e->device) &&
    MATCH(subv, e->subvendor) &&
    MATCH(subd, e->subdevice) &&
    (class & e->class_mask) == e->class;
#undef MATCH
}

static const char *next_module(struct device *d)
{
  static struct pcimap_entry *current;

  if (!current)
    current = pcimap_head;
  else
    current = current->next;

  while (current)
    {
      if (match_pcimap(d, current))
	return current->module;
      current = current->next;
    }

  return NULL;
}

void
show_kernel_cleanup(void)
{
}

#endif

#define DRIVER_BUF_SIZE 1024

static char *
find_driver(struct device *d, char *buf)
{
  struct pci_dev *dev = d->dev;
  char name[1024], *drv, *base;
  int n;

  if (dev->access->method != PCI_ACCESS_SYS_BUS_PCI)
    return NULL;

  base = pci_get_param(dev->access, "sysfs.path");
  if (!base || !base[0])
    return NULL;

  n = snprintf(name, sizeof(name), "%s/devices/%04x:%02x:%02x.%d/driver",
	       base, dev->domain, dev->bus, dev->dev, dev->func);
  if (n < 0 || n >= (int)sizeof(name))
    die("show_driver: sysfs device name too long, why?");

  n = readlink(name, buf, DRIVER_BUF_SIZE);
  if (n < 0)
    return NULL;
  if (n >= DRIVER_BUF_SIZE)
    return "<name-too-long>";
  buf[n] = 0;

  if (drv = strrchr(buf, '/'))
    return drv+1;
  else
    return buf;
}

static const char *
next_module_filtered(struct device *d)
{
  static char prev_module[256];
  const char *module;

  while (module = next_module(d))
    {
      if (strcmp(module, prev_module))
	{
	  strncpy(prev_module, module, sizeof(prev_module));
	  prev_module[sizeof(prev_module) - 1] = 0;
	  return module;
	}
    }
  prev_module[0] = 0;
  return NULL;
}

void
show_kernel(struct device *d)
{
  char buf[DRIVER_BUF_SIZE];
  const char *driver, *module;

  if (driver = find_driver(d, buf))
    printf("\tKernel driver in use: %s\n", driver);

  if (!show_kernel_init())
    return;

  int cnt = 0;
  while (module = next_module_filtered(d))
    printf("%s %s", (cnt++ ? "," : "\tKernel modules:"), module);
  if (cnt)
    putchar('\n');
}

void
show_kernel_machine(struct device *d)
{
  char buf[DRIVER_BUF_SIZE];
  const char *driver, *module;

  if (driver = find_driver(d, buf))
    printf("Driver:\t%s\n", driver);

  if (!show_kernel_init())
    return;

  while (module = next_module_filtered(d))
    printf("Module:\t%s\n", module);
}

#else

void
show_kernel(struct device *d UNUSED)
{
}

void
show_kernel_machine(struct device *d UNUSED)
{
}

void
show_kernel_cleanup(void)
{
}

#endif

