/*************************************************************************
 *
 * ivi_map_tcp.c :
 *
 * This file defines the TCP mapping list data structure and basic 
 * operations with TCP state tracking, which will be used in other modules.
 *
 * Copyright (C) 2013 CERNET Network Center
 * All rights reserved.
 * 
 * Design and coding: 
 *   Xing Li <xing@cernet.edu.cn> 
 *	 Congxiao Bao <congxiao@cernet.edu.cn>
 *   Guoliang Han <bupthgl@gmail.com>
 * 	 Yuncheng Zhu <haoyu@cernet.edu.cn>
 * 	 Wentao Shang <wentaoshang@gmail.com>
 * 	 
 * 
 * Contributions:
 *
 * This file is part of MAP-T/MAP-E Kernel Module.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * You should have received a copy of the GNU General Public License 
 * along with MAP-T/MAP-E Kernel Module. If not, see 
 * <http://www.gnu.org/licenses/>.
 *
 * For more versions, please send an email to <bupthgl@gmail.com> to
 * obtain an password to access the svn server.
 *
 * LIC: GPLv2
 *
 ************************************************************************/

#include "ivi_map_tcp.h"
#include "ivi_portmap.h"

#define SECS * 1
#define MINS * 60 SECS
#define HOURS * 60 MINS
#define DAYS * 24 HOURS

#define STATE_OPTION_WINDOW_SCALE      0x01    // Sender uses windows scale
#define STATE_OPTION_SACK_PERM         0x02    // Sender allows SACK option
#define STATE_OPTION_CLOSE_INIT        0x04    // Sender sent Fin first
#define STATE_OPTION_DATA_UNACK        0x10    // Has unacknowledged data
#define STATE_OPTION_MAXACK_SET        0x20    // MaxAck in sender state info has been set. 
                                               // This flag is set when we see the first non-zero
                                               // ACK in TCP header sent by the sender.

typedef enum _FILTER_STATUS {
	FILTER_ACCEPT = 0,    // Everything is good, let the packet pass
	FILTER_DROP,          // Packet is invalid, but the state is not tainted
	FILTER_DROP_CLEAN     // Both packet and state is invalid
} FILTER_STATUS, *PFILTER_STATUS;

// TCP timeouts
static unsigned int tcp_timeouts[TCP_STATUS_MAX] __read_mostly = {
	0,        // TCP_STATUS_NONE
	2 MINS,   // TCP_STATUS_SYN_SENT
	60 SECS,  // TCP_STATUS_SYN_RECV
	5 DAYS,   // TCP_STATUS_ESTABLISHED
	2 MINS,   // TCP_STATUS_FIN_WAIT
	60 SECS,  // TCP_STATUS_CLOSE_WAIT
	30 SECS,  // TCP_STATUS_LAST_ACK
	2 MINS,   // TCP_STATUS_TIME_WAIT
	10 SECS,  // TCP_STATUS_CLOSE
	2 MINS    // TCP_STATUS_SYN_SENT2
};

static unsigned int TcpTimeOutMaxRetrans __read_mostly = 5 MINS;
static unsigned int TcpTimeOutUnack  __read_mostly     = 5 MINS;

static int TcpMaxRetrans __read_mostly = 3;

// Short name for TCP_STATUS
#define sNO TCP_STATUS_NONE
#define sSS TCP_STATUS_SYN_SENT
#define sSR TCP_STATUS_SYN_RECV
#define sES TCP_STATUS_ESTABLISHED
#define sFW TCP_STATUS_FIN_WAIT
#define sCW TCP_STATUS_CLOSE_WAIT
#define sLA TCP_STATUS_LAST_ACK
#define sTW TCP_STATUS_TIME_WAIT
#define sCL TCP_STATUS_CLOSE
#define sS2 TCP_STATUS_SYN_SENT2
#define sIV TCP_STATUS_MAX
#define sIG TCP_STATUS_IGNORE

/* What TCP flags are set from RST/SYN/FIN/ACK. */
enum tcp_bit_set {
	TCP_SYN_SET = 0,
	TCP_SYNACK_SET,
	TCP_FIN_SET,
	TCP_ACK_SET,
	TCP_RST_SET,
	TCP_NONE_SET,
};

/*
 * The TCP state transition table needs a few words...
 *
 * We are the man in the middle. All the packets go through us
 * but might get lost in transit to the destination.
 * It is assumed that the destinations can't receive segments
 * we haven't seen.
 *
 * The checked segment is in window, but our windows are *not*
 * equivalent with the ones of the sender/receiver. We always
 * try to guess the state of the current sender.
 *
 * The meaning of the states are:
 *
 * NONE:         initial state
 * SYN_SENT:     SYN-only packet seen
 * SYN_SENT2:    SYN-only packet seen from reply dir, simultaneous open
 * SYN_RECV:     SYN-ACK packet seen
 * ESTABLISHED:  ACK packet seen
 * FIN_WAIT:     FIN packet seen
 * CLOSE_WAIT:   ACK seen (after FIN)
 * LAST_ACK:     FIN seen (after FIN)
 * TIME_WAIT:    last ACK seen
 * CLOSE:        closed connection (RST)
 *
 * Packets marked as IGNORED (sIG):
 *    if they may be either invalid or valid
 *    and the receiver may send back a connection
 *    closing RST or a SYN/ACK.
 *
 * Packets marked as INVALID (sIV):
 *    if we regard them as truly invalid packets
 */
