Calxeda Highbank/Midway board support
=====================================

The Calxeda ECX-1000 ("Highbank") and ECX-2000 ("Midway") were ARM based
servers, providing high-density cluster systems. A single motherboard could
host between 12 and 48 nodes, each with their own quad-core ARMv7
processor, private DRAM and peripherals, connected through a high-bandwith
and low-latency "fabric" network. Multiple motherboards could be connected
together, to extend this fabric.

For the purpose of U-Boot we just care about a single node, this can be
used as a single system, just using the fabric to connect to some Ethernet
network. Each node boots on its own, either from a local hard disk, or
via the network.

The earlier ECX-1000 nodes ("Highbank") contain four ARM Cortex-A9 cores,
a Cortex-M3 system controller, three 10GBit/s MACs and five SATA
controllers. The DRAM is limited to 4GB.

The later ECX-2000 nodes ("Midway") use four Cortex-A15 cores, alongside
two Cortex-A7 management cores, and support up to 32GB of DRAM, while
keeping the other peripherals.

For the purpose of U-Boot those two SoCs are very similar, so we offer
one build target. The subtle differences are handled at runtime.
Calxeda as a company is long defunct, and the remaining systems are
considered legacy at this point.

Bgilding U-Boot
---------------
There is only one defconfig to cover both systems::

    $ make highbank_defconfig
    $ make

This will create ``u-boot.bin``, which could become part of the firmware update
package, or could be chainloaded by the existing U-Boot, see below for more
details.

Boot process
------------
Upon powering up a node (which would be controlled by some BMC style
management controller on the motherboard), the system controller ("ECME")
would start and do some system initialisation (fabric registration,
DRAM init, clock setup). It would load the device tree binary, some secure
monitor code (``a9boot``/``a15boot``) and a U-Boot binary from SPI flash
into DRAM, then power up the actual application cores (ARM Cortex-A9/A15).
They would start executing ``a9boot``/``a15boot``, registering the PSCI SMC
handlers, then dropping into U-Boot, but in non-secure state (HYP mode on
the A15s).

U-Boot would act as a mere loader, trying to find some ``boot.scr`` file on
the local hard disks, or reverting to PXE boot.

Updating U-Boot
---------------
The U-Boot binary is loaded from SPI flash, which is controlled exclusively
by the ECME. This can be reached via IPMI using the LANplus transport protocol.
Updating the SPI flash content requires vendor specific additions to the
IPMI protocol, support for which was never upstreamed to ipmitool or
FreeIPMI. Some older repositories for `ipmitool`_, the `pyipmi`_ library and
a Python `management script`_ to update the SPI flash can be found on Github.

A simpler and safer way to get an up-to-date U-Boot running, is chainloading
it via the legacy U-Boot::

    $ mkimage -A arm -O u-boot -T standalone -C none -a 0x8000 -e 0x8000 \
      -n U-Boot -d u-boot.bin u-boot-highbank.img

Then load this image file, either from hard disk, or via TFTP, from the
existing U-Boot, and execute it with bootm::

    => tftpboot 0x8000 u-boot-highbank.img
    => bootm

.. _`ipmitool`: https://github.com/Cynerva/ipmitool
.. _`pyipmi`: https://pypi.org/project/pyipmi/
.. _`management script`: https://github.com/Cynerva/cxmanage
