Acct-Type
=========

FreeRADIUS supports the Acct-Type attribute to select between
accounting methods based on arbitrary attribute/value pairs contained
in an accounting packet. Its use follows the same general configuration
syntax as Auth-Type and Autz-Type. The main difference in configuration
between Acct-Type and Auth/Autz-Type lies in where the Acct-Type
method is assigned. With Auth/Autz-Type, the method is typically
assigned in the 'users' file. The 'users' file, naturally, is not
processed during the handling of the accounting {} section. However,
part of the default files {} module is the 'acct_users' file, which
serves the same purpose as the 'users' file, but applies to accounting
packets.

For example, a server administrator is responsible for handling the
accounting data for two different realms, foo.com and bar.com, and
wishes to use different instances of the SQL module for each. In
addition, there is one RADIUS client sending accounting data that is
to be logged only to a specific detail file. Everything else should
use a third SQL instance.

The acct_users file would look something like this::

  DEFAULT Realm == "foo.com", Acct-Type := "SQLFOO"

  DEFAULT Realm == "bar.com", Acct-Type := "SQLBAR"

  DEFAULT Client-IP-Address == "10.0.0.1", Acct-Type := "OTHERNAS"

And in radiusd.conf::

  $INCLUDE  ${confdir}/sql0.conf # Instance named 'sql0'.
  $INCLUDE  ${confdir}/sql1.conf # Instance named 'sql1'.
  $INCLUDE  ${confdir}/sql2.conf # Instance named 'sql2'.

  detail othernas {
        filename = ${radacctdir}/10.0.0.1/detail-%Y%m%d
  }

  preacct {
        suffix # Add the Realm A/V pair.
        files  # Add the Acct-Type A/V pair based on the Realm A/V pair.
  }

  accounting {

        # If Acct-Type is SQLFOO use the 'sql1' instance of the SQL module.

        Acct-Type SQLFOO {
                sql1
        }

        # If Acct-Type is SQLBAR, use the 'sql2' instance of the SQL module.

        Acct-Type SQLBAR {
                sql2
        }

        # If Acct-Type is OTHERNAS, use the 'othernas' instance of the detail
        # module

        Acct-Type OTHERNAS {
                othernas
        }

        # If we've made it this far, we haven't matched an Acct-Type, so use
        # the sql0 instance.

        sql0
  }
