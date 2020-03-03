# About Stubby

Stubby is an application that acts as a local **DNS Privacy stub resolver** (using DNS-over-TLS). Stubby encrypts DNS queries sent 
from a client machine (desktop or laptop) to a DNS Privacy resolver increasing end user privacy. Stubby is in the early stages 
of development but is suitable for technical/advanced users. A more generally user-friendly version is on the way!

Stubby provides DNS Privacy by:

* Running as a daemon
* Listening on the loopback address to send all outgoing DNS queries received on that address out over TLS
* Using a default configuration which provides Strict Privacy and uses a subset
of the available [DNS Privacy servers](https://dnsprivacy.org/wiki/x/E4AT)

Stubby is developed by the getdns team.

# Documentation

See [Stubby Homepage](https://dnsprivacy.org/wiki/x/JYAT) for more details

# Dependencies

Stubby uses [getdns](https://getdnsapi.net/) and requires the 1.5.0 release of getdns or later.

It also requires [yaml](http://pyyaml.org/wiki/LibYAML).

# Installing Using a Package Manager

Check to see if stubby, getdns and yaml are available via a package manager for your system:
https://repology.org/metapackage/stubby/versions
https://repology.org/metapackage/getdns/versions
https://repology.org/metapackage/libyaml/versions

* A [Windows Installer](https://dnsprivacy.org/wiki/x/CoBn) is now available for Stubby.
* A Homebrew package for stubby is now available (`brew install stubby`).
* A [GUI for macOS](https://dnsprivacy.org/wiki/x/CIBn) is also available for testing

If you need to install getdns from source, see the section [at the end of this document.](#building-getdns-from-source)

# Build Stubby from source

Get the code:
```
git clone https://github.com/getdnsapi/stubby.git
```

Build and install (NOTE: from release 0.3.0 stubby uses cmake)
```
cd stubby
cmake .
make
````

# Configure Stubby

_It is recommended to use the default configuration file provided which will use 'Strict' privacy mode and spread the DNS 
queries among several of the current DNS Privacy test servers. Note that this file contains both IPv4 and IPv6 addresses. 
This file is installed on *nix systems as /usr/local/etc/stubby/stubby.yml_

The configuration file format is a YAML like format and the name of the file must have an extension of .yml. Essentially the 
configuration options available are the same as the options that can be set on a getdns `context` - Doxygen documentation for 
which is available [here](https://getdnsapi.net/doxygen/group__getdns__context.html). To aid with creating a custom configuration file, an example is given below.

NOTE: As of the 0.1.3 release of Stubby the YAML format replaces the JSON like format used in earlier versions of the configuration file for getdns/stubby. The YAML format is 
more human readable and supports comments allowing options to be easily enabled and disabled. The JSON format is that which is used internally in getdns
(it is the same as the output returned by `stubby -i`) and is still available by directly specifying a file with the name 'stubby.conf' on the command line using the -C option.

## Create Custom Configuration File

Alternatively the configuration file location can be specified on the command line using the `-C` flag. Changes to the configuration file require a restart of Stubby.

The config file below will configure Stubby in the following ways:

*  `resolution_type`: Work in stub mode only (not recursive mode) - required for Stubby operation.
*  `dns_transport_list`: Use TLS only as a transport (no fallback to UDP or TCP). 
*  `tls_authentication`:  Use Strict Privacy i.e. require a TLS connection and authentication of the upstream
  * If Opportunistic mode is desired, simply remove the `tls_authentication: GETDNS_AUTHENTICATION_REQUIRED` field. In Opportunistic mode authentication of the nameserver is not required and fallback to clear text transports is permitted if they are in the `dns_transport_list`.
*  `tls_query_padding_blocksize`: Use the EDNS0 padding option to pad DNS queries to hide their size
*  `edns_client_subnet_private`: Use EDNS0 Client Subnet privacy so the client subnet is not sent to authoritative servers
* ` idle_timeout`:  Use an EDNS0 Keepalive idle timeout of 10s unless overridden by the server. This keeps idle TLS connections open to avoid the overhead of opening a new connection for every query.
*  `listen_address`: have the Stubbby daemon listen on IPv4 and IPv6 on port 53 on the loopback address
*   `round_robin_upstreams`: Round robin queries across all the configured upstream servers. Without this option Stubby will use each upstream server sequentially until it becomes unavailable and then move on to use the next. 
*  `upstream_recursive_servers`: Use the NLnet labs test DNS Privacy Server for outgoing queries. In Strict Privacy mode, at least one of the following is required for each nameserver:
  *  `tls_auth_name`: This is the authentication domain name that will be verified against the presented certificate. 
  * `tls_pubkey_pinset`: The sha256 SPKI pinset for the server. This is also verified against the presented certificate. 

```
resolution_type: GETDNS_RESOLUTION_STUB
dns_transport_list: 
  - GETDNS_TRANSPORT_TLS
tls_authentication: GETDNS_AUTHENTICATION_REQUIRED
tls_query_padding_blocksize: 256
edns_client_subnet_private : 1
idle_timeout: 10000
listen_addresses:
  - 127.0.0.1
  -  0::1
round_robin_upstreams: 1
upstream_recursive_servers:
  - address_data: 185.49.141.38
    tls_auth_name: "getdnsapi.net"
    tls_pubkey_pinset:
      digest: "sha256"
       value: foxZRnIh9gZpWnl+zEiKa0EJ2rdCGroMWm02gaxSc9Q=
```

Additional privacy servers can be specified by adding more entries to the `upstream_recursive_servers` list above (note a separate entry must be made for the IPv4 and IPv6 addresses of a given server. More DNS Privacy test servers are listed [here](https://dnsprivacy.org/wiki/x/E4AT).

A custom port can be specified by adding the `tls_port:` attribute to the `upstream_recursive_server` in the config file. 

More details can be found in the comments in the default configuration file and at https://dnsprivacy.org/wiki/display/DP/Configuring+Stubby

# Run Stubby


Simply invoke Stubby on the command line. By default it runs in the foreground, the `-g` flag runs it in the background. 

```sh
> sudo stubby
```

Or, to let it run as an unprivileged user:
```sh
> sudo setcap 'cap_net_bind_service=+ep' /usr/local/bin/stubby
> stubby
```

* Enable connection logging by using the `-l` flag. The logging is currently simplistic and simply writes to stdout. (We are working on making this better!)
* A custom configuration file can be specified using the -C flag.
* The pid file is /usr/local/var/run/stubby.pid

# Platform specific management
The Windows and macOS installers include scripts to run stubby as a managed daemon. We have basic support for using systemd to manage Stubby, see [systemd](https://github.com/getdnsapi/stubby/tree/master/systemd)

# Test Stubby

A quick test can be done by using dig (or your favourite DNS tool) on the loopback address

```sh
> dig @127.0.0.1 www.example.com
```

```sh
> getdns_query -s @127.0.0.1 www.example.com
```

# Modify your upstream resolvers

!!! <span class="glyphicon glyphicon-warning-sign"></span> Once this change is made your DNS queries will be re-directed to Stubby and sent over TLS! <br>
(You may need to restart some applications to have them pick up the network settings). <p>You can monitor the traffic using Wireshark watching on port 853.</p>

For Stubby to re-send outgoing DNS queries over TLS the system stub resolvers on your machine must be changed to send all the local queries to the loopback interface on which Stubby is listening. This depends on the operating system being run. It is useful to note your existing default nameservers before making this change!


## Linux/Unix systems

* Edit the /etc/resolv.conf file
* Comment out the existing *nameserver* entries
* Add the following (only add the IPv4 address if you don't have IPv6)
  ```sh
  nameserver 127.0.0.1
  nameserver ::1
  ```

## OS X

A script is provided with Stubby for easier configuration. From the command line you can do the following to switch all your queries to use Stubby

```sh
> sudo /usr/local/sbin/stubby-setdns-macos.sh
```

If you want to reset, just use:

```sh
> sudo /usr/local/sbin/stubby-setdns-macos.sh -r
```

which should pick up the default DHCP nameservers.

You can add /usr/local/sbin to your path to avoid having to type it above by doing
```
export PATH=/usr/local/sbin:$PATH
```

Or via the GUI:

* Open *System Preferences &rarr; Network &rarr; Advanced &rarr; DNS*
* Use the '-' button to remove the existing nameservers
* Use the '+' button to add `127.0.0.1` and `::1` (only add the IPv4 address if you don't have IPv6)
* Hit 'OK' in the *DNS* pane and then 'Apply' on the *Network* pane

## Windows 8 and later

Powershell scripts are provided in the the windows directory of the source code that can be used to update the system resolvers. 
Instructions for how to update the resolvers manually are provided are also provided - see https://dnsprivacy.org/wiki/display/DP/Windows+installer+for+Stubby 
Stubby has been reported to work on Windows 7, but we don't officially support it. 

## Notes:

* If Stubby works for a while but you then see failures from Stubby such as "None of the configured upstreams could be used to send queries on the specified transports" try restarting Stubby.
* If you are using a DNS Privacy server that does not support concurrent processing of TLS queries, you may experience some issues due to timeouts causing subsequent queries on the same connection to fail.

# Building getdns from source

Note that from getdns 1.1.3 stubby is included in the getdns code as a git submodule. Therefore stubby and getdns can be built together by following the
instructions in the getdns [README](https://github.com/getdnsapi/getdns/blob/develop/README.md) and setting the ``BUILD_STUBBY`` option.

## Logging/debugging when building from source

> **`--enable-debug-stub`**   If you do want to see _very_ detailed debug information as messages are processed then add the `--enable-debug-stub` option to the `configure` line above (not recommended for use with Stubby)


# Contributions

The contrib directory contains code kindly contributed by various people:

vapniks
Tom Matthews
CameronNemo