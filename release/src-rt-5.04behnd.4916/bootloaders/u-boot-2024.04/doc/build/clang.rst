Building with Clang
===================

The biggest problem when trying to compile U-Boot with Clang is that almost all
archs rely on storing gd in a global register and the Clang 3.5 user manual
states: "Clang does not support global register variables; this is unlikely to
be implemented soon because it requires additional LLVM backend support."

The ARM backend can be instructed not to use the r9 and x18 registers using
-ffixed-r9 or -ffixed-x18 respectively. As global registers themselves are not
supported inline assembly is needed to get and set the r9 or x18 value. This
leads to larger code then strictly necessary, but at least works.

Debian based
------------

Required packages can be installed via apt, e.g.

.. code-block:: bash

    sudo apt-get install clang

We make use of the CROSS_COMPILE variable to derive the build target which is
passed as the --target parameter to clang.

The CROSS_COMPILE variable further determines the paths to other build
tools. As assembler we use the binary pointed to by '$(CROSS_COMPILE)as'
instead of the LLVM integrated assembler (IAS).

Here is an example demonstrating building U-Boot for the Raspberry Pi 2
using clang:

.. code-block:: bash

    make HOSTCC=clang rpi_2_defconfig
    make HOSTCC=clang CROSS_COMPILE=arm-linux-gnueabi- CC=clang -j8

It can also be used to compile sandbox:

.. code-block:: bash

    make HOSTCC=clang sandbox_defconfig
    make HOSTCC=clang CC=clang -j8


FreeBSD 11
----------

Since llvm 3.4 is currently in the base system, the integrated assembler as
is incapable of building U-Boot. Therefore gas from devel/arm-gnueabi-binutils
is used instead. It needs a symlink to be picked up correctly though:

.. code-block:: bash

    ln -s /usr/local/bin/arm-gnueabi-freebsd-as /usr/bin/arm-freebsd-eabi-as

The following commands compile U-Boot using the Clang xdev toolchain.

**NOTE:** CROSS_COMPILE and target differ on purpose!

.. code-block:: bash

    export CROSS_COMPILE=arm-gnueabi-freebsd-
    gmake rpi_2_defconfig
    gmake CC="clang -target arm-freebsd-eabi --sysroot /usr/arm-freebsd" -j8

Given that U-Boot will default to gcc, above commands can be
simplified with a simple wrapper script - saved as
/usr/local/bin/arm-gnueabi-freebsd-gcc - listed below:

.. code-block:: bash

    #!/bin/sh
    exec clang -target arm-freebsd-eabi --sysroot /usr/arm-freebsd "$@"


Known Issues
------------

When build U-boot for `xenguest_arm64_defconfig` target, it reports linkage
error:

.. code-block:: bash

    aarch64-linux-gnu-ld.bfd: drivers/xen/hypervisor.o: in function `do_hypervisor_callback':
    /home/leoy/Dev2/u-boot/drivers/xen/hypervisor.c:188: undefined reference to `__aarch64_swp8_acq_rel'
    aarch64-linux-gnu-ld.bfd: drivers/xen/hypervisor.o: in function `synch_test_and_set_bit':
    /home/leoy/Dev2/u-boot/./arch/arm/include/asm/xen/system.h:40: undefined reference to `__aarch64_ldset1_acq_rel'
    aarch64-linux-gnu-ld.bfd: drivers/xen/hypervisor.o: in function `synch_test_and_clear_bit':
    /home/leoy/Dev2/u-boot/./arch/arm/include/asm/xen/system.h:28: undefined reference to `__aarch64_ldclr1_acq_rel'
    aarch64-linux-gnu-ld.bfd: drivers/xen/hypervisor.o: in function `synch_test_and_set_bit':
    /home/leoy/Dev2/u-boot/./arch/arm/include/asm/xen/system.h:40: undefined reference to `__aarch64_ldset1_acq_rel'
    aarch64-linux-gnu-ld.bfd: drivers/xen/hypervisor.o: in function `synch_test_and_clear_bit':
    /home/leoy/Dev2/u-boot/./arch/arm/include/asm/xen/system.h:28: undefined reference to `__aarch64_ldclr1_acq_rel'
    aarch64-linux-gnu-ld.bfd: drivers/xen/events.o: in function `synch_test_and_clear_bit':
    /home/leoy/Dev2/u-boot/./arch/arm/include/asm/xen/system.h:28: undefined reference to `__aarch64_ldclr1_acq_rel'
    aarch64-linux-gnu-ld.bfd: drivers/xen/events.o: in function `synch_test_and_set_bit':
    /home/leoy/Dev2/u-boot/./arch/arm/include/asm/xen/system.h:40: undefined reference to `__aarch64_ldset1_acq_rel'
    aarch64-linux-gnu-ld.bfd: drivers/xen/gnttab.o: in function `gnttab_end_access':
    /home/leoy/Dev2/u-boot/drivers/xen/gnttab.c:109: undefined reference to `__aarch64_cas2_acq_rel'
    Segmentation fault

To fix the failure, we need to append option `-mno-outline-atomics` in Clang
command to not generate local calls to out-of-line atomic operations:

.. code-block:: bash

    make HOSTCC=clang xenguest_arm64_defconfig
    make HOSTCC=clang CROSS_COMPILE=aarch64-linux-gnu- \
         CC="clang -target aarch64-linux-gnueabi -mno-outline-atomics" -j8
