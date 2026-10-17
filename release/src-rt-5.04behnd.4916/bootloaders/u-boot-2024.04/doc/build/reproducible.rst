Reproducible builds
===================

In order to achieve reproducible builds, timestamps used in the U-Boot build
process have to be set to a fixed value.

This is done using the SOURCE_DATE_EPOCH environment variable which specifies
the number of seconds since 1970-01-01T00:00:00Z.

Example
-------

To build the sandbox with 2023-01-01T00:00:00Z as timestamp we can use:

.. code-block:: bash

    make sandbox_defconfig
    SOURCE_DATE_EPOCH=1672531200 make

This date is shown when we launch U-Boot:

.. code-block:: console

    ./u-boot -T
    U-Boot 2023.01 (Jan 01 2023 - 00:00:00 +0000)

The same effect can be obtained with buildman using the `-r` flag.
