/*
 *	The PCI Utilities -- Manipulate PCI Configuration Registers
 *
 *	Copyright (c) 1998--2008 Martin Mares <mj@ucw.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>

#define PCIUTILS_SETPCI
#include "pciutils.h"

static int force;			/* Don't complain if no devices match */
static int verbose;			/* Verbosity level */
static int demo_mode;			/* Only show */

const char program_name[] = "setpci";

static struct pci_access *pacc;

struct value {
  unsigned int value;
  unsigned int mask;
};

struct op {
  struct op *next;
  struct pci_dev **dev_vector;
  u16 cap_type;				/* PCI_CAP_xxx or 0 */
  u16 cap_id;
  unsigned int addr;
  unsigned int width;			/* Byte width of the access */
  unsigned int num_values;		/* Number of values to write; 0=read */
  struct value values[0];
};

static struct op *first_op, **last_op = &first_op;
static unsigned int max_values[] = { 0, 0xff, 0xffff, 0, 0xffffffff };

static struct pci_dev **
select_devices(struct pci_filter *filt)
{
  struct pci_dev *z, **a, **b;
  int cnt = 1;

  for (z=pacc->devices; z; z=z->next)
    if (pci_filter_match(filt, z))
      cnt++;
  a = b = xmalloc(sizeof(struct device *) * cnt);
  for (z=pacc->devices; z; z=z->next)
    if (pci_filter_match(filt, z))
      *a++ = z;
  *a = NULL;
  return b;
}

static void PCI_PRINTF(1,2)
trace(const char *fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  if (verbose)
    vprintf(fmt, args);
  va_end(args);
}

static void
exec_op(struct op *op, struct pci_dev *dev)
{
  const char * const formats[] = { NULL, " %02x", " %04x", NULL, " %08x" };
  const char * const mask_formats[] = { NULL, " %02x->(%02x:%02x)->%02x", " %04x->(%04x:%04x)->%04x", NULL, " %08x->(%08x:%08x)->%08x" };
  unsigned int i, x, y;
  int addr = 0;
  int width = op->width;
  char slot[16];

  sprintf(slot, "%04x:%02x:%02x.%x", dev->domain, dev->bus, dev->dev, dev->func);
  trace("%s ", slot);
  if (op->cap_type)
    {
      struct pci_cap *cap;
      cap = pci_find_cap(dev, op->cap_id, op->cap_type);
      if (cap)
	addr = cap->addr;
      else
	die("%s: %s %04x not found", slot, ((op->cap_type == PCI_CAP_NORMAL) ? "Capability" : "Extended capability"), op->cap_id);
      trace(((op->cap_type == PCI_CAP_NORMAL) ? "(cap %02x @%02x) " : "(ecap %04x @%03x) "), op->cap_id, addr);
    }
  addr += op->addr;
  trace("@%02x", addr);

  /* We have already checked it when parsing, but addressing relative to capabilities can change the address. */
  if (addr & (width-1))
    die("%s: Unaligned access of width %d to register %04x", slot, width, addr);
  if (addr + width > 0x1000)
    die("%s: Access of width %d to register %04x out of range", slot, width, addr);

  if (op->num_values)
    {
      for (i=0; i<op->num_values; i++)
	{
	  if ((op->values[i].mask & max_values[width]) == max_values[width])
	    {
	      x = op->values[i].value;
	      trace(formats[width], op->values[i].value);
	    }
	  else
	    {
	      switch (width)
		{
		case 1:
		  y = pci_read_byte(dev, addr);
		  break;
		case 2:
		  y = pci_read_word(dev, addr);
		  break;
		default:
		  y = pci_read_long(dev, addr);
		  break;
		}
	      x = (y & ~op->values[i].mask) | op->values[i].value;
	      trace(mask_formats[width], y, op->values[i].value, op->values[i].mask, x);
	    }
	  if (!demo_mode)
	    {
	      switch (width)
		{
		case 1:
		  pci_write_byte(dev, addr, x);
		  break;
		case 2:
		  pci_write_word(dev, addr, x);
		  break;
		default:
		  pci_write_long(dev, addr, x);
		  break;
		}
	    }
	  addr += width;
	}
      trace("\n");
    }
  else
    {
      trace(" = ");
      switch (width)
	{
	case 1:
	  x = pci_read_byte(dev, addr);
	  break;
	case 2:
	  x = pci_read_word(dev, addr);
	  break;
	default:
	  x = pci_read_long(dev, addr);
	  break;
	}
      printf(formats[width]+1, x);
      putchar('\n');
    }
}

