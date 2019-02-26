About
-----

The strongSwan VICI protocol allows external applications to monitor, configure
and control the IKE daemon charon. This Python package provides a native client
side implementation of the VICI protocol, well suited to script automated tasks
in a reliable way.


Example Usage
-------------

.. code-block:: python

    >>> import vici
    >>> s = vici.Session()
    >>> s.version()
    OrderedDict([('daemon', b'charon'), ('version', b'5.4.0'),
    ('sysname', b'Linux'), ('release', b'3.13.0-27-generic'), ('machine', b'x86_64')])
    >>> s.load_pool({"p1": {"addrs": "10.0.0.0/24"}})
    OrderedDict([('success', b'yes')])
    >>> s.get_pools()
    OrderedDict([('p1', OrderedDict([('base', b'10.0.0.0'), ('size', b'254'),
    ('online', b'0'), ('offline', b'0')]))])
