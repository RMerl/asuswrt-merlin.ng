rlm_mschap
==========

The mschap module provides support for MS-CHAPv1 and MS-CHAPv2, which is
a common authentication mechanisms for Microsoft clients.

If you want to support mschap, there are only 3 possibilities:

 1. You have access to the users plaintext password, and you configure
    FreeRADIUS to read this, and set the Cleartext-Password control attribute.

 2. You have access to the NT (MS-CHAPv2) or LM (MS-CHAPv1) hashes,
    and you configure FreeRADIUS to read this and set the NT/LM-Password
    control attribute.

 3. You have Samba installed, joined into a windows domain, and use
    the ntlm_auth helper binary to pass authentication onwards to
    a domain controller.

These are the ONLY possibilities; MS-CHAP is IMPOSSIBLE if you e.g. only
have the unix/md5/sha crypt of your users password.

For more info, see:

 http://deployingradius.com/documents/protocols/compatibility.html

EAP-MSCHAPv2
============

The EAP module provides MS-CHAPv2 support as well. It simply passes the
data through to the mschap module, so you must configure mschap properly.

ntlm_auth
=========

Method 3 above involves configuring the mschap module to call the Samba
ntlm_auth helper:

::

  mschap {
    ntlm_auth = "/path/to/bin ..."
  }

You need to be careful about setting this command line. There are several
options for the arguments, in particular username and domain:

 * --username=%{User-Name} - this will fail if you're using realms or host-based auth
 * --username=%{mschap:User-Name} - this will fail if using using suffix i.e. user@domain

You'll need to fit this to your local needs.

Disabling ntlm_auth for some users
----------------------------------

You might have some users in the domain, and others in files or SQL that you
want to authenticate locally. To do this, set::

 MS-CHAP-Use-NTLM-Auth := 0

This will disable ntlm_auth for that user/group. This is also obeyed
for password changes (see below).

Password changes
================

From FreeRADIUS version 3.0.0 the mschap module supports password changes.

There are two options, ntlm_auth and local.

ntlm_auth
---------

If you are using ntlm_auth to check passwords, you must also use
ntlm_auth to change passwords. In modules/mschap you should configure::

  mschap {
    ntlm_auth = "...."
    passchange {

      # path to the binary
      ntlm_auth = "/path/to/ntlm_auth --helper-protocol=ntlm-change-password-1"

      # initial data to send
      # this MUST be supplied
      ntlm_auth_username = "username: %{mschap:User-Name}"
      ntlm_auth_domain = "nt-domain: %{%{mschap:NT-Domain}:-YOURDOMAIN}"

      # Or, you could try:
      ntlm_auth_username = "full-username: %{User-Name}"
      # ntlm_auth_domain - disabled

    }


If you are using ntlm_auth, then domain controllers might say
"Password expired" if the user password is valid but has expired; the
mschap module will detect this and return error 648 to the client,
instructing it to try a password change.

Note: if you have disabled ntlm_auth for a user/group, this will apply
for password changes too - they will fall through to using the Local
method.

Local
-----

If you are performing mschap locally with Cleartext-Password/NT-Password, you
can decrypt and process the password change locally. To do this, you configure
the "local_cpw" string::

  mschap {
    passchange {
      local_cpw = "%{xlat:...}
    }
  }

To actually force a client to change passwords, you must set the expiry bit
in the SMB-Account-Ctrl value - for example::

  update control {
    # U == user
    # e == expired
    SMB-Account-Ctrl-Text := '[Ue]'
  }

This will cause the client to receive "error 648 - password
expired". Obviously you will need to ensure your local_cpw xlat clears
this value, or else the client password will be expired the next time
they log in. For example, you might use an SQL stored procedure to
change passwords::

  mschap {
    passchange {
      local_cpw = "%{sql:select change_password('%{SQL-User-Name}','%{MS-CHAP-New-NT-Password}')}"
    }
  }

...and an example stored procedure for Postgres might be::

  CREATE FUNCTION change_password(raduser text, ntpassword text) RETURNS text
      LANGUAGE plpgsql
      AS $$
  BEGIN
          update radcheck set value=ntpassword where username=raduser and attribute='NT-Password';
          if not FOUND then
                  -- the user does not exist; die
                  return '';
          end if;
          update radcheck set value=replace(value,'e','') where username=raduser and attribute='SMB-Account-Ctrl-Text' and value like '%e%';
          return 'ok';
  END;
  $$;


The local_cpw xlat has access to two variables:

 * MS-CHAP-New-NT-Password        - the new value of NT-Password
 * MS-CHAP-New-Cleartext-PAssword - the new value of Cleartext-Password

This allows you to do things like::

  # update via SQL
  local_cpw = "%{sql:update radcheck set value='%{MS-CHAP-New-NT-Password}' where username='%{SQL-User-Name} and attribute='NT-Password'}"

Or::

  # update via exec/script
  local_cpw = "%{exec:/my/script %{User-Name} %{MS-CHAP-New-Cleartext-Password}}"

WARNING - wherever possible, you should use
MS-CHAP-New-NT-Password. The reason is that cleartext passwords have
undergone unicode transformation from the client encoding (utf-16) to
the server encoding (utf-8) and the current code does this in a very
ad-hoc way. The reverse transformation is also not done - when the
server reads Cleartext-Password out of files/database, it assumes
US-ASCII and thus international characters will fail.

N.B. this could be fixed, if we wanted to pull in something like iconv.

In addition, you should beware of Cleartext-Password when using SQL;
any password character not in safe_characters will be encoded as a hex
number, e.g. =20.

Password changes over EAP
=========================

You must set the following in eap.conf::

 eap {
   mschapv2 {
     send_error = yes
   }
 }

Otherwise password changes for PEAP/MSCHAPv2 will not work.
