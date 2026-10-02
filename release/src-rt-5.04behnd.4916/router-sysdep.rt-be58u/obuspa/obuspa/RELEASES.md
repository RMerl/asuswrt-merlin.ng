# Release History

## Release 7.0.0

  * Bug Fixes
    * MQTT MTP
      * GH#30: MQTT publish QoS cannot be configured (stuck at 0)
      * For MQTTv3.x MTP, Agent should subscribe to wildcarded agent topic and publish to topic containing '/reply-to='
      * When changing a subscription's topic, The agent was unsubscribing from the new topic (not the old topic)
      * For MQTTv5 MTP, Agent should subscribe to topic from CONNACK, if available
      * MQTT exponential backoff reconnect time was 1 second too long
      * libmosquitto subscribe/unsubscribe functions were being called unnecessarily (and failing)
      * MQTT_DeleteSubscription() was being called unnecessarily (and failing)
      * Memory leak when unescaping a received MQTT response topic
      * MQTT subscription topic must be unique
      * MQTT subscription topic must be non empty string
      * When disabling the agent's MQTT client, the MQTT DISCONNECT frame is not sent and socket stays open
      * Missing mutex protection in some MQTT functions

    * WebSocket MTP
      * Agent's websocket server is not restarted after IP address change or initial failure
      * Requirement for libwebsockets version >= 4.1.0 made explicit in configure.ac

    * USP Spec and Test Plan modifications
      * MQTT Content Type property changed to usp.msg (changed R-MQTT.27)
      * A USP Set referencing zero instances returns an empty success (changed TP-469 conformance test 1.23)
      * Errors in received USP packets should be handled according to requirements in R-MTP.5
      * USP Record should be ignored if the protobuf cannot be unpacked

    * Miscellaneous
      * GH#55: Updated Dockerfile to use Ubuntu Kinetic which includes the required versions of libmosquitto and libwebsockets
      * GH#64: Crash if PeriodicNotifInterval is set to 0
      * GetResponse optimized for large numbers of object instances
      * If 'obuspa -c get' truncates the printing of a parameter value, this is now indicated
      * OnBoardRequest notification should be retried, if corresponding NotifyResponse not received
      * Adding a Controller MTP object fails, if Protocol is not specified and CoAP is disabled ('configure --disable-coap')
      * Default value for RebootCause would be better as "FactoryReset"
      * Alias should be registered as a unique key for LocalAgent.Certificate
      * GetSupportedDM response should indicate that parameters registered with USP_REGISTER_DBParam_SecureWithType() are readable
      * Added note that the start_transaction vendor hook must never return a failure, as it is not possible for OBUSPA to handle the failure in all cases


## Release 6.0.0
  * New Features
    * USP 1.2 Spec Enhancements
      * Get request and response (max_depth field)
      * GetSupportedDM response (value_type, value_change and command_type fields)
      * USP Connect and Disconnect records
    * Basic End-to-End session context support (mainly segmentation and reassembly aspects). Disabled by default. To enable, uncomment E2ESESSION_EXPERIMENTAL_USP_V_1_2 in vendor_defs.h
    * The severity level of log messages now propagates to syslog() invocations. To override the default severity level passed to syslog, modify SYSLOG_SEVERITY_OVERRIDE in vendor_defs.h.
    * Added extra parameter types: DM_BASE_64, DM_DECIMAL, DM_LONG and DM_HEX_BINARY
    * MTP credentials username core vendor hook

  * Data Model Enhancements
    * Device.DeviceInfo.UpTime
    * Device.LocalAgent.MTP.{i}.MQTT.ResponseTopicDiscovered

  * Bug Fixes
    * Data model thread locks up on high message throughput
    * Agent crashes when nested objects are created in an ADD message with allow_partial=false
    * Password in STOMP header should be escaped
    * GetSupportedDM Response reports incorrect Add and Delete permissions for objects registered using USP_REGISTER_GroupedObject()
    * Bulk Data Collection Protocol parameter is validating against old value, not new value
    * Reference following should allow references to contain key based addressing
    * 64 bit signed and unsigned integer parameter values are now represented with full precision in JSON encoded data (for Bulk Data Collection reports and Boot! event). To enable the old behavior (which represented them as floating point doubles) comment out the REPRESENT_JSON_NUMBERS_WITH_FULL_PRECISION define in vendor_defs.h
    * Empty Subscription ID should not be allowed
    * Subscription ID should be auto-assigned
    * Compile error if MQTT and STOMP are disabled
    * getopt_long options structure was wrong for authcert option
    * Set Request obj_path is not required to contain an instance number
    * STOMP DISCONNECT frame should be sent before disconnecting if no agent destination configured (USP Compliance Test 6.7)
    * Compilation fails when WebSockets enabled and CoAP disabled
    * MQTT does not wait until all responses are sent when disabling MTP (GH Issue 33)
    * MQTT v5.0 response topic memory leak
    * Optional input arguments to USP Commands should be ignored
    * EndpointId should be in quotes in Sec-WebSocket-Extension header
    * MQTT KeepAlive 0 not working as expected

  * Other
    * Tidy up MQTT code: Added function header comments and re-ordered some MQTT functions
    * Removed GET_RESPONSE_SIMPLE_FORMAT from vendor_defs.h. In USP 1.2 it was clarified that only the simple format must be used.
    * WebSockets example factory reset file
    * Updated quick start guide with instructions for cloning from github (GH Issue 39)

