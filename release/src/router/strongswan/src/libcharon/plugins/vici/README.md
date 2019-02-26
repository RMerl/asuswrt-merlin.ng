# The Versatile IKE Control Interface (VICI) protocol #

The vici _[ˈvitʃi]_ plugin implements the server side of an IPC protocol to
configure, monitor and control the IKE daemon charon. It uses request/response
and event messages to communicate over a reliable stream based transport.

## Transport protocol ##

To provide the service, the plugin opens a listening socket using a reliable,
stream based transport. charon relies on the different stream service
abstractions provided by libstrongswan, such as TCP and UNIX sockets.

A client connects to this service to access functionality. It may send an
arbitrary number of packets over the connection before closing it.

To exchange data, the transport protocol is segmented into byte sequences.
Each byte sequence is prefixed by a 32-bit length header in network order,
followed by the data. The maximum segment length is currently limited to 512KB
of data, and the length field contains the length of the data only, not
including the length field itself.

The order of byte sequences must be strict, byte sequences must arrive in the
same order as sent.

## Packet layer ##

Within the byte sequences defined by the transport layer, both the client
and the server can exchange packets. The type of packet defines its structure
and purpose. The packet type is a 8-bit identifier, and is the first byte
in a transport layer byte sequence. The length of the packet is given by the
transport layer.

While a packet type may define the format of the wrapped data freely, currently
all types either contain a name, a message or both. The following packet types
are currently defined:

* _CMD_REQUEST = 0_: A named request message
* _CMD_RESPONSE = 1_: An unnamed response message for a request
* _CMD_UNKNOWN = 2_: An unnamed response if requested command is unknown
* _EVENT_REGISTER = 3_: A named event registration request
* _EVENT_UNREGISTER = 4_: A named event deregistration request
* _EVENT_CONFIRM = 5_: An unnamed response for successful event (de-)registration
* _EVENT_UNKNOWN = 6_: A unnamed response if event (de-)registration failed
* _EVENT = 7_: A named event message

For packets having a named type, after the packet type an 8-bit length header
of the name follows, indicating the string length in bytes of the name tag, not
including the length field itself. The name is an ASCII string that is not
null-terminated.

The rest of the packet forms the exchanged message, the length is determined
by the transport byte sequence length, subtracting the packet type and
the optional name tag in some messages.

### Commands ###

Commands are currently always requested by the client. The server replies with
a response, or with a CMD_UNKNOWN failure message to let the client know
that it does not have a handler for such a command. There is no sequence number
to associate responses to requests, so only one command can be active at
a time on a single connection.

### Events ###

To receive event messages, the client explicitly registers for events by name,
and also unregisters if it does not want to receive events of the named kind
anymore. The server confirms event registration using EVENT_CONFIRM, or
indicates that there is no such event source with EVENT_UNKNOWN.

Events may get raised at any time while registered, even during an active
request command. This mechanism is used to feed continuous data during a request,
for example.

## Message format ##

The defined packet types optionally wrap a message with additional data.
Messages are currently used in CMD_REQUEST/CMD_RESPONSE, and in EVENT packets.
A message uses a hierarchical tree of sections. Each section (or the implicit
root section) contains an arbitrary set of key/value pairs, lists and
sub-sections. The length of a message is not part of the message itself, but
the wrapping layer, usually calculated from the transport byte sequence length.

The message encoding consists of a sequence of elements. Each element starts
with the element type, optionally followed by an element name and/or an element
value. Currently the following message element types are defined:

* _SECTION_START = 1_: Begin a new section having a name
* _SECTION_END = 2_: End a previously started section
* _KEY_VALUE = 3_: Define a value for a named key in the current section
* _LIST_START = 4_: Begin a named list for list items
* _LIST_ITEM = 5_: Define an unnamed item value in the current list
* _LIST_END = 6_: End a previously started list

Types are encoded as 8-bit values. Types having a name (SECTION_START,
KEY_VALUE and LIST_START) have an ASCII string following the type, which itself
uses an 8-bit length header. The string must not be null-terminated, the string
length does not include the length field itself.

Types having a value (KEY_VALUE and LIST_ITEM) have a raw blob sequence,
prefixed with a 16-bit network order length. The blob follows the type or the
name tag if available, the length defined by the length field does not include
the length field itself.

The interpretation of any value is not defined by the message format; it can
take arbitrary blobs. The application may specify types for specific keys, such
as strings or integer representations. The vici plugin currently uses
non-null terminated strings as values only; numbers get encoded as strings.

### Sections ###

Sections may be opened in the implicit root section, or any previously section.
They can be nested to arbitrary levels. A SECTION_END marker always closes
the last opened section; SECTION_START and SECTION_END items must be balanced
in a valid message.

