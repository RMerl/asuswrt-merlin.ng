.. SPDX-License-Identifier: GPL-2.0+

Compiled-in Device Tree / Platform Data
=======================================


Introduction
------------

Device tree is the standard configuration method in U-Boot. It is used to
define what devices are in the system and provide configuration information
to these devices.

The overhead of adding devicetree access to U-Boot is fairly modest,
approximately 3KB on Thumb 2 (plus the size of the DT itself). This means
that in most cases it is best to use devicetree for configuration.

However there are some very constrained environments where U-Boot needs to
work. These include SPL with severe memory limitations. For example, some
SoCs require a 16KB SPL image which must include a full MMC stack. In this
case the overhead of devicetree access may be too great.

It is possible to create platform data manually by defining C structures
for it, and reference that data in a `U_BOOT_DRVINFO()` declaration. This
bypasses the use of devicetree completely, effectively creating a parallel
configuration mechanism. But it is an available option for SPL.

As an alternative, the 'of-platdata' feature is provided. This converts the
devicetree contents into C code which can be compiled into the SPL binary.
This saves the 3KB of code overhead and perhaps a few hundred more bytes due
to more efficient storage of the data.


How it works
------------

The feature is enabled by CONFIG OF_PLATDATA. This is only available in
SPL/TPL and should be tested with:

.. code-block:: c

    #if CONFIG_IS_ENABLED(OF_PLATDATA)

A tool called 'dtoc' converts a devicetree file either into a set of
struct declarations, one for each compatible node, and a set of
`U_BOOT_DRVINFO()` declarations along with the actual platform data for each
device. As an example, consider this MMC node:

.. code-block:: none

    sdmmc: dwmmc@ff0c0000 {
            compatible = "rockchip,rk3288-dw-mshc";
            clock-freq-min-max = <400000 150000000>;
            clocks = <&cru HCLK_SDMMC>, <&cru SCLK_SDMMC>,
                     <&cru SCLK_SDMMC_DRV>, <&cru SCLK_SDMMC_SAMPLE>;
            clock-names = "biu", "ciu", "ciu_drv", "ciu_sample";
            fifo-depth = <0x100>;
            interrupts = <GIC_SPI 32 IRQ_TYPE_LEVEL_HIGH>;
            reg = <0xff0c0000 0x4000>;
            bus-width = <4>;
            cap-mmc-highspeed;
            cap-sd-highspeed;
            card-detect-delay = <200>;
            disable-wp;
            num-slots = <1>;
            pinctrl-names = "default";
            pinctrl-0 = <&sdmmc_clk>, <&sdmmc_cmd>, <&sdmmc_cd>, <&sdmmc_bus4>;
                vmmc-supply = <&vcc_sd>;
                status = "okay";
                bootph-all;
        };


Some of these properties are dropped by U-Boot under control of the
CONFIG_OF_SPL_REMOVE_PROPS option. The rest are processed. This will produce
the following C struct declaration:

.. code-block:: c

    struct dtd_rockchip_rk3288_dw_mshc {
            fdt32_t         bus_width;
            bool            cap_mmc_highspeed;
            bool            cap_sd_highspeed;
            fdt32_t         card_detect_delay;
            fdt32_t         clock_freq_min_max[2];
            struct phandle_1_arg clocks[4];
            bool            disable_wp;
            fdt32_t         fifo_depth;
            fdt32_t         interrupts[3];
            fdt32_t         num_slots;
            fdt32_t         reg[2];
            fdt32_t         vmmc_supply;
    };

and the following device declarations:

.. code-block:: c

    /* Node /clock-controller@ff760000 index 0 */
    ...

    /* Node /dwmmc@ff0c0000 index 2 */
    static struct dtd_rockchip_rk3288_dw_mshc dtv_dwmmc_at_ff0c0000 = {
            .fifo_depth             = 0x100,
            .cap_sd_highspeed       = true,
            .interrupts             = {0x0, 0x20, 0x4},
            .clock_freq_min_max     = {0x61a80, 0x8f0d180},
            .vmmc_supply            = 0xb,
            .num_slots              = 0x1,
            .clocks                 = {{0, 456},
                                       {0, 68},
                                       {0, 114},
                                       {0, 118}},
            .cap_mmc_highspeed      = true,
            .disable_wp             = true,
            .bus_width              = 0x4,
            .u_boot_dm_pre_reloc    = true,
            .reg                    = {0xff0c0000, 0x4000},
            .card_detect_delay      = 0xc8,
    };

    U_BOOT_DRVINFO(dwmmc_at_ff0c0000) = {
            .name           = "rockchip_rk3288_dw_mshc",
            .plat       = &dtv_dwmmc_at_ff0c0000,
            .plat_size  = sizeof(dtv_dwmmc_at_ff0c0000),
            .parent_idx     = -1,
    };

The device is then instantiated at run-time and the platform data can be
accessed using:

.. code-block:: c

    struct udevice *dev;
    struct dtd_rockchip_rk3288_dw_mshc *plat = dev_get_plat(dev);

This avoids the code overhead of converting the devicetree data to
platform data in the driver. The `of_to_plat()` method should
therefore do nothing in such a driver.

Note that for the platform data to be matched with a driver, the 'name'
property of the `U_BOOT_DRVINFO()` declaration has to match a driver declared
via `U_BOOT_DRIVER()`. This effectively means that a `U_BOOT_DRIVER()` with a
'name' corresponding to the devicetree 'compatible' string (after converting
it to a valid name for C) is needed, so a dedicated driver is required for
each 'compatible' string.

