.. SPDX-License-Identifier: GPL-2.0+
.. Copyright (c) 2017, Pantelis Antoniou <pantelis.antoniou@konsulko.com>

Device Tree Overlays
====================

Overlay Syntax
--------------

Device-tree overlays require a slightly different syntax compared to traditional
device-trees. Please refer to dt-object-internal.txt in the device-tree compiler
sources for information regarding the internal format of overlays:
https://git.kernel.org/pub/scm/utils/dtc/dtc.git/tree/Documentation/dt-object-internal.txt

Building Overlays
-----------------

In a nutshell overlays provides a means to manipulate a symbol a previous
device-tree or device-tree overlay has defined. It requires both the base
device-tree and all the overlays to be compiled with the *-@* command line
switch of the device-tree compiler so that symbol information is included.

Note
    Support for *-@* option can only be found in dtc version 1.4.4 or newer.
    Only version 4.14 or higher of the Linux kernel includes a built in version
    of dtc that meets this requirement.

Building a binary device-tree overlay follows the same process as building a
traditional binary device-tree. For example:

**base.dts**

::

	/dts-v1/;
	/ {
		foo: foonode {
			foo-property;
		};
	};

.. code-block:: console

	$ dtc -@ -I dts -O dtb -o base.dtb base.dts

**overlay.dtso**

::

	/dts-v1/;
	/plugin/;
	/ {
		fragment@1 {
			target = <&foo>;
			__overlay__ {
				overlay-1-property;
				bar: barnode {
					bar-property;
				};
			};
		};
	};

.. code-block:: console

	$ dtc -@ -I dts -O dtb -o overlay.dtbo overlay.dtso

Ways to Utilize Overlays in U-Boot
----------------------------------

There are two ways to apply overlays in U-Boot.

* Include and define overlays within a FIT image and have overlays
  automatically applied.

* Manually load and apply overlays

The remainder of this document will discuss using overlays via the manual
approach. For information on using overlays as part of a FIT image please see:
doc/uImage.FIT/overlay-fdt-boot.txt

Manually Loading and Applying Overlays
--------------------------------------

1. Figure out where to place both the base device tree blob and the
   overlay. Make sure you have enough space to grow the base tree without
   overlapping anything.

::

    => setenv fdtaddr 0x87f00000
    => setenv fdtovaddr 0x87fc0000

2. Load the base binary device-tree and the binary device-tree overlay.

::

    => load ${devtype} ${bootpart} ${fdtaddr} ${bootdir}/base.dtb
    => load ${devtype} ${bootpart} ${fdtovaddr} ${bootdir}/overlay.dtbo

3. Set the base binary device-tree as the working fdt tree.

::

    => fdt addr $fdtaddr

4. Grow it enough so it can encompass all applied overlays

::

    => fdt resize 8192

5. You are now ready to apply the overlay.

::

    => fdt apply $fdtovaddr

6. Boot system like you would do with a traditional dtb.

For bootm:

::

    => bootm ${kerneladdr} - ${fdtaddr}

For bootz:

::

    => bootz ${kerneladdr} - ${fdtaddr}

Please note that in case of an error, both the base and overlays are going
to be invalidated, so keep copies to avoid reloading.
