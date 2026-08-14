Easy-RSA 3 Documentation Readme
===============================

This document explains how Easy-RSA 3 and each of its assorted features work.

If you are looking for a quickstart with less background or detail, an
implementation-specific How-to or Readme may be available in this (the [`doc/`](./))
directory.

Easy-RSA Overview
-----------------

Easy-RSA is a utility for managing X.509 PKI, or Public Key Infrastructure. A
PKI is based on the notion of trusting a particular authority to authenticate a
remote peer; for more background on how PKI works, see the [Intro-To-PKI](Intro-To-PKI.md)
document.

The code is written in platform-neutral POSIX shell, allowing use on a wide
range of host systems. The official Windows release also comes bundled with the
programs necessary to use Easy-RSA. The shell code attempts to limit the number
of external programs it depends on. Crypto-related tasks use openssl as the
functional backend.

Feature Highlights
------------------

Here's a non-exhaustive list of the more notable Easy-RSA features:

 *  Easy-RSA is able to manage multiple PKIs, each with their own independent
    configuration, storage directory, and X.509 extension handling.
 *  Multiple Subject Name (X.509 DN field) formatting options are supported. For
    VPNs, this means a cleaner commonName only setup can be used.
 *  A single backend is used across all supported platforms, ensuring that no
    platform is 'left out' of the rich features. Unix-alikes (BSD, Linux, etc)
    and Windows are all supported.
 *  Easy-RSA's X.509 support includes CRL, CDP, keyUsage/eKu attributes, and
    additional features. The included support can be changed or extended as an
    advanced feature.
 *  Interactive and automated (batch) modes of operation
 *  Flexible configuration: features can be enabled through command-line
    options, environment variables, a config file, or a combination of these.
 *  Built-in defaults allow Easy-RSA to be used without first editing a config
    file.

Obtaining and Using Easy-RSA
----------------------------

#### Download and extraction (installation)

  Easy-RSA's main program is a script, supported by a couple of config files. As
  such, there is no formal "installation" required. Preparing to use Easy-RSA is
  as simple as downloading the compressed package (.tar.gz for Linux/Unix or
  .zip for Windows) and extract it to a location of your choosing. There is no
  compiling or OS-dependent setup required.

  You should install and run Easy-RSA as a non-root (non-Administrator) account
  as root access is not required.

#### Running Easy-RSA

  Invoking Easy-RSA is done through your preferred shell. Under Windows, you
  will use the `EasyRSA Start.bat` program to provide a POSIX-shell environment
  suitable for using Easy-RSA.

  The basic format for running commands is:

    ./easyrsa command [ cmd-opts ]

  where `command` is the name of a command to run, and `cmd-opts` are any
  options to supply to the command. Some commands have mandatory or optional
  cmd-opts. Note the leading `./` component of the command: this is required in
  Unix-like environments and may be a new concept to some Windows users.

  General usage and command help can be shown with:

    ./easyrsa help [ command ]

  When run without any command, general usage and a list of available commands
  are shown; when a command is supplied, detailed help output for that command
  is shown.

Configuring Easy-RSA
--------------------

Easy-RSA 3 no longer needs any configuration file prior to operation, unlike
earlier versions. However, the `vars.example` file contains many commented
options that can be used to control non-default behavior as required. Reading
this file will provide an idea of the basic configuration available. Note that
a vars file must be named just `vars` (without an extension) to actively use it.

Additionally, some options can be defined at runtime with options on the
command-line. A full list can be shown with:

    ./easyrsa help options

Any of these options can appear before the command as required as shown below:

    ./easyrsa [options] command [ cmd-opts ]

For experts, additional configuration with env-vars and custom X.509 extensions
is possible. Consult the [EasyRSA-Advanced](EasyRSA-Advanced.md) documentation for details.

Getting Started: The Basics
---------------------------

Some of the terms used here will be common to those familiar with how PKI works.
Instead of describing PKI basics, please consult the document [Intro-To-PKI](Intro-To-PKI.md) if
you need a more basic description of how a PKI works.

