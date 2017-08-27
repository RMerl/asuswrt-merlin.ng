Daemonization
-------------

There's a helper api `lws_daemonize` built by default that does everything you
need to daemonize well, including creating a lock file.  If you're making
what's basically a daemon, just call this early in your init to fork to a
headless background process and exit the starting process.

Notice stdout, stderr, stdin are all redirected to /dev/null to enforce your
daemon is headless, so you'll need to sort out alternative logging, by, eg,
syslog.


Maximum number of connections
-----------------------------

The maximum number of connections the library can deal with is decided when
it starts by querying the OS to find out how many file descriptors it is
allowed to open (1024 on Fedora for example).  It then allocates arrays that
allow up to that many connections, minus whatever other file descriptors are
in use by the user code.

If you want to restrict that allocation, or increase it, you can use ulimit or
similar to change the avaiable number of file descriptors, and when restarted
**libwebsockets** will adapt accordingly.


Libwebsockets is singlethreaded
-------------------------------

Directly performing websocket actions from other threads is not allowed.
Aside from the internal data being inconsistent in `forked()` processes,
the scope of a `wsi` (`struct websocket`) can end at any time during service
with the socket closing and the `wsi` freed.

Websocket write activities should only take place in the
`LWS_CALLBACK_SERVER_WRITEABLE` callback as described below.

Only live connections appear in the user callbacks, so this removes any
possibility of trying to used closed and freed wsis.

If you need to service other socket or file descriptors as well as the
websocket ones, you can combine them together with the websocket ones
in one poll loop, see "External Polling Loop support" below, and
still do it all in one thread / process context.

If you insist on trying to use it from multiple threads, take special care if
you might simultaneously create more than one context from different threads.

SSL_library_init() is called from the context create api and it also is not
reentrant.  So at least create the contexts sequentially.


Only send data when socket writeable
------------------------------------

You should only send data on a websocket connection from the user callback
`LWS_CALLBACK_SERVER_WRITEABLE` (or `LWS_CALLBACK_CLIENT_WRITEABLE` for
clients).

If you want to send something, do not just send it but request a callback
when the socket is writeable using

 - `lws_callback_on_writable(context, wsi)`` for a specific `wsi`, or
 - `lws_callback_on_writable_all_protocol(protocol)` for all connections
using that protocol to get a callback when next writeable.

Usually you will get called back immediately next time around the service
loop, but if your peer is slow or temporarily inactive the callback will be
delayed accordingly.  Generating what to write and sending it should be done
in the ...WRITEABLE callback.

See the test server code for an example of how to do this.


Do not rely on only your own WRITEABLE requests appearing
---------------------------------------------------------

Libwebsockets may generate additional `LWS_CALLBACK_CLIENT_WRITEABLE` events
if it met network conditions where it had to buffer your send data internally.

So your code for `LWS_CALLBACK_CLIENT_WRITEABLE` needs to own the decision
about what to send, it can't assume that just because the writeable callback
came it really is time to send something.

It's quite possible you get an 'extra' writeable callback at any time and
just need to `return 0` and wait for the expected callback later.


Closing connections from the user side
--------------------------------------

When you want to close a connection, you do it by returning `-1` from a
callback for that connection.

You can provoke a callback by calling `lws_callback_on_writable` on
the wsi, then notice in the callback you want to close it and just return -1.
But usually, the decision to close is made in a callback already and returning
-1 is simple.

If the socket knows the connection is dead, because the peer closed or there
was an affirmitive network error like a FIN coming, then **libwebsockets**  will
take care of closing the connection automatically.

If you have a silently dead connection, it's possible to enter a state where
the send pipe on the connection is choked but no ack will ever come, so the
dead connection will never become writeable.  To cover that, you can use TCP
keepalives (see later in this document)


Fragmented messages
-------------------

To support fragmented messages you need to check for the final
frame of a message with `lws_is_final_fragment`. This
check can be combined with `libwebsockets_remaining_packet_payload`
to gather the whole contents of a message, eg:

```
    case LWS_CALLBACK_RECEIVE:
    {
        Client * const client = (Client *)user;
        const size_t remaining = lws_remaining_packet_payload(wsi);

        if (!remaining && lws_is_final_fragment(wsi)) {
            if (client->HasFragments()) {
                client->AppendMessageFragment(in, len, 0);
                in = (void *)client->GetMessage();
                len = client->GetMessageLength();
            }

            client->ProcessMessage((char *)in, len, wsi);
            client->ResetMessage();
        } else
            client->AppendMessageFragment(in, len, remaining);
    }
    break;
