/*
 * Copyright (c) 2002 - 2005 NetGroup, Politecnico di Torino (Italy)
 * Copyright (c) 2005 - 2008 CACE Technologies, Davis (California)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Politecnico di Torino, CACE Technologies
 * nor the names of its contributors may be used to endorse or promote
 * products derived from this software without specific prior written
 * permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef __PCAP_REMOTE_H__
#define __PCAP_REMOTE_H__

#include "pcap.h"
#include "sockutils.h"	// Needed for some structures (like SOCKET, sockaddr_in) which are used here

/*!
	\file pcap-remote.h

	This file keeps all the new definitions and typedefs that are exported to the user and
	that are needed for the RPCAP protocol.

	\warning All the RPCAP functions that are allowed to return a buffer containing
	the error description can return max PCAP_ERRBUF_SIZE characters.
	However there is no guarantees that the string will be zero-terminated.
	Best practice is to define the errbuf variable as a char of size 'PCAP_ERRBUF_SIZE+1'
	and to insert manually the termination char at the end of the buffer. This will
	guarantee that no buffer overflows occur even if we use the printf() to show
	the error on the screen.

	\warning This file declares some typedefs that MUST be of a specific size.
	These definitions (i.e. typedefs) could need to be changed on other platforms than
	Intel IA32.

	\warning This file defines some structures that are used to transfer data on the network.
	Be careful that you compiler MUST not insert padding into these structures
	for better alignment.
	These structures have been created in order to be correctly aligned to a 32 bits
	boundary, but be careful in any case.
*/

/*********************************************************
 *                                                       *
 * General definitions / typedefs for the RPCAP protocol *
 *                                                       *
 *********************************************************/

// All the following structures and typedef belongs to the Private Documentation
/*! \addtogroup remote_pri_struct
	\{
*/

#define RPCAP_DEFAULT_NETPORT "2002" /*!< Default port on which the RPCAP daemon is waiting for connections. */
/*! Default port on which the client workstation is waiting for connections in case of active mode. */
#define RPCAP_DEFAULT_NETPORT_ACTIVE "2003"
#define RPCAP_DEFAULT_NETADDR ""	/*!< Default network address on which the RPCAP daemon binds to. */
#define RPCAP_VERSION 0				/*!< Present version of the RPCAP protocol (0 = Experimental). */
#define RPCAP_TIMEOUT_INIT 90		/*!< Initial timeout for RPCAP connections (default: 90 sec) */
#define RPCAP_TIMEOUT_RUNTIME 180	/*!< Run-time timeout for RPCAP connections (default: 3 min) */
#define RPCAP_ACTIVE_WAIT 30		/*!< Waiting time betweek two attempts to open a connection, in active mode (default: 30 sec) */
#define RPCAP_SUSPEND_WRONGAUTH 1	/*!< If the authentication is wrong, stops 1 sec before accepting a new auth message */

/*!
	\brief Buffer used by socket functions to send-receive packets.
	In case you plan to have messages larger than this value, you have to increase it.
*/
#define RPCAP_NETBUF_SIZE 512000

/*!
	\brief Separators used for the host list.

	It is used:
	- by the rpcapd daemon, when you types a list of allowed connecting hosts
	- by the rpcap in active mode, when the client waits for incoming connections from other hosts
*/
#define RPCAP_HOSTLIST_SEP " ,;\n\r"

// WARNING: These could need to be changed on other platforms
typedef unsigned char uint8;		//!< Provides an 8-bits unsigned integer
typedef unsigned short uint16;		//!< Provides a 16-bits unsigned integer
typedef unsigned int uint32;		//!< Provides a 32-bits unsigned integer
typedef int int32;					//!< Provides a 32-bits integer

/*!
	\brief Keeps a list of all the opened connections in the active mode.

	This structure defines a linked list of items that are needed to keep the info required to
	manage the active mode.
	In other words, when a new connection in active mode starts, this structure is updated so that
	it reflects the list of active mode connections currently opened.
	This structure is required by findalldevs() and open_remote() to see if they have to open a new
	control connection toward the host, or they already have a control connection in place.
*/
struct activehosts
{
	struct sockaddr_storage host;
	SOCKET sockctrl;
	struct activehosts *next;
};

