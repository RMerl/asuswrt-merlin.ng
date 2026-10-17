.. SPDX-License-Identifier: GPL-2.0+:

.. index::
   single: pinmux (command)

pinmux command
==============

Synopsis
--------

::

    pinmux list
    pinmux dev [pincontroller-name]
    pinmux status [-a | pin-name]

Description
-----------

The pinmux command is used to show the pin-controller muxing.

The 'pinmux list' command diplays the available pin-controller.

The 'pinmux dev' command selects the pin-controller for next commands.

    pincontroller-name
        name of the pin-controller to select

The 'pinmux status' command displays the pin muxing information.

    \-a
        display pin muxing of all pin-controllers.
    pin-name
        name of the pin to display

Example
-------

::

    => pinmux list
    | Device                        | Driver                        | Parent
    | pinctrl-gpio                  | sandbox_pinctrl_gpio          | root_driver
    | pinctrl                       | sandbox_pinctrl               | root_driver
    =>
    => pinmux dev pinctrl
    dev: pinctrl
    =>
    => pinmux status
    P0        : UART TX.
    P1        : UART RX.
    P2        : I2S SCK.
    P3        : I2S SD.
    P4        : I2S WS.
    P5        : GPIO0 bias-pull-up input-disable.
    P6        : GPIO1 drive-open-drain.
    P7        : GPIO2 bias-pull-down input-enable.
    P8        : GPIO3 bias-disable.
    =>
    => pinmux status P0
    P0        : UART TX.
    =>
    => pinmux status -a
    --------------------------
    pinctrl-gpio:
    a0        : gpio input .
    a1        : gpio input .
    a2        : gpio input .
    a3        : gpio input .
    a4        : gpio input .
    a5        : gpio output .
    a6        : gpio output .
    a7        : gpio input .
    a8        : gpio input .
    a9        : gpio input .
    --------------------------
    pinctrl:
    P0        : UART TX.
    P1        : UART RX.
    P2        : I2S SCK.
    P3        : I2S SD.
    P4        : I2S WS.
    P5        : GPIO0 bias-pull-up input-disable.
    P6        : GPIO1 drive-open-drain.
    P7        : GPIO2 bias-pull-down input-enable.
    P8        : GPIO3 bias-disable.

Configuration
-------------

The pinmux command is only available if CONFIG_CMD_PINMUX=y.

Return value
------------

The return value $? is set to 0 (true) if the command succeded and to 1 (false)
otherwise.
