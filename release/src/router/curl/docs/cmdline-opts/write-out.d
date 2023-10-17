c: Copyright (C) Daniel Stenberg, <daniel@haxx.se>, et al.
SPDX-License-Identifier: curl
Long: write-out
Short: w
Arg: <format>
Help: Use output FORMAT after completion
Category: verbose
Example: -w '%{response_code}\\n' $URL
Added: 6.5
See-also: verbose head
Multi: single
---
Make curl display information on stdout after a completed transfer. The format
is a string that may contain plain text mixed with any number of
variables. The format can be specified as a literal "string", or you can have
curl read the format from a file with "@filename" and to tell curl to read the
format from stdin you write "@-".

The variables present in the output format are substituted by the value or
text that curl thinks fit, as described below. All variables are specified as
%{variable_name} and to output a normal % you just write them as %%. You can
output a newline by using \\n, a carriage return with \\r and a tab space with
\\t.

The output is by default written to standard output, but can be changed with
%{stderr} and %output{}.

Output HTTP headers from the most recent request by using *%header{name}*
where *name* is the case insensitive name of the header (without the trailing
colon). The header contents are exactly as sent over the network, with leading
and trailing whitespace trimmed (added in 7.84.0).

Select a specific target destination file to write the output to, by using
*%output{name}* (added in curl 8.3.0) where *name* is the full file name. The
output following that instruction is then written to that file. More than one
*%output{}* instruction can be specified in the same write-out argument. If
the file name cannot be created, curl leaves the output destination to the one
used prior to the *%output{}* instruction. Use *%output{>>name}* to append
data to an existing file.

**NOTE:**
In Windows the %-symbol is a special symbol used to expand environment
variables. In batch files all occurrences of % must be doubled when using this
option to properly escape. If this option is used at the command prompt then
the % cannot be escaped and unintended expansion is possible.

The variables available are:
.RS
.TP 15
**certs**
Output the certificate chain with details. Supported only by the OpenSSL,
GnuTLS, Schannel and Secure Transport backends. (Added in 7.88.0)
.TP
**content_type**
The Content-Type of the requested document, if there was any.
.TP
**errormsg**
The error message. (Added in 7.75.0)
.TP
**exitcode**
The numerical exit code of the transfer. (Added in 7.75.0)
.TP
**filename_effective**
The ultimate filename that curl writes out to. This is only meaningful if curl
is told to write to a file with the --remote-name or --output
option. It's most useful in combination with the --remote-header-name
option. (Added in 7.26.0)
.TP
**ftp_entry_path**
The initial path curl ended up in when logging on to the remote FTP
server. (Added in 7.15.4)
.TP
**header_json**
A JSON object with all HTTP response headers from the recent transfer. Values
are provided as arrays, since in the case of multiple headers there can be
multiple values. (Added in 7.83.0)

