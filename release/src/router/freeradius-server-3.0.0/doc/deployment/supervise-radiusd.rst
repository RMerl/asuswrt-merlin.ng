
Supervising the Radiusd Daemon
==============================

Introduction
------------

We all hope that our radius daemons won't die in the middle of the
nite stranding customer and beeping beepers.  But, alas, it's going to
happen, and when you least expect it.  That's why you want a another
process watching your radius daemon, restarting it if and when it
dies.

This text describes how to setup both the free radius daemon so that
it is automatically restarted if the process quits unexpectedly.  To
do this, we'll use either Dan Bernstein's 'daemontools' package or the
inittab file. Note: The radwatch script that used to be part of this
distribution, is depreciated and SHOULD NOT BE USED.

Setting Up Daemontools
----------------------

First, download (and install) daemontools from:

	http://cr.yp.to/daemontools.html

The latest version as of this writing is 0.70.  It would be well worth
your while to read all the documentation at that site too, as you can
do much more with daemontools than I describe here.

Next, we'll need a directory for the radius 'service' to use with
daemontools.  I usually create a dir '/var/svc' to hold all my
daemontool supervised services. i.e.::

  $ mkdir /var/svc
  $ mkdir /var/svc/radiusd

Now we just need a short shell script called 'run' in our new service
directory that will start our daemon.  The following should get you
started::

  #!/bin/sh
  # Save as /var/svc/radiusd/run
  exec /usr/local/sbin/radiusd -s -f

Of course you'll want to make that 'run' file executable::

  $ chmod +x /var/svc/radiusd/run

Note, you *MUST* use the '-f' option when supervising.  That option
tells radiusd not to detach from the tty when starting.  If you don't
use that option, the daemontools will always think that radiusd has
just died and will (try to) restart it.  Not good.

Now the only left to do is to start the 'supervise' command that came
with daemontools.  Do that like so::

  $ supervise /var/svc/radiusd

Maintenance With Daemontools
----------------------------

 Any maintenance you need to do with almost certainly be done with the
 'svc' program in the deamontools package.  i.e.::

  Shutdown radiusd:
  $ svc -d /var/svc/radiusd

  Start it back up:
  $ svc -u /var/svc/radiusd

  Send HUP to radiusd:
  $ svc -h /var/svc/radiusd

  Shutdown and stop supervising radiusd:
  $ svc -dx /var/svc/radiusd

Supervising With Inittab
------------------------

This is really pretty easy, but it is system dependent.  I strongly
suggest you read the man pages for your 'init' before playing with
this.  You can seriously hose your system if you screw up your
inittab.

Add this line (or something similar to it) to your inittab::

   fr:23:respawn:/usr/local/sbin/radiusd -f -s &> /dev/null

Now all that's left is to have the system reread the inittab.  Usually
that's done with one of the following::

      $ telinit Q

or::

      $ init q

Now you should see a 'radiusd' process when you issue a 'ps'.  If you
don't, try to run the radiusd command you put in inittab manually. If
it works, that means you didn't tell the system to reread inittab
properly.  If it doesn't work, that means your radius start command is
bad and you need to fix it.

Acknowledgements
----------------

     Document author                 :  Jeff Carneal
     daemontools auther              :  Dan Bernstein
     Further daemontool notes (below):  Antonio Dias
     Radwatch note                   : Andrey Melnikov

Further Daemontools notes
=========================

Here are some notes by Antonia Dias sent to the free radius mailing
list. Some of you may find this useful after reading the above and the
docs for daemontools.

Daemontools Instructions
------------------------

I am running radiusd under supervise from daemontools without
problems. The only thing I am missing right now is an option to force
radiusd to send log to stderr so I can manage logs better with
multilog (also included in daemontools package). Here is the procedure
I've been following (for Cistron RADIUS)::

   $ groupadd log
   $ useradd -g log log
   $ mkdir /etc/radiusd
   $ mkdir /etc/radiusd/log
   $ mkdir /etc/radiusd/log/main
   $ chmod +t+s /etc/radiusd /etc/radiusd/log
   $ chown log.log /etc/radiusd/log/main

Here are the contents of run files from '/etc/radiusd' and '/etc/radiusd/log'::

  $ cd /etc/radiusd
  $ cat run
  #!/bin/sh
  exec 2>&1
  exec /usr/sbin/radiusd -fyzx
  $ cd /etc/radiusd/log
  $ cat run
  #!/bin/sh
  exec setuidgid log multilog t ./main

 To make service wake-up do::

  $ ln -sf /etc/radiusd /service

 Hang-up (to reload config) it using::

  $ svc -h /service/radiusd

Disable (down) it using::

  $ svc -d /service/radiusd

Reenable (up) it using::

  $ svc -u /service/radiusd