### Key/Values ###

Key/Value pair elements may appear in the implicit root section or any explicit
sub-section at any level. Key names must be unique in the current section, use
lists to define multiple values for a key. Key/values may not appear in lists,
use a sub-section instead.

### Lists ###

Lists may appear at the same locations as Key/Values, and may not be nested.
Only a single list may be opened at the same time, and all lists must be closed
in valid messages. After opening a list, only list items may appear before the
list closing element. Empty lists are allowed, list items may appear within
lists only.

### Encoding example ###

Consider the following structure using pseudo-markup for this example:

	key1 = value1
	section1 = {
		sub-section = {
			key2 = value2
		}
		list1 = [ item1, item2 ]
	}

The example above represents a valid tree structure, that gets encoded as
the following C array:

	char msg[] = {
		/* key1 = value1 */
		3, 4,'k','e','y','1', 0,6,'v','a','l','u','e','1',
		/* section1 */
		1, 8,'s','e','c','t','i','o','n','1',
		/* sub-section */
		1, 11,'s','u','b','-','s','e','c','t','i','o','n',
		/* key2 = value2 */
		3, 4,'k','e','y','2', 0,6,'v','a','l','u','e','2',
		/* sub-section end */
		2,
		/* list1 */
		4, 5, 'l','i','s','t','1',
		/* item1 */
		5, 0,5,'i','t','e','m','1',
		/* item2 */
		5, 0,5,'i','t','e','m','2',
		/* list1 end */
		6,
		/* section1 end */
		2,
	};

## Client-initiated commands ##

Based on the packet layer, VICI implements commands requested by the client
and responded to by the server using named _CMD_REQUEST_ and _CMD_RESPONSE_
packets wrapping messages. The request message may contain command arguments,
the response message the reply.

Some commands use response streaming, that is, a request triggers a series of
events to consecutively stream data to the client before the response message
completes the stream. A client must register for the appropriate event to
receive the stream, and unregister after the response has been received.

The following client issued commands with the appropriate command input and
output messages are currently defined:

### version() ###

Returns daemon and system specific version information.

	{} => {
		daemon = <IKE daemon name>
		version = <strongSwan version>
		sysname = <operating system name>
		release = <operating system release>
		machine = <hardware identifier>
	}

### stats() ###

Returns IKE daemon statistics and load information.

	{} => {
		uptime = {
			running = <relative uptime in human-readable form>
			since = <absolute startup time>
		}
		workers = {
			total = <total number of worker threads>
			idle = <worker threads currently idle>
			active = {
				critical = <threads processing "critical" priority jobs>
				high = <threads processing "high" priority jobs>
				medium = <threads processing "medium" priority jobs>
				low = <threads processing "low" priority jobs>
			}
		}
		queues = {
			critical = <jobs queued with "critical" priority>
			high = <jobs queued with "high" priority>
			medium = <jobs queued with "medium" priority>
			low = <jobs queued with "low" priority>
		}
		scheduled = <number of jobs scheduled for timed execution>
		ikesas = {
			total = <total number of IKE_SAs active>
			half-open = <number of IKE_SAs in half-open state>
		}
		plugins = [
			<names of loaded plugins>
		]
		mem = { # available if built with leak-detective or on Windows
			total = <total heap memory usage in bytes>
			allocs = <total heap allocation blocks>
			<heap-name>* = { # on Windows only
				total = <heap memory usage in bytes by this heap>
				allocs = <allocated blocks for this heap>
			}
		}
		mallinfo = { # available with mallinfo() support
			sbrk = <non-mmaped space available>
			mmap = <mmaped space available>
			used = <total number of bytes used>
			free = <available but unused bytes>
		}
	}

### reload-settings() ###

Reloads _strongswan.conf_ settings and all plugins supporting configuration
reload.

	{} => {
		success = <yes or no>
		errmsg = <error string on failure>
	}

### initiate() ###

Initiates an SA while streaming _control-log_ events.

	{
		child = <CHILD_SA configuration name to initiate>
		ike = <optional IKE_SA configuration name to find child under>
		timeout = <timeout in ms before returning>
		init-limits = <whether limits may prevent initiating the CHILD_SA>
		loglevel = <loglevel to issue "control-log" events for>
	} => {
		success = <yes or no>
		errmsg = <error string on failure or timeout>
	}

The default timeout of 0 waits indefinitely for a result, and a timeout value
of -1 returns a result immediately.

### terminate() ###