static void
execute(struct op *op)
{
  struct pci_dev **vec = NULL;
  struct pci_dev **pdev, *dev;
  struct op *oops;

  while (op)
    {
      pdev = vec = op->dev_vector;
      while (dev = *pdev++)
	for (oops=op; oops && oops->dev_vector == vec; oops=oops->next)
	  exec_op(oops, dev);
      while (op && op->dev_vector == vec)
	op = op->next;
    }
}

static void
scan_ops(struct op *op)
{
  if (demo_mode)
    return;
  while (op)
    {
      if (op->num_values)
	pacc->writeable = 1;
      op = op->next;
    }
}

struct reg_name {
  unsigned int cap;
  unsigned int offset;
  unsigned int width;
  const char *name;
};

static const struct reg_name pci_reg_names[] = {
  {       0, 0x00, 2, "VENDOR_ID" },
  {       0, 0x02, 2, "DEVICE_ID" },
  {       0, 0x04, 2, "COMMAND" },
  {       0, 0x06, 2, "STATUS" },
  {       0, 0x08, 1, "REVISION" },
  {       0, 0x09, 1, "CLASS_PROG" },
  {       0, 0x0a, 2, "CLASS_DEVICE" },
  {       0, 0x0c, 1, "CACHE_LINE_SIZE" },
  {       0, 0x0d, 1, "LATENCY_TIMER" },
  {       0, 0x0e, 1, "HEADER_TYPE" },
  {       0, 0x0f, 1, "BIST" },
  {       0, 0x10, 4, "BASE_ADDRESS_0" },
  {       0, 0x14, 4, "BASE_ADDRESS_1" },
  {       0, 0x18, 4, "BASE_ADDRESS_2" },
  {       0, 0x1c, 4, "BASE_ADDRESS_3" },
  {       0, 0x20, 4, "BASE_ADDRESS_4" },
  {       0, 0x24, 4, "BASE_ADDRESS_5" },
  {       0, 0x28, 4, "CARDBUS_CIS" },
  {       0, 0x2c, 4, "SUBSYSTEM_VENDOR_ID" },
  {       0, 0x2e, 2, "SUBSYSTEM_ID" },
  {       0, 0x30, 4, "ROM_ADDRESS" },
  {       0, 0x3c, 1, "INTERRUPT_LINE" },
  {       0, 0x3d, 1, "INTERRUPT_PIN" },
  {       0, 0x3e, 1, "MIN_GNT" },
  {       0, 0x3f, 1, "MAX_LAT" },
  {       0, 0x18, 1, "PRIMARY_BUS" },
  {       0, 0x19, 1, "SECONDARY_BUS" },
  {       0, 0x1a, 1, "SUBORDINATE_BUS" },
  {       0, 0x1b, 1, "SEC_LATENCY_TIMER" },
  {       0, 0x1c, 1, "IO_BASE" },
  {       0, 0x1d, 1, "IO_LIMIT" },
  {       0, 0x1e, 2, "SEC_STATUS" },
  {       0, 0x20, 2, "MEMORY_BASE" },
  {       0, 0x22, 2, "MEMORY_LIMIT" },
  {       0, 0x24, 2, "PREF_MEMORY_BASE" },
  {       0, 0x26, 2, "PREF_MEMORY_LIMIT" },
  {       0, 0x28, 4, "PREF_BASE_UPPER32" },
  {       0, 0x2c, 4, "PREF_LIMIT_UPPER32" },
  {       0, 0x30, 2, "IO_BASE_UPPER16" },
  {       0, 0x32, 2, "IO_LIMIT_UPPER16" },
  {       0, 0x38, 4, "BRIDGE_ROM_ADDRESS" },
  {       0, 0x3e, 2, "BRIDGE_CONTROL" },
  {       0, 0x10, 4, "CB_CARDBUS_BASE" },
  {       0, 0x14, 2, "CB_CAPABILITIES" },
  {       0, 0x16, 2, "CB_SEC_STATUS" },
  {       0, 0x18, 1, "CB_BUS_NUMBER" },
  {       0, 0x19, 1, "CB_CARDBUS_NUMBER" },
  {       0, 0x1a, 1, "CB_SUBORDINATE_BUS" },
  {       0, 0x1b, 1, "CB_CARDBUS_LATENCY" },
  {       0, 0x1c, 4, "CB_MEMORY_BASE_0" },
  {       0, 0x20, 4, "CB_MEMORY_LIMIT_0" },
  {       0, 0x24, 4, "CB_MEMORY_BASE_1" },
  {       0, 0x28, 4, "CB_MEMORY_LIMIT_1" },
  {       0, 0x2c, 2, "CB_IO_BASE_0" },
  {       0, 0x2e, 2, "CB_IO_BASE_0_HI" },
  {       0, 0x30, 2, "CB_IO_LIMIT_0" },
  {       0, 0x32, 2, "CB_IO_LIMIT_0_HI" },
  {       0, 0x34, 2, "CB_IO_BASE_1" },
  {       0, 0x36, 2, "CB_IO_BASE_1_HI" },
  {       0, 0x38, 2, "CB_IO_LIMIT_1" },
  {       0, 0x3a, 2, "CB_IO_LIMIT_1_HI" },
  {       0, 0x40, 2, "CB_SUBSYSTEM_VENDOR_ID" },
  {       0, 0x42, 2, "CB_SUBSYSTEM_ID" },
  {       0, 0x44, 4, "CB_LEGACY_MODE_BASE" },
  { 0x10001,    0, 0, "CAP_PM" },
  { 0x10002,    0, 0, "CAP_AGP" },
  { 0x10003,    0, 0, "CAP_VPD" },
  { 0x10004,    0, 0, "CAP_SLOTID" },
  { 0x10005,    0, 0, "CAP_MSI" },
  { 0x10006,    0, 0, "CAP_CHSWP" },
  { 0x10007,    0, 0, "CAP_PCIX" },
  { 0x10008,    0, 0, "CAP_HT" },
  { 0x10009,    0, 0, "CAP_VNDR" },
  { 0x1000a,    0, 0, "CAP_DBG" },
  { 0x1000b,    0, 0, "CAP_CCRC" },
  { 0x1000c,    0, 0, "CAP_HOTPLUG" },
  { 0x1000d,    0, 0, "CAP_SSVID" },
  { 0x1000e,    0, 0, "CAP_AGP3" },
  { 0x1000f,    0, 0, "CAP_SECURE" },
  { 0x10010,    0, 0, "CAP_EXP" },
  { 0x10011,    0, 0, "CAP_MSIX" },
  { 0x10012,    0, 0, "CAP_SATA" },
  { 0x10013,    0, 0, "CAP_AF" },
  { 0x20001,	0, 0, "ECAP_AER" },
  { 0x20002,	0, 0, "ECAP_VC" },
  { 0x20003,	0, 0, "ECAP_DSN" },
  { 0x20004,	0, 0, "ECAP_PB" },
  { 0x20005,	0, 0, "ECAP_RCLINK" },
  { 0x20006,	0, 0, "ECAP_RCILINK" },
  { 0x20007,	0, 0, "ECAP_RCECOLL" },
  { 0x20008,	0, 0, "ECAP_MFVC" },
  { 0x2000a,	0, 0, "ECAP_RBCB" },
  { 0x2000b,	0, 0, "ECAP_VNDR" },
  { 0x2000d,	0, 0, "ECAP_ACS" },
  { 0x2000e,	0, 0, "ECAP_ARI" },
  { 0x2000f,	0, 0, "ECAP_ATS" },
  { 0x20010,	0, 0, "ECAP_SRIOV" },
  { 0x2001d,	0, 0, "ECAP_DPC" },
  {       0,    0, 0, NULL }
};

