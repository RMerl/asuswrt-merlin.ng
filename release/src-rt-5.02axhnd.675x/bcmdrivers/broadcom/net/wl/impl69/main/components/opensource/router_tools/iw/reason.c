#include <stdint.h>
#include "iw.h"

static const char *reason_table[] = {
	[1] = "Unspecified",
	[2] = "Previous authentication no longer valid",
	[3] = "Deauthenticated because sending station is leaving (or has left) the IBSS or ESS",
	[4] = "Disassociated due to inactivity",
	[5] = "Disassociated because AP is unable to handle all currently associated STA",
	[6] = "Class 2 frame received from non-authenticated station",
	[7] = "Class 3 frame received from non-authenticated station",
	[8] = "Disassociated because sending station is leaving (or has left) the BSS",
	[9] = "Station requesting (re)association is not authenticated with responding station",
	[10] = "Disassociated because the information in the Power Capability element is unacceptable",
	[11] = "Disassociated because the information in the Supported Channels element is unacceptable",
	[13] = "Invalid information element",
	[14] = "MIC failure",
	[15] = "4-way handshake timeout",
	[16] = "Group key update timeout",
	[17] = "Information element in 4-way handshake different from (Re-)associate request/Probe response/Beacon",
	[18] = "Multicast cipher is not valid",
	[19] = "Unicast cipher is not valid",
	[20] = "AKMP is not valid",
	[21] = "Unsupported RSNE version",
	[22] = "Invalid RSNE capabilities",
	[23] = "IEEE 802.1X authentication failed",
	[24] = "Cipher Suite rejected per security policy",
	[31] = "TS deleted because QoS AP lacks sufficient bandwidth for this QoS STA due to a change in BSS service characteristics or operational mode",
	[32] = "Disassociated for unspecified QoS-related reason",
	[33] = "Disassociated because QAP lacks sufficient bandwidth for this STA",
	[34] = "Disassociated because of excessive frame losses and/or poor channel conditions",
	[35] = "Disassociated because QSTA is transmitting outside the limits of its polled TXOPs",
	[36] = "Requested from peer QSTA as the QSTA is leaving the QBSS (or resetting)",
	[37] = "Requested from peer QSTA as it does not want to use Traffic Stream",
	[38] = "Requested from peer QSTA as the QSTA received frames indicated Traffic Stream for which it has not set up",
	[39] = "Requested from peer QSTA due to time out",
	[40] = "Requested from peer QSTA as the QSTA is leaving the QBSS (or resetting)",
	[41] = "Requested from peer QSTA as it does not want to receive frames directly from the QSTA",
	[42] = "Requested from peer QSTA as the QSTA received DLP frames for which it has not set up",
	[43] = "Requested from peer QSTA as it does not want to use Block Ack",
	[44] = "Requested from peer QSTA as the QSTA received frames indicated Block Acknowledgement policy for which it has not set up",
	[45] = "Peer QSTA does not support the requested cipher suite",
};

const char *get_reason_str(uint16_t reason)
{
	if (reason < ARRAY_SIZE(reason_table) && reason_table[reason])
		return reason_table[reason];
	return "<unknown>";
}
