/*
 * $Id$
 *
 * --- Peter Nixon [ codemonkey@peternixon.net ]
 *
 * This is a custom SQL schema for doing H323 and SIP VoIP accounting
 * with FreeRadius and Cisco equipment. It is currently known to work
 * with 3640, 5300 and 5350 series as well as CSPS (Cisco SIP Proxy
 * Server).  It will scale A LOT better than the default radius schema
 * which is designed for simple dialup installations of FreeRadius.
 *
 * For this schema to work properly you MUST use
 * raddb/sql/postgresql/voip-postpaid.conf rather than
 * raddb/sql/postgresql/dialup.conf
 *
 * If you wish to do RADIUS Authentication using the same database,
 * you MUST use use raddb/sql/postgresql/schema.sql as well as this schema.
 */

/*
 * Table structure for 'Start' tables
 */

CREATE TABLE StartVoIP (
	RadAcctId		BIGSERIAL PRIMARY KEY,
	AcctTime		TIMESTAMP with time zone NOT NULL,
	h323SetupTime		TIMESTAMP with time zone,
	H323ConnectTime		TIMESTAMP with time zone,
	UserName		VARCHAR(64),
	RadiusServerName	VARCHAR(32),
	NASIPAddress		INET NOT NULL,
	CalledStationId		VARCHAR(80),
	CallingStationId	VARCHAR(80),
	AcctDelayTime		INTEGER,
	H323GWID		VARCHAR(32),
	h323CallOrigin		VARCHAR(10),
	CallID			VARCHAR(80) NOT NULL,
	processed		BOOLEAN DEFAULT false
);
create index startvoipcombo on startvoip (AcctTime, nasipaddress);


CREATE TABLE StartTelephony (
	RadAcctId		BIGSERIAL PRIMARY KEY,
	AcctTime		TIMESTAMP with time zone NOT NULL,
	h323SetupTime		TIMESTAMP with time zone,
	H323ConnectTime		TIMESTAMP with time zone,
	UserName		VARCHAR(64),
	RadiusServerName	VARCHAR(32),
	NASIPAddress		INET NOT NULL,
	CalledStationId		VARCHAR(80),
	CallingStationId	VARCHAR(80),
	AcctDelayTime		INTEGER,
	H323GWID		VARCHAR(32),
	h323CallOrigin		VARCHAR(10),
	CallID			VARCHAR(80) NOT NULL,
	processed		BOOLEAN DEFAULT false
);
create index starttelephonycombo on starttelephony (AcctTime, nasipaddress);



/*
 * Table structure for 'Stop' tables
 */
CREATE TABLE StopVoIP (
	RadAcctId		BIGSERIAL PRIMARY KEY,
	AcctTime		TIMESTAMP with time zone NOT NULL,
	H323SetupTime		TIMESTAMP with time zone,
	H323ConnectTime		TIMESTAMP with time zone,
	H323DisconnectTime	TIMESTAMP with time zone,
	UserName		VARCHAR(32),
	RadiusServerName	VARCHAR(32),
	NASIPAddress		INET NOT NULL,
	AcctSessionTime		BIGINT,
	AcctInputOctets		BIGINT,
	AcctOutputOctets	BIGINT,
	CalledStationId		VARCHAR(80),
	CallingStationId	VARCHAR(80),
	AcctDelayTime		SMALLINT,
	CiscoNASPort		VARCHAR(1),
	H323GWID		VARCHAR(32),
	H323CallOrigin		VARCHAR(10),
	H323DisconnectCause	VARCHAR(20),
	H323RemoteAddress	INET,
	H323VoiceQuality	INTEGER,
	CallID			VARCHAR(80) NOT NULL,
	processed		BOOLEAN DEFAULT false
);
create UNIQUE index stopvoipcombo on stopvoip (AcctTime, nasipaddress, CallID);


CREATE TABLE StopTelephony (
	RadAcctId		BIGSERIAL PRIMARY KEY,
	AcctTime		TIMESTAMP with time zone NOT NULL,
	H323SetupTime		TIMESTAMP with time zone NOT NULL,
	H323ConnectTime		TIMESTAMP with time zone NOT NULL,
	H323DisconnectTime	TIMESTAMP with time zone NOT NULL,
	UserName		VARCHAR(32) DEFAULT '' NOT NULL,
	RadiusServerName	VARCHAR(32),
	NASIPAddress		INET NOT NULL,
	AcctSessionTime		BIGINT,
	AcctInputOctets		BIGINT,
	AcctOutputOctets	BIGINT,
	CalledStationId		VARCHAR(80),
	CallingStationId	VARCHAR(80),
	AcctDelayTime		SMALLINT,
	CiscoNASPort		VARCHAR(16),
	H323GWID		VARCHAR(32),
	H323CallOrigin		VARCHAR(10),
	H323DisconnectCause	VARCHAR(20),
	H323RemoteAddress	INET,
	H323VoiceQuality	INTEGER,
	CallID			VARCHAR(80) NOT NULL,
	processed		BOOLEAN DEFAULT false
);
-- You can have more than one record that is identical except for CiscoNASPort if you have a dial peer hungroup
-- configured for multiple PRIs.
create UNIQUE index stoptelephonycombo on stoptelephony (AcctTime, nasipaddress, CallID, CiscoNASPort);

