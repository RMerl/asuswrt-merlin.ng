.. SPDX-License-Identifier: GPL-2.0+
.. sectionauthor:: Bin Meng <bmeng.cn@gmail.com>

Galileo
=======

Only one binary blob is needed for Remote Management Unit (RMU) within Intel
Quark SoC. Not like FSP, U-Boot does not call into the binary. The binary is
needed by the Quark SoC itself.

You can get the binary blob from Quark Board Support Package from Intel website:

   * ./QuarkSocPkg/QuarkNorthCluster/Binary/QuarkMicrocode/RMU.bin

Rename the file and put it to the board directory by::

   $ cp RMU.bin board/intel/galileo/rmu.bin

Now you can build U-Boot and obtain u-boot.rom::

   $ make galileo_defconfig
   $ make all
