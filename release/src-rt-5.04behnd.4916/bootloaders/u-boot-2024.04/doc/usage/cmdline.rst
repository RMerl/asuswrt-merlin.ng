.. SPDX-License-Identifier: GPL-2.0+

Command-line Parsing
====================

The command line is available in U-Boot proper, enabled by CONFIG_CMDLINE which
is on by default. It is not enabled in SPL.

There are two different command-line parsers available with U-Boot:
the old "simple" one, and the much more powerful "hush" shell:

Simple command-line parser
--------------------------

This takes very little code space and offers only basic features:

- supports environment variables (through :doc:`cmd/env`)
- several commands on one line, separated by ';'
- variable substitution using "... ${name} ..." syntax
- special characters ('$', ';') can be escaped by prefixing with '\',
  for example::

    setenv bootcmd bootm \${address}

- You can also escape text by enclosing in single apostrophes, for example::

    setenv addip 'setenv bootargs $bootargs ip=$ipaddr:$serverip:$gatewayip:$netmask:$hostname::off'

Hush shell
----------

This is similar to Bourne shell, with control structures like:

- `if`... `then` ... `else`... `fi`
- `for`... `do` ... `done`
- `while` ... `do` ... `done`
- `until` ... `do` ... `done`

Hush supports environment ("global") variables (through setenv / saveenv
commands) and local shell variables (through standard shell syntax
`name=value`); only environment variables can be used with the "run" command

The Hush shell is enabled with `CONFIG_HUSH_PARSER`.

General rules
-------------

#. If a command line (or an environment variable executed by a "run"
   command) contains several commands separated by semicolon, and
   one of these commands fails, then the remaining commands will be
   executed anyway.

#. If you execute several variables with one call to run (i. e.
   calling run with a list of variables as arguments), any failing
   command will cause "run" to terminate, i. e. the remaining
   variables are not executed.

Representing numbers
--------------------

Most U-Boot commands use hexadecimal (hex) as the default base, for convenient
use of addresses, for example::

  => md 1000 6
  00001000: 2c786f62 00697073 03000000 0c000000  box,spi.........
  00001010: 67020000 00000000                    ...g....

There is no need to add a `0x` prefix to the arguments and the output is shown
in hex also, without any prefixes. This helps to avoid clutter.

Some commands use decimal where it is more natural::

  => i2c dev 0
  Setting bus to 0
  => i2c speed
  Current bus speed=400000
  => i2c speed 100000
  Setting bus speed to 100000 Hz

In some cases the default is decimal but it is possible to use octal if that is
useful::

  pmic dev pmic@41
  dev: 1 @ pmic@41
  => pmic write 2 0177
  => pmic read 2
  0x02: 0x00007f

It is possible to use a `0x` prefix to use a hex value if that is more
convenient::

  => i2c speed 0x30000
  Setting bus speed to 196608 Hz
