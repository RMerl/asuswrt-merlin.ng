*******************
Netlink Core Module
*******************

.. py:module:: netlink.core

Examples::

	import netlink.core as netlink

===============
Object
===============

.. py:class:: Object
   
   Base class for all classes representing a cacheable object

   Example::
	obj = netlink.Object("route/link", "link")

   .. py:method:: clone

      Clone the object and return a duplicate (used for COW)

   .. py:method:: dump([params=None])

      Call the libnl internal dump mechanism to dump the object
      according to the parameters specified.

   .. py:method:: apply(attr, val)

      Applies a attribute=value pair and modifies the object accordingly.
      Example::
	obj.apply("mtu", 1200)      # Sets attribute mtu to 1200 (link obj)

      :raises: KeyError if attribute is unknown
      :raises: ImmutableError if attribute is not mutable

   .. py:attribute:: mark

      True if the object is marked, otherwise False.

   .. py:attribute:: shared

      True if the object is used by multiple parties, otherwise False.

   .. py:attribute:: refcnt

      Number of users sharing a reference to the object
      :rtype: int

   .. py:attribute:: attrs

      List of attributes

      :rtype: list of strings

===============
Cache
===============

.. py:class:: Cache
   
   Base class for all cache implementations.

   A cache is a collection of cacheable objects which is typically used
   by netlink protocols which handle any kind of object, e.g. network
   links, network addresses, neighbours, ...

   .. py:method:: subset(filter)

      Returns a new cache containing the subset which matches the
      provided filter.

      :raises: ValueError if no filter is specified
      :rtype: :py:class:`Cache`

   .. py:method:: dump([params=None, filter=None])

      Calls the libnl internal dump mechanism to dump the cache according
      to the parameters and filter specified.

   .. py:method:: clear()

      Remove and possibly destroy all objects in the cache

   .. py:method:: refill([socket=None]) -> :py:class:`Cache`

      Clears and refills the cache with the content which is provided by
      the kernel, e.g. for a link cache this would mean refilling the
      cache with all configured network links.

   .. py:method:: provide()
      
      Caches which have been "provided" are made available to other users
      (of the same application context) which "require" it. F.e. a link
      cache is generally provided to allow others to translate interface
      indexes to link names


   .. py:method:: unprovide()
      
      No longer make the cache available to others. If the cache has been
      handed out already, that reference will still be valid.

===============
AbstractAddress
===============

.. py:class:: AbstractAddress
   
   Abstract representation of an address. This class is not to be mistaken
   with :py:class:`route.Address` which represents a configured network
   address. This class represents the actual address in a family independent
   way::

	addr = netlink.AbstractAddress('127.0.0.1/8')
	print addr               # => '127.0.0.1/8'
	print addr.prefixlen     # => '8'
	print addr.family        # => 'inet'
	print len(addr)          # => '4' (32bit ipv4 address)

	a = netlink.AbstractAddress('10.0.0.1/24')
	b = netlink.AbstractAddress('10.0.0.2/24')
	print a == b             # => False

   .. py:attribute:: prefixlen

      Length of prefix in number of bits.

      :rtype: int

   .. py:attribute:: family

      The family type of the address. Setting the address family can be
      done with a string or a :py:class:`AddressFamily` object.

      :rtype: :py:class:`AddressFamily`

   .. py:attribute:: shared

      True if address is in use by multiple callers, otherwise False

      :rtype: bool

===============
AddressFamily
===============

.. py:class:: AddressFamily
   
   Address family representation::
   
	af = netlink.AddressFamily('inet6')
	# raises:
	#   - ValueError if family name is not known
	#   - TypeError if invalid type is specified for family
   
	print af        # => 'inet6' (string representation)
	print int(af)   # => 10 (numeric representation)
	print repr(af)  # => AddressFamily('inet6')

===============
Exceptions
===============

.. py:exception:: NetlinkError

   Generic exception raised by netlink modules.

.. py:exception:: KernelError

   Raised if an error occured while communicating with the kernel. Contains
   the error code returning which is automatically included in the error
   message.

.. py:exception:: ImmutableError

   Raised if an attribute is modified which is marked immutable.

===============
Socket
===============

.. py:class:: Socket

   Netlink socket.

   Note: It is not required to manually create and connect netlink sockets
   when using caches. The caches will automatically lookup or create a
   socket as needed.

   .. py:attribute:: local_port

      Local port (address) of netlink socket

   .. py:attribute:: peer_port

      Peer port (remote address) of netlink socket. If set, all messages
      will be sent to that peer.

   .. py:method:: connect(proto)

      Connect the netlink socket using the specified netlink protocol::
	sock.connect(netlink.NETLINK_ROUTE)

   .. py:method:: disconnect()

      Disconnect the socket

   .. py:method:: set_bufsize(rx, tx)

      Sets the size of the socket buffer