static const u8 tcp_state_table[PACKET_DIR_MAX][6][TCP_STATUS_MAX] = {
    {
/* LOCAL */
/*        sNO, sSS, sSR, sES, sFW, sCW, sLA, sTW, sCL, sS2    */
/*syn*/ { sSS, sSS, sIG, sIG, sIG, sIG, sIG, sSS, sSS, sS2 },
/*
 *    sNO -> sSS    Initialize a new connection
 *    sSS -> sSS    Retransmitted SYN
 *    sS2 -> sS2    Late retransmitted SYN
 *    sSR -> sIG
 *    sES -> sIG    Error: SYNs in window outside the SYN_SENT state
 *                  are errors. Receiver will reply with RST
 *                  and close the connection.
 *                  Or we are not in sync and hold a dead connection.
 *    sFW -> sIG
 *    sCW -> sIG
 *    sLA -> sIG
 *    sTW -> sSS    Reopened connection (RFC 1122).
 *    sCL -> sSS
 */
/*           sNO, sSS, sSR, sES, sFW, sCW, sLA, sTW, sCL, sS2    */
/*synack*/ { sIV, sIV, sIG, sIG, sIG, sIG, sIG, sIG, sIG, sSR },
/*
 *    sNO -> sIV    Too late and no reason to do anything
 *    sSS -> sIV    Client can't send SYN and then SYN/ACK
 *    sS2 -> sSR    SYN/ACK sent to SYN2 in simultaneous open
 *    sSR -> sIG
 *    sES -> sIG    Error: SYNs in window outside the SYN_SENT state
 *                  are errors. Receiver will reply with RST
 *                  and close the connection.
 *                  Or we are not in sync and hold a dead connection.
 *    sFW -> sIG
 *    sCW -> sIG
 *    sLA -> sIG
 *    sTW -> sIG
 *    sCL -> sIG
 */
/*        sNO, sSS, sSR, sES, sFW, sCW, sLA, sTW, sCL, sS2    */
/*fin*/ { sIV, sIV, sFW, sFW, sLA, sLA, sLA, sTW, sCL, sIV },
/*
 *    sNO -> sIV    Too late and no reason to do anything...
 *    sSS -> sIV    Client migth not send FIN in this state:
 *                  we enforce waiting for a SYN/ACK reply first.
 *    sS2 -> sIV
 *    sSR -> sFW    Close started.
 *    sES -> sFW
 *    sFW -> sLA    FIN seen in both directions, waiting for
 *                  the last ACK.
 *                  Migth be a retransmitted FIN as well...
 *    sCW -> sLA
 *    sLA -> sLA    Retransmitted FIN. Remain in the same state.
 *    sTW -> sTW
 *    sCL -> sCL
 */
/*        sNO, sSS, sSR, sES, sFW, sCW, sLA, sTW, sCL, sS2    */
/*ack*/ { sES, sIV, sES, sES, sCW, sCW, sTW, sTW, sCL, sIV },
/*
 *    sNO -> sES    Assumed.
 *    sSS -> sIV    ACK is invalid: we haven't seen a SYN/ACK yet.
 *    sS2 -> sIV
 *    sSR -> sES    Established state is reached.
 *    sES -> sES    :-)
 *    sFW -> sCW    Normal close request answered by ACK.
 *    sCW -> sCW
 *    sLA -> sTW    Last ACK detected.
 *    sTW -> sTW    Retransmitted last ACK. Remain in the same state.
 *    sCL -> sCL
 */
/*         sNO, sSS, sSR, sES, sFW, sCW, sLA, sTW, sCL, sS2    */
/*rst*/  { sIV, sCL, sCL, sCL, sCL, sCL, sCL, sCL, sCL, sCL },
/*none*/ { sIV, sIV, sIV, sIV, sIV, sIV, sIV, sIV, sIV, sIV }
    },
    {
/* REMOTE */
/*        sNO, sSS, sSR, sES, sFW, sCW, sLA, sTW, sCL, sS2    */
/*syn*/ { sIV, sS2, sIV, sIV, sIV, sIV, sIV, sIV, sIV, sS2 },
/*
 *    sNO -> sIV    Never reached.
 *    sSS -> sS2    Simultaneous open
 *    sS2 -> sS2    Retransmitted simultaneous SYN
 *    sSR -> sIV    Invalid SYN packets sent by the server
 *    sES -> sIV
 *    sFW -> sIV
 *    sCW -> sIV
 *    sLA -> sIV
 *    sTW -> sIV    Reopened connection, but server may not do it.
 *    sCL -> sIV
 */
/*           sNO, sSS, sSR, sES, sFW, sCW, sLA, sTW, sCL, sS2    */
/*synack*/ { sIV, sSR, sSR, sIG, sIG, sIG, sIG, sIG, sIG, sSR },
/*
 *    sSS -> sSR    Standard open.
 *    sS2 -> sSR    Simultaneous open
 *    sSR -> sSR    Retransmitted SYN/ACK.
 *    sES -> sIG    Late retransmitted SYN/ACK?
 *    sFW -> sIG    Might be SYN/ACK answering ignored SYN
 *    sCW -> sIG
 *    sLA -> sIG
 *    sTW -> sIG
 *    sCL -> sIG
 */
/*        sNO, sSS, sSR, sES, sFW, sCW, sLA, sTW, sCL, sS2    */
/*fin*/ { sIV, sIV, sFW, sFW, sLA, sLA, sLA, sTW, sCL, sIV },
/*
 *    sSS -> sIV    Server might not send FIN in this state.
 *    sS2 -> sIV
 *    sSR -> sFW    Close started.
 *    sES -> sFW
 *    sFW -> sLA    FIN seen in both directions.
 *    sCW -> sLA
 *    sLA -> sLA    Retransmitted FIN.
 *    sTW -> sTW
 *    sCL -> sCL
 */
/*        sNO, sSS, sSR, sES, sFW, sCW, sLA, sTW, sCL, sS2    */
/*ack*/ { sIV, sIG, sSR, sES, sCW, sCW, sTW, sTW, sCL, sIG },
/*
 *    sSS -> sIG    Might be a half-open connection.
 *    sS2 -> sIG
 *    sSR -> sSR    Might answer late resent SYN.
 *    sES -> sES    :-)
 *    sFW -> sCW    Normal close request answered by ACK.
 *    sCW -> sCW
 *    sLA -> sTW    Last ACK detected.
 *    sTW -> sTW    Retransmitted last ACK.
 *    sCL -> sCL
 */
/*         sNO, sSS, sSR, sES, sFW, sCW, sLA, sTW, sCL, sS2    */
/*rst*/  { sIV, sCL, sCL, sCL, sCL, sCL, sCL, sCL, sCL, sCL },
/*none*/ { sIV, sIV, sIV, sIV, sIV, sIV, sIV, sIV, sIV, sIV }
    }
};


static unsigned int get_bits_index(const struct tcphdr *tcph)
{
	if (tcph->rst) return TCP_RST_SET;
	else if (tcph->syn) return (tcph->ack ? TCP_SYNACK_SET : TCP_SYN_SET);
	else if (tcph->fin) return TCP_FIN_SET;
	else if (tcph->ack) return TCP_ACK_SET;
	else return TCP_NONE_SET;
}


/*  TCP connection tracking based on 'Real Stateful TCP Packet Filtering
    in IP Filter' by Guido van Rooij.

    http://www.sane.nl/events/sane2000/papers.html
    http://www.darkart.com/mirrors/www.obfuscation.org/ipf/

    The boundaries and the conditions are changed according to RFC793:
    the packet must intersect the window (i.e. segments may be
    after the right or before the left edge) and thus receivers may ACK
    segments after the right edge of the window.

    MaxEnd    = max(sack + max(win,1)) seen in reply packets
    MaxWindow = max(max(win, 1)) + (sack - ack) seen in sent packets
    MaxWindow += seq + len - sender.MaxEnd
            if seq + len > sender.MaxEnd
    End       = max(seq + len) seen in sent packets

    I.   Upper bound for valid data:     seq <= sender.MaxEnd
    II.  Lower bound for valid data:     seq + len >= sender.End - receiver.MaxWindow
    III. Upper bound for valid (s)ack:   sack <= receiver.End
    IV.  Lower bound for valid (s)ack:   sack >= receiver.End - MAXACKWINDOW

    where sack is the highest right edge of sack block found in the packet
    or ack in the case of packet without SACK option.

    The upper bound limit for a valid (s)ack is not ignored -
    we doesn't have to deal with fragments.
*/

static inline __u32 segment_seq_plus_len(__u32 seq, size_t len, const struct tcphdr *tcph)
{
	return (seq + len - tcph->doff * 4 + (tcph->syn ? 1 : 0) + (tcph->fin ? 1 : 0));
}

#define MAXACKWINCONST         66000
#define MAXACKWINDOW(sender) ((sender)->MaxWindow > MAXACKWINCONST ? (sender)->MaxWindow : MAXACKWINCONST)


static void tcp_options(struct tcphdr *th, PTCP_STATE_INFO StateInfo)
{
	unsigned char *ptr = (unsigned char *)(th) + sizeof(struct tcphdr);
	int optlen = (th->doff*4) - sizeof(struct tcphdr);
	
	if (optlen == 0)
		return;

	StateInfo->Scale = 0;
	StateInfo->Options = 0;
	
	while (optlen > 0) {
		unsigned char optcode = *ptr++;
		unsigned char optsize;
		
		switch (optcode) {
			case TCPOPT_EOL:
				// End of options
				return;
				
			case TCPOPT_NOP:
				// Zero padding
				optlen--;
				continue;
				
			default:
				optsize = *ptr++;
				
				if (optsize < 2) {
					// "silly options"
					return;
				}
				
				if (optsize > optlen) {
					break;  // don't parse partial options
				}
				
				if (optcode == TCPOPT_SACK_PERM && optsize == TCPOLEN_SACK_PERM) {
					StateInfo->Options |= STATE_OPTION_SACK_PERM;
				}
				else if (optcode == TCPOPT_WINDOW && optsize == TCPOLEN_WINDOW) {
					StateInfo->Scale = *ptr;
					
					if (StateInfo->Scale > 14) {
						// See RFC1323
						StateInfo->Scale = 14;
					}
					StateInfo->Options |= STATE_OPTION_WINDOW_SCALE;
				}
				ptr += optsize - 2;
				optlen -= optsize;
				break;
		}
	}
}


