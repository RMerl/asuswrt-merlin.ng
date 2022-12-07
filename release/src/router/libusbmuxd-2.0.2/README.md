# libusbmuxd

*A client library for applications to handle usbmux protocol connections with iOS devices.*

## Features

This project is a client library to multiplex connections from and to iOS
devices alongside command-line utilities.

It is primarily used by applications which use the [libimobiledevice](https://github.com/libimobiledevice/)
library to communicate with services running on iOS devices.

The library does not establish a direct connection with a device but requires
connecting to a socket provided by the usbmuxd daemon.

The usbmuxd daemon is running upon installing iTunes on Windows and Mac OS X.

The [libimobiledevice project](https://github.com/libimobiledevice/) provides an open-source reimplementation of
the [usbmuxd daemon](https://github.com/libimobiledevice/usbmuxd.git/) to use on Linux or as an alternative to communicate with
iOS devices without the need to install iTunes.

Some key features are:

- **Protocol:** Provides an interface to handle the usbmux protocol
- **Port Proxy:** Provides the `iproxy` utility to proxy ports to the device
- **Netcat:** Provides the `inetcat` utility to expose a raw connection to the device
- **Cross-Platform:** Tested on Linux, macOS, Windows and Android platforms
- **Flexible:** Allows using the open-source or proprietary usbmuxd daemon

Furthermore the Linux build optionally provides support using inotify if
available.

## Installation / Getting started

### Debian / Ubuntu Linux

First install all required dependencies and build tools:
```shell
sudo apt-get install \
	build-essential \
	checkinstall \
	git \
	autoconf \
	automake \
	libtool-bin \
	libplist-dev \
	usbmuxd
```

Then clone the actual project repository:
```shell
git clone https://github.com/libimobiledevice/libusbmuxd.git
cd libusbmuxd
```

Now you can build and install it:
```shell
./autogen.sh
make
sudo make install
```

## Usage

### iproxy

This utility allows binding local TCP ports so that a connection to one (or
more) of the local ports will be forwarded to the specified port (or ports) on
a usbmux device.

Bind local TCP port 2222 and forward to port 22 of the first device connected
via USB:
```shell
iproxy 2222:22
```

This would allow using ssh with `localhost:2222` to connect to the sshd daemon
on the device. Please mind that this is just an example and the sshd daemon is
only available for jailbroken devices that actually have it installed.

Please consult the usage information or manual page for a full documentation of
available command line options:
```shell
iproxy --help
man iproxy
```

### inetcat

This utility is a simple netcat-like tool that allows opening a read/write
interface to a TCP port on a usbmux device and expose it via STDIN/STDOUT.

Use ssh ProxyCommand to connect to a jailbroken iOS device via SSH:
```shell
ssh -oProxyCommand="inetcat 22" root@localhost
```

Please consult the usage information or manual page for a full documentation of
available command line options:
```shell
inetcat --help
man inetcat
```

### Environment

The environment variable `USBMUXD_SOCKET_ADDRESS` allows to change the location
of the usbmuxd socket away from the local default one.

An example of using an utility from the libimobiledevice project with an usbmuxd
socket exposed on a port of a remote host:
```shell
export USBMUXD_SOCKET_ADDRESS=192.168.179.1:27015
ideviceinfo
```

This sets the usbmuxd socket address to `192.168.179.1:27015` for applications
that use the libusbmuxd library.

## Contributing

We welcome contributions from anyone and are grateful for every pull request!

If you'd like to contribute, please fork the `master` branch, change, commit and
send a pull request for review. Once approved it can be merged into the main
code base.

If you plan to contribute larger changes or a major refactoring, please create a
ticket first to discuss the idea upfront to ensure less effort for everyone.

Please make sure your contribution adheres to:
* Try to follow the code style of the project
* Commit messages should describe the change well without being to short
* Try to split larger changes into individual commits of a common domain
* Use your real name and a valid email address for your commits

We are still working on the guidelines so bear with us!

## Links

* Homepage: https://libimobiledevice.org/
* Repository: https://git.libimobiledevice.org/libusbmuxd.git
* Repository (Mirror): https://github.com/libimobiledevice/libusbmuxd.git
* Issue Tracker: https://github.com/libimobiledevice/libusbmuxd/issues
* Mailing List: https://lists.libimobiledevice.org/mailman/listinfo/libimobiledevice-devel
* Twitter: https://twitter.com/libimobiledev

## License

This library is licensed under the [GNU Lesser General Public License v2.1](https://www.gnu.org/licenses/lgpl-2.1.en.html),
also included in the repository in the `COPYING` file.

The utilities `iproxy` and `inetcat` are licensed under the [GNU General Public License v2.0](https://www.gnu.org/licenses/gpl-2.0.en.html).

## Credits

Apple, iPhone, iPad, iPod, iPod Touch, Apple TV, Apple Watch, Mac, iOS,
iPadOS, tvOS, watchOS, and macOS are trademarks of Apple Inc.

This project is an independent software library and has not been authorized,
sponsored, or otherwise approved by Apple Inc.

README Updated on: 2020-06-12
