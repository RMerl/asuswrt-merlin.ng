/// Use mdio_alloc and mdio_register instead of miiphy_register
///
//# Stop using the oldest mii interface in drivers
//
// Confidence: High
// Copyright: (C) 2016 Joe Hershberger.  GPLv2.
// Comments:
// Options: --include-headers --recursive-includes --local-includes -I include

@ mii_reg @
expression devname;
identifier readfunc, writefunc;
@@

+ int retval;
- miiphy_register(devname, readfunc, writefunc);
+ struct mii_dev *mdiodev = mdio_alloc();
+ if (!mdiodev) return -ENOMEM;
+ strncpy(mdiodev->name, devname, MDIO_NAME_LEN);
+ mdiodev->read = readfunc;
+ mdiodev->write = writefunc;
+ 
+ retval = mdio_register(mdiodev);
+ if (retval < 0) return retval;

@ update_read_sig @
identifier mii_reg.readfunc;
identifier name0, addr0, reg0, output;
type addrT, outputT;
@@

- readfunc (
- 	const char *name0,
- 	addrT addr0,
- 	addrT reg0,
- 	outputT *output
- )
+ readfunc (
+ 	struct mii_dev *bus,
+ 	int addr0,
+ 	int devad,
+ 	int reg0
+ )
  {
  ...
  }

@ update_read_impl @
identifier mii_reg.readfunc;
identifier update_read_sig.output;
type update_read_sig.outputT;
constant c;
identifier retvar;
expression E;
@@

  readfunc (...)
  {
+ outputT output = 0;
  ...
(
- return 0;
+ return *output;
|
  return c;
|
- return retvar;
+ if (retvar < 0)
+ 	return retvar;
+ return *output;
|
- return E;
+ int retval = E;
+ if (retval < 0)
+ 	return retval;
+ return *output;
)
  }

@ update_read_impl2 @
identifier mii_reg.readfunc;
identifier update_read_sig.output;
@@

  readfunc (...)
  {
  <...
(
- *output
+ output
|
- output
+ &output
)
  ...>
  }

@ update_read_name @
identifier mii_reg.readfunc;
identifier update_read_sig.name0;
@@
  readfunc (...) {
  <...
- name0
+ bus->name
  ...>
  }

@ update_write_sig @
identifier mii_reg.writefunc;
identifier name0, addr0, reg0, value0;
type addrT, valueT;
typedef u16;
@@

- writefunc (
- 	const char *name0,
- 	addrT addr0,
- 	addrT reg0,
- 	valueT value0
- )
+ writefunc (
+ 	struct mii_dev *bus,
+ 	int addr0,
+ 	int devad,
+ 	int reg0,
+ 	u16 value0
+ )
  {
  ...
  }

@ update_write_name @
identifier mii_reg.writefunc;
identifier update_write_sig.name0;
@@
  writefunc (...) {
  <...
- name0
+ bus->name
  ...>
  }
