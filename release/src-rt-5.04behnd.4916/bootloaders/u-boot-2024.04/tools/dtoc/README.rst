.. SPDX-License-Identifier: GPL-2.0+

Devicetree-to-C generator
=========================

This is a Python program and associated utilities, which supports converting
devicetree files into C code. It generates header files containing struct
definitions, as well as C files containing the data. It does not require any
modification of the devicetree files.

Some high-level libraries are provided for working with devicetree. These may
be useful in other projects.

This package also includes some U-Boot-specific features, such as creating
`struct udevice` and `struct uclass` entries for devicetree nodes.
