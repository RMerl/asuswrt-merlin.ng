.. SPDX-License-Identifier: GPL-2.0+

Xen guest ARM64 board
=====================

This board specification
------------------------

This board is to be run as a virtual Xen [1] guest with U-Boot as its primary
bootloader. Xen is a type 1 hypervisor that allows multiple operating systems
to run simultaneously on a single physical server. Xen is capable of running
virtual machines in both full virtualization and para-virtualization (PV)
modes. Xen runs virtual machines, which are called “domains”.

Paravirtualized drivers are a special type of device drivers that are used in
a guest system in the Xen domain and perform I/O operations using a special
interface provided by the virtualization system and the host system.

Xen support for U-Boot is implemented by introducing a new Xen guest ARM64
board and porting essential drivers from MiniOS [3] as well as some of the work
previously done by NXP [4]:

- PV block device frontend driver with XenStore based device enumeration and
  UCLASS_PVBLOCK class;
- PV serial console device frontend driver;
- Virtio block device support;
- Xen hypervisor support with minimal set of the essential headers adapted from
  the Linux kernel;
- Xen grant table support;
- Xen event channel support in polling mode;
- XenBus support;
- dynamic RAM size as defined in the device tree instead of the statically
  defined values;
- position-independent pre-relocation code is used as we cannot statically
  define any start addresses at compile time which is up to Xen to choose at
  run-time;
- new defconfig introduced: xenguest_arm64_defconfig.
- new defconfig introduced: xenguest_arm64_virtio_defconfig.


Board limitations
-----------------

1. U-Boot runs without MMU enabled at the early stages.
   According to Xen on ARM ABI (xen/include/public/arch-arm.h): all memory
   which is shared with other entities in the system (including the hypervisor
   and other guests) must reside in memory which is mapped as Normal Inner
   Write-Back Outer Write-Back Inner-Shareable.
   Thus, page attributes must be equally set for all the entities working with
   that page.
   Before MMU is set up the data cache is turned off and pages are seen by the
   vCPU and Xen in different ways - cacheable by Xen and non-cacheable by vCPU.
   So it means that manual data cache maintenance is required at the early
   stages.

2. No serial console until MMU is up.
   Because data cache maintenance is required until the MMU setup the
   early/debug serial console is not implemented. Therefore, we do not have
   usual prints like U-Boot’s banner etc. until the serial driver is
   initialized.

3. Single RAM bank supported.
   If a Xen guest is given much memory it is possible that Xen allocates two
   memory banks for it. The first one is allocated under 4GB address space and
   in some cases may represent the whole guest’s memory. It is assumed that
   U-Boot most likely won’t require high memory bank for its work andlaunching
   OS, so it is enough to take the first one.


Board default configuration
---------------------------

One can select the configuration as follows:

 - make xenguest_arm64_defconfig

[1] - https://xenproject.org/

[2] - https://wiki.xenproject.org/wiki/Paravirtualization_(PV)

[3] - https://wiki.xenproject.org/wiki/Mini-OS

[4] - https://source.codeaurora.org/external/imx/uboot-imx/tree/?h=imx_v2018.03_4.14.98_2.0.0_ga
