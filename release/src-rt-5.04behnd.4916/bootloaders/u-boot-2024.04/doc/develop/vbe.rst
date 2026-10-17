.. SPDX-License-Identifier: GPL-2.0+

Verified Boot for Embedded (VBE)
================================

Introduction
------------

VBE provides a standard boot mechanism for embedded systems. If defines
how firmware and Operating Systems are located, updated and verified.

Within U-Boot, one or more VBE bootmeths implement the boot logic. For example,
the vbe-simple bootmeth handles finding the firmware (e.g. in MMC) and starting
it. Typically the bootmeth is started up in VPL and controls which SPL and
U-Boot binaries are loaded.

A 'vbe' command provides access to various aspects of VBE's operation, including
listing methods and getting the status for a method.

For a detailed overview of VBE, see vbe-intro_. A fuller description of
bootflows is at vbe-bootflows_ and the firmware-update mechanism is described at
vbe-fwupdate_. VBE OS requests are described at  vbe-osrequests_.

.. _vbe-intro: https://docs.google.com/document/d/e/2PACX-1vQjXLPWMIyVktaTMf8edHZYDrEvMYD_iNzIj1FgPmKF37fpglAC47Tt5cvPBC5fvTdoK-GA5Zv1wifo/pub
.. _vbe-bootflows: https://docs.google.com/document/d/e/2PACX-1vR0OzhuyRJQ8kdeOibS3xB1rVFy3J4M_QKTM5-3vPIBNcdvR0W8EXu9ymG-yWfqthzWoM4JUNhqwydN/pub
.. _vbe-fwupdate: https://docs.google.com/document/d/e/2PACX-1vTnlIL17vVbl6TVoTHWYMED0bme7oHHNk-g5VGxblbPiKIdGDALE1HKId8Go5f0g1eziLsv4h9bocbk/pub
.. _vbe-osrequests: https://docs.google.com/document/d/e/2PACX-1vTHhxX7WSZe68i9rAkW-DHdx6koU-jxYHhamLhZn9GQ9QT2_epSBosMV1_r7yPHOXZccx71rF_t0PXL/pub