static void
dump_registers(void)
{
  const struct reg_name *r;

  printf("cap pos w name\n");
  for (r = pci_reg_names; r->name; r++)
    {
      if (r->cap >= 0x20000)
	printf("%04x", r->cap - 0x20000);
      else if (r->cap)
	printf("  %02x", r->cap - 0x10000);
      else
	printf("    ");
      printf(" %02x %c %s\n", r->offset, "-BW?L"[r->width], r->name);
    }
}

static void NONRET
usage(void)
{
  fprintf(stderr,
"Usage: setpci [<options>] (<device>+ <reg>[=<values>]*)*\n"
"\n"
"General options:\n"
"-f\t\tDon't complain if there's nothing to do\n"
"-v\t\tBe verbose\n"
"-D\t\tList changes, don't commit them\n"
"--dumpregs\tDump all known register names and exit\n"
"\n"
"PCI access options:\n"
GENERIC_HELP
"\n"
"Setting commands:\n"
"<device>:\t-s [[[<domain>]:][<bus>]:][<slot>][.[<func>]]\n"
"\t\t-d [<vendor>]:[<device>]\n"
"<reg>:\t\t<base>[+<offset>][.(B|W|L)]\n"
"<base>:\t\t<address>\n"
"\t\t<named-register>\n"
"\t\t[E]CAP_<capability-name>\n"
"\t\t[E]CAP<capability-number>\n"
"<values>:\t<value>[,<value>...]\n"
"<value>:\t<hex>\n"
"\t\t<hex>:<mask>\n");
  exit(0);
}

