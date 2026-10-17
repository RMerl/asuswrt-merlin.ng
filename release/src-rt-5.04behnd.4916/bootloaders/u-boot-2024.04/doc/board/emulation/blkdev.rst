.. SPDX-License-Identifier: GPL-2.0+

Emulation of block devices
--------------------------

QEMU can emulate common block devices by adding the following parameters to
the qemu-system-<arch> command line:

* MMC

  .. code-block:: bash

      -device sdhci-pci,sd-spec-version=3 \
      -drive if=none,file=disk.img,format=raw,id=MMC1 \
      -device sd-card,drive=MMC1

* NVMe

  .. code-block:: bash

      -drive if=none,file=disk.img,format=raw,id=NVME1 \
      -device nvme,drive=NVME1,serial=nvme-1

* SATA

  .. code-block:: bash

      -device ahci,id=ahci0 \
      -drive if=none,file=disk.img,format=raw,id=SATA1 \
      -device ide-hd,bus=ahci0.0,drive=SATA1

* USB

  .. code-block:: bash

      -device qemu-xhci \
      -drive if=none,file=disk.img,format=raw,id=USB1 \
      -device usb-storage,drive=USB1

* Virtio

  .. code-block:: bash

      -drive if=none,file=disk.img,format=raw,id=VIRTIO1 \
      -device virtio-blk,drive=VIRTIO1

  .. note::
     As of v2023.07 U-Boot does not have a driver for virtio-scsi-pci.
