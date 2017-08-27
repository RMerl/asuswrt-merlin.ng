
RLM Module Interface (for developers)
=====================================

Overview
--------

Intent of the server
^^^^^^^^^^^^^^^^^^^^

FreeRADIUS is an authentication server.  It does RADIUS authorization,
authentication, and accounting.  It does NOT do database management,
user configuration updates, or email.  All of those functions may be
more easily (and correctly) performed in programs outside of the
server.

The only functionality performed by the server is:

- receive a RADIUS request
 - process the request
 - look up information one or more databases
- store information in one or more databases (proxying can be viewed this way)
- respond to the request

There is no room, or need, for timers, listening on sockets, or
anything else in the server.  Adding those functions to the server
means that it will become more complex, more unstable, more insecure,
and more difficult to maintain.


Intent of the modules
^^^^^^^^^^^^^^^^^^^^^

The intent of modules is that they do small, simple, well-defined
things when RADIUS packets are received.  If the module does things
when RADIUS packets are NOT received, then it has no business being in
the server.  Similarly, the module infrastructure is NOT intended to
allow servers, applications, timed events, or anything other than
handling incoming RADIUS packets.

Modules perform an action when RADIUS packets are received.  Modules
which do more (creating threads, forking programs) will NOT be added
to the server, and the server core will NOT be modified to enable
these kinds of modules.  Those functions more properly belong in a
seperate application.

Modules ARE permitted to open sockets to other network programs, and
to send and receive data on those sockets.  Modules are NOT permitted
to open sockets, and to listen for requests.  Only the server core has
that functionality, and it only listens for RADIUS requests.


Module outline
^^^^^^^^^^^^^^

The fundamental concepts of the rlm interface are module, instance,
and component.

A module is a chunk of code that knows how to deal with a particular
kind of database, or perhaps a collection of similar
databases. Examples:

- rlm_sql contains code for talking to MySQL or Postgres, and for mapping RADIUS records onto SQL tables
- rlm_unix contains code for making radiusd fit in well on unix-like systems, including getpw* authentication and utmp/wtmp-style logging.

An instance specifies the actual location of a collection data that
can be used by a module. Examples:

- /var/log/radutmp
- "the MySQL database on bigserver.theisp.com.example"

A module can have multiple components which act on
RADIUS requests at different stages. The components are:

- authorization: check that a user exists, decide on an authentication method or proxy realm, and possibly apply some attributes to be returned in the reply packet.
- authentication: verify that the password is correct.
- preaccounting: decide whether to proxy the request, and possibly add attributes that should be included in any logs
- accounting: record the request in the log
- checksimul: count the number of active sessions for the user
- postauth: process the response before it's sent to the NAS
- preproxy: process a request before it's proxied
- postproxy: filter attributes from a reply to a proxied request

A module declares which components it supports by putting function
pointers in its "module_t rlm_*" structure.


Module configuration
^^^^^^^^^^^^^^^^^^^^

The administrator requests the creation of a module instance by adding
it inside the modules{} block in radiusd.conf. The instance definition
looks like this::

  module_name [instance_name] {
    param1 = value1
    param2 = value2
    param3 = value3
    ...
  }

The module_name is used to load the module. To see the names of the available
modules, look for the rlm\_\*.so files in $installprefix/lib. The module_name
is that, minus the rlm\_ and the .so.

instance_name is an identifier for distinguishing multiple instances of the
same module. If you are only loading a module once, you can leave out the
instance_name and it will be assumed to be the same as the module_name.

The parameters inside the module block are passed without interpretation to
the module and generally point to the exact location of a database or enable
optional features of the module. Each module should document what parameters
it accepts and what they do.

For each Access-Request that comes to the server, the authorize{}
block is called. Then one of the Auth-Type{} blocks from authenticate{}
is called, depending on the Auth-Type attribute that was chosen by
authorize{}. Finally, the post-auth{} block is called.  If authorize{}
set the Proxy-To-Realm attribute, then proxying takes over via
pre-proxy{} and post-proxy{}, and the local authenticate{} phase is
skipped.

For each Accounting-Request that comes to the server, the preacct{} block is
called, followed by the accounting{} block. accounting{} is skipped if
preacct{} sets Proxy-To-Realm.

For an explanation of what "calling" a config block means, see
the "configurable_failover" file.


The lifetime of a module
^^^^^^^^^^^^^^^^^^^^^^^^

When the server is starting up, or reinitializing itself as a result of a
SIGHUP, it reads the modules{} section. Each configured module will be loaded
and its init() method will be called::

  int init(void)

The init() method should take care of
any setup that is not tied to a specific instance. It will only be called
once, even if there are multiple instances configured.

For each configured instance, after the init() method, the instantiate()
method is called. It is given a handle to the configuration block holding its
parameters, which it can access with cf_section_parse().::

  int instantiate(CONF_SECTION \*cs, void \**instance)

The instantiate() function should look up options in the config section, and
open whatever files or network connections are necessary for the module to do
its job. It also should create a structure holding all of the persistent
variables that are particular to this instance (open file descriptors,
configured pathnames, etc.) and store a pointer to it in \*instance. That
void \* becomes a handle (some would call it a "cookie") representing this
instance. The instance handle is passed as a parameter in all subsequent
calls to the module's methods, so they can determine which database they are
supposed to act on.

The authorize(), authenticate(), preaccounting(), and accounting() functions
are all called the same way::

  int authorize(void \*instance, REQUEST \*request)
  int authenticate(void \*instance, REQUEST \*request)
  int preaccounting(void \*instance, REQUEST \*request)
  int accounting(void \*instance, REQUEST \*request)

they each receive the instance handle and the request, and are expected to
act on the request using the database pointed to by the instance handle
(which was set by the instantiate() function).

When the server is being shut down (as the first part of SIGHUP for example)
detach() is called for each module instance.::

  int detach(void \*instance)

The detach() method should release whatever resources were allocated by the
instantiate() method.

After all instances are detached, the destroy() method is called.::

  int destroy(void)

It should release resources that were acquired by the init() method.

--Alan Curry <pacman@world.std.com>
