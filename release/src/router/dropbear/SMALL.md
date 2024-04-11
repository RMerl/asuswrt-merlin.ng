## Tips for a small system

If you only want server functionality, compile with

```
make PROGRAMS=dropbear
```

rather than just

```
make dropbear
```

so that client functionality in shared portions of Dropbear won't be included.
The same applies for `PROGRAMS=dbclient`.

---
The following are set in `localoptions.h`. See `default_options.h` for possibilities.

You can disable either password or public-key authentication.

Various algorithms can be disabled if they are not required by any connecting SSH clients/servers. 
Disabling many is fine for a local install, though
builds for public consumption require more consideration.

You can disable x11, tcp and agent forwarding as desired. None of these are essential (depending on use cases).

---
If you are compiling statically, you may want to disable zlib, as it will use a few tens of kB of binary size
```
./configure --disable-zlib
```

You can create a combined binary, see the file [MULTI.md](MULTI.md), which will put all the functions into one binary, avoiding repeated code.

If you're compiling with gcc, you might want to look at gcc's options for stripping unused code.
The relevant vars to set before configure are:

```
LDFLAGS=-Wl,--gc-sections
CFLAGS="-ffunction-sections -fdata-sections"
```

You can also experiment with optimisation flags such as `-Os`. Note that in some cases these flags actually seem to increase size, so experiment before
deciding.

Of course using small C libraries such as musl can also help.

---
Libtommath has its own default `CFLAGS` to improve speed. You can use

```
./configure LTM_CFLAGS=-Os
```

to reduce size at the expense of speed.

If you have any queries, mail me and I'll see if I can help.