#if 0
static void tcp_sack(struct tcphdr *th, __u32 *sack)
{
	unsigned char *ptr = (unsigned char *)(th) + sizeof(struct tcphdr);
	int optlen = (th->doff*4) - sizeof(struct tcphdr);

	if (optlen == 0)
		return;

	while (optlen > 0) {
		unsigned char optcode = *ptr++;
		unsigned char optsize, i;

		switch (optcode) {
			case TCPOPT_EOL:
				// End of options
				return;

			case TCPOPT_NOP:
				// Zero padding
				optlen--;
				continue;

			default:
				optsize = *ptr++;

				if (optsize < 2) {
					// "silly options"
					return;
				}
				
				if (optsize > optlen) {
					break;  // don't parse partial options
				}

				if (optcode == TCPOPT_SACK && optsize >= (TCPOLEN_SACK_BASE + TCPOLEN_SACK_PERBLOCK)
				    && (((optsize - TCPOLEN_SACK_BASE) % TCPOLEN_SACK_PERBLOCK) == 0)) 
				{
					for (i = 0; i < (optsize - TCPOLEN_SACK_BASE); i += TCPOLEN_SACK_PERBLOCK) {
						// Read the right edge of the SACK block, see RFC2018
						__u32 tmp = get_unaligned_be32((__be32 *)(ptr+i)+1);
						if (after(tmp, *sack)) {
							*sack = tmp;
						}
					}
					return;
				}
				ptr += optsize - 2;
				optlen -= optsize;
				break;
		}
	}
}


static bool tcp_in_window(struct tcphdr *th, __u32 len, PACKET_DIR dir, PTCP_STATE_CONTEXT StateContext)
{
	PTCP_STATE_INFO sender = &(StateContext->Seen[dir]);
	PTCP_STATE_INFO receiver = &(StateContext->Seen[!dir]);
	__u32 seq, ack, sack, end, win, swin;
	bool res;

	// Get the required data from header
	seq = ntohl(th->seq);
	ack = sack = ntohl(th->ack_seq);
	win = ntohs(th->window);
	end = segment_seq_plus_len(seq, len, th);

	if (receiver->Options & STATE_OPTION_SACK_PERM) {
		// Receiver allows SACK option from sender
		tcp_sack(th, &sack);
	}

	if (sender->MaxWindow == 0) {
		// Initialize sender data
		if (th->syn) {
			// SYN-ACK reply to a SYN or SYN from receiver in simultaneous open
			// We set receiver->MaxWin to 0 in CreateTcpStateContext().
			sender->End = sender->MaxEnd = end;
			sender->MaxWindow = ((win == 0) ? 1 : win);
			// Read TCP options on SYN packet.
			tcp_options(th, sender);

			/*
			 * RFC 1323:
			 * Both sides must send the Window Scale option
			 * to enable window scaling in either direction.
			 */
			if (!(sender->Options & STATE_OPTION_WINDOW_SCALE && receiver->Options & STATE_OPTION_WINDOW_SCALE)) {
				// At least one side does not support window scale.
				sender->Scale = receiver->Scale = 0;
			}
		}
	}
	else if (((StateContext->Status == TCP_STATUS_SYN_SENT && dir == PACKET_DIR_LOCAL)
		|| (StateContext->Status == TCP_STATUS_SYN_RECV && dir == PACKET_DIR_REMOTE))
		&& after(end, sender->End))
	{
		/*
		 * RFC 793: "if a TCP is reinitialized ... then it need
		 * not wait at all; it must only be sure to use sequence
		 * numbers larger than those recently used."
		 */
		sender->End = sender->MaxEnd = end;
		sender->MaxWindow = ((win == 0) ? 1 : win);
		// Read TCP options on SYN packet.
		tcp_options(th, sender);
    	}

	if (!(th->ack)) {
		// If there is no ACK, just pretend it was set and OK.
		ack = sack = receiver->End;
	} else if (((tcp_flag_word(th) & (TCP_FLAG_ACK|TCP_FLAG_RST)) == (TCP_FLAG_ACK|TCP_FLAG_RST)) && (ack == 0)) {
		// Broken TCP stacks, that set ACK in RST packets as well with zero ack value.
		ack = sack = receiver->End;
	}

	if (seq == end && (!(th->rst) 
	    || (seq == 0 && StateContext->Status == TCP_STATUS_SYN_SENT)))
	{
		/*
		 * Packets contains no data: we assume it is valid
		 * and check the ack value only.
		 * However RST segments are always validated by their
		 * SEQ number, except when seq == 0 (reset sent answering
		 * SYN.
		 */
		seq = end = sender->End;
	}
#ifdef IVI_DEBUG_TCP
	printk(KERN_DEBUG "tcp_in_window: dir = %u, seq = %u, ack = %u, sack = %u, win = %u, end = %u\n", dir, seq, ack, sack, win, end);
	printk(KERN_DEBUG "tcp_in_window: sender end=%u maxend=%u maxwin=%u scale=%u\n", 
			sender->End, sender->MaxEnd, sender->MaxWindow, sender->Scale);
	printk(KERN_DEBUG "tcp_in_window: receiver end=%u maxend=%u maxwin=%u scale=%u\n", 
			receiver->End, receiver->MaxEnd, receiver->MaxWindow, receiver->Scale);
	printk(KERN_DEBUG "tcp_in_window: I=%d II=%d III=%d IV=%d\n",
			before(seq, sender->MaxEnd + 1),
			after(end, sender->End - receiver->MaxWindow - 1),
			before(sack, receiver->End + 1),
			after(sack, receiver->End - MAXACKWINDOW(sender) - 1));
#endif    
	if (before(seq, sender->MaxEnd + 1) && after(end, sender->End - receiver->MaxWindow - 1) &&
	    before(sack, receiver->End + 1) && after(sack, receiver->End - MAXACKWINDOW(sender) - 1))
	{
		/*
		 * Take into account window scaling (RFC 1323).
		 */
		if (!(th->syn))
			win <<= sender->Scale;

		/*
		 * Update sender data.
		 */
		swin = win + (sack - ack);
		if (sender->MaxWindow < swin) {
			sender->MaxWindow = swin;
		}
		if (after(end, sender->End)) {
			sender->End = end;
			sender->Options |= STATE_OPTION_DATA_UNACK;
		}
		if (th->ack) {
			if (!(sender->Options & STATE_OPTION_MAXACK_SET)) {
				sender->MaxAck = ack;
				sender->Options |= STATE_OPTION_MAXACK_SET;
			} else if (after(ack, sender->MaxAck)) {
				sender->MaxAck = ack;
			}
		}

		/*
		 * Update receiver data.
		 */
		if (receiver->MaxWindow != 0 && after(end, sender->MaxEnd)) {
			receiver->MaxWindow += end - sender->MaxEnd;
		}
		if (after(sack + win, receiver->MaxEnd - 1)) {
			receiver->MaxEnd = sack + win;
			if (win == 0) {
				receiver->MaxEnd++;
			}
		}
		if (ack == receiver->End) {
			receiver->Options &= ~STATE_OPTION_MAXACK_SET;
			receiver->Options &= ~STATE_OPTION_DATA_UNACK;
		}

		/*
		 * Check retransmissions.
		 */
		if (get_bits_index(th) == TCP_ACK_SET) {
			if (StateContext->LastDir == dir
				&& StateContext->LastSeq == seq
				&& StateContext->LastAck == ack
				&& StateContext->LastEnd == end
				&& StateContext->LastWindow == win)
			{
				StateContext->RetransCount++;
			} else {
				StateContext->LastDir = dir;
				StateContext->LastSeq = seq;
				StateContext->LastAck = ack;
				StateContext->LastEnd = end;
				StateContext->LastWindow = win;
				StateContext->RetransCount = 0;
			}
		}
		res = true;
	} else {
		res = false;
	}
#ifdef IVI_DEBUG_TCP
	printk(KERN_DEBUG "tcp_in_window: sender end=%u maxend=%u maxwin=%u scale=%u\n", 
			sender->End, sender->MaxEnd, sender->MaxWindow, sender->Scale);
	printk(KERN_DEBUG "tcp_in_window: receiver end=%u maxend=%u maxwin=%u scale=%u\n", 
			receiver->End, receiver->MaxEnd, receiver->MaxWindow, receiver->Scale);
#endif
	return res;
}
#endif


