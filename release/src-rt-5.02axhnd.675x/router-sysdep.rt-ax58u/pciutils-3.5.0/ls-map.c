/*
 *	The PCI Utilities -- Bus Mapping Mode
 *
 *	Copyright (c) 1997--2008 Martin Mares <mj@ucw.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "lspci.h"

struct bus_bridge {
  struct bus_bridge *next;
  byte this, dev, func, first, last, bug;
};

struct bus_info {
  byte exists;
  byte guestbook;
  struct bus_bridge *bridges, *via;
};

static struct bus_info *bus_info;

static void
map_bridge(struct bus_info *bi, struct device *d, int np, int ns, int nl)
{
  struct bus_bridge *b = xmalloc(sizeof(struct bus_bridge));
  struct pci_dev *p = d->dev;

  b->next = bi->bridges;
  bi->bridges = b;
  b->this = get_conf_byte(d, np);
  b->dev = p->dev;
  b->func = p->func;
  b->first = get_conf_byte(d, ns);
  b->last = get_conf_byte(d, nl);
  printf("## %02x:%02x.%d is a bridge from %02x to %02x-%02x\n",
	 p->bus, p->dev, p->func, b->this, b->first, b->last);
  if (b->this != p->bus)
    printf("!!! Bridge points to invalid primary bus.\n");
  if (b->first > b->last)
    {
      printf("!!! Bridge points to invalid bus range.\n");
      b->last = b->first;
    }
}

static void
do_map_bus(int bus)
{
  int dev, func;
  int verbose = pacc->debugging;
  struct bus_info *bi = bus_info + bus;
  struct device *d;

  if (verbose)
    printf("Mapping bus %02x\n", bus);
  for (dev = 0; dev < 32; dev++)
    if (filter.slot < 0 || filter.slot == dev)
      {
	int func_limit = 1;
	for (func = 0; func < func_limit; func++)
	  if (filter.func < 0 || filter.func == func)
	    {
	      /* XXX: Bus mapping supports only domain 0 */
	      struct pci_dev *p = pci_get_dev(pacc, 0, bus, dev, func);
	      u16 vendor = pci_read_word(p, PCI_VENDOR_ID);
	      if (vendor && vendor != 0xffff)
		{
		  if (!func && (pci_read_byte(p, PCI_HEADER_TYPE) & 0x80))
		    func_limit = 8;
		  if (verbose)
		    printf("Discovered device %02x:%02x.%d\n", bus, dev, func);
		  bi->exists = 1;
		  if (d = scan_device(p))
		    {
		      show_device(d);
		      switch (get_conf_byte(d, PCI_HEADER_TYPE) & 0x7f)
			{
			case PCI_HEADER_TYPE_BRIDGE:
			  map_bridge(bi, d, PCI_PRIMARY_BUS, PCI_SECONDARY_BUS, PCI_SUBORDINATE_BUS);
			  break;
			case PCI_HEADER_TYPE_CARDBUS:
			  map_bridge(bi, d, PCI_CB_PRIMARY_BUS, PCI_CB_CARD_BUS, PCI_CB_SUBORDINATE_BUS);
			  break;
			}
		      free(d);
		    }
		  else if (verbose)
		    printf("But it was filtered out.\n");
		}
	      pci_free_dev(p);
	    }
      }
}

static void
do_map_bridges(int bus, int min, int max)
{
  struct bus_info *bi = bus_info + bus;
  struct bus_bridge *b;

  bi->guestbook = 1;
  for (b=bi->bridges; b; b=b->next)
    {
      if (bus_info[b->first].guestbook)
	b->bug = 1;
      else if (b->first < min || b->last > max)
	b->bug = 2;
      else
	{
	  bus_info[b->first].via = b;
	  do_map_bridges(b->first, b->first, b->last);
	}
    }
}

static void
map_bridges(void)
{
  int i;

  printf("\nSummary of buses:\n\n");
  for (i=0; i<256; i++)
    if (bus_info[i].exists && !bus_info[i].guestbook)
      do_map_bridges(i, 0, 255);
  for (i=0; i<256; i++)
    {
      struct bus_info *bi = bus_info + i;
      struct bus_bridge *b = bi->via;

      if (bi->exists)
	{
	  printf("%02x: ", i);
	  if (b)
	    printf("Entered via %02x:%02x.%d\n", b->this, b->dev, b->func);
	  else if (!i)
	    printf("Primary host bus\n");
	  else
	    printf("Secondary host bus (?)\n");
	}
      for (b=bi->bridges; b; b=b->next)
	{
	  printf("\t%02x.%d Bridge to %02x-%02x", b->dev, b->func, b->first, b->last);
	  switch (b->bug)
	    {
	    case 1:
	      printf(" <overlap bug>");
	      break;
	    case 2:
	      printf(" <crossing bug>");
	      break;
	    }
	  putchar('\n');
	}
    }
}

void
map_the_bus(void)
{
  if (pacc->method == PCI_ACCESS_PROC_BUS_PCI ||
      pacc->method == PCI_ACCESS_SYS_BUS_PCI ||
      pacc->method == PCI_ACCESS_DUMP)
    printf("WARNING: Bus mapping can be reliable only with direct hardware access enabled.\n\n");
  bus_info = xmalloc(sizeof(struct bus_info) * 256);
  memset(bus_info, 0, sizeof(struct bus_info) * 256);
  if (filter.bus >= 0)
    do_map_bus(filter.bus);
  else
    {
      int bus;
      for (bus=0; bus<256; bus++)
	do_map_bus(bus);
    }
  map_bridges();
}
