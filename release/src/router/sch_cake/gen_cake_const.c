/**
 * cake_const.c
 * No point in calculating the diffserv lookup tables at runtime
 * Dave Taht
 * 2015-12-21
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>


/*	List of known Diffserv codepoints:
 *
 *	Least Effort (CS1)
 *	Best Effort (CS0)
 *	Max Reliability (TOS1)
 *	Max Throughput (TOS2)
 *	Min Delay (TOS4)
 *	Assured Forwarding 1 (AF1x) - x3
 *	Assured Forwarding 2 (AF2x) - x3
 *	Assured Forwarding 3 (AF3x) - x3
 *	Assured Forwarding 4 (AF4x) - x3
 *	Precedence Class 2 (CS2)
 *	Precedence Class 3 (CS3)
 *	Precedence Class 4 (CS4)
 *	Precedence Class 5 (CS5)
 *	Precedence Class 6 (CS6)
 *	Precedence Class 7 (CS7)
 *	Voice Admit (VA)
 *	Expedited Forwarding (EF)

 *	Total 25 codepoints.
 */

/*	List of traffic classes in RFC 4594:
 *		(roughly descending order of contended priority)
 *		(roughly ascending order of uncontended throughput)
 *
 *	Network Control (CS6,CS7)      - routing traffic
 *	Telephony (EF,VA)         - aka. VoIP streams
 *	Signalling (CS5)               - VoIP setup
 *	Multimedia Conferencing (AF4x) - aka. video calls
 *	Realtime Interactive (CS4)     - eg. games
 *	Multimedia Streaming (AF3x)    - eg. YouTube, NetFlix, Twitch
 *	Broadcast Video (CS3)
 *	Low Latency Data (AF2x,TOS4)      - eg. database
 *	Ops, Admin, Management (CS2,TOS1) - eg. ssh
 *	Standard Service (CS0 & unrecognised codepoints)
 *	High Throughput Data (AF1x,TOS2)  - eg. web traffic
 *	Low Priority Data (CS1)           - eg. BitTorrent

 *	Total 12 traffic classes.
 */

static int min(int a, int b) {
	return (a < b ? a : b);
}

static void print_dscp(char *var, uint8_t *dscp) {
	printf("static const u8 %s[] = {", var);
	for(int i=0;i<64;i+=8) {
		for(int j=0; j<7; j++) {
			printf("%d, ",(int)dscp[i+j]);
		}
		printf("%d,\n\t\t\t\t", dscp[i+7]);
	}
	printf("};\n");
}

void precedence() {
	uint8_t dscp[64];
	for (int i = 0; i < 64; i++)
		dscp[i]= min((i >> 3), 8);
	print_dscp("precedence",dscp);
}

/*	Pruned list of traffic classes for typical applications:
 *
 *		Network Control          (CS6, CS7)
 *		Minimum Latency          (EF, VA, CS5, CS4)
 *		Interactive Shell        (CS2, TOS1)
 *		Low Latency Transactions (AF2x, TOS4)
 *		Video Streaming          (AF4x, AF3x, CS3)
 *		Bog Standard             (CS0 etc.)
 *		High Throughput          (AF1x, TOS2)
 *		Background Traffic       (CS1)
 *
 *		Total 8 traffic classes.
*/

void diffserv8() {
	uint8_t dscp[64];

	/* codepoint to class mapping */
	for (int i = 0; i < 64; i++)
		dscp[i] = 2;	/* default to best-effort */

	dscp[0x08] = 0;	/* CS1 */
	dscp[0x02] = 1;	/* TOS2 */
	dscp[0x18] = 3;	/* CS3 */
	dscp[0x04] = 4;	/* TOS4 */
	dscp[0x01] = 5;	/* TOS1 */
	dscp[0x10] = 5;	/* CS2 */
	dscp[0x20] = 6;	/* CS4 */
	dscp[0x28] = 6;	/* CS5 */
	dscp[0x2c] = 6;	/* VA */
	dscp[0x2e] = 6;	/* EF */
	dscp[0x30] = 7;	/* CS6 */
	dscp[0x38] = 7;	/* CS7 */

	for (int i = 2; i <= 6; i += 2) {
		dscp[0x08 + i] = 1;	/* AF1x */
		dscp[0x10 + i] = 4;	/* AF2x */
		dscp[0x18 + i] = 3;	/* AF3x */
		dscp[0x20 + i] = 3;	/* AF4x */
	}

	print_dscp("diffserv8",dscp);
}

