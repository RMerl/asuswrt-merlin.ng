.. SPDX-License-Identifier: GPL-2.0+

Package U-Boot
==============

U-Boot uses Flat Image Tree (FIT) as a standard file format for packaging
images that it reads and boots. Documentation about FIT is available at
doc/uImage.FIT

U-Boot also provides binman for cases not covered by FIT. Examples include
initial execution (since FIT itself does not have an executable header) and
dealing with device boundaries, such as the read-only/read-write separation in
SPI flash.


.. toctree::
   :maxdepth: 2

   binman
