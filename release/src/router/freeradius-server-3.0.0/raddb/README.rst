Upgrading to Version 3.0
========================

The configuration for 3.0 is *largely* compatible with the 2.x.x
configuration.  However, it is NOT possible to simply use the 2.x.x
configuration as-is.  Instead, you should re-create it.

Security
--------

A number of configuration items have moved into the "security"
subsection of radiusd.conf.  If you use these, you should move them.
Otherwise, they can be ignored.

The list of moved options is::

  chroot
  user
  group
  allow_core_dumps
  reject_delay
  status_server

These entries should be moved from "radiusd.conf" to the "security"
subsection of that file

Modules Directory
-----------------

As of version 3.0, the ``modules/`` directory no longer exists.

Instead, all "example" modules have been put into the
``mods-available/`` directory.  Modules which can be loaded by the
server are placed in the ``mods-enabled/`` directory.  All of the
modules in that directory will be loaded.  This means that the
"instantiate" section of radiusd.conf is less important.

Modules can be enabled by creating a soft link.  For module ``foo``, do::

  $ cd raddb
  $ ln -s mods-available/foo mods-enabled/foo

To create "local" versions of the modules, we suggest copying the file
instead.  This leaves the original file (with documentation) in the
``mods-available/`` directory.  Local changes should go into the
``mods-enabled/`` directory.

Module-specific configuration files are now in the ``mods-config/``
directory.  This change allows for better organization, and means that
there are fewer files in the main ``raddb`` directory.  See
``mods-config/README.rst`` for more details.

Naming
------

Many names used by configuration items were inconsistent in earlier
versions of the server.  These names have been unified in version 3.0.

If a file is being referenced or created the config item ``filename``
is used.

If a file is being created, the initial permissions are set by the
``permissions`` config item.

If a directory hierarchy needs to be created, the permissions are set
by ``dir_permissions``.

If an external host is referenced in the context of a module the
``server`` config item is used.

Unless the config item is a well recognised portmanteau
(as ``filename`` is for example), it must be written as multiple
distinct words separated by underscores ``_``.

The configuration items ``file``, ``script_file``, ``module``,
``detail``, ``detailfile``, ``attrsfile``, ``perm``, ``dirperm``,
``detailperm``, and ``hostname`` are deprecated. As well as any
faux portmanteaus, and configuration items that used hyphens
as word delimiters.
Please update your module configuration to use the new syntax.

In most cases the server will tell you the replacement config item to
use.  As always, run the server in debugging mode to see these
messages.

rlm_sql
-------

The SQL configuration has been moved from ``sql.conf`` to
``mods-available/sql``.  The ``sqlippool.conf`` file has also been
moved to ``mods-available/sqlippool``.

The SQL module configuration has been changed.  The old connection
pool options are no longer accepted::

  num_sql_socks
  connect_failure_retry_delay
  lifetime
  max_queries

Instead, a connection pool configuration is used.  This configuration
contains all of the functionality of the previous configuration, but
in a more generic form.  It also is used in multiple modules, meaning
that there are fewer different configuration items.  The mapping
between the configuration items is::

  num_sql_socks			-> pool { max }
  connect_failure_retry_delay	-> NOT SUPPORTED
  lifetime			-> pool { lifetime }
  max_queries			-> pool { uses }

The pool configuration adds a number of new configuration options,
which allow the administrator to better control how FreeRADIUS uses
SQL connection pools.

The following parameters have been changed::

  trace				-> removed
  tracefile			-> logfile

The logfile is intended to log SQL queries performed.  If you need to
debug the server, use debugging mode.  If ``logfile`` is set, then
*all* SQL queries will go to ``logfile``.

You can now use a NULL SQL database::

  driver = rlm_sql_null

This is an empty driver which will always return "success".  It is
intended to be used to replace the ``sql_log`` module, and in
conjunction with the ``radsqlrelay`` program.  Simply take your normal
configuration for raddb/mods-enabled/sql, and set::

  driver = rlm_sql_null
  ...
  logfile = ${radacctdir}/log.sql

And all of the SQL queries will be logged to that file.  The
connection pool	will still need to be configured for the NULL SQL
driver, but the defaults will work.

rlm_sql_sybase
--------------

