.. SPDX-License-Identifier: GPL-2.0+

Automatic software update
=========================

Make sure the flashing addresses ('load' prop) is correct for your board!

::

    /dts-v1/;

    / {
        description = "Automatic U-Boot update";
        #address-cells = <1>;

        images {
            update-1 {
                description = "U-Boot binary";
                data = /incbin/("./u-boot.bin");
                compression = "none";
                type = "firmware";
                load = <0xFFFC0000>;
                hash-1 {
                    algo = "sha256";
                };
            };
        };
    };