/*********************************************************
 *                                                       *
 * Protocol messages formats                             *
 *                                                       *
 *********************************************************/
// WARNING Take care you compiler does not insert padding for better alignments into these structs

//! Common header for all the RPCAP messages
struct rpcap_header
{
	uint8 ver;							//!< RPCAP version number
	uint8 type;							//!< RPCAP message type (error, findalldevs, ...)
	uint16 value;						//!< Message-dependent value (not always used)
	uint32 plen;						//!< Length of the payload of this RPCAP message
};

//! Format of the message for the interface description (findalldevs command)
struct rpcap_findalldevs_if
{
	uint16 namelen;						//!< Length of the interface name
	uint16 desclen;						//!< Length of the interface description
	uint32 flags;						//!< Interface flags
	uint16 naddr;						//!< Number of addresses
	uint16 dummy;						//!< Must be zero
};

//! Format of the message for the address listing (findalldevs command)
struct rpcap_findalldevs_ifaddr
{
	struct sockaddr_storage addr;		//!< Network address
	struct sockaddr_storage netmask;	//!< Netmask for that address
	struct sockaddr_storage broadaddr;	//!< Broadcast address for that address
	struct sockaddr_storage dstaddr;	//!< P2P destination address for that address
};

/*!
	\brief Format of the message of the connection opening reply (open command).

	This structure transfers over the network some of the values useful on the client side.
*/
struct rpcap_openreply
{
	int32 linktype;						//!< Link type
	int32 tzoff;						//!< Timezone offset
};

//! Format of the message that starts a remote capture (startcap command)
struct rpcap_startcapreq
{
	uint32 snaplen;						//!< Length of the snapshot (number of bytes to capture for each packet)
	uint32 read_timeout;				//!< Read timeout in milliseconds
	uint16 flags;						//!< Flags (see RPCAP_STARTCAPREQ_FLAG_xxx)
	uint16 portdata;					//!< Network port on which the client is waiting at (if 'serveropen')
};

//! Format of the reply message that devoted to start a remote capture (startcap reply command)
struct rpcap_startcapreply
{
	int32 bufsize;						//!< Size of the user buffer allocated by WinPcap; it can be different from the one we chose
	uint16 portdata;					//!< Network port on which the server is waiting at (passive mode only)
	uint16 dummy;						//!< Must be zero
};

/*!
	\brief Format of the header which encapsulates captured packets when transmitted on the network.

	This message requries the general header as well, since we want to be albe to exchange
	more information across the network in the future (for example statistics, and kind like that).
*/
struct rpcap_pkthdr
{
	uint32 timestamp_sec;	//!< 'struct timeval' compatible, it represents the 'tv_sec' field
	uint32 timestamp_usec;	//!< 'struct timeval' compatible, it represents the 'tv_usec' field
	uint32 caplen;			//!< Length of portion present in the capture
	uint32 len;				//!< Real length this packet (off wire)
	uint32 npkt;			//!< Ordinal number of the packet (i.e. the first one captured has '1', the second one '2', etc)
};

//! General header used for the pcap_setfilter() command; keeps just the number of BPF instructions
struct rpcap_filter
{
	uint16 filtertype;			//!< type of the filter transferred (BPF instructions, ...)
	uint16 dummy;				//!< Must be zero
	uint32 nitems;				//!< Number of items contained into the filter (e.g. BPF instructions for BPF filters)
};

//! Structure that keeps a single BPF instuction; it is repeated 'ninsn' times according to the 'rpcap_filterbpf' header
struct rpcap_filterbpf_insn
{
	uint16 code;			//!< opcode of the instuction
	uint8 jt;				//!< relative offset to jump to in case of 'true'
	uint8 jf;				//!< relative offset to jump to in case of 'false'
	int32 k;				//!< instruction-dependent value
};

