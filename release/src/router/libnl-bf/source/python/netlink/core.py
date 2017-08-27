#
# Netlink interface based on libnl
#
# Copyright (c) 2011 Thomas Graf <tgraf@suug.ch>
#

"""netlink library based on libnl

This module provides an interface to netlink sockets

The module contains the following public classes:
 - Socket -- The netlink socket
 - Object -- Abstract object (based on struct nl_obect in libnl) used as
             base class for all object types which can be put into a Cache
 - Cache -- A collection of objects which are derived from the base
            class Object. Used for netlink protocols which maintain a list
            or tree of objects.
 - DumpParams --

The following exceptions are defined:
 - NetlinkError -- Base exception for all general purpose exceptions raised.
 - KernelError -- Raised when the kernel returns an error as response to a
                  request.

All other classes or functions in this module are considered implementation
details.
"""

import capi
import sys
import socket
import struct

__all__ = ['Message', 'Socket', 'DumpParams', 'Object', 'Cache', 'KernelError',
           'NetlinkError']
__version__ = "0.1"

# netlink protocols
NETLINK_ROUTE = 0
# NETLINK_UNUSED = 1
NETLINK_USERSOCK = 2
NETLINK_FIREWALL = 3
NETLINK_INET_DIAG = 4	
NETLINK_NFLOG = 5
NETLINK_XFRM = 6
NETLINK_SELINUX = 7
NETLINK_ISCSI = 8
NETLINK_AUDIT = 9
NETLINK_FIB_LOOKUP = 10	
NETLINK_CONNECTOR = 11
NETLINK_NETFILTER = 12
NETLINK_IP6_FW = 13
NETLINK_DNRTMSG = 14
NETLINK_KOBJECT_UEVENT = 15
NETLINK_GENERIC = 16
NETLINK_SCSITRANSPORT = 18
NETLINK_ECRYPTFS = 19

NL_DONTPAD = 0
NL_AUTO_PORT = 0
NL_AUTO_SEQ = 0

NL_DUMP_LINE = 0
NL_DUMP_DETAILS = 1
NL_DUMP_STATS = 2

NLM_F_REQUEST = 1
NLM_F_MULTI = 2
NLM_F_ACK = 4
NLM_F_ECHO = 8

NLM_F_ROOT = 0x100
NLM_F_MATCH = 0x200
NLM_F_ATOMIC = 0x400
NLM_F_DUMP = NLM_F_ROOT | NLM_F_MATCH

NLM_F_REPLACE = 0x100
NLM_F_EXCL = 0x200
NLM_F_CREATE = 0x400
NLM_F_APPEND = 0x800

class NetlinkError(Exception):
	def __init__(self, error):
        	self._error = error
                self._msg = capi.nl_geterror(error)

	def __str__(self):
        	return self._msg

class KernelError(NetlinkError):
	def __str__(self):
        	return "Kernel returned: " + self._msg

class ImmutableError(NetlinkError):
	def __init__(self, msg):
        	self._msg = msg

	def __str__(self):
        	return "Immutable attribute: " + self._msg

class Message(object):
	"""Netlink message"""

	def __init__(self, size=0):
        	if size == 0:
                        self._msg = capi.nlmsg_alloc()
                else:
                        self._msg = capi.nlmsg_alloc_size(size)

                if self._msg is None:
                        raise Exception("Message allocation returned NULL")

	def __del__(self):
        	capi.nlmsg_free(self._msg)

	def __len__(self):
        	return capi.nlmsg_len(nlmsg_hdr(self._msg))

        @property
        def protocol(self):
        	return capi.nlmsg_get_proto(self._msg)

	@protocol.setter
        def protocol(self, value):
        	capi.nlmsg_set_proto(self._msg, value)

	@property
        def maxSize(self):
        	return capi.nlmsg_get_max_size(self._msg)

	@property
        def hdr(self):
        	return capi.nlmsg_hdr(self._msg)

	@property
        def data(self):
        	return capi.nlmsg_data(self._msg)

	@property
	def attrs(self):
        	return capi.nlmsg_attrdata(self._msg)

	def send(self, socket):
        	socket.send(self)

