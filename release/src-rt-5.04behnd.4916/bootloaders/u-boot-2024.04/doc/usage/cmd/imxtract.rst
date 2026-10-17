.. SPDX-License-Identifier: GPL-2.0+:

.. index::
   single: imxtract (command)

imxtract command
================

Synopsis
--------

::

    imxtract addr part [dest]
    imxtract addr uname [dest]

Description
-----------

The imxtract command is used to extract a part of a multi-image file.

Two different file formats are supported:

* FIT images
* legacy U-Boot images

addr
    Address of the multi-image file from which a part shall be extracted

part
    Index (hexadecimal) of the part of a legacy U-Boot image to be extracted

uname
    Name of the part of a FIT image to be extracted

dest
    Destination address (defaults to 0x0)

The value of environment variable *verify* controls if the hashes and
signatures of FIT images or the check sums of legacy U-Boot images are checked.
To enable checking set *verify* to one of the values *1*, *yes*, *true*.
(Actually only the first letter is checked disregarding the case.)

To list the parts of an image the *iminfo* command can be used.

Examples
--------

With verify=no incorrect hashes, signatures, or check sums don't stop the
extraction. But correct hashes are still indicated in the output
(here: sha256, sha512).

.. code-block:: console

    => setenv verify no
    => imxtract $loadaddr kernel-1 $kernel_addr_r
    ## Copying 'kernel-1' subimage from FIT image at 40200000 ...
    sha256+ sha512+    Loading part 0 ... OK
    =>

With verify=yes incorrect hashes, signatures, or check sums stop the extraction.

.. code-block:: console

    => setenv verify yes
    => imxtract $loadaddr kernel-1 $kernel_addr_r
    ## Copying 'kernel-1' subimage from FIT image at 40200000 ...
    sha256 error!
    Bad hash value for 'hash-1' hash node in 'kernel-1' image node
    Bad Data Hash
    =>

Configuration
-------------

The imxtract command is only available if CONFIG_CMD_XIMG=y. Support for FIT
images requires CONFIG_FIT=y. Support for legacy U-Boot images requires
CONFIG_LEGACY_IMAGE_FORMAT=y.

Return value
------------

On success the return value $? of the command is 0 (true). On failure the
return value is 1 (false).
