=========
rscryutil
=========

--------------------------
Manage Encrypted Log Files
--------------------------

:Author: Rainer Gerhards <rgerhards@adiscon.com>
:Date: 2013-04-15
:Manual section: 1

SYNOPSIS
========

::

   rscryutil [OPTIONS] [FILE] ...


DESCRIPTION
===========

This tool performs various operations on encrypted log files.
Most importantly, it provides the ability to decrypt them.


OPTIONS
=======

-d, --decrypt
  Select decryption mode. This is the default mode.

-W, --write-keyfile <file>
  Utility function to write a key to a keyfile. The key can be obtained
  via any method.

-v, --verbose
  Select verbose mode.

-f, --force
  Forces operations that otherwise would fail.

-k, --keyfile <file>
  Reads the key from <file>. File _must_ contain the key, only, no headers
  or other meta information. Keyfiles can be generated via the
  *--write-keyfile* option.

-p, --key-program <path-to-program>
 In this mode, the key is provided by a so-called "key program". This program
 is executed and must return the key to (as well as some meta information)
 via stdout. The core idea of key programs is that using this interface the
 user can implement as complex (and secure) method to obtain keys as
 desired, all without the need to make modifications to rsyslog.

-K, --key <KEY>
  TESTING AID, NOT FOR PRODUCTION USE. This uses the KEY specified
  on the command line. This is the actual key, and as such this mode
  is highly insecure. However, it can be useful for intial testing
  steps. This option may be removed in the future.

-a, --algo <algo>
  Sets the encryption algorightm (cipher) to be used. See below
  for supported algorithms. The default is "AES128".

-m, --mode <mode>
  Sets the ciphermode to be used. See below for supported modes.
  The default is "CBC".

-r, --generate-random-key <bytes>
  Generates a random key of length <bytes>. This option is
  meant to be used together with *--write-keyfile* (and it is hard
  to envision any other valid use for it).

OPERATION MODES
===============

The operation mode specifies what exactly the tool does with the provided
files. The default operation mode is "dump", but this may change in the future.
Thus, it is recommended to always set the operations mode explicitely. If 
multiple operations mode are set on the command line, results are 
unpredictable.

decrypt
-------

The provided log files are decrypted. Note that the *.encinfo* side files
must exist and be accessible in order for decryption to to work.

write-keyfile
-------------

In this mode no log files are processed; thus it is an error to specify
any on the command line. The specified keyfile is written. The key itself
is obtained via the usual key commands. If *--keyfile* is used, that
file is effectively copied.

For security reasons, existing key files are _not_ overwritten. To permit
this, specify the *--force* option. When doing so, keep in mind that lost
keys cannot be recovered and data encrypted with them may also be considered
lost.

Keyfiles are always created with 0400 permission, that is read access for only
the user. An exception is when an existing file is overwritten via the
*--force* option, in which case the former permissions still apply.

EXIT CODES
==========

The command returns an exit code of 0 if everything went fine, and some 
other code in case of failures.


SUPPORTED ALGORITHMS
====================

We basically support what libgcrypt supports. This is:

	3DES
	CAST5
	BLOWFISH
	AES128
	AES192
	AES256
	TWOFISH
	TWOFISH128
	ARCFOUR
	DES
	SERPENT128
	SERPENT192
	SERPENT256
	RFC2268_40
	SEED
	CAMELLIA128
	CAMELLIA192
	CAMELLIA256


SUPPORTED CIPHER MODES
======================

We basically support what libgcrypt supports. This is:

  	ECB
	CFB
	CBC
	STREAM
	OFB
	CTR
	AESWRAP

EXAMPLES
========

**rscryutil logfile**

Decrypts "logfile" and sends data to stdout.


**rscryutil --generate-random-key 16 --keyfile /some/secured/path/keyfile**

Generates random key and stores it in the specified keyfile.

LOG SIGNATURES
==============

Encrypted log files can be used together with signing. To verify such a file,
it must be decrypted first, and the verification tool **rsgtutil(1)** must be
run on the decrypted file.

SECURITY CONSIDERATIONS
=======================

Specifying keys directly on the command line (*--key* option) is very 
insecure and should
not be done, except for testing purposes with test keys. Even then it is
recommended to use keyfiles, which are also easy to handle during testing.
Keep in mind that command history is usally be kept by bash and can also
easily be monitored.

Local keyfiles are also a security risk. At a minimum, they should be
used with very restrictive file permissions. For this reason,
the *rscryutil* tool creates them with read permissions for the user,
only, no matter what umask is set to.

When selecting cipher algorithms and modes, care needs to be taken. The
defaults should be reasonable safe to use, but this tends to change over
time. Keep up with the most current crypto recommendations.


SEE ALSO
========
**rsgtutil(1)**, **rsyslogd(8)**

COPYRIGHT
=========

This page is part of the *rsyslog* project, and is available under
LGPLv2.
