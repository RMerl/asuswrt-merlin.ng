Easy-RSA Advanced Reference
=============================

This is a technical reference for advanced users familiar with PKI processes. If
you need a more detailed description, see the `EasyRSA-Readme` or `Intro-To-PKI`
docs instead.

Configuration Reference
-----------------------

#### Configuration Sources

  There are 3 possible ways to perform external configuration of Easy-RSA,
  selected in the following order where the first defined result wins:

  1. Command-line option
  2. Environmental variable
  3. 'vars' file, if one is present (see `vars Autodetection` below)
  4. Built-in default

  Note that not every possible config option can be set everywhere, although any
  env-var can be added to the 'vars' file even if it's not shown by default.

#### vars Autodetection

  A 'vars' file is a file named simply `vars` (without an extension) that
  Easy-RSA will source for configuration. This file is specifically designed
  *not* to replace variables that have been set with a higher-priority method
  such as CLI opts or env-vars.

  The following locations are checked, in this order, for a vars file. Only the
  first one found is used:

  1. The file referenced by the `--vars` CLI option
  2. The file referenced by the env-var named `EASYRSA_VARS_FILE`
  3. The directory referenced by the `--pki` CLI option (Recommended)
  4. The directory referenced by the `EASYRSA_PKI` env-var
  5. The directory referenced by the `EASYRSA` env-var
  6. The default PKI directory at `$PWD/pki` (See note below)
  7. The default working directory at `$PWD`

  Defining the env-var `EASYRSA_NO_VARS` will override the sourcing of the vars
  file in all cases, including defining it subsequently as a global option.

  Note: If the vars file `$PWD/pki/vars` is sourced then it is forbidden from
        setting/changing the current PKI, as defined by `EASYRSA_PKI` env-var.

#### Use of `--pki` verses `--vars`

  It is recommended to use option `--pki=DIR` to define your PKI at runtime.
  This method will always auto-load the `vars` file found in defined PKI.

  In a multi-PKI installation, use of `--vars` can potentially lead to
  a vars file that is configured to set a PKI which cannot be verified
  as the expected PKI. Use of `--vars` is not recommended.

#### OpenSSL Config

  Easy-RSA is tightly coupled to the OpenSSL config file (.cnf) for the
  flexibility the script provides. It is required that this file be available,
  yet it is possible to use a different OpenSSL config file for a particular
  PKI, or even change it for a particular invocation.

  The OpenSSL config file is searched for in the following order:

  1. The env-var `EASYRSA_SSL_CONF`
  2. The 'vars' file (see `vars Autodetection` above)
  3. The `EASYRSA_PKI` directory with a filename of `openssl-easyrsa.cnf`
  4. The `EASYRSA` directory with a filename of `openssl-easyrsa.cnf`

Advanced extension handling
---------------------------

Normally the cert extensions are selected by the cert type given on the CLI
during signing; this causes the matching file in the x509-types subdirectory to
be processed for OpenSSL extensions to add. This can be overridden in a
particular PKI by placing another x509-types dir inside the `EASYRSA_PKI` dir
which will be used instead.

The file named `COMMON` in the x509-types dir is appended to every cert type;
this is designed for CDP usage, but can be used for any extension that should
apply to every signed cert.

Additionally, the contents of the env-var `EASYRSA_EXTRA_EXTS` is appended with
its raw text added to the OpenSSL extensions. The contents are appended as-is to
the cert extensions; invalid OpenSSL configs will usually result in failure.

Advanced configuration files
----------------------------

The following files are used by Easy-RSA to configure the SSL library:
* openssl-easyrsa.cnf - Configuration for Certificate Authority [CA]
* x509-types: COMMON, ca, server, serverClient, client, codeSigning, email, kdc.
  Each type is used to define an X509 purpose.

Since Easy-RSA version 3.2.0, these files are created on-demand by each command
that requires them.  However, if these files are found in one of the supported
locations then those files are used instead, no temporary files are created.

The supported locations are listed, in order of preference, as follows:
* `EASYRSA_PKI` - Always preferred.
* `EASYRSA` - For Windows.
* `PWD` - For Windows.
* `easyrsa` script directory - DEPRECATED, will be removed. Only for Windows.
* `/usr/local/share/easy-rsa`
* `/usr/share/easy-rsa`
* `/etc/easy-rsa`

