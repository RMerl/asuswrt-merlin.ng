#include <stdint.h>
#include "iw.h"

static const char *status_table[] = {
	[0] = "Successful",
	[1] = "Unspecified failure",
	[10] = "Cannot support all requested capabilities in the capability information field",
	[11] = "Reassociation denied due to inability to confirm that association exists",
	[12] = "Association denied due to reason outside the scope of this standard",
	[13] = "Responding station does not support the specified authentication algorithm",
	[14] = "Received an authentication frame with authentication transaction sequence number out of expected sequence",
	[15] = "Authentication rejected because of challenge failure",
	[16] = "Authentication rejected due to timeout waiting for next frame in sequence",
	[17] = "Association denied because AP is unable to handle additional associated STA",
	[18] = "Association denied due to requesting station not supporting all of the data rates in the BSSBasicRateSet parameter",
	[19] = "Association denied due to requesting station not supporting the short preamble option",
	[20] = "Association denied due to requesting station not supporting the PBCC modulation option",
	[21] = "Association denied due to requesting station not supporting the channel agility option",
	[22] = "Association request rejected because Spectrum Management capability is required",
	[23] = "Association request rejected because the information in the Power Capability element is unacceptable",
	[24] = "Association request rejected because the information in the Supported Channels element is unacceptable",
	[25] = "Association request rejected due to requesting station not supporting the short slot time option",
	[26] = "Association request rejected due to requesting station not supporting the ER-PBCC modulation option",
	[27] = "Association denied due to requesting STA not supporting HT features",
	[28] = "R0KH Unreachable",
	[29] = "Association denied because the requesting STA does not support the PCO transition required by the AP",
	[30] = "Association request rejected temporarily; try again later",
	[31] = "Robust Management frame policy violation",
	[32] = "Unspecified, QoS related failure",
	[33] = "Association denied due to QAP having insufficient bandwidth to handle another QSTA",
	[34] = "Association denied due to poor channel conditions",
	[35] = "Association (with QBSS) denied due to requesting station not supporting the QoS facility",
	[37] = "The request has been declined",
	[38] = "The request has not been successful as one or more parameters have invalid values",
	[39] = "The TS has not been created because the request cannot be honored. However, a suggested Tspec is provided so that the initiating QSTA may attempt to send another TS with the suggested changes to the TSpec",
	[40] = "Invalid Information Element",
	[41] = "Group Cipher is not valid",
	[42] = "Pairwise Cipher is not valid",
	[43] = "AKMP is not valid",
	[44] = "Unsupported RSN IE version",
	[45] = "Invalid RSN IE Capabilities",
	[46] = "Cipher suite is rejected per security policy",
	[47] = "The TS has not been created. However, the HC may be capable of creating a TS, in response to a request, after the time indicated in the TS Delay element",
	[48] = "Direct link is not allowed in the BSS by policy",
	[49] = "Destination STA is not present within this QBSS",
	[50] = "The destination STA is not a QSTA",
	[51] = "Association denied because Listen Interval is too large",
	[52] = "Invalid Fast BSS Transition Action Frame Count",
	[53] = "Invalid PMKID",
	[54] = "Invalid MDIE",
	[55] = "Invalid FTIE",
};

const char *get_status_str(uint16_t status)
{
	if (status < ARRAY_SIZE(status_table) && status_table[status])
		return status_table[status];
	return "<unknown>";
}
