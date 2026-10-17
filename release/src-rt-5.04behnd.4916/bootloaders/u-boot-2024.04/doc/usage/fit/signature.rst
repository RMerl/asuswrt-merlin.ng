.. SPDX-License-Identifier: GPL-2.0+

U-Boot FIT Signature Verification
=================================

Introduction
------------

FIT supports hashing of images so that these hashes can be checked on
loading. This protects against corruption of the image. However it does not
prevent the substitution of one image for another.

The signature feature allows the hash to be signed with a private key such
that it can be verified using a public key later. Provided that the private
key is kept secret and the public key is stored in a non-volatile place,
any image can be verified in this way.

See verified-boot.txt for more general information on verified boot.


Concepts
--------

Some familiarity with public key cryptography is assumed in this section.

The procedure for signing is as follows:

   - hash an image in the FIT
   - sign the hash with a private key to produce a signature
   - store the resulting signature in the FIT

The procedure for verification is:

   - read the FIT
   - obtain the public key
   - extract the signature from the FIT
   - hash the image from the FIT
   - verify (with the public key) that the extracted signature matches the
     hash

The signing is generally performed by mkimage, as part of making a firmware
image for the device. The verification is normally done in U-Boot on the
device.


Algorithms
----------
In principle any suitable algorithm can be used to sign and verify a hash.
U-Boot supports a few hashing and verification algorithms. See below for
details.

While it is acceptable to bring in large cryptographic libraries such as
openssl on the host side (e.g. mkimage), it is not desirable for U-Boot.
For the run-time verification side, it is important to keep code and data
size as small as possible.

For this reason the RSA image verification uses pre-processed public keys
which can be used with a very small amount of code - just some extraction
of data from the FDT and exponentiation mod n. Code size impact is a little
under 5KB on Tegra Seaboard, for example.

It is relatively straightforward to add new algorithms if required. If
another RSA variant is needed, then it can be added with the
U_BOOT_CRYPTO_ALGO() macro. If another algorithm is needed (such as DSA) then
it can be placed in a directory alongside lib/rsa/, and its functions added
using U_BOOT_CRYPTO_ALGO().


Creating an RSA key pair and certificate
----------------------------------------
To create a new public/private key pair, size 2048 bits::

    $ openssl genpkey -algorithm RSA -out keys/dev.key \
        -pkeyopt rsa_keygen_bits:2048 -pkeyopt rsa_keygen_pubexp:65537

To create a certificate for this containing the public key::

    $ openssl req -batch -new -x509 -key keys/dev.key -out keys/dev.crt

If you like you can look at the public key also::

    $ openssl rsa -in keys/dev.key -pubout


Public Key Storage
------------------
In order to verify an image that has been signed with a public key we need to
have a trusted public key. This cannot be stored in the signed image, since
it would be easy to alter. For this implementation we choose to store the
public key in U-Boot's control FDT (using CONFIG_OF_CONTROL).

Public keys should be stored as sub-nodes in a /signature node. Required
properties are:

algo
    Algorithm name (e.g. "sha256,rsa2048" or "sha512,ecdsa256")

Optional properties are:

key-name-hint
    Name of key used for signing. This is only a hint since it
    is possible for the name to be changed. Verification can proceed by checking
    all available signing keys until one matches.

required
    If present this indicates that the key must be verified for the
    image / configuration to be considered valid. Only required keys are
    normally verified by the FIT image booting algorithm. Valid values are
    "image" to force verification of all images, and "conf" to force verification
    of the selected configuration (which then relies on hashes in the images to
    verify those).

Each signing algorithm has its own additional properties.

For RSA the following are mandatory:

rsa,num-bits
    Number of key bits (e.g. 2048)

rsa,modulus
    Modulus (N) as a big-endian multi-word integer

rsa,exponent
    Public exponent (E) as a 64 bit unsigned integer

rsa,r-squared
    (2^num-bits)^2 as a big-endian multi-word integer

