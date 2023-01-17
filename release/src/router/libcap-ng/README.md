libcap-ng
=========

The libcap-ng library should make programming with posix capabilities
easier. The library has some utilities to help you analyse a system
for apps that may have too much privileges.

The included utilities are designed to let admins and developers spot apps from various ways that may be running with too much privilege. For example, any investigation should start with network facing apps since they would be prime targets for intrusion. The netcap program will check all running apps and display the results. Sample output from netcap:

```
ppid  pid   acct       command          type port  capabilities
1     2295  root       nasd             tcp  8000  full
2323  2383  root       dnsmasq          tcp  53    net_admin, net_raw +
1     2286  root       sshd             tcp  22    full
1     2365  root       cupsd            tcp  631   full
1     2286  root       sshd             tcp6 22    full
1     2365  root       cupsd            tcp6 631   full
2323  2383  root       dnsmasq          udp  53    net_admin, net_raw +
2323  2383  root       dnsmasq          udp  67    net_admin, net_raw +
1     2365  root       cupsd            udp  631   full
```

C Examples
----------
As an application developer, there are probabaly 6 use cases that you are
interested in: drop all capabilities, keep one capability, keep several
capabilities, check if you have any capabilities at all, check for certain
capabilities, and retain capabilities across a uid change.

```
1) Drop all capabilities
     capng_clear(CAPNG_SELECT_BOTH);
     capng_apply(CAPNG_SELECT_BOTH);

2) Keep one capability
     capng_clear(CAPNG_SELECT_BOTH);
     capng_update(CAPNG_ADD, CAPNG_EFFECTIVE|CAPNG_PERMITTED, CAP_CHOWN);
     capng_apply(CAPNG_SELECT_BOTH);

3) Keep several capabilities
     capng_clear(CAPNG_SELECT_BOTH);
     capng_updatev(CAPNG_ADD, CAPNG_EFFECTIVE|CAPNG_PERMITTED, CAP_SETUID, CAP_SETGID, -1);
     capng_apply(CAPNG_SELECT_BOTH);

4) Check if you have any capabilities
     if (capng_have_capabilities(CAPNG_SELECT_CAPS) > CAPNG_NONE)
         do_something();

5) Check for certain capabilities
     if (capng_have_capability(CAPNG_EFFECTIVE, CAP_CHOWN))
         do_something();

6) Retain capabilities across a uid change
     capng_clear(CAPNG_SELECT_BOTH);
     capng_update(CAPNG_ADD, CAPNG_EFFECTIVE|CAPNG_PERMITTED, CAP_CHOWN);
     if (capng_change_id(99, 99, CAPNG_DROP_SUPP_GRP | CAPNG_CLEAR_BOUNDING))
         error();
```

Now, isn't that a lot simpler? Note that the last example takes about 60 lines
of code using the older capabilities library. As of the 0.6 release, there is
a m4 macro file to help adding libcap-ng to your autotools config system. In
configure.ac, add LIBCAP_NG_PATH. Then in Makefile.am locate the apps that
link to libcap-ng, add $(CAPNG_LDADD) to their LDADD entries. And lastly,
surround the optional capabilities code with #ifdef HAVE_LIBCAP_NG.

Python
------
Libcap-ng 0.6 and later has python bindings. You simply add import capng in your script.  Here are the same examples as above in python:

```
1) Drop all capabilities
     capng.capng_clear(capng.CAPNG_SELECT_BOTH)
     capng.capng_apply(capng.CAPNG_SELECT_BOTH)

2) Keep one capability
     capng.capng_clear(capng.CAPNG_SELECT_BOTH)
     capng.capng_update(capng.CAPNG_ADD, capng.CAPNG_EFFECTIVE|capng.CAPNG_PERMITTED, capng.CAP_CHOWN)
     capng.capng_apply(capng.CAPNG_SELECT_BOTH)

3) Keep several capabilities
     capng.capng_clear(capng.CAPNG_SELECT_BOTH)
     capng.capng_updatev(capng.CAPNG_ADD, capng.CAPNG_EFFECTIVE|capng.CAPNG_PERMITTED, capng.CAP_SETUID, capng.CAP_SETGID, -1)
     capng.capng_apply(capng.CAPNG_SELECT_BOTH)

4) Check if you have any capabilities
     if capng.capng_have_capabilities(capng.CAPNG_SELECT_CAPS) > capng.CAPNG_NONE:
         do_something()

5) Check for certain capabilities
     if capng.capng_have_capability(capng.CAPNG_EFFECTIVE, capng.CAP_CHOWN):
         do_something()

6) Retain capabilities across a uid change
     capng.capng_clear(capng.CAPNG_SELECT_BOTH)
     capng.capng_update(capng.CAPNG_ADD, capng.CAPNG_EFFECTIVE|capng.CAPNG_PERMITTED, capng.CAP_CHOWN)
     if capng.capng_change_id(99, 99, capng.CAPNG_DROP_SUPP_GRP | capng.CAPNG_CLEAR_BOUNDING) < 0:
         error()
```

The one caveat is that printing capabilities from python does not work. But
you can still manipulate capabilities, though.


NOTE: to distributions
----------------------
There is a "make check" target. It only works if the headers match the kernel.
IOW, if you have a chroot build system that is using a much older kernel,
the macros in the kernel header files will do the wrong thing when the
capng_init function probes the kernel and decides we are doing v1 rather
than v3 protocol. If that is your case, just don't do the "make check" as
part of the build process.


Reporting
---------
Report any bugs in this package to:
https://github.com/stevegrubb/libcap-ng/issue