static void NONRET PCI_PRINTF(1,2)
parse_err(const char *msg, ...)
{
  va_list args;
  va_start(args, msg);
  fprintf(stderr, "setpci: ");
  vfprintf(stderr, msg, args);
  fprintf(stderr, ".\nTry `setpci --help' for more information.\n");
  exit(1);
}

static int
parse_options(int argc, char **argv)
{
  const char opts[] = GENERIC_OPTIONS;
  int i=1;

  if (argc == 2)
    {
      if (!strcmp(argv[1], "--help"))
	usage();
      if (!strcmp(argv[1], "--version"))
	{
	  puts("setpci version " PCIUTILS_VERSION);
	  exit(0);
	}
      if (!strcmp(argv[1], "--dumpregs"))
	{
	  dump_registers();
	  exit(0);
	}
    }

  while (i < argc && argv[i][0] == '-')
    {
      char *c = argv[i++] + 1;
      char *d = c;
      char *e;
      while (*c)
	switch (*c)
	  {
	  case 0:
	    break;
	  case 'v':
	    verbose++;
	    c++;
	    break;
	  case 'f':
	    force++;
	    c++;
	    break;
	  case 'D':
	    demo_mode++;
	    c++;
	    break;
	  default:
	    if (e = strchr(opts, *c))
	      {
		char *arg;
		c++;
		if (e[1] == ':')
		  {
		    if (*c)
		      arg = c;
		    else if (i < argc)
		      arg = argv[i++];
		    else
		      parse_err("Option -%c requires an argument", *e);
		    c = "";
		  }
		else
		  arg = NULL;
		if (!parse_generic_option(*e, pacc, arg))
		  parse_err("Unable to parse option -%c", *e);
	      }
	    else
	      {
		if (c != d)
		  parse_err("Invalid or misplaced option -%c", *c);
		return i-1;
	      }
	  }
    }

  return i;
}

static int parse_filter(int argc, char **argv, int i, struct pci_filter *filter)
{
  char *c = argv[i++];
  char *d;

  if (!c[1] || !strchr("sd", c[1]))
    parse_err("Invalid option -%c", c[1]);
  if (c[2])
    d = (c[2] == '=') ? c+3 : c+2;
  else if (i < argc)
    d = argv[i++];
  else
    parse_err("Option -%c requires an argument", c[1]);
  switch (c[1])
    {
    case 's':
      if (d = pci_filter_parse_slot(filter, d))
	parse_err("Unable to parse filter -s %s", d);
      break;
    case 'd':
      if (d = pci_filter_parse_id(filter, d))
	parse_err("Unable to parse filter -d %s", d);
      break;
    default:
      parse_err("Unknown filter option -%c", c[1]);
    }

  return i;
}

static const struct reg_name *parse_reg_name(char *name)
{
  const struct reg_name *r;

  for (r = pci_reg_names; r->name; r++)
    if (!strcasecmp(r->name, name))
      return r;
  return NULL;
}

static int parse_x32(char *c, char **stopp, unsigned int *resp)
{
  char *stop;
  unsigned long int l;

  if (!*c)
    return -1;
  errno = 0;
  l = strtoul(c, &stop, 16);
  if (errno)
    return -1;
  if ((l & ~0U) != l)
    return -1;
  *resp = l;
  if (*stop)
    {
      if (stopp)
	*stopp = stop;
      return 0;
    }
  else
    {
      if (stopp)
	*stopp = NULL;
      return 1;
    }
}

static void parse_register(struct op *op, char *base)
{
  const struct reg_name *r;
  unsigned int cap;

  op->cap_type = op->cap_id = 0;
  if (parse_x32(base, NULL, &op->addr) > 0)
    return;
  else if (r = parse_reg_name(base))
    {
      switch (r->cap & 0xff0000)
	{
	case 0x10000:
	  op->cap_type = PCI_CAP_NORMAL;
	  break;
	case 0x20000:
	  op->cap_type = PCI_CAP_EXTENDED;
	  break;
	}
      op->cap_id = r->cap & 0xffff;
      op->addr = r->offset;
      if (r->width && !op->width)
	op->width = r->width;
      return;
    }
  else if (!strncasecmp(base, "CAP", 3))
    {
      if (parse_x32(base+3, NULL, &cap) > 0 && cap < 0x100)
	{
	  op->cap_type = PCI_CAP_NORMAL;
	  op->cap_id = cap;
	  op->addr = 0;
	  return;
	}
    }
  else if (!strncasecmp(base, "ECAP", 4))
    {
      if (parse_x32(base+4, NULL, &cap) > 0 && cap < 0x1000)
	{
	  op->cap_type = PCI_CAP_EXTENDED;
	  op->cap_id = cap;
	  op->addr = 0;
	  return;
	}
    }
  parse_err("Unknown register \"%s\"", base);
}