rsa,n0-inverse
    -1 / modulus[0] mod 2^32

For ECDSA the following are mandatory:

ecdsa,curve
    Name of ECDSA curve (e.g. "prime256v1")

ecdsa,x-point
    Public key X coordinate as a big-endian multi-word integer

ecdsa,y-point
    Public key Y coordinate as a big-endian multi-word integer

These parameters can be added to a binary device tree using parameter -K of the
mkimage command::

    tools/mkimage -f fit.its -K control.dtb -k keys -r image.fit

Here is an example of a generated device tree node::

    signature {
        key-dev {
            required = "conf";
            algo = "sha256,rsa2048";
            rsa,r-squared = <0xb76d1acf 0xa1763ca5 0xeb2f126
                    0x742edc80 0xd3f42177 0x9741d9d9
                    0x35bb476e 0xff41c718 0xd3801430
                    0xf22537cb 0xa7e79960 0xae32a043
                    0x7da1427a 0x341d6492 0x3c2762f5
                    0xaac04726 0x5b262d96 0xf984e86d
                    0xb99443c7 0x17080c33 0x940f6892
                    0xd57a95d1 0x6ea7b691 0xc5038fa8
                    0x6bb48a6e 0x73f1b1ea 0x37160841
                    0xe05715ce 0xa7c45bbd 0x690d82d5
                    0x99c2454c 0x6ff117b3 0xd830683b
                    0x3f81c9cf 0x1ca38a91 0x0c3392e4
                    0xd817c625 0x7b8e9a24 0x175b89ea
                    0xad79f3dc 0x4d50d7b4 0x9d4e90f8
                    0xad9e2939 0xc165d6a4 0x0ada7e1b
                    0xfb1bf495 0xfc3131c2 0xb8c6e604
                    0xc2761124 0xf63de4a6 0x0e9565f9
                    0xc8e53761 0x7e7a37a5 0xe99dcdae
                    0x9aff7e1e 0xbd44b13d 0x6b0e6aa4
                    0x038907e4 0x8e0d6850 0xef51bc20
                    0xf73c94af 0x88bea7b1 0xcbbb1b30
                    0xd024b7f3>;
            rsa,modulus = <0xc0711d6cb 0x9e86db7f 0x45986dbe
                       0x023f1e8c9 0xe1a4c4d0 0x8a0dfdc9
                       0x023ba0c48 0x06815f6a 0x5caa0654
                       0x07078c4b7 0x3d154853 0x40729023
                       0x0b007c8fe 0x5a3647e5 0x23b41e20
                       0x024720591 0x66915305 0x0e0b29b0
                       0x0de2ad30d 0x8589430f 0xb1590325
                       0x0fb9f5d5e 0x9eba752a 0xd88e6de9
                       0x056b3dcc6 0x9a6b8e61 0x6784f61f
                       0x000f39c21 0x5eec6b33 0xd78e4f78
                       0x0921a305f 0xaa2cc27e 0x1ca917af
                       0x06e1134f4 0xd48cac77 0x4e914d07
                       0x0f707aa5a 0x0d141f41 0x84677f1d
                       0x0ad47a049 0x028aedb6 0xd5536fcf
                       0x03fef1e4f 0x133a03d2 0xfd7a750a
                       0x0f9159732 0xd207812e 0x6a807375
                       0x06434230d 0xc8e22dad 0x9f29b3d6
                       0x07c44ac2b 0xfa2aad88 0xe2429504
                       0x041febd41 0x85d0d142 0x7b194d65
                       0x06e5d55ea 0x41116961 0xf3181dde
                       0x068bf5fbc 0x3dd82047 0x00ee647e
                       0x0d7a44ab3>;
            rsa,exponent = <0x00 0x10001>;
            rsa,n0-inverse = <0xb3928b85>;
            rsa,num-bits = <0x800>;
            key-name-hint = "dev";
        };
    };


