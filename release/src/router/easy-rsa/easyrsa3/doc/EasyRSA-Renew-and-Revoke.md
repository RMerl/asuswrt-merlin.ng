Easy-RSA 3 Certificate Renewal and Revocation Documentation
===========================================================

This document explains how the **differing versions** of Easy-RSA 3 work
with regard to Renewal and Revocation of Certificates.

## In summary:

**Easy-RSA Version 3.1.7** provides the most flexible support of renewal.
This includes command `rewind-renew`, which is required to recover certificates
renewed by `renew` command version 1. However, this does **not** include renewing
any supported certificate attributes.

**Easy-RSA Version 3.2.1** is preferred for future support.

----

Reason codes available for revoke commands
------------------------------------------

The follow is an exhaustive list of available `reason` codes, with abbreviations:

- `us | uns* | unspecified`
- `kc | key* | keyCompromise`
- `cc | ca*  | CACompromise`
- `ac | aff* | affiliationChanged`
- `ss | sup* | superseded`
- `co | ces* | cessationOfOperation`
- `ch | cer* | certificateHold`

  `reason` must be one of these abbreviations/codes, otherwise not be used.

----

Easy-RSA version 3.2.x
======================
For **Easy-RSA Version 3.2.0**, command `renew` is NOT supported.

Please upgrade to Easy-RSA Version 3.2.1

For **Easy-RSA Version 3.2.1+**, command `renew` is supported.

The command `renew` has been rewritten and now supports the renewal of
supported attributes. During renewal, the certificate is inspected and all
supported attributes are applied to the renewed certificate, as they were
in the original.

User added attributes from `$EASYRSA_EXTRA_EXTS`, that are not supported,
are dropped.

If the renewed certificate requires unsupported attibutes or changing the
`commonName` then the following process, that of expiry and then signing a
new certificate from the original request file, is required.

The expiry and signing process is as follows:
1. Command `expire <NAME>`

   This will move an existing certificate from `pki/issued` to `pki/expired`,
   so that a new certificate can be signed, using the original request.

   Generally, renewing is required ONLY when a certificate is due to expire.
   This means that certificates moved to `pki/expired` are expected to be expired
   or to expire in the near future, however, this is not a requirement.

2. Command `sign-req <TYPE> <NAME>`

   Sign a new certificate. This allows ALL command line customisations to be used.

3. If required, command `revoke-expired` can be used to revoke an
   expired certificate in the `pki/expired` directory.

This approach allows original certificates, which have been edited during `sign-req`,
to be edited the same way.

----

Easy-RSA version 3.1.x
======================

Command Details: `renew`
------------------------

    easyrsa renew file-name-base [ cmd-opts ]

`renew` is **only** available since Easy-RSA version `3.0.6`

#### `renew` has three different versions:

 *  `renew` **Version 1**: Easy-RSA versions `3.0.6`, `3.0.7` and `3.0.8`.
    - Both certificate and private key are rebuilt.
    - Once a certificate has been renewed it **cannot** be revoked.

 *  `renew` **Version 2**: Easy-RSA versions `3.0.9` and `3.1.0`.
    - Both certificate and private key are rebuilt.
    - Once a certificate has been renewed it **can** be revoked.
    - Use command:

        `revoke-renewed file-name-base [ reason ]`

 *  `renew` **Version 3**: Easy-RSA versions `3.1.1` through `3.1.7`.
    - Only certificate is renewed.
    - The original `renew` command has been renamed to `rebuild`, which
      rebuilds both certificate and private key.

 *  `renew` **Version 4**: Easy-RSA version `3.2.0+`.
    - Only certificate is renewed.
    - Supports standard Easy-RSA X509 extension duplication.


Resolving issues with `renew` version 1
---------------------------------------

#### Upgrade Easy-RSA to version `3.1.1+` is required.

`renew` version 1 **rebuilds** the certificate and private key.

Once a certificate has been renewed by version 1, the files are saved in the
`renewed/` storage area by `serialNumber`. These files must be recovered by
using command:

    easyrsa rewind-renew serialNumber

Command `rewind-renew` is only available in Easy-RSA version `3.1.1` to `3.1.7`.