## Release 5.0.0
  * New Features
    * WebSockets MTP Support in both client and server modes
    * 'obuspa -c event' signals a USP event (for testing purposes)
    * TLS Server Name Indication for STOMP
    * Added ABOUT files (for open source attribution and inventory purposes)
    * 'obuspa -c version' includes versions of shared library dependencies

  * Bug Fixes
    * MQTT connect can block, holding up the data model thread (GH Issue #31)
    * MQTT Connect callback should not add trust store certs everytime (GH Issue #29)
    * MQTT Subscriptions don't work at startup (GH Issue #23)
    * MQTT subscribe-topic in CONNACK is not retrieved correctly
    * CoAP RST message format error (GH#25)
    * Support for mallinfo2, mallinfo deprecated on glibc >= 2.33 (GH#26)
    * RequestChallenge/ChallengeResponse : Retries and Lockout period applied per ChallengeRef
    * ControllerTrust parameter set permission was incorrectly additionally considering the set permission of the parent object
    * Compilation error in protobuf C library with GCC 10.3
    * Compilation errors on GCC 11.2 cross compiler for ARM

  * Other
    * Added 'const' declaration to 'char *' usage in logger functions for calling from C++
    * Removed unnecessary log message for object deletion subscription containing wildcard and partial path
    * Get Supported Protocol version updated
    * Factory reset database improvements

## Release 4.1.0
  * New Features
    * Search expressions supported in Bulk Data Collection
    * 'obuspa -c show data model' includes USP command and event arguments
  * Bug Fixes
    * MQTT Connection failure over TLS with libmosquitto 1.6.13+
    * Some ChallengeResponse parameters were not using base64
    * Device.LocalAgent.CertificateNumberOfEntries returns empty string rather than 0
    * Device.MQTT.Client.1.Status returns "Running" rather than "Connected"
    * Device.Security.Certificate.1.Alias is not defined in TR181
    * Probuf protocol trace does not indicate if truncated
    * Removed unnecessary allowed_controllers code
    * Untrusted role cannot issue a RequestChallenge() or ChallengeResponse() command
    * Bulk Data Collection using USPNotif throws console error related to HTTP URL (GH Issue #20)
    * Race hazard prevents changing MQTT connection parameters
    * 'obuspa -e' option accepted, even if C library cannot print callstack
    * Certificate validity dates reported incorrectly on 32 bit platforms for dates after 2038
    * Object deletion notification not sent for nested objects (GH Issue #21)
  * Data Model Enhancements
    * Device.MQTT.Capabilities
  * API Enhancements
    * Table objects must now be registered before child parameters
    * USP_REGISTER_Param_SupportedList()
    * USP_DM_InformDataModelEvent()
    * MQTT password via get_mtp_password_cb
  * Known Issues
    * ControllerTrust ChallengeResponse handling of retries

## Release 4.0.0
  * ControllerTrust support
    * Challenge/Response mechanism
    * OnBoardRequest mechanism
    * Device.LocalAgent.Certificate (certs trusted by OBUSPA)
    * Device.Security.Certificate (all other certs)
  * Bulk Data Collection via USP notifications
  * Configure options for supported MTPs
  * ScheduleTimer USP command support
  * Reference following in search expressions
  * Example Dockerfile
  * Bug Fixes
    * Reporting of memory leaks when -m option used
    * Exponential backoff for MQTT
    * Reporting of synchronous USP command failures
    * Database persistence of multi-instance objects consisting purely of ReadOnly parameters
    * USP SetResponse when using wildcards
    * ObjectCreation/Deletion notifications if ReferenceList contains a partial path and object uses refresh instances vendor hook
    * KV_VECTOR_FindKey() when start_index is non zero
    * Reporting of object access in GetSupportedDM response for read only objects that use the grouped vendor interface
    * Default Endpoint ID containing characters which should be percent encoded
    * Legacy OpenSSL 1.0.2g support
    * MQTT Response Topic
    * MQTT Secure Password Parameter
  * Other Changes
    * MQTT broker certificate hostname validation
    * Timeout for SSL handshake if STOMP broker doesn't respond
    * subscription_id field ignored in NotifyResp
    * MQTT protocol tracing improved
    * X_ARRIS-COM_EnableEncryption parameter name changed to EnableEncryption, for STOMP MTP
  * Known Issues
    * ControllerTrust ChallengeResponse handling of retries

## Release 3.0.0
  * MQTT MTP support
     * MQTT v5.0 and MQTT v3.1.1 support
     * TLS support
     * MQTT topic subscriptions
  * Architectural enhancements
     * More efficient USP Get and Set for groups of parameters provided by separate components
     * On-demand querying of table instance numbers
  * Build support for MUSL libc
  * Bug fixes
     * Conformance test failures
     * Defaulted unique key parameters are validated for uniqueness when adding an instance to a table
     * Deleting a controller deletes its subscriptions
     * References to objects without trailing instance numbers do not cause assert
     * USPRetry set of parameters, should actually be named USPNotifRetry
  * Other changes
     * Ignore sender certificate in USP records
     * Annotations on function prototypes for better Clang static analyzer results
     * CoAP.Interfaces parameter removed
     * More robust handling of badly formed percent encoded CoAP resource in URI query option

## Release 2.1.0
  * Simultaneous sessions with multiple CoAP based controllers are now supported
  * USP Record error handling has been upgraded to be compliant with the USP 1.1 Specification
  * The InstantiatedPath field in the USP AddResponse message is now formed correctly
  * Percent encoded CoAP resource names are now handled correctly
  * Queued USP notifications are not sent out if their NotifExpiration period has elapsed

## Release 2.0.0
  * CoAP
     * New USP optimized implementation
     * DTLS support
     * Robustness improvements
     * Specification of CoAP server network interface
     * reply-to URI query option support
  * New command line arguments
     * Trust store certificates file (-t)
     * Factory reset parameters text file (-r)
     * Network interface for USP communications (-i)
  * Data Model improvements
     * Device.Time.LocalTimezone added
     * Bulk Data Collection client certificate support
  * Bug fixes
     * Session Retry wait interval calculation should not overflow
     * Agent should not generate an error if trying to delete an instance which is already deleted
     * An empty subscription ReferenceList should not cause a crash
     * UINT32 parameter limit validation should not fail for 32 bit architectures
     * Error message for path containing back-to-back instance searches should not be confusing

## Release 1.0.1
  * configure now checks for libcoap availability
  * Out of tree builds are now supported
  * Added contibuting guidelines

## Release 1.0.0
This release contains the following features:
* MTPs
   * CoAP (alpha quality).
      *  No DTLS Support
   * STOMP
      * TLS Support
      * Authentication: username/password or client cert
      * IPv4 and IPv6 Supported
      * USP Record
      * NoSessionContext Record Type Supported
      * PLAINTEXT Payload Security Supported
      * No Support for: SessionContext Record Type, TLS12 Payload Security, Payload SAR, or MAC Signature
* USP Message
   * Support for: Get, Set, Add, Delete, Operate, GetSupportedDM, GetInstances, GetSupportedProtocol, and Notify
* Data Model Addressing
   * By Unique Key
   * By Instance Number
* Data Model Searching
   * By Wildcard
   * By Expression
* Data Model Reference Path Following
   * Fully supported except when: embedded in a Search Expression, as part of a list of references
* HTTP Bulk Data Collection
   * Only JSON is supported with the NameValuePair Report Format
* Controller Trust
   * Roles and Permissions are supported (but fixed at compile time)
   * Credentials are supported (but limited to trust store certificates that are fixed at initialization time)
* USP Data Model implemented
   * Device.Reboot()
   * Device.Boot!
   * Device.FactoryReset()
   * Device.SelfTestDiagnostics()
   * Device.DeviceInfo.
   * Device.LocalAgent.
   * Device.LocalAgent.MTP.{i}.
   * Device.LocalAgent.MTP.{i}.CoAP.
   * Device.LocalAgent.MTP.{i}.STOMP.
   * Device.LocalAgent.Controller.{i}.MTP.{i}.
   * Device.LocalAgent.Controller.{i}.MTP.{i}.CoAP.
   * Device.LocalAgent.Controller.{i}.MTP.{i}.STOMP.
   * Device.LocalAgent.Controller.{i}.BootParameter.{i}.
   * Device.LocalAgent.Subscription.{i}.
   * Device.LocalAgent.Request.{i}.
   * Device.LocalAgent.ControllerTrust.
   * Device.LocalAgent.ControllerTrust.Role.{i}.
   * Device.LocalAgent.ControllerTrust.Role.{i}.Permission.{i}.
   * Device.LocalAgent.ControllerTrust.Credential.{i}.
   * Device.Time.
   * Device.Security.
   * Device.Security.Certificate.{i}.
   * Device.BulkData.
   * Device.BulkData.Profile.{i}.
   * Device.BulkData.Profile.{i}.Parameter.{i}.
   * Device.BulkData.Profile.{i}.JSONEncoding.
   * Device.BulkData.Profile.{i}.HTTP.
   * Device.BulkData.Profile.{i}.HTTP.RequestURIParameter.{i}.
   * Device.STOMP.
   * Device.STOMP.Connection.{i}.

