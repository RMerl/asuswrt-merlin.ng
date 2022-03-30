/* copy from new wifi_hal.h */

// 802.11v BSS Transition Management Definitions
// Notes to developers:
// The name of the file is specific to the neighbor report ticket and is expected
// to be merged into wifi_hal.h.
// The elements included in this file are part of the optional subelements list
// for a Neighbor report; Table 9-151.  There is some overlap with
// beacon_report.h in RDKB-16367 that will need to be resolved as both files
// are merged with wifi_hal.h.

typedef mac_address_t   bssid_t;
typedef char            ssid_t[32];

#define MAX_BTM_DEVICES     64
#define MAX_URL_LEN         512
#define MAX_CANDIDATES      64
#define MAX_VENDOR_SPECIFIC 32

// BSS Termination Duration subelement, ID = 4, 802.11 section 9.4.2.2.
// This is a subelement because it is specific to Neighbor Report, and BTM
// Request Frame.
typedef struct {
    ULONG               tsf;    // 8 octet TSF timer value.
    USHORT              duration;
} wifi_BTMTerminationDuration_t;

typedef struct {
    CHAR                condensedStr[3];  // 2 char country code from do11CountryString.
} wifi_CondensedCountryString_t;

typedef struct {
    USHORT              offset;
    USHORT              interval;
} wifi_TSFInfo_t;

typedef struct {
    UCHAR               preference;
} wifi_BSSTransitionCandidatePreference_t;

typedef struct {
    USHORT              bearing;
    UINT                dist;
    USHORT              height;
} wifi_Bearing_t;

// Wide Bandwidth Channel Element, ID = 194.  802.11-2016 section 9.4.2.161.
typedef struct {
    UCHAR               bandwidth;
    UCHAR               centerSeg0;
    UCHAR               centerSeg1;
} wifi_WideBWChannel_t;

typedef struct {
    UCHAR                   token;
    UCHAR                   mode;
    UCHAR                   type;
    union {
        UCHAR               lci;
        UCHAR               lcr;
    } u;
} wifi_Measurement_t;

// HT Capabilities Element, ID = 45.  802.11-2016 section 9.4.2.56.
typedef struct {
    USHORT                  info;           // Bitfield where bit 0 is info[0] bit 0.
    UCHAR                   ampduParams;
    UCHAR                   mcs[16];        // Bitfield where bit 0 is mcs[0] bit 0.
    USHORT                  extended;       // Bitfield where bit 0 is ele_HTExtendedCapabilities[0] bit 0.
    UINT                    txBeamCaps;     // Bitfield where bit 0 is ele_TransmitBeamFormingCapabilities[0] bit 0.
    UCHAR                   aselCaps;
} wifi_HTCapabilities_t;

// VHT Capabilities Element, ID = 191.  802.11-2016 section 9.4.2.158.
typedef struct {
    UINT                    info;
    // The Supported VHT-MCS and NSS Set field is 64 bits long, but is broken
    // into 4 16 bit fields for convenience.
    USHORT                  mcs;
    USHORT                  rxHighestSupportedRate;
    USHORT                  txVHTmcs;
    USHORT                  txHighestSupportedRate;
} wifi_VHTCapabilities_t;

// HT OperationElement, ID = 61, 802.11-2016 section 9.4.2.57.
typedef struct {
    UCHAR                   primary;
    UCHAR                   opInfo[5];   // Bitfield where bit 0 is ele_HTOperationInfo[0] bit 0;
    UCHAR                   mcs[16];
} wifi_HTOperation_t;

// VHT Operation Element, ID = 192.  802.11-2016 section 9.4.2.159.
typedef struct {
    wifi_WideBWChannel_t        opInfo;         // channel width, center of seg0, center of seg1
    USHORT                      mcs_nss;        // Bit field.
} wifi_VHTOperation_t;

// Secondary Channel Offset Element, ID = 62, 802.11-2016 section
// 9.4.2.20.
typedef struct {
    UCHAR                       secondaryChOffset;
} wifi_SecondaryChannelOffset_t;

// RM Enabled Capabilities Element, ID = 70, 802.11-2016 section
// 9.4.2.45.
typedef struct {
    // This is a bit field defined by table 9-157.  Bit 0 for all of the
    // capabilities is ele_RMEnabledCapabilities[5] bit 0.
    UCHAR                       capabilities[5];
} wifi_RMEnabledCapabilities_t;

// Vendor Specific Element, ID = 221.  802.11-2016 section 9.4.2.26.
typedef struct {
    // 3 or 5 octet OUI depending on format; see 802.11-2016 section 9.4.1.32.
    UCHAR           oui[5];
    // Vendor specific content.
    UCHAR           buff[MAX_VENDOR_SPECIFIC];
} wifi_VendorSpecific_t;

// Measurement Pilot Transmission Element, ID = 66, 802.11-2016 section
// 9.4.2.42.
typedef struct {
    UCHAR                       pilot;
    // Series of (sub)elements.  Table 9-155 only lists vendor specific.
    wifi_VendorSpecific_t                       vendorSpecific;
} wifi_MeasurementPilotTransmission_t;