FILTER_STATUS CreateTcpStateContext(struct tcphdr *th, __u32 len, PTCP_STATE_CONTEXT StateContext)
{
	PTCP_STATE_INFO sender = &(StateContext->Seen[0]);   // Sender is always local
	PTCP_STATE_INFO receiver = &(StateContext->Seen[1]); // Receiver is always remote
	unsigned int index = get_bits_index(th);
	__u32 seq = ntohl(th->seq);

	TCP_STATUS NewStatus = tcp_state_table[0][index][TCP_STATUS_NONE];  // We always start from NONE state

	if (NewStatus != TCP_STATUS_SYN_SENT) {
		// Invalid packet or we are in middle of a connection, which is not supported now
#ifdef IVI_DEBUG_MAP_TCP
		printk(KERN_ERR "CreateTcpStateContext: invalid new packet causing state change to %d, drop. index = %d\n", NewStatus, index);
#endif
		return FILTER_DROP_CLEAN;
	}

	// SYN packet from local
	sender->End = segment_seq_plus_len(seq, len, th);
	sender->MaxEnd = sender->End;
	sender->MaxWindow = ntohs(th->window);
	if (sender->MaxWindow == 0) {
		// Window probing
		sender->MaxWindow = 1;
	}
	// Read window scale and SACK permit options in SYN packet
	tcp_options(th, sender);

	receiver->Options = 0;
	receiver->End = 0;
	receiver->MaxEnd = 0;
	receiver->MaxWindow = 0;
	receiver->Scale = 0;

	StateContext->Status = TCP_STATUS_SYN_SENT;
	do_gettimeofday(&(StateContext->StateSetTime));
	StateContext->StateTimeOut = tcp_timeouts[TCP_STATUS_SYN_SENT];
	StateContext->LastDir = PACKET_DIR_LOCAL;
	StateContext->RetransCount = 0;
	StateContext->LastControlBits = (unsigned char)index;
	StateContext->LastWindow = sender->MaxWindow;
	StateContext->LastSeq = seq;
	StateContext->LastAck = 0;
	StateContext->LastEnd = sender->End;

	return FILTER_ACCEPT;
}


FILTER_STATUS UpdateTcpStateContext(struct tcphdr *th, __u32 len, PACKET_DIR dir, PTCP_STATE_CONTEXT StateContext, struct sk_buff *skb)
{
	PTCP_STATE_INFO sender = &(StateContext->Seen[dir]);
	PTCP_STATE_INFO receiver = &(StateContext->Seen[!dir]);
	TCP_STATUS  OldStatus = StateContext->Status;
	unsigned int index = get_bits_index(th);
	TCP_STATUS  NewStatus = tcp_state_table[dir][index][OldStatus];
	TCP_STATE_CONTEXT iter;

	switch (NewStatus) {
		case TCP_STATUS_SYN_SENT:
			if (OldStatus < TCP_STATUS_TIME_WAIT) {
				// Retransmitted SYN
				break;
			} 
			else  // Reopened connection from TIME_WAIT or CLOSE state
			{
				/* RFC 1122: "When a connection is closed actively,
				 * it MUST linger in TIME-WAIT state for a time 2xMSL
				 * (Maximum Segment Lifetime). However, it MAY accept
				 * a new SYN from the remote TCP to reopen the connection
				 * directly from TIME-WAIT state, if..."
				 * We ignore the conditions because we are in the
				 * TIME-WAIT state anyway.
				 *
				 * Handle aborted connections: we and the server
				 * think there is an existing connection but the client
				 * aborts it and starts a new one.
				 */
				if (((sender->Options | receiver->Options) & STATE_OPTION_CLOSE_INIT)
					|| (StateContext->LastDir == dir && StateContext->LastControlBits == TCP_RST_SET))
				{
					/* Attempt to reopen a closed/aborted connection. */
					iter = *StateContext;
					
					memset(StateContext, 0, sizeof(TCP_STATE_CONTEXT));
					
					/* Port Mapping list information MUST NOT be dropped */
					StateContext->out_node = iter.out_node;
					StateContext->in_node = iter.in_node;
					StateContext->dest_node = iter.dest_node;
					StateContext->oldaddr = iter.oldaddr;
					StateContext->oldport = iter.oldport;
					StateContext->dstaddr = iter.dstaddr;
					StateContext->dstport = iter.dstport;
					StateContext->newport = iter.newport;
					tcp_list.state_seq = (tcp_list.state_seq >= 2147483647) ? 0 : (tcp_list.state_seq + 1);
					StateContext->state_seq = tcp_list.state_seq;
					
					return CreateTcpStateContext(th, len, StateContext);
				}
			}
			/* Fall through */
		case TCP_STATUS_IGNORE:
			// Ignored packets, just record them in LastXXX fields and do not update state machine.
			//XXX: We do not support connection pick-up at present.
			StateContext->LastDir = dir;
			StateContext->RetransCount = 0;   // Ignored packet is surely not a retransmitted packet.
			StateContext->LastControlBits = (unsigned char)index;
			StateContext->LastWindow = ntohs(th->window);
			StateContext->LastSeq = ntohl(th->seq);
			StateContext->LastAck = ntohl(th->ack_seq);
			StateContext->LastEnd = segment_seq_plus_len(StateContext->LastSeq, len, th);
#if defined(CONFIG_BLOG)
			blog_skip(skb, blog_skip_reason_map_tcp);
#endif
#ifdef IVI_DEBUG_TCP
			printk(KERN_DEBUG "UpdateTcpStateContext: ignore packet on map %d -> %d, state %d\n", 
				StateContext->oldport, StateContext->newport, OldStatus);
#endif
			return FILTER_ACCEPT;

		case TCP_STATUS_MAX:
			// Invalid state, should be released.
#ifdef IVI_DEBUG_TCP
			printk(KERN_ERR "UpdateTcpStateContext: invalid packet on map %d -> %d, state %d, drop packet and clear state.\n", 
				StateContext->oldport, StateContext->newport, OldStatus);
#endif
			return FILTER_DROP_CLEAN;

		case TCP_STATUS_CLOSE:
			// This happens when we are already in CLOSE or received a RST.
			if (index == TCP_RST_SET && (receiver->Options & STATE_OPTION_MAXACK_SET) 
				&& before(ntohl(th->seq), receiver->MaxAck))
			{
				// Invalid RST
#ifdef IVI_DEBUG_TCP
				printk(KERN_ERR "UpdateTcpStateContext: invalid RST packet on map %d -> %d, state %d, drop packet.\n", 
					StateContext->oldport, StateContext->newport, OldStatus);
#endif
				return FILTER_DROP;
			}
			break;

		default:
			break;
	}

#if 0 // liberal mode
	if (tcp_in_window(th, len, dir, StateContext) == false) {
		// Segment is outside the window.
		return FILTER_DROP;
	}
#endif

	// From now on we have got in-window packets.
	StateContext->LastControlBits = (unsigned char)index;
	StateContext->LastDir = dir;
#ifdef IVI_DEBUG_TCP
	printk(KERN_DEBUG "UpdateTcpStateContext: syn=%d ack=%d fin=%d rst=%d old_state=%d new_state=%d\n",
		th->syn, th->ack, th->fin, th->rst, OldStatus, NewStatus);
#endif  
	StateContext->Status = NewStatus;
	if (OldStatus != NewStatus && NewStatus == TCP_STATUS_FIN_WAIT) {
		sender->Options |= STATE_OPTION_CLOSE_INIT;
	}
#if defined(CONFIG_BLOG)
	if (StateContext->Status != sES)
		blog_skip(skb, blog_skip_reason_map_tcp);
#endif

	// Update State Timer.
	if (StateContext->RetransCount >= TcpMaxRetrans && StateContext->StateTimeOut > TcpTimeOutMaxRetrans) {
		StateContext->StateTimeOut = TcpTimeOutMaxRetrans;
	} 
	else if (((sender->Options & receiver->Options) & STATE_OPTION_DATA_UNACK) 
			   && StateContext->StateTimeOut > TcpTimeOutUnack) {
		StateContext->StateTimeOut = TcpTimeOutUnack;
	}
	else {
		StateContext->StateTimeOut = tcp_timeouts[NewStatus];
	}

	// Update state set time.
	do_gettimeofday(&(StateContext->StateSetTime));

	return FILTER_ACCEPT;
}

