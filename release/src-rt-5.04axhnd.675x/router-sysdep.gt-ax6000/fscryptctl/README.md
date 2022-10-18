# fscryptctl

fscryptctl is a low-level tool written in C that handles raw keys and manages
policies for [Linux filesystem encryption](https://lwn.net/Articles/639427). For
a tool that presents a higher level interface and manages metadata, key
generation, key wrapping, PAM integration, and passphrase hashing, see
[fscrypt](https://github.com/google/fscrypt).

To use fscryptctl, you must have a filesystem with encryption enabled and a
kernel that supports reading/writing from that filesystem. Currently,
[ext4](https://en.wikipedia.org/wiki/Ext4),
[F2FS](https://en.wikipedia.org/wiki/F2FS), and
[UBIFS](https://en.wikipedia.org/wiki/UBIFS) support Linux filesystem
encryption. Ext4 has supported Linux filesystem encryption
[since v4.1](https://lwn.net/Articles/639427), F2FS
[added support in v4.2](https://lwn.net/Articles/649652), and UBIFS
[added support in v4.10](https://lwn.net/Articles/707900). Note that only
certain configurations of the Linux kernel enable encryption, and other
filesystems may add support for encryption.

Most of the testing for fscrypt has been done with ext4 filesystems. However,
the kernel uses a common userspace interface, so this tool should work with all
existing and future filesystems which support encryption. If there is a problem
using fscrypt with other filesystems, please open an issue.

### Other encryption solutions

It is important to distinguish Linux filesystem encryption from two other
encryption solutions: [eCryptfs](https://en.wikipedia.org/wiki/ECryptfs) and
[dm-crypt](https://en.wikipedia.org/wiki/Dm-crypt).

Currently, dm-crypt encrypts an entire block device with a single master key. If
you do not need the fine-grained controls of fscryptctl or want to fully
encrypt your filesystem metadata, dm-crypt could be a simpler choice.

On the other hand, eCryptfs is another form of filesystem encryption on Linux;
it encrypts a filesystem directory with some key or passphrase. eCryptfs sits on
top of an existing filesystem. This make eCryptfs an alternative choice if your
filesystem or kernel does not support Linux filesystem encryption or you do not
want to modify your existing filesystem.

Also note that fscryptctl does not support or setup either eCryptfs or
dm-crypt. For these tools, use
[ecryptfs-utils](https://packages.debian.org/source/jessie/ecryptfs-utils) for
eCryptfs or [cryptsetup](https://linux.die.net/man/8/cryptsetup) for dm-crypt.

## Features

This tool aims to improve upon the work in
[e4crypt](http://man7.org/linux/man-pages/man8/e4crypt.8.html) with fscryptctl
providing a smaller and simpler interface. It only supports the minimal
functionality required to use filesystem encryption.  It supports the following
actions:
*   Getting the key descriptor for a provided key
*   Inserting a provided key into the keyring (with optional legacy flags)
*   Querying the encryption policy (i.e. key descriptor) for a file or directory
*   Setting an encryption policy on a directory

## Building

Get the source by running `git clone https://github.com/google/fscryptctl`.
Run `make` to build the executable fscryptctl. The only build dependencies are
`make` and a C compiler.

## Running and Installing

fscryptctl is a standalone binary, so it just needs to have support for
filesystem encryption and for the `keyctl()` and `add_key()` syscalls to exist,
which they will be available on any kernel which supports filesystem encryption.

Run `fscryptctl --help` to see the full usage and description of the available
commands and flags. Installing the tool just requires placing it in your path or
running `sudo make install` (set `DESTDIR` to install to a custom locations).

## Example Usage
```shell
# Make a random 512-bit key and store it in a file
> dd if=/dev/urandom of=key.data count=64 bs=1
# Get the descriptor for the key
> ./fscryptctl get_descriptor < key.data
cd8c77009a9a3e6d
# Insert the key into the keyring (using legacy ext4 options)
> ./fscryptctl insert_key --ext4 < key.data
cd8c77009a9a3e6d
> keyctl show
Session Keyring
 827244259 --alswrv  416424 65534  keyring: _uid_ses.416424
 111054036 --alswrv  416424 65534   \_ keyring: _uid.416424
 227138126 --alsw-v  416424  5000   \_ logon: ext4:cd8c77009a9a3e6d

# Remove the key from the keyring
> keyctl unlink 227138126
# Make a test directory on a filesystem that supports encryption
> mkdir /mnt/disks/encrypted/test
# Setup an encryption policy on that directory
> ./fscryptctl set_policy cd8c77009a9a3e6d /mnt/disks/encrypted/test
> ./fscryptctl get_policy /mnt/disks/encrypted/test
Encryption policy for /mnt/disks/encrypted/test:
        Policy Version: 0
        Key Descriptor: cd8c77009a9a3e6d
        Contents: AES-256-XTS
        Filenames: AES-256-CTS
        Padding: 32
# We cannot create files in the directory without the key
> echo "Hello World!" > /mnt/disks/encrypted/test/foo.txt
An error occurred while redirecting file '/mnt/disks/encrypted/test/foo.txt'
open: No such file or directory
> ./fscryptctl insert_key --ext4 < key.data
cd8c77009a9a3e6d
# Now we can make the file and write data to it
> echo "Hello World!" > /mnt/disks/encrypted/test/foo.txt
> ls -lA /mnt/disks/encrypted/test/
total 4
-rw-rw-r-- 1 joerichey joerichey 12 Mar 30 20:00 foo.txt
> cat /mnt/disks/encrypted/test/foo.txt
Hello World!

# Now we remove the key, remount the filesystem, and see the encrypted data
> keyctl show
Session Keyring
1047869403 --alswrv   1001  1002  keyring: _ses
 967765418 --alswrv   1001 65534   \_ keyring: _uid.1001
1009690551 --alsw-v   1001  1002   \_ logon: ext4:cd8c77009a9a3e6d
> keyctl unlink 1009690551
1 links removed
> umount /mnt/disks/encrypted
> mount /mnt/disks/encrypted
> ls -lA /mnt/disks/encrypted/test/
total 4
-rw-rw-r-- 1 joerichey joerichey 13 Mar 30 20:00 wnJP+VX33Y6OSbN08+,jtQXK9yMHm8CFcI64CxDFPxL
> cat /mnt/disks/encrypted/test/wnJP+VX33Y6OSbN08+,jtQXK9yMHm8CFcI64CxDFPxL
cat: /mnt/disks/encrypted/test/wnJP+VX33Y6OSbN08+,jtQXK9yMHm8CFcI64CxDFPxL: Required key not available

# Reinserting the key restores access to the data
> ./fscryptctl insert_key --ext4 < key.data
cd8c77009a9a3e6d
> ls -lA /mnt/disks/encrypted/test/
total 4
-rw-rw-r-- 1 joerichey joerichey 12 Mar 30 20:00 foo.txt
> cat /mnt/disks/encrypted/test/foo.txt
Hello World!
```

## Contributing

We would love to accept your contributions to fscryptctl. See the
`CONTRIBUTING.md` file for more information about singing the CLA and submitting
a pull request.

## Known Issues

#### Getting "filesystem encryption has been disabled" on an ext4 filesystem.

Getting this error on an ext4 system usually means the filesystem has not been
setup for encryption. To setup a filesystem to support encryption, first check
that your block size is equal to your page size by comparing the outputs of
`getconf PAGE_SIZE` and `tune2fs -l /dev/device | grep 'Block size'`. If these
are not the same, DO NOT ENABLE ENCRYPTION.

To turn on the encryption feature flag for your filesystem, run
```
tune2fs -O encrypt /dev/device
```
This command may require root privileges. Once the flag is enabled, older
kernels may not be able to mount the filesystem. Note that there was a bug in an
older kernel version that allowed encryption policies to be set on ext4
filesystems without enabling this encryption feature flag.

#### Files are still visible in plaintext after encryption key is removed.

This is an issue with how the Linux kernel implements filesystem encryption. The
plaintext is still cached even after the key is removed. To clear these caches
after removing the appropriate key, either unmount and remount the filesystem,
or run:
```bash
echo 3 | sudo tee /proc/sys/vm/drop_caches
```
There used to be kernel functionality to "lock" files after their keys had been
removed. However, [this was removed](https://patchwork.kernel.org/patch/9585865)
because the implementation was insecure and buggy.

## Legal

Copyright 2017 Google Inc. under the
[Apache 2.0 License](https://www.apache.org/licenses/LICENSE-2.0); see the
`LICENSE` file for more information.

Author: Joe Richey <joerichey@google.com>

This is not an official Google product.