Terminates an SA while streaming _control-log_ events.

	{
		child = <terminate a CHILD_SA by configuration name>
		ike = <terminate an IKE_SA by configuration name>
		child-id = <terminate a CHILD_SA by its reqid>
		ike-id = <terminate an IKE_SA by its unique id>
		force = <terminate IKE_SA without waiting for proper DELETE, if timeout
				 is given, waits for a response until it is reached>
		timeout = <timeout in ms before returning, see below>
		loglevel = <loglevel to issue "control-log" events for>
	} => {
		success = <yes or no>
		matches = <number of matched SAs>
		terminated = <number of terminated SAs>
		errmsg = <error string on failure or timeout>
	}

The default timeout of 0 waits indefinitely for a result, and a timeout value
of -1 returns a result immediately.

### rekey() ###

Initiate the rekeying of an SA.

	{
		child = <rekey a CHILD_SA by configuration name>
		ike = <rekey an IKE_SA by configuration name>
		child-id = <rekey a CHILD_SA by its reqid>
		ike-id = <rekey an IKE_SA by its unique id>
		reauth = <reauthenticate instead of rekey an IKEv2 SA>
	} => {
		success = <yes or no>
		matches = <number of matched SAs>
		errmsg = <error string on failure>
	}

### redirect() ###

Redirect a client-initiated IKE_SA to another gateway.  Only for IKEv2 and if
supported by the peer.

	{
		ike = <redirect an IKE_SA by configuration name>
		ike-id = <redirect an IKE_SA by its unique id>
		peer-ip = <redirect an IKE_SA with matching peer IP, may also be a
				   subnet in CIDR notation or an IP range>
		peer-id = <redirect an IKE_SA with matching peer identity, may contain
				   wildcards>
	} => {
		success = <yes or no>
		matches = <number of matched SAs>
		errmsg = <error string on failure>
	}

### install() ###

Install a trap, drop or bypass policy defined by a CHILD_SA config.

	{
		child = <CHILD_SA configuration name to install>
		ike = <optional IKE_SA configuration name to find child under>
	} => {
		success = <yes or no>
		errmsg = <error string on failure>
	}

### uninstall() ###

Uninstall a trap, drop or bypass policy defined by a CHILD_SA config.

	{
		child = <CHILD_SA configuration name to install>
		ike = <optional IKE_SA configuration name to find child under,
			   if not given the first policy matching child is removed>
	} => {
		success = <yes or no>
		errmsg = <error string on failure>
	}

### list-sas() ###

Lists currently active IKE_SAs and associated CHILD_SAs by streaming _list-sa_
events.

	{
		noblock = <use non-blocking mode if key is set>
		ike = <filter listed IKE_SAs by its name>
		ike-id = <filter listed IKE_SA by its unique id>
	} => {
		# completes after streaming list-sa events
	}

### list-policies() ###

List currently installed trap, drop and bypass policies by streaming
_list-policy_ events.

	{
		drop = <set to yes to list drop policies>
		pass = <set to yes to list bypass policies>
		trap = <set to yes to list trap policies>
		child = <filter by CHILD_SA configuration name>
		ike = <filter by IKE_SA configuration name>
	} => {
		# completes after streaming list-sa events
	}

### list-conns() ###

List currently loaded connections by streaming _list-conn_ events. This
call includes all connections known by the daemon, not only those loaded
over vici.

	{
		ike = <list connections matching a given configuration name only>
	} => {
		# completes after streaming list-conn events
	}

### get-conns() ###

Return a list of connection names loaded exclusively over vici, not including
connections found in other backends.

	{} => {
		conns = [
			<list of connection names>
		]
	}

### list-certs() ###

List currently loaded certificates by streaming _list-cert_ events. This
call includes all certificates known by the daemon, not only those loaded
over vici.

	{
		type = <certificate type to filter for, X509|X509_AC|X509_CRL|
												OCSP_RESPONSE|PUBKEY  or ANY>
		flag = <X.509 certificate flag to filter for, NONE|CA|AA|OCSP or ANY>
		subject = <set to list only certificates having subject>
	} => {
		# completes after streaming list-cert events
	}

### list-authorities() ###

List currently loaded certification authority information by streaming
_list-authority_ events.

	{
		name = <list certification authority of a given name>
	} => {
		# completes after streaming list-authority events
	}

### get-authorities() ###

Return a list of currently loaded certification authority names.

	{} => {
		authorities = [
			<list of certification authority names>
		]
	}

### load-conn() ###

Load a single connection definition into the daemon. An existing connection
with the same name gets updated or replaced.

	{
		<IKE_SA config name> = {
			# IKE configuration parameters with authentication and CHILD_SA
			# subsections. Refer to swanctl.conf(5) for details.
		} => {
			success = <yes or no>
			errmsg = <error string on failure>
		}
	}

### unload-conn() ###