The ``rlm_sql_sybase`` module has been renamed to ``rlm_sql_freetds``
and the old ``rlm_sql_freetds`` module has been removed.

``rlm_sql_sybase`` used the newer ct-lib API, and ``rlm_sql_freetds``
used and older API and was incomplete.

The new ``rlm_sql_freetds`` module now also supports database
selection on connection startup so ``use`` statements no longer
have to be included in queries.

sql/dialup.conf
---------------

Queries for post-auth and accounting calls have been re-arranged.  The
SQL module will now expand the 'reference' configuration item in the
appropriate sub-section, and resolve this to a configuration
item. This behaviour is similar to rlm_linelog.  This dynamic
expansion allows for a dynamic mapping between accounting types and
SQL queries.  Previously, the mapping was fixed.  Any "new" accounting
type was ignored by the module.  Now, support for any accounting type
can be added by just adding a new target, as below.

Queries from v2.x.x may be manually copied to the new v3.0
``dialup.conf`` file (``raddb/sql/main/<dialect>/queries.conf``).
When doing this you may also need to update references to the
accounting tables, as their definitions will now be outside of
the subsection containing the query.

The mapping from old "fixed" query to new "dynamic" query is as follows::

  accounting_onoff_query		-> accounting.type.accounting-on.query
  accounting_update_query		-> accounting.type.interim-update.query
  accounting_update_query_alt		+> accounting.type.interim-update.query
  accounting_start_query		-> accounting.type.start.query
  accounting_start_query_alt		+> accounting.type.start.query
  accounting_stop_query			-> accounting.type.stop.query
  accounting_stop_query_alt		+> accounting.type.stop.query
  postauth_query			-> post-auth.query

Alternatively a 2.x.x config may be patched to work with the
3.0 module by adding the following::

  accounting {
  	reference = "%{tolower:type.%{Acct-Status-Type}.query}"
  	type {
  		accounting-on {
  			query = "${....accounting_onoff_query}"
  		}
  		accounting-off {
  			query = "${....accounting_onoff_query}"
  		}
   		start {
  			query = "${....accounting_start_query}"
  			query = "${....accounting_start_query_alt}"
  		}
  		interim-update {
  			query = "${....accounting_update_query}"
  			query = "${....accounting_update_query_alt}"
  		}
  		stop {
  			query = "${....accounting_stop_query}"
  			query = "${....accounting_stop_query_alt}"
  		}
  	}
  }

  post-auth {
  	query = "${..postauth_query}"
  }

In general, it is safer to migrate the configuration rather than
trying to "patch" it, to make it look like a v2 configuration.

rlm_ldap
--------

The LDAP module configuration has been substantially changed.  Please
read ``raddb/mods-available/ldap``.  It now uses a connection pool,
just like the SQL module.

Many of the configuration items remain the same, but they have been
moved into subsections.  This change is largely cosmetic, but it makes
the configuration clearer.  Instead of having a large set of random
configuration items, they are now organized into logical groups.

You will need to read your old LDAP configuration, and migrate it
manually to the new configuration.  Simply copying the old
configuration WILL NOT WORK.

Users upgrading from 2.x.x who used to call the ldap module in
``post-auth`` should now set ``edir_autz = yes``, and remove the ``ldap``
module from the ``post-auth`` section.

rlm_ldap/LDAP-Group
-------------------

In 2.x.x the registration of the ``LDAP-Group`` pair comparison was done
by the last instance of rlm_ldap to be instantiated. In 3.0 this has
changed so that only the default ``ldap {}`` instance registers
``LDAP-Group``.

If ``<instance>-LDAP-Group`` is already used throughout your configuration
no changes need to be made.

rlm_eap
-------

The EAP configuration has been moved from ``eap.conf`` to
``mods-available/eap``.  A new ``pwd`` subsection has been added for
EAP-PWD.

rlm_expiration & rlm_logintime
-------------------------------

The rlm_expiration and rlm_logintime modules no longer add a ``Reply-Message``,
the same behaviour can be achieved checking the return code of the module and
adding the ``Reply-Message`` with unlang::

  expiration
  if (userlock) {
    update reply {
      Reply-Message := "Your account has expired"
    }
  }

rlm_unix
--------