The files above can all be created by using command: `easyrsa write legacy <DIR>`
To OVER-WRITE any existing files use command: `eaysrsa write legacy-hard <DIR>`
`<DIR>` is optional, the default is `EASYRSA_PKI`. This will create the files in
the current PKI or `<DIR>`.  If created then these new files may take priority
over system wide versions of the same files.  See `help write` for further details.

Note, Over-writing files:
Only command `write legacy-hard` will over-write files. All other uses of `write`
will leave an existing file intact, without error. If you want to over-write an
existing file using `write` then you must redirect `>foo` the output manually.

Example command: `easyrsa write vars >vars` - This will over-write `./vars`.

Environmental Variables Reference
---------------------------------

A list of env-vars, any matching global option (CLI) to set/override it, and a
short description is shown below:

 *  `EASYRSA` - should point to the Easy-RSA top-level dir, where the easyrsa
    script is located.
 *  `EASYRSA_OPENSSL` - command to invoke openssl
 *  `EASYRSA_SSL_CONF` - the openssl config file to use
 *  `EASYRSA_PKI` (CLI: `--pki-dir`) - dir to use to hold all PKI-specific
    files, defaults to `$PWD/pki`.
 *  `EASYRSA_VARS_FILE` (CLI: `--vars`) - Set the `vars` file to use
 *  `EASYRSA_DN` (CLI: `--dn-mode`) - set to the string `cn_only` or `org` to
    alter the fields to include in the req DN
 *  `EASYRSA_REQ_COUNTRY` (CLI: `--req-c`) - set the DN country with org mode
 *  `EASYRSA_REQ_PROVINCE` (CLI: `--req-st`) - set the DN state/province with
    org mode
 *  `EASYRSA_REQ_CITY` (CLI: `--req-city`) - set the DN city/locality with org
    mode
 *  `EASYRSA_REQ_ORG` (CLI: `--req-org`) - set the DN organization with org mode
 *  `EASYRSA_REQ_EMAIL` (CLI: `--req-email`) - set the DN email with org mode
 *  `EASYRSA_REQ_OU` (CLI: `--req-ou`) - set the DN organizational unit with org
    mode
 *  `EASYRSA_REQ_SERIAL` (CLI: `--req-serial`) - set the DN serialNumber with
    org mode (OID 2.5.4.5)
 *  `EASYRSA_KEY_SIZE` (CLI: `--keysize`) - set the key size in bits to
    generate
 *  `EASYRSA_ALGO` (CLI: `--use-algo`) - set the crypto alg to use: rsa, ec or
    ed
 *  `EASYRSA_CURVE` (CLI: `--curve`) - define the named EC curve to use
 *  `EASYRSA_CA_EXPIRE` (CLI: `--days`) - set the CA expiration time in days
 *  `EASYRSA_CERT_EXPIRE` (CLI: `--days`) - set the issued cert expiration time
    in days
 *  `EASYRSA_CRL_DAYS` (CLI: `--days`) - set the CRL 'next publish' time in days
 *  `EASYRSA_NS_SUPPORT` (CLI: `--ns-cert`) - string 'yes' or 'no' fields to
    include the **deprecated** Netscape extensions
 *  `EASYRSA_NS_COMMENT` (CLI: `--ns-comment`) - string comment to include when
    using the **deprecated** Netscape extensions
 *  `EASYRSA_REQ_CN` (CLI: `--req-cn`) - default CN, can only be used in BATCH
    mode
 *  `EASYRSA_DIGEST` (CLI: `--digest`) - set a hash digest to use for req/cert
    signing
 *  `EASYRSA_BATCH` (CLI: `--batch`) - enable batch (no-prompt) mode; set
    env-var to non-zero string to enable (CLI takes no options)
 *  `EASYRSA_PASSIN` (CLI: `--passin`) - allows to specify a source for
    password using any openssl password options like pass:1234 or env:var
 *  `EASYRSA_PASSOUT` (CLI: `--passout`) - allows to specify a source for
    password using any openssl password options like pass:1234 or env:var
 *  `EASYRSA_NO_PASS` (CLI: `--nopass`) - disable use of passwords
 *  `EASYRSA_UMASK` - safe umask to use for file creation. Defaults to `077`
 *  `EASYRSA_NO_UMASK` - disable safe umask. Files will be created using the
    system's default
 *  `EASYRSA_TEMP_DIR` (CLI: `--tmp-dir`) - a temp directory to use for temporary files
**NOTE:** the global options must be provided before the commands.