The header names provided in lowercase, listed in order of appearance over the
wire. Except for duplicated headers. They are grouped on the first occurrence
of that header, each value is presented in the JSON array.
.TP
**http_code**
The numerical response code that was found in the last retrieved HTTP(S) or
FTP(s) transfer.
.TP
**http_connect**
The numerical code that was found in the last response (from a proxy) to a
curl CONNECT request. (Added in 7.12.4)
.TP
**http_version**
The http version that was effectively used. (Added in 7.50.0)
.TP
**json**
A JSON object with all available keys.
.TP
**local_ip**
The IP address of the local end of the most recently done connection - can be
either IPv4 or IPv6. (Added in 7.29.0)
.TP
**local_port**
The local port number of the most recently done connection. (Added in 7.29.0)
.TP
**method**
The http method used in the most recent HTTP request. (Added in 7.72.0)
.TP
**num_certs**
Number of server certificates received in the TLS handshake. Supported only by
the OpenSSL, GnuTLS, Schannel and Secure Transport backends.
(Added in 7.88.0)
.TP
**num_connects**
Number of new connects made in the recent transfer. (Added in 7.12.3)
.TP
**num_headers**
The number of response headers in the most recent request (restarted at each
redirect). Note that the status line IS NOT a header. (Added in 7.73.0)
.TP
**num_redirects**
Number of redirects that were followed in the request. (Added in 7.12.3)
.TP
**onerror**
The rest of the output is only shown if the transfer returned a non-zero error.
(Added in 7.75.0)
.TP
**proxy_ssl_verify_result**
The result of the HTTPS proxy's SSL peer certificate verification that was
requested. 0 means the verification was successful. (Added in 7.52.0)
.TP
**redirect_url**
When an HTTP request was made without --location to follow redirects (or when
--max-redirs is met), this variable shows the actual URL a redirect
*would* have gone to. (Added in 7.18.2)
.TP
**referer**
The Referer: header, if there was any. (Added in 7.76.0)
.TP
**remote_ip**
The remote IP address of the most recently done connection - can be either
IPv4 or IPv6. (Added in 7.29.0)
.TP
**remote_port**
The remote port number of the most recently done connection. (Added in 7.29.0)
.TP
**response_code**
The numerical response code that was found in the last transfer (formerly
known as "http_code"). (Added in 7.18.2)
.TP
**scheme**
The URL scheme (sometimes called protocol) that was effectively used. (Added in 7.52.0)
.TP
**size_download**
The total amount of bytes that were downloaded. This is the size of the
body/data that was transferred, excluding headers.
.TP
**size_header**
The total amount of bytes of the downloaded headers.
.TP
**size_request**
The total amount of bytes that were sent in the HTTP request.
.TP
**size_upload**
The total amount of bytes that were uploaded. This is the size of the
body/data that was transferred, excluding headers.
.TP
**speed_download**
The average download speed that curl measured for the complete download. Bytes
per second.
.TP
**speed_upload**
The average upload speed that curl measured for the complete upload. Bytes per
second.
.TP
**ssl_verify_result**
The result of the SSL peer certificate verification that was requested. 0
means the verification was successful. (Added in 7.19.0)
.TP
**stderr**
From this point on, the --write-out output is written to standard
error. (Added in 7.63.0)
.TP
**stdout**
From this point on, the --write-out output is written to standard output.
This is the default, but can be used to switch back after switching to stderr.
(Added in 7.63.0)
.TP
**time_appconnect**
The time, in seconds, it took from the start until the SSL/SSH/etc
connect/handshake to the remote host was completed. (Added in 7.19.0)
.TP
**time_connect**
The time, in seconds, it took from the start until the TCP connect to the
remote host (or proxy) was completed.
.TP
**time_namelookup**
The time, in seconds, it took from the start until the name resolving was
completed.
.TP
**time_pretransfer**
The time, in seconds, it took from the start until the file transfer was just
about to begin. This includes all pre-transfer commands and negotiations that
are specific to the particular protocol(s) involved.
.TP
**time_redirect**
The time, in seconds, it took for all redirection steps including name lookup,
connect, pretransfer and transfer before the final transaction was
started. time_redirect shows the complete execution time for multiple
redirections. (Added in 7.12.3)
.TP
**time_starttransfer**
The time, in seconds, it took from the start until the first byte is received.
This includes time_pretransfer and also the time the server needed to calculate
the result.
.TP
**time_total**
The total time, in seconds, that the full operation lasted.
.TP
**url**
The URL that was fetched. (Added in 7.75.0)
.TP
**url.scheme**
The scheme part of the URL that was fetched. (Added in 8.1.0)
.TP
**url.user**
The user part of the URL that was fetched. (Added in 8.1.0)
.TP
**url.password**
The password part of the URL that was fetched. (Added in 8.1.0)
.TP
**url.options**
The options part of the URL that was fetched. (Added in 8.1.0)
.TP
**url.host**
The host part of the URL that was fetched. (Added in 8.1.0)
.TP
**url.port**
The port number of the URL that was fetched. If no port number was specified,
but the URL scheme is known, that scheme's default port number is
shown. (Added in 8.1.0)
.TP
**url.path**
The path part of the URL that was fetched. (Added in 8.1.0)
.TP
**url.query**
The query part of the URL that was fetched. (Added in 8.1.0)
.TP
**url.fragment**
The fragment part of the URL that was fetched. (Added in 8.1.0)
.TP
**url.zoneid**
The zone id part of the URL that was fetched. (Added in 8.1.0)
.TP
**urle.scheme**
The scheme part of the effective (last) URL that was fetched. (Added in 8.1.0)
.TP
**urle.user**
The user part of the effective (last) URL that was fetched. (Added in 8.1.0)
.TP
**urle.password**
The password part of the effective (last) URL that was fetched. (Added in 8.1.0)
.TP
**urle.options**
The options part of the effective (last) URL that was fetched. (Added in 8.1.0)
.TP
**urle.host**
The host part of the effective (last) URL that was fetched. (Added in 8.1.0)
.TP
**urle.port**
The port number of the effective (last) URL that was fetched. If no port
number was specified, but the URL scheme is known, that scheme's default port
number is shown. (Added in 8.1.0)
.TP
**urle.path**
The path part of the effective (last) URL that was fetched. (Added in 8.1.0)
.TP
**urle.query**
The query part of the effective (last) URL that was fetched. (Added in 8.1.0)
.TP
**urle.fragment**
The fragment part of the effective (last) URL that was fetched. (Added in 8.1.0)
.TP
**urle.zoneid**
The zone id part of the effective (last) URL that was fetched. (Added in 8.1.0)
.TP
**urlnum**
The URL index number of this transfer, 0-indexed. Unglobbed URLs share the
same index number as the origin globbed URL. (Added in 7.75.0)
.TP
**url_effective**
The URL that was fetched last. This is most meaningful if you have told curl
to follow location: headers.
.RE
.IP