In order to make this a bit more flexible, the `DM_DRIVER_ALIAS()` macro can be
used to declare an alias for a driver name, typically a 'compatible' string.
This macro produces no code, but is used by dtoc tool. It must be located in the
same file as its associated driver, ideally just after it.

The parent_idx is the index of the parent `driver_info` structure within its
linker list (instantiated by the `U_BOOT_DRVINFO()` macro). This is used to
support `dev_get_parent()`.

During the build process dtoc parses both `U_BOOT_DRIVER()` and
`DM_DRIVER_ALIAS()` to build a list of valid driver names and driver aliases.
If the 'compatible' string used for a device does not not match a valid driver
name, it will be checked against the list of driver aliases in order to get the
right driver name to use. If in this step there is no match found a warning is
issued to avoid run-time failures.

Where a node has multiple compatible strings, dtoc generates a `#define` to
make them equivalent, e.g.:

.. code-block:: c

    #define dtd_rockchip_rk3299_dw_mshc dtd_rockchip_rk3288_dw_mshc


Converting of-platdata to a useful form
---------------------------------------

Of course it would be possible to use the of-platdata directly in your driver
whenever configuration information is required. However this means that the
driver will not be able to support devicetree, since the of-platdata
structure is not available when devicetree is used. It would make no sense
to use this structure if devicetree were available, since the structure has
all the limitations metioned in caveats below.

Therefore it is recommended that the of-platdata structure should be used
only in the `probe()` method of your driver. It cannot be used in the
`of_to_plat()` method since this is not called when platform data is
already present.


How to structure your driver
----------------------------

Drivers should always support devicetree as an option. The of-platdata
feature is intended as a add-on to existing drivers.

Your driver should convert the plat struct in its `probe()` method. The
existing devicetree decoding logic should be kept in the
`of_to_plat()` method and wrapped with `#if`.

For example:

.. code-block:: c

    #include <dt-structs.h>

    struct mmc_plat {
    #if CONFIG_IS_ENABLED(OF_PLATDATA)
            /* Put this first since driver model will copy the data here */
            struct dtd_mmc dtplat;
    #endif
            /*
             * Other fields can go here, to be filled in by decoding from
             * the devicetree (or the C structures when of-platdata is used).
             */
            int fifo_depth;
    };

    static int mmc_of_to_plat(struct udevice *dev)
    {
        if (CONFIG_IS_ENABLED(OF_REAL)) {
            /* Decode the devicetree data */
            struct mmc_plat *plat = dev_get_plat(dev);
            const void *blob = gd->fdt_blob;
            int node = dev_of_offset(dev);

            plat->fifo_depth = fdtdec_get_int(blob, node, "fifo-depth", 0);
        }

        return 0;
    }

    static int mmc_probe(struct udevice *dev)
    {
            struct mmc_plat *plat = dev_get_plat(dev);

    #if CONFIG_IS_ENABLED(OF_PLATDATA)
            /* Decode the of-platdata from the C structures */
            struct dtd_mmc *dtplat = &plat->dtplat;

            plat->fifo_depth = dtplat->fifo_depth;
    #endif
            /* Set up the device from the plat data */
            writel(plat->fifo_depth, ...)
    }

    static const struct udevice_id mmc_ids[] = {
            { .compatible = "vendor,mmc" },
            { }
    };

    U_BOOT_DRIVER(mmc_drv) = {
            .name           = "mmc_drv",
            .id             = UCLASS_MMC,
            .of_match       = mmc_ids,
            .of_to_plat = mmc_of_to_plat,
            .probe          = mmc_probe,
            .priv_auto = sizeof(struct mmc_priv),
            .plat_auto = sizeof(struct mmc_plat),
    };

    DM_DRIVER_ALIAS(mmc_drv, vendor_mmc) /* matches compatible string */

Note that `struct mmc_plat` is defined in the C file, not in a header. This
is to avoid needing to include dt-structs.h in a header file. The idea is to
keep the use of each of-platdata struct to the smallest possible code area.
There is just one driver C file for each struct, that can convert from the
of-platdata struct to the standard one used by the driver.

In the case where SPL_OF_PLATDATA is enabled, `plat_auto` is
still used to allocate space for the platform data. This is different from
the normal behaviour and is triggered by the use of of-platdata (strictly
speaking it is a non-zero `plat_size` which triggers this).

The of-platdata struct contents is copied from the C structure data to the
start of the newly allocated area. In the case where devicetree is used,
the platform data is allocated, and starts zeroed. In this case the
`of_to_plat()` method should still set up the platform data (and the
of-platdata struct will not be present).

SPL must use either of-platdata or devicetree. Drivers cannot use both at
the same time, but they must support devicetree. Supporting of-platdata is
optional.

The devicetree becomes inaccessible when CONFIG_SPL_OF_PLATDATA is enabled,
since the devicetree access code is not compiled in. A corollary is that
a board can only move to using of-platdata if all the drivers it uses support
it. There would be little point in having some drivers require the device
tree data, since then libfdt would still be needed for those drivers and
there would be no code-size benefit.


Build-time instantiation
------------------------

Even with of-platdata there is a fair amount of code required in driver model.
It is possible to have U-Boot handle the instantiation of devices at build-time,
so avoiding the need for the `device_bind()` code and some parts of
`device_probe()`.

