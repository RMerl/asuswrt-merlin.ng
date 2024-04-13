## Basic Dropbear Build Instructions

### Build Options

Custom build options can be configured in `localoptions.h` in the build directory. This is a local file, not checked in to git.

Available options are described in [`src/default_options.h`](src/default_options.h)
Options include available cryptographic algorithms, SSH features, and file paths.

In addition, a `src/distrooptions.h` file will be used if it exists, for distributions to set configuration options.

### Configure for your system
```
./configure
```
Optionally with `--disable-zlib` or `--disable-syslog`.
Or `--help` for other options.

You'll need to first run `autoconf; autoheader` if you edit `configure.ac`.

### Compile:

```
make PROGRAMS="dropbear dbclient dropbearkey dropbearconvert scp"
```

Optionally install, or copy the binaries another way:

```
make install
```
`/usr/local/bin` is usual default.

or

```
make PROGRAMS="dropbear dbclient dropbearkey dropbearconvert scp" install
```

To test the installation targeting a temporary forder set `DESTDIR`:
```
make install DESTDIR=/same/temp/location
```

You can leave items out of the `PROGRAMS` list to avoid compiling them.
If you recompile after changing the `PROGRAMS` list, you **MUST** `make clean` before recompiling - bad things will happen otherwise.

[DEVELOPING.md](DEVELOPING.md) has some notes on other developer topics, including debugging.

See [MULTI.md](MULTI.md) for instructions on making all-in-one binaries.

If you want to compile statically use
```
./configure --enable-static
```

By default Dropbear adds various build flags that improve robustness against programming bugs (good for security).
If these cause problems they can be disabled with `./configure --disable-harden`.

Binaries can be stripped with `make strip`.

> **Note**
> If you're compiling for a 386-class CPU, you will probably need to add CFLAGS=-DLTC_NO_BSWAP so that libtomcrypt doesn't use 486+ instructions.

## Compiling with uClibc

Firstly, make sure you have at least uclibc 0.9.17, as `getusershell()` in prior versions is broken.
Also note that you may get strange issues if your uClibc headers don't match the library you are running with.
I.e. the headers might say that shadow password support exists, but the libraries don't have it.

Compiling for uClibc should be the same as normal, just set CC to the magic uClibc toolchain compiler (ie `export CC=i386-uclibc-gcc` or whatever).
You can use `make STATIC=1` to make statically linked binaries, and it is advisable to strip the binaries too.
If you're looking to make a small binary, you should remove unneeded ciphers and algorithms, by editing [localoptions.h](./localoptions.h).

It is possible to compile zlib in, by copying zlib.h and zconf.h into a subdirectory (ie zlibincludes), and

```
export CFLAGS="-Izlibincludes -I../zlibincludes"
export LDFLAGS=/usr/lib/libz.a
```
before `./configure` and `make`.

If you disable zlib, you must explicitly disable compression for the client.
OpenSSH is possibly buggy in this regard, it seems you need to disable it globally in `~/.ssh/config`, not just in the host entry in that file.

You may want to manually disable lastlog recording when using uClibc, configure with `--disable-lastlog`.

One common problem is pty allocation.
There are a number of types of pty allocation which can be used -- if they work properly, the end result is the same for each type.
Running configure should detect the best type to use automatically, however for some systems, this may be incorrect.
Some things to note:

* If your system expects `/dev/pts` to be mounted (this is a uClibc option), make sure that it is.
* Make sure that your libc headers match the library version you are using.
* If `openpty()` is being used (`HAVE_OPENPTY` defined in `config.h`) and it fails, you can try compiling with `--disable-openpty`. 
  You will probably then need to create all the `/dev/pty??` and `/dev/tty??` devices, which can be problematic for `devfs`.
  In general, `openpty()` is the best way to allocate PTYs, so it's best to try and get it working.