Once `rewind-renew` has recovered the files, the certificate can be revoked:

    easyrsa revoke-renewed file-name-base [ reason ]


Using `renew` version 2
-----------------------

#### Upgrade Easy-RSA to version `3.1.1+` is required.

`renew` version 2 **rebuilds** the certificate and private key.

Renewed certificate can be revoked:

    easyrsa revoke-renewed file-name-base [ reason ]


Using `renew` version 3
-----------------------

#### Upgrade Easy-RSA to version `3.1.1+` is required.

`renew` version 3 **renews** the certificate only.

Renewed certificate can be revoked:

    easyrsa revoke-renewed file-name-base [ reason ]

This is the preferred method to renew a certificate because the original
private key is still valid.

Using `renew` version 4
-----------------------

#### Upgrade Easy-RSA to version `3.2.0+` is required.

This is the most comprensive version of `renew`, which supports automatic
copying of Easy-RSA X509 extensions.


----

Easy-RSA Reporting tools for certificate status
-----------------------------------------------

Easy-RSA version `3.1.x`, also has the following tools to keep track of
certificate status:

    easyrsa [ --days=# ] show-expire [ file-name-base ]

  `show-expire` shows all certificates which will expire in given `--days`.

    easyrsa show-renew [ file-name-base ]

  `show-renew` shows all certificates which have been renewed, where the old
  certificate has not been revoked.

    easyrsa show-revoke [ file-name-base ]

  `show-revoke` shows all certificates which have been revoked.

----

About command `rebuild`
-----------------------

If `rebuild` is used then the output directory of old certificate, key and
request is also the `renewed` directory.  Use **`revoke-renewed`** to revoke
an old certificate/key pair, which has been _rebuilt_ by command `rebuild`.

----

Renew CA Certificate
====================

Easy-RSA Version `3.2.2+ includes command `renew-ca`, which will create a new
CA certificate using the original CA key.  This new certificate will completely
replace the previous CA certificate.  This command can be safely tested without
disturbing your current PKI. The command requires user confirmation before
installing the new CA certificate.  The old CA certificate is archived to the
file 'pki/expired-ca.list'.


Easy-RSA Version `3.2.1+` supports a simple way to effectively renew a CA Certificate.

**Preamble** - Specifically for use with OpenVPN:

When a CA certificate expires it must be replaced, this is unavoidable.
No matter what method is used to create a new or renewed CA certificate,
that CA certificate must be distributed to all of your servers and clients.

Please consider the method outlined here, which requires very little work:

1. **Before you do anything else -- Make a BACKUP of your current PKI.**

2. Use command `init-pki soft`

   This will reset your current PKI but will keep your `vars` setting file
   and your current Request files [CSR], in the `pki/reqs` directory.

   If you have an Easy-RSA generated TLS key for OpenVPN, that will also be
   preserved. However, it will NOT be used for new `inline` files. The file
   `pki/private/easyrsa-tls.key` will be moved to `pki/easyrsa-keepsafe-tls.key`,
   for safe keeping. Easy-RSA will display a warning that this key is still
   valid and possibly in use, before allowing another TLS key to be generated.

3. Use command `build-ca`

   (With or without password and other preferences)

   This will build a completely new CA Certificate and private key.

   Use option `--days` to extend the lifetime of your new CA.

4. Use command `sign-req <TYPE> <NAME>`

   (With or without other preferences, password is not relevant)

   This will use an existing Request to sign a new Certificate.

   This will NOT generate a new Private Key for each new Certificate.

   This will generate new `inline` files that can be distributed publicly.
   These `inline` files will not contain any security sensitive data.

   This means that you will have a new CA certificate and private key.
   And signed certificates for all of your users, including servers.

5. Distribute the new `inline` files to all members of your PKI/VPN.

   These new `inline` files will not contain the user private key or the
   OpenVPN Pre-shared TLS key.

   These new `inline` files can be used by OpenVPN, examples below:

   * specify: `--config <INLNE-FILE>` in the OpenVPN user config file.
   * Use copy/paste to add the new details to the OpenVPN user config file.
   * Use `cat` to append the `inline` file to the OpenVPN user config file.

   Note:
   `inline` files in the `pki/inline/private` directory include security keys,
   which MUST only be transmitted over a secure connection, such as `https`.

