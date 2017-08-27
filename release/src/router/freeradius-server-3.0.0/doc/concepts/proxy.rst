FreeRADIUS as a proxy RADIUS server.
====================================


Introduction
------------

It is possible to use FreeRADIUS as a proxy RADIUS server. This
means that it can consult a remote RADIUS server to validate a user.
This is handy for roaming setups, or for renting ports to someone else.

Files
-----

If a user logs in with a defined realm syntax, the "realm" portion is
matched against the configuration to determine how the request should
be handled.  Common realm formats are:

::

  username@realm
  realm/username
  username%realm
  realm\username

The realm parsing syntax ( and search order ) is user definable via the
realm module config in the ``/etc/raddb/radiusd.conf`` configuration file.

You can define multiple instances of the realm module to support multiple
realm syntax's at the same time.  Be sure to pay close attention to the
search order that you define, as you may inadvertently get unexpected
behaviour ( by having a user use ``realm1/username@realm2`` for instance ).
If you need to proxy to IPASS, it should go first, because usernames will
be in the form ``IPASS/username@realm`` and you want to proxy these users to
IPASS, not to the realm behind the ``@``.

The realms are configured in the file ``/etc/raddb/proxy.conf``, which is
included by ``radiusd.conf``. The formats and sample configurations are
included as comments.

The realm ``DEFAULT`` (without the quotes) matches all realms.
The realm ``NULL`` matches any requests WITHOUT a realm.

If you set the remote server to ``LOCAL``, the request will be handled
locally as usual, without sending it to a remote radius server.

There are several options you can add in both files:

- nostrip:
  By default the realm is stripped from the username before sending it
  on to the remote radius server. By specifying the "nostrip" option
  the @realm suffix will not be stripped.
- hints
  By default the original username is sent to the remote radius
  server. By specifying the "hints" option the username will be
  sent as it is after the "hints" file was processed.
- notrealm:
  By default if a realm is matched, it will be proxied to the server
  specified.  However, if you are using Replication functionality, you
  may want to override this behaviour.  This option will prevent a
  user who enters ``user@foobar`` from being proxied if the ``foobar``
  realm configuration contains ``notrealm``.  This function used to be
  called ``notsuffix``, and the old syntax is still supported.

The ``/etc/raddb/realms`` file is deprecated and should not be used anymore.
If you use the ``/etc/raddb/realms`` file to enter realm configurations you will
need to add the hostname and secret for the remote server in the
file ``/etc/raddb/clients.conf``.
It is not recommended to use both the realms file and the proxy.conf file,
as that could cause confusion.

Accounting
----------

All accounting data for proxied requests does `not` get stored in the
standard logfiles, but in a separate directory. The name of this
directory is the name of the remote radius server.

Remote Server
----------------

When your server proxies requests to another server, it acts as a NAS for
the remote server. On the remote server, you need to add the hostname of
your server and the same secret to ``/etc/raddb/clients.conf`` as well.

As you might not control the remote radius server, you might want to
control the attributes sent back by the remote server in an Access-Accept
packet. Have a look at the attrs file for this!

What Happens
---------------
The exact thing that happens is this:

- A user logs in with a realm
- The hints file gets processed as usual
- The user is checked against the huntgroups file. At this point
  the user `might` already be rejected.
- The realm is looked up in the realms file. If it isn't defined,
  the users file is processed normally.
- If the ``notrealm`` option is defined, the user is processed
  locally.
- The realm is stripped from the username unless ``nostrip`` was
  set, and the request is sent to a remote radius server. Note that
  any stripping done in the hints file doesn't have an effect on the
  username sent to the remote radius server unless you set the
  ``hints`` option.
- The remote server replies with Access-Accept or Access-Reject

::

  On Access-Accept:    The initial Auth-Type is set to Accept
  On Access-Reject:    The initial Auth-Type is set to Reject

Then the users file is processed as usual. The username used at
this point is the one after hints file processing (regardless of
the ``hints`` option). It also includes the realm (regardless of the
setting of the ``nostrip`` option) unless the realm is ``LOCAL``.
