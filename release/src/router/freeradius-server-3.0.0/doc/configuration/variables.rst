Run-time variables
==================

See "man unlang" for more complete documentation on the run-time
variables.  This file is here only for historical purposes.

The above variable expansions also support the following
meta-attributes.  These are not normal RADIUS attributes, but are
created by the server to be used like them, for ease of use.  They can
only be queried, and cannot be assigned.

+-----------------------+-------------------------------------------------+
| Packet-Type           | RADIUS packet type (Access-Request, etc.)       |
+-----------------------+-------------------------------------------------+
| Packet-Src-IP-Address | IP address from which the packet was sent       |
+-----------------------+-------------------------------------------------+
| Packet-Dst-IP-Address | IP address to which the packet was sent.        |
|                       | This may be "0.0.0.0", if the server            |
|                       | was configured with ``bind_address = *``.       |
+-----------------------+-------------------------------------------------+
| Packet-Src-Port       | UDP port from which the packet was sent         |
+-----------------------+-------------------------------------------------+
| Packet-Dst-Port       | UDP port to which the packet was sent.          |
+-----------------------+-------------------------------------------------+

``%{config:section.subsection.item}``
  Corresponding value in ``radiusd.conf`` for the string value of that item.

The ``%{config:...}`` variables should be used VERY carefully, as they
may leak secret information from your RADIUS server, if you use them
in reply attributes to the NAS!

::

  DEFAULT  User-Name =~ "^([^@]+)@(.*)"
  	   All-That-Matched = `%{0}`
  	   Just-The-User-Name = `%{1}`
  	   Just-The-Realm-Name = `%{2}`


The variables are used in dynamically translated strings.  Most of the
configuration entries in ``radiusd.conf`` (and related files) will do
dynamic string translation.  To do the same dynamic translation in a
RADIUS attribute (when pulling it from a database, or "users" file),
you must put the string into an back-quoted string:

::

  Session-Timeout = `%{expr: 2 + 3}`

To do the dynamic translation in the ``radiusd.conf`` (or some other
configuration files), just use the variable as-is.  See
``radiusd.conf`` for examples.


Attributes as environment variables in executed programs
--------------------------------------------------------

When calling an external program (e.g. from ``rlm_exec`` module), these
variables can be passed on the command line to the program. In
addition, the server places all of the attributes in the RADIUS
request into environment variables for the external program. The
variables are renamed under the following rules:

  #. All letters are made upper-case.
  #. All hyphens '-' are turned into underscores '_'

so the attribute ``User-Name`` can be passed on the command line to the
program as ``%{User-Name}``, or used inside the program as the environment
variable ``USER_NAME`` (or ``$USER_NAME`` for shell scripts).

If you want to see the list of all of the variables, try adding a line
``printenv > /tmp/exec-program-wait`` to the script.  Then look in the
file for a complete list of variables.

One-character variables
-----------------------

The following one-character variables were defined.  They were duplicates of the
previous general cases, and were only provided for backwards compatibility.
They are in the process of being removed, this table documents the old variables
and their new equivalents.
(i.e. ``:-``, as described above.

+-----------+---------------------------+-----------------------+
| Variable  | Description               | Proper Equivalent     |
+===========+===========================+=======================+
|%a         |Protocol (SLIP/PPP)        |%{Framed-Protocol}	|
+-----------+---------------------------+-----------------------+
|%c         |Callback-Number		|%{Callback-Number}	|
+-----------+---------------------------+-----------------------+
|%d         |request day (DD)           |                 	|
+-----------+---------------------------+-----------------------+
|%f         |Framed IP address	  	|%{Framed-IP-Address}	|
+-----------+---------------------------+-----------------------+
|%i         |Calling Station ID	  	|%{Calling-Station-Id}	|
+-----------+---------------------------+-----------------------+
|%l         |request timestamp          |		        |
+-----------+---------------------------+-----------------------+
|%m         |request month (MM)         |	                |
+-----------+---------------------------+-----------------------+
|%n         |NAS IP address		|%{NAS-IP-Address}	|
+-----------+---------------------------+-----------------------+
|%p         |Port number		|%{NAS-Port}            |
+-----------+---------------------------+-----------------------+
|%s         |Speed (PW_CONNECT_INFO)    |%{Connect-Info}	|
+-----------+---------------------------+-----------------------+
|%t         |request in ctime format	|		        |
+-----------+---------------------------+-----------------------+
|%u         |User name		  	|%{User-Name}           |
+-----------+---------------------------+-----------------------+
|%A         |radacct_dir		|%{config:radacctdir}	|
+-----------+---------------------------+-----------------------+
|%C         |clientname	                |                       |
+-----------+---------------------------+-----------------------+
|%D         |request date (YYYYMMDD)	|	                |
+-----------+---------------------------+-----------------------+
|%G         |request minute	        |                       |
+-----------+---------------------------+-----------------------+
|%H         |request hour	        |                       |
+-----------+---------------------------+-----------------------+
|%I         |request ID		  	|			|
+-----------+---------------------------+-----------------------+
|%L         |radlog_dir		  	|%{config:logdir}	|
+-----------+---------------------------+-----------------------+
|%M         |MTU			|%{Framed-MTU}          |
+-----------+---------------------------+-----------------------+
|%R         |radius_dir		  	|%{config:raddbdir}	|
+-----------+---------------------------+-----------------------+
|%S         |request timestamp          |                       |
|           |in SQL format              |                       |
+-----------+---------------------------+-----------------------+
|%T         |request timestamp          |                       |
|           |in database format         |                       |
+-----------+---------------------------+-----------------------+
|%U         |Stripped User name	  	|%{Stripped-User-Name}	|
+-----------+---------------------------+-----------------------+
|%V         |Request-Authenticator      |                       |
|           |(Verified/None)            |                       |
+-----------+---------------------------+-----------------------+
|%Y         |request year (YYYY)        |                       |
+-----------+---------------------------+-----------------------+
|%Z         |All request attributes     |                       |
|           |except password            |                       |
|           |(must have a big buffer)   |                       |
+-----------+---------------------------+-----------------------+


 $Id$