Unload a previously loaded connection definition by name.

	{
		name = <IKE_SA config name>
	} => {
		success = <yes or no>
		errmsg = <error string on failure>
	}

### load-cert() ###

Load a certificate into the daemon.

	{
		type = <certificate type, X509|X509_AC|X509_CRL>
		flag = <X.509 certificate flag, NONE|CA|AA|OCSP>
		data = <PEM or DER encoded certificate data>
	} => {
		success = <yes or no>
		errmsg = <error string on failure>
	}

### load-key() ###

Load a private key into the daemon.

	{
		type = <private key type, rsa|ecdsa|bliss|any>
		data = <PEM or DER encoded key data>
	} => {
		success = <yes or no>
		errmsg = <error string on failure>
		id = <hex-encoded SHA-1 key identifier of the public key on success>
	}

### unload-key() ###

Unload the private key with the given key identifier.

	{
		id = <hex-encoded SHA-1 key identifier of the private key to unload>
	} => {
		success = <yes or no>
		errmsg = <error string on failure>
	}

### get-keys() ###

Return a list of identifiers of private keys loaded exclusively over vici, not
including keys found in other backends.

	{} => {
		keys = [
			<list of hex-encoded SHA-1 key identifiers>
		]
	}

### load-token() ###

Load a private key located on a token into the daemon.  Such keys may be listed
and unloaded using the _get-keys_ and _unload-key_ commands, respectively (based
on the key identifier derived from the public key).

	{
		handle = <hex-encoded CKA_ID of the private key on token>
		slot = <optional slot number>
		module = <optional PKCS#11 module>
		pin = <optional PIN to access the key, has to be provided via other
			   means if not given>
	} => {
		success = <yes or no>
		errmsg = <error string on failure>
		id = <hex-encoded SHA-1 key identifier of the public key on success>
	}

### load-shared() ###

Load a shared IKE PSK, EAP, XAuth or NTLM secret into the daemon.

	{
		id = <optional unique identifier of this shared key>
		type = <shared key type, IKE|EAP|XAUTH|NTLM>
		data = <raw shared key data>
		owners = [
			<list of shared key owner identities>
		]
	} => {
		success = <yes or no>
		errmsg = <error string on failure>
	}

### unload-shared() ###

Unload a previously loaded shared IKE PSK, EAP, XAuth or NTLM secret by its
unique identifier.

	{
		id = <unique identifier of the shared key to unload>
	} => {
		success = <yes or no>
		errmsg = <error string on failure>
	}

### get-shared() ###

Return a list of unique identifiers of shared keys loaded exclusively over vici,
not including keys found in other backends.

	{} => {
		keys = [
			<list of unique identifiers>
		]
	}

### flush-certs() ###

Flushes the certificate cache. The optional type argument allows to flush
only certificates of a given type, e.g. all cached CRLs.

	{
		type = <certificate type to filter for, X509|X509_AC|X509_CRL|
												OCSP_RESPONSE|PUBKEY or ANY>
	} => {
		success = <yes or no>
		errmsg = <error string on failure>
	}

### clear-creds() ###

Clear all loaded certificate, private key and shared key credentials. This
affects only credentials loaded over vici, but additionally flushes the
credential cache.

	{} => {
		success = <yes or no>
		errmsg = <error string on failure>
	}

### load-authority() ###

Load a single certification authority definition into the daemon. An existing
authority with the same name gets replaced.

	{
		<certification authority name> = {
			# certification authority parameters
			# refer to swanctl.conf(5) for details.
		} => {
			success = <yes or no>
			errmsg = <error string on failure>
		}
	}

### unload-authority() ###

Unload a previously loaded certification authority definition by name.

	{
		name = <certification authority name>
	} => {
		success = <yes or no>
		errmsg = <error string on failure>
	}

### load-pool() ###

Load an in-memory virtual IP and configuration attribute pool. Existing
pools with the same name get updated, if possible.

	{
		<pool name> = {
			addrs = <subnet of virtual IP pool addresses>
			<attribute type>* = [
				# attribute type is one of address, dns, nbns, dhcp, netmask,
				# server, subnet, split_include, split_exclude or a numerical
				# attribute type identifier.
				<list of attributes for type>
			]
		}
	} => {
		success = <yes or no>
		errmsg = <error string on failure>
	}

### unload-pool() ###

Unload a previously loaded virtual IP and configuration attribute pool.
Unloading fails for pools with leases currently online.

	{
		name = <virtual IP address pool to delete>
	} => {
		success = <yes or no>
		errmsg = <error string on failure>
	}

### get-pools() ###