struct tcp_map_list tcp_list;

void init_tcp_map_list(void)
{
	int i;
	spin_lock_init(&tcp_list.lock);
	for (i = 0; i < IVI_HTABLE_SIZE; i++) {
		INIT_HLIST_HEAD(&tcp_list.out_chain[i]);
		INIT_HLIST_HEAD(&tcp_list.in_chain[i]);
		INIT_HLIST_HEAD(&tcp_list.dest_chain[i]);
	}
	tcp_list.size = 0;
	tcp_list.port_num = 0;
	tcp_list.state_seq = 0;
	tcp_list.last_alloc_port = 0;
}

// Refresh the timer for each map_tuple, must NOT acquire spin lock when calling this function
void refresh_tcp_map_list(int threshold)
{
	PTCP_STATE_CONTEXT iter, i0;
	struct hlist_node *loop;
	struct timeval now;
	time_t delta;
	int i, flag;
	do_gettimeofday(&now);
	
	spin_lock_bh(&tcp_list.lock);
	// Iterate all the map_tuple through out_chain only, in_chain contains the same info.
	for (i = 0; i < IVI_HTABLE_SIZE; i++) {
		hlist_for_each_entry_safe(iter, loop, &tcp_list.out_chain[i], out_node) {
			delta = now.tv_sec - iter->StateSetTime.tv_sec;
			//if (delta >= iter->StateTimeOut || iter->Status == TCP_STATUS_TIME_WAIT || iter->state_seq <= threshold) {
			if (delta >= iter->StateTimeOut) {
#if defined(CONFIG_BCM_KF_BLOG) && defined(CONFIG_BLOG)
	            blog_lock();
                if (iter->blog_key[BLOG_PARAM1_MAP_DIR_US] != BLOG_KEY_FC_INVALID || 
		            iter->blog_key[BLOG_PARAM1_MAP_DIR_DS] != BLOG_KEY_FC_INVALID) {
		            if (blog_query(QUERY_MAP_TUPLE, (void*)iter, 
                            iter->blog_key[BLOG_PARAM1_MAP_DIR_US],
                            iter->blog_key[BLOG_PARAM1_MAP_DIR_DS], 0)) {
	                    blog_unlock();
                        continue;
                    }
                }
                else {
                    // flow cache flow might have disassociated itself from map tuple.
                    if (iter->evict_time.tv_sec) {
			            iter->StateSetTime.tv_sec = iter->evict_time.tv_sec;
			            delta = now.tv_sec - iter->StateSetTime.tv_sec;
			            if (delta < iter->StateTimeOut) {
	                        blog_unlock();
                            continue;
                        }
                    }
                }
	            blog_unlock();
#endif

				hlist_del(&iter->out_node);
				hlist_del(&iter->in_node);
				hlist_del(&iter->dest_node);
				tcp_list.size--;
				
#ifdef IVI_DEBUG_MAP_TCP
				printk(KERN_INFO "refresh_tcp_map_list: time out map " NIP4_FMT ":%d -> %d (dst " NIP4_FMT ":%d) "
				                 "on out_chain[%d], TCP state %d\n", NIP4(iter->oldaddr), iter->oldport, iter->newport, 
				                 NIP4(iter->dstaddr), iter->dstport, i, iter->Status);
				                 
				//if (iter->Status == TCP_STATUS_TIME_WAIT)
				//	printk(KERN_INFO "refresh_tcp_map_list: clean time-wait mappings\n");
 				//else if (iter->state_seq <= threshold)
 				//	printk(KERN_INFO "refresh_tcp_map_list: recycle ports: threshold = %d, state_seq = %d\n", threshold, iter->state_seq);
				//else
				//	printk(KERN_INFO "refresh_tcp_map_list: time out map " NIP4_FMT ":%d -> %d (dst " NIP4_FMT ":%d) on out_chain[%d], TCP state %d\n", 
				//		NIP4(iter->oldaddr), iter->oldport, iter->newport, NIP4(iter->dstaddr), iter->dstport, i, iter->Status);
#endif

				flag = 0; // indicating whether tcp_list.port_num needs to be substracted by 1.
 				hlist_for_each_entry(i0, &tcp_list.in_chain[port_hashfn(iter->newport)], in_node) {
 					if (i0->newport == iter->newport) {
 						flag = 1;
 											
#ifdef IVI_DEBUG_MAP_TCP
 						printk(KERN_INFO "refresh_tcp_map_list: newport %d is still used by someone(" 
 						                 NIP4_FMT ":%d -> " NIP4_FMT ":%d). port_num is still %d\n", 
 						                 iter->newport, NIP4(i0->oldaddr), i0->oldport, 
 						                 NIP4(i0->dstaddr), i0->dstport, tcp_list.port_num);
#endif
 						break;
 					}
 				}
 				if (!flag) {
 					tcp_list.port_num--;
#ifdef IVI_DEBUG_MAP_TCP
 					printk(KERN_INFO "refresh_tcp_map_list: port_num is decreased by 1 to %d(%d)\n", 
 					                 tcp_list.port_num, iter->newport);
#endif
 				}				
#if defined(CONFIG_BCM_KF_BLOG) && defined(CONFIG_BLOG)
	            blog_lock();
                if (iter->blog_key[BLOG_PARAM1_MAP_DIR_US] != BLOG_KEY_FC_INVALID || 
		            iter->blog_key[BLOG_PARAM1_MAP_DIR_DS] != BLOG_KEY_FC_INVALID) {
		            blog_notify(DESTROY_MAP_TUPLE, (void*)iter, 
                            iter->blog_key[BLOG_PARAM1_MAP_DIR_US],
                            iter->blog_key[BLOG_PARAM1_MAP_DIR_DS]);
                }
	            blog_unlock();
#endif
				kfree(iter);
			}
		}
	}
	spin_unlock_bh(&tcp_list.lock);
}

