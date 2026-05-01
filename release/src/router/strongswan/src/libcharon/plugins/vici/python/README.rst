About
-----

The strongSwan VICI protocol allows external applications to monitor, configure
and control the IKE daemon charon. This Python package provides a native client
side implementation of the VICI protocol, well suited to script automated tasks
in a reliable way.


Basic Usage
-----------

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

Event Handling
--------------

Either use the convenient decorators provided by EventListener or directly call
listen() on a Session object to loop over received events and dispatch them
manually.

.. code-block:: python

    >>> import vici
    >>> s = vici.Session()
    >>> l = vici.EventListener(s)
    >>> @l.on_events(['ike-updown', 'ike-rekey'])
    ... def ike_events(name, data):
    ...   """Handle event with given 'name' and 'data'."""
    ...   print(name, data)
    ...
    >>> @l.on_events(['child-updown', 'child-rekey'])
    ... def child_events(name, data):
    ...   print(name, data)
    ...
    >>> l.listen()
