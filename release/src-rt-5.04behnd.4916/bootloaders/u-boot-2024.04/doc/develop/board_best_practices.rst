.. SPDX-License-Identifier: GPL-2.0+:

Best Practices for Board Ports
==============================

In addition to the regular best practices such as using :doc:`checkpatch` and
following the :doc:`docstyle` and the :doc:`codingstyle` there are some things
which are specific to creating a new board port.

* Implement :doc:`bootstd` to ensure that most operating systems will be
  supported by the platform.

* The platform defconfig file must be generated via `make savedefconfig`.

* The Kconfig and Kbuild infrastructure supports using "fragments" that can be
  used to apply changes on top of a defconfig file. These can be useful for
  many things such as:

  * Supporting different firmware locations (e.g. eMMC, SD, QSPI).

  * Multiple board variants when runtime detection is not desired.

  * Supporting different build types such as production and development.

  Kconfig fragments should reside in the board directory itself rather than in
  the top-level `configs/` directory.