```

The test app libwebsockets-test-fraggle sources also show how to
deal with fragmented messages.


Debug Logging
-------------

Also using `lws_set_log_level` api you may provide a custom callback to actually
emit the log string.  By default, this points to an internal emit function
that sends to stderr.  Setting it to `NULL` leaves it as it is instead.

A helper function `lwsl_emit_syslog()` is exported from the library to simplify
logging to syslog.  You still need to use `setlogmask`, `openlog` and `closelog`
in your user code.

The logging apis are made available for user code.

- `lwsl_err(...)`
- `lwsl_warn(...)`
- `lwsl_notice(...)`
- `lwsl_info(...)`
- `lwsl_debug(...)`

The difference between notice and info is that notice will be logged by default
whereas info is ignored by default.


External Polling Loop support
-----------------------------

**libwebsockets** maintains an internal `poll()` array for all of its
sockets, but you can instead integrate the sockets into an
external polling array.  That's needed if **libwebsockets** will
cooperate with an existing poll array maintained by another
server.

Four callbacks `LWS_CALLBACK_ADD_POLL_FD`, `LWS_CALLBACK_DEL_POLL_FD`,
`LWS_CALLBACK_SET_MODE_POLL_FD` and `LWS_CALLBACK_CLEAR_MODE_POLL_FD`
appear in the callback for protocol 0 and allow interface code to
manage socket descriptors in other poll loops.

You can pass all pollfds that need service to `lws_service_fd()`, even
if the socket or file does not belong to **libwebsockets** it is safe.

If **libwebsocket** handled it, it zeros the pollfd `revents` field before returning.
So you can let **libwebsockets** try and if `pollfd->revents` is nonzero on return,
you know it needs handling by your code.


Using with in c++ apps
----------------------

The library is ready for use by C++ apps.  You can get started quickly by
copying the test server

```bash
$ cp test-server/test-server.c test.cpp
```

and building it in C++ like this

```bash
$ g++ -DINSTALL_DATADIR=\"/usr/share\" -ocpptest test.cpp -lwebsockets
```

`INSTALL_DATADIR` is only needed because the test server uses it as shipped, if
you remove the references to it in your app you don't need to define it on
the g++ line either.


Availability of header information
----------------------------------

From v1.2 of the library onwards, the HTTP header content is `free()`d as soon
as the websocket connection is established.  For websocket servers, you can
copy interesting headers by handling `LWS_CALLBACK_FILTER_PROTOCOL_CONNECTION`
callback, for clients there's a new callback just for this purpose
`LWS_CALLBACK_CLIENT_FILTER_PRE_ESTABLISH`.


TCP Keepalive
-------------

It is possible for a connection which is not being used to send to die
silently somewhere between the peer and the side not sending.  In this case
by default TCP will just not report anything and you will never get any more
incoming data or sign the link is dead until you try to send.

To deal with getting a notification of that situation, you can choose to
enable TCP keepalives on all **libwebsockets** sockets, when you create the
context.

To enable keepalive, set the ka_time member of the context creation parameter
struct to a nonzero value (in seconds) at context creation time.  You should
also fill ka_probes and ka_interval in that case.

With keepalive enabled, the TCP layer will send control packets that should
stimulate a response from the peer without affecting link traffic.  If the
response is not coming, the socket will announce an error at `poll()` forcing
a close.

Note that BSDs don't support keepalive time / probes / interval per-socket
like Linux does.  On those systems you can enable keepalive by a nonzero
value in `ka_time`, but the systemwide kernel settings for the time / probes/
interval are used, regardless of what nonzero value is in `ka_time`.

Optimizing SSL connections
--------------------------

There's a member `ssl_cipher_list` in the `lws_context_creation_info` struct
which allows the user code to restrict the possible cipher selection at
context-creation time.

You might want to look into that to stop the ssl peers selecting a cipher which
is too computationally expensive.  To use it, point it to a string like

`"RC4-MD5:RC4-SHA:AES128-SHA:AES256-SHA:HIGH:!DSS:!aNULL"`

if left `NULL`, then the "DEFAULT" set of ciphers are all possible to select.


Async nature of client connections
----------------------------------

When you call `lws_client_connect(..)` and get a `wsi` back, it does not
mean your connection is active.  It just mean it started trying to connect.

Your client connection is actually active only when you receive
`LWS_CALLBACK_CLIENT_ESTABLISHED` for it.

There's a 5 second timeout for the connection, and it may give up or die for
other reasons, if any of that happens you'll get a
`LWS_CALLBACK_CLIENT_CONNECTION_ERROR` callback on protocol 0 instead for the
`wsi`.

After attempting the connection and getting back a non-`NULL` `wsi` you should
loop calling `lws_service()` until one of the above callbacks occurs.

As usual, see [test-client.c](test-server/test-client.c) for example code.

Lws platform-independent file access apis
-----------------------------------------

lws now exposes his internal platform file abstraction in a way that can be
both used by user code to make it platform-agnostic, and be overridden or
subclassed by user code.  This allows things like handling the URI "directory
space" as a virtual filesystem that may or may not be backed by a regular
filesystem.  One example use is serving files from inside large compressed
archive storage without having to unpack anything except the file being
requested.

The test server shows how to use it, basically the platform-specific part of
lws prepares a file operations structure that lives in the lws context.

The user code can get a pointer to the file operations struct

LWS_VISIBLE LWS_EXTERN struct lws_plat_file_ops *
`lws_get_fops`(struct lws_context *context);

and then can use helpers to also leverage these platform-independent
file handling apis

static inline lws_filefd_type
`lws_plat_file_open`(struct lws *wsi, const char *filename, unsigned long *filelen, int flags)

static inline int
`lws_plat_file_close`(struct lws *wsi, lws_filefd_type fd)

static inline unsigned long
`lws_plat_file_seek_cur`(struct lws *wsi, lws_filefd_type fd, long offset_from_cur_pos)

static inline int
`lws_plat_file_read`(struct lws *wsi, lws_filefd_type fd, unsigned long *amount, unsigned char *buf, unsigned long len)

static inline int
`lws_plat_file_write`(struct lws *wsi, lws_filefd_type fd, unsigned long *amount, unsigned char *buf, unsigned long len)
		    
The user code can also override or subclass the file operations, to either
wrap or replace them.  An example is shown in test server.