class Socket(object):
	"""Netlink socket"""

	def __init__(self, cb=None):
		if cb is None:
                        self._sock = capi.nl_socket_alloc()
                else:
                        self._sock = capi.nl_socket_alloc_cb(cb)

                if self._sock is None:
                        raise Exception("NULL pointer returned while allocating socket")

	def __del__(self):
        	capi.nl_socket_free(self._sock)

	def __str__(self):
        	return "nlsock<" + str(self.localPort) + ">"

	@property
        def local_port(self):
        	return capi.nl_socket_get_local_port(self._sock)

	@local_port.setter
        def local_port(self, value):
        	capi.nl_socket_set_local_port(self._sock, int(value))

	@property
        def peer_port(self):
        	return capi.nl_socket_get_peer_port(self._sock)

	@peer_port.setter
	def peer_port(self, value):
        	capi.nl_socket_set_peer_port(self._sock, int(value))

	@property
        def peer_groups(self):
        	return capi.nl_socket_get_peer_groups(self._sock)

	@peer_groups.setter
        def peer_groups(self, value):
        	capi.nl_socket_set_peer_groups(self._sock, value)

	def set_bufsize(self, rx, tx):
        	capi.nl_socket_set_buffer_size(self._sock, rx, tx)

	def connect(self, proto):
        	capi.nl_connect(self._sock, proto)
                return self

	def disconnect(self):
        	capi.nl_close(self._sock)

	def sendto(self, buf):
        	ret = capi.nl_sendto(self._sock, buf, len(buf))
                if ret < 0:
                        raise Exception("Failed to send")
                else:
                        return ret

_sockets = {}

def lookup_socket(protocol):
	try:
        	sock = _sockets[protocol]
        except KeyError:
        	sock = Socket()
                sock.connect(protocol)
                _sockets[protocol] = sock

        return sock

class DumpParams(object):
	"""Dumping parameters"""

	def __init__(self, type=NL_DUMP_LINE):
        	self._dp = capi.alloc_dump_params()
                if not self._dp:
                        raise Exception("Unable to allocate struct nl_dump_params")

		self._dp.dp_type = type

	def __del__(self):
        	capi.free_dump_params(self._dp)

	@property
        def type(self):
        	return self._dp.dp_type

	@type.setter
	def type(self, value):
        	self._dp.dp_type = value

	@property
	def prefix(self):
        	return self._dp.dp_prefix

	@prefix.setter
	def prefix(self, value):
        	self._dp.dp_prefix = value

# underscore this to make sure it is deleted first upon module deletion
_defaultDumpParams = DumpParams(type=NL_DUMP_LINE)

