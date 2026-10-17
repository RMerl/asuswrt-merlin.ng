.. SPDX-License-Identifier: GPL-2.0+

Arm FF-A Support
================

Summary
-------

FF-A stands for Firmware Framework for Arm A-profile processors.

FF-A specifies interfaces that enable a pair of software execution environments aka partitions to
communicate with each other. A partition could be a VM in the Normal or Secure world, an
application in S-EL0, or a Trusted OS in S-EL1.

The U-Boot FF-A support (the bus) implements the interfaces to communicate
with partitions in the Secure world aka Secure partitions (SPs).

The FF-A support specifically focuses on communicating with SPs that
isolate portions of EFI runtime services that must run in a protected
environment which is inaccessible by the Host OS or Hypervisor.
Examples of such services are set/get variables.

The FF-A support uses the SMC ABIs defined by the FF-A specification to:

- Discover the presence of SPs of interest
- Access an SP's service through communication protocols
  e.g. EFI MM communication protocol

At this stage of development only EFI boot-time services are supported.
Runtime support will be added in future developments.

The U-Boot FF-A support provides the following parts:

- A Uclass driver providing generic FF-A methods.
- An Arm FF-A device driver providing Arm-specific methods and reusing the Uclass methods.
- A sandbox emulator for Arm FF-A, emulates the FF-A side of the Secure World and provides
  FF-A ABIs inspection methods.
- An FF-A sandbox device driver for FF-A communication with the emulated Secure World.
  The driver leverages the FF-A Uclass to establish FF-A communication.
- Sandbox FF-A test cases.

FF-A and SMC specifications
---------------------------

The current implementation of the U-Boot FF-A support relies on
`FF-A v1.0 specification`_ and uses SMC32 calling convention which
means using the first 32-bit data of the Xn registers.

At this stage we only need the FF-A v1.0 features.

The FF-A support has been tested with OP-TEE which supports SMC32 calling
convention.

Hypervisors are supported if they are configured to trap SMC calls.

The FF-A support uses 64-bit registers as per `SMC Calling Convention v1.2 specification`_.

Supported hardware
------------------

Aarch64 plaforms

Configuration
-------------

CONFIG_ARM_FFA_TRANSPORT
    Enables the FF-A support. Turn this on if you want to use FF-A
    communication.
    When using an Arm 64-bit platform, the Arm FF-A driver will be used.
    When using sandbox, the sandbox FF-A emulator and FF-A sandbox driver will be used.

FF-A ABIs under the hood
------------------------

Invoking an FF-A ABI involves providing to the secure world/hypervisor the
expected arguments from the ABI.

On an Arm 64-bit platform, the ABI arguments are stored in x0 to x7 registers.
Then, an SMC instruction is executed.

At the secure side level or hypervisor the ABI is handled at a higher exception
level and the arguments are read and processed.

The response is put back through x0 to x7 registers and control is given back
to the U-Boot Arm FF-A driver (non-secure world).

The driver reads the response and processes it accordingly.

This methodology applies to all the FF-A ABIs.

FF-A bus discovery on Arm 64-bit platforms
------------------------------------------

When CONFIG_ARM_FFA_TRANSPORT is enabled, the FF-A bus is considered as
an architecture feature and discovered using ARM_SMCCC_FEATURES mechanism.
This discovery mechanism is performed by the PSCI driver.

The PSCI driver comes with a PSCI device tree node which is the root node for all
architecture features including FF-A bus.