The feature is enabled by CONFIG_OF_PLATDATA_INST.

Here is an example device, as generated by dtoc::

   /*
    * Node /serial index 6
    * driver sandbox_serial parent root_driver
   */

   #include <asm/serial.h>
   struct sandbox_serial_plat __attribute__ ((section (".priv_data")))
      _sandbox_serial_plat_serial = {
      .dtplat = {
         .sandbox_text_colour   = "cyan",
      },
   };
   #include <asm/serial.h>
   u8 _sandbox_serial_priv_serial[sizeof(struct sandbox_serial_priv)]
      __attribute__ ((section (".priv_data")));
   #include <serial.h>
   u8 _sandbox_serial_uc_priv_serial[sizeof(struct serial_dev_priv)]
      __attribute__ ((section (".priv_data")));

   DM_DEVICE_INST(serial) = {
      .driver     = DM_DRIVER_REF(sandbox_serial),
      .name       = "sandbox_serial",
      .plat_      = &_sandbox_serial_plat_serial,
      .priv_      = _sandbox_serial_priv_serial,
      .uclass     = DM_UCLASS_REF(serial),
      .uclass_priv_ = _sandbox_serial_uc_priv_serial,
      .uclass_node   = {
         .prev = &DM_UCLASS_REF(serial)->dev_head,
         .next = &DM_UCLASS_REF(serial)->dev_head,
      },
      .child_head   = {
         .prev = &DM_DEVICE_REF(serial)->child_head,
         .next = &DM_DEVICE_REF(serial)->child_head,
      },
      .sibling_node   = {
         .prev = &DM_DEVICE_REF(i2c_at_0)->sibling_node,
         .next = &DM_DEVICE_REF(spl_test)->sibling_node,
      },
      .seq_ = 0,
   };

Here is part of the driver, for reference::

   static const struct udevice_id sandbox_serial_ids[] = {
      { .compatible = "sandbox,serial" },
      { }
   };

   U_BOOT_DRIVER(sandbox_serial) = {
      .name   = "sandbox_serial",
      .id   = UCLASS_SERIAL,
      .of_match    = sandbox_serial_ids,
      .of_to_plat  = sandbox_serial_of_to_plat,
      .plat_auto   = sizeof(struct sandbox_serial_plat),
      .priv_auto   = sizeof(struct sandbox_serial_priv),
      .probe = sandbox_serial_probe,
      .remove = sandbox_serial_remove,
      .ops   = &sandbox_serial_ops,
      .flags = DM_FLAG_PRE_RELOC,
   };


The `DM_DEVICE_INST()` macro declares a struct udevice so you can see that the
members are from that struct. The private data is declared immediately above,
as `_sandbox_serial_priv_serial`, so there is no need for run-time memory
allocation. The #include lines are generated as well, since dtoc searches the
U-Boot source code for the definition of `struct sandbox_serial_priv` and adds
the relevant header so that the code will compile without errors.