###########################################################################
# Cacheable Object (Base Class)
class Object(object):
	"""Cacheable object (base class)"""

	def __init__(self, obj_name, name, obj=None):
		self._obj_name = obj_name
		self._name = name
		self._modules = []

		if not obj:
                        obj = capi.object_alloc_name(self._obj_name)

                self._nl_object = obj

		# Create a clone which stores the original state to notice
                # modifications
                clone_obj = capi.nl_object_clone(self._nl_object)
		self._orig = self._obj2type(clone_obj)

	def __del__(self):
        	if not self._nl_object:
                        raise ValueError()

		capi.nl_object_put(self._nl_object)

	def __str__(self):
		if hasattr(self, 'format'):
			return self.format()
		else:
			return capi.nl_object_dump_buf(self._nl_object, 4096).rstrip()

	def _new_instance(self):
        	raise NotImplementedError()

	def clone(self):
        	"""Clone object"""
        	return self._new_instance(capi.nl_object_clone(self._nl_object))

	def _module_lookup(self, path, constructor=None):
        	"""Lookup object specific module and load it

                Object implementations consisting of multiple types may
                offload some type specific code to separate modules which
                are loadable on demand, e.g. a VLAN link or a specific
                queueing discipline implementation.

                Loads the module `path` and calls the constructor if
                supplied or `module`.init()

                The constructor/init function typically assigns a new
                object covering the type specific implementation aspects
                to the new object, e.g. link.vlan = VLANLink()
                """
                try:
                        tmp = __import__(path)
                except ImportError:
                	return

                module = sys.modules[path]

		if constructor:
			ret = getattr(module, constructor)(self)
                else:
                        ret = module.init(self)

                if ret:
                        self._modules.append(ret)

	def _module_brief(self):
        	ret = ''

        	for module in self._modules:
                        if hasattr(module, 'brief'):
                                ret += module.brief()

                return ret

	def dump(self, params=None):
        	"""Dump object as human readable text"""
        	if params is None:
                        params = _defaultDumpParams

                capi.nl_object_dump(self._nl_object, params._dp)


	#####################################################################
	# mark
	@property
        def mark(self):
        	if capi.nl_object_is_marked(self.obj):
                        return True
                else:
                        return False

	@mark.setter
        def mark(self, value):
        	if value:
                        capi.nl_object_mark(self._nl_object)
                else:
                        capi.nl_object_unmark(self._nl_object)

	#####################################################################
	# shared
	@property
        def shared(self):
        	return capi.nl_object_shared(self._nl_object) != 0

	#####################################################################
	# attrs
	@property
	def attrs(self):
        	attr_list = capi.nl_object_attr_list(self._nl_object, 1024)
        	return re.split('\W+', attr_list[0])

	#####################################################################
	# refcnt
	@property
        def refcnt(self):
        	return capi.nl_object_get_refcnt(self._nl_object)

	# this method resolves multiple levels of sub types to allow
	# accessing properties of subclass/subtypes (e.g. link.vlan.id)
        def _resolve(self, attr):
        	obj = self
                l = attr.split('.')
                while len(l) > 1:
                        obj = getattr(obj, l.pop(0))
                return (obj, l.pop(0))

        def _setattr(self, attr, val):
                obj, attr = self._resolve(attr)
                return setattr(obj, attr, val)

        def _hasattr(self, attr):
                obj, attr = self._resolve(attr)
                return hasattr(obj, attr)

	def apply(self, attr, val):
                try:
                        d = attrs[self._name + "." + attr]
                except KeyError:
                        raise KeyError("Unknown " + self._name +
                                           " attribute: " + attr)

                if 'immutable' in d:
                        raise ImmutableError(attr)

                if not self._hasattr(attr):
                	raise KeyError("Invalid " + self._name +
                        		   " attribute: " + attr)
                self._setattr(attr, val)

class ObjIterator(object):
	def __init__(self, cache, obj):
                self._cache = cache
                self._nl_object = None

		if not obj:
                        self._end = 1
                else:
                        capi.nl_object_get(obj)
                        self._nl_object = obj
                        self._first = 1
                        self._end = 0

	def __del__(self):
        	if self._nl_object:
                        capi.nl_object_put(self._nl_object)

	def __iter__(self):
        	return self

	def get_next(self):
                return capi.nl_cache_get_next(self._nl_object)

	def next(self):
		if self._end:
                        raise StopIteration()

        	if self._first:
                        ret = self._nl_object
                        self._first = 0
                else:
                        ret = self.get_next()
                        if not ret:
                                self._end = 1
                                raise StopIteration()

		# return ref of previous element and acquire ref of current
		# element to have object stay around until we fetched the
		# next ptr
                capi.nl_object_put(self._nl_object)
                capi.nl_object_get(ret)
                self._nl_object = ret

		# reference used inside object
                capi.nl_object_get(ret)
                return self._cache._new_object(ret)


class ReverseObjIterator(ObjIterator):
	def get_next(self):
        	return capi.nl_cache_get_prev(self._nl_object)

