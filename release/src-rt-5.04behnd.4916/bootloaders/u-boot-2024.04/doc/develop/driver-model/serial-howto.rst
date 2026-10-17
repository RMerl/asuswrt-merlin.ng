.. SPDX-License-Identifier: GPL-2.0+

How to port a serial driver to driver model
===========================================

Here is a suggested approach for converting your serial driver over to driver
model. Please feel free to update this file with your ideas and suggestions.

- #ifdef out all your own serial driver code (#ifndef CONFIG_DM_SERIAL)
- Define CONFIG_DM_SERIAL for your board, vendor or architecture
- If the board does not already use driver model, you need CONFIG_DM also
- Your board should then build, but will not boot since there will be no serial
  driver
- Add the U_BOOT_DRIVER piece at the end (e.g. copy serial_s5p.c for example)
- Add a private struct for the driver data - avoid using static variables
- Implement each of the driver methods, perhaps by calling your old methods
- You may need to adjust the function parameters so that the old and new
  implementations can share most of the existing code
- If you convert all existing users of the driver, remove the pre-driver-model
  code

In terms of patches a conversion series typically has these patches:
- clean up / prepare the driver for conversion
- add driver model code
- convert at least one existing board to use driver model serial
- (if no boards remain that don't use driver model) remove the old code

This may be a good time to move your board to use the device tree too. Mostly
this involves these steps:

- define CONFIG_OF_CONTROL and CONFIG_OF_SEPARATE
- add your device tree files to arch/<arch>/dts
- update the Makefile there
- Add stdout-path to your /chosen device tree node if it is not already there
- build and get u-boot-dtb.bin so you can test it
- Your drivers can now use device tree
- For device tree in SPL, define CONFIG_SPL_OF_CONTROL


Converting boards to CONFIG_DM_SERIAL
-------------------------------------

If your SoC has a serial driver that uses driver model (has U_BOOT_DRIVER() in
it), then you may still find that your board has not been converted. To convert
your board, enable the option and see if you can get it working.

Firstly you will have a lot more success if you have a method of debugging your
board, such as a JTAG connection. Failing that the debug UART is useful,
although since you are trying to get the UART driver running, it will interfere
with your efforts eventually.

Secondly, while the UART is a relatively simple peripheral, it may need quite a
few pieces to be up and running before it will work, such as the correct pin
muxing, clocks, power domains and possibly even GPIOs, if an external
transceiver is used. Look at other boards that use the same SoC, for clues as to
what is needed.

Thirdly, when added tags, put them in a xxx-u-boot.dtsi file, where xxx is your
board name, or SoC name. There may already be a file for your SoC which contains
what you need. U-Boot automatically includes these files: see :ref:`dttweaks`.

Here are some things you might need to consider:

1. The serial driver itself needs to be present before relocation, so that the
   U-Boot banner appears. Make sure it has a bootph-all tag in the device
   tree, so that the serial driver is bound when U-Boot starts.

   For example, on iMX8::

       lpuart3: serial@5a090000 {
           compatible = "fsl,imx8qm-lpuart";
           ...
       };

   put this in your xxx-u-boot.dtsi file::

       &lpuart3 {
           bootph-some-ram;
       };

2. If your serial port requires a particular pinmux configuration, you may need
   a pinctrl driver. This needs to have a bootph-all tag also. Take care
   that any subnodes have the same tag, if they are needed to make the correct
   pinctrl available.

   For example, on RK3288, the UART2 uses uart2_xfer::

       uart2: serial@ff690000 {
           ...
           pinctrl-0 = <&uart2_xfer>;
       };

   which is defined as follows::

       pinctrl: pinctrl {
           compatible = "rockchip,rk3228-pinctrl";

           uart2: uart2 {
               uart2_xfer: uart2-xfer {
                   rockchip,pins = <1 RK_PC2 RK_FUNC_2 &pcfg_pull_up>,
                         <1 RK_PC3 RK_FUNC_2 &pcfg_pull_none>;
           };
           ...
       };

   This means you must make the uart2-xfer node available as well as all its
   parents, so put this in your xxx-u-boot.dtsi file::

       &pinctrl {
           bootph-all;
       };

       &uart2 {
           bootph-all;
       };

       &uart2_xfer {
           bootph-all;
       };

3. The same applies to power domains. For example, if a particular power domain
   must be enabled for the serial port to work, you need to ensure it is
   available before relocation:

   For example, on iMX8, put this in your xxx-u-boot.dtsi file::

       &pd_dma {
           bootph-some-ram;
       };

       &pd_dma_lpuart3 {
           bootph-some-ram;
       };

4. The same applies to clocks, in the same way. Make sure that when your driver
   requests a clock, typically with clk_get_by_index(), it is available.


Generally a failure to find a required device will cause an error which you can
catch, if you have the debug UART working. U-Boot outputs serial data to the
debug UART until the point where the real serial driver takes over. This point
is marked by gd->flags having the GD_FLG_SERIAL_READY flag set. This change
happens in serial_init() in serial-uclass.c so until that point the debug UART
is used. You can see the relevant code in putc()
, for example::

   /* if we don't have a console yet, use the debug UART */
   if (IS_ENABLED(CONFIG_DEBUG_UART) && !(gd->flags & GD_FLG_SERIAL_READY)) {
      printch(c);
      return;
   }
   ... carries on to use the console / serial driver

Note that in device_probe() the call to pinctrl_select_state() silently fails
if the pinctrl driver fails. You can add a temporary check there if needed.

Why do we have all these tags? The problem is that before relocation we don't
want to bind all the drivers since memory is limited and the CPU may be running
at a slow speed. So many boards will fail to boot without this optimisation, or
may take a long time to start up (e.g. hundreds of milliseconds). The tags tell
U-Boot which drivers to bind.

The good news is that this problem is normally solved by the SoC, so that any
boards that use it will work as normal. But in some cases there are multiple
UARTs or multiple pinmux options, which means that each board may need to do
some customisation.

Serial in SPL
-------------

A similar process is needed in SPL, but in this case the bootph-pre-ram or
bootph-pre-sram tags are used. Add these in the same way as above, to ensure
that the SPL device tree contains the required nodes (see spl/u-boot-spl.dtb
for what it actually contains).

Removing old code
-----------------

In some cases there may be initialisation code that is no-longer needed when
driver model is used, such as setting up the pin muxing, or enabling a clock.
Be sure to remove this.

Example patch
-------------

See this serial_patch_ for iMX7.

.. _serial_patch: https://patchwork.ozlabs.org/project/uboot/patch/20220314232406.1945308-1-festevam@gmail.com/
