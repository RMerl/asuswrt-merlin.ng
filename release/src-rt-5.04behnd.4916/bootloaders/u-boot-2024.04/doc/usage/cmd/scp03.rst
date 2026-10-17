.. SPDX-License-Identifier: GPL-2.0+

.. index::
   single: scp03 (command)

scp03 command
=============

Synopsis
--------

::

    scp03 enable
    scp03 provision

Description
-----------

The *scp03* command calls into a Trusted Application executing in a
Trusted Execution Environment to enable (if present) the Secure
Channel Protocol 03 stablished between the processor and the secure
element.

This protocol encrypts all the communication between the processor and
the secure element using a set of pre-defined keys. These keys can be
rotated (provisioned) using the *provision* request.

See also
--------

For some information on the internals implemented in the TEE, please
check the GlobalPlatform documentation on `Secure Channel Protocol '03'`_

.. _Secure Channel Protocol '03':
   https://globalplatform.org/wp-content/uploads/2014/07/GPC_2.3_D_SCP03_v1.1.2_PublicRelease.pdf