typedef struct {
    bssid_t             bssid;
    //  32 bit optional value, bit fileds are
    //  b0, b1 for reachability
    //  b2 security
    //  b3 key scope
    //  b4 to b9 capabilities
    //  b10 mobility domain
    //  b11 high troughput
    //  b12 very high throughput
    //  b13 ftm
    //  b14 to b31 reserved
    UINT                info;
    UCHAR               opClass;
    UCHAR               channel;
    UCHAR               phyTable;
    BOOL                tsfPresent;
    wifi_TSFInfo_t      tsfInfo;
    BOOL                condensedCountrySringPresent;
    wifi_CondensedCountryString_t   condensedCountryStr;
    BOOL                bssTransitionCandidatePreferencePresent;
    wifi_BSSTransitionCandidatePreference_t         bssTransitionCandidatePreference;
    BOOL                btmTerminationDurationPresent;
    wifi_BTMTerminationDuration_t   btmTerminationDuration;
    BOOL                bearingPresent;
    wifi_Bearing_t      bearing;
    BOOL                wideBandWidthChannelPresent;
    wifi_WideBWChannel_t    wideBandwidthChannel;
    BOOL                htCapsPresent;
    wifi_HTCapabilities_t   htCaps;
    BOOL                vhtCapsPresent;
    wifi_VHTCapabilities_t  vbhtCaps;
    BOOL                    htOpPresent;
    wifi_HTOperation_t      htOp;
    BOOL                    vhtOpPresent;
    wifi_VHTOperation_t     vhtOp;
    BOOL                    secondaryChannelOffsetPresent;
    wifi_SecondaryChannelOffset_t   secondaryChannelOffset;
    BOOL                    rmEnabledCapsPresent;
    wifi_RMEnabledCapabilities_t    rmEnabledCaps;
    BOOL                            msmtPilotTransmissionPresent;
    wifi_MeasurementPilotTransmission_t     msmtPilotTransmission;
    BOOL                    vendorSpecificPresent;
    wifi_VendorSpecific_t   vendorSpecific;
    ssid_t                  target_ssid;
} wifi_NeighborReport_t;

// BSS Transition Management Request Frame, 802.11-2016 section 9.6.14.9.
typedef struct {
    UCHAR               token;              // set by STA to relate reports
    UCHAR               requestMode;        // Requested instructions for the STA.
    USHORT              timer;
    UCHAR               validityInterval;
    // The optional fields may include:
    // 1. BSS Termination Duration Subelement, ID = 4. 802.11-2016 Figure 9-300.
    // 2. Session Information URL.
    // 3. BSS Transition Candidate List Entries
    wifi_BTMTerminationDuration_t    termDuration;
    UCHAR               disassociationImminent;
    USHORT              urlLen;
    CHAR                url[MAX_URL_LEN];
    UCHAR               numCandidates;
    wifi_NeighborReport_t    candidates[MAX_CANDIDATES];
} wifi_BTMRequest_t;

// BSS Transition Management Query Frame, 802.11-2016 section 9.6.14.8.
// Received from non-AP STA.
typedef struct {
    UCHAR                   token;          // set by STA to relate reports
    UCHAR                   queryReason;
    UCHAR                   numCandidates;
    wifi_NeighborReport_t   candidates[MAX_CANDIDATES];
} wifi_BTMQuery_t;

// BSS Transition Management Response Frame, 802.11-2016 section 9.6.14.10.
// Received from non-AP STA.
typedef struct {
    UCHAR               token;          // set by STA to relate reports
    UCHAR               status;
    UCHAR               terminationDelay;
    bssid_t             target;
    UCHAR                   numCandidates;
    wifi_NeighborReport_t    candidates[MAX_CANDIDATES];
} wifi_BTMResponse_t;

// Structure to return BTM extended capability from devices on the LAN.
// The peer and capability arrays are parallel
// and have the same number of entries.
typedef struct {
    UINT                entries;                        // Number of entries in each of the following arrays.
    mac_address_t       peer[MAX_BTM_DEVICES];          // Array a peer device MAC addresses.
    BOOL                capability[MAX_BTM_DEVICES];    // Array of bool indicating peer BSS transition capability.
} wifi_BTMCapabilities_t;

/* @description This call back is invoked when a STA sends a BTM query
 * message to a vAP in the gateway.  The driver will use the frame returned
 * from this function to process the response to the query.
 * A BTM transaction is started by a STA sending a query or by the AP sending
 * an autonomous request.  This callback is used for the former.
 *
 * @param apIndex - Access Point Index.
 * @param peerMACAddress - MAC address of the peer STA the Query was received from.
 * @param inQueryFrame - Query frame received from a non-AP STA.
 * @param inMemSize - Size of the memory allocated by the callback.  The caller
 *      should set to max size for the request.  Otherwise the callback may
 *      drop elements or return an error.
 * @param inRequestFrame - Frame to use for the response.  The caller
 *      allocates the memory for the response.  The caller may free the memory
 *      when the callback returns and the response is sent to the STA.
 * @return The status of the operation.
 * @retval RETURN_OK if successful.
 * @retval RETURN_ERR if any error is detected.
 *
 * @execution Synchronous
 * @sideeffect None
 *
 * @note This function must not suspend and must not invoke any blocking system
 * calls.
 */