static void parse_op(char *c, struct pci_dev **selected_devices)
{
  char *base, *offset, *width, *value;
  char *e, *f;
  int n, j;
  struct op *op;

  /* Split the argument */
  base = xstrdup(c);
  if (value = strchr(base, '='))
    *value++ = 0;
  if (width = strchr(base, '.'))
    *width++ = 0;
  if (offset = strchr(base, '+'))
    *offset++ = 0;

  /* Look for setting of values and count how many */
  n = 0;
  if (value)
    {
      if (!*value)
	parse_err("Missing value");
      n++;
      for (e=value; *e; e++)
	if (*e == ',')
	  n++;
    }

  /* Allocate the operation */
  op = xmalloc(sizeof(struct op) + n*sizeof(struct value));
  op->dev_vector = selected_devices;
  op->num_values = n;

  /* What is the width suffix? */
  if (width)
    {
      if (width[1])
	parse_err("Invalid width \"%s\"", width);
      switch (*width & 0xdf)
	{
	case 'B':
	  op->width = 1; break;
	case 'W':
	  op->width = 2; break;
	case 'L':
	  op->width = 4; break;
	default:
	  parse_err("Invalid width \"%c\"", *width);
	}
    }
  else
    op->width = 0;

  /* Find the register */
  parse_register(op, base);
  if (!op->width)
    parse_err("Missing width");

  /* Add offset */
  if (offset)
    {
      unsigned int off;
      if (parse_x32(offset, NULL, &off) <= 0 || off >= 0x1000)
	parse_err("Invalid offset \"%s\"", offset);
      op->addr += off;
    }

  /* Check range */
  if (op->addr >= 0x1000 || op->addr + op->width*(n ? n : 1) > 0x1000)
    parse_err("Register number %02x out of range", op->addr);
  if (op->addr & (op->width - 1))
    parse_err("Unaligned register address %02x", op->addr);

  /* Parse the values */
  for (j=0; j<n; j++)
    {
      unsigned int ll, lim;
      e = strchr(value, ',');
      if (e)
	*e++ = 0;
      if (parse_x32(value, &f, &ll) < 0 || f && *f != ':')
	parse_err("Invalid value \"%s\"", value);
      lim = max_values[op->width];
      if (ll > lim && ll < ~0U - lim)
	parse_err("Value \"%s\" is out of range", value);
      op->values[j].value = ll;
      if (f && *f == ':')
	{
	  if (parse_x32(f+1, NULL, &ll) <= 0)
	    parse_err("Invalid mask \"%s\"", f+1);
	  if (ll > lim && ll < ~0U - lim)
	    parse_err("Mask \"%s\" is out of range", f+1);
	  op->values[j].mask = ll;
	  op->values[j].value &= ll;
	}
      else
	op->values[j].mask = ~0U;
      value = e;
    }

  *last_op = op;
  last_op = &op->next;
  op->next = NULL;
}

static void parse_ops(int argc, char **argv, int i)
{
  enum { STATE_INIT, STATE_GOT_FILTER, STATE_GOT_OP } state = STATE_INIT;
  struct pci_filter filter;
  struct pci_dev **selected_devices = NULL;

  while (i < argc)
    {
      char *c = argv[i++];

      if (*c == '-')
	{
	  if (state != STATE_GOT_FILTER)
	    pci_filter_init(pacc, &filter);
	  i = parse_filter(argc, argv, i-1, &filter);
	  state = STATE_GOT_FILTER;
	}
      else
	{
	  if (state == STATE_INIT)
	    parse_err("Filter specification expected");
	  if (state == STATE_GOT_FILTER)
	    selected_devices = select_devices(&filter);
	  if (!selected_devices[0] && !force)
	    fprintf(stderr, "setpci: Warning: No devices selected for \"%s\".\n", c);
	  parse_op(c, selected_devices);
	  state = STATE_GOT_OP;
	}
    }
  if (state == STATE_INIT)
    parse_err("No operation specified");
}

int
main(int argc, char **argv)
{
  int i;

  pacc = pci_alloc();
  pacc->error = die;
  i = parse_options(argc, argv);

  pci_init(pacc);
  pci_scan_bus(pacc);

  parse_ops(argc, argv, i);
  scan_ops(first_op);
  execute(first_op);

  return 0;
}