/*
 * Table structure for 'gateways'
 *
 * This table should list the IP addresses, names and locations of all your gateways
 * This can be used to make more useful reports.
 *
 * Note: This table should be removed in favour of using the "nas" table.
 */

CREATE TABLE gateways (
	gw_ip		INET NOT NULL,
	gw_name		VARCHAR(32) NOT NULL,
	gw_city		VARCHAR(32)
);


/*
 * Table structure for 'customers'
 *
 * This table should list your Customers names and company
 * This can be used to make more useful reports.
 */

CREATE TABLE customers (
	cust_id		SERIAL NOT NULL,
	company		VARCHAR(32),
	customer	VARCHAR(32)
);

/*
 * Table structure for 'cust_gw'
 *
 * This table should list the IP addresses and Customer IDs of all your Customers gateways
 * This can be used to make more useful reports.
 */

CREATE TABLE cust_gw (
	cust_gw		INET PRIMARY KEY,
	cust_id		INTEGER NOT NULL,
	"location"	VARCHAR(32)
);


CREATE VIEW customerip AS
    SELECT gw.cust_gw AS ipaddr, cust.company, cust.customer, gw."location" FROM customers cust, cust_gw gw WHERE (cust.cust_id = gw.cust_id);


-- create plpgsql language (You need to be a database superuser to be able to do this)
CREATE FUNCTION "plpgsql_call_handler" () RETURNS LANGUAGE_HANDLER AS '$libdir/plpgsql' LANGUAGE C;
CREATE TRUSTED LANGUAGE "plpgsql" HANDLER "plpgsql_call_handler";

/*
 * Function 'strip_dot'
 * removes "." from the start of cisco timestamps
 *
 * From the cisco website:
 * "A timestamp that is preceded by an asterisk (*) or a dot (.) may not be accurate.
 *  An asterisk (*) means that after a gateway reboot, the gateway clock was not manually set
 *  and the gateway has not synchronized with an NTP server yet. A dot (.) means the gateway
 *  NTP has lost synchronization with an NTP server."
 *
 * We therefore do not bother to strip asterisks (*) from timestamps, as you NEED ntp setup
 * unless you don't care about billing at all!
 *
 *  * Example useage:
 *      insert into mytable values (strip_dot('.16:46:02.356 EET Wed Dec 11 2002'));
 *
 */


