.. SPDX-License-Identifier: GPL-2.0+

Kernel and multiple compressed FDT blobs
========================================

Since the FDTs are compressed, configurations must provide a compatible
string to match directly.

::

    /dts-v1/;

    / {
        description = "Image with single Linux kernel and compressed FDT blobs";
        #address-cells = <1>;

        images {
            kernel {
                description = "Vanilla Linux kernel";
                data = /incbin/("./vmlinux.bin.gz");
                type = "kernel";
                arch = "ppc";
                os = "linux";
                compression = "gzip";
                load = <00000000>;
                entry = <00000000>;
                hash-1 {
                    algo = "crc32";
                };
                hash-2 {
                    algo = "sha256";
                };
            };
            fdt@1 {
                description = "Flattened Device Tree blob 1";
                data = /incbin/("./myboard-var1.dtb");
                type = "flat_dt";
                arch = "ppc";
                compression = "gzip";
                hash-1 {
                    algo = "crc32";
                };
                hash-2 {
                    algo = "sha256";
                };
            };
            fdt@2 {
                description = "Flattened Device Tree blob 2";
                data = /incbin/("./myboard-var2.dtb");
                type = "flat_dt";
                arch = "ppc";
                compression = "lzma";
                hash-1 {
                    algo = "crc32";
                };
                hash-2 {
                    algo = "sha256";
                };
            };
        };

        configurations {
            default = "conf@1";
            conf@1 {
                description = "Boot Linux kernel with FDT blob 1";
                kernel = "kernel";
                fdt = "fdt@1";
                compatible = "myvendor,myboard-variant1";
            };
            conf@2 {
                description = "Boot Linux kernel with FDT blob 2";
                kernel = "kernel";
                fdt = "fdt@2";
                compatible = "myvendor,myboard-variant2";
            };
        };
    };
