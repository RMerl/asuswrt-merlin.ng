The FreeRADIUS server
=====================

0. BRANCH STATE
---------------
|BuildStatus|_

.. |BuildStatus| image:: https://travis-ci.org/FreeRADIUS/freeradius-server.png
.. _BuildStatus: https://travis-ci.org/FreeRADIUS/freeradius-server

1. INTRODUCTION
---------------

The FreeRADIUS Server Project is a high performance and highly
configurable RADIUS server that is available under the terms of the
GNU GPLv2.  Using RADIUS allows authentication and authorization for a
network to be centralized, and minimizes the number of changes that
have to be done when adding or deleting new users to a network.

FreeRADIUS can authenticate users on systems such as 802.1x (WiFi),
dialup, PPPoE, VPN's, VoIP, and many others.  It supports back-end
databases such as MySQL, PostgreSQL, Oracle, Microsoft Active
Directory, OpenLDAP, and many more.  It is used daily to authenticate
the Internet access for hundreds of millions of people, in sites
ranging from 10 users, to 10 million and more users.

Version 3.0 of the server is largely compatible with version 2.x, but
we highly recommend that you recreate your configuration, rather than
trying to get the older configuration to work.

For a list of changes in version 3.0, please see ``doc/ChangeLog``.

See ``raddb/README.rst`` for information on what to do to update your
configuration.

Administrators upgrading from a previous version should install this
version in a different location from their existing systems.  Any
existing configuration should be carefully migrated to the new
version, in order to take advantage of the new features which can
greatly simply configuration.

Please see http://freeradius.org and http://wiki.freeradius.org for
more information.


2. INSTALLATION
---------------

To install the server, please see the INSTALL file in this directory.


3. DEBUGGING THE SERVER
-----------------------

Run the server in debugging mode, (``radiusd -X``) and READ the output.
We cannot emphasize this point strongly enough.  The vast majority of
problems can be solved by carefully reading the debugging output,
which includes WARNINGs about common issues, and suggestions for how
they may be fixed.

Read the FAQ.  Many questions are answered there.  See the Wiki

http://wiki.freeradius.org

Read the configuration files.  Many parts of the server have NO
documentation, other than comments in the configuration file.

Search the mailing lists.  There is a Google link on the bottom of
the page:

http://www.freeradius.org/list/users.html

Type some key words into the search box, and you should find
discussions about common problems and solution.


4. ADDITIONAL INFORMATION
-------------------------

See ``doc/README`` for more information about FreeRADIUS.

There is an O'Reilly book available.  It serves as a good
introduction for anyone new to RADIUS.  However, it is almost 11 years
old, and is not much more than a basic introduction to the subject.

http://www.amazon.com/exec/obidos/ASIN/0596003226/freeradiusorg-20/

For other RADIUS information, the Livington internet site had a lot
of information about radius online.  Unfortunately Livingston, and the
site, don't exist anymore but there is a copy of the site still at:

http://portmasters.com/www.livingston.com/

Especially worth reading is the "RADIUS for Unix administrators guide"

* HTML:  http://portmasters.com/tech/docs/radius/1185title.html
* PDF:   http://portmasters.com/tech/docs/pdf/radius.pdf


5. PROBLEMS AND CONCERNS
------------------------

We understand that the server may be difficult to configure,
install, or administer.  It is, after all, a complex system with many
different configuration possibilities.

The most common problem is that people change large amounts of the
configuration without understanding what they're doing, and without
testing their changes.  The preferred method of operation is the
following:

1. Start off with the default configuration files.
2. Save a copy of the default configuration: It WORKS.  Don't change it!
3. Verify that the server starts.  (You ARE using debugging mode, right?)
4. Send it test packets using "radclient", or a NAS or AP.
5. Verify that the server does what you expect.
      - If it does not work, change the configuration, and go to step (3) 
        If you're stuck, revert to using the "last working" configuration.
      - If it works, proceed to step (6).
6. Save a copy of the working configuration, along with a note of what 
   you changed, and why.
7. Make a SMALL change to the configuration.
8. Repeat from step (3).

This method will ensure that you have a working configuration that
is customized to your site as quickly as possible.  While it may seem
frustrating to proceed via a series of small steps, the alternative
will always take more time.  The "fast and loose" way will be MORE
frustrating than quickly making forward progress!


6. FEEDBACK
-----------

If you have any comments, bug reports, problems, or concerns, please
send them to the 'freeradius-users' list (see the URL above).  We will
do our best to answer your questions, to fix the problems, and to
generally improve the server in any way we can.

Please do NOT complain that the developers aren't answering your
questions quickly enough, or aren't fixing the problems quickly
enough.  Please do NOT complain if you're told to go read
documentation.  We recognize that the documentation isn't perfect, but
it *does* exist, and reading it can solve most common questions.

FreeRADIUS is the cumulative effort of many years of work by many
people, and you've gotten it for free.  No one gets paid to work on
FreeRADIUS, and no one is getting paid to answer your questions.  This
is free software, and the only way it gets better is if you make a
contribution back to the project ($$, code, or documentation).

We will note that the people who get most upset about any answers to
their questions usually do not have any intention of contributing to
the project.  We will repeat the comments above: no one is getting
paid to answer your questions or to fix your bugs.  If you don't like
the responses you are getting, then fix the bug yourself, or pay
someone to address your concerns.  Either way, make sure that any fix
is contributed back to the project so that no one else runs into the
same issue.

Support is available.  See the "support" link at the top of the main
web page:

http://freeradius.org

Please submit bug reports, suggestions, or patches.  That feedback
gives the developers a guide as to where they should focus their work.
If you like the server, feel free to mail the list and say so.