//! Structure that keeps the data required for the authentication on the remote host
struct rpcap_auth
{
	uint16 type;			//!< Authentication type
	uint16 dummy;			//!< Must be zero
	uint16 slen1;			//!< Length of the first authentication item (e.g. username)
	uint16 slen2;			//!< Length of the second authentication item (e.g. password)
};

//! Structure that keeps the statistics about the number of packets captured, dropped, etc.
struct rpcap_stats
{
	uint32 ifrecv;		//!< Packets received by the kernel filter (i.e. pcap_stats.ps_recv)
	uint32 ifdrop;		//!< Packets dropped by the network interface (e.g. not enough buffers) (i.e. pcap_stats.ps_ifdrop)
	uint32 krnldrop;	//!< Packets dropped by the kernel filter (i.e. pcap_stats.ps_drop)
	uint32 svrcapt;		//!< Packets captured by the RPCAP daemon and sent on the network
};

//! Structure that is needed to set sampling parameters
struct rpcap_sampling
{
	uint8 method;		//!< Sampling method
	uint8 dummy1;		//!< Must be zero
	uint16 dummy2;		//!< Must be zero
	uint32 value;		//!< Parameter related to the sampling method
};

// Messages field coding
#define RPCAP_MSG_ERROR 1				/*!< Message that keeps an error notification */
#define RPCAP_MSG_FINDALLIF_REQ 2		/*!< Request to list all the remote interfaces */
#define RPCAP_MSG_OPEN_REQ 3			/*!< Request to open a remote device */
#define RPCAP_MSG_STARTCAP_REQ 4		/*!< Request to start a capture on a remote device */
#define RPCAP_MSG_UPDATEFILTER_REQ 5	/*!< Send a compiled filter into the remote device */
#define RPCAP_MSG_CLOSE 6				/*!< Close the connection with the remote peer */
#define RPCAP_MSG_PACKET 7				/*!< This is a 'data' message, which carries a network packet */
#define RPCAP_MSG_AUTH_REQ 8			/*!< Message that keeps the authentication parameters */
#define RPCAP_MSG_STATS_REQ 9			/*!< It requires to have network statistics */
#define RPCAP_MSG_ENDCAP_REQ 10			/*!< Stops the current capture, keeping the device open */
#define RPCAP_MSG_SETSAMPLING_REQ 11	/*!< Sset sampling parameters */

#define RPCAP_MSG_FINDALLIF_REPLY	(128+RPCAP_MSG_FINDALLIF_REQ)		/*!< Keeps the list of all the remote interfaces */
#define RPCAP_MSG_OPEN_REPLY		(128+RPCAP_MSG_OPEN_REQ)			/*!< The remote device has been opened correctly */
#define RPCAP_MSG_STARTCAP_REPLY	(128+RPCAP_MSG_STARTCAP_REQ)		/*!< The capture is staarting correctly */
#define RPCAP_MSG_UPDATEFILTER_REPLY (128+RPCAP_MSG_UPDATEFILTER_REQ)	/*!< The filter has been applied correctly on the remote device */
#define RPCAP_MSG_AUTH_REPLY		(128+RPCAP_MSG_AUTH_REQ)			/*!< Sends a message that says 'ok, authorization successful' */
#define RPCAP_MSG_STATS_REPLY		(128+RPCAP_MSG_STATS_REQ)			/*!< Message that keeps the network statistics */
#define RPCAP_MSG_ENDCAP_REPLY		(128+RPCAP_MSG_ENDCAP_REQ)			/*!< Confirms that the capture stopped succesfully */
#define RPCAP_MSG_SETSAMPLING_REPLY	(128+RPCAP_MSG_SETSAMPLING_REQ)		/*!< Confirms that the capture stopped succesfully */