###########################################################################
# Cache
class Cache(object):
	"""Collection of netlink objects"""
	def __init__(self):
        	raise NotImplementedError()

	def __del(self):
        	capi.nl_cache_free(self._nl_cache)

	def __len__(self):
        	return capi.nl_cache_nitems(self._nl_cache)

	def __iter__(self):
        	obj = capi.nl_cache_get_first(self._nl_cache)
        	return ObjIterator(self, obj)

	def __reversed__(self):
		obj = capi.nl_cache_get_last(self._nl_cache)
        	return ReverseObjIterator(self, obj)

	def __contains__(self, item):
        	obj = capi.nl_cache_search(self._nl_cache, item._nl_object)
                if obj is None:
                        return False
                else:
                        capi.nl_object_put(obj)
                        return True

	# called by sub classes to allocate type specific caches by name
	def _alloc_cache_name(self, name):
        	return capi.alloc_cache_name(name)

	# implemented by sub classes, must return new instasnce of cacheable
	# object
	def _new_object(self, obj):
        	raise NotImplementedError()

	# implemented by sub classes, must return instance of sub class
	def _new_cache(self, cache):
        	raise NotImplementedError()

	def subset(self, filter):
        	"""Return new cache containing subset of cache

		Cretes a new cache containing all objects which match the
		specified filter.
		"""
        	if not filter:
                        raise ValueError()

        	c = capi.nl_cache_subset(self._nl_cache, filter._nl_object)
        	return self._new_cache(cache=c)

	def dump(self, params=None, filter=None):
        	"""Dump (print) cache as human readable text"""
        	if not params:
                        params = _defaultDumpParams

		if filter:
                        filter = filter._nl_object

                capi.nl_cache_dump_filter(self._nl_cache, params._dp, filter)

	def clear(self):
        	"""Remove all cache entries"""
        	capi.nl_cache_clear(self._nl_cache)

	# Called by sub classes to set first cache argument
	def _set_arg1(self, arg):
        	self.arg1 = arg
                capi.nl_cache_set_arg1(self._nl_cache, arg)

	# Called by sub classes to set second cache argument
	def _set_arg2(self, arg):
        	self.arg2 = arg
                capi.nl_cache_set_arg2(self._nl_cache, arg)

	def refill(self, socket=None):
        	"""Clear cache and refill it"""
		if socket is None:
                        socket = lookup_socket(self._protocol)

        	capi.nl_cache_refill(socket._sock, self._nl_cache)
                return self

	def resync(self, socket=None, cb=None):
        	"""Synchronize cache with content in kernel"""
		if socket is None:
                        socket = lookup_socket(self._protocol)

        	capi.nl_cache_resync(socket._sock, self._nl_cache, cb)

	def provide(self):
        	"""Provide this cache to others

		Caches which have been "provided" are made available
		to other users (of the same application context) which
		"require" it. F.e. a link cache is generally provided
		to allow others to translate interface indexes to
		link names
		"""

        	capi.nl_cache_mngt_provide(self._nl_cache)

	def unprovide(self):
        	"""Unprovide this cache

		No longer make the cache available to others. If the cache
		has been handed out already, that reference will still
		be valid.
		"""
        	capi.nl_cache_mngt_unprovide(self._nl_cache)

###########################################################################
# Cache Manager (Work in Progress)
NL_AUTO_PROVIDE = 1
class CacheManager(object):
	def __init__(self, protocol, flags=None):

		self._sock = Socket()
		self._sock.connect(protocol)

		if not flags:
			flags = NL_AUTO_PROVIDE

		self._mngr = cache_mngr_alloc(self._sock._sock, protocol, flags)

	def __del__(self):
		if self._sock:
			self._sock.disconnect()

		if self._mngr:
			capi.nl_cache_mngr_free(self._mngr)
	
	def add(self, name):
		capi.cache_mngr_add(self._mngr, name, None, None)

###########################################################################
# Address Family
class AddressFamily(object):
	"""Address family representation

        af = AddressFamily('inet6')
        # raises:
        #   - ValueError if family name is not known
        #   - TypeError if invalid type is specified for family

        print af        # => 'inet6' (string representation)
        print int(af)   # => 10 (numeric representation)
        print repr(af)  # => AddressFamily('inet6')
        """
	def __init__(self, family=socket.AF_UNSPEC):
		if isinstance(family, str):
                        family = capi.nl_str2af(family)
                        if family < 0:
                                raise ValueError('Unknown family name')
                elif not isinstance(family, int):
                	raise TypeError()

        	self._family = family

	def __str__(self):
        	return capi.nl_af2str(self._family, 32)[0]

	def __len__(self):
		return len(str(self))

	def __int__(self):
        	return self._family

	def __repr__(self):
        	return 'AddressFamily(\'' + str(self) + '\')'


