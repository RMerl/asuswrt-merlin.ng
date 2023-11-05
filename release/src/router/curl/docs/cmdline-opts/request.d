c: Copyright (C) Daniel Stenberg, <daniel@haxx.se>, et al.
SPDX-License-Identifier: curl
Long: request
Short: X
Arg: <method>
Help: Specify request method to use
Category: connection
Example: -X "DELETE" $URL
Example: -X NLST ftp://example.com/
Added: 6.0
See-also: request-target
Multi: single
---
Change the method to use when starting the transfer.
.RS
.TP 15
**HTTP**
Specifies a custom request method to use when communicating with the HTTP
server. The specified request method is used instead of the method otherwise
used (which defaults to *GET*). Read the HTTP 1.1 specification for details
and explanations. Common additional HTTP requests include *PUT* and *DELETE*,
but related technologies like WebDAV offers *PROPFIND*, *COPY*, *MOVE* and
more.

Normally you do not need this option. All sorts of *GET*, *HEAD*, *POST* and
*PUT* requests are rather invoked by using dedicated command line options.

This option only changes the actual word used in the HTTP request, it does not
alter the way curl behaves. So for example if you want to make a proper HEAD
request, using -X HEAD does not suffice. You need to use the --head option.

The method string you set with --request is used for all requests, which
if you for example use --location may cause unintended side-effects when curl
does not change request method according to the HTTP 30x response codes - and
similar.
.TP
**FTP**
Specifies a custom FTP command to use instead of *LIST* when doing file lists
with FTP.
.TP
**POP3**
Specifies a custom POP3 command to use instead of *LIST* or *RETR*.
(Added in 7.26.0)
.TP
**IMAP**
Specifies a custom IMAP command to use instead of *LIST*. (Added in 7.30.0)
.TP
**SMTP**
Specifies a custom SMTP command to use instead of *HELP* or **VRFY**. (Added in 7.34.0)
.RE
.IP
