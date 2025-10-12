=================
Network Addresses
=================

The **Address** module provides access to the network address configuration
of the kernel. It provides an interface to fetch all configured addresses,
add new addresses and to delete existing addresses.

Fetching the list of network addresses is achieved by creating a new
address cache::

	import netlink.route.address as Address

        addr_cache = Address.AddressCache()
        addr_cache.refill()

        for addr in addr_cache:
                print addr

.. py:module:: netlink.route.addr


AddressCache
------------

.. py:class:: AddressCache

   Represents a cache containing all or a subset of network addresses.

   .. py:method:: lookup(ifindex, local)

      Lookup the address which matches ifindex and local address

      :raises: KeyError if address is not found.

Address
-------

.. py:class:: Address

   Representation of a configured network address.

   .. py:attribute:: ifindex

      Interface index

      :rtype: int