The unix module does not have an "authenticate" section.  So you
cannot set "Auth-Type := System".  The "unix" module has also been
deleted from the examples in sites-available/.  Listing it there has
been deprecated for many years.

The PAP module can do crypt authentication.  It should be used instead
of Unix authentication.

The Unix module still can pull the passwords from /etc/passwd, or
/etc/shadow.  This is done by listing it in the "authorize" section,
as is done in the sites-available/ examples.


RadSec
------

RadSec (or RADIUS over TLS) is now supported.  RADIUS over bare TCP
is also supported, but is recommended only for secure networks.

See ``sites-available/tls`` for complete details on using TLS.  The server
can both receive incoming TLS connections, and also originate outgoing
TLS connections.

The TLS configuration is taken from the old EAP-TLS configuration.  It
is largely identical to the old EAP-TLS configuration, so it should be
simple to use and configure.  It re-uses much of the EAP-TLS code,
so it is well-tested and reliable.

Once RadSec is enabled, normal debugging mode will not work.  This is
because the TLS code requires threading to work properly.  Instead of doing::

  $ radiusd -X

you will need to do::

  $ radiusd -fxx -l stdout

Sorry, but that's the price to pay for using RadSec.

PAP and User-Password
---------------------

From version 3.0 onwards the server no longer supports authenticating
against a cleartext password in the 'User-Password' attribute. Any
occurences of this (for instance, in the users file) should now be changed
to 'Cleartext-Password' instead.

If this is not done, authentication will likely fail.  The server will
also print a helpful message in debugging mode.

If it really is impossible to do this, the following unlang inserted above
the call to the pap module may be used to copy User-Password to the correct
attribute::

  if (!control:Cleartext-Password && control:User-Password) {
    update control {
      Cleartext-Password := "%{control:User-Password}"
    }
  }

However, this should only be seen as a temporary, not permanent, fix.
It is better to fix your databases to contain the correct
configuration.

Deleted Modules
===============

The following modules have been deleted, and are no longer supported
in Version 3.  If you are using one of these modules, your
configuration can probably be changed to not need it.  Otherwise email
the freeradius-devel list, and ask about the module.

rlm_acct_unique
---------------

This module has been replaced by the "acct_unique" policy.  See
raddb/policy.d/accounting.

The method for calculating the value of acct_unique has changed.
However, as this method was configurable, this change should not
matter.  The only issue is in having a v2 and v3 server writing to the
same database at the same time.  They will calculate different values
for Acct-Unique-Id.

rlm_acctlog
-----------

You should use rlm_linelog instead.  That module has a superset of the
acctlog functionality.

rlm_attr_rewrite
----------------

The attr_rewrite module looked for an attribute, and then re-wrote it,
or created a new attribute.  All of that can be done in "unlang".

A sample configuration in "unlang" is::

  if (request:Calling-Station-Id) {
    update request {
      Calling-Station-Id := "...."
    }
  }

We suggest updating all uses of attr_rewrite to use unlang instead.

rlm_checkval
------------

The checkval module compared two attributes.  All of that can be done in "unlang"::

  if (&request:Calling-Station-Id == &control:Calling-Station-Id) {
    ok
  }

We suggest updating all uses of checkval to use unlang instead.

rlm_dbm
-------

No one seems to use it.  There is no sample configuration for it.
There is no speed advantage to using it over the "files" module.
Modern systems are fast enough that 10K entries can be read from the
"users" file in about 10ms.  If you need more users than that, use a
real database such as SQL.

rlm_fastusers
-------------

No one seems to use it.  It has been deprecated since Version 2.0.0.
The "files" module was rewritten so that the "fastusers" module was no
longer necessary.

rlm_policy
----------

No one seems to use it.  Almost all of its functionality is available
via "unlang".

rlm_sim_files
-------------

The rlm_sim_files module has been deleted.  It was never marked "stable",
and was never used in a production environment.  There are better ways
to test EAP.

If you want similar functionality, see rlm_passwd.  It can read CSV
files, and create attributes from them.

rlm_sql_log
-----------

This has been replaced with the "null" sql driver.  See
raddb/mods-available/sql for an example configuration.

The main SQL module has more functionality than rlm_sql_log, and
results in less code in the server.