Signed Configurations
---------------------
While signing images is useful, it does not provide complete protection
against several types of attack. For example, it is possible to create a
FIT with the same signed images, but with the configuration changed such
that a different one is selected (mix and match attack). It is also possible
to substitute a signed image from an older FIT version into a newer FIT
(roll-back attack).

As an example, consider this FIT::

    / {
        images {
            kernel-1 {
                data = <data for kernel1>
                signature-1 {
                    algo = "sha256,rsa2048";
                    value = <...kernel signature 1...>
                };
            };
            kernel-2 {
                data = <data for kernel2>
                signature-1 {
                    algo = "sha256,rsa2048";
                    value = <...kernel signature 2...>
                };
            };
            fdt-1 {
                data = <data for fdt1>;
                signature-1 {
                    algo = "sha256,rsa2048";
                    value = <...fdt signature 1...>
                };
            };
            fdt-2 {
                data = <data for fdt2>;
                signature-1 {
                    algo = "sha256,rsa2048";
                    value = <...fdt signature 2...>
                };
            };
        };
        configurations {
            default = "conf-1";
            conf-1 {
                kernel = "kernel-1";
                fdt = "fdt-1";
            };
            conf-2 {
                kernel = "kernel-2";
                fdt = "fdt-2";
            };
        };
    };

Since both kernels are signed it is easy for an attacker to add a new
configuration 3 with kernel 1 and fdt 2::

    configurations {
        default = "conf-1";
        conf-1 {
            kernel = "kernel-1";
            fdt = "fdt-1";
        };
        conf-2 {
            kernel = "kernel-2";
            fdt = "fdt-2";
        };
        conf-3 {
            kernel = "kernel-1";
            fdt = "fdt-2";
        };
    };

With signed images, nothing protects against this. Whether it gains an
advantage for the attacker is debatable, but it is not secure.

To solve this problem, we support signed configurations. In this case it
is the configurations that are signed, not the image. Each image has its
own hash, and we include the hash in the configuration signature.

So the above example is adjusted to look like this::

    / {
        images {
            kernel-1 {
                data = <data for kernel1>
                hash-1 {
                    algo = "sha256";
                    value = <...kernel hash 1...>
                };
            };
            kernel-2 {
                data = <data for kernel2>
                hash-1 {
                    algo = "sha256";
                    value = <...kernel hash 2...>
                };
            };
            fdt-1 {
                data = <data for fdt1>;
                hash-1 {
                    algo = "sha256";
                    value = <...fdt hash 1...>
                };
            };
            fdt-2 {
                data = <data for fdt2>;
                hash-1 {
                    algo = "sha256";
                    value = <...fdt hash 2...>
                };
            };
        };
        configurations {
            default = "conf-1";
            conf-1 {
                kernel = "kernel-1";
                fdt = "fdt-1";
                signature-1 {
                    algo = "sha256,rsa2048";
                    value = <...conf 1 signature...>;
                };
            };
            conf-2 {
                kernel = "kernel-2";
                fdt = "fdt-2";
                signature-1 {
                    algo = "sha256,rsa2048";
                    value = <...conf 1 signature...>;
                };
            };
        };
    };