// Clear the entire list, must NOT acquire spin lock when calling this function
void free_tcp_map_list(void)
{
	PTCP_STATE_CONTEXT iter;
	struct hlist_node *loop;
	int i;
	
	spin_lock_bh(&tcp_list.lock);
	// Iterate all the map_tuple through out_chain only, in_chain contains the same info.
	for (i = 0; i < IVI_HTABLE_SIZE; i++) {
		if (!hlist_empty(&tcp_list.out_chain[i])) {
			hlist_for_each_entry_safe(iter, loop, &tcp_list.out_chain[i], out_node) {
				hlist_del(&iter->out_node);
				hlist_del(&iter->in_node);
				hlist_del(&iter->dest_node);
				tcp_list.size--;

				printk(KERN_INFO "free_tcp_map_list: delete map " NIP4_FMT ":%d -> %d (dst " NIP4_FMT ":%d) on out_chain[%d], TCP state %d\n", 
					NIP4(iter->oldaddr), iter->oldport, iter->newport, NIP4(iter->dstaddr), iter->dstport, i, iter->Status);

				kfree(iter);
			}

		}
	}
	tcp_list.port_num = tcp_list.state_seq = 0;
	spin_unlock_bh(&tcp_list.lock);
}

// Check whether a port is in use now, must be protected by spin lock when calling this function
static inline int tcp_port_in_use(__be16 port)
{
	int ret = 0;
	int hash;
	PTCP_STATE_CONTEXT iter;

	hash = port_hashfn(port);
	if (!hlist_empty(&tcp_list.in_chain[hash])) {
		hlist_for_each_entry(iter, &tcp_list.in_chain[hash], in_node) {
			if (iter->newport == port) {
				ret = 1;
				break;
			}
		}
	}

	return ret;
}

// generate a new MAP port, must be protected by spin lock when calling this function
static inline int new_tcp_map_port(u16 ratio, u16 adjacent, u16 offset, int start_port) {
	int retport, rover_j, rover_k, remaining; 
	__be16 low, high;
			
	low = (__u16)((start_port - 1) >> (ratio + adjacent)) + 1;
	high = (__u16)(65536 >> (ratio + adjacent)) - 1;
	remaining = (high - low) + 1;

	if (tcp_list.last_alloc_port != 0) {
		rover_j = tcp_list.last_alloc_port >> (ratio + adjacent);
		rover_k = (tcp_list.last_alloc_port - ((tcp_list.last_alloc_port >> adjacent) << adjacent)) + 1;
		if (rover_k == (1 << adjacent)) {
			rover_j++;
			rover_k = 0;
			if (rover_j > high)
				rover_j = low;
		}
	}	
	else {
		rover_j = low;
		rover_k = 0;
	}
				
	do { 
		retport = (rover_j << (ratio + adjacent)) + (offset << adjacent) + rover_k;
					
		if (!tcp_port_in_use(retport))
			break;
					
		rover_k++;
		if (rover_k == (1 << adjacent)) {
			rover_j++;
			remaining--;
			rover_k = 0;
			if (rover_j > high)
				rover_j = low;
		}
	} while (remaining > 0);
			
	if (remaining <= 0)
		return -1;
	
	return retport;
}

// Create packet state and add mapping info to state list
// MUST NOT acquire spin lock when calling this function
// multiplexflag: 0 -> no multiplex (generate a new unused port)
//                1 -> multiplex
static inline int create_tcp_mapping(u32 oldaddr, u16 oldp, u32 dstaddr, u16 dstp, u16 newport, 
                                     struct tcphdr *th, unsigned int len, int multiplexflag) 
{
	PTCP_STATE_CONTEXT StateContext;
	FILTER_STATUS ftState;
	int hash;
	
	spin_lock_bh(&tcp_list.lock);
	StateContext = (PTCP_STATE_CONTEXT)kmalloc(sizeof(TCP_STATE_CONTEXT), GFP_ATOMIC);
	if (StateContext == NULL) // No memory for state info. Fail this map.
	{	
		spin_unlock_bh(&tcp_list.lock);
		printk(KERN_ERR "create_tcp_mapping: kmalloc failed.\n");
		return -1;
	}
	memset(StateContext, 0, sizeof(TCP_STATE_CONTEXT));
	
	// Check packet state for new mapping.
	ftState = CreateTcpStateContext(th, len, StateContext);

	if (ftState == FILTER_DROP_CLEAN) {
#ifdef IVI_DEBUG_MAP_TCP
		printk(KERN_ERR "create_tcp_mapping: Invalid state on " NIP4_FMT ":%d -> " NIP4_FMT 
		                ":%d, TCP state %d, fail to add new map.\n", NIP4(oldaddr), oldp, 
		                NIP4(dstaddr), dstp, StateContext->Status);
#endif
		kfree(StateContext);			
		spin_unlock_bh(&tcp_list.lock);
		return -1;
	}

	// Routine to add new map-info
	StateContext->oldaddr = oldaddr;
	StateContext->oldport = oldp;
	StateContext->dstaddr = dstaddr;
	StateContext->dstport = dstp;
	StateContext->newport = newport;
	hash = v4addr_port_hashfn(oldaddr, oldp);
	hlist_add_head(&StateContext->out_node, &tcp_list.out_chain[hash]);
	hash = port_hashfn(newport);
	hlist_add_head(&StateContext->in_node, &tcp_list.in_chain[hash]);
	hash = v4addr_port_hashfn(dstaddr, dstp);
	hlist_add_head(&StateContext->dest_node, &tcp_list.dest_chain[hash]);
	
	tcp_list.size++;
	tcp_list.state_seq = (tcp_list.state_seq >= 2147483647) ? 0 : (tcp_list.state_seq + 1);	
	if (!multiplexflag) {
		tcp_list.port_num++;
		tcp_list.last_alloc_port = newport;
	}
	
	StateContext->state_seq = tcp_list.state_seq;
	
#ifdef IVI_DEBUG_MAP_TCP
	printk(KERN_INFO "create_tcp_mapping: Add new mapping (" NIP4_FMT \
		             ":%d -> " NIP4_FMT ":%d -------> %d), list_len = %d, port_num = %d\n", \
	                 NIP4(oldaddr), oldp, NIP4(dstaddr), dstp, newport, \
	                 tcp_list.size, tcp_list.port_num);
#endif
				   
	spin_unlock_bh(&tcp_list.lock);
	return 0;
}

