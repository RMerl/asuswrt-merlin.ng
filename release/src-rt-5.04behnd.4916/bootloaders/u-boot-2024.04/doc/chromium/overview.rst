.. SPDX-License-Identifier: GPL-2.0+
.. Copyright 2020 Google LLC

Chromium OS Support in U-Boot
=============================

Introduction
------------

This describes how to use U-Boot with Chromium OS. Several options are
available:

   - Running U-Boot from the 'altfw' feature, which is available on selected
     Chromebooks from 2019 onwards (initially Grunt). Press '1' from the
     developer-mode screen to get into U-Boot. See here for details:
     https://chromium.googlesource.com/chromiumos/docs/+/HEAD/developer_mode.md

   - Running U-Boot from the disk partition. This involves signing U-Boot and
     placing it on the disk, for booting as a 'kernel'. See
     :doc:`chainload` for information on this. This is the only
     option on non-U-Boot Chromebooks from 2013 to 2018 and is somewhat
     more involved.

   - Running U-Boot with Chromium OS verified boot. This allows U-Boot to be
     used instead of either or both of depthcharge (a bootloader which forked
     from U-Boot in 2013) and coreboot. See :doc:`run_vboot` for more
     information on this.

   - Running U-Boot from coreboot. This allows U-Boot to run on more devices
     since many of them only support coreboot as the bootloader and have
     no bare-metal support in U-Boot. For this, use the 'coreboot' target.

   - Running U-Boot and booting into a Chrome OS image, but without verified
     boot. This can be useful for testing.


Talks and documents
-------------------

Here is some material relevant to Chromium OS verified boot with U-Boot:

   - "U-Boot with Chrome OS and firmware packaging"

     - Author: Simon Glass
     - Presented at Open Source Firmware Conference 2018, Erlangen
     - Describes the work in progress as at the end of 2018
     - Slides at `OSFC <https://2018.osfc.io/uploads/talk/paper/26/U-Boot_with_Chrome_OS_and_firmware_packaging.pdf>`_
     - `Youtube video 'OSFC - U-Boot with Chrome OS and firmware packaging' <https://www.youtube.com/watch?v=1jknxUvmwpo>`_

   - "Verified Boot in Chrome OS and how to make it work for you"

     - Author: Simon Glass
     - Presented at ELCE 2013, Edinburgh
     - Describes the original 2013 implementation as shipped on snow (first
       `ARM Chromebook was a Samsung Chromebook <https://www.cnet.com/products/samsung-series-3-chromebook-xe303c12-11-6-exynos-5250-2-gb-ram-16-gb-ssd-bilingual-english-french/>`_
       with Samsung Exynos5250 `review <https://www.cnet.com/reviews/samsung-chromebook-series-3-review/>`_),
       spring (`HP Chromebook 11 <https://www.cnet.com/products/hp-chromebook-11-g2-11-6-exynos-5250-4-gb-ram-16-gb-emmc/>`_)
       and pit/pi (`Samsung Chromebook 2 <https://www.cnet.com/products/samsung-chromebook-2-xe503c12-11-6-exynos-5-octa-4-gb-ram-16-gb-ssd/>`_
       with Exynos 5 Octa 5420 in 2014).
     - Slides at `Google research <https://research.google/pubs/pub42038/>`_
     - `Youtube video 'Verified Boot on Chrome OS and How to do it yourself' <https://www.youtube.com/watch?v=kdpZC9jFzZA>`_

   - "Chrome University 2018: Chrome OS Firmware and Verified Boot 201"

     - Author: Duncan Laurie
     - Describes Chrome OS firmware as of 2018 and includes a wide range of
       topics. This has no U-Boot information, but does cover coreboot and also
       talks about the Chrome OS EC and Security chip. This is probably the
       best introduction talk.
     - `Youtube video 'Chrome University 2018: Chrome OS Firmware and Verified Boot 201' <https://www.youtube.com/watch?v=WY2sWpuda2g>`_

   - `Chromium OS U-Boot <https://www.chromium.org/developers/u-boot>`_

   - `Firmware porting Guide <https://www.chromium.org/chromium-os/firmware-porting-guide>`_
