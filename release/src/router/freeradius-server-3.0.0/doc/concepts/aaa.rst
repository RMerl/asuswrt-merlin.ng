Authorization, Authentication, and Accounting request handling
==============================================================

There are a lot of questions about misconfigured FreeRADIUS servers
because of misunderstanding of FreeRADIUS operations.  This document
explains how the server operates.

Normally there are 2 steps in processing authentication request coming
from NAS in FreeRADIUS (plus additional steps to proxy request if we
use FreeRADIUS as a proxy): authorization and authentication.


Authorization
-------------

Authorization is a process of obtaining information about the user
from external source (file, database or LDAP), and checking that the
information in request is enough to authenticate user.  Authorization
modules deal with data sources, so ldap, sql, files, passwd are
authorization modules.

The authentication method is decided during the authorization phase,
along with any reply attributes.  The reason for this behaviour is
that for example, a user may not be permitted to use a particular
authentication method.  So during the authorize phase, we can deny
them the ability to use that kind of authentication.

Authentication
--------------

Authentication is simply a process of comparing user's credentials in
request with credentials stored in database.  Authentication usually
deals with password encryption.  PAP, CHAP, MS-CHAP are authentication
modules.  Few modules act as both authorization and authentication.
For example, the MS-CHAP module is normally authentication only, but it
may be used during authorization to verify that request contains
MS-CHAP related attribute and only in this case perform MS-CHAP based
authentication. LDAP is normally an authorization module, but it may
be used for authentication (In this case FreeRADIUS will authenticate
user in case he can connect to LDAP server with his account).  SQL is
only an authorization module, as dial-in users are not normally given
passwords to access an SQL server.


Request Processing
------------------

During authorization and authentication processes, there are 3 lists
of RADIUS attributes supported by FreeRADIUS: request items, config
items and reply items.  (See 'man 5 users' for additional
information.)  Attributes from the RADIUS authentication request
packet are included into request items list.  Both authorization and
authentication modules can add attributes into reply items list. These
attributes will be added to reply will be sent by RADIUS server to
NAS.  There is third list, called config items.  It's used for
internal FreeRADIUS operations, for example to pass some data from
authorization to authentication module.

Before authorization begins FreeRADIUS creates request items list with
attributes from request and empty config and reply lists.

An authorization module searches a database with attributes
(e.g. User-Name) taken from request list as a key, and fetches all
relevant records.  It retrieves 3 types of attributes: check
attributes, configure attributes and reply attributes. It compares the
check attributes with attributes from request items. If none of
database record for this User-Name matches in check attributes with
request items authorization will fail. If a matching record is found,
then the configure attributes will be added to configure items, and
the reply attributes will be added to reply items list.  The check
list may be required if we need to authenticate users with same name
for different services (for example to treat User1 from NAS1 and User1
from NAS2 as different users).

There should be at list one configure attribute provided by
authorization module, called Auth-Type (since this attribute is from
config items list it can't be in request or reply).  This attribute
decides which module will be used to authenticate the user.  The
Config items also contains information from database required to
authenticate user, for example valid user's password or it's hash,
login restrictions, etc.

A quite common mistake is to place the attributes in the wrong lists,
for example placing Auth-Type, Password, NT-Password etc in the check
list, or in the reply list.  When run in debugging mode, the server
will normally issue 'WARNING' messages saying that the attributes are
in the wrong list.

If you place Password into check list and user does cleartext
authentication it may work, because authorization module compares 2
cleartext passwords.  But if user does some encrypted authentication
(for example MS-CHAP), then the authorization will fail, because the
Password in the request items will not match the password in the check
attributes.  You should place Password attribute obtained from
database into configure items and also place Auth-Type attribute with
value of 'MS-CHAP' into same list.  The same goes for NT-Password
(before calling MS-CHAP Password attribute should be converted to
NT-Password, it may be achieved by calling mschap module in
authorization section after module which does actual authorization).
