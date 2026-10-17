.. SPDX-License-Identifier: GPL-2.0+:

.. index::
   single: gpio (command)

gpio command
============

Synopsis
--------

::

    gpio <input|set|clear|toggle> <pin>
    gpio read <name> <pin>
    gpio status [-a] [<bank>|<pin>]

The gpio command is used to access General Purpose Inputs/Outputs.

gpio input
----------

Switch the GPIO *pin* to input mode.

gpio set
--------

Switch the GPIO *pin* to output mode and set the signal to 1.

gpio clear
----------

Switch the GPIO *pin* to output mode and set the signal to 0.

gpio toggle
-----------

Switch the GPIO *pin* to output mode and reverse the signal state.

gpio read
---------

Read the signal state of the GPIO *pin* and save it in environment variable
*name*.

gpio status
-----------

Display the status of one or multiple GPIOs. By default only claimed GPIOs
are displayed.
gpio status command output fields are::

    <name>: <function>: <value> [x] <label>

*function* can take the following values:

output
    pin configured in gpio output, *value* indicates the pin's level

input
    pin configured in gpio input, *value* indicates the pin's level

func
    pin configured in alternate function, followed by *label*
    which shows pinmuxing label.

unused
    pin not configured

*[x]* or *[ ]* indicate respectively if the gpio is used or not.

*label* shows the gpio label.

Parameters
----------

-a
    Display GPIOs irrespective of being claimed.

bank
    Name of a bank of GPIOs to be displayed.

pin
    Name of a single GPIO to be displayed or manipulated.

Examples
--------

Switch the status of a GPIO::

    => gpio set a5
    gpio: pin a5 (gpio 133) value is 1
    => gpio clear a5
    gpio: pin a5 (gpio 133) value is 0
    => gpio toggle a5
    gpio: pin a5 (gpio 133) value is 1
    => gpio read myvar a5
    gpio: pin a5 (gpio 133) value is 1
    => echo $myvar
    1
    => gpio toggle a5
    gpio: pin a5 (gpio 133) value is 0
    => gpio read myvar a5
    gpio: pin a5 (gpio 133) value is 0
    => echo $myvar
    0

Show the GPIO status::

    => gpio status
    Bank GPIOA:
    GPIOA1: func rgmii-0
    GPIOA2: func rgmii-0
    GPIOA7: func rgmii-0
    GPIOA10: output: 0 [x] hdmi-transmitter@39.reset-gpios
    GPIOA13: output: 1 [x] red.gpios

    Bank GPIOB:
    GPIOB0: func rgmii-0
    GPIOB1: func rgmii-0
    GPIOB2: func uart4-0
    GPIOB7: input: 0 [x] mmc@58005000.cd-gpios
    GPIOB11: func rgmii-0

Configuration
-------------

The *gpio* command is only available if CONFIG_CMD_GPIO=y.
The *gpio read* command is only available if CONFIG_CMD_GPIO_READ=y.

Return value
------------

If the command succeds the return value $? is set to 0. If an error occurs, the
return value $? is set to 1.