List the currently loaded pools.

	{
		leases = <set to yes to include leases>
		name = <optional name of the pool to query>
	} => {
		<pool name>* = {
			base = <virtual IP pool base address>
			size = <total number of addresses in the pool>
			online = <number of leases online>
			offline = <number of leases offline>
			leases = {
				<zero-based index>* = {
					address = <IP address>
					identity = <assigned identity>
					status = <online|offline>
				}
			}
		}
	}

### get-algorithms() ###

List currently loaded algorithms and their implementation.

	{} => {
		<algorithm type> = {
			<algorithm> = <plugin providing the implementation>
		}
	}

### get-counters() ###

List global or connection-specific counters for several IKE events.

	{
		name = <optional connection name, omit for global counters>
		all = <yes to get counters for all connections, name is ignored>
	} => {
		counters = {
			<name|empty for global counters> = {
				<pairs of counter name and 64-bit counter value>
			}
		}
		success = <yes or no>
		errmsg = <error string on failure>
	}

### reset-counters() ###

Reset global or connection-specific IKE event counters.

	{
		name = <optional connection name, omit for global counters>
		all = <yes to reset counters for all connections, name is ignored>
	} => {
		success = <yes or no>
		errmsg = <error string on failure>
	}

## Server-issued events ##

Based on the packet layer, the vici plugin raises event messages using named
EVENT packets wrapping messages. The message contains event details.

### log ###

The _log_ event is issued to registered clients for each debug log message.
This event is not associated with a command.

	{
		group = <subsystem identifier for debug message>
		level = <log level, 0-4>
		thread = <numerical thread identifier issuing the log message>
		ikesa-name = <name of IKE_SA, if log is associated with any>
		ikesa-uniqued = <unique identifier of IKE_A, if log associated with any>
		msg = <log message text>
	}

### control-log ###

The _control-log_ event is issued for log events during active _initiate_ or
_terminate_ commands. It is issued only to clients currently having such
a command active.

	{
		group = <subsystem identifier for debug message>
		level = <log level, 0-4>
		ikesa-name = <name of IKE_SA, if log associated with any>
		ikesa-uniqued = <unique identifier of IKE_A, if log associated with any>
		msg = <log message text>
	}

### list-sa ###

The _list-sa_ event is issued to stream IKE_SAs during an active _list-sas_
command.

	{
		<IKE_SA config name> = {
			uniqueid = <IKE_SA unique identifier>
			version = <IKE version, 1 or 2>
			state = <IKE_SA state name>
			local-host = <local IKE endpoint address>
			local-port = <local IKE endpoint port>
			local-id = <local IKE identity>
			remote-host = <remote IKE endpoint address>
			remote-port = <remote IKE endpoint port>
			remote-id = <remote IKE identity>
			remote-xauth-id = <remote XAuth identity, if XAuth-authenticated>
			remote-eap-id = <remote EAP identity, if EAP-authenticated>
			initiator = <yes, if initiator of IKE_SA>
			initiator-spi = <hex encoded initiator SPI / cookie>
			responder-spi = <hex encoded responder SPI / cookie>
			nat-local = <yes, if local endpoint is behind a NAT>
			nat-remote = <yes, if remote endpoint is behind a NAT>
			nat-fake = <yes, if NAT situation has been faked as responder>
			nat-any = <yes, if any endpoint is behind a NAT (also if faked)>
			encr-alg = <IKE encryption algorithm string>
			encr-keysize = <key size for encr-alg, if applicable>
			integ-alg = <IKE integrity algorithm string>
			integ-keysize = <key size for encr-alg, if applicable>
			prf-alg = <IKE pseudo random function string>
			dh-group = <IKE Diffie-Hellman group string>
			established = <seconds the IKE_SA has been established>
			rekey-time = <seconds before IKE_SA gets rekeyed>
			reauth-time = <seconds before IKE_SA gets re-authenticated>
			local-vips = [
				<list of virtual IPs assigned by the remote peer, installed locally>
			]
			remote-vips = [
				<list of virtual IPs assigned to the remote peer>
			]
			tasks-queued = [
				<list of currently queued tasks for execution>
			]
			tasks-active = [
				<list of tasks currently initiating actively>
			]
			tasks-passive = [
				<list of tasks currently handling passively>
			]
			child-sas = {
				<unique child-sa-name>* = {
					name = <name of the CHILD_SA>
					uniqueid = <unique CHILD_SA identifier>
					reqid = <reqid of CHILD_SA>
					state = <state string of CHILD_SA>
					mode = <IPsec mode, tunnel|transport|beet>
					protocol = <IPsec protocol AH|ESP>
					encap = <yes if using UDP encapsulation>
					spi-in = <hex encoded inbound SPI>
					spi-out = <hex encoded outbound SPI>
					cpi-in = <hex encoded inbound CPI, if using compression>
					cpi-out = <hex encoded outbound CPI, if using compression>
					mark-in = <hex encoded inbound Netfilter mark value>
					mark-mask-in = <hex encoded inbound Netfilter mark mask>
					mark-out = <hex encoded outbound Netfilter mark value>
					mark-mask-out = <hex encoded outbound Netfilter mark mask>
					encr-alg = <ESP encryption algorithm name, if any>
					encr-keysize = <ESP encryption key size, if applicable>
					integ-alg = <ESP or AH integrity algorithm name, if any>
					integ-keysize = <ESP or AH integrity key size, if applicable>
					prf-alg = <CHILD_SA pseudo random function name>
					dh-group = <CHILD_SA PFS rekeying DH group name, if any>
					esn = <1 if using extended sequence numbers>
					bytes-in = <number of input bytes processed>
					packets-in = <number of input packets processed>
					use-in = <seconds since last inbound packet, if any>
					bytes-out = <number of output bytes processed>
					packets-out = <number of output packets processed>
					use-out = <seconds since last outbound packet, if any>
					rekey-time = <seconds before CHILD_SA gets rekeyed>
					life-time = <seconds before CHILD_SA expires>
					install-time = <seconds the CHILD_SA has been installed>
					local-ts = [
						<list of local traffic selectors>
					]
					remote-ts = [
						<list of remote traffic selectors>
					]
				}
			}
		}
	}

