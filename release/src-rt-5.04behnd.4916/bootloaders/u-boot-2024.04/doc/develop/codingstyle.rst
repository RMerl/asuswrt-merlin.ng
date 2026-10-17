.. SPDX-License-Identifier: GPL-2.0+:

U-Boot Coding Style
===================

The following Coding Style requirements shall be mandatory for all code contributed to
the U-Boot project.

Exceptions are only allowed if code from other projects is integrated with no
or only minimal changes.

The following rules apply:

* All contributions to U-Boot should conform to the `Linux kernel
  coding style <https://www.kernel.org/doc/html/latest/process/coding-style.html>`_
  and the `Lindent script <https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/tree/scripts/Lindent>`_.
  * The exception for net files to the `multi-line comment
  <https://www.kernel.org/doc/html/latest/process/coding-style.html#commenting>`_
  applies only to Linux, not to U-Boot. Only large hunks which are copied
  unchanged from Linux may retain that comment format.

* Python code shall conform to `PEP8 (Style Guide for Python Code)
  <https://peps.python.org/pep-0008/>`_. Use `pylint
  <https://github.com/pylint-dev/pylint>`_ for checking the code.

* Use patman to send your patches (``tools/patman/patman -H`` for full
  instructions). With a few tags in your commits this will check your patches
  and take care of emailing them.

* If you don't use patman, make sure to run ``scripts/checkpatch.pl``. For
  more information, read :doc:`checkpatch`. Note that this should be done
  *before* posting on the mailing list!

* Source files originating from different projects (for example the MTD
  subsystem or the hush shell code from the BusyBox project) may, after
  careful consideration, be exempted from these rules. For such files, the
  original coding style may be kept to ease subsequent migration to newer
  versions of those sources.

* Please also stick to the following formatting rules:

  * Remove any trailing white space

  * Use TAB characters for indentation and vertical alignment, not spaces

    * The exception here is Python which requires 4 spaces instead.

  * All source files need to be in "Unix" and not "DOS" or "Windows" format,
    with respect to line ends.

  * Do not add more than 2 consecutive empty lines to source files

  * Do not add trailing empty lines to source files

  * Using the option ``git config --global color.diff auto`` will help to
    visually see whitespace problems in ``diff`` output from ``git``.

  * In Emacs one can use ``=M-x whitespace-global-mode=`` to get visual
    feedback on the nasty details. ``=M-x whitespace-cleanup=`` does The Right
    Thing (tm)

Submissions of new code or patches that do not conform to these requirements
shall be rejected with a request to reformat the changes.

U-Boot Code Documentation
-------------------------

U-Boot adopted the kernel-doc annotation style, this is the only exception from
multi-line comment rule of Coding Style. While not mandatory, adding
documentation is strongly advised. The Linux kernel `kernel-doc
<https://www.kernel.org/doc/html/latest/doc-guide/kernel-doc.html>`_
documentation applies with no changes.

Our Python code documentation follows `PEP257 (Docstring Conventions)
<https://peps.python.org/pep-0257/>`_.

Use structures for I/O access
-----------------------------

U-Boot typically uses a C structure to map out the registers in an I/O region,
rather than offsets. The reasons for this are:

* It dissociates the register location (offset) from the register type, which
  means the developer has to make sure the type is right for each access,
  whereas with the struct method, this is checked by the compiler;

* It avoids actually writing all offsets, which is (more) error-prone;

* It allows for better compile time sanity-checking of values we write to registers.

Some reasons why you might not use C structures:

* Where the registers appear at different offsets in different hardware
  revisions supported by the same driver

* Where the driver only uses a small subset of registers and it is not worth
  defining a struct to cover them all, with large empty regions

* Where the offset of a register might be hard to figure out when buried a long
  way down a structure, possibly with embedded sub-structures

* This may need to change to the kernel model if we allow for more run-time
  detection of what drivers are appropriate for what we're running on.

Please use the check_member() macro to verify that your structure is the
expected size, or that particular members appear at the right offset.