###########################################################################
# Abstract Address
class AbstractAddress(object):
	"""Abstract address object

        addr = AbstractAddress('127.0.0.1/8')
        print addr               # => '127.0.0.1/8'
        print addr.prefixlen     # => '8'
        print addr.family        # => 'inet'
        print len(addr)          # => '4' (32bit ipv4 address)

        a = AbstractAddress('10.0.0.1/24')
        b = AbstractAddress('10.0.0.2/24')
        print a == b             # => False


        """
	def __init__(self, addr):
        	self._nl_addr = None

        	if isinstance(addr, str):
                        addr = capi.addr_parse(addr, socket.AF_UNSPEC)
                        if addr is None:
                                raise ValueError('Invalid address format')
                elif addr:
			capi.nl_addr_get(addr)

        	self._nl_addr = addr

	def __del__(self):
                if self._nl_addr:
                        capi.nl_addr_put(self._nl_addr)

	def __cmp__(self, other):
        	if isinstance(other, str):
                        other = AbstractAddress(other)

        	diff = self.prefixlen - other.prefixlen
                if diff == 0:
                        diff = capi.nl_addr_cmp(self._nl_addr, other._nl_addr)

                return diff

	def contains(self, item):
        	diff = int(self.family) - int(item.family)
                if diff:
                        return False

                if item.prefixlen < self.prefixlen:
                        return False

                diff = capi.nl_addr_cmp_prefix(self._nl_addr, item._nl_addr)
                return diff == 0

	def __nonzero__(self):
                if self._nl_addr:
                        return not capi.nl_addr_iszero(self._nl_addr)
                else:
                        return False

	def __len__(self):
                if self._nl_addr:
                        return capi.nl_addr_get_len(self._nl_addr)
                else:
                        return 0

	def __str__(self):
                if self._nl_addr:
                        return capi.nl_addr2str(self._nl_addr, 64)[0]
                else:
                        return "none"

	@property
        def shared(self):
        	"""True if address is shared (multiple users)"""
                if self._nl_addr:
                        return capi.nl_addr_shared(self._nl_addr) != 0
                else:
                        return False

	@property
        def prefixlen(self):
        	"""Length of prefix (number of bits)"""
                if self._nl_addr:
                        return capi.nl_addr_get_prefixlen(self._nl_addr)
                else:
                        return 0

	@prefixlen.setter
	def prefixlen(self, value):
                if not self._nl_addr:
                        raise TypeError()

        	capi.nl_addr_set_prefixlen(self._nl_addr, int(value))

	@property
        def family(self):
        	"""Address family"""
                f = 0
                if self._nl_addr:
                        f = capi.nl_addr_get_family(self._nl_addr)

        	return AddressFamily(f)

	@family.setter
	def family(self, value):
                if not self._nl_addr:
                        raise TypeError()

        	if not isinstance(value, AddressFamily):
                        value = AddressFamily(value)

                capi.nl_addr_set_family(self._nl_addr, int(value))


# global dictionay for all object attributes
#
# attrs[type][keyword] : value
#
# keyword:
#   type = { int | str }
#   immutable = { True | False }
#   fmt = func (formatting function)
#
attrs = {}

def add_attr(name, **kwds):
	attrs[name] = {}
        for k in kwds:
                attrs[name][k] = kwds[k]

def nlattr(name, **kwds):
	"""netlink object attribute decorator

	decorator used to mark mutable and immutable properties
        of netlink objects. All properties marked as such are
        regarded to be accessable.

	@netlink.nlattr('my_type.my_attr', type=int)
	@property
        def my_attr(self):
        	return self._my_attr

        """

	attrs[name] = {}
        for k in kwds:
                attrs[name][k] = kwds[k]

        def wrap_fn(func):
        	return func

        return wrap_fn

