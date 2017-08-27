Testing server with a browser
-----------------------------

If you run [libwebsockets-test-server](test-server/test-server.c) and point your browser
(eg, Chrome) to

  http://127.0.0.1:7681

It will fetch a script in the form of `test.html`, and then run the
script in there on the browser to open a websocket connection.
Incrementing numbers should appear in the browser display.

By default the test server logs to both stderr and syslog, you can control
what is logged using `-d <log level>`, see later.


Running test server as a Daemon
-------------------------------

You can use the -D option on the test server to have it fork into the
background and return immediately.  In this daemonized mode all stderr is
disabled and logging goes only to syslog, eg, `/var/log/messages` or similar.

The server maintains a lockfile at `/tmp/.lwsts-lock` that contains the pid
of the master process, and deletes this file when the master process
terminates.

To stop the daemon, do

```bash
$ kill `cat /tmp/.lwsts-lock`
```

If it finds a stale lock (the pid mentioned in the file does not exist
any more) it will delete the lock and create a new one during startup.

If the lock is valid, the daemon will exit with a note on stderr that
it was already running.


Using SSL on the server side
----------------------------

To test it using SSL/WSS, just run the test server with

```bash
$ libwebsockets-test-server --ssl
```

and use the URL

  https://127.0.0.1:7681

The connection will be entirely encrypted using some generated
certificates that your browser will not accept, since they are
not signed by any real Certificate Authority.  Just accept the
certificates in the browser and the connection will proceed
in first https and then websocket wss, acting exactly the
same.

[test-server.c](test-server/test-server.c) is all that is needed to use libwebsockets for
serving both the script html over http and websockets.


Testing websocket client support
--------------------------------

If you run the test server as described above, you can also
connect to it using the test client as well as a browser.

```bash
$ libwebsockets-test-client localhost
```

will by default connect to the test server on localhost:7681
and print the dumb increment number from the server at the
same time as drawing random circles in the mirror protocol;
if you connect to the test server using a browser at the
same time you will be able to see the circles being drawn.


Testing simple echo
-------------------

You can test against `echo.websockets.org` as a sanity test like
this (the client connects to port `80` by default):

```bash
$ libwebsockets-test-echo --client echo.websocket.org
```

This echo test is of limited use though because it doesn't
negotiate any protocol.  You can run the same test app as a
local server, by default on localhost:7681

```bash
$ libwebsockets-test-echo
```

and do the echo test against the local echo server

```bash
$ libwebsockets-test-echo --client localhost --port 7681
```

If you add the `--ssl` switch to both the client and server, you can also test
with an encrypted link.


Testing SSL on the client side
------------------------------

To test SSL/WSS client action, just run the client test with

```bash
$ libwebsockets-test-client localhost --ssl
```

By default the client test applet is set to accept selfsigned
certificates used by the test server, this is indicated by the
`use_ssl` var being set to `2`.  Set it to `1` to reject any server
certificate that it doesn't have a trusted CA cert for.


Using the websocket ping utility
--------------------------------

libwebsockets-test-ping connects as a client to a remote
websocket server using 04 protocol and pings it like the
normal unix ping utility.

```bash
$ libwebsockets-test-ping localhost
handshake OK for protocol lws-mirror-protocol
Websocket PING localhost.localdomain (127.0.0.1) 64 bytes of data.
64 bytes from localhost: req=1 time=0.1ms
64 bytes from localhost: req=2 time=0.1ms
64 bytes from localhost: req=3 time=0.1ms
64 bytes from localhost: req=4 time=0.2ms
64 bytes from localhost: req=5 time=0.1ms
64 bytes from localhost: req=6 time=0.2ms
64 bytes from localhost: req=7 time=0.2ms
64 bytes from localhost: req=8 time=0.1ms
^C
--- localhost.localdomain websocket ping statistics ---
8 packets transmitted, 8 received, 0% packet loss, time 7458ms
rtt min/avg/max = 0.110/0.185/0.218 ms
$
```

By default it sends 64 byte payload packets using the 04
PING packet opcode type.  You can change the payload size
using the `-s=` flag, up to a maximum of 125 mandated by the
04 standard.

