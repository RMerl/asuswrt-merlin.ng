.. SPDX-License-Identifier: GPL-2.0+

UEFI
====

Example FIT image description file demonstrating the usage of the
bootm command to launch UEFI binaries.

Two boot configurations are available to enable booting GRUB2 on QEMU,
the former uses a FDT blob contained in the FIT image, while the later
relies on the FDT provided by the board emulator.

::

    /dts-v1/;

    / {
        description = "GRUB2 EFI and QEMU FDT blob";
        #address-cells = <1>;

        images {
            efi-grub {
                description = "GRUB EFI Firmware";
                data = /incbin/("bootarm.efi");
                type = "kernel_noload";
                arch = "arm";
                os = "efi";
                compression = "none";
                load = <0x0>;
                entry = <0x0>;
                hash-1 {
                    algo = "sha256";
                };
            };

            fdt-qemu {
                description = "QEMU DTB";
                data = /incbin/("qemu-arm.dtb");
                type = "flat_dt";
                arch = "arm";
                compression = "none";
                hash-1 {
                    algo = "sha256";
                };
            };
        };

        configurations {
            default = "config-grub-fdt";

            config-grub-fdt {
                description = "GRUB EFI Boot w/ FDT";
                kernel = "efi-grub";
                fdt = "fdt-qemu";
                signature-1 {
                    algo = "sha256,rsa2048";
                    key-name-hint = "dev";
                    sign-images = "kernel", "fdt";
                };
            };

            config-grub-nofdt {
                description = "GRUB EFI Boot w/o FDT";
                kernel = "efi-grub";
                signature-1 {
                    algo = "sha256,rsa2048";
                    key-name-hint = "dev";
                    sign-images = "kernel";
                };
            };
        };
    };
