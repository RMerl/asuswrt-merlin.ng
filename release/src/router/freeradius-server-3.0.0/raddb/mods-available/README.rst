Modules in Version 3
====================

As of Version 3, all of the modules have been places in the
"mods-available/" directory.  This practice follows that used by other
servers such as Nginx, Apache, etc.  The "modules" directory should
not be used.

Modules are enabled by creating a file in the mods-enabled/ directory.
You can also create a soft-link from one directory to another::

  $ cd raddb/mods-enabled
  $ ln -s ../mods-available/foo

This will enable module "foo".  Be sure that you have configured the
module correctly before enabling it, otherwise the server will not
start.  You can verify the server configuration by running
"radiusd -XC".

A large number of modules are enabled by default.  This allows the
server to work with the largest number of authentication protocols.
Please be careful when disabling modules.  You will likely need to
edit the "sites-enabled/" files to remove references to any disabled
modules.

Conditional Modules
-------------------

Version 3 allows modules to be conditionally loaded.  This is useful
when you want to have a virtual server which references a module, but
does not require it.  Instead of editing the virtual server file, you
can just conditionally enable the module.

Modules are conditionally enabled by adding a "-" before their name in
a virtual server.  For example, you can do::

  server {
    authorize {
      ...
      ldap
      -sql
      ...
    }
  }

This says "require the LDAP module, but use the SQL module only if it
is configured."

This feature is not very useful for production configurations.  It is,
however, very useful for the default examples that ship with the
server.

Ignoring module
---------------

If you see this message::

  Ignoring module (see raddb/mods-available/README.rst)

Then you are in the right place.  Most of the time this message can be
ignored.  The message can be fixed by find the references to "-module"
in the virtual server, and deleting them.

Another way to fix it is to configure the module, as described above.

Simplification
--------------

Allowing conditional modules simplifies the default virtual servers
that are shipped with FreeRADIUS.  This means that if you want to
enable LDAP (for example), you no longer need to edit the files in
raddb/sites-available/ in order to enable it.

Instead, you should edit the raddb/mods-available/ldap file to point
to your local LDAP server.  Then, enable the module via the soft-link
method described above.

Once the module is enabled, it will automatically be used in the
default configuration.
