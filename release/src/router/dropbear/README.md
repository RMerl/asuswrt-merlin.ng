## Dropbear SSH
A smallish SSH server and client
https://matt.ucc.asn.au/dropbear/dropbear.html

[INSTALL.md](INSTALL.md) has compilation instructions.

[MULTI.md](MULTI.md) has instructions on making a multi-purpose binary (ie a single binary which performs multiple tasks, to save disk space).

[SMALL.md](SMALL.md) has some tips on creating small binaries.

A mirror of the Dropbear website and tarballs is available at https://dropbear.nl/mirror/.

Please contact me if you have any questions/bugs found/features/ideas/comments etc
There is also a mailing list https://lists.ucc.asn.au/mailman/listinfo/dropbear

Matt Johnston
matt@ucc.asn.au


### In the absence of detailed documentation, some notes follow

----
#### Server public key auth

You can use `~/.ssh/authorized_keys` in the same way as with OpenSSH, just put the key entries in that file.
They should be of the form:

    ssh-rsa AAAAB3NzaC1yc2EAAAABIwAAAIEAwVa6M6cGVmUcLl2cFzkxEoJd06Ub4bVDsYrWvXhvUV+ZAM9uGuewZBDoAqNKJxoIn0Hyd0NkyU99UVv6NWV/5YSHtnf35LKds56j7cuzoQpFIdjNwdxAN0PCET/MG8qyskG/2IE2DPNIaJ3Wy+Ws4IZEgdJgPlTYUBWWtCWOGc= someone@hostname

You must make sure that `~/.ssh`, and the key file, are only writable by the user.
Beware of editors that split the key into multiple lines.

Dropbear supports some options for authorized_keys entries, see the manpage.

----
#### Client public key auth

Dropbear can do public key auth as a client.
But you will have to convert OpenSSH style keys to Dropbear format, or use dropbearkey to create them.

If you have an OpenSSH-style private key `~/.ssh/id_rsa`, you need to do:

```sh
dropbearconvert openssh dropbear ~/.ssh/id_rsa  ~/.ssh/id_rsa.db
dbclient -i ~/.ssh/id_rsa.db <hostname>
```

Dropbear does not support encrypted hostkeys though can connect to ssh-agent.

----
If you want to get the public-key portion of a Dropbear private key, look at dropbearkey's `-y` option.
It will print both public key and fingerprint. If you need the pub key only you can grep by a prefix `ssh-`: 
```sh
./dropbearkey -y -f ~/.ssh/id_ed25519 | grep "^ssh-" > ~/.ssh/id_ed25519.pub
```

----
To run the server, you need to generate server keys, this is one-off:

```sh
./dropbearkey -t rsa -f dropbear_rsa_host_key
./dropbearkey -t dss -f dropbear_dss_host_key
./dropbearkey -t ecdsa -f dropbear_ecdsa_host_key
./dropbearkey -t ed25519 -f dropbear_ed25519_host_key
```

Or alternatively convert OpenSSH keys to Dropbear:

```sh
./dropbearconvert openssh dropbear /etc/ssh/ssh_host_dsa_key dropbear_dss_host_key
```

You can also get Dropbear to create keys when the first connection is made - this is preferable to generating keys when the system boots.
Make sure `/etc/dropbear/` exists and then pass `-R` to the dropbear server.

----
If the server is run as non-root, you most likely won't be able to allocate a pty, and you cannot login as any user other than that running the daemon (obviously).
Shadow passwords will also be unusable as non-root.

----
The Dropbear distribution includes a standalone version of OpenSSH's `scp` program.
You can compile it with `make scp`.
You may want to change the path of the ssh binary, specified by `_PATH_SSH_PROGRAM` in `options.h`.
By default the progress meter isn't compiled in to save space, you can enable it by adding `SCPPROGRESS=1` to the `make` commandline.
