Cisco IOS and Radius
====================

Introduction
------------

Cisco NAS equipment has become quite popular of late, but being Cisco
equipment running IOS, the configuration can be a bit non-obvious to the
unfamiliar.  This document aims to describe the most common configuration
options to make your Ciscos interoperate with radius as you would expect a
well-behaved NAS to do.

IOS 12.x
--------

For Cisco 12.x ( 12.0 and 12.1 ), the following AAA configuration directives
are suggested:

::

  aaa new-model
  aaa authentication login default group radius local
  aaa authentication login localauth local
  aaa authentication ppp default if-needed group radius local
  aaa authorization exec default group radius local
  aaa authorization network default group radius local
  aaa accounting delay-start
  aaa accounting exec default start-stop group radius
  aaa accounting network default start-stop group radius
  aaa processes 6

this configuration works very well with most radius servers.  One of the more
important configurations is:

::

  aaa accounting delay-start

This directive will delay the sending of the Accounting Start packet until
after an IP address has been assigned during the PPP negotiation process.
This will supersede the need to enable the sending of "Alive" packets as
described below for IOS versions 11.x

*Note* with the above it will use the radius server to authenticate
your inbound 'telnet' connections.  You will need to create an entry
in your users file similar to the following to allow access:

::

  !root Cleartext-Password := "somepass" Service-Type = NAS-Prompt-User

This will let a user in for the first level of access to your Cisco.  You
will still need to 'enable' ( using the locally configured enable secret )
to perform any configuration changes or anything requiring a higher level
of access.  The username '!root' was used as an example here, you can make
this any username you want, of course.

Unique Acct-Session-Id's
^^^^^^^^^^^^^^^^^^^^^^^^

From: http://isp-lists.isp-planet.com/isp-australia/0201/msg05143.html

Just a note to all cisco ISPs out there who want RFC2866 compliance need to
enable the hidden command ``radius-server unique-ident <n>``

Minimum IOS: 12.1(4.1)T.

Acct-Session-Id should be unique and wrap after every 256 reboots.

You must reboot after entering this command to take effect. If not, you
will observe after 10 minutes
of entering this command, the following message.

::

  %RADIUS-3-IDENTFAIL: Save of unique accounting ident aborted.

IOS 11.x
--------

For Cisco 11.1, you normally use

::

  aaa new-model
  aaa authentication ppp radppp if-needed radius
  aaa authorization network radius none
  aaa accounting network wait-start radius

to get the Cisco to talk to a radius server.

With IOS 11.3
^^^^^^^^^^^^^

::

  aaa accounting update newinfo

If you want the IP address of the user to show up in the radutmp file
(and thus, the output of "radwho").

This is because with IOS 11.3, the Cisco first sends a "Start" accounting
packet without the IP address included. By setting "update newinfo" it
will send an account "Alive" packet which updates the information.

Also you might see a lot of "duplicates" in the logfile. That can be
fixed by:

::

  aaa accounting network wait radius
  radius-server timeout 3

To disable the Ascend style attributes (which is a VERY good idea!):

::

  radius-server host X.Y.Z.A auth-port 1645 acct-port 1646

To enable the Ascend style attributes (which we do NOT recommend!):

::

  radius-server host X.Y.Z.A auth-port 1645 acct-port 1646 non-standard

To see Cisco-AVPair attributes in the Cisco debugging log:

  radius-server vsa accounting

Cisco 36xx & 26xx, keeping the NAS IP static
--------------------------------------------

The Cisco 36/26 by default selects (it seems at random) any IP address
assigned to it (serial, ethernet etc.) as it's RADIUS client source
address, thus the access request may be dropped by the RADIUS server,
because it can not verify the client. To make the cisco box always use
one fixed address, add the following to your configuration:

::

  ip radius source-interface Loopback0

and configure the loopback interface on your router as follows:

::

  interface Loopback0
   ip address 192.0.2.250 255.255.255.255

Use a real world IP address and check the Cisco documentation for why
it is a good idea to have working loopback interface configured on
your router.

If you don't want to use the loopback interface of course you can set
the source-interface to any interface on your Cisco box which has an
IP address.

Credits
-------

Original  - Alan DeKok <aland@ox.org>
12.x Info - Chris Parker <cparker@starnetusa.net>  2000-10-12

More Information
----------------
For more information, the following page on Cisco's web site may help:

http://www.cisco.com/univercd/cc/td/doc/product/access/acs_serv/vapp_dev/vsaig3.htm