#### Creating an Easy-RSA PKI

  In order to do something useful, Easy-RSA needs to first initialize a
  directory for the PKI. Multiple PKIs can be managed with a single installation
  of Easy-RSA, but the default directory is called simply "pki" unless otherwise
  specified.

  To create or clear out (re-initialize) a new PKI, use the command:

    ./easyrsa init-pki

  which will create a new, blank PKI structure ready to be used. Once created,
  this PKI can be used to make a new CA or generate keypairs.

#### The PKI Directory Structure

  An Easy-RSA PKI contains the following directory structure:

  * `private/` - dir with private keys generated on this host
  * `reqs/` - dir with locally generated certificate requests (for a CA imported
    requests are stored here)

  In a clean PKI no files exist yet, just the bare directories. Commands called
  later will create the necessary files depending on the operation.

  When building a CA, a number of new files are created by a combination of
  Easy-RSA and (indirectly) openssl. The important CA files are:

  * `ca.crt` - This is the CA certificate
  * `index.txt` - This is the "master database" of all issued certs
  * `serial` - Stores the next serial number (serial numbers increment)
  * `private/ca.key` - This is the CA private key (security-critical)
  * `certs_by_serial/` - dir with all CA-signed certs by serial number
  * `issued/` - dir with issued certs by commonName

#### After Creating a PKI

  Once you have created a PKI, the next useful step will be to either create a
  CA, or generate keypairs for a system that needs them. Continue with the
  relevant section below.

Using Easy-RSA as a CA
----------------------

#### Building the CA

  In order to sign requests to produce certificates, you need a CA. To create a
  new CA in the PKI you have created, run:

    ./easyrsa build-ca

  Be sure to use a strong passphrase to protect the CA private key. Note that
  you must supply this passphrase in the future when performing signing
  operations with your CA, so be sure to remember it.

  During the creation process, you will also select a name for the CA called the
  Common Name (CN.) This name is purely for display purposes and can be set as
  you like.

#### Importing requests to the CA

  Once a CA is built, the PKI is intended to be used to import requests from
  external systems that are requesting a signed certificate from this CA. In
  order to sign the request, it must first be imported so Easy-RSA knows about
  it. This request file must be a standard CSR in PKCS#10 format.

  Regardless of the file name to import, Easy-RSA uses a "short name" defined
  during import to refer to this request. Importing works like this:

    ./easyrsa import-req /path/to/request.req nameOfRequest

  The nameOfRequest should normally refer to the system or person making the
  request.

#### Signing a request

  Once Easy-RSA has imported a request, it can be reviewed and signed:

    ./easyrsa sign-req <type> nameOfRequest

  Every certificate needs a `type` which controls what extensions the certificate
  gets.

  Easy-RSA ships with 4 possible "types":

  * `client` - A TLS client, suitable for a VPN user or web browser (web client)
  * `server` - A TLS server, suitable for a VPN or web server
  * `ca` - A intermediate CA, used when chaining multiple CAs together
  * `serverClient` - A TLS server and TLS client

  Additional types of certs may be defined by local sites as needed; see the
  advanced documentation for details.

#### Revoking and publishing Certificate Revocation Lists (CRLs)

  If an issue certificate needs to be revoked, this can be done as follows:

    ./easyrsa revoke nameOfRequest

  To generate a CRL suitable for publishing to systems that use it, run:

    ./easyrsa gen-crl

  Note that this will need to be published or sent to systems that rely on an
  up-to-date CRL as the certificate is still valid otherwise.

Using Easy-RSA to generate keypairs & requests
----------------------------------------------

Easy-RSA can generate a keypair and certificate request in PKCS#10 format. This
request is what a CA needs in order to generate and return a signed certificate.

Ideally you should never generate entity keypairs for a client or server in a
PKI you are using for your CA. It is best to separate this process and generate
keypairs only on the systems you plan to use them.

Easy-RSA can generate a keypair and request with the following command:

    ./easyrsa gen-req nameOfRequest

You will then be given a chance to modify the Subject details of your request.
Easy-RSA uses the short name supplied on the command-line by default, though you
are free to change it if necessary. After providing a passphrase and Subject
details, the keypair and request files will be shown.

In order to obtain a signed certificate, the request file must be sent to the
CA for signing; this step is obviously not required if a single PKI is used as
both the CA and keypair/request generation as the generated request is already
"imported."