typedef INT (* wifi_BTMQueryRequest_callback)(UINT apIndex,
                                                    CHAR *peerMac,
                                                    wifi_BTMQuery_t *query,
                                                    UINT inMemSize,
                                                    wifi_BTMRequest_t *request);

/* @description This call back is invoked when a STA responds to a BTM Request
 * from the gateway.
 *
 * @param apIndex - Access Point Index.
 * @param peerMACAddress - MAC address of the peer the response was received
 * from.
 * @param in_struct - Response frame received from a non-AP STA.
 * @return The status of the operation.
 * @retval RETURN_OK if successful.
 * @retval RETURN_ERR if any error is detected.
 *
 * @execution Synchronous
 * @sideeffect None
 *
 * @note This function must not suspend and must not invoke any blocking system
 * calls.
 */
typedef INT (* wifi_BTMResponse_callback)(UINT apIndex,
                                            CHAR *peerMac,
                                            wifi_BTMResponse_t *response);
/*
 * @description BTM Query callback registration function.
 *
 * @param callback_proc - wifi_newApAssociatedDevice_callback callback function
 *
 * @return The status of the operation.
 * @retval RETURN_OK if successful.
 * @retval RETURN_ERR if any error is detected.
 * @return The status of the operation.
 * @retval RETURN_OK if successful.
 * @retval RETURN_ERR if any error is detected.
 *
 * @execution Synchronous
 * @sideeffect None
 *
 * @note This function must not suspend and must not invoke any blocking system
 * calls.
 */
INT wifi_BTMQueryRequest_callback_register(UINT apIndex,
                                            wifi_BTMQueryRequest_callback btmQueryCallback,
                                            wifi_BTMResponse_callback btmResponseCallback);

/*
 * @description Set a BTM Request to a non-AP STA.  The callback register
 * function should be called first so that the response can be handled by the
 * application.
 *
 * @param apIndex; index of the vAP to send the request from.
 * @param peerMACAddress; MAC address of the peer device to send the request to.
 * @param in_struct; BTM Request Frame to send to the non-AP STA.
 * @return The status of the operation.
 * @retval RETURN_OK if successful.
 * @retval RETURN_ERR if any error is detected.
 *
 * @execution Synchronous
 * @sideeffect None
 *
 * @note This function must not suspend and must not invoke any blocking system
 * calls.
 */
INT wifi_setBTMRequest(UINT apIndex,
                        CHAR       *peerMac,
                        wifi_BTMRequest_t *request);

/* @description Get the BTM implemented value.  When not implemented the
 * gateway ignores a BTM query request as defined in 802.11-2016 section
 * 11.11.10.3.
 *
 * @param apIndex - AP Index the setting applies to.
 * @param activate - True for implemented false for not implemented.
 * @return The status of the operation.
 * @retval RETURN_OK if successful.
 * @retval RETURN_ERR if any error is detected.
 */
INT wifi_getBSSTransitionImplemented(UINT apIndex, BOOL *activate);

/* @description Set the BTM capability to activated or deactivated,
 * same as enabled or disabled.  The word "activated" is used here because
 * that's what's used in the 802.11 specification.  When deactivate the
 * gateway ignores a BTM report request as defined in 802.11-2016 section
 * 11.11.10.3.  The AP (apIndex) BSS Transition bit in any Extended Capabilities
 * element sent out is set corresponding to the activate parameter.
 *
 * @param apIndex - AP Index the setting applies to.
 * @param activate - True for activate false for deactivate.
 * @return The status of the operation.
 * @retval RETURN_OK if successful.
 * @retval RETURN_ERR if any error is detected.
 */
INT wifi_setBSSTransitionActivation(UINT apIndex, BOOL activate);

/* @description Get the BTM capability of activated or deactivated,
 * same as enabled or disabled.
 *
 * @param apIndex - AP Index the setting applies to.
 * @param activate - True for activate false for deactivate.
 * @return The status of the operation.
 * @retval RETURN_OK if successful.
 * @retval RETURN_ERR if any error is detected.
 */
INT wifi_getBSSTransitionActivation(UINT apIndex, BOOL *activate);

/* @description Get the BTM capability of an external STA.  Reports the value
 * of the BSS Transition bit in the Extended Capabilities element, if detected,
 * from an external STA.  Reports the latest value detected in the element
 * received by any vAP in any frame type.
 *
 * @param apIndex - AP the Extended Capabilities elements were received on.
 * @param extBTMCapabilities - structure with parallel arrays of peer MAC
 * addresses and BTM capability indicators.
 * @return The status of the operation.
 * @retval RETURN_OK if successful.
 * @retval RETURN_ERR if any error is detected.
 */
INT wifi_getBTMClientCapabilityList(UINT apIndex,
                                     wifi_BTMCapabilities_t *extBTMCapabilities);