// Check tcp_list if any port can be multiplexed with diffent destination (addr, port) pairs
// must be protected by spin lock when calling this function
static inline int tcp_dest_multiplex_port(u32 dstaddr, u16 dstp)
{
	int status, chance, i, rand_j, dsthash, hash, retport;
	PTCP_STATE_CONTEXT iter, multiplex_state;
	
	status = 0;
	chance = TCP_MAX_LOOP_NUM;
			
	dsthash = v4addr_port_hashfn(dstaddr, dstp);		
	while (1) { // generate an integer between [1, 31]
		get_random_bytes(&rand_j, sizeof(int));
		rand_j = (rand_j >= 0) ? rand_j : -rand_j;
		rand_j -= (rand_j >> 5) << 5;
		if (rand_j) break;
	}

	/* hash is a random number between [0,31] except dsthash, so MAYBE its newport can be multiplexed 
	   because dest_chain[hash] is impossible to have the same destination with this packet.*/
	hash = (dsthash + rand_j >= 32) ? (dsthash + rand_j - 32) : (dsthash + rand_j); 
		
	for (i = 0; i < 31 && chance > 0; i++) {	
		if (!hlist_empty(&tcp_list.dest_chain[hash])) {
			hlist_for_each_entry(multiplex_state, &tcp_list.dest_chain[hash], dest_node) {
				retport = multiplex_state->newport;
				status = 1;
					
				/* don't worry:) we have to check whether this port has been multiplexed by another 
				   connection with the same destination */
				if (!hlist_empty(&tcp_list.dest_chain[dsthash])) {
					hlist_for_each_entry(iter, &tcp_list.dest_chain[dsthash], dest_node) {
						if (iter->dstaddr == dstaddr && iter->dstport == dstp && iter->newport == retport) {
							status = 0; // this port cannot be multiplexed
							break;
						}
					}
				}
				if (status == 1) { // this port can be multiplexed
				
#ifdef IVI_DEBUG_MAP_TCP	
					printk(KERN_INFO "tcp_dest_multiplex_port: multiplex port %d on dest_chain[%d], "
					                 "round %d\n", retport, hash, i + 1);
#endif

					return retport;
				}
			}
				
			if (status == 0) {
				//printk(KERN_DEBUG "ooops, you have only %d chance left now~\n", chance);
				chance--;
			}
		}
		else {
			if (++hash >= 32)      
				hash = 0;
			if (hash == dsthash) {
				if (++hash >= 32)
					hash = 0;
			}
		}
	}
	
	return 0;
}

int get_outflow_tcp_map_port(__be32 oldaddr, __be16 oldp, __be32 dstaddr, __be16 dstp, u16 ratio, 
                             u16 adjacent, u16 offset, struct tcphdr *th, __u32 len, __be16 *newp, struct sk_buff *skb)
{	
	int hash, reusing, status, flag, start_port;
	__be16 retport;
	PTCP_STATE_CONTEXT StateContext;
	PTCP_STATE_CONTEXT i0;
	struct hlist_node *loop;
	FILTER_STATUS ftState;
		
	retport = 0;
	*newp = 0;
	reusing = 0;
	status = 0;
	ratio = fls(ratio) - 1;
	adjacent = fls(adjacent) - 1;
	start_port = ((1 << (ratio + adjacent)) > 1024) ? 1 << (ratio + adjacent) : 1024; // the ports below start_port are reserved for system ports.
	
	refresh_tcp_map_list(0);
	spin_lock_bh(&tcp_list.lock);

	hash = v4addr_port_hashfn(oldaddr, oldp);
	if (!hlist_empty(&tcp_list.out_chain[hash])) {
		hlist_for_each_entry_safe(StateContext, loop, &tcp_list.out_chain[hash], out_node) {
			if (StateContext->oldport == oldp && StateContext->oldaddr == oldaddr) {
				if (StateContext->dstaddr == dstaddr && StateContext->dstport == dstp) {
					// Update state context.
					ftState = UpdateTcpStateContext(th, len, PACKET_DIR_LOCAL, StateContext, skb);
			
					if (ftState == FILTER_ACCEPT) {
						retport = StateContext->newport;
						StateContext->state_seq = tcp_list.state_seq;
						
#ifdef IVI_DEBUG_MAP_TCP
						//printk(KERN_INFO "get_outflow_tcp_map_port: Found map " NIP4_FMT ":%d -> " 
						//                 NIP4_FMT ":%d ------> %d on out_chain[%d], TCP state %d\n", 
						//                 NIP4(oldaddr), oldp, NIP4(dstaddr), dstp, retport, 
						//                 hash, StateContext->Status);
#endif
					}
					else if (ftState == FILTER_DROP) {
						// Return -1 to drop current segment, keep the state info.
#ifdef IVI_DEBUG_MAP_TCP
						printk(KERN_ERR "get_outflow_tcp_map_port: drop packet on map " NIP4_FMT ":%d -> " 
						                NIP4_FMT ":%d ------> %d on out_chain[%d], TCP state %d\n", 
						                NIP4(oldaddr), oldp, NIP4(dstaddr), dstp, StateContext->newport, 
						                hash, StateContext->Status);
#endif
					}
					else  // FILTER_DROP_CLEAN                         
					{
						// Remove state info, return -1
						hlist_del(&StateContext->out_node);
						hlist_del(&StateContext->in_node);
						hlist_del(&StateContext->dest_node);
						tcp_list.size--;
						flag = 0; // indicating whether tcp_list.port_num needs to be substracted by 1.

						hlist_for_each_entry(i0, &tcp_list.in_chain[port_hashfn(StateContext->newport)], in_node) {
							if (i0->newport == StateContext->newport) {
								flag = 1;
								
#ifdef IVI_DEBUG_MAP_TCP
								printk(KERN_INFO "get_outflow_tcp_map_port: newport %d is still used by someone(" 
								                 NIP4_FMT ":%d -> " NIP4_FMT ":%d). port_num is still %d\n", 
								                 StateContext->newport, NIP4(i0->oldaddr), i0->oldport, 
								                 NIP4(i0->dstaddr), i0->dstport, tcp_list.port_num);
#endif
								break;
							}
						}
						if (!flag) {
							tcp_list.port_num--;
#ifdef IVI_DEBUG_MAP_TCP
							printk(KERN_INFO "get_outflow_tcp_map_port: port_num is decreased by 1 to %d(%d)\n", 
							                 tcp_list.port_num, StateContext->newport);
#endif
						}
						
                    	kfree(StateContext);
                    	
#ifdef IVI_DEBUG_MAP_TCP
						printk(KERN_ERR "get_outflow_tcp_map_port: clean state on map " NIP4_FMT ":%d -> " NIP4_FMT 
						                ":%d ------> %d on out_chain[%d], TCP state %d\n", NIP4(oldaddr), oldp, 
						                NIP4(dstaddr), dstp, retport, hash, StateContext->Status);
#endif
               		}
               		*newp = retport;

#if defined(CONFIG_BCM_KF_BLOG) && defined(CONFIG_BLOG)
                    blog_link(MAP_TUPLE, blog_ptr(skb), (void*)StateContext, BLOG_PARAM1_MAP_DIR_US, 0);
#endif
					spin_unlock_bh(&tcp_list.lock);
					return (retport == 0 ? -1 : 0);
               	}
               	
               	else if (reusing == 0) {
               		// src addr&port same, while dest addr&port different: reuse the mapped port (Endpoint-independent)
               		retport = StateContext->newport;
               		reusing = 1;
#ifdef IVI_DEBUG_MAP_TCP
					printk(KERN_INFO "get_outflow_tcp_map_port: port %d can be multiplexed with source address " 
					                 NIP4_FMT ":%d\n", retport, NIP4(oldaddr), oldp);
#endif
				}
            }
		}
	}
	
	if (reusing == 1 && retport > 0) {
		spin_unlock_bh(&tcp_list.lock);
		if (create_tcp_mapping(oldaddr, oldp, dstaddr, dstp, retport, th, len, 1) < 0) {
#ifdef IVI_DEBUG_MAP_TCP
			printk(KERN_ERR "get_outflow_tcp_map_port: create_tcp_mapping when multiplexing1 failed.\n");
#endif
			return -1;
		}
		*newp = retport;
		return 0;
	}

	else // No existing map
	{
		// Now we have to find a mapping whose src & dest are both different to multiplex:
		retport = tcp_dest_multiplex_port(dstaddr, dstp);
		if (retport > 0) { // multiplex port found
			spin_unlock_bh(&tcp_list.lock);
			if (create_tcp_mapping(oldaddr, oldp, dstaddr, dstp, retport, th, len, 1) < 0) {
#ifdef IVI_DEBUG_MAP_TCP
				printk(KERN_ERR "get_outflow_tcp_map_port: create_tcp_mapping when multiplexing2 failed\n");
#endif
				return -1;
			}
			*newp = retport;
			return 0;
		}
		else {
			// If it's so lucky to reach here, we have to generate a new port
			if (tcp_list.port_num >= ((65536 - start_port)>>ratio)) {
				spin_unlock_bh(&tcp_list.lock);
				printk(KERN_ERR "get_outflow_tcp map_port: tcp map list full, port_num = %d\n", tcp_list.port_num);
				return -1;
				
				/*spin_unlock_bh(&tcp_list.lock);
				refresh_tcp_map_list(tcp_list.state_seq - (((65536 - 4096) >> ratio)) - 1); // the port whose state_seq is less or equal to this threshold will be recycled
				spin_lock_bh(&tcp_list.lock);
				if (tcp_list.port_num >= ((65536 - 4096) >> ratio)) {
					spin_unlock_bh(&tcp_list.lock);
					printk(KERN_ERR "get_outflow_tcp map_port: map list is rather full.\n");
					return -1;
				}*/
			}
					
			if (ratio == 0)
				retport = oldp; // In 1:1 mapping mode, use old port directly.
			
			else if ((retport = new_tcp_map_port(ratio, adjacent, offset, start_port)) < 0) {
				spin_unlock_bh(&tcp_list.lock);
				printk(KERN_ERR "get_outflow_tcp_map_port: failed to assign a new map port.\n");
				return -1;
			}
			
			spin_unlock_bh(&tcp_list.lock);
			if (create_tcp_mapping(oldaddr, oldp, dstaddr, dstp, retport, th, len, 0) < 0) {
#ifdef IVI_DEBUG_MAP_TCP
				printk(KERN_ERR "get_outflow_tcp_map_port: create_tcp_mapping failed.\n");
#endif
				return -1;
			}
			*newp = retport;
			return 0;
		}
	}
}