Include files
-------------

You should follow this ordering in U-Boot. In all cases, they should be listed
in alphabetical order. First comes headers which are located directly in our
top-level include diretory. This excludes the common.h header file which is to
be removed. Second are headers within subdirectories, Finally directory-local
includes should be listed. See this example:

.. code-block:: C

   #include <bootstage.h>
   #include <dm.h>
   #include <others.h>
   #include <asm/...>
   #include <asm/arch/...>
   #include <dm/device_compat.h>
   #include <linux/...>
   #include "local.h"

For files that need to be compiled for the host (e.g. tools), you need to use
``#ifndef USE_HOSTCC`` to avoid including U-Boot specific include files. See
common/image.c for an example.

If you encounter code which still uses <common.h> a patch to remove that and
replace it with any required include files directly is much appreciated.

If your file uses driver model, include <dm.h> in the C file. Do not include
dm.h in a header file. Try to use forward declarations (e.g. ``struct
udevice``) instead.

Filenames
---------

For .c and .h files try to use underscore rather than hyphen unless you want
the file to stand out (e.g. driver-model uclasses should be named xxx-uclass.h.
Avoid upper case and keep the names fairly short.

Function and struct comments
----------------------------

Non-trivial functions should have a comment which describes what they do. If it
is an exported function, put the comment in the header file so the API is in
one place. If it is a static function, put it in the C file.

If the function returns errors, mention that and list the different errors that
are returned. If it is merely passing errors back from a function it calls,
then you can skip that.

See `here
<https://www.kernel.org/doc/html/latest/doc-guide/kernel-doc.html#function-documentation>`_
for style.

Driver model
------------

When declaring a device, try to use ``struct udevice *dev``, i.e. ``dev`` as the name:

.. code-block:: C

   struct udevice *dev;

Use ``ret`` as the return value:

.. code-block:: C

   struct udevice *dev;
   int ret;

   ret = uclass_first_device_err(UCLASS_ACPI_PMC, &dev);
   if (ret)
           return log_msg_ret("pmc", dev);

Consider using log_ret() or log_msg_ret() to return a value (see above).

Add a ``p`` suffix on return arguments:

.. code-block:: C

   int dm_pci_find_class(uint find_class, int index, struct udevice **devp)
   {
   ...
           *devp = dev;

           return 0;
   }

There are standard variable names that you should use in drivers:

* ``struct xxx_priv`` and ``priv`` for dev_get_priv()

* ``struct xxx_plat`` and ``plat`` for dev_get_platdata()

For example:

.. code-block:: C

   struct simple_bus_plat {
      u32 base;
      u32 size;
      u32 target;
   };

   /* Davinci MMC board definitions */
   struct davinci_mmc_priv {
      struct davinci_mmc_regs *reg_base;   /* Register base address */
      uint input_clk;      /* Input clock to MMC controller */
      struct gpio_desc cd_gpio;       /* Card Detect GPIO */
      struct gpio_desc wp_gpio;       /* Write Protect GPIO */
   };

      struct rcar_gpio_priv *priv = dev_get_priv(dev);

      struct pl01x_serial_platdata *plat = dev_get_platdata(dev);

Other
-----

Some minor things:

* Put a blank line before the last ``return`` in a function unless it is the only line:

.. code-block:: C

   struct udevice *pci_get_controller(struct udevice *dev)
   {
      while (device_is_on_pci_bus(dev))
         dev = dev->parent;

      return dev;
   }

Tests
-----

Please add tests when you add code. Please change or expand tests when you change code.

Run the tests with::

   make check
   make qcheck   (skips some tests)

Python tests are in test/py/tests - see the docs in test/py for info.

Try to write your tests in C if you can. For example, tests to check a command
will be much faster (10-100x or more) if they can directly call run_command()
and ut_check_console_line() instead of using Python to send commands over a
pipe to U-Boot.

Tests run all supported CI systems (GitLab, Azure) using scripts in the root of
the U-Boot tree.
