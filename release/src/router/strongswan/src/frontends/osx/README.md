# strongSwan OS X App #

## Introduction ##

The strongSwan OS X App consists of two components:

* A frontend App to configure and control connections (under strongSwan)
* A privileged helper daemon, controlled using XPC (under charon-xpc)

The privileged helper daemon gets installed automatically using SMJobBless
functionality on its first use, and gets started automatically by Launchd when
needed.

charon-xpc is a special build linking statically against strongSwan components.

charon-xpc and the App sources are currently not part of the official strongSwan
distribution. Build the charon-xpc tarball with:

    git archive -o osx-sources-$(grep AC_INIT configure.ac | \
                                cut -d '[' -f3 | cut -d ']' -f1).tar.bz2 \
        HEAD src/frontends/osx

## Building strongSwan ##

Before building the Xcode project, the strongSwan base tree must be built using
a monolithic and static build. This can be achieved on OS X by using:

    CFLAGS="-O2 -g -Wall -Wno-format -Wno-pointer-sign -Wno-deprecated-declarations" \
    ./configure --enable-monolithic --disable-shared --enable-static \
        --disable-defaults \
        --enable-openssl --enable-kernel-libipsec --enable-kernel-pfroute \
        --enable-eap-mschapv2 --enable-eap-identity --enable-eap-md5 \
        --enable-eap-gtc --enable-pkcs1 --enable-socket-default --enable-osx-attr \
        --enable-xauth-generic --enable-gcm --enable-ccm --enable-ctr \
        --enable-keychain --enable-nonce --enable-charon \
        --enable-ikev1 --enable-ikev2

followed by calling make (no need to make install).

Building charon-xpc using the Xcode project yields a single binary without
any non OS X dependencies. The strongSwan target in the same project builds
the App and integrates charon-xpc for the deployment.

Both charon-xpc and the App must be code-signed to allow the installation of
the privileged helper. By default both targets use the _Developer ID: *_
wildcard to use the first usable code signing identity. Both the App and
charon-xpc require a hardcoded certificate subject under
_strongSwan/strongSwan-Info.plist_ respectively
_charon-xpc/charon-xpc-Info.plist_. Update the _org.strongswan.charon-xpc_
_SMPrivilegedExecutables_ in the App and _SMAuthorizedClients_ in charon-xpc
with your code signing certificate identity.

## XPC application protocol ##

charon-xpc provides a Mach service under the name _org.strongswan.charon-xpc_.
Clients can connect to this service to control the daemon. All messages
on all connections use the following string dictionary keys/values:

* _type_: XPC message type, currently either
	* _rpc_ for a remote procedure call, expects a response
	* _event_ for application specific event messages
* _rpc_: defines the name of the RPC function to call (for _type_ = _rpc_)
* _event_: defines a name for the event (for _type_ = _event_)

Additional arguments and return values are specified by the call and can have
any type. Keys are directly attached to the message dictionary.

On the Mach service connection, the following RPC messages are currently
defined:

* string version = get_version()
	* _version_: strongSwan version of charon-xpc
* bool success = start_connection(string name, string host, string id,
								  endpoint channel)
	* _success_: TRUE if initiation started successfully
	* _name_: connection name to initiate
	* _host_: server hostname (and identity)
	* _id_: client identity to use
	* _channel_: XPC endpoint for this connection

The start_connection() RPC returns just after the initiation of the call and
does not wait for the connection to establish. Nonetheless does it have a
return value to indicate if connection initiation could be triggered.

The App passes an (anonymous) XPC endpoint to start_connection(). If the call
succeeds, charon-xpc connects to this endpoint to establish a channel used for
this specific IKE connection.

On this channel, the following RPC calls are currently defined from charon-xpc
to the App:

* string password = get_password(string username)
	* _password_: user password returned
	* _username_: username to query a password for

And the following from the App to charon-xpc:

* bool success = stop_connection()
	* _success_: TRUE if termination of connection initiated

The following events are currently defined from charon-xpc to the App:

* up(): IKE_SA has been established
* down(): IKE_SA has been closed or failed to establish
* child_up(string local_ts, string remote_ts): CHILD_SA has been established
* child_down(string local_ts, string remote_ts): CHILD_SA has been closed
* log(string message): debug log message for this connection