The `plat_` member is set to the dtv data which is declared immediately above
the device. This is similar to how it would look without of-platdata-inst, but
node that the `dtplat` member inside is part of the wider
`_sandbox_serial_plat_serial` struct. This is because the driver declares its
own platform data, and the part generated by dtoc can only be a portion of it.
The `dtplat` part is always first in the struct. If the device has no
`.plat_auto` field, then a simple dtv struct can be used as with this example::

   static struct dtd_sandbox_clk dtv_clk_sbox = {
      .assigned_clock_rates   = 0x141,
      .assigned_clocks   = {0x7, 0x3},
   };

   #include <asm/clk.h>
   u8 _sandbox_clk_priv_clk_sbox[sizeof(struct sandbox_clk_priv)]
      __attribute__ ((section (".priv_data")));

   DM_DEVICE_INST(clk_sbox) = {
      .driver    = DM_DRIVER_REF(sandbox_clk),
      .name      = "sandbox_clk",
      .plat_     = &dtv_clk_sbox,

Here is part of the driver, for reference::

   static const struct udevice_id sandbox_clk_ids[] = {
      { .compatible = "sandbox,clk" },
      { }
   };

   U_BOOT_DRIVER(sandbox_clk) = {
      .name       = "sandbox_clk",
      .id         = UCLASS_CLK,
      .of_match   = sandbox_clk_ids,
      .ops        = &sandbox_clk_ops,
      .probe      = sandbox_clk_probe,
      .priv_auto  = sizeof(struct sandbox_clk_priv),
   };


You can see that `dtv_clk_sbox` just has the devicetree contents and there is
no need for the `dtplat` separation, since the driver has no platform data of
its own, besides that provided by the devicetree (i.e. no `.plat_auto` field).

The doubly linked lists are handled by explicitly declaring the value of each
node, as you can see with the `.prev` and `.next` values in the example above.
Since dtoc knows the order of devices it can link them into the appropriate
lists correctly.

One of the features of driver model is the ability for a uclass to have a
small amount of private data for each device in that uclass. This is used to
provide a generic data structure that the uclass can use for all devices, thus
allowing generic features to be implemented in common code. An example is I2C,
which stores the bus speed there.

Similarly, parent devices can have data associated with each of their children.
This is used to provide information common to all children of a particular bus.
For an I2C bus, this is used to store the I2C address of each child on the bus.

This is all handled automatically by dtoc::

   #include <asm/i2c.h>
   u8 _sandbox_i2c_priv_i2c_at_0[sizeof(struct sandbox_i2c_priv)]
      __attribute__ ((section (".priv_data")));
   #include <i2c.h>
   u8 _sandbox_i2c_uc_priv_i2c_at_0[sizeof(struct dm_i2c_bus)]
      __attribute__ ((section (".priv_data")));

   DM_DEVICE_INST(i2c_at_0) = {
      .driver      = DM_DRIVER_REF(sandbox_i2c),
      .name      = "sandbox_i2c",
      .plat_   = &dtv_i2c_at_0,
      .priv_      = _sandbox_i2c_priv_i2c_at_0,
      .uclass   = DM_UCLASS_REF(i2c),
      .uclass_priv_ = _sandbox_i2c_uc_priv_i2c_at_0,
     ...

Part of driver, for reference::

   static const struct udevice_id sandbox_i2c_ids[] = {
      { .compatible = "sandbox,i2c" },
      { }
   };

   U_BOOT_DRIVER(sandbox_i2c) = {
      .name   = "sandbox_i2c",
      .id   = UCLASS_I2C,
      .of_match = sandbox_i2c_ids,
      .ops   = &sandbox_i2c_ops,
      .priv_auto   = sizeof(struct sandbox_i2c_priv),
   };

Part of I2C uclass, for reference::

   UCLASS_DRIVER(i2c) = {
      .id         = UCLASS_I2C,
      .name       = "i2c",
      .flags      = DM_UC_FLAG_SEQ_ALIAS,
      .post_bind  = i2c_post_bind,
      .pre_probe  = i2c_pre_probe,
      .post_probe = i2c_post_probe,
      .per_device_auto   = sizeof(struct dm_i2c_bus),
      .per_child_plat_auto   = sizeof(struct dm_i2c_chip),
      .child_post_bind = i2c_child_post_bind,
   };

Here, `_sandbox_i2c_uc_priv_i2c_at_0` is required by the uclass but is declared
in the device, as required by driver model. The required header file is included
so that the code will compile without errors. A similar mechanism is used for
child devices, but is not shown by this example.

It would not be that useful to avoid binding devices but still need to allocate
uclasses at runtime. So dtoc generates uclass instances as well::

   struct list_head uclass_head = {
      .prev = &DM_UCLASS_REF(serial)->sibling_node,
      .next = &DM_UCLASS_REF(clk)->sibling_node,
   };

   DM_UCLASS_INST(clk) = {
      .uc_drv      = DM_UCLASS_DRIVER_REF(clk),
      .sibling_node   = {
         .prev = &uclass_head,
         .next = &DM_UCLASS_REF(i2c)->sibling_node,
      },
      .dev_head   = {
         .prev = &DM_DEVICE_REF(clk_sbox)->uclass_node,
         .next = &DM_DEVICE_REF(clk_fixed)->uclass_node,
      },
   };

At the top is the list head. Driver model uses this on start-up, instead of
creating its own.

Below that are a set of `DM_UCLASS_INST()` macros, each declaring a
`struct uclass`. The doubly linked lists work as for devices.

All private data is placed into a `.priv_data` section so that it is contiguous
in the resulting output binary.


Indexes
-------

U-Boot stores drivers, devices and many other things in linker_list structures.
These are sorted by name, so dtoc knows the order that they will appear when
the linker runs. Each driver_info / udevice is referenced by its index in the
linker_list array, called 'idx' in the code.

When CONFIG_OF_PLATDATA_INST is enabled, idx is the udevice index, otherwise it
is the driver_info index. In either case, indexes are used to reference devices
using device_get_by_ofplat_idx(). This allows phandles to work as expected.


Phases
------

U-Boot operates in several phases, typically TPL, SPL and U-Boot proper.
The latter does not use dtoc.

In some rare cases different drivers are used for two phases. For example,
in TPL it may not be necessary to use the full PCI subsystem, so a simple
driver can be used instead.

This works in the build system simply by compiling in one driver or the
other (e.g. PCI driver + uclass for SPL; simple_bus for TPL). But dtoc has
no way of knowing which code is compiled in for which phase, since it does
not inspect Makefiles or dependency graphs.

So to make this work for dtoc, we need to be able to explicitly mark
drivers with their phase. This is done by adding a macro to the driver::

   /* code in tpl.c only compiled into TPL */
   U_BOOT_DRIVER(pci_x86) = {
      .name   = "pci_x86",
      .id   = UCLASS_SIMPLE_BUS,
      .of_match = of_match_ptr(tpl_fake_pci_ids),
      DM_PHASE(tpl)
   };


   /* code in pci_x86.c compiled into SPL and U-Boot proper */
   U_BOOT_DRIVER(pci_x86) = {
      .name   = "pci_x86",
      .id   = UCLASS_PCI,
      .of_match = pci_x86_ids,
      .ops   = &pci_x86_ops,
   };


Notice that the second driver has the same name but no DM_PHASE(), so it will be
used for SPL and U-Boot.

Note also that this only affects the code generated by dtoc. You still need to
make sure that only the required driver is build into each phase.


Header files
------------

With OF_PLATDATA_INST, dtoc must include the correct header file in the
generated code for any structs that are used, so that the code will compile.
For example, if `struct ns16550_plat` is used, the code must include the
`ns16550.h` header file.

Typically dtoc can detect the header file needed for a driver by looking
for the structs that it uses. For example, if a driver as a `.priv_auto`
that uses `struct ns16550_plat`, then dtoc can search header files for the
definition of that struct and use the file.

In some cases, enums are used in drivers, typically with the `.data` field
of `struct udevice_id`. Since dtoc does not support searching for these,
you must use the `DM_HDR()` macro to tell dtoc which header to use. This works
as a macro included in the driver definition::

   static const struct udevice_id apl_syscon_ids[] = {
      { .compatible = "intel,apl-punit", .data = X86_SYSCON_PUNIT },
      { }
   };

   U_BOOT_DRIVER(intel_apl_punit) = {
      .name       = "intel_apl_punit",
      .id         = UCLASS_SYSCON,
      .of_match   = apl_syscon_ids,
      .probe      = apl_punit_probe,
      DM_HEADER(<asm/cpu.h>)    /* for X86_SYSCON_PUNIT */
   };



Problems
--------

This section shows some common problems and how to fix them.

Driver not found
~~~~~~~~~~~~~~~~

In some cases you will you see something like this::

   WARNING: the driver rockchip_rk3188_grf was not found in the driver list

The driver list is a list of drivers, each with a name. The name is in the
U_BOOT_DRIVER() declaration, repeated twice, one in brackets and once as the
.name member. For example, in the following declaration the driver name is
`rockchip_rk3188_grf`::

  U_BOOT_DRIVER(rockchip_rk3188_grf) = {
       .name = "rockchip_rk3188_grf",
       .id = UCLASS_SYSCON,
       .of_match = rk3188_syscon_ids + 1,
       .bind = rk3188_syscon_bind_of_plat,
  };

The first name U_BOOT_DRIVER(xx) is used to create a linker symbol so that the
driver can be accessed at build-time without any overhead. The second one
(.name = "xx") is used at runtime when something wants to print out the driver
name.

The dtoc tool expects to be able to find a driver for each compatible string in
the devicetree. For example, if the devicetree has::

   grf: grf@20008000 {
      compatible = "rockchip,rk3188-grf", "syscon";
      reg = <0x20008000 0x200>;
      bootph-pre-ram;
   };

then dtoc looks at the first compatible string ("rockchip,rk3188-grf"),
converts that to a C identifier (rockchip_rk3188_grf) and then looks for that.

Missing .compatible or Missing .id
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Various things can cause dtoc to fail to find the driver and it tries to
warn about these. For example::

   rockchip_rk3188_uart: Missing .compatible in drivers/serial/serial_rockchip.c
                    : WARNING: the driver rockchip_rk3188_uart was not found in the driver list

Without a compatible string a driver cannot be used by dtoc, even if the
compatible string is not actually needed at runtime.

If the problem is simply that there are multiple compatible strings, the
DM_DRIVER_ALIAS() macro can be used to tell dtoc about this and avoid a problem.

Checks are also made to confirm that the referenced driver has a .compatible
member and a .id member. The first provides the array of compatible strings and
the second provides the uclass ID.

Missing parent
~~~~~~~~~~~~~~

When a device is used, its parent must be present as well. If you see an error
like::

   Node '/i2c@0/emul/emul0' requires parent node '/i2c@0/emul' but it is not in
      the valid list

it indicates that you are using a node whose parent is not present in the
devicetree. In this example, if you look at the device tree output
(e.g. fdtdump tpl/u-boot-tpl.dtb in your build directory), you may see something
like this::

   emul {
       emul0 {
           compatible = "sandbox,i2c-rtc-emul";
           #emul-cells = <0x00000000>;
           phandle = <0x00000003>;
       };
   };

In this example, 'emul0' exists but its parent 'emul' has no properties. These
have been dropped by fdtgrep in an effort to reduce the devicetree size. This
indicates that the two nodes have different phase settings. Looking at the
source .dts::

   i2c_emul: emul {
      bootph-pre-ram;
      reg = <0xff>;
      compatible = "sandbox,i2c-emul-parent";
      emul0: emul0 {
         bootph-all;
         compatible = "sandbox,i2c-rtc-emul";
         #emul-cells = <0>;
      };
   };

you can see that the child node 'emul0' usees 'bootph-all', indicating
that the node is present in all SPL builds, but its parent uses
'bootph-pre-ram' indicating it is only present in SPL, not TPL. For a TPL
build, this will fail with the above message. The fix is to change 'emul0' to
use the same 'bootph-pre-ram' condition, so that it is not present in TPL,
like its parent.

Link errors / undefined reference
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Sometimes dtoc does not find the problem for you, but something is wrong and
you get a link error, e.g.::

   :(__u_boot_list_2_udevice_2_spl_test5+0x0): undefined reference to
      `_u_boot_list_2_driver_2_sandbox_spl_test'
   /usr/bin/ld: dts/dt-uclass.o:(__u_boot_list_2_uclass_2_misc+0x8):
        undefined reference to `_u_boot_list_2_uclass_driver_2_misc'

The first one indicates that the device cannot find its driver. This means that
there is a driver 'sandbox_spl_test' but it is not compiled into the build.
Check your Kconfig settings to make sure it is. If you don't want that in the
build, adjust your phase settings, e.g. by using 'bootph-pre-ram' in the node
to exclude it from the TPL build::

	spl-test5 {
		bootph-pre-sram;
		compatible = "sandbox,spl-test";
		stringarray = "tpl";
	};

We can drop the 'bootph-pre-sram' line so this node won't appear in the TPL
devicetree and thus the driver won't be needed.

The second error above indicates that the MISC uclass is needed by the driver
(since it is in the MISC uclass) but that uclass is not compiled in the build.
The fix above would fix this error too. But if you do want this uclass in the
build, check your Kconfig settings to make sure the uclass is being built
(CONFIG_MISC in this case).

Another error that can crop up is something like::

   spl/dts/dt-device.c:257:38: error: invalid application of ‘sizeof’ to
         incomplete type ‘struct sandbox_irq_priv’
      257 | u8 _sandbox_irq_priv_irq_sbox[sizeof(struct sandbox_irq_priv)]
          |                                      ^~~~~~

This indicates that `struct sandbox_irq_priv` is not defined anywhere. The
solution is to add a DM_HEADER() line, as below, so this is included in the
dt-device.c file::

   U_BOOT_DRIVER(sandbox_irq) = {
      .name		= "sandbox_irq",
      .id		= UCLASS_IRQ,
      .of_match	= sandbox_irq_ids,
      .ops		= &sandbox_irq_ops,
      .priv_auto	= sizeof(struct sandbox_irq_priv),
      DM_HEADER(<asm/irq.h>)
   };

Note that there is no dependency checking on the above, so U-Boot will not
regenerate the dt-device.c file when you update the source file (here,
`irq_sandbox.c`). You need to run `make mrproper` first to get a fresh build.

Another error that can crop up is something like::

   spl/dts/dt-device.c:257:38: error: invalid application of ‘sizeof’ to
         incomplete type ‘struct sandbox_irq_priv’
      257 | u8 _sandbox_irq_priv_irq_sbox[sizeof(struct sandbox_irq_priv)]
          |                                      ^~~~~~

This indicates that `struct sandbox_irq_priv` is not defined anywhere. The
solution is to add a DM_HEADER() line, as below, so this is included in the
dt-device.c file::

   U_BOOT_DRIVER(sandbox_irq) = {
      .name		= "sandbox_irq",
      .id		= UCLASS_IRQ,
      .of_match	= sandbox_irq_ids,
      .ops		= &sandbox_irq_ops,
      .priv_auto	= sizeof(struct sandbox_irq_priv),
      DM_HEADER(<asm/irq.h>)
   };

Note that there is no dependency checking on the above, so U-Boot will not
regenerate the dt-device.c file when you update the source file (here,
`irq_sandbox.c`). You need to run `make mrproper` first to get a fresh build.


Caveats
-------

There are various complications with this feature which mean it should only
be used when strictly necessary, i.e. in SPL with limited memory. Notable
caveats include:

   - Device tree does not describe data types. But the C code must define a
     type for each property. These are guessed using heuristics which
     are wrong in several fairly common cases. For example an 8-byte value
     is considered to be a 2-item integer array, and is byte-swapped. A
     boolean value that is not present means 'false', but cannot be
     included in the structures since there is generally no mention of it
     in the devicetree file.

   - Naming of nodes and properties is automatic. This means that they follow
     the naming in the devicetree, which may result in C identifiers that
     look a bit strange.

   - It is not possible to find a value given a property name. Code must use
     the associated C member variable directly in the code. This makes
     the code less robust in the face of devicetree changes. To avoid having
     a second struct with similar members and names you need to explicitly
     declare it as an alias with `DM_DRIVER_ALIAS()`.

   - The platform data is provided to drivers as a C structure. The driver
     must use the same structure to access the data. Since a driver
     normally also supports devicetree it must use `#ifdef` to separate
     out this code, since the structures are only available in SPL. This could
     be fixed fairly easily by making the structs available outside SPL, so
     that `IS_ENABLED()` could be used.

   - With CONFIG_OF_PLATDATA_INST all binding happens at build-time, meaning
     that (by default) it is not possible to call `device_bind()` from C code.
     This means that all devices must have an associated devicetree node and
     compatible string. For example if a GPIO device currently creates child
     devices in its `bind()` method, it will not work with
     CONFIG_OF_PLATDATA_INST. Arguably this is bad practice anyway and the
     devicetree binding should be updated to declare compatible strings for
     the child devices. It is possible to disable OF_PLATDATA_NO_BIND but this
     is not recommended since it increases code size.


Internals
---------

Generated files
~~~~~~~~~~~~~~~

When enabled, dtoc generates the following five files:

include/generated/dt-decl.h (OF_PLATDATA_INST only)
   Contains declarations for all drivers, devices and uclasses. This allows
   any `struct udevice`, `struct driver` or `struct uclass` to be located by its
   name

include/generated/dt-structs-gen.h
   Contains the struct definitions for the devicetree nodes that are used. This
   is the same as without OF_PLATDATA_INST

spl/dts/dt-plat.c (only with !OF_PLATDATA_INST)
   Contains the `U_BOOT_DRVINFO()` declarations that U-Boot uses to bind devices
   at start-up. See above for an example

spl/dts/dt-device.c (only with OF_PLATDATA_INST)
   Contains `DM_DEVICE_INST()` declarations for each device that can be used at
   run-time. These are declared in the file along with any private/platform data
   that they use. Every device has an idx, as above. Since each device must be
   part of a double-linked list, the nodes are declared in the code as well.

spl/dts/dt-uclass.c (only with OF_PLATDATA_INST)
   Contains `DM_UCLASS_INST()` declarations for each uclass that can be used at
   run-time. These are declared in the file along with any private data
   associated with the uclass itself (the `.priv_auto` member). Since each
   uclass must be part of a double-linked list, the nodes are declared in the
   code as well.

The dt-structs.h file includes the generated file
`(include/generated/dt-structs.h`) if CONFIG_SPL_OF_PLATDATA is enabled.
Otherwise (such as in U-Boot proper) these structs are not available. This
prevents them being used inadvertently. All usage must be bracketed with
`#if CONFIG_IS_ENABLED(OF_PLATDATA)`.

The dt-plat.c file contains the device declarations and is is built in
spl/dt-plat.c.


CONFIG options
~~~~~~~~~~~~~~

Several CONFIG options are used to control the behaviour of of-platdata, all
available for both SPL and TPL:

OF_PLATDATA
   This is the main option which enables the of-platdata feature

OF_PLATDATA_PARENT
   This allows `device_get_parent()` to work. Without this, all devices exist as
   direct children of the root node. This option is highly desirable (if not
   always absolutely essential) for buses such as I2C.

OF_PLATDATA_INST
   This controls the instantiation of devices at build time. With it disabled,
   only `U_BOOT_DRVINFO()` records are created, with U-Boot handling the binding
   in `device_bind()` on start-up. With it enabled, only `DM_DEVICE_INST()` and
   `DM_UCLASS_INST()` records are created, and `device_bind()` is not needed at
   runtime.

OF_PLATDATA_NO_BIND
   This controls whether `device_bind()` is supported. It is enabled by default
   with OF_PLATDATA_INST since code-size reduction is really the main point of
   the feature. It can be disabled if needed but is not likely to be supported
   in the long term.

OF_PLATDATA_DRIVER_RT
   This controls whether the `struct driver_rt` records are used by U-Boot.
   Normally when a device is bound, U-Boot stores the device pointer in one of
   these records. There is one for every `struct driver_info` in the system,
   i.e. one for every device that is bound from those records. It provides a
   way to locate a device in the code and is used by
   `device_get_by_ofplat_idx()`. This option is always enabled with of-platdata,
   provided OF_PLATDATA_INST is not. In that case the records are useless since
   we don't have any `struct driver_info` records.

OF_PLATDATA_RT
   This controls whether the `struct udevice_rt` records are used by U-Boot.
   It moves the updatable fields from `struct udevice` (currently only `flags`)
   into a separate structure, allowing the records to be kept in read-only
   memory. It is generally enabled if OF_PLATDATA_INST is enabled. This option
   also controls whether the private data is used in situ, or first copied into
   an allocated region. Again this is to allow the private data declared by
   dtoc-generated code to be in read-only memory. Note that access to private
   data must be done via accessor functions, such as `dev_get_priv()`, so that
   the relocation is handled.

READ_ONLY
   This indicates that the data generated by dtoc should not be modified. Only
   a few fields actually do get changed in U-Boot, such as device flags. This
   option causes those to move into an allocated space (see OF_PLATDATA_RT).
   Also, since updating doubly linked lists is generally impossible when some of
   the nodes cannot be updated, OF_PLATDATA_NO_BIND is enabled.

Data structures
~~~~~~~~~~~~~~~

A few extra data structures are used with of-platdata:

`struct udevice_rt`
   Run-time information for devices. When OF_PLATDATA_RT is enabled, this holds
   the flags for each device, so that `struct udevice` can remain unchanged by
   U-Boot, and potentially reside in read-only memory. Access to flags is then
   via functions like `dev_get_flags()` and `dev_or_flags()`. This data
   structure is allocated on start-up, where the private data is also copied.
   All flags values start at 0 and any changes are handled by `dev_or_flags()`
   and `dev_bic_flags()`. It would be more correct for the flags to be set to
   `DM_FLAG_BOUND`, or perhaps `DM_FLAG_BOUND | DM_FLAG_ALLOC_PDATA`, but since
   there is no code to bind/unbind devices and no code to allocate/free
   private data / platform data, it doesn't matter.

`struct driver_rt`
   Run-time information for `struct driver_info` records. When
   OF_PLATDATA_DRIVER_RT is enabled, this holds a pointer to the device
   created by each record. This is needed so that is it possible to locate a
   device from C code. Specifically, the code can use `DM_DRVINFO_GET(name)` to
   get a reference to a particular `struct driver_info`, with `name` being the
   name of the devicetree node. This is very convenient. It is also fast, since
   no    searching or string comparison is needed. This data structure is
   allocated    on start-up, filled out by `device_bind()` and used by
   `device_get_by_ofplat_idx()`.

Other changes
~~~~~~~~~~~~~

Some other changes are made with of-platdata:

Accessor functions
   Accessing private / platform data via functions such as `dev_get_priv()` has
   always been encouraged. With OF_PLATDATA_RT this is essential, since the
   `priv_` and `plat_`  (etc.) values point to the data generated by dtoc, not
   the read-write copy that is sometimes made on start-up. Changing the
   private / platform data  pointers has always been discouraged (the API is
   marked internal) but with OF_PLATDATA_RT this is not currently supported in
   general, since it assumes that all such pointers point to the relocated data.
   Note also that the renaming of struct members to have a trailing underscore
   was partly done to make people aware that they should not be accessed
   directly.

`gd->uclass_root_s`
   Normally U-Boot sets up the head of the uclass list here and makes
   `gd->uclass_root` point to it. With OF_PLATDATA_INST, dtoc generates a
   declaration of `uclass_head` in `dt-uclass.c` since it needs to link the
   head node into the list. In that case, `gd->uclass_root_s` is not used and
   U-Boot just makes `gd->uclass_root` point to `uclass_head`.

`gd->dm_driver_rt`
   This holds a pointer to a list of `struct driver_rt` records, one for each
   `struct driver_info`. The list is in alphabetical order by the name used
   in `U_BOOT_DRVINFO(name)` and indexed by idx, with the first record having
   an index of 0. It is only used if OF_PLATDATA_INST is not enabled. This is
   accessed via macros so that it can be used inside IS_ENABLED(), rather than
   requiring #ifdefs in the C code when it is not present.

`gd->dm_udevice_rt`
   This holds a pointer to a list of `struct udevice_rt` records, one for each
   `struct udevice`. The list is in alphabetical order by the name used
   in `DM_DEVICE_INST(name)` (a C version of the devicetree node) and indexed by
   idx, with the first record having an index of 0. It is only used if
   OF_PLATDATA_INST is enabled. This is accessed via macros so that it can be
   used inside `IS_ENABLED()`, rather than requiring #ifdefs in the C code when
   it is not present.

`gd->dm_priv_base`
   When OF_PLATDATA_RT is enabled, the private/platform data for each device is
   copied into an allocated region by U-Boot on start-up. This points to that
   region. All calls to accessor functions (e.g. `dev_get_priv()`) then
   translate from the pointer provided by the caller (assumed to lie between
   `__priv_data_start` and `__priv_data_end`) to the new allocated region. This
   member is accessed via macros so that it can be used inside IS_ENABLED(),
   rather than required #ifdefs in the C code when it is not present.

`struct udevice->flags_`
   When OF_PLATDATA_RT is enabled, device flags are no-longer part of
   `struct udevice`, but are instead kept in `struct udevice_rt`, as described
   above. Flags are accessed via functions, such as `dev_get_flags()` and
   `dev_or_flags()`.

`struct udevice->node_`
   When OF_PLATDATA is enabled, there is no devicetree at runtime, so no need
   for this field. It is removed, just to save space.

`DM_PHASE`
   This macro is used to indicate which phase of U-Boot a driver is intended
   for. See above for details.

`DM_HDR`
   This macro is used to indicate which header file dtoc should use to allow
   a driver declaration to compile correctly. See above for details.

`device_get_by_ofplat_idx()`
   There used to be a function called `device_get_by_driver_info()` which
   looked up a `struct driver_info` pointer and returned the `struct udevice`
   that was created from it. It was only available for use with of-platdata.
   This has been removed in favour of `device_get_by_ofplat_idx()` which uses
   `idx`, the index of the `struct driver_info` or `struct udevice` in the
   linker_list. Similarly, the `struct phandle_0_arg` (etc.) structs have been
   updated to use this index instead of a pointer to `struct driver_info`.

`DM_DRVINFO_GET`
   This has been removed since we now use indexes to obtain a driver from
   `struct phandle_0_arg` and the like.

Two-pass binding
   The original of-platdata tried to order `U_BOOT_DRVINFO()` in the generated
   files so as to have parents declared ahead of children. This was convenient
   as it avoided any special code in U-Boot. With OF_PLATDATA_INST this does
   not work as the idx value relies on using alphabetical order for everything,
   so that dtoc and U-Boot's linker_lists agree on the idx value. Devices are
   then bound in order of idx, having no regard to parent/child relationships.
   For this reason, device binding now hapens in multiple passes, with parents
   being bound before their children. This is important so that children can
   find their parents in the bind() method if needed.

Root device
   The root device is generally bound by U-Boot but with OF_PLATDATA_INST it
   cannot be, since binding needs to be done at build time. So in this case
   dtoc sets up a root device using `DM_DEVICE_INST()` in `dt-device.c` and
   U-Boot makes use of that. When OF_PLATDATA_INST is not enabled, U-Boot
   generally ignores the root node and does not create a `U_BOOT_DRVINFO()`
   record for it. This means that the idx numbers used by `struct driver_info`
   (when OF_PLATDATA_INST is disabled) and the idx numbers used by
   `struct udevice` (when OF_PLATDATA_INST is enabled) differ, since one has a
   root node and the other does not. This does not actually matter, since only
   one of them is actually used for any particular build, but it is worth
   keeping in mind if comparing index values and switching OF_PLATDATA_INST on
   and off.

`__priv_data_start` and `__priv_data_end`
   The private/platform data declared by dtoc is all collected together in
   a linker section and these symbols mark the start and end of it. This allows
   U-Boot to relocate the area to a new location if needed (with
   OF_PLATDATA_RT)

`dm_priv_to_rw()`
   This function converts a private- or platform-data pointer value generated by
   dtoc into one that can be used by U-Boot. It is a NOP unless OF_PLATDATA_RT
   is enabled, in which case it translates the address to the relocated
   region. See above for more information.

The dm_populate_phandle_data() function that was previous needed has now been
removed, since dtoc can address the drivers directly from dt-plat.c and does
not need to fix up things at runtime.

The pylibfdt Python module is used to access the devicetree.


Credits
-------

This is an implementation of an idea by Tom Rini <trini@konsulko.com>.


Future work
-----------
- Consider programmatically reading binding files instead of devicetree
  contents
- Allow IS_ENABLED() to be used in the C code instead of #if


.. Simon Glass <sjg@chromium.org>
.. Google, Inc
.. 6/6/16
.. Updated Independence Day 2016
.. Updated 1st October 2020
.. Updated 5th February 2021