int get_inflow_tcp_map_port(__be16 newp, __be32 dstaddr,  __be16 dstp, struct tcphdr *th, __u32 len, __be32 *oldaddr, __be16 *oldp, struct sk_buff *skb)
{
	FILTER_STATUS ftState;
	PTCP_STATE_CONTEXT  StateContext = NULL, i0;
	struct hlist_node  *loop;
	int ret, hash, flag;
	
	refresh_tcp_map_list(0);
	spin_lock_bh(&tcp_list.lock);
	ret = 1;
	*oldp = 0;
	*oldaddr = 0;
	
	hash = port_hashfn(newp);
	hlist_for_each_entry_safe(StateContext, loop, &tcp_list.in_chain[hash], in_node) {
		// Found existing mapping info
		if (StateContext->newport == newp && StateContext->dstaddr == dstaddr && StateContext->dstport == dstp)
		{
			*oldaddr = StateContext->oldaddr;
			*oldp = StateContext->oldport;
			
			// Update state context.
			ftState = UpdateTcpStateContext(th, len, PACKET_DIR_REMOTE, StateContext, skb);

			if (ftState == FILTER_ACCEPT) {
				ret = 0;

#if defined(CONFIG_BCM_KF_BLOG) && defined(CONFIG_BLOG)
                blog_link(MAP_TUPLE, blog_ptr(skb), (void*)StateContext, BLOG_PARAM1_MAP_DIR_DS, 0);
#endif
#ifdef IVI_DEBUG_MAP_TCP
				printk(KERN_INFO "get_inflow_tcp_map_port: Found map " NIP4_FMT ":%d -> " NIP4_FMT ":%d -----> %d "
				                 "on in_chain[%d], TCP state %d\n", NIP4(*oldaddr), *oldp, NIP4(dstaddr), dstp, newp, 
				                 hash, StateContext->Status);
#endif

			}
			else if (ftState == FILTER_DROP) { 
				ret = -1; // FILTER_DROP: drop current segment, keep the state info.
				
#ifdef IVI_DEBUG_MAP_TCP
				printk(KERN_INFO "get_inflow_tcp_map_port: Invalid packet on map " NIP4_FMT ":%d -> " 
				                 NIP4_FMT ":%d -----> %d on in_chain[%d], TCP state %d\n", 
				                 NIP4(StateContext->oldaddr), StateContext->oldport, NIP4(dstaddr), dstp, newp, 
				                 hash, StateContext->Status);
#endif

			}
			else  // FILTER_DROP_CLEAN: drop current segment, and clean the state info
			{
				// Remove state info, return -1
				hlist_del(&StateContext->out_node);
				hlist_del(&StateContext->in_node);
				hlist_del(&StateContext->dest_node);
				tcp_list.size--;
				
				flag = 0; // indicating whether tcp_list.port_num needs to be substracted by 1.
 				hlist_for_each_entry(i0, &tcp_list.in_chain[port_hashfn(StateContext->newport)], in_node) {
 					if (i0->newport == StateContext->newport) {
 						flag = 1;
 						
#ifdef IVI_DEBUG_MAP_TCP
 						printk(KERN_INFO "get_inflow_tcp_map_port: newport %d is still used by someone(" NIP4_FMT ":%d -> " 
 						                 NIP4_FMT ":%d). port_num is still %d\n", StateContext->newport, 
 						                 NIP4(i0->oldaddr), i0->oldport, NIP4(i0->dstaddr), i0->dstport, tcp_list.port_num);
#endif
 						break;
 					}
 				}
 				if (!flag) {
 					tcp_list.port_num--;
 					
#ifdef IVI_DEBUG_MAP_TCP
 					printk(KERN_INFO "get_inflow_tcp_map_port: port_num is decreased by 1 to %d\n", tcp_list.port_num);
#endif

 				}
				kfree(StateContext);
				ret = -1;
				
#ifdef IVI_DEBUG_MAP_TCP
				printk(KERN_ERR "get_inflow_tcp_map_port: clean state on map " NIP4_FMT ":%d -> " NIP4_FMT 
				                ":%d -----> %d on in_chain[%d], TCP state %d\n", NIP4(StateContext->oldaddr), 
				                StateContext->oldport, NIP4(dstaddr), dstp, newp, hash, StateContext->Status);
#endif
			}

			break;
		}
	}
	
	if (ret == 1) {	// fail to find a mapping either in tcp_list.
		u32 idx;
#ifdef IVI_DEBUG_MAP_TCP
		printk(KERN_INFO "get_inflow_tcp_map_port: in_chain[%d] empty.\n", hash);
#endif
		idx = mapportmap_lookup(oldaddr, dstaddr, newp, (1<<MAPPORTMAP_PROTO_TCP), MAPPORTMAP_MODE_FIND);
		if (idx != MAPPORTMAP_IX_INVALID) {

			spin_unlock_bh(&tcp_list.lock);
			if (create_tcp_mapping(*oldaddr, *oldp, dstaddr, dstp, newp, th, len, 0) < 0) {
#ifdef IVI_DEBUG_MAP_TCP
				printk(KERN_ERR "fail add new map for portmap TCP case\n");
#endif
				return -1;
			}
			*oldp = newp;

			goto out;
		}
		else
			ret = -1;
	}

	spin_unlock_bh(&tcp_list.lock);
out:
	return ret;
}


int ivi_map_tcp_init(void) {
	init_tcp_map_list();
#ifdef IVI_DEBUG
	printk(KERN_DEBUG "IVI: ivi_map_tcp loaded.\n");
#endif 
	return 0;
}

void ivi_map_tcp_exit(void) {
	free_tcp_map_list();
#ifdef IVI_DEBUG
	printk(KERN_DEBUG "IVI: ivi_map_tcp unloaded.\n");
#endif
}
