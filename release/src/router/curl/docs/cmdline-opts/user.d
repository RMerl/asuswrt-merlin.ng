c: Copyright (C) Daniel Stenberg, <daniel@haxx.se>, et al.
SPDX-License-Identifier: curl
Long: user
Short: u
Arg: <user:password>
Help: Server user and password
Category: important auth
Example: -u user:secret $URL
Added: 4.0
See-also: netrc config
Multi: single
---
Specify the user name and password to use for server authentication. Overrides
--netrc and --netrc-optional.

If you simply specify the user name, curl prompts for a password.

The user name and passwords are split up on the first colon, which makes it
impossible to use a colon in the user name with this option. The password can,
still.

On systems where it works, curl hides the given option argument from process
listings. This is not enough to protect credentials from possibly getting seen
by other users on the same system as they still are visible for a brief moment
before cleared. Such sensitive data should be retrieved from a file instead or
similar and never used in clear text in a command line.

When using Kerberos V5 with a Windows based server you should include the
Windows domain name in the user name, in order for the server to successfully
obtain a Kerberos Ticket. If you do not, then the initial authentication
handshake may fail.

When using NTLM, the user name can be specified simply as the user name,
without the domain, if there is a single domain and forest in your setup
for example.

To specify the domain name use either Down-Level Logon Name or UPN (User
Principal Name) formats. For example, EXAMPLE\\user and user@example.com
respectively.

If you use a Windows SSPI-enabled curl binary and perform Kerberos V5,
Negotiate, NTLM or Digest authentication then you can tell curl to select
the user name and password from your environment by specifying a single colon
with this option: "-u :".