You can see that we have added hashes for all images (since they are no
longer signed), and a signature to each configuration. In the above example,
mkimage will sign configurations/conf-1, the kernel and fdt that are
pointed to by the configuration (/images/kernel-1, /images/kernel-1/hash-1,
/images/fdt-1, /images/fdt-1/hash-1) and the root structure of the image
(so that it isn't possible to add or remove root nodes). The signature is
written into /configurations/conf-1/signature-1/value. It can easily be
verified later even if the FIT has been signed with other keys in the
meantime.


Details
-------
The signature node contains a property ('hashed-nodes') which lists all the
nodes that the signature was made over.  The image is walked in order and each
tag processed as follows:

DTB_BEGIN_NODE
    The tag and the following name are included in the signature
    if the node or its parent are present in 'hashed-nodes'

DTB_END_NODE
    The tag is included in the signature if the node or its parent
    are present in 'hashed-nodes'

DTB_PROPERTY
    The tag, the length word, the offset in the string table, and
    the data are all included if the current node is present in 'hashed-nodes'
    and the property name is not 'data'.

DTB_END
    The tag is always included in the signature.

DTB_NOP
    The tag is included in the signature if the current node is present
    in 'hashed-nodes'

In addition, the signature contains a property 'hashed-strings' which contains
the offset and length in the string table of the strings that are to be
included in the signature (this is done last).

IMPORTANT:  To verify the signature outside u-boot, it is vital to not only
calculate the hash of the image and verify the signature with that, but also to
calculate the hashes of the kernel, fdt, and ramdisk images and check those
match the hash values in the corresponding 'hash*' subnodes.


Verification
------------
FITs are verified when loaded. After the configuration is selected a list
of required images is produced. If there are 'required' public keys, then
each image must be verified against those keys. This means that every image
that might be used by the target needs to be signed with 'required' keys.

This happens automatically as part of a bootm command when FITs are used.

For Signed Configurations, the default verification behavior can be changed by
the following optional property in /signature node in U-Boot's control FDT.

required-mode
    Valid values are "any" to allow verified boot to succeed if
    the selected configuration is signed by any of the 'required' keys, and "all"
    to allow verified boot to succeed if the selected configuration is signed by
    all of the 'required' keys.

This property can be added to a binary device tree using fdtput as shown in
below examples::

    fdtput -t s control.dtb /signature required-mode any
    fdtput -t s control.dtb /signature required-mode all


Enabling FIT Verification
-------------------------
In addition to the options to enable FIT itself, the following CONFIGs must
be enabled:

CONFIG_FIT_SIGNATURE
    enable signing and verification in FITs

CONFIG_RSA
    enable RSA algorithm for signing

CONFIG_ECDSA
    enable ECDSA algorithm for signing

WARNING: When relying on signed FIT images with required signature check
the legacy image format is default disabled by not defining
CONFIG_LEGACY_IMAGE_FORMAT


Testing
-------

An easy way to test signing and verification is to use the test script
provided in test/vboot/vboot_test.sh. This uses sandbox (a special version
of U-Boot which runs under Linux) to show the operation of a 'bootm'
command loading and verifying images.

A sample run is show below::

    $ make O=sandbox sandbox_config
    $ make O=sandbox
    $ O=sandbox ./test/vboot/vboot_test.sh


Simple Verified Boot Test
-------------------------

Please see :doc:`verified-boot` for more information::

    /home/hs/ids/u-boot/sandbox/tools/mkimage -D -I dts -O dtb -p 2000
    Build keys
    do sha1 test
    Build FIT with signed images
    Test Verified Boot Run: unsigned signatures:: OK
    Sign images
    Test Verified Boot Run: signed images: OK
    Build FIT with signed configuration
    Test Verified Boot Run: unsigned config: OK
    Sign images
    Test Verified Boot Run: signed config: OK
    check signed config on the host
    Signature check OK
    OK
    Test Verified Boot Run: signed config: OK
    Test Verified Boot Run: signed config with bad hash: OK
    do sha256 test
    Build FIT with signed images
    Test Verified Boot Run: unsigned signatures:: OK
    Sign images
    Test Verified Boot Run: signed images: OK
    Build FIT with signed configuration
    Test Verified Boot Run: unsigned config: OK
    Sign images
    Test Verified Boot Run: signed config: OK
    check signed config on the host
    Signature check OK
    OK
    Test Verified Boot Run: signed config: OK
    Test Verified Boot Run: signed config with bad hash: OK

    Test passed


Software signing: keydir vs keyfile
-----------------------------------

In the simplest case, signing is done by giving mkimage the 'keyfile'. This is
the path to a file containing the signing key.

The alternative is to pass the 'keydir' argument. In this case the filename of
the key is derived from the 'keydir' and the "key-name-hint" property in the
FIT. In this case the "key-name-hint" property is mandatory, and the key must
exist in "<keydir>/<key-name-hint>.<ext>" Here the extension "ext" is
specific to the signing algorithm.


Hardware Signing with PKCS#11 or with HSM
-----------------------------------------

Securely managing private signing keys can challenging, especially when the
keys are stored on the file system of a computer that is connected to the
Internet. If an attacker is able to steal the key, they can sign malicious FIT
images which will appear genuine to your devices.

An alternative solution is to keep your signing key securely stored on hardware
device like a smartcard, USB token or Hardware Security Module (HSM) and have
them perform the signing. PKCS#11 is standard for interfacing with these crypto
device.

Requirements:
    - Smartcard/USB token/HSM which can work with some openssl engine
    - openssl

For pkcs11 engine usage:
    - libp11 (provides pkcs11 engine)
    - p11-kit (recommended to simplify setup)
    - opensc (for smartcards and smartcard like USB devices)
    - gnutls (recommended for key generation, p11tool)

For generic HSMs respective openssl engine must be installed and locateable by
openssl. This may require setting up LD_LIBRARY_PATH if engine is not installed
to openssl's default search paths.

PKCS11 engine support forms "key id" based on "keydir" and with
"key-name-hint". "key-name-hint" is used as "object" name (if not defined in
keydir). "keydir" (if defined) is used to define (prefix for) which PKCS11 source
is being used for lookup up for the key.

PKCS11 engine key ids
    "pkcs11:<keydir>;object=<key-name-hint>;type=<public|private>"

or, if keydir contains "object="
    "pkcs11:<keydir>;type=<public|private>"

or
    "pkcs11:object=<key-name-hint>;type=<public|private>",

Generic HSM engine support forms "key id" based on "keydir" and with
"key-name-hint". If "keydir" is specified for mkimage it is used as a prefix in
"key id" and is appended with "key-name-hint".

Generic engine key ids:
    "<keydir><key-name-hint>"

or
    "<  key-name-hint>"

In order to set the pin in the HSM, an environment variable "MKIMAGE_SIGN_PIN"
can be specified.

The following examples use the Nitrokey Pro using pkcs11 engine. Instructions
for other devices may vary.

Notes on pkcs11 engine setup:

Make sure p11-kit, opensc are installed and that p11-kit is setup to use opensc.
/usr/share/p11-kit/modules/opensc.module should be present on your system.


Generating Keys On the Nitrokey::

    $ gpg --card-edit

    Reader ...........: Nitrokey Nitrokey Pro (xxxxxxxx0000000000000000) 00 00
    Application ID ...: xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    Version ..........: 2.1
    Manufacturer .....: ZeitControl
    Serial number ....: xxxxxxxx
    Name of cardholder: [not set]
    Language prefs ...: de
    Sex ..............: unspecified
    URL of public key : [not set]
    Login data .......: [not set]
    Signature PIN ....: forced
    Key attributes ...: rsa2048 rsa2048 rsa2048
    Max. PIN lengths .: 32 32 32
    PIN retry counter : 3 0 3
    Signature counter : 0
    Signature key ....: [none]
    Encryption key....: [none]
    Authentication key: [none]
    General key info..: [none]

    gpg/card> generate
    Make off-card backup of encryption key? (Y/n) n

    Please note that the factory settings of the PINs are
    PIN = '123456' Admin PIN = '12345678'
    You should change them using the command --change-pin

    What keysize do you want for the Signature key? (2048) 4096
    The card will now be re-configured to generate a key of 4096 bits
    Note: There is no guarantee that the card supports the requested size.
    If the key generation does not succeed, please check the
    documentation of your card to see what sizes are allowed.
    What keysize do you want for the Encryption key? (2048) 4096
    The card will now be re-configured to generate a key of 4096 bits
    What keysize do you want for the Authentication key? (2048) 4096
    The card will now be re-configured to generate a key of 4096 bits
    Please specify how long the key should be valid.
    0 = key does not expire
    <n> = key expires in n days
    <n>w = key expires in n weeks
    <n>m = key expires in n months
    <n>y = key expires in n years
    Key is valid for? (0)
    Key does not expire at all
    Is this correct? (y/N) y

    GnuPG needs to construct a user ID to identify your key.

    Real name: John Doe
    Email address: john.doe@email.com
    Comment:
    You selected this USER-ID:
    "John Doe <john.doe@email.com>"

    Change (N)ame, (C)omment, (E)mail or (O)kay/(Q)uit? o


Using p11tool to get the token URL:

Depending on system configuration, gpg-agent may need to be killed first::

    $ p11tool --provider /usr/lib/opensc-pkcs11.so --list-tokens
    Token 0:
    URL: pkcs11:model=PKCS%2315%20emulated;manufacturer=ZeitControl;serial=000xxxxxxxxx;token=OpenPGP%20card%20%28User%20PIN%20%28sig%29%29
    Label: OpenPGP card (User PIN (sig))
    Type: Hardware token
    Manufacturer: ZeitControl
    Model: PKCS#15 emulated
    Serial: 000xxxxxxxxx
    Module: (null)


    Token 1:
    URL: pkcs11:model=PKCS%2315%20emulated;manufacturer=ZeitControl;serial=000xxxxxxxxx;token=OpenPGP%20card%20%28User%20PIN%29
    Label: OpenPGP card (User PIN)
    Type: Hardware token
    Manufacturer: ZeitControl
    Model: PKCS#15 emulated
    Serial: 000xxxxxxxxx
    Module: (null)

Use the portion of the signature token URL after "pkcs11:" as the keydir argument (-k) to mkimage below.


Use the URL of the token to list the private keys::

    $ p11tool --login --provider /usr/lib/opensc-pkcs11.so --list-privkeys \
    "pkcs11:model=PKCS%2315%20emulated;manufacturer=ZeitControl;serial=000xxxxxxxxx;token=OpenPGP%20card%20%28User%20PIN%20%28sig%29%29"
    Token 'OpenPGP card (User PIN (sig))' with URL 'pkcs11:model=PKCS%2315%20emulated;manufacturer=ZeitControl;serial=000xxxxxxxxx;token=OpenPGP%20card%20%28User%20PIN%20%28sig%29%29' requires user PIN
    Enter PIN:
    Object 0:
    URL: pkcs11:model=PKCS%2315%20emulated;manufacturer=ZeitControl;serial=000xxxxxxxxx;token=OpenPGP%20card%20%28User%20PIN%20%28sig%29%29;id=%01;object=Signature%20key;type=private
    Type: Private key
    Label: Signature key
    Flags: CKA_PRIVATE; CKA_NEVER_EXTRACTABLE; CKA_SENSITIVE;
    ID: 01

Use the label, in this case "Signature key" as the key-name-hint in your FIT.

Create the fitImage::

    $ ./tools/mkimage -f fit-image.its fitImage


Sign the fitImage with the hardware key::

    $ ./tools/mkimage -F -k \
    "pkcs11:model=PKCS%2315%20emulated;manufacturer=ZeitControl;serial=000xxxxxxxxx;token=OpenPGP%20card%20%28User%20PIN%20%28sig%29%29" \
    -K u-boot.dtb -N pkcs11 -r fitImage


Future Work
-----------

- Roll-back protection using a TPM is done using the tpm command. This can
  be scripted, but we might consider a default way of doing this, built into
  bootm.


Possible Future Work
--------------------

- More sandbox tests for failure modes
- Passwords for keys/certificates
- Perhaps implement OAEP
- Enhance bootm to permit scripted signature verification (so that a script
  can verify an image but not actually boot it)


.. sectionauthor:: Simon Glass <sjg@chromium.org>, 1-1-13