### list-policy ###

The _list-policy_ event is issued to stream installed policies during an active
_list-policies_ command.

	{
		<ike-sa-config-name/child-sa-config-name> = {
			child = <CHILD_SA configuration name>
			ike = <IKE_SA configuration name or namespace, if available>
			mode = <policy mode, tunnel|transport|pass|drop>
			local-ts = [
				<list of local traffic selectors>
			]
			remote-ts = [
				<list of remote traffic selectors>
			]
		}
	}

### list-conn ###

The _list-conn_ event is issued to stream loaded connection during an active
_list-conns_ command.

	{
		<IKE_SA connection name> = {
			local_addrs = [
				<list of valid local IKE endpoint addresses>
			]
			remote_addrs = [
				<list of valid remote IKE endpoint addresses>
			]
			version = <IKE version as string, IKEv1|IKEv2 or 0 for any>
			reauth_time = <IKE_SA reauthentication interval in seconds>
			rekey_time = <IKE_SA rekeying interval in seconds>

			local*, remote* = { # multiple local and remote auth sections
				class = <authentication type>
				eap-type = <EAP type to authenticate if when using EAP>
				eap-vendor = <EAP vendor for type, if any>
				xauth = <xauth backend name>
				revocation = <revocation policy>
				id = <IKE identity>
				aaa_id = <AAA authentication backend identity>
				eap_id = <EAP identity for authentication>
				xauth_id = <XAuth username for authentication>
				groups = [
					<group membership required to use connection>
				]
				certs = [
					<certificates allowed for authentication>
				]
				cacerts = [
					<CA certificates allowed for authentication>
				]
			}
			children = {
				<CHILD_SA config name>* = {
					mode = <IPsec mode>
					rekey_time = <CHILD_SA rekeying interval in seconds>
					rekey_bytes = <CHILD_SA rekeying interval in bytes>
					rekey_packets = <CHILD_SA rekeying interval in packets>
					local-ts = [
						<list of local traffic selectors>
					]
					remote-ts = [
						<list of remote traffic selectors>
					]
				}
			}
		}
	}

### list-cert ###

The _list-cert_ event is issued to stream loaded certificates during an active
_list-certs_ command.

	{
		type = <certificate type, X509|X509_AC|X509_CRL|OCSP_RESPONSE|PUBKEY>
		flag = <X.509 certificate flag, NONE|CA|AA|OCSP>
		has_privkey = <set if a private key for the certificate is available>
		data = <ASN1 encoded certificate data>
		subject = <subject string if defined and certificate type is PUBKEY>
		not-before = <time string if defined and certificate type is PUBKEY>
		not-after  = <time string if defined and certificate type is PUBKEY>
	}

### list-authority ###

The _list-authority_ event is issued to stream loaded certification authority
information during an active_list-authorities_ command.

	{
		<certification authority name> = {
			cacert = <subject distinguished name of CA certificate>
			crl_uris = [
				<CRL URI (http, ldap or file)>
			]
			ocsp_uris = [
				<OCSP URI (http)>
			]
			cert_uri_base = <base URI for download of hash-and-URL certificates>
		}
	}

### ike-updown ###

The _ike-updown_ event is issued when an IKE_SA is established or terminated.

	{
		up = <set if up event>
		<IKE_SA config name> = {
			<same data as in the list-sas event, but without child-sas section>
		}
	}