CREATE OR REPLACE FUNCTION strip_dot (VARCHAR) RETURNS TIMESTAMPTZ AS '
 DECLARE
        original_timestamp ALIAS FOR $1;
 BEGIN
	IF original_timestamp = '''' THEN
		RETURN NULL;
        END IF;
        IF substring(original_timestamp from 1 for 1) = ''.'' THEN
                RETURN substring(original_timestamp from 2);
        ELSE
                RETURN original_timestamp;
        END IF;
 END;
' LANGUAGE 'plpgsql';


CREATE OR REPLACE FUNCTION pick_id (VARCHAR, VARCHAR) RETURNS VARCHAR AS '
 DECLARE
        h323confid ALIAS FOR $1;
        callid ALIAS FOR $2;
 BEGIN
        IF h323confid <> '''' THEN
                RETURN h323confid;
        END IF;
        IF callid <> '''' THEN
                RETURN callid;
        END IF;
	RETURN NULL;
 END;
' LANGUAGE 'plpgsql';



/*
 * Table structure for 'isdn_error_codes' table
 *
 * Taken from cisco.com this data can be JOINED against h323DisconnectCause to
 * give human readable error reports.
 *
 */


CREATE TABLE isdn_error_codes (
	error_code	VARCHAR(2) PRIMARY KEY,
	desc_short	VARCHAR(90),
	desc_long	TEXT
);

/*
 * Data for 'isdn_error_codes' table
 */

INSERT INTO isdn_error_codes VALUES ('1', 'Unallocated (unassigned) number', 'The ISDN number was sent to the switch in the correct format; however, the number is not assigned to any destination equipment.');
INSERT INTO isdn_error_codes VALUES ('10', 'Normal call clearing', 'Normal call clearing has occurred.');
INSERT INTO isdn_error_codes VALUES ('11', 'User busy', 'The called system acknowledges the connection request but is unable to accept the call because all B channels are in use.');
INSERT INTO isdn_error_codes VALUES ('12', 'No user responding', 'The connection cannot be completed because the destination does not respond to the call.');
INSERT INTO isdn_error_codes VALUES ('13', 'No answer from user (user alerted)', 'The destination responds to the connection request but fails to complete the connection within the prescribed time. The problem is at the remote end of the connection.');
INSERT INTO isdn_error_codes VALUES ('15', 'Call rejected', 'The destination is capable of accepting the call but rejected the call for an unknown reason.');
INSERT INTO isdn_error_codes VALUES ('16', 'Number changed', 'The ISDN number used to set up the call is not assigned to any system.');
INSERT INTO isdn_error_codes VALUES ('1A', 'Non-selected user clearing', 'The destination is capable of accepting the call but rejected the call because it was not assigned to the user.');
INSERT INTO isdn_error_codes VALUES ('1B', 'Designation out of order', 'The destination cannot be reached because the interface is not functioning correctly, and a signaling message cannot be delivered. This might be a temporary condition, but it could last for an extended period of time. For example, the remote equipment might be turned off.');
INSERT INTO isdn_error_codes VALUES ('1C', 'Invalid number format', 'The connection could be established because the destination address was presented in an unrecognizable format or because the destination address was incomplete.');
INSERT INTO isdn_error_codes VALUES ('1D', 'Facility rejected', 'The facility requested by the user cannot be provided by the network.');
INSERT INTO isdn_error_codes VALUES ('1E', 'Response to STATUS ENQUIRY', 'The status message was generated in direct response to the prior receipt of a status enquiry message.');
INSERT INTO isdn_error_codes VALUES ('1F', 'Normal, unspecified', 'Reports the occurrence of a normal event when no standard cause applies. No action required.');
INSERT INTO isdn_error_codes VALUES ('2', 'No route to specified transit network', 'The ISDN exchange is asked to route the call through an unrecognized intermediate network.');
INSERT INTO isdn_error_codes VALUES ('22', 'No circuit/channel available', 'The connection cannot be established because no appropriate channel is available to take the call.');
INSERT INTO isdn_error_codes VALUES ('26', 'Network out of order', 'The destination cannot be reached because the network is not functioning correctly, and the condition might last for an extended period of time. An immediate reconnect attempt will probably be unsuccessful.');
INSERT INTO isdn_error_codes VALUES ('29', 'Temporary failure', 'An error occurred because the network is not functioning correctly. The problem will be resolved shortly.');
INSERT INTO isdn_error_codes VALUES ('2A', 'Switching equipment congestion', 'The destination cannot be reached because the network switching equipment is temporarily overloaded.');
INSERT INTO isdn_error_codes VALUES ('2B', 'Access information discarded', 'The network cannot provide the requested access information.');
INSERT INTO isdn_error_codes VALUES ('2C', 'Requested circuit/channel not available', 'The remote equipment cannot provide the requested channel for an unknown reason. This might be a temporary problem.');
INSERT INTO isdn_error_codes VALUES ('2F', 'Resources unavailable, unspecified', 'The requested channel or service is unavailable for an unknown reason. This might be a temporary problem.');
INSERT INTO isdn_error_codes VALUES ('3', 'No route to destination', 'The call was routed through an intermediate network that does not serve the destination address.');
INSERT INTO isdn_error_codes VALUES ('31', 'Quality of service unavailable', 'The requested quality of service cannot be provided by the network. This might be a subscription problem.');
INSERT INTO isdn_error_codes VALUES ('32', 'Requested facility not subscribed', 'The remote equipment supports the requested supplementary service by subscription only.');
INSERT INTO isdn_error_codes VALUES ('39', 'Bearer capability not authorized', 'The user requested a bearer capability that the network provides, but the user is not authorized to use it. This might be a subscription problem.');
INSERT INTO isdn_error_codes VALUES ('3A', 'Bearer capability not presently available', 'The network normally provides the requested bearer capability, but it is unavailable at the present time. This might be due to a temporary network problem or to a subscription problem.');
INSERT INTO isdn_error_codes VALUES ('3F', 'Service or option not available, unspecified', 'The network or remote equipment was unable to provide the requested service option for an unspecified reason. This might be a subscription problem.');
INSERT INTO isdn_error_codes VALUES ('41', 'Bearer capability not implemented', 'The network cannot provide the bearer capability requested by the user.');
INSERT INTO isdn_error_codes VALUES ('42', 'Channel type not implemented', 'The network or the destination equipment does not support the requested channel type.');
INSERT INTO isdn_error_codes VALUES ('45', 'Requested facility not implemented', 'The remote equipment does not support the requested supplementary service.');
INSERT INTO isdn_error_codes VALUES ('46', 'Only restricted digital information bearer capability is available', 'The network is unable to provide unrestricted digital information bearer capability.');
INSERT INTO isdn_error_codes VALUES ('4F', 'Service or option not implemented, unspecified', 'The network or remote equipment is unable to provide the requested service option for an unspecified reason. This might be a subscription problem.');
INSERT INTO isdn_error_codes VALUES ('51', 'Invalid call reference value', 'The remote equipment received a call with a call reference that is not currently in use on the user-network interface.');
INSERT INTO isdn_error_codes VALUES ('52', 'Identified channel does not exist', 'The receiving equipment is requested to use a channel that is not activated on the interface for calls.');
INSERT INTO isdn_error_codes VALUES ('53', 'A suspended call exists, but this call identity does not', 'The network received a call resume request. The call resume request contained a Call Identify information element that indicates that the call identity is being used for a suspended call.');
INSERT INTO isdn_error_codes VALUES ('54', 'Call identity in use', 'The network received a call resume request. The call resume request contained a Call Identify information element that indicates that it is in use for a suspended call.');
INSERT INTO isdn_error_codes VALUES ('55', 'No call suspended', 'The network received a call resume request when there was not a suspended call pending. This might be a transient error that will be resolved by successive call retries.');
INSERT INTO isdn_error_codes VALUES ('56', 'Call having the requested call identity has been cleared', 'The network received a call resume request. The call resume request contained a Call Identity information element, which once indicated a suspended call. However, the suspended call was cleared either by timeout or by the remote user.');
INSERT INTO isdn_error_codes VALUES ('58', 'Incompatible destination', 'Indicates that an attempt was made to connect to non-ISDN equipment. For example, to an analog line.');
INSERT INTO isdn_error_codes VALUES ('5B', 'Invalid transit network selection', 'The ISDN exchange was asked to route the call through an unrecognized intermediate network.');
INSERT INTO isdn_error_codes VALUES ('5F', 'Invalid message, unspecified', 'An invalid message was received, and no standard cause applies. This is usually due to a D-channel error. If this error occurs systematically, report it to your ISDN service provider.');
INSERT INTO isdn_error_codes VALUES ('6', 'Channel unacceptable', 'The service quality of the specified channel is insufficient to accept the connection.');
INSERT INTO isdn_error_codes VALUES ('60', 'Mandatory information element is missing', 'The receiving equipment received a message that did not include one of the mandatory information elements. This is usually due to a D-channel error. If this error occurs systematically, report it to your ISDN service provider.');
INSERT INTO isdn_error_codes VALUES ('61', 'Message type non-existent or not implemented', 'The receiving equipment received an unrecognized message, either because the message type was invalid or because the message type was valid but not supported. The cause is due to either a problem with the remote configuration or a problem with the local D channel.');
INSERT INTO isdn_error_codes VALUES ('62', 'Message not compatible with call state or message type non-existent or not implemented', 'The remote equipment received an invalid message, and no standard cause applies. This cause is due to a D-channel error. If this error occurs systematically, report it to your ISDN service provider.');
INSERT INTO isdn_error_codes VALUES ('63', 'Information element non-existent or not implemented', 'The remote equipment received a message that includes information elements, which were not recognized. This is usually due to a D-channel error. If this error occurs systematically, report it to your ISDN service provider.');
INSERT INTO isdn_error_codes VALUES ('64', 'Invalid information element contents', 'The remote equipment received a message that includes invalid information in the information element. This is usually due to a D-channel error.');
INSERT INTO isdn_error_codes VALUES ('65', 'Message not compatible with call state', 'The remote equipment received an unexpected message that does not correspond to the current state of the connection. This is usually due to a D-channel error.');
INSERT INTO isdn_error_codes VALUES ('66', 'Recovery on timer expires', 'An error-handling (recovery) procedure was initiated by a timer expiry. This is usually a temporary problem.');
INSERT INTO isdn_error_codes VALUES ('6F', 'Protocol error, unspecified', 'An unspecified D-channel error when no other standard cause applies.');
INSERT INTO isdn_error_codes VALUES ('7', 'Call awarded and being delivered in an established channel', 'The user is assigned an incoming call that is being connected to an already-established call channel.');
INSERT INTO isdn_error_codes VALUES ('7F', 'Internetworking, unspecified', 'An event occurred, but the network does not provide causes for the action that it takes. The precise problem is unknown.');

