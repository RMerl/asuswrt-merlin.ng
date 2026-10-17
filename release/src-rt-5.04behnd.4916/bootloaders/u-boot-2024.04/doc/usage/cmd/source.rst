.. SPDX-License-Identifier: GPL-2.0+
.. Copyright 2022, Heinrich Schuchardt <xypron.glpk@gmx.de>

.. index::
   single: source (command)

source command
==============

Synopsis
--------

::

    source [<addr>][:[<image>]|#[<config>]]

Description
-----------

The *source* command is used to execute a script file from memory.

Two formats for script files exist:

* legacy U-Boot image format
* Flat Image Tree (FIT)

The benefit of the FIT images is that they can be signed and verifed as
described in :doc:`../fit/signature`.

Both formats can be created with the mkimage tool.

addr
    location of the script file in memory, defaults to CONFIG_SYS_LOAD_ADDR.

image
    name of an image in a FIT file

config
    name of a configuration in a FIT file. A hash sign following white space
    starts a comment. Hence, if no *addr* is specified, the hash sign has to be
    escaped with a backslash or the argument must be quoted.

If both *image* and *config* are omitted, the default configuration is used, or
if no configuration is defined, the default image.

Examples
--------

FIT image
'''''''''

For creating a FIT image an image tree source file (\*.its) is needed. Here is
an example (source.its).

.. code-block::

    /dts-v1/;

    / {
        description = "FIT image to test the source command";
        #address-cells = <1>;

        images {
            default = "script-1";

            script-1 {
                data = "echo 1";
                type = "script";
                compression = "none";
            };

            script-2 {
                data = "echo 2";
                type = "script";
                compression = "none";
            };
        };

        configurations {
            default = "conf-2";

            conf-1 {
                script = "script-1";
            };

            conf-2 {
                script = "script-2";
            };
        };
    };

The FIT image file (boot.itb) is created with:

.. code-block:: bash

    mkimage -f source.its boot.itb

In U-Boot the script can be loaded and execute like this

.. code-block::

    => load host 0:1 $loadaddr boot.itb
    1552 bytes read in 0 ms
    => source $loadaddr#conf-1
    ## Executing script at 00000000
    1
    => source $loadaddr#conf-2
    ## Executing script at 00000000
    2
    => source $loadaddr:script-1
    ## Executing script at 00000000
    1
    => source $loadaddr:script-2
    ## Executing script at 00000000
    2
    => source $loadaddr
    ## Executing script at 00000000
    2
    => source \#conf-1
    ## Executing script at 00000000
    1
    => source '#conf-1'
    ## Executing script at 00000000
    1
    => source ':script-1'
    ## Executing script at 00000000
    1
    => source
    ## Executing script at 00000000
    2
    =>

Instead of specifying command line instructions directly in the *data* property
of the image tree source file another file can be included. Here is a minimal
example which encapsulates the file boot.txt:

.. code-block::

    /dts-v1/;
    / {
        description = "";
        images {
            script {
                data = /incbin/("./boot.txt");
                type = "script";
            };
        };
    };

Legacy U-Boot image
'''''''''''''''''''

A script file using the legacy U-Boot image file format can be created based on
a text file. Let's use this example text file (boot.txt):

.. code-block:: bash

    echo Hello from a script
    echo -------------------

The boot scripts (boot.scr) is created with:

.. code-block:: bash

    mkimage -T script -n 'Test script' -d boot.txt boot.scr

The script can be execute in U-Boot like this:

.. code-block::

    => load host 0:1 $loadaddr boot.scr
    122 bytes read in 0 ms
    => source $loadaddr
    ## Executing script at 00000000
    Hello from a script
    -------------------
    => source
    ## Executing script at 00000000
    Hello from a script
    -------------------
    =>

Configuration
-------------

The source command is only available if CONFIG_CMD_SOURCE=y.
The FIT image file format requires CONFIG_FIT=y.#
The legacy U-Boot image file format requires CONFIG_LEGACY_IMAGE_FORMAT=y.
On hardened systems support for the legacy U-Boot image format should be
disabled as these images cannot be signed and verified.

Return value
------------

If the scripts is executed successfully, the return value $? is 0 (true).
Otherwise it is 1 (false).
