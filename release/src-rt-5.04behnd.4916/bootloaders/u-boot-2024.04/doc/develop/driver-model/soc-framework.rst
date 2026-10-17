.. SPDX-License-Identifier: GPL-2.0+
.. (C) Copyright 2020
.. Texas Instruments Incorporated - https://www.ti.com/

SOC ID Framework
================

Introduction
------------

The driver-model SOC ID framework is able to provide identification
information about a specific SoC in use at runtime, and also provide matching
from a set of identification information from an array. This can be useful for
enabling small quirks in drivers that exist between SoC variants that are
impractical to implement using device tree flags. It is based on UCLASS_SOC.

UCLASS_SOC:
  - drivers/soc/soc-uclass.c
  - include/soc.h

Configuration:
  - CONFIG_SOC_DEVICE is selected by drivers as needed.

Implementing a UCLASS_SOC provider
----------------------------------

The purpose of this framework is to allow UCLASS_SOC provider drivers to supply
identification information about the SoC in use at runtime. The framework
allows drivers to define soc_ops that return identification strings.  All
soc_ops need not be defined and can be left as NULL, in which case the
framework will return -ENOSYS and not consider the value when doing an
soc_device_match.

It is left to the driver implementor to decide how the information returned is
determined, but in general the same SOC should always return the same set of
identifying information. Information returned must be in the form of a NULL
terminated string.

See include/soc.h for documentation of the available soc_ops and the intended
meaning of the values that can be returned. See drivers/soc/soc_sandbox.c for
an example UCLASS_SOC provider driver.

Using a UCLASS_SOC driver
-------------------------

The framework provides the ability to retrieve and use the identification
strings directly. It also has the ability to return a match from a list of
different sets of SoC data using soc_device_match.

An array of 'struct soc_attr' can be defined, each containing ID information
for a specific SoC, and when passed to soc_device_match, the identifier values
for each entry in the list will be compared against the values provided by the
UCLASS_SOC driver that is in use. The first entry in the list that matches all
non-null values will be returned by soc_device_match.

An example of various uses of the framework can be found at test/dm/soc.c.

Describing the device using device tree
---------------------------------------

.. code-block:: none

   chipid: chipid {
        compatible = "sandbox,soc";
   };

All that is required in a DT node is a compatible for a corresponding
UCLASS_SOC driver.