/*  Diffserv structure specialised for Latency-Loss-Tradeoff spec.
 *		Loss Sensitive		(TOS1, TOS2)
 *		Best Effort
 *		Latency Sensitive	(TOS4, TOS5, VA, EF)
 *		Low Priority		(CS1)
 *		Network Control		(CS6, CS7)
 */

void diffserv_llt() {
	uint8_t dscp[64];
	/* codepoint to class mapping */

	for (int i = 0; i < 64; i++)
		dscp[i] = 1;	/* default to best-effort */

	dscp[0x01] = 0;	/* TOS1 */
	dscp[0x02] = 0;	/* TOS2 */
	dscp[0x04] = 2;	/* TOS4 */
	dscp[0x05] = 2;	/* TOS5 */
	dscp[0x2c] = 2;	/* VA */
	dscp[0x2e] = 2;	/* EF */
	dscp[0x08] = 3;	/* CS1 */
	dscp[0x30] = 4;	/* CS6 */
	dscp[0x38] = 4;	/* CS7 */

	print_dscp("diffserv_llt",dscp);

}

/*  Further pruned list of traffic classes for four-class system:
 *
 *	    Latency Sensitive  (CS7, CS6, EF, VA, CS5, CS4)
 *	    Streaming Media    (AF4x, AF3x, CS3, AF2x, TOS4, CS2, TOS1)
 *	    Best Effort        (CS0, AF1x, TOS2, and those not specified)
 *	    Background Traffic (CS1)
 *
 *		Total 4 traffic classes.
 */

void diffserv4() {
	uint8_t dscp[64];
	/* codepoint to class mapping */
	for (int i = 0; i < 64; i++)
		dscp[i] = 1;	/* default to best-effort */

	dscp[0x08] = 0;	/* CS1 */

	dscp[0x18] = 2;	/* CS3 */
	dscp[0x04] = 2;	/* TOS4 */
	dscp[0x01] = 2;	/* TOS1 */
	dscp[0x10] = 2;	/* CS2 */

	dscp[0x20] = 3;	/* CS4 */
	dscp[0x28] = 3;	/* CS5 */
	dscp[0x2c] = 3;	/* VA */
	dscp[0x2e] = 3;	/* EF */
	dscp[0x30] = 3;	/* CS6 */
	dscp[0x38] = 3;	/* CS7 */

	for (int i = 2; i <= 6; i += 2) {
		dscp[0x10 + i] = 2;	/* AF2x */
		dscp[0x18 + i] = 2;	/* AF3x */
		dscp[0x20 + i] = 2;	/* AF4x */
	}

	print_dscp("diffserv4",dscp);
}

/*  Simplified Diffserv structure with 3 tins.
 *		Low Priority		(CS1)
 *		Best Effort
 *		Latency Sensitive	(TOS4, VA, EF, CS6, CS7)
 */

void diffserv3() {
	uint8_t dscp[64];
	/* codepoint to class mapping */
	for (int i = 0; i < 64; i++)
		dscp[i] = 1;	/* default to best-effort */

	dscp[0x08] = 0;	/* CS1 */

	dscp[0x04] = 2;	/* TOS4 */
	dscp[0x2c] = 2;	/* VA */
	dscp[0x2e] = 2;	/* EF */
	dscp[0x30] = 2;	/* CS6 */
	dscp[0x38] = 2;	/* CS7 */

	print_dscp("diffserv3",dscp);
}

void besteffort() {
	uint8_t dscp[64];
	for (int i = 0; i < 64; i++)
		dscp[i]=0;
	print_dscp("besteffort",dscp);
}

int main(int argc, char **argv) {
	precedence();
	diffserv_llt();
	diffserv8();
	diffserv4();
	diffserv3();
	besteffort();
}
