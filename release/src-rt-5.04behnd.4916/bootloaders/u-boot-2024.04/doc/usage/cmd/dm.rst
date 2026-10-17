.. SPDX-License-Identifier: GPL-2.0+:

.. index::
   single: dm (command)

dm command
==========

Synopsis
--------

::

    dm compat
    dm devres
    dm drivers
    dm static
    dm tree [-s][-e] [uclass name]
    dm uclass [-e] [udevice name]

Description
-----------

The *dm* command allows viewing information about driver model, including the
tree of devices and list of available uclasses.


dm compat
~~~~~~~~~

This shows the compatible strings associated with each driver. Often there
is only one, but multiple strings are shown on their own line. These strings
can be looked up in the device tree files for each board, to see which driver is
used for each node.

dm devres
~~~~~~~~~

This shows a list of a `devres` (device resource) records for a device. Some
drivers use the devres API to allocate memory, so that it can be freed
automatically (without any code needed in the driver's remove() method) when the
device is removed.

This feature is controlled by CONFIG_DEVRES so no useful output is obtained if
this option is disabled.

dm drivers
~~~~~~~~~~

This shows all the available drivers, their uclass and a list of devices that
use that driver, each on its own line. Drivers with no devices are shown with
`<none>` as the driver name.


dm mem
~~~~~~

This subcommand is really just for debugging and exploration. It can be enabled
with the `CONFIG_DM_STATS` option.

All output is in hex except that in brackets which is decimal.

The output consists of a header shows the size of the main device model
structures (struct udevice, struct driver, struct uclass and struct uc_driver)
and the count and memory used by each (number of devices, memory used by
devices, memory used by device names, number of uclasses, memory used by
uclasses).

After that is a table of information about each type of data that can be
attached to a device, showing the number that have non-null data for that type,
the total size of all that data, the amount of memory used in total, the
amount that would be used if this type uses tags instead and the amount that
would be thus saved.

The `driver_data` line shows the number of devices which have non-NULL driver
data.

The `tags` line shows the number of tags and the memory used by those.

At the bottom is an indication of the total memory usage obtained by undertaking
various changes, none of which is currently implemented in U-Boot:

With tags
    Using tags instead of all attached types

Singly linked
    Using a singly linked list

driver index
    Using a driver index instead of a pointer

uclass index
    Using a uclass index instead of a pointer

Drop device name
    Using empty device names


dm static
~~~~~~~~~

This shows devices bound by platform data, i.e. not from the device tree. There
are normally none of these, but some boards may use static devices for space
reasons.


dm tree
~~~~~~~

This shows the full tree of devices including the following fields:

uclass
    Shows the name of the uclass for the device

Index
    Shows the index number of the device, within the uclass. This shows the
    ordering within the uclass, but not the sequence number.

Probed
    Shows `+` if the device is active

Driver
    Shows the name of the driver that this device uses

Name
    Shows the device name as well as the tree structure, since child devices are
    shown attached to their parent.

If -s is given, the top-level devices (those which are children of the root
device) are shown sorted in order of uclass ID, so it is easier to find a
particular device type.

If -e is given, forward-matching against existing devices is
made and only the matched devices are shown.

If a device name is given, forward-matching against existing devices is
made and only the matched devices are shown.

dm uclass
~~~~~~~~~

This shows each uclass along with a list of devices in that uclass. The uclass
ID is shown (e.g. uclass 7) and its name.

For each device, the format is::

    n    name @ a, seq s

where `n` is the index within the uclass, `a` is the address of the device in
memory and `s` is the sequence number of the device.

If -e is given, forward-matching against existing uclasses is
made and only the matched uclasses are shown.

If no uclass name is given, all the uclasses are shown.


Examples
--------

dm compat
~~~~~~~~~

This example shows an abridged version of the sandbox output::

    => dm compat
    Driver                Compatible
    --------------------------------
    act8846_reg
    sandbox_adder         sandbox,adder
    axi_sandbox_bus       sandbox,axi
    blk_partition
    bootcount-rtc         u-boot,bootcount-rtc
    ...
    rockchip_rk805        rockchip,rk805
                          rockchip,rk808
                          rockchip,rk809
                          rockchip,rk816
                          rockchip,rk817
                          rockchip,rk818
    root_driver
    rtc-rv8803            microcrystal,rv8803
                          epson,rx8803
                          epson,rx8900
    ...
    wdt_gpio              linux,wdt-gpio
    wdt_sandbox           sandbox,wdt


dm devres
~~~~~~~~~

This example shows an abridged version of the sandbox test output (running
U-Boot with the -T flag)::

    => dm devres
    - root_driver
    - demo_shape_drv
    - demo_simple_drv
    - demo_shape_drv
    ...
    - h-test
    - devres-test
        00000000130194e0 (100 byte) devm_kmalloc_release  BIND
    - another-test
    ...
    - syscon@3
    - a-mux-controller
        0000000013025e60 (96 byte) devm_kmalloc_release  PROBE
        0000000013025f00 (24 byte) devm_kmalloc_release  PROBE
        0000000013026010 (24 byte) devm_kmalloc_release  PROBE
        0000000013026070 (24 byte) devm_kmalloc_release  PROBE
        00000000130260d0 (24 byte) devm_kmalloc_release  PROBE
    - syscon@3
    - a-mux-controller
        0000000013026150 (96 byte) devm_kmalloc_release  PROBE
        00000000130261f0 (24 byte) devm_kmalloc_release  PROBE
        0000000013026300 (24 byte) devm_kmalloc_release  PROBE
        0000000013026360 (24 byte) devm_kmalloc_release  PROBE
        00000000130263c0 (24 byte) devm_kmalloc_release  PROBE
    - emul-mux-controller
        0000000013025fa0 (32 byte) devm_kmalloc_release  PROBE
    - testfdtm0
    - testfdtm1
    ...
    - pinmux_spi0_pins
    - pinmux_uart0_pins
    - pinctrl-single-bits
        0000000013229180 (320 byte) devm_kmalloc_release  PROBE
        0000000013229300 (40 byte) devm_kmalloc_release  PROBE
        0000000013229370 (160 byte) devm_kmalloc_release  PROBE
        000000001322c190 (40 byte) devm_kmalloc_release  PROBE
        000000001322c200 (32 byte) devm_kmalloc_release  PROBE
    - pinmux_i2c0_pins
    ...
    - reg@0
    - reg@1


dm drivers
~~~~~~~~~~

This example shows an abridged version of the sandbox output::

    => dm drivers
    Driver                    uid uclass               Devices
    ----------------------------------------------------------
    act8846_reg               087 regulator            <none>
    sandbox_adder             021 axi                  adder
                                                    adder
    axi_sandbox_bus           021 axi                  axi@0
    ...
    da7219                    061 misc                 <none>
    demo_shape_drv            001 demo                 demo_shape_drv
                                                    demo_shape_drv
                                                    demo_shape_drv
    demo_simple_drv           001 demo                 demo_simple_drv
                                                    demo_simple_drv
    testfdt_drv               003 testfdt              a-test
                                                    b-test
                                                    d-test
                                                    e-test
                                                    f-test
                                                    g-test
                                                    another-test
                                                    chosen-test
    testbus_drv               005 testbus              some-bus
                                                    mmio-bus@0
                                                    mmio-bus@1
    dsa-port                  039 ethernet             lan0
                                                    lan1
    dsa_sandbox               035 dsa                  dsa-test
    eep_sandbox               121 w1_eeprom            <none>
    ...
    pfuze100_regulator        087 regulator            <none>
    phy_sandbox               077 phy                  bind-test-child1
                                                    gen_phy@0
                                                    gen_phy@1
                                                    gen_phy@2
    pinconfig                 078 pinconfig            gpios
                                                    gpio0
                                                    gpio1
                                                    gpio2
                                                    gpio3
                                                    i2c
                                                    groups
                                                    pins
                                                    i2s
                                                    spi
                                                    cs
                                                    pinmux_pwm_pins
                                                    pinmux_spi0_pins
                                                    pinmux_uart0_pins
                                                    pinmux_i2c0_pins
                                                    pinmux_lcd_pins
    pmc_sandbox               017 power-mgr            pci@1e,0
    act8846 pmic              080 pmic                 <none>
    max77686_pmic             080 pmic                 <none>
    mc34708_pmic              080 pmic                 pmic@41
    ...
    wdt_gpio                  122 watchdog             gpio-wdt
    wdt_sandbox               122 watchdog             wdt@0
    =>


dm mem
~~~~~~

This example shows the sandbox output::

    > dm mem
    Struct sizes: udevice b0, driver 80, uclass 30, uc_driver 78
    Memory: device fe:aea0, device names a16, uclass 5e:11a0

    Attached type    Count   Size    Cur   Tags   Save
    ---------------  -----  -----  -----  -----  -----
    plat                45    a8f   aea0   a7c4    6dc (1756)
    parent_plat         1a    3b8   aea0   a718    788 (1928)
    uclass_plat         3d    6b4   aea0   a7a4    6fc (1788)
    priv                8a   68f3   aea0   a8d8    5c8 (1480)
    parent_priv          8   38a0   aea0   a6d0    7d0 (2000)
    uclass_priv         4e   14a6   aea0   a7e8    6b8 (1720)
    driver_data          f      0   aea0   a6ec    7b4 (1972)
    uclass               6     20
    Attached total     191   cb54                  3164 (12644)
    tags                 0      0

    Total size: 18b94 (101268)

    With tags:       15a30 (88624)
    - singly-linked: 14260 (82528)
    - driver index:  13b6e (80750)
    - uclass index:  1347c (78972)
    Drop device name (not SRAM): a16 (2582)
    =>


dm static
~~~~~~~~~

This example shows the sandbox output::

    => dm static
    Driver                    Address
    ---------------------------------
    demo_shape_drv            0000562edab8dca0
    demo_simple_drv           0000562edab8dca0
    demo_shape_drv            0000562edab8dc90
    demo_simple_drv           0000562edab8dc80
    demo_shape_drv            0000562edab8dc80
    test_drv                  0000562edaae8840
    test_drv                  0000562edaae8848
    test_drv                  0000562edaae8850
    sandbox_gpio              0000000000000000
    mod_exp_sw                0000000000000000
    sandbox_test_proc         0000562edabb5330
    qfw_sandbox               0000000000000000
    sandbox_timer             0000000000000000
    sandbox_serial            0000562edaa8ed00
    sysreset_sandbox          0000000000000000


dm tree
-------

This example shows the abridged sandbox output::

    => dm tree
    Class     Index  Probed  Driver                Name
    -----------------------------------------------------------
    root          0  [ + ]   root_driver           root_driver
    demo          0  [   ]   demo_shape_drv        |-- demo_shape_drv
    demo          1  [   ]   demo_simple_drv       |-- demo_simple_drv
    demo          2  [   ]   demo_shape_drv        |-- demo_shape_drv
    demo          3  [   ]   demo_simple_drv       |-- demo_simple_drv
    demo          4  [   ]   demo_shape_drv        |-- demo_shape_drv
    test          0  [   ]   test_drv              |-- test_drv
    test          1  [   ]   test_drv              |-- test_drv
    test          2  [   ]   test_drv              |-- test_drv
    ..
    sysreset      0  [   ]   sysreset_sandbox      |-- sysreset_sandbox
    bootstd       0  [   ]   bootstd_drv           |-- bootstd
    bootmeth      0  [   ]   bootmeth_extlinux     |   |-- extlinux
    bootmeth      1  [   ]   bootmeth_efi          |   `-- efi
    reboot-mod    0  [   ]   reboot-mode-gpio      |-- reboot-mode0
    reboot-mod    1  [   ]   reboot-mode-rtc       |-- reboot-mode@14
    ...
    ethernet      7  [ + ]   dsa-port              |   `-- lan1
    pinctrl       0  [ + ]   sandbox_pinctrl_gpio  |-- pinctrl-gpio
    gpio          1  [ + ]   sandbox_gpio          |   |-- base-gpios
    nop           0  [ + ]   gpio_hog              |   |   |-- hog_input_active_low
    nop           1  [ + ]   gpio_hog              |   |   |-- hog_input_active_high
    nop           2  [ + ]   gpio_hog              |   |   |-- hog_output_low
    nop           3  [ + ]   gpio_hog              |   |   `-- hog_output_high
    gpio          2  [   ]   sandbox_gpio          |   |-- extra-gpios
    gpio          3  [   ]   sandbox_gpio          |   `-- pinmux-gpios
    i2c           0  [ + ]   sandbox_i2c           |-- i2c@0
    i2c_eeprom    0  [   ]   i2c_eeprom            |   |-- eeprom@2c
    i2c_eeprom    1  [   ]   i2c_eeprom_partition  |   |   `-- bootcount@10
    rtc           0  [   ]   sandbox_rtc           |   |-- rtc@43
    rtc           1  [ + ]   sandbox_rtc           |   |-- rtc@61
    i2c_emul_p    0  [ + ]   sandbox_i2c_emul_par  |   |-- emul
    i2c_emul      0  [   ]   sandbox_i2c_eeprom_e  |   |   |-- emul-eeprom
    i2c_emul      1  [   ]   sandbox_i2c_rtc_emul  |   |   |-- emul0
    i2c_emul      2  [ + ]   sandbox_i2c_rtc_emul  |   |   |-- emull
    i2c_emul      3  [   ]   sandbox_i2c_pmic_emu  |   |   |-- pmic-emul0
    i2c_emul      4  [   ]   sandbox_i2c_pmic_emu  |   |   `-- pmic-emul1
    pmic          0  [   ]   sandbox_pmic          |   |-- sandbox_pmic
    regulator     0  [   ]   sandbox_buck          |   |   |-- buck1
    regulator     1  [   ]   sandbox_buck          |   |   |-- buck2
    regulator     2  [   ]   sandbox_ldo           |   |   |-- ldo1
    regulator     3  [   ]   sandbox_ldo           |   |   |-- ldo2
    regulator     4  [   ]   sandbox_buck          |   |   `-- no_match_by_nodename
    pmic          1  [   ]   mc34708_pmic          |   `-- pmic@41
    bootcount     0  [ + ]   bootcount-rtc         |-- bootcount@0
    bootcount     1  [   ]   bootcount-i2c-eeprom  |-- bootcount
    ...
    clk           4  [   ]   fixed_clock           |-- osc
    firmware      0  [   ]   sandbox_firmware      |-- sandbox-firmware
    scmi_agent    0  [   ]   sandbox-scmi_agent    `-- scmi
    clk           5  [   ]   scmi_clk                  |-- protocol@14
    reset         2  [   ]   scmi_reset_domain         |-- protocol@16
    nop           8  [   ]   scmi_voltage_domain       `-- regulators
    regulator     5  [   ]   scmi_regulator                |-- reg@0
    regulator     6  [   ]   scmi_regulator                `-- reg@1
    => dm tree pinc
    pinctrl       0  [ + ]   sandbox_pinctrl_gpio  pinctrl-gpio
    gpio          1  [ + ]   sandbox_gpio          |-- base-gpios
    nop           0  [ + ]   gpio_hog              |   |-- hog_input_active_low
    nop           1  [ + ]   gpio_hog              |   |-- hog_input_active_high
    nop           2  [ + ]   gpio_hog              |   |-- hog_output_low
    nop           3  [ + ]   gpio_hog              |   `-- hog_output_high
    gpio          2  [   ]   sandbox_gpio          |-- extra-gpios
    gpio          3  [   ]   sandbox_gpio          `-- pinmux-gpios
    =>


dm uclass
~~~~~~~~~

This example shows the abridged sandbox output::

    => dm uclass
    uclass 0: root
    0   * root_driver @ 03015460, seq 0

    uclass 1: demo
    0     demo_shape_drv @ 03015560, seq 0
    1     demo_simple_drv @ 03015620, seq 1
    2     demo_shape_drv @ 030156e0, seq 2
    3     demo_simple_drv @ 030157a0, seq 3
    4     demo_shape_drv @ 03015860, seq 4

    uclass 2: test
    0     test_drv @ 03015980, seq 0
    1     test_drv @ 03015a60, seq 1
    2     test_drv @ 03015b40, seq 2
    ...
    uclass 20: audio-codec
    0     audio-codec @ 030168e0, seq 0

    uclass 21: axi
    0     adder @ 0301db60, seq 1
    1     adder @ 0301dc40, seq 2
    2     axi@0 @ 030217d0, seq 0

    uclass 22: blk
    0     mmc2.blk @ 0301ca00, seq 0
    1     mmc1.blk @ 0301cee0, seq 1
    2     mmc0.blk @ 0301d380, seq 2

    uclass 23: bootcount
    0   * bootcount@0 @ 0301b3f0, seq 0
    1     bootcount @ 0301b4b0, seq 1
    2     bootcount_4@0 @ 0301b570, seq 2
    3     bootcount_2@0 @ 0301b630, seq 3

    uclass 24: bootdev
    0     mmc2.bootdev @ 0301cbb0, seq 0
    1     mmc1.bootdev @ 0301d050, seq 1
    2     mmc0.bootdev @ 0301d4f0, seq 2

    ...
    uclass 78: pinconfig
    0     gpios @ 03022410, seq 0
    1     gpio0 @ 030224d0, seq 1
    2     gpio1 @ 03022590, seq 2
    3     gpio2 @ 03022650, seq 3
    4     gpio3 @ 03022710, seq 4
    5     i2c @ 030227d0, seq 5
    6     groups @ 03022890, seq 6
    7     pins @ 03022950, seq 7
    8     i2s @ 03022a10, seq 8
    9     spi @ 03022ad0, seq 9
    10    cs @ 03022b90, seq 10
    11    pinmux_pwm_pins @ 03022e10, seq 11
    12    pinmux_spi0_pins @ 03022ed0, seq 12
    13    pinmux_uart0_pins @ 03022f90, seq 13
    14  * pinmux_i2c0_pins @ 03023130, seq 14
    15  * pinmux_lcd_pins @ 030231f0, seq 15

    ...
    uclass 119: virtio
    0     sandbox_virtio1 @ 030220d0, seq 0
    1     sandbox_virtio2 @ 03022190, seq 1

    uclass 120: w1
    uclass 121: w1_eeprom
    uclass 122: watchdog
    0   * gpio-wdt @ 0301c070, seq 0
    1   * wdt@0 @ 03021710, seq 1

    => dm uclass blk
    uclass 22: blk
    0     mmc2.blk @ 0301ca00, seq 0
    1     mmc1.blk @ 0301cee0, seq 1
    2     mmc0.blk @ 0301d380, seq 2

    =>
