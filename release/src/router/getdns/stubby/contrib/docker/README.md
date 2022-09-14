# Docker usage

A [Dockerfile](./Dockerfile) is provided to enable building and running stubby
using Docker only.

It enables development and testing with just Docker installed. It can be also
used to deploy in production, although it's not specifically optimized for that
usage (eg. image size). You can build and run the latest HEAD or whatever
branch, tag or commit id you want.

Keep in mind that the main purpose the Dockerfile addresses is development and
building from source locally. It might not be optimal for distribution purposes,
but can also work as a quick build and run solution with a one-liner, though.

Image builds code on latest Debian LTS (Bullseye), with these library versions:
- libidn2 2.3.0
- libgetdns 1.6.0
- libunbound 1.13.1
- openssl 1.1.1k

The built binary and the example configuration file are copied to the final
image. The configuration file gets added the `0.0.0.0` listen address to allow
the program to get incoming connections from outside the container (no IPv6).

## Build
To build from a local git checkout:
``` sh
docker build --force-rm --pull -t stubby \
  -f contrib/docker/Dockerfile .
```

To build from Github source (without cloning), issue the following:
``` sh
docker build --force-rm --pull -t stubby \
  -f contrib/docker/Dockerfile github.com/getdnsapi/stubby
```
You can specify any branch, tag or commit id by appending it after the URL,
separated by a `#` sign (e.g. `#develop`, `#v0.4.0`, etc.).

## Run
Docker image provides a default configuration file at `/usr/local/etc/stubby.yml`
which is a copy of [stubby.yml.example](../../stubby.yml.example) from the source repository.
The build process only adds `0.0.0.0` to the listen addresses to allow `stubby`
to get incoming connections from outside the container. Use `docker` command's
`-v` switch to mount an external configuration file, if needed.

When lauching containers, make sure to use `docker`'s `-p` switch to expose
ports on the desired bind address/port/protocol, as shown here:
``` sh
docker run --rm -it -p 127.0.0.1:5353:53/tcp -p 127.0.0.1:5353:53/udp stubby
```
The above example exposes internal port 53 where `stubby` is listening on the
host's `127.0.0.1:5353`, both on TCP and UDP.

For passing command line options to `stubby`, just append them after the image
name (e.g. `stubby` in the example above).

Notice that `stubby` will be forcefully killed on `docker stop` after timeout
due to #188.