#define RPCAP_STARTCAPREQ_FLAG_PROMISC 1	/*!< Enables promiscuous mode (default: disabled) */
#define RPCAP_STARTCAPREQ_FLAG_DGRAM 2		/*!< Use a datagram (i.e. UDP) connection for the data stream (default: use TCP)*/
#define RPCAP_STARTCAPREQ_FLAG_SERVEROPEN 4	/*!< The server has to open the data connection toward the client */
#define RPCAP_STARTCAPREQ_FLAG_INBOUND 8	/*!< Capture only inbound packets (take care: the flag has no effects with promiscuous enabled) */
#define RPCAP_STARTCAPREQ_FLAG_OUTBOUND 16	/*!< Capture only outbound packets (take care: the flag has no effects with promiscuous enabled) */

#define RPCAP_UPDATEFILTER_BPF 1			/*!< This code tells us that the filter is encoded with the BPF/NPF syntax */

// Network error codes
#define PCAP_ERR_NETW 1					/*!< Network error */
#define PCAP_ERR_INITTIMEOUT 2			/*!< The RPCAP initial timeout has expired */
#define PCAP_ERR_AUTH 3					/*!< Generic authentication error */
#define PCAP_ERR_FINDALLIF 4			/*!< Generic findalldevs error */
#define PCAP_ERR_NOREMOTEIF 5			/*!< The findalldevs was ok, but the remote end had no interfaces to list */
#define PCAP_ERR_OPEN 6					/*!< Generic pcap_open error */
#define PCAP_ERR_UPDATEFILTER 7			/*!< Generic updatefilter error */
#define PCAP_ERR_GETSTATS 8				/*!< Generic pcap_stats error */
#define PCAP_ERR_READEX 9				/*!< Generic pcap_next_ex error */
#define PCAP_ERR_HOSTNOAUTH 10			/*!< The host is not authorized to connect to this server */
#define PCAP_ERR_REMOTEACCEPT 11		/*!< Generic pcap_remoteaccept error */
#define PCAP_ERR_STARTCAPTURE 12		/*!< Generic pcap_startcapture error */
#define PCAP_ERR_ENDCAPTURE 13			/*!< Generic pcap_endcapture error */
#define PCAP_ERR_RUNTIMETIMEOUT	14		/*!< The RPCAP run-time timeout has expired */
#define PCAP_ERR_SETSAMPLING 15			/*!< Error diring the settings of sampling parameters */
#define PCAP_ERR_WRONGMSG 16			/*!< The other end endpoint sent a message which has not been recognized */
#define PCAP_ERR_WRONGVER 17			/*!< The other end endpoint ahs a version number that is not compatible with our */
/*!
	\}
*/ // end of private documentation

/*********************************************************
 *                                                       *
 * Exported funtion prototypes                           *
 *                                                       *
 *********************************************************/
int pcap_opensource_remote(pcap_t *p, struct pcap_rmtauth *auth);
int pcap_startcapture_remote(pcap_t *fp);

int pcap_read_nocb_remote(pcap_t *p, struct pcap_pkthdr **pkt_header, u_char **pkt_data);
int pcap_read_remote(pcap_t *p, int cnt, pcap_handler callback, u_char *user);
int pcap_updatefilter_remote(pcap_t *fp, struct bpf_program *prog);
int pcap_setfilter_remote(pcap_t *fp, struct bpf_program *prog);
int pcap_stats_remote(pcap_t *p, struct pcap_stat *ps);
int pcap_setsampling_remote(pcap_t *p);
struct pcap_stat *pcap_stats_ex_remote(pcap_t *p);
void pcap_cleanup_remote(pcap_t *p);

void rpcap_createhdr(struct rpcap_header *header, uint8 type, uint16 value, uint32 length);
int rpcap_deseraddr(struct sockaddr_storage *sockaddrin, struct sockaddr_storage **sockaddrout, char *errbuf);
int rpcap_checkmsg(char *errbuf, SOCKET sock, struct rpcap_header *header, uint8 first, ...);
int rpcap_senderror(SOCKET sock, char *error, unsigned short errcode, char *errbuf);
int rpcap_sendauth(SOCKET sock, struct pcap_rmtauth *auth, char *errbuf);

int rpcap_remoteact_getsock(const char *host, char *errbuf);

#endif // endif
