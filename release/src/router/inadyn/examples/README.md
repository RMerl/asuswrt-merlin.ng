Example /etc/inadyn.conf files
==============================

This directory holds a few example configuration files for common
DDNS providers.  Please feel free to submit pull requests for your
examples at GitHub! :)

https://gitub.com/troglobit/inadyn


Usage
-----

Simply copy the desired example to /etc/inadyn.conf, edit it with
your hostname, username, and password and then start Inadyn.


Example
-------

    user@example:~$ sudo cp freedns.conf /etc/inadyn.conf
    user@example:~$ sudo chmod 600 /etc/inadyn.conf
    user@example:~$ sudo vim /etc/inadyn.conf
    [Change username, password and hostname]
    user@example:~$ sudo inadyn
    user@example:~$

See the the system logfile for progress and the inadyn man page for
debugging help, should you run into problems.
