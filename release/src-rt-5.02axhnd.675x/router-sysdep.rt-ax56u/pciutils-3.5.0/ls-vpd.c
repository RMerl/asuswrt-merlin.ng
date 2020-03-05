/*
 *	The PCI Utilities -- Show Vital Product Data
 *
 *	Copyright (c) 2008 Solarflare Communications
 *
 *	Written by Ben Hutchings <bhutchings@solarflare.com>
 *	Improved by Martin Mares <mj@ucw.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#include <stdio.h>

#include "lspci.h"

/*
 *  The list of all known VPD items and their formats.
 *  Technically, this belongs to the pci.ids file, but the VPD does not seem
 *  to be developed any longer, so we have chosen the easier way.
 */

enum vpd_format {
  F_BINARY,
  F_TEXT,
  F_RESVD,
  F_RDWR,
};

static const struct vpd_item {
  byte id1, id2;
  byte format;
  const char *name;
} vpd_items[] = {
  { 'C','P', F_BINARY,	"Extended capability" },
  { 'E','C', F_TEXT,	"Engineering changes" },
  { 'M','N', F_BINARY,	"Manufacture ID" },
  { 'P','N', F_TEXT,	"Part number" },
  { 'R','V', F_RESVD,	"Reserved" },
  { 'R','W', F_RDWR,	"Read-write area" },
  { 'S','N', F_TEXT,	"Serial number" },
  { 'Y','A', F_TEXT,	"Asset tag" },
  { 'V', 0 , F_TEXT,	"Vendor specific" },
  { 'Y', 0 , F_TEXT,	"System specific" },
  {  0,  0 , F_BINARY,	"Unknown" }
};

static void
print_vpd_string(const byte *buf, word len)
{
  while (len--)
    {
      byte ch = *buf++;
      if (ch == '\\')
        printf("\\\\");
      else if (!ch && !len)
        ;  /* Cards with null-terminated strings have been observed */
      else if (ch < 32 || ch == 127)
        printf("\\x%02x", ch);
      else
        putchar(ch);
    }
}

static void
print_vpd_binary(const byte *buf, word len)
{
  int i;
  for (i = 0; i < len; i++)
    {
      if (i)
        putchar(' ');
      printf("%02x", buf[i]);
    }
}

static int
read_vpd(struct device *d, int pos, byte *buf, int len, byte *csum)
{
  if (!pci_read_vpd(d->dev, pos, buf, len))
    return 0;
  while (len--)
    *csum += *buf++;
  return 1;
}

void
cap_vpd(struct device *d)
{
  word res_addr = 0, res_len, part_pos, part_len;
  byte buf[256];
  byte tag;
  byte csum = 0;

  printf("Vital Product Data\n");
  if (verbose < 2)
    return;

  while (res_addr <= PCI_VPD_ADDR_MASK)
    {
      if (!read_vpd(d, res_addr, &tag, 1, &csum))
	break;
      if (tag & 0x80)
	{
	  if (res_addr > PCI_VPD_ADDR_MASK + 1 - 3)
	    break;
	  if (!read_vpd(d, res_addr + 1, buf, 2, &csum))
	    break;
	  res_len = buf[0] + (buf[1] << 8);
	  res_addr += 3;
	}
      else
	{
	  res_len = tag & 7;
	  tag >>= 3;
	  res_addr += 1;
	}
      if (res_len > PCI_VPD_ADDR_MASK + 1 - res_addr)
	break;

      part_pos = 0;

      switch (tag)
	{
	case 0x0f:
	  printf("\t\tEnd\n");
	  return;

	case 0x82:
	  printf("\t\tProduct Name: ");
	  while (part_pos < res_len)
	    {
	      part_len = res_len - part_pos;
	      if (part_len > sizeof(buf))
		part_len = sizeof(buf);
	      if (!read_vpd(d, res_addr + part_pos, buf, part_len, &csum))
		break;
	      print_vpd_string(buf, part_len);
	      part_pos += part_len;
	    }
	  printf("\n");
	  break;

	case 0x90:
	case 0x91:
	  printf("\t\t%s fields:\n",
		 (tag == 0x90) ? "Read-only" : "Read/write");

	  while (part_pos + 3 <= res_len)
	    {
	      word read_len;
	      const struct vpd_item *item;
	      byte id1, id2;

	      if (!read_vpd(d, res_addr + part_pos, buf, 3, &csum))
		break;
	      part_pos += 3;
	      id1 = buf[0];
	      id2 = buf[1];
	      part_len = buf[2];
	      if (part_len > res_len - part_pos)
		break;

	      /* Is this item known? */
	      for (item=vpd_items; item->id1 && item->id1 != id1 ||
				   item->id2 && item->id2 != id2; item++)
		;

	      /* Only read the first byte of the RV field because the
	       * remaining bytes are not included in the checksum. */
	      read_len = (item->format == F_RESVD) ? 1 : part_len;
	      if (!read_vpd(d, res_addr + part_pos, buf, read_len, &csum))
		break;

	      printf("\t\t\t[%c%c] %s: ", id1, id2, item->name);

	      switch (item->format)
	        {
		case F_TEXT:
		  print_vpd_string(buf, part_len);
		  printf("\n");
		  break;
		case F_BINARY:
		  print_vpd_binary(buf, part_len);
		  printf("\n");
		  break;
		case F_RESVD:
		  printf("checksum %s, %d byte(s) reserved\n", csum ? "bad" : "good", part_len - 1);
		  break;
		case F_RDWR:
		  printf("%d byte(s) free\n", part_len);
		  break;
		}

	      part_pos += part_len;
	    }
	  break;

	default:
	  printf("\t\tUnknown %s resource type %02x, will not decode more.\n",
		 (tag & 0x80) ? "large" : "small", tag & ~0x80);
	  return;
	}

      res_addr += res_len;
    }

  if (res_addr == 0)
    printf("\t\tNot readable\n");
  else
    printf("\t\tNo end tag found\n");
}