### ike-rekey ###

The _ike-rekey_ event is issued when an IKE_SA is rekeyed.

	{
		<IKE_SA config name> = {
			old = {
				<same data as in the list-sas event, but without child-sas section>
			}
			new = {
				<same data as in the list-sas event, but without child-sas section>
			}
		}
	}

### child-updown ###

The _child-updown_ event is issued when a CHILD_SA is established or terminated.

	{
		up = <set if up event>
		<IKE_SA config name> = {
			<same data as in the list-sas event, but with only the affected
			 CHILD_SA in the child-sas section>
		}
	}

### child-rekey ###

The _child-rekey_ event is issued when a CHILD_SA is rekeyed.

	{
		<IKE_SA config name> = {
			<same data as in the list-sas event, but with the child-sas section
			 as follows>
			child-sas = {
				<child-sa-name> = {
					old = {
						<same data as in the list-sas event>
					}
					new = {
						<same data as in the list-sas event>
					}
				}
			}
		}
	}

# libvici C client library #

libvici is the reference implementation of a C client library implementing
the vici protocol. It builds upon libstrongswan, but provides a stable API
to implement client applications in the C programming language. libvici uses
the libstrongswan thread pool to deliver event messages asynchronously.

## Connecting to the daemon ##

This example shows how to connect to the daemon using the default URI, and
then perform proper cleanup:

	#include <stdio.h>
	#include <errno.h>
	#include <string.h>

	#include <libvici.h>

	int main(int argc, char *argv[])
	{
		vici_conn_t *conn;
		int ret = 0;

		vici_init();
		conn = vici_connect(NULL);
		if (conn)
		{
			/* do stuff */
			vici_disconnect(conn);
		}
		else
		{
			ret = errno;
			fprintf(stderr, "connecting failed: %s\n", strerror(errno));
		}
		vici_deinit();
		return ret;
	}

## A simple client request ##

In the following example, a simple _version_ request is issued to the daemon
and the result is printed:

	int get_version(vici_conn_t *conn)
	{
		vici_req_t *req;
		vici_res_t *res;
		int ret = 0;

		req = vici_begin("version");
		res = vici_submit(req, conn);
		if (res)
		{
			printf("%s %s (%s, %s, %s)\n",
				vici_find_str(res, "", "daemon"),
				vici_find_str(res, "", "version"),
				vici_find_str(res, "", "sysname"),
				vici_find_str(res, "", "release"),
				vici_find_str(res, "", "machine"));
			vici_free_res(res);
		}
		else
		{
			ret = errno;
			fprintf(stderr, "version request failed: %s\n", strerror(errno));
		}
		return ret;
	}

## A request with event streaming and callback parsing ##

In this more advanced example, the _list-conns_ command is used to stream
loaded connections with the _list-conn_ event. The event message is parsed
with a simple callback to print the connection name:

	int conn_cb(void *null, vici_res_t *res, char *name)
	{
		printf("%s\n", name);
		return 0;
	}

	void list_cb(void *null, char *name, vici_res_t *res)
	{
		if (vici_parse_cb(res, conn_cb, NULL, NULL, NULL) != 0)
		{
			fprintf(stderr, "parsing failed: %s\n", strerror(errno));
		}
	}

	int list_conns(vici_conn_t *conn)
	{
		vici_req_t *req;
		vici_res_t *res;
		int ret = 0;

		if (vici_register(conn, "list-conn", list_cb, NULL) == 0)
		{
			req = vici_begin("list-conns");
			res = vici_submit(req, conn);
			if (res)
			{
				vici_free_res(res);
			}
			else
			{
				ret = errno;
				fprintf(stderr, "request failed: %s\n", strerror(errno));
			}
			vici_register(conn, "list-conn", NULL, NULL);
		}
		else
		{
			ret = errno;
			fprintf(stderr, "registration failed: %s\n", strerror(errno));
		}
		return ret;
	}

## API documentation ##

More information about the libvici API is available in the _libvici.h_ header
file or the generated Doxygen documentation.

# vici ruby gem #

The _vici ruby gem_ is a pure ruby implementation of the VICI protocol to
implement client applications. It is provided in the _ruby_ subdirectory, and
gets built and installed if strongSwan has been _./configure_'d with
_--enable-vici_ and _--enable-ruby-gems_.

The _Connection_ class from the _Vici_ module provides the high level interface,
the underlying classes are usually not required to build ruby applications
using VICI. The _Connection_ class provides methods for the supported VICI
commands and an event listening mechanism.

To represent the VICI message data tree, the gem converts the binary encoding
to ruby data types. The _Connection_ class takes and returns ruby objects for
the exchanged message data:
 * Sections get encoded as Hash, containing other sections as Hash, or
 * Key/Values, where the values are Strings as Hash values
 * Lists get encoded as Arrays with String values