::

   => dm tree

    Class     Index  Probed  Driver                Name
   -----------------------------------------------------------
    firmware      0  [ + ]   psci                      |-- psci
    ffa                   0  [   ]   arm_ffa               |   `-- arm_ffa

The PSCI driver is bound to the PSCI device and when probed it tries to discover
the architecture features by calling a callback the features drivers provide.

In case of FF-A, the callback is arm_ffa_is_supported() which tries to discover the
FF-A framework by querying the FF-A framework version from secure world using
FFA_VERSION ABI. When discovery is successful, the ARM_SMCCC_FEATURES
mechanism creates a U-Boot device for the FF-A bus and binds the Arm FF-A driver
with the device using device_bind_driver().

At this stage the FF-A bus is registered with the DM and can be interacted with using
the DM APIs.

Clients are able to probe then use the FF-A bus by calling uclass_first_device().
Please refer to the armffa command implementation as an example of how to probe
and interact with the FF-A bus.

When calling uclass_first_device(), the FF-A driver is probed and ends up calling
ffa_do_probe() provided by the Uclass which does the following:

    - saving the FF-A framework version in uc_priv
    - querying from secure world the u-boot endpoint ID
    - querying from secure world the supported features of FFA_RXTX_MAP
    - mapping the RX/TX buffers
    - querying from secure world all the partitions information

When one of the above actions fails, probing fails and the driver stays not active
and can be probed again if needed.

Requirements for clients
------------------------

When using the FF-A bus with EFI, clients must query the SPs they are looking for
during EFI boot-time mode using the service UUID.

The RX/TX buffers are only available at EFI boot-time. Querying partitions is
done at boot time and data is cached for future use.

RX/TX buffers should be unmapped before EFI runtime mode starts.
The driver provides a bus operation for that called ffa_rxtx_unmap().

The user should call ffa_rxtx_unmap() to unmap the RX/TX buffers when required
(e.g: at efi_exit_boot_services()).

The Linux kernel allocates its own RX/TX buffers. To be able to register these kernel buffers
with secure world, the U-Boot's RX/TX buffers should be unmapped before EFI runtime starts.

When invoking FF-A direct messaging, clients should specify which ABI protocol
they want to use (32-bit vs 64-bit). Selecting the protocol means using
the 32-bit or 64-bit version of FFA_MSG_SEND_DIRECT_{REQ, RESP}.
The calling convention between U-Boot and the secure world stays the same: SMC32.

Requirements for user drivers
-----------------------------

Users who want to implement their custom FF-A device driver while reusing the FF-A Uclass can do so
by implementing their own invoke_ffa_fn() in the user driver.

The bus driver layer
--------------------

FF-A support comes on top of the SMCCC layer and is implemented by the FF-A Uclass drivers/firmware/arm-ffa/arm-ffa-uclass.c

The following features are provided:

- Support for the 32-bit version of the following ABIs:

    - FFA_VERSION
    - FFA_ID_GET
    - FFA_FEATURES
    - FFA_PARTITION_INFO_GET
    - FFA_RXTX_UNMAP
    - FFA_RX_RELEASE
    - FFA_RUN
    - FFA_ERROR
    - FFA_SUCCESS
    - FFA_INTERRUPT
    - FFA_MSG_SEND_DIRECT_REQ
    - FFA_MSG_SEND_DIRECT_RESP

- Support for the 64-bit version of the following ABIs:

    - FFA_RXTX_MAP
    - FFA_MSG_SEND_DIRECT_REQ
    - FFA_MSG_SEND_DIRECT_RESP

- Processing the received data from the secure world/hypervisor and caching it

- Hiding from upper layers the FF-A protocol and registers details. Upper
  layers focus on exchanged data, FF-A support takes care of how to transport
  that to the secure world/hypervisor using FF-A

- FF-A support provides driver operations to be used by upper layers:

    - ffa_partition_info_get
    - ffa_sync_send_receive
    - ffa_rxtx_unmap

- FF-A bus discovery makes sure FF-A framework is responsive and compatible
  with the driver

- FF-A bus can be compiled and used without EFI

Relationship between the sandbox emulator and the FF-A device
-------------------------------------------------------------

::

   => dm tree

    Class     Index  Probed  Driver                Name
   -----------------------------------------------------------
   ffa_emul      0  [ + ]   sandbox_ffa_emul      `-- arm-ffa-emul
    ffa                  0  [    ]   sandbox_arm_ffa               `-- sandbox-arm-ffa

The armffa command
------------------

armffa is a command showcasing how to use the FF-A bus and how to invoke the driver operations.

Please refer the command documentation at :doc:`../usage/cmd/armffa`

Example of boot logs with FF-A enabled
--------------------------------------

For example, when using FF-A with Corstone-1000, debug logs enabled, the output is as follows:

::

   U-Boot 2023.01 (May 10 2023 - 11:08:07 +0000) corstone1000 aarch64

   DRAM:  2 GiB
   Arm FF-A framework discovery
   FF-A driver 1.0
   FF-A framework 1.0
   FF-A versions are compatible
   ...
   FF-A driver 1.0
   FF-A framework 1.0
   FF-A versions are compatible
   EFI: MM partition ID 0x8003
   ...
   EFI stub: Booting Linux Kernel...
   ...
   Linux version 6.1.9-yocto-standard (oe-user@oe-host) (aarch64-poky-linux-musl-gcc (GCC) 12.2.0, GNU ld (GNU Binutils) 2.40.202301193
   Machine model: ARM Corstone1000 FPGA MPS3 board

Contributors
------------
   * Abdellatif El Khlifi <abdellatif.elkhlifi@arm.com>

.. _`FF-A v1.0 specification`: https://documentation-service.arm.com/static/5fb7e8a6ca04df4095c1d65e
.. _`SMC Calling Convention v1.2 specification`: https://documentation-service.arm.com/static/5f8edaeff86e16515cdbe4c6