Using the lws-mirror protocol that is provided by the test
server, libwebsockets-test-ping can also use larger payload
sizes up to 4096 is BINARY packets; lws-mirror will copy
them back to the client and they appear as a PONG.  Use the
`-m` flag to select this operation.

The default interval between pings is 1s, you can use the -i=
flag to set this, including fractions like `-i=0.01` for 10ms
interval.

Before you can even use the PING opcode that is part of the
standard, you must complete a handshake with a specified
protocol.  By default lws-mirror-protocol is used which is
supported by the test server.  But if you are using it on
another server, you can specify the protcol to handshake with
by `--protocol=protocolname`


Fraggle test app
----------------

By default it runs in server mode

```bash
$ libwebsockets-test-fraggle
libwebsockets test fraggle
(C) Copyright 2010-2011 Andy Green <andy@warmcat.com> licensed under LGPL2.1
 Compiled with SSL support, not using it
 Listening on port 7681
server sees client connect
accepted v06 connection
Spamming 360 random fragments
Spamming session over, len = 371913. sum = 0x2D3C0AE
Spamming 895 random fragments
Spamming session over, len = 875970. sum = 0x6A74DA1
...
```

You need to run a second session in client mode, you have to
give the `-c` switch and the server address at least:

```bash
$ libwebsockets-test-fraggle -c localhost
libwebsockets test fraggle
(C) Copyright 2010-2011 Andy Green <andy@warmcat.com> licensed under LGPL2.1
 Client mode
Connecting to localhost:7681
denied deflate-stream extension
handshake OK for protocol fraggle-protocol
client connects to server
EOM received 371913 correctly from 360 fragments
EOM received 875970 correctly from 895 fragments
EOM received 247140 correctly from 258 fragments
EOM received 695451 correctly from 692 fragments
...
```

The fraggle test sends a random number up to 1024 fragmented websocket frames
each of a random size between 1 and 2001 bytes in a single message, then sends
a checksum and starts sending a new randomly sized and fragmented message.

The fraggle test client receives the same message fragments and computes the
same checksum using websocket framing to see when the message has ended.  It
then accepts the server checksum message and compares that to its checksum.


proxy support
-------------

The http_proxy environment variable is respected by the client
connection code for both `ws://` and `wss://`.  It doesn't support
authentication.

You use it like this

```bash
$ export http_proxy=myproxy.com:3128
$ libwebsockets-test-client someserver.com
```


debug logging
-------------

By default logging of severity "notice", "warn" or "err" is enabled to stderr.

Again by default other logging is compiled in but disabled from printing.

If you want to eliminate the debug logging below notice  in severity, use the
`--disable-debug` configure option to have it removed from the code by the
preprocesser.

If you want to see more detailed debug logs, you can control a bitfield to
select which logs types may print using the `lws_set_log_level()` api, in the
test apps you can use `-d <number>` to control this.  The types of logging
available are (OR together the numbers to select multiple)

 - 1   ERR
 - 2   WARN
 - 4   NOTICE
 - 8   INFO
 - 16  DEBUG
 - 32  PARSER
 - 64  HEADER
 - 128 EXTENSION
 - 256 CLIENT
 - 512 LATENCY


Websocket version supported
---------------------------

The final IETF standard is supported for both client and server, protocol
version 13.


Latency Tracking
----------------

Since libwebsockets runs using `poll()` and a single threaded approach, any
unexpected latency coming from system calls would be bad news.  There's now
a latency tracking scheme that can be built in with `--with-latency` at
configure-time, logging the time taken for system calls to complete and if
the whole action did complete that time or was deferred.

You can see the detailed data by enabling logging level 512 (eg, `-d 519` on
the test server to see that and the usual logs), however even without that
the "worst" latency is kept and reported to the logs with NOTICE severity
when the context is destroyed.

Some care is needed interpreting them, if the action completed the first figure
(in us) is the time taken for the whole action, which may have retried through
the poll loop many times and will depend on network roundtrip times.  High
figures here don't indicate a problem.  The figure in us reported after "lat"
in the logging is the time taken by this particular attempt.  High figures
here may indicate a problem, or if you system is loaded with another app at
that time, such as the browser, it may simply indicate the OS gave preferential
treatment to the other app during that call.