Non-String values that are not a Hash nor an Array get converted with .to_s
during encoding.

## Connecting to the daemon ##

To create a connection to the daemon, a socket can be passed to the
_Connection_ constructor. If none is passed, a default Unix socket at
_/var/run/charon.vici_ is used:

	require "vici"
	require "socket"

	v = Vici::Connection.new(UNIXSocket.new("/var/run/charon.vici"))

## A simple client request ##

An example to print the daemon version information is as simple as:

	x = v.version
	puts "%s %s (%s, %s, %s)" % [
		x["daemon"], x["version"], x["sysname"], x["release"], x["machine"]
	]

## A request with closure invocation ##

The _Connection_ class takes care of event streaming by invoking a closure
for each event. The following example lists all loaded connections using the
_list-conns_ command and implicitly the _list-conn_ event:

	v.list_conns { |conn|
		conn.each { |key, value|
			puts key
		}
	}

## API documentation ##

For more details about the ruby gem refer to the comments in the gem source
code or the generated documentation.

# vici Python egg #

The _vici Python egg_ is a pure Python implementation of the VICI protocol to
implement client applications. It is provided in the _python_ subdirectory, and
gets built and installed if strongSwan has been _./configure_'d with
_--enable-vici_ and _--enable-python-eggs_.

The _vici_ module provides a _Session()_ constructor for a high level interface,
the underlying classes are usually not required to build Python applications
using VICI. The _Session_ class provides methods for the supported VICI
commands.

To represent the VICI message data tree, the library converts the binary
encoding to Python data types. The _Session_ class takes and returns Python
objects for the exchanged message data:
 * Sections get encoded as OrderedDict, containing other sections, or
 * Key/Values, where the values are strings as dictionary values
 * Lists get encoded as Python Lists with string values
Values that do not conform to Python dict or list get converted to strings using
str().

## Connecting to the daemon ##

To create a connection to the daemon, a socket can be passed to the _Session_
constructor. If none is passed, a default Unix socket at _/var/run/charon.vici_
is used:

	import vici
	import socket

	s = socket.socket(socket.AF_UNIX)
	s.connect("/var/run/charon.vici")
	v = vici.Session(s)

## A simple client request ##

An example to print the daemon version information is as simple as:

	ver = v.version()

	print "{daemon} {version} ({sysname}, {release}, {machine})".format(**ver)

## A request with response iteration ##

The _Session_ class returns an iterable Python generator for streamed events to
continuously stream objects to the caller. The following example lists all
loaded connections using the _list-conns_ command and implicitly the _list-conn_
event:

	for conn in v.list_conns():
		for key in conn:
			print key

Please note that if the returned generator is not iterated completely, it must
be closed using _close()_. This is implicitly done when breaking from a loop,
but an explicit call may be required when directly iterating the generator with
_next()_.

## Sorting in dictionaries ##

In VICI, in some message trees the order of objects in dictionary matters. In
contrast to ruby Hashes, Python dictionaries do not preserve order of added
objects. It is therefore recommended to use OrderedDicts instead of the default
dictionaries. Objects returned by the library use OrderedDicts.

## API documentation ##

For more details about the Python egg refer to the comments in the Python source
code.

# Vici::Session Perl CPAN module #

The _Vici::Session Perl CPAN module_ is a pure Perl implementation of the VICI
protocol to implement client applications. It is provided in the _perl_
subdirectory, and gets built and installed if strongSwan has been
 _./configure_'d with_--enable-vici_ and _--enable-perl-cpan_.

The _Vici::Session_ module provides a _new()_ constructor for a high level
interface, the underlying _Vici::Packet_ and _Vici::Transport_ classes are
usually not required to build Perl applications using VICI. The _Vici::Session_
class provides methods for the supported VICI commands. The auxiliare
 _Vici::Message_ class is used to encode configuration parameters sent to
the daemon and decode data returned by the daemon.

## Connecting to the daemon ##

	use IO::Socket::UNIX;
	use Vici::Session;
	use Vici::Message;

	my $socket = IO::Socket::UNIX->new(
			Type => SOCK_STREAM,
			Peer => '/var/run/charon.vici',
	) or die "Vici socket: $!";

	my $session = Vici::Session->new($socket);

## A simple client request ##

An example to print the daemon version information is as simple as:

	my $version = $session->version()->hash();

	foreach my $key ('daemon', 'version', 'sysname', 'release', 'machine' ) {
		print $version->{$key}, " ";
	}

The _Vici::Session_ methods are explained in the perl/Vici-Session/README.pod
document.
