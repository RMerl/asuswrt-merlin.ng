Stubby integration with systemd
===============================

For GNU/Linux operating systems which use systemd as a process
manager, you might want to run stubby as a system service.

This directory provides recommended systemd unit files.
Normally, a downstream distributor will install it as:

    /lib/systemd/system/stubby.service

For systemd versions before 235:

  * This setup assumes that there is a system-level user named "stubby"
    which is in group "stubby", and try to limit the privileges of the running
    daemon to that user as closely as possible.

    The stubby.conf file also needs to be installed in:

        /usr/lib/tmpfiles.d/stubby.conf

    to make sure a cache directory is created for stubby on startup.


For systemd version of 235 and higher:

  * Creation of the system-level user and the cache directory is handled
    automatically by systemd and no further actions are required.


When the system-level user does have a home directory, stubby will store the
for Zero configuration DNSSEC dynamically acquired root trust anchor in a
subdirectory called ".getdns" of that home directory.  If the system-level
user does not have a home directory or the home directory is not writeable
or readable, stubby will fallback to the current working directory.

This can be overruled by supplying a "appdata_dir" in the stubby.yml
configuration file.  When a "appdata_dir" was specified, that directory will be
used for storing data related to Zero configuration DNSSEC immediately, without
the other paths being tried.  It is recommended for systemd setups using the
provided systemd.service file(s) to have a "appdata_dir" directive set to
"/var/cache/stubby" in the stubby.yml configuration file.
