Autz-Type
=========

Like Auth-Type for authentication method selection freeradius also
supports the Autz-Type to select between authorization methods.  The only
problem is that authorization is the first thing to be called when an
authentication request is handled.  As a result we first have to call the
authorize section without checking for Autz-Type. After that we check for
Autz-Type and if it exists we call the corresponding subsection in the
authorize section.  In other words the authorize section in radiusd.conf
should look like this::

 authorize{
         suffix
         preprocess
         # whatever other authorize modules here
         Autz-Type Ldap{
                 ldap
         }
         Autz-Type SQL{
                 sql
         }
         files
 }

What happens is that the first time the authorize section is examined the
suffix, preprocess and files modules are executed.  If Autz-Type is set
after that the server core checks for any matching Autz-Type subsection.
If one is found it is called.  The users file should look something
like this::

  DEFAULT        Called-Station-Id == "123456789", Autz-Type := Ldap

  DEFAULT Realm == "other.company.com", Autz-Type := SQL

Autz-Type could also be used to select between multiple instances of
a module (ie sql or ldap) which have been configured differently.  For
example based on the user realm different ldap servers (belonging to
different companies) could be queried.  If Auth-Type was also set then we
could do both Authentication and Authorization with the user databases
belonging to other companies.  In detail:

radiusd.conf::

  authenticate{
         Auth-Type customer1{
                 ldap1
         }
         Auth-Type customer2{
                 ldap2
         }
  }

  authorize{
         preprocess
         suffix
         Autz-Type customer1{
                 ldap1
         }
         Autz-Type customer2{
                 ldap2
         }
         files
  }

The users file::

  DEFAULT Realm == "customer1", Autz-Type := customer1, Auth-Type := customer1

  DEFAULT Realm == "customer2", Autz-Type := customer2, Auth-Type := customer2


Apart from Autz-Type the server also supports the use of
Acct-Type, Session-Type and Post-Auth-Type for the corresponding sections.
The corresponding section names in the radiusd.conf file are the same.  So for example:

users file::

  DEFAULT Called-Station-Id == "236473", Session-Type := SQL

radiusd.conf::

 session {
         radutmp
         Session-Type SQL {
                 sql
         }
 }
