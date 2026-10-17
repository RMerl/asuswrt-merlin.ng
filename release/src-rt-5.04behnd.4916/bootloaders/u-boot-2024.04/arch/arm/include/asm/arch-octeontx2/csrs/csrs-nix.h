/* SPDX-License-Identifier:    GPL-2.0
 *
 * Copyright (C) 2020 Marvell International Ltd.
 *
 * https://spdx.org/licenses
 */
#ifndef __CSRS_NIX_H__
#define __CSRS_NIX_H__

/**
 * @file
 *
 * Configuration and status register (CSR) address and type definitions for
 * NIX.
 *
 * This file is auto generated.  Do not edit.
 *
 */

/**
 * Enumeration nix_af_int_vec_e
 *
 * NIX Admin Function Interrupt Vector Enumeration Enumerates the NIX AF
 * MSI-X interrupt vectors.
 */
#define NIX_AF_INT_VEC_E_AF_ERR (3)
#define NIX_AF_INT_VEC_E_AQ_DONE (2)
#define NIX_AF_INT_VEC_E_GEN (1)
#define NIX_AF_INT_VEC_E_POISON (4)
#define NIX_AF_INT_VEC_E_RVU (0)

/**
 * Enumeration nix_aq_comp_e
 *
 * NIX Completion Enumeration Enumerates the values of
 * NIX_AQ_RES_S[COMPCODE].
 */
#define NIX_AQ_COMP_E_CTX_FAULT (4)
#define NIX_AQ_COMP_E_CTX_POISON (3)
#define NIX_AQ_COMP_E_GOOD (1)
#define NIX_AQ_COMP_E_LOCKERR (5)
#define NIX_AQ_COMP_E_NOTDONE (0)
#define NIX_AQ_COMP_E_SQB_ALLOC_FAIL (6)
#define NIX_AQ_COMP_E_SWERR (2)

/**
 * Enumeration nix_aq_ctype_e
 *
 * NIX Context Type Enumeration Enumerates NIX_AQ_INST_S[CTYPE] values.
 */
#define NIX_AQ_CTYPE_E_CQ (2)
#define NIX_AQ_CTYPE_E_DYNO (5)
#define NIX_AQ_CTYPE_E_MCE (3)
#define NIX_AQ_CTYPE_E_RQ (0)
#define NIX_AQ_CTYPE_E_RSS (4)
#define NIX_AQ_CTYPE_E_SQ (1)

/**
 * Enumeration nix_aq_instop_e
 *
 * NIX Admin Queue Opcode Enumeration Enumerates NIX_AQ_INST_S[OP]
 * values.
 */
#define NIX_AQ_INSTOP_E_INIT (1)
#define NIX_AQ_INSTOP_E_LOCK (4)
#define NIX_AQ_INSTOP_E_NOP (0)
#define NIX_AQ_INSTOP_E_READ (3)
#define NIX_AQ_INSTOP_E_UNLOCK (5)
#define NIX_AQ_INSTOP_E_WRITE (2)

/**
 * Enumeration nix_chan_e
 *
 * NIX Channel Number Enumeration Enumerates the receive and transmit
 * channels, and values of NIX_RX_PARSE_S[CHAN],
 * NIX_SQ_CTX_S[DEFAULT_CHAN]. CNXXXX implements a subset of these
 * channels. Specifically, only channels for links enumerated by
 * NIX_LINK_E are implemented.  Internal: P2X/X2P channel enumeration for
 * t9x.
 */
#define NIX_CHAN_E_CGXX_LMACX_CHX(a, b, c)	\
	(0x800 + 0x100 * (a) + 0x10 * (b) + (c))
#define NIX_CHAN_E_LBKX_CHX(a, b) (0 + 0x100 * (a) + (b))
#define NIX_CHAN_E_RX(a) (0 + 0x100 * (a))
#define NIX_CHAN_E_SDP_CHX(a) (0x700 + (a))

/**
 * Enumeration nix_colorresult_e
 *
 * NIX Color Result Enumeration Enumerates the values of
 * NIX_MEM_RESULT_S[COLOR], NIX_AF_TL1()_MD_DEBUG1[COLOR] and
 * NIX_AF_TL1()_MD_DEBUG1[COLOR].
 */
#define NIX_COLORRESULT_E_GREEN (0)
#define NIX_COLORRESULT_E_RED_DROP (3)
#define NIX_COLORRESULT_E_RED_SEND (2)
#define NIX_COLORRESULT_E_YELLOW (1)

/**
 * Enumeration nix_cqerrint_e
 *
 * NIX Completion Queue Interrupt Enumeration Enumerates the bit index of
 * NIX_CQ_CTX_S[CQ_ERR_INT,CQ_ERR_INT_ENA].
 */
#define NIX_CQERRINT_E_CQE_FAULT (2)
#define NIX_CQERRINT_E_DOOR_ERR (0)
#define NIX_CQERRINT_E_WR_FULL (1)

/**
 * Enumeration nix_intf_e
 *
 * NIX Interface Number Enumeration Enumerates the bit index of
 * NIX_AF_STATUS[CALIBRATE_STATUS].
 */
#define NIX_INTF_E_CGXX(a) (0 + (a))
#define NIX_INTF_E_LBKX(a) (3 + (a))
#define NIX_INTF_E_SDP (4)

/**
 * Enumeration nix_lf_int_vec_e
 *
 * NIX Local Function Interrupt Vector Enumeration Enumerates the NIX
 * MSI-X interrupt vectors per LF.
 */
#define NIX_LF_INT_VEC_E_CINTX(a) (0x40 + (a))
#define NIX_LF_INT_VEC_E_ERR_INT (0x81)
#define NIX_LF_INT_VEC_E_GINT (0x80)
#define NIX_LF_INT_VEC_E_POISON (0x82)
#define NIX_LF_INT_VEC_E_QINTX(a) (0 + (a))

/**
 * Enumeration nix_link_e
 *
 * NIX Link Number Enumeration Enumerates the receive and transmit links,
 * and LINK index of NIX_AF_RX_LINK()_CFG, NIX_AF_RX_LINK()_WRR_CFG,
 * NIX_AF_TX_LINK()_NORM_CREDIT, NIX_AF_TX_LINK()_HW_XOFF and
 * NIX_AF_TL3_TL2()_LINK()_CFG.
 */
#define NIX_LINK_E_CGXX_LMACX(a, b) (0 + 4 * (a) + (b))
#define NIX_LINK_E_LBKX(a) (0xc + (a))
#define NIX_LINK_E_MC (0xe)
#define NIX_LINK_E_SDP (0xd)

/**
 * Enumeration nix_lsoalg_e
 *
 * NIX Large Send Offload Algorithm Enumeration Enumerates
 * NIX_AF_LSO_FORMAT()_FIELD()[ALG] values. Specifies algorithm for
 * modifying the associated LSO packet field.
 */
#define NIX_LSOALG_E_ADD_OFFSET (3)
#define NIX_LSOALG_E_ADD_PAYLEN (2)
#define NIX_LSOALG_E_ADD_SEGNUM (1)
#define NIX_LSOALG_E_NOP (0)
#define NIX_LSOALG_E_TCP_FLAGS (4)

/**
 * Enumeration nix_maxsqesz_e
 *
 * NIX Maximum SQE Size Enumeration Enumerates the values of
 * NIX_SQ_CTX_S[MAX_SQE_SIZE].
 */
#define NIX_MAXSQESZ_E_W16 (0)
#define NIX_MAXSQESZ_E_W8 (1)

/**
 * Enumeration nix_mdtype_e
 *
 * NIX Meta Descriptor Type Enumeration Enumerates values of
 * NIX_AF_MDQ()_MD_DEBUG[MD_TYPE].
 */
#define NIX_MDTYPE_E_FLUSH (1)
#define NIX_MDTYPE_E_PMD (2)
#define NIX_MDTYPE_E_RSVD (0)

/**
 * Enumeration nix_mnqerr_e
 *
 * NIX Meta-Descriptor Enqueue Error Enumeration Enumerates
 * NIX_LF_MNQ_ERR_DBG[ERRCODE] values.
 */
#define NIX_MNQERR_E_CQ_QUERY_ERR (6)
#define NIX_MNQERR_E_LSO_ERR (5)
#define NIX_MNQERR_E_MAXLEN_ERR (8)
#define NIX_MNQERR_E_MAX_SQE_SIZE_ERR (7)
#define NIX_MNQERR_E_SQB_FAULT (2)
#define NIX_MNQERR_E_SQB_POISON (3)
#define NIX_MNQERR_E_SQE_SIZEM1_ZERO (9)
#define NIX_MNQERR_E_SQ_CTX_FAULT (0)
#define NIX_MNQERR_E_SQ_CTX_POISON (1)
#define NIX_MNQERR_E_TOTAL_ERR (4)

/**
 * Enumeration nix_ndc_rx_port_e
 *
 * NIX Receive NDC Port Enumeration Enumerates NIX receive NDC
 * (NDC_IDX_E::NIX()_RX) ports and the PORT index of
 * NDC_AF_PORT()_RT()_RW()_REQ_PC and NDC_AF_PORT()_RT()_RW()_LAT_PC.
 */
#define NIX_NDC_RX_PORT_E_AQ (0)
#define NIX_NDC_RX_PORT_E_CINT (2)
#define NIX_NDC_RX_PORT_E_CQ (1)
#define NIX_NDC_RX_PORT_E_MC (3)
#define NIX_NDC_RX_PORT_E_PKT (4)
#define NIX_NDC_RX_PORT_E_RQ (5)

/**
 * Enumeration nix_ndc_tx_port_e
 *
 * NIX Transmit NDC Port Enumeration Enumerates NIX transmit NDC
 * (NDC_IDX_E::NIX()_TX) ports and the PORT index of
 * NDC_AF_PORT()_RT()_RW()_REQ_PC and NDC_AF_PORT()_RT()_RW()_LAT_PC.
 */
#define NIX_NDC_TX_PORT_E_DEQ (3)
#define NIX_NDC_TX_PORT_E_DMA (4)
#define NIX_NDC_TX_PORT_E_ENQ (1)
#define NIX_NDC_TX_PORT_E_LMT (0)
#define NIX_NDC_TX_PORT_E_MNQ (2)
#define NIX_NDC_TX_PORT_E_XQE (5)

/**
 * Enumeration nix_re_opcode_e
 *
 * NIX Receive Error Opcode Enumeration Enumerates
 * NIX_RX_PARSE_S[ERRCODE] values when NIX_RX_PARSE_S[ERRLEV] =
 * NPC_ERRLEV_E::RE.
 */
#define NIX_RE_OPCODE_E_OL2_LENMISM (0x12)
#define NIX_RE_OPCODE_E_OVERSIZE (0x11)
#define NIX_RE_OPCODE_E_RE_DMAPKT (0xf)
#define NIX_RE_OPCODE_E_RE_FCS (7)
#define NIX_RE_OPCODE_E_RE_FCS_RCV (8)
#define NIX_RE_OPCODE_E_RE_JABBER (2)
#define NIX_RE_OPCODE_E_RE_NONE (0)
#define NIX_RE_OPCODE_E_RE_PARTIAL (1)
#define NIX_RE_OPCODE_E_RE_RX_CTL (0xb)
#define NIX_RE_OPCODE_E_RE_SKIP (0xc)
#define NIX_RE_OPCODE_E_RE_TERMINATE (9)
#define NIX_RE_OPCODE_E_UNDERSIZE (0x10)

/**
 * Enumeration nix_redalg_e
 *
 * NIX Red Algorithm Enumeration Enumerates the different algorithms of
 * NIX_SEND_EXT_S[SHP_RA].
 */
#define NIX_REDALG_E_DISCARD (3)
#define NIX_REDALG_E_SEND (1)
#define NIX_REDALG_E_STALL (2)
#define NIX_REDALG_E_STD (0)

/**
 * Enumeration nix_rqint_e
 *
 * NIX Receive Queue Interrupt Enumeration Enumerates the bit index of
 * NIX_RQ_CTX_S[RQ_INT,RQ_INT_ENA].
 */
#define NIX_RQINT_E_DROP (0)
#define NIX_RQINT_E_RX(a) (0 + (a))
#define NIX_RQINT_E_RED (1)

/**
 * Enumeration nix_rx_actionop_e
 *
 * NIX Receive Action Opcode Enumeration Enumerates the values of
 * NIX_RX_ACTION_S[OP].
 */
#define NIX_RX_ACTIONOP_E_DROP (0)
#define NIX_RX_ACTIONOP_E_MCAST (3)
#define NIX_RX_ACTIONOP_E_MIRROR (6)
#define NIX_RX_ACTIONOP_E_PF_FUNC_DROP (5)
#define NIX_RX_ACTIONOP_E_RSS (4)
#define NIX_RX_ACTIONOP_E_UCAST (1)
#define NIX_RX_ACTIONOP_E_UCAST_IPSEC (2)

/**
 * Enumeration nix_rx_mcop_e
 *
 * NIX Receive Multicast/Mirror Opcode Enumeration Enumerates the values
 * of NIX_RX_MCE_S[OP].
 */
#define NIX_RX_MCOP_E_RQ (0)
#define NIX_RX_MCOP_E_RSS (1)

/**
 * Enumeration nix_rx_perrcode_e
 *
 * NIX Receive Protocol Error Code Enumeration Enumerates
 * NIX_RX_PARSE_S[ERRCODE] values when NIX_RX_PARSE_S[ERRLEV] =
 * NPC_ERRLEV_E::NIX.
 */
#define NIX_RX_PERRCODE_E_BUFS_OFLOW (0xa)
#define NIX_RX_PERRCODE_E_DATA_FAULT (8)
#define NIX_RX_PERRCODE_E_IL3_LEN (0x20)
#define NIX_RX_PERRCODE_E_IL4_CHK (0x22)
#define NIX_RX_PERRCODE_E_IL4_LEN (0x21)
#define NIX_RX_PERRCODE_E_IL4_PORT (0x23)
#define NIX_RX_PERRCODE_E_MCAST_FAULT (4)
#define NIX_RX_PERRCODE_E_MCAST_POISON (6)
#define NIX_RX_PERRCODE_E_MEMOUT (9)
#define NIX_RX_PERRCODE_E_MIRROR_FAULT (5)
#define NIX_RX_PERRCODE_E_MIRROR_POISON (7)
#define NIX_RX_PERRCODE_E_NPC_RESULT_ERR (2)
#define NIX_RX_PERRCODE_E_OL3_LEN (0x10)
#define NIX_RX_PERRCODE_E_OL4_CHK (0x12)
#define NIX_RX_PERRCODE_E_OL4_LEN (0x11)
#define NIX_RX_PERRCODE_E_OL4_PORT (0x13)

/**
 * Enumeration nix_send_status_e
 *
 * NIX Send Completion Status Enumeration Enumerates values of
 * NIX_SEND_COMP_S[STATUS] and NIX_LF_SEND_ERR_DBG[ERRCODE].
 */
#define NIX_SEND_STATUS_E_DATA_FAULT (0x16)
#define NIX_SEND_STATUS_E_DATA_POISON (0x17)
#define NIX_SEND_STATUS_E_GOOD (0)
#define NIX_SEND_STATUS_E_INVALID_SUBDC (0x14)
#define NIX_SEND_STATUS_E_JUMP_FAULT (7)
#define NIX_SEND_STATUS_E_JUMP_POISON (8)
#define NIX_SEND_STATUS_E_LOCK_VIOL (0x21)
#define NIX_SEND_STATUS_E_NPC_DROP_ACTION (0x20)
#define NIX_SEND_STATUS_E_NPC_MCAST_ABORT (0x24)
#define NIX_SEND_STATUS_E_NPC_MCAST_CHAN_ERR (0x23)
#define NIX_SEND_STATUS_E_NPC_UCAST_CHAN_ERR (0x22)
#define NIX_SEND_STATUS_E_NPC_VTAG_PTR_ERR (0x25)
#define NIX_SEND_STATUS_E_NPC_VTAG_SIZE_ERR (0x26)
#define NIX_SEND_STATUS_E_SEND_CRC_ERR (0x10)
#define NIX_SEND_STATUS_E_SEND_EXT_ERR (6)
#define NIX_SEND_STATUS_E_SEND_HDR_ERR (5)
#define NIX_SEND_STATUS_E_SEND_IMM_ERR (0x11)
#define NIX_SEND_STATUS_E_SEND_MEM_ERR (0x13)
#define NIX_SEND_STATUS_E_SEND_MEM_FAULT (0x27)
#define NIX_SEND_STATUS_E_SEND_SG_ERR (0x12)
#define NIX_SEND_STATUS_E_SQB_FAULT (3)
#define NIX_SEND_STATUS_E_SQB_POISON (4)
#define NIX_SEND_STATUS_E_SQ_CTX_FAULT (1)
#define NIX_SEND_STATUS_E_SQ_CTX_POISON (2)
#define NIX_SEND_STATUS_E_SUBDC_ORDER_ERR (0x15)

/**
 * Enumeration nix_sendcrcalg_e
 *
 * NIX Send CRC Algorithm Enumeration Enumerates the CRC algorithm used,
 * see NIX_SEND_CRC_S[ALG].
 */
#define NIX_SENDCRCALG_E_CRC32 (0)
#define NIX_SENDCRCALG_E_CRC32C (1)
#define NIX_SENDCRCALG_E_ONES16 (2)

/**
 * Enumeration nix_sendl3type_e
 *
 * NIX Send Layer 3 Header Type Enumeration Enumerates values of
 * NIX_SEND_HDR_S[OL3TYPE], NIX_SEND_HDR_S[IL3TYPE]. Internal: Encoding
 * matches DPDK TX IP types: \<pre\> PKT_TX_IP_CKSUM      (1ULL \<\< 54)
 * PKT_TX_IPV4          (1ULL \<\< 55) PKT_TX_IPV6          (1ULL \<\<
 * 56)  PKT_TX_OUTER_IP_CKSUM(1ULL \<\< 58) PKT_TX_OUTER_IPV4    (1ULL
 * \<\< 59) PKT_TX_OUTER_IPV6    (1ULL \<\< 60) \</pre\>
 */
#define NIX_SENDL3TYPE_E_IP4 (2)
#define NIX_SENDL3TYPE_E_IP4_CKSUM (3)
#define NIX_SENDL3TYPE_E_IP6 (4)
#define NIX_SENDL3TYPE_E_NONE (0)

/**
 * Enumeration nix_sendl4type_e
 *
 * NIX Send Layer 4 Header Type Enumeration Enumerates values of
 * NIX_SEND_HDR_S[OL4TYPE], NIX_SEND_HDR_S[IL4TYPE]. Internal: Encoding
 * matches DPDK TX L4 types. \<pre\> PKT_TX_L4_NO_CKSUM   (0ULL \<\< 52)
 * // Disable L4 cksum of TX pkt. PKT_TX_TCP_CKSUM     (1ULL \<\< 52)  //
 * TCP cksum of TX pkt. computed by nic. PKT_TX_SCTP_CKSUM    (2ULL \<\<
 * 52)  // SCTP cksum of TX pkt. computed by nic. PKT_TX_UDP_CKSUM
 * (3ULL \<\< 52)  // UDP cksum of TX pkt. computed by nic. \</pre\>
 */
#define NIX_SENDL4TYPE_E_NONE (0)
#define NIX_SENDL4TYPE_E_SCTP_CKSUM (2)
#define NIX_SENDL4TYPE_E_TCP_CKSUM (1)
#define NIX_SENDL4TYPE_E_UDP_CKSUM (3)

/**
 * Enumeration nix_sendldtype_e
 *
 * NIX Send Load Type Enumeration Enumerates the load transaction types
 * for reading segment bytes specified by NIX_SEND_SG_S[LD_TYPE] and
 * NIX_SEND_JUMP_S[LD_TYPE].  Internal: The hardware implementation
 * treats undefined encodings as LDD load type.
 */
#define NIX_SENDLDTYPE_E_LDD (0)
#define NIX_SENDLDTYPE_E_LDT (1)
#define NIX_SENDLDTYPE_E_LDWB (2)

/**
 * Enumeration nix_sendmemalg_e
 *
 * NIX Memory Modify Algorithm Enumeration Enumerates the different
 * algorithms for modifying memory; see NIX_SEND_MEM_S[ALG]. mbufs_freed
 * is the number of gather buffers freed to NPA for the send descriptor.
 * See NIX_SEND_HDR_S[DF] and NIX_SEND_SG_S[I*].
 */
#define NIX_SENDMEMALG_E_ADD (8)
#define NIX_SENDMEMALG_E_ADDLEN (0xa)
#define NIX_SENDMEMALG_E_ADDMBUF (0xc)
#define NIX_SENDMEMALG_E_SET (0)
#define NIX_SENDMEMALG_E_SETRSLT (2)
#define NIX_SENDMEMALG_E_SETTSTMP (1)
#define NIX_SENDMEMALG_E_SUB (9)
#define NIX_SENDMEMALG_E_SUBLEN (0xb)
#define NIX_SENDMEMALG_E_SUBMBUF (0xd)

/**
 * Enumeration nix_sendmemdsz_e
 *
 * NIX Memory Data Size Enumeration Enumerates the datum size for
 * modifying memory; see NIX_SEND_MEM_S[DSZ].
 */
#define NIX_SENDMEMDSZ_E_B16 (2)
#define NIX_SENDMEMDSZ_E_B32 (1)
#define NIX_SENDMEMDSZ_E_B64 (0)
#define NIX_SENDMEMDSZ_E_B8 (3)

/**
 * Enumeration nix_sqint_e
 *
 * NIX Send Queue Interrupt Enumeration Enumerates the bit index of
 * NIX_SQ_CTX_S[SQ_INT,SQ_INT_ENA].
 */
#define NIX_SQINT_E_LMT_ERR (0)
#define NIX_SQINT_E_MNQ_ERR (1)
#define NIX_SQINT_E_SEND_ERR (2)
#define NIX_SQINT_E_SQB_ALLOC_FAIL (3)

/**
 * Enumeration nix_sqoperr_e
 *
 * NIX SQ Operation Error Enumeration Enumerates
 * NIX_LF_SQ_OP_ERR_DBG[ERRCODE] values.
 */
#define NIX_SQOPERR_E_MAX_SQE_SIZE_ERR (4)
#define NIX_SQOPERR_E_SQB_FAULT (7)
#define NIX_SQOPERR_E_SQB_NULL (6)
#define NIX_SQOPERR_E_SQE_OFLOW (5)
#define NIX_SQOPERR_E_SQE_SIZEM1_ZERO (8)
#define NIX_SQOPERR_E_SQ_CTX_FAULT (1)
#define NIX_SQOPERR_E_SQ_CTX_POISON (2)
#define NIX_SQOPERR_E_SQ_DISABLED (3)
#define NIX_SQOPERR_E_SQ_OOR (0)

/**
 * Enumeration nix_stat_lf_rx_e
 *
 * NIX Local Function Receive Statistics Enumeration Enumerates the last
 * index of NIX_AF_LF()_RX_STAT() and NIX_LF_RX_STAT().
 */
#define NIX_STAT_LF_RX_E_RX_BCAST (2)
#define NIX_STAT_LF_RX_E_RX_DROP (4)
#define NIX_STAT_LF_RX_E_RX_DROP_OCTS (5)
#define NIX_STAT_LF_RX_E_RX_DRP_BCAST (8)
#define NIX_STAT_LF_RX_E_RX_DRP_L3BCAST (0xa)
#define NIX_STAT_LF_RX_E_RX_DRP_L3MCAST (0xb)
#define NIX_STAT_LF_RX_E_RX_DRP_MCAST (9)
#define NIX_STAT_LF_RX_E_RX_ERR (7)
#define NIX_STAT_LF_RX_E_RX_FCS (6)
#define NIX_STAT_LF_RX_E_RX_MCAST (3)
#define NIX_STAT_LF_RX_E_RX_OCTS (0)
#define NIX_STAT_LF_RX_E_RX_UCAST (1)

/**
 * Enumeration nix_stat_lf_tx_e
 *
 * NIX Local Function Transmit Statistics Enumeration Enumerates the
 * index of NIX_AF_LF()_TX_STAT() and NIX_LF_TX_STAT(). These statistics
 * do not account for packet replication due to NIX_TX_ACTION_S[OP] =
 * NIX_TX_ACTIONOP_E::MCAST.
 */
#define NIX_STAT_LF_TX_E_TX_BCAST (1)
#define NIX_STAT_LF_TX_E_TX_DROP (3)
#define NIX_STAT_LF_TX_E_TX_MCAST (2)
#define NIX_STAT_LF_TX_E_TX_OCTS (4)
#define NIX_STAT_LF_TX_E_TX_UCAST (0)

/**
 * Enumeration nix_stype_e
 *
 * NIX SQB Caching Type Enumeration Enumerates the values of
 * NIX_SQ_CTX_S[SQE_STYPE].
 */
#define NIX_STYPE_E_STF (0)
#define NIX_STYPE_E_STP (2)
#define NIX_STYPE_E_STT (1)

/**
 * Enumeration nix_subdc_e
 *
 * NIX Subdescriptor Operation Enumeration Enumerates send and receive
 * subdescriptor codes. The codes differentiate subdescriptors within a
 * NIX send or receive descriptor, excluding NIX_SEND_HDR_S for send and
 * NIX_CQE_HDR_S/NIX_WQE_HDR_S for receive, which are determined by their
 * position as the first subdescriptor, and NIX_RX_PARSE_S, which is
 * determined by its position as the second subdescriptor.
 */
#define NIX_SUBDC_E_CRC (2)
#define NIX_SUBDC_E_EXT (1)
#define NIX_SUBDC_E_IMM (3)
#define NIX_SUBDC_E_JUMP (6)
#define NIX_SUBDC_E_MEM (5)
#define NIX_SUBDC_E_NOP (0)
#define NIX_SUBDC_E_SG (4)
#define NIX_SUBDC_E_SOD (0xf)
#define NIX_SUBDC_E_WORK (7)

/**
 * Enumeration nix_tx_actionop_e
 *
 * NIX Transmit Action Opcode Enumeration Enumerates the values of
 * NIX_TX_ACTION_S[OP].
 */
#define NIX_TX_ACTIONOP_E_DROP (0)
#define NIX_TX_ACTIONOP_E_DROP_VIOL (5)
#define NIX_TX_ACTIONOP_E_MCAST (3)
#define NIX_TX_ACTIONOP_E_UCAST_CHAN (2)
#define NIX_TX_ACTIONOP_E_UCAST_DEFAULT (1)

/**
 * Enumeration nix_tx_vtagop_e
 *
 * NIX Transmit Vtag Opcode Enumeration Enumerates the values of
 * NIX_TX_VTAG_ACTION_S[VTAG0_OP,VTAG1_OP].
 */
#define NIX_TX_VTAGOP_E_INSERT (1)
#define NIX_TX_VTAGOP_E_NOP (0)
#define NIX_TX_VTAGOP_E_REPLACE (2)

/**
 * Enumeration nix_txlayer_e
 *
 * NIX Transmit Layer Enumeration Enumerates the values of
 * NIX_AF_LSO_FORMAT()_FIELD()[LAYER].
 */
#define NIX_TXLAYER_E_IL3 (2)
#define NIX_TXLAYER_E_IL4 (3)
#define NIX_TXLAYER_E_OL3 (0)
#define NIX_TXLAYER_E_OL4 (1)

/**
 * Enumeration nix_vtagsize_e
 *
 * NIX Vtag Size Enumeration Enumerates the values of
 * NIX_AF_TX_VTAG_DEF()_CTL[SIZE] and NIX_AF_LF()_RX_VTAG_TYPE()[SIZE].
 */
#define NIX_VTAGSIZE_E_T4 (0)
#define NIX_VTAGSIZE_E_T8 (1)

/**
 * Enumeration nix_xqe_type_e
 *
 * NIX WQE/CQE Type Enumeration Enumerates the values of
 * NIX_WQE_HDR_S[WQE_TYPE], NIX_CQE_HDR_S[CQE_TYPE].
 */
#define NIX_XQE_TYPE_E_INVALID (0)
#define NIX_XQE_TYPE_E_RX (1)
#define NIX_XQE_TYPE_E_RX_IPSECD (4)
#define NIX_XQE_TYPE_E_RX_IPSECH (3)
#define NIX_XQE_TYPE_E_RX_IPSECS (2)
#define NIX_XQE_TYPE_E_SEND (8)

/**
 * Enumeration nix_xqesz_e
 *
 * NIX WQE/CQE Size Enumeration Enumerates the values of
 * NIX_AF_LF()_CFG[XQE_SIZE].
 */
#define NIX_XQESZ_E_W16 (1)
#define NIX_XQESZ_E_W64 (0)

/**
 * Structure nix_aq_inst_s
 *
 * NIX Admin Queue Instruction Structure This structure specifies the AQ
 * instruction. Instructions and associated software structures are
 * stored in memory as little-endian unless NIX_AF_CFG[AF_BE] is set.
 * Hardware reads of NIX_AQ_INST_S do not allocate into LLC.  Hardware
 * reads and writes of the context structure selected by [CTYPE], [LF]
 * and [CINDEX] use the NDC and LLC caching style configured for that
 * context. For example: * When [CTYPE] = NIX_AQ_CTYPE_E::RQ: use
 * NIX_AF_LF()_RSS_CFG[CACHING] and NIX_AF_LF()_RSS_CFG[WAY_MASK]. * When
 * [CTYPE] = NIX_AQ_CTYPE_E::MCE: use NIX_AF_RX_MCAST_CFG[CACHING] and
 * NIX_AF_RX_MCAST_CFG[WAY_MASK].
 */
union nix_aq_inst_s {
	u64 u[2];
	struct nix_aq_inst_s_s {
		u64 op                               : 4;
		u64 ctype                            : 4;
		u64 lf                               : 7;
		u64 reserved_15_23                   : 9;
		u64 cindex                           : 20;
		u64 reserved_44_62                   : 19;
		u64 doneint                          : 1;
		u64 res_addr                         : 64;
	} s;
	/* struct nix_aq_inst_s_s cn; */
};

/**
 * Structure nix_aq_res_s
 *
 * NIX Admin Queue Result Structure NIX writes this structure after it
 * completes the NIX_AQ_INST_S instruction. The result structure is
 * exactly 16 bytes, and each instruction completion produces exactly one
 * result structure.  Results and associated software structures are
 * stored in memory as little-endian unless NIX_AF_CFG[AF_BE] is set.
 * When [OP] = NIX_AQ_INSTOP_E::INIT, WRITE or READ, this structure is
 * immediately followed by context read or write data. See
 * NIX_AQ_INSTOP_E.  Hardware writes of NIX_AQ_RES_S and context data
 * always allocate into LLC. Hardware reads of context data do not
 * allocate into LLC.
 */
union nix_aq_res_s {
	u64 u[2];
	struct nix_aq_res_s_s {
		u64 op                               : 4;
		u64 ctype                            : 4;
		u64 compcode                         : 8;
		u64 doneint                          : 1;
		u64 reserved_17_63                   : 47;
		u64 reserved_64_127                  : 64;
	} s;
	/* struct nix_aq_res_s_s cn; */
};

/**
 * Structure nix_cint_hw_s
 *
 * NIX Completion Interrupt Context Hardware Structure This structure
 * contains context state maintained by hardware for each completion
 * interrupt (CINT) in NDC/LLC/DRAM. Software accesses this structure
 * with the NIX_LF_CINT()* registers. Hardware maintains a table of
 * NIX_AF_CONST2[CINTS] contiguous NIX_CINT_HW_S structures per LF
 * starting at AF IOVA NIX_AF_LF()_CINTS_BASE. Always stored in byte
 * invariant little-endian format (LE8).
 */
union nix_cint_hw_s {
	u64 u[2];
	struct nix_cint_hw_s_s {
		u64 ecount                           : 32;
		u64 qcount                           : 16;
		u64 intr                             : 1;
		u64 ena                              : 1;
		u64 timer_idx                        : 8;
		u64 reserved_58_63                   : 6;
		u64 ecount_wait                      : 32;
		u64 qcount_wait                      : 16;
		u64 time_wait                        : 8;
		u64 reserved_120_127                 : 8;
	} s;
	/* struct nix_cint_hw_s_s cn; */
};

/**
 * Structure nix_cq_ctx_s
 *
 * NIX Completion Queue Context Structure This structure contains context
 * state maintained by hardware for each CQ in NDC/LLC/DRAM. Software
 * uses the same structure format to read and write an CQ context with
 * the NIX admin queue.
 */
union nix_cq_ctx_s {
	u64 u[4];
	struct nix_cq_ctx_s_s {
		u64 base                             : 64;
		u64 reserved_64_67                   : 4;
		u64 bp_ena                           : 1;
		u64 reserved_69_71                   : 3;
		u64 bpid                             : 9;
		u64 reserved_81_83                   : 3;
		u64 qint_idx                         : 7;
		u64 cq_err                           : 1;
		u64 cint_idx                         : 7;
		u64 avg_con                          : 9;
		u64 wrptr                            : 20;
		u64 tail                             : 20;
		u64 head                             : 20;
		u64 avg_level                        : 8;
		u64 update_time                      : 16;
		u64 bp                               : 8;
		u64 drop                             : 8;
		u64 drop_ena                         : 1;
		u64 ena                              : 1;
		u64 reserved_210_211                 : 2;
		u64 substream                        : 20;
		u64 caching                          : 1;
		u64 reserved_233_235                 : 3;
		u64 qsize                            : 4;
		u64 cq_err_int                       : 8;
		u64 cq_err_int_ena                   : 8;
	} s;
	/* struct nix_cq_ctx_s_s cn; */
};

/**
 * Structure nix_cqe_hdr_s
 *
 * NIX Completion Queue Entry Header Structure This 64-bit structure
 * defines the first word of every CQE. It is immediately followed by
 * NIX_RX_PARSE_S in a receive CQE, and by NIX_SEND_COMP_S in a send
 * completion CQE. Stored in memory as little-endian unless
 * NIX_AF_LF()_CFG[BE] is set.
 */
union nix_cqe_hdr_s {
	u64 u;
	struct nix_cqe_hdr_s_s {
		u64 tag                              : 32;
		u64 q                                : 20;
		u64 reserved_52_57                   : 6;
		u64 node                             : 2;
		u64 cqe_type                         : 4;
	} s;
	/* struct nix_cqe_hdr_s_s cn; */
};

/**
 * Structure nix_inst_hdr_s
 *
 * NIX Instruction Header Structure This structure defines the
 * instruction header that precedes the packet header supplied to NPC for
 * packets to be transmitted by NIX.
 */
union nix_inst_hdr_s {
	u64 u;
	struct nix_inst_hdr_s_s {
		u64 pf_func                          : 16;
		u64 sq                               : 20;
		u64 reserved_36_63                   : 28;
	} s;
	/* struct nix_inst_hdr_s_s cn; */
};

/**
 * Structure nix_iova_s
 *
 * NIX I/O Virtual Address Structure
 */
union nix_iova_s {
	u64 u;
	struct nix_iova_s_s {
		u64 addr                             : 64;
	} s;
	/* struct nix_iova_s_s cn; */
};

/**
 * Structure nix_ipsec_dyno_s
 *
 * INTERNAL: NIX IPSEC Dynamic Ordering Counter Structure  Internal: Not
 * used; no IPSEC fast-path.
 */
union nix_ipsec_dyno_s {
	u32 u;
	struct nix_ipsec_dyno_s_s {
		u32 count                            : 32;
	} s;
	/* struct nix_ipsec_dyno_s_s cn; */
};

/**
 * Structure nix_mem_result_s
 *
 * NIX Memory Value Structure When
 * NIX_SEND_MEM_S[ALG]=NIX_SENDMEMALG_E::SETRSLT, the value written to
 * memory is formed with this structure.
 */
union nix_mem_result_s {
	u64 u;
	struct nix_mem_result_s_s {
		u64 v                                : 1;
		u64 color                            : 2;
		u64 reserved_3_63                    : 61;
	} s;
	/* struct nix_mem_result_s_s cn; */
};

/**
 * Structure nix_op_q_wdata_s
 *
 * NIX Statistics Operation Write Data Structure This structure specifies
 * the write data format of an atomic 64-bit load-and-add of some
 * NIX_LF_RQ_OP_*, NIX_LF_SQ_OP* and NIX_LF_CQ_OP* registers.
 */
union nix_op_q_wdata_s {
	u64 u;
	struct nix_op_q_wdata_s_s {
		u64 reserved_0_31                    : 32;
		u64 q                                : 20;
		u64 reserved_52_63                   : 12;
	} s;
	/* struct nix_op_q_wdata_s_s cn; */
};

/**
 * Structure nix_qint_hw_s
 *
 * NIX Queue Interrupt Context Hardware Structure This structure contains
 * context state maintained by hardware for each queue interrupt (QINT)
 * in NDC/LLC/DRAM. Software accesses this structure with the
 * NIX_LF_QINT()* registers. Hardware maintains a table of
 * NIX_AF_CONST2[QINTS] contiguous NIX_QINT_HW_S structures per LF
 * starting at IOVA NIX_AF_LF()_QINTS_BASE. Always stored in byte
 * invariant little-endian format (LE8).
 */
union nix_qint_hw_s {
	u32 u;
	struct nix_qint_hw_s_s {
		u32 count                            : 22;
		u32 reserved_22_30                   : 9;
		u32 ena                              : 1;
	} s;
	/* struct nix_qint_hw_s_s cn; */
};

/**
 * Structure nix_rq_ctx_hw_s
 *
 * NIX Receive Queue Context Structure This structure contains context
 * state maintained by hardware for each RQ in NDC/LLC/DRAM. Software
 * uses the equivalent NIX_RQ_CTX_S structure format to read and write an
 * RQ context with the NIX admin queue. Always stored in byte invariant
 * little-endian format (LE8).
 */
union nix_rq_ctx_hw_s {
	u64 u[16];
	struct nix_rq_ctx_hw_s_s {
		u64 ena                              : 1;
		u64 sso_ena                          : 1;
		u64 ipsech_ena                       : 1;
		u64 ena_wqwd                         : 1;
		u64 cq                               : 20;
		u64 substream                        : 20;
		u64 wqe_aura                         : 20;
		u64 spb_aura                         : 20;
		u64 lpb_aura                         : 20;
		u64 sso_grp                          : 10;
		u64 sso_tt                           : 2;
		u64 pb_caching                       : 2;
		u64 wqe_caching                      : 1;
		u64 xqe_drop_ena                     : 1;
		u64 spb_drop_ena                     : 1;
		u64 lpb_drop_ena                     : 1;
		u64 wqe_skip                         : 2;
		u64 reserved_124_127                 : 4;
		u64 reserved_128_139                 : 12;
		u64 spb_sizem1                       : 6;
		u64 reserved_146_150                 : 5;
		u64 spb_ena                          : 1;
		u64 lpb_sizem1                       : 12;
		u64 first_skip                       : 7;
		u64 reserved_171                     : 1;
		u64 later_skip                       : 6;
		u64 xqe_imm_size                     : 6;
		u64 reserved_184_189                 : 6;
		u64 xqe_imm_copy                     : 1;
		u64 xqe_hdr_split                    : 1;
		u64 xqe_drop                         : 8;
		u64 xqe_pass                         : 8;
		u64 wqe_pool_drop                    : 8;
		u64 wqe_pool_pass                    : 8;
		u64 spb_aura_drop                    : 8;
		u64 spb_aura_pass                    : 8;
		u64 spb_pool_drop                    : 8;
		u64 spb_pool_pass                    : 8;
		u64 lpb_aura_drop                    : 8;
		u64 lpb_aura_pass                    : 8;
		u64 lpb_pool_drop                    : 8;
		u64 lpb_pool_pass                    : 8;
		u64 reserved_288_319                 : 32;
		u64 ltag                             : 24;
		u64 good_utag                        : 8;
		u64 bad_utag                         : 8;
		u64 flow_tagw                        : 6;
		u64 reserved_366_383                 : 18;
		u64 octs                             : 48;
		u64 reserved_432_447                 : 16;
		u64 pkts                             : 48;
		u64 reserved_496_511                 : 16;
		u64 drop_octs                        : 48;
		u64 reserved_560_575                 : 16;
		u64 drop_pkts                        : 48;
		u64 reserved_624_639                 : 16;
		u64 re_pkts                          : 48;
		u64 reserved_688_702                 : 15;
		u64 ena_copy                         : 1;
		u64 reserved_704_739                 : 36;
		u64 rq_int                           : 8;
		u64 rq_int_ena                       : 8;
		u64 qint_idx                         : 7;
		u64 reserved_763_767                 : 5;
		u64 reserved_768_831                 : 64;
		u64 reserved_832_895                 : 64;
		u64 reserved_896_959                 : 64;
		u64 reserved_960_1023                : 64;
	} s;
	/* struct nix_rq_ctx_hw_s_s cn; */
};

/**
 * Structure nix_rq_ctx_s
 *
 * NIX Receive Queue Context Structure This structure specifies the
 * format used by software to read and write an RQ context with the NIX
 * admin queue.
 */
union nix_rq_ctx_s {
	u64 u[16];
	struct nix_rq_ctx_s_s {
		u64 ena                              : 1;
		u64 sso_ena                          : 1;
		u64 ipsech_ena                       : 1;
		u64 ena_wqwd                         : 1;
		u64 cq                               : 20;
		u64 substream                        : 20;
		u64 wqe_aura                         : 20;
		u64 spb_aura                         : 20;
		u64 lpb_aura                         : 20;
		u64 sso_grp                          : 10;
		u64 sso_tt                           : 2;
		u64 pb_caching                       : 2;
		u64 wqe_caching                      : 1;
		u64 xqe_drop_ena                     : 1;
		u64 spb_drop_ena                     : 1;
		u64 lpb_drop_ena                     : 1;
		u64 reserved_122_127                 : 6;
		u64 reserved_128_139                 : 12;
		u64 spb_sizem1                       : 6;
		u64 wqe_skip                         : 2;
		u64 reserved_148_150                 : 3;
		u64 spb_ena                          : 1;
		u64 lpb_sizem1                       : 12;
		u64 first_skip                       : 7;
		u64 reserved_171                     : 1;
		u64 later_skip                       : 6;
		u64 xqe_imm_size                     : 6;
		u64 reserved_184_189                 : 6;
		u64 xqe_imm_copy                     : 1;
		u64 xqe_hdr_split                    : 1;
		u64 xqe_drop                         : 8;
		u64 xqe_pass                         : 8;
		u64 wqe_pool_drop                    : 8;
		u64 wqe_pool_pass                    : 8;
		u64 spb_aura_drop                    : 8;
		u64 spb_aura_pass                    : 8;
		u64 spb_pool_drop                    : 8;
		u64 spb_pool_pass                    : 8;
		u64 lpb_aura_drop                    : 8;
		u64 lpb_aura_pass                    : 8;
		u64 lpb_pool_drop                    : 8;
		u64 lpb_pool_pass                    : 8;
		u64 reserved_288_291                 : 4;
		u64 rq_int                           : 8;
		u64 rq_int_ena                       : 8;
		u64 qint_idx                         : 7;
		u64 reserved_315_319                 : 5;
		u64 ltag                             : 24;
		u64 good_utag                        : 8;
		u64 bad_utag                         : 8;
		u64 flow_tagw                        : 6;
		u64 reserved_366_383                 : 18;
		u64 octs                             : 48;
		u64 reserved_432_447                 : 16;
		u64 pkts                             : 48;
		u64 reserved_496_511                 : 16;
		u64 drop_octs                        : 48;
		u64 reserved_560_575                 : 16;
		u64 drop_pkts                        : 48;
		u64 reserved_624_639                 : 16;
		u64 re_pkts                          : 48;
		u64 reserved_688_703                 : 16;
		u64 reserved_704_767                 : 64;
		u64 reserved_768_831                 : 64;
		u64 reserved_832_895                 : 64;
		u64 reserved_896_959                 : 64;
		u64 reserved_960_1023                : 64;
	} s;
	/* struct nix_rq_ctx_s_s cn; */
};

/**
 * Structure nix_rsse_s
 *
 * NIX Receive Side Scaling Entry Structure This structure specifies the
 * format of each hardware entry in the NIX RSS tables in NDC/LLC/DRAM.
 * See NIX_AF_LF()_RSS_BASE and NIX_AF_LF()_RSS_GRP(). Software uses the
 * same structure format to read and write an RSS table entry with the
 * NIX admin queue.
 */
union nix_rsse_s {
	u32 u;
	struct nix_rsse_s_s {
		u32 rq                               : 20;
		u32 reserved_20_31                   : 12;
	} s;
	/* struct nix_rsse_s_s cn; */
};

/**
 * Structure nix_rx_action_s
 *
 * NIX Receive Action Structure This structure defines the format of
 * NPC_RESULT_S[ACTION] for a receive packet.
 */
union nix_rx_action_s {
	u64 u;
	struct nix_rx_action_s_s {
		u64 op                               : 4;
		u64 pf_func                          : 16;
		u64 index                            : 20;
		u64 match_id                         : 16;
		u64 flow_key_alg                     : 5;
		u64 reserved_61_63                   : 3;
	} s;
	/* struct nix_rx_action_s_s cn; */
};

/**
 * Structure nix_rx_imm_s
 *
 * NIX Receive Immediate Subdescriptor Structure The receive immediate
 * subdescriptor indicates that bytes immediately following this
 * NIX_RX_IMM_S (after skipping [APAD] bytes) were saved from the
 * received packet. The next subdescriptor following this NIX_RX_IMM_S
 * (when one exists) will follow the immediate bytes, after rounding up
 * the address to a multiple of 16 bytes.
 */
union nix_rx_imm_s {
	u64 u;
	struct nix_rx_imm_s_s {
		u64 size                             : 16;
		u64 apad                             : 3;
		u64 reserved_19_59                   : 41;
		u64 subdc                            : 4;
	} s;
	/* struct nix_rx_imm_s_s cn; */
};

/**
 * Structure nix_rx_mce_s
 *
 * NIX Receive Multicast/Mirror Entry Structure This structure specifies
 * the format of entries in the NIX receive multicast/mirror table
 * maintained by hardware in NDC/LLC/DRAM. See NIX_AF_RX_MCAST_BASE and
 * NIX_AF_RX_MCAST_CFG. Note the table may contain both multicast and
 * mirror replication lists. Software uses the same structure format to
 * read and write a multicast/mirror table entry with the NIX admin
 * queue.
 */
union nix_rx_mce_s {
	u64 u;
	struct nix_rx_mce_s_s {
		u64 op                               : 2;
		u64 reserved_2                       : 1;
		u64 eol                              : 1;
		u64 index                            : 20;
		u64 reserved_24_31                   : 8;
		u64 pf_func                          : 16;
		u64 next                             : 16;
	} s;
	/* struct nix_rx_mce_s_s cn; */
};

/**
 * Structure nix_rx_parse_s
 *
 * NIX Receive Parse Structure This structure contains the receive packet
 * parse result. It immediately follows NIX_CQE_HDR_S in a receive CQE,
 * or NIX_WQE_HDR_S in a receive WQE. Stored in memory as little-endian
 * unless NIX_AF_LF()_CFG[BE] is set.  Header layers are always 2-byte
 * aligned, so all header pointers in this structure ([EOH_PTR], [LAPTR]
 * through [LHPTR], [VTAG*_PTR]) are even.
 */
union nix_rx_parse_s {
	u64 u[7];
	struct nix_rx_parse_s_s {
		u64 chan                             : 12;
		u64 desc_sizem1                      : 5;
		u64 imm_copy                         : 1;
		u64 express                          : 1;
		u64 wqwd                             : 1;
		u64 errlev                           : 4;
		u64 errcode                          : 8;
		u64 latype                           : 4;
		u64 lbtype                           : 4;
		u64 lctype                           : 4;
		u64 ldtype                           : 4;
		u64 letype                           : 4;
		u64 lftype                           : 4;
		u64 lgtype                           : 4;
		u64 lhtype                           : 4;
		u64 pkt_lenm1                        : 16;
		u64 l2m                              : 1;
		u64 l2b                              : 1;
		u64 l3m                              : 1;
		u64 l3b                              : 1;
		u64 vtag0_valid                      : 1;
		u64 vtag0_gone                       : 1;
		u64 vtag1_valid                      : 1;
		u64 vtag1_gone                       : 1;
		u64 pkind                            : 6;
		u64 reserved_94_95                   : 2;
		u64 vtag0_tci                        : 16;
		u64 vtag1_tci                        : 16;
		u64 laflags                          : 8;
		u64 lbflags                          : 8;
		u64 lcflags                          : 8;
		u64 ldflags                          : 8;
		u64 leflags                          : 8;
		u64 lfflags                          : 8;
		u64 lgflags                          : 8;
		u64 lhflags                          : 8;
		u64 eoh_ptr                          : 8;
		u64 wqe_aura                         : 20;
		u64 pb_aura                          : 20;
		u64 match_id                         : 16;
		u64 laptr                            : 8;
		u64 lbptr                            : 8;
		u64 lcptr                            : 8;
		u64 ldptr                            : 8;
		u64 leptr                            : 8;
		u64 lfptr                            : 8;
		u64 lgptr                            : 8;
		u64 lhptr                            : 8;
		u64 vtag0_ptr                        : 8;
		u64 vtag1_ptr                        : 8;
		u64 flow_key_alg                     : 5;
		u64 reserved_341_383                 : 43;
		u64 reserved_384_447                 : 64;
	} s;
	/* struct nix_rx_parse_s_s cn; */
};

/**
 * Structure nix_rx_sg_s
 *
 * NIX Receive Scatter/Gather Subdescriptor Structure The receive
 * scatter/gather subdescriptor specifies one to three segments of packet
 * data bytes. There may be multiple NIX_RX_SG_Ss in each NIX receive
 * descriptor.  NIX_RX_SG_S is immediately followed by one NIX_IOVA_S
 * word when [SEGS] = 1, three NIX_IOVA_S words when [SEGS] \>= 2. Each
 * NIX_IOVA_S word specifies the LF IOVA of first packet data byte in the
 * corresponding segment; first NIX_IOVA_S word for segment 1, second
 * word for segment 2, third word for segment 3. Note the third word is
 * present when [SEGS] \>= 2 but only valid when [SEGS] = 3.
 */
union nix_rx_sg_s {
	u64 u;
	struct nix_rx_sg_s_s {
		u64 seg1_size                        : 16;
		u64 seg2_size                        : 16;
		u64 seg3_size                        : 16;
		u64 segs                             : 2;
		u64 reserved_50_59                   : 10;
		u64 subdc                            : 4;
	} s;
	/* struct nix_rx_sg_s_s cn; */
};

/**
 * Structure nix_rx_vtag_action_s
 *
 * NIX Receive Vtag Action Structure This structure defines the format of
 * NPC_RESULT_S[VTAG_ACTION] for a receive packet. It specifies up to two
 * Vtags (e.g. C-VLAN/S-VLAN tags, 802.1BR E-TAG) for optional capture
 * and/or stripping.
 */
union nix_rx_vtag_action_s {
	u64 u;
	struct nix_rx_vtag_action_s_s {
		u64 vtag0_relptr                     : 8;
		u64 vtag0_lid                        : 3;
		u64 reserved_11                      : 1;
		u64 vtag0_type                       : 3;
		u64 vtag0_valid                      : 1;
		u64 reserved_16_31                   : 16;
		u64 vtag1_relptr                     : 8;
		u64 vtag1_lid                        : 3;
		u64 reserved_43                      : 1;
		u64 vtag1_type                       : 3;
		u64 vtag1_valid                      : 1;
		u64 reserved_48_63                   : 16;
	} s;
	/* struct nix_rx_vtag_action_s_s cn; */
};

/**
 * Structure nix_send_comp_s
 *
 * NIX Send Completion Structure This structure immediately follows
 * NIX_CQE_HDR_S in a send completion CQE.
 */
union nix_send_comp_s {
	u64 u;
	struct nix_send_comp_s_s {
		u64 status                           : 8;
		u64 sqe_id                           : 16;
		u64 reserved_24_63                   : 40;
	} s;
	/* struct nix_send_comp_s_s cn; */
};

/**
 * Structure nix_send_crc_s
 *
 * NIX Send CRC Subdescriptor Structure The send CRC subdescriptor
 * specifies a CRC calculation be performed during transmission. Ignored
 * when present in a send descriptor with NIX_SEND_EXT_S[LSO] set. There
 * may be up to two NIX_SEND_CRC_Ss per send descriptor.  NIX_SEND_CRC_S
 * constraints: * When present, NIX_SEND_CRC_S subdescriptors must
 * precede all NIX_SEND_SG_S, NIX_SEND_IMM_S and NIX_SEND_MEM_S
 * subdescriptors in the send descriptor. * NIX_SEND_CRC_S subdescriptors
 * must follow the same order as their checksum and insert regions in the
 * packet, i.e. the checksum and insert regions of a NIX_SEND_CRC_S must
 * come after the checksum and insert regions of a preceding
 * NIX_SEND_CRC_S. There must be no overlap between any NIX_SEND_CRC_S
 * checksum and insert regions. * If either
 * NIX_SEND_HDR_S[OL4TYPE,IL4TYPE] = NIX_SENDL4TYPE_E::SCTP_CKSUM, the
 * SCTP checksum region and NIX_SEND_CRC_S insert region must not
 * overlap, and likewise the NIX_SEND_CRC_S checksum region and SCTP
 * insert region must not overlap. * If either
 * NIX_SEND_HDR_S[OL3TYPE,IL3TYPE] = NIX_SENDL3TYPE_E::IP4_CKSUM, the
 * IPv4 header checksum region and NIX_SEND_CRC_S insert region must not
 * overlap. * Any checksums inserted by
 * NIX_SEND_HDR_S[OL3TYPE,OL4TYPE,IL3TYPE,IL4TYPE] must be outside of the
 * NIX_SEND_CRC_S checksum and insert regions.  Hardware adjusts [START],
 * [SIZE] and [INSERT] as needed to account for any VLAN inserted by
 * NIX_SEND_EXT_S[VLAN*] or Vtag inserted by NIX_TX_VTAG_ACTION_S.
 */
union nix_send_crc_s {
	u64 u[2];
	struct nix_send_crc_s_s {
		u64 size                             : 16;
		u64 start                            : 16;
		u64 insert                           : 16;
		u64 reserved_48_57                   : 10;
		u64 alg                              : 2;
		u64 subdc                            : 4;
		u64 iv                               : 32;
		u64 reserved_96_127                  : 32;
	} s;
	/* struct nix_send_crc_s_s cn; */
};

/**
 * Structure nix_send_ext_s
 *
 * NIX Send Extended Header Subdescriptor Structure The send extended
 * header specifies LSO, VLAN insertion, timestamp and/or scheduling
 * services on the packet. If present, it must immediately follow
 * NIX_SEND_HDR_S. All fields are assumed to be zero when this
 * subdescriptor is not present.
 */
union nix_send_ext_s {
	u64 u[2];
	struct nix_send_ext_s_s {
		u64 lso_mps                          : 14;
		u64 lso                              : 1;
		u64 tstmp                            : 1;
		u64 lso_sb                           : 8;
		u64 lso_format                       : 5;
		u64 reserved_29_31                   : 3;
		u64 shp_chg                          : 9;
		u64 shp_dis                          : 1;
		u64 shp_ra                           : 2;
		u64 markptr                          : 8;
		u64 markform                         : 7;
		u64 mark_en                          : 1;
		u64 subdc                            : 4;
		u64 vlan0_ins_ptr                    : 8;
		u64 vlan0_ins_tci                    : 16;
		u64 vlan1_ins_ptr                    : 8;
		u64 vlan1_ins_tci                    : 16;
		u64 vlan0_ins_ena                    : 1;
		u64 vlan1_ins_ena                    : 1;
		u64 reserved_114_127                 : 14;
	} s;
	/* struct nix_send_ext_s_s cn; */
};

/**
 * Structure nix_send_hdr_s
 *
 * NIX Send Header Subdescriptor Structure The send header is the first
 * subdescriptor of every send descriptor.
 */
union nix_send_hdr_s {
	u64 u[2];
	struct nix_send_hdr_s_s {
		u64 total                            : 18;
		u64 reserved_18                      : 1;
		u64 df                               : 1;
		u64 aura                             : 20;
		u64 sizem1                           : 3;
		u64 pnc                              : 1;
		u64 sq                               : 20;
		u64 ol3ptr                           : 8;
		u64 ol4ptr                           : 8;
		u64 il3ptr                           : 8;
		u64 il4ptr                           : 8;
		u64 ol3type                          : 4;
		u64 ol4type                          : 4;
		u64 il3type                          : 4;
		u64 il4type                          : 4;
		u64 sqe_id                           : 16;
	} s;
	/* struct nix_send_hdr_s_s cn; */
};

/**
 * Structure nix_send_imm_s
 *
 * NIX Send Immediate Subdescriptor Structure The send immediate
 * subdescriptor requests that bytes immediately following this
 * NIX_SEND_IMM_S (after skipping [APAD] bytes) are to be included in the
 * packet data. The next subdescriptor following this NIX_SEND_IMM_S
 * (when one exists) will follow the immediate bytes, after rounding up
 * the address to a multiple of 16 bytes.  There may be multiple
 * NIX_SEND_IMM_S in one NIX send descriptor. A NIX_SEND_IMM_S is ignored
 * in a NIX send descriptor if the sum of all prior
 * NIX_SEND_SG_S[SEG*_SIZE]s and NIX_SEND_IMM_S[SIZE]s meets or exceeds
 * NIX_SEND_HDR_S[TOTAL].  When NIX_SEND_EXT_S[LSO] is set in the
 * descriptor, all NIX_SEND_IMM_S bytes must be included in the first
 * NIX_SEND_EXT_S[LSO_SB] bytes of the source packet.
 */
union nix_send_imm_s {
	u64 u;
	struct nix_send_imm_s_s {
		u64 size                             : 16;
		u64 apad                             : 3;
		u64 reserved_19_59                   : 41;
		u64 subdc                            : 4;
	} s;
	/* struct nix_send_imm_s_s cn; */
};

/**
 * Structure nix_send_jump_s
 *
 * NIX Send Jump Subdescriptor Structure The send jump subdescriptor
 * selects a new address for fetching the remaining subdescriptors of a
 * send descriptor. This allows software to create a send descriptor
 * longer than SQE size selected by NIX_SQ_CTX_S[MAX_SQE_SIZE].  There
 * can be only one NIX_SEND_JUMP_S subdescriptor in a send descriptor. If
 * present, it must immediately follow NIX_SEND_HDR_S if NIX_SEND_EXT_S
 * is not present, else it must immediately follow NIX_SEND_EXT_S. In
 * either case, it must terminate the SQE enqueued by software.
 */
union nix_send_jump_s {
	u64 u[2];
	struct nix_send_jump_s_s {
		u64 sizem1                           : 7;
		u64 reserved_7_13                    : 7;
		u64 ld_type                          : 2;
		u64 aura                             : 20;
		u64 reserved_36_58                   : 23;
		u64 f                                : 1;
		u64 subdc                            : 4;
		u64 addr                             : 64;
	} s;
	/* struct nix_send_jump_s_s cn; */
};

/**
 * Structure nix_send_mem_s
 *
 * NIX Send Memory Subdescriptor Structure The send memory subdescriptor
 * atomically sets, increments or decrements a memory location.
 * NIX_SEND_MEM_S subdescriptors must follow all NIX_SEND_SG_S and
 * NIX_SEND_IMM_S subdescriptors in the NIX send descriptor. NIX will not
 * initiate the memory update for this subdescriptor until after it has
 * completed all LLC/DRAM fetches that service all prior NIX_SEND_SG_S
 * subdescriptors. The memory update is executed once, even if the packet
 * is replicated due to NIX_TX_ACTION_S[OP] = NIX_TX_ACTIONOP_E::MCAST.
 * Performance is best if a memory decrement by one is used rather than
 * any other memory set/increment/decrement. (Less internal bus bandwidth
 * is used with memory decrements by one.)  When NIX_SEND_EXT_S[LSO] is
 * set in the descriptor, NIX executes the memory update only while
 * processing the last LSO segment, after processing prior segments.
 */
union nix_send_mem_s {
	u64 u[2];
	struct nix_send_mem_s_s {
		u64 offset                           : 16;
		u64 reserved_16_52                   : 37;
		u64 wmem                             : 1;
		u64 dsz                              : 2;
		u64 alg                              : 4;
		u64 subdc                            : 4;
		u64 addr                             : 64;
	} s;
	/* struct nix_send_mem_s_s cn; */
};

/**
 * Structure nix_send_sg_s
 *
 * NIX Send Scatter/Gather Subdescriptor Structure The send
 * scatter/gather subdescriptor requests one to three segments of packet
 * data bytes to be transmitted. There may be multiple NIX_SEND_SG_Ss in
 * each NIX send descriptor.  NIX_SEND_SG_S is immediately followed by
 * one NIX_IOVA_S word when [SEGS] = 1, three NIX_IOVA_S words when
 * [SEGS] \>= 2. Each NIX_IOVA_S word specifies the LF IOVA of first
 * packet data byte in the corresponding segment; first NIX_IOVA_S word
 * for segment 1, second word for segment 2, third word for segment 3.
 * Note the third word is present when [SEGS] \>= 2 but only valid when
 * [SEGS] = 3.  If the sum of all prior NIX_SEND_SG_S[SEG*_SIZE]s and
 * NIX_SEND_IMM_S[SIZE]s meets or exceeds NIX_SEND_HDR_S[TOTAL], this
 * subdescriptor will not contribute any packet data but may free buffers
 * to NPA (see [I1]).
 */
union nix_send_sg_s {
	u64 u;
	struct nix_send_sg_s_s {
		u64 seg1_size                        : 16;
		u64 seg2_size                        : 16;
		u64 seg3_size                        : 16;
		u64 segs                             : 2;
		u64 reserved_50_54                   : 5;
		u64 i1                               : 1;
		u64 i2                               : 1;
		u64 i3                               : 1;
		u64 ld_type                          : 2;
		u64 subdc                            : 4;
	} s;
	/* struct nix_send_sg_s_s cn; */
};

/**
 * Structure nix_send_work_s
 *
 * NIX Send Work Subdescriptor Structure This subdescriptor adds work to
 * the SSO. At most one NIX_SEND_WORK_S subdescriptor can exist in the
 * NIX send descriptor. If a NIX_SEND_WORK_S exists in the descriptor, it
 * must be the last subdescriptor. NIX will not initiate the work add for
 * this subdescriptor until after (1) it has completed all LLC/DRAM
 * fetches that service all prior NIX_SEND_SG_S subdescriptors, (2) it
 * has fetched all subdescriptors in the descriptor, and (3) all
 * NIX_SEND_MEM_S[WMEM]=1 LLC/DRAM updates have completed.  Provided the
 * path of descriptors from the SQ through NIX to an output FIFO is
 * unmodified between the descriptors (as should normally be the case,
 * but it is possible for software to change the path), NIX also (1) will
 * submit the SSO add works from all descriptors in the SQ in order, and
 * (2) will not submit an SSO work add until after all prior descriptors
 * in the SQ have completed their NIX_SEND_SG_S processing, and (3) will
 * not submit an SSO work add until after it has fetched all
 * subdescriptors from prior descriptors in the SQ.  When
 * NIX_SEND_EXT_S[LSO] is set in the descriptor, NIX executes the
 * NIX_SEND_WORK_S work add only while processing the last LSO segment,
 * after processing prior segments.  Hardware ignores NIX_SEND_WORK_S
 * when NIX_SQ_CTX_S[SSO_ENA] is clear.
 */
union nix_send_work_s {
	u64 u[2];
	struct nix_send_work_s_s {
		u64 tag                              : 32;
		u64 tt                               : 2;
		u64 grp                              : 10;
		u64 reserved_44_59                   : 16;
		u64 subdc                            : 4;
		u64 addr                             : 64;
	} s;
	/* struct nix_send_work_s_s cn; */
};

/**
 * Structure nix_sq_ctx_hw_s
 *
 * NIX SQ Context Hardware Structure This structure contains context
 * state maintained by hardware for each SQ in NDC/LLC/DRAM. Software
 * uses the equivalent NIX_SQ_CTX_S structure format to read and write an
 * SQ context with the NIX admin queue. Always stored in byte invariant
 * little-endian format (LE8).
 */
union nix_sq_ctx_hw_s {
	u64 u[16];
	struct nix_sq_ctx_hw_s_s {
		u64 ena                              : 1;
		u64 substream                        : 20;
		u64 max_sqe_size                     : 2;
		u64 sqe_way_mask                     : 16;
		u64 sqb_aura                         : 20;
		u64 gbl_rsvd1                        : 5;
		u64 cq_id                            : 20;
		u64 cq_ena                           : 1;
		u64 qint_idx                         : 6;
		u64 gbl_rsvd2                        : 1;
		u64 sq_int                           : 8;
		u64 sq_int_ena                       : 8;
		u64 xoff                             : 1;
		u64 sqe_stype                        : 2;
		u64 gbl_rsvd                         : 17;
		u64 head_sqb                         : 64;
		u64 head_offset                      : 6;
		u64 sqb_dequeue_count                : 16;
		u64 default_chan                     : 12;
		u64 sdp_mcast                        : 1;
		u64 sso_ena                          : 1;
		u64 dse_rsvd1                        : 28;
		u64 sqb_enqueue_count                : 16;
		u64 tail_offset                      : 6;
		u64 lmt_dis                          : 1;
		u64 smq_rr_quantum                   : 24;
		u64 dnq_rsvd1                        : 17;
		u64 tail_sqb                         : 64;
		u64 next_sqb                         : 64;
		u64 mnq_dis                          : 1;
		u64 smq                              : 9;
		u64 smq_pend                         : 1;
		u64 smq_next_sq                      : 20;
		u64 smq_next_sq_vld                  : 1;
		u64 scm1_rsvd2                       : 32;
		u64 smenq_sqb                        : 64;
		u64 smenq_offset                     : 6;
		u64 cq_limit                         : 8;
		u64 smq_rr_count                     : 25;
		u64 scm_lso_rem                      : 18;
		u64 scm_dq_rsvd0                     : 7;
		u64 smq_lso_segnum                   : 8;
		u64 vfi_lso_total                    : 18;
		u64 vfi_lso_sizem1                   : 3;
		u64 vfi_lso_sb                       : 8;
		u64 vfi_lso_mps                      : 14;
		u64 vfi_lso_vlan0_ins_ena            : 1;
		u64 vfi_lso_vlan1_ins_ena            : 1;
		u64 vfi_lso_vld                      : 1;
		u64 smenq_next_sqb_vld               : 1;
		u64 scm_dq_rsvd1                     : 9;
		u64 smenq_next_sqb                   : 64;
		u64 seb_rsvd1                        : 64;
		u64 drop_pkts                        : 48;
		u64 drop_octs_lsw                    : 16;
		u64 drop_octs_msw                    : 32;
		u64 pkts_lsw                         : 32;
		u64 pkts_msw                         : 16;
		u64 octs                             : 48;
	} s;
	/* struct nix_sq_ctx_hw_s_s cn; */
};

/**
 * Structure nix_sq_ctx_s
 *
 * NIX Send Queue Context Structure This structure specifies the format
 * used by software with the NIX admin queue to read and write a send
 * queue's NIX_SQ_CTX_HW_S structure maintained by hardware in
 * NDC/LLC/DRAM.  The SQ statistics ([OCTS], [PKTS], [DROP_OCTS],
 * [DROP_PKTS]) do not account for packet replication due to
 * NIX_TX_ACTION_S[OP] = NIX_TX_ACTIONOP_E::MCAST.
 */
union nix_sq_ctx_s {
	u64 u[16];
	struct nix_sq_ctx_s_s {
		u64 ena                              : 1;
		u64 qint_idx                         : 6;
		u64 substream                        : 20;
		u64 sdp_mcast                        : 1;
		u64 cq                               : 20;
		u64 sqe_way_mask                     : 16;
		u64 smq                              : 9;
		u64 cq_ena                           : 1;
		u64 xoff                             : 1;
		u64 sso_ena                          : 1;
		u64 smq_rr_quantum                   : 24;
		u64 default_chan                     : 12;
		u64 sqb_count                        : 16;
		u64 smq_rr_count                     : 25;
		u64 sqb_aura                         : 20;
		u64 sq_int                           : 8;
		u64 sq_int_ena                       : 8;
		u64 sqe_stype                        : 2;
		u64 reserved_191                     : 1;
		u64 max_sqe_size                     : 2;
		u64 cq_limit                         : 8;
		u64 lmt_dis                          : 1;
		u64 mnq_dis                          : 1;
		u64 smq_next_sq                      : 20;
		u64 smq_lso_segnum                   : 8;
		u64 tail_offset                      : 6;
		u64 smenq_offset                     : 6;
		u64 head_offset                      : 6;
		u64 smenq_next_sqb_vld               : 1;
		u64 smq_pend                         : 1;
		u64 smq_next_sq_vld                  : 1;
		u64 reserved_253_255                 : 3;
		u64 next_sqb                         : 64;
		u64 tail_sqb                         : 64;
		u64 smenq_sqb                        : 64;
		u64 smenq_next_sqb                   : 64;
		u64 head_sqb                         : 64;
		u64 reserved_576_583                 : 8;
		u64 vfi_lso_total                    : 18;
		u64 vfi_lso_sizem1                   : 3;
		u64 vfi_lso_sb                       : 8;
		u64 vfi_lso_mps                      : 14;
		u64 vfi_lso_vlan0_ins_ena            : 1;
		u64 vfi_lso_vlan1_ins_ena            : 1;
		u64 vfi_lso_vld                      : 1;
		u64 reserved_630_639                 : 10;
		u64 scm_lso_rem                      : 18;
		u64 reserved_658_703                 : 46;
		u64 octs                             : 48;
		u64 reserved_752_767                 : 16;
		u64 pkts                             : 48;
		u64 reserved_816_831                 : 16;
		u64 reserved_832_895                 : 64;
		u64 drop_octs                        : 48;
		u64 reserved_944_959                 : 16;
		u64 drop_pkts                        : 48;
		u64 reserved_1008_1023               : 16;
	} s;
	/* struct nix_sq_ctx_s_s cn; */
};

/**
 * Structure nix_tx_action_s
 *
 * NIX Transmit Action Structure This structure defines the format of
 * NPC_RESULT_S[ACTION] for a transmit packet.
 */
union nix_tx_action_s {
	u64 u;
	struct nix_tx_action_s_s {
		u64 op                               : 4;
		u64 reserved_4_11                    : 8;
		u64 index                            : 20;
		u64 match_id                         : 16;
		u64 reserved_48_63                   : 16;
	} s;
	/* struct nix_tx_action_s_s cn; */
};

/**
 * Structure nix_tx_vtag_action_s
 *
 * NIX Transmit Vtag Action Structure This structure defines the format
 * of NPC_RESULT_S[VTAG_ACTION] for a transmit packet. It specifies the
 * optional insertion or replacement of up to two Vtags (e.g.
 * C-VLAN/S-VLAN tags, 802.1BR E-TAG).  If two Vtags are specified: * The
 * Vtag 0 byte offset from packet start (see [VTAG0_RELPTR]) must be less
 * than or equal to the Vtag 1 byte offset. * Hardware executes the Vtag
 * 0 action first, Vtag 1 action second. * If Vtag 0 is inserted,
 * hardware adjusts the Vtag 1 byte offset accordingly. Thus, if the two
 * offsets are equal in the structure, hardware inserts Vtag 1
 * immediately after Vtag 0 in the packet.  A Vtag must not be inserted
 * or replaced within an outer or inner L3/L4 header, but may be inserted
 * or replaced within an outer L4 payload.
 */
union nix_tx_vtag_action_s {
	u64 u;
	struct nix_tx_vtag_action_s_s {
		u64 vtag0_relptr                     : 8;
		u64 vtag0_lid                        : 3;
		u64 reserved_11                      : 1;
		u64 vtag0_op                         : 2;
		u64 reserved_14_15                   : 2;
		u64 vtag0_def                        : 10;
		u64 reserved_26_31                   : 6;
		u64 vtag1_relptr                     : 8;
		u64 vtag1_lid                        : 3;
		u64 reserved_43                      : 1;
		u64 vtag1_op                         : 2;
		u64 reserved_46_47                   : 2;
		u64 vtag1_def                        : 10;
		u64 reserved_58_63                   : 6;
	} s;
	/* struct nix_tx_vtag_action_s_s cn; */
};

/**
 * Structure nix_wqe_hdr_s
 *
 * NIX Work Queue Entry Header Structure This 64-bit structure defines
 * the first word of every receive WQE generated by NIX. It is
 * immediately followed by NIX_RX_PARSE_S. Stored in memory as little-
 * endian unless NIX_AF_LF()_CFG[BE] is set.
 */
union nix_wqe_hdr_s {
	u64 u;
	struct nix_wqe_hdr_s_s {
		u64 tag                              : 32;
		u64 tt                               : 2;
		u64 grp                              : 10;
		u64 node                             : 2;
		u64 q                                : 14;
		u64 wqe_type                         : 4;
	} s;
	/* struct nix_wqe_hdr_s_s cn; */
};

/**
 * Register (RVU_PF_BAR0) nix#_af_aq_base
 *
 * NIX AF Admin Queue Base Address Register
 */
union nixx_af_aq_base {
	u64 u;
	struct nixx_af_aq_base_s {
		u64 reserved_0_6                     : 7;
		u64 base_addr                        : 46;
		u64 reserved_53_63                   : 11;
	} s;
	/* struct nixx_af_aq_base_s cn; */
};

static inline u64 NIXX_AF_AQ_BASE(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_AQ_BASE(void)
{
	return 0x410;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_aq_cfg
 *
 * NIX AF Admin Queue Configuration Register
 */
union nixx_af_aq_cfg {
	u64 u;
	struct nixx_af_aq_cfg_s {
		u64 qsize                            : 4;
		u64 reserved_4_63                    : 60;
	} s;
	/* struct nixx_af_aq_cfg_s cn; */
};

static inline u64 NIXX_AF_AQ_CFG(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_AQ_CFG(void)
{
	return 0x400;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_aq_done
 *
 * NIX AF Admin Queue Done Count Register
 */
union nixx_af_aq_done {
	u64 u;
	struct nixx_af_aq_done_s {
		u64 done                             : 20;
		u64 reserved_20_63                   : 44;
	} s;
	/* struct nixx_af_aq_done_s cn; */
};

static inline u64 NIXX_AF_AQ_DONE(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_AQ_DONE(void)
{
	return 0x450;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_aq_done_ack
 *
 * NIX AF Admin Queue Done Count Ack Register This register is written by
 * software to acknowledge interrupts.
 */
union nixx_af_aq_done_ack {
	u64 u;
	struct nixx_af_aq_done_ack_s {
		u64 done_ack                         : 20;
		u64 reserved_20_63                   : 44;
	} s;
	/* struct nixx_af_aq_done_ack_s cn; */
};

static inline u64 NIXX_AF_AQ_DONE_ACK(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_AQ_DONE_ACK(void)
{
	return 0x460;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_aq_done_ena_w1c
 *
 * NIX AF Admin Queue Done Interrupt Enable Clear Register
 */
union nixx_af_aq_done_ena_w1c {
	u64 u;
	struct nixx_af_aq_done_ena_w1c_s {
		u64 done                             : 1;
		u64 reserved_1_63                    : 63;
	} s;
	/* struct nixx_af_aq_done_ena_w1c_s cn; */
};

static inline u64 NIXX_AF_AQ_DONE_ENA_W1C(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_AQ_DONE_ENA_W1C(void)
{
	return 0x498;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_aq_done_ena_w1s
 *
 * NIX AF Admin Queue Done Interrupt Enable Set Register
 */
union nixx_af_aq_done_ena_w1s {
	u64 u;
	struct nixx_af_aq_done_ena_w1s_s {
		u64 done                             : 1;
		u64 reserved_1_63                    : 63;
	} s;
	/* struct nixx_af_aq_done_ena_w1s_s cn; */
};

static inline u64 NIXX_AF_AQ_DONE_ENA_W1S(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_AQ_DONE_ENA_W1S(void)
{
	return 0x490;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_aq_done_int
 *
 * INTERNAL: NIX AF Admin Queue Done Interrupt Register
 */
union nixx_af_aq_done_int {
	u64 u;
	struct nixx_af_aq_done_int_s {
		u64 done                             : 1;
		u64 reserved_1_63                    : 63;
	} s;
	/* struct nixx_af_aq_done_int_s cn; */
};

static inline u64 NIXX_AF_AQ_DONE_INT(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_AQ_DONE_INT(void)
{
	return 0x480;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_aq_done_int_w1s
 *
 * INTERNAL: NIX AF Admin Queue Done Interrupt Set Register
 */
union nixx_af_aq_done_int_w1s {
	u64 u;
	struct nixx_af_aq_done_int_w1s_s {
		u64 done                             : 1;
		u64 reserved_1_63                    : 63;
	} s;
	/* struct nixx_af_aq_done_int_w1s_s cn; */
};

static inline u64 NIXX_AF_AQ_DONE_INT_W1S(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_AQ_DONE_INT_W1S(void)
{
	return 0x488;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_aq_done_timer
 *
 * NIX AF Admin Queue Done Interrupt Timer Register
 */
union nixx_af_aq_done_timer {
	u64 u;
	struct nixx_af_aq_done_timer_s {
		u64 count                            : 16;
		u64 reserved_16_63                   : 48;
	} s;
	/* struct nixx_af_aq_done_timer_s cn; */
};

static inline u64 NIXX_AF_AQ_DONE_TIMER(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_AQ_DONE_TIMER(void)
{
	return 0x470;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_aq_done_wait
 *
 * NIX AF Admin Queue Done Interrupt Coalescing Wait Register Specifies
 * the queue interrupt coalescing settings.
 */
union nixx_af_aq_done_wait {
	u64 u;
	struct nixx_af_aq_done_wait_s {
		u64 num_wait                         : 20;
		u64 reserved_20_31                   : 12;
		u64 time_wait                        : 16;
		u64 reserved_48_63                   : 16;
	} s;
	/* struct nixx_af_aq_done_wait_s cn; */
};

static inline u64 NIXX_AF_AQ_DONE_WAIT(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_AQ_DONE_WAIT(void)
{
	return 0x440;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_aq_door
 *
 * NIX AF Admin Queue Doorbell Register Software writes to this register
 * to enqueue entries to AQ.
 */
union nixx_af_aq_door {
	u64 u;
	struct nixx_af_aq_door_s {
		u64 count                            : 16;
		u64 reserved_16_63                   : 48;
	} s;
	/* struct nixx_af_aq_door_s cn; */
};

static inline u64 NIXX_AF_AQ_DOOR(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_AQ_DOOR(void)
{
	return 0x430;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_aq_status
 *
 * NIX AF Admin Queue Status Register
 */
union nixx_af_aq_status {
	u64 u;
	struct nixx_af_aq_status_s {
		u64 reserved_0_3                     : 4;
		u64 head_ptr                         : 20;
		u64 reserved_24_35                   : 12;
		u64 tail_ptr                         : 20;
		u64 reserved_56_61                   : 6;
		u64 aq_busy                          : 1;
		u64 aq_err                           : 1;
	} s;
	struct nixx_af_aq_status_cn {
		u64 reserved_0_3                     : 4;
		u64 head_ptr                         : 20;
		u64 reserved_24_31                   : 8;
		u64 reserved_32_35                   : 4;
		u64 tail_ptr                         : 20;
		u64 reserved_56_61                   : 6;
		u64 aq_busy                          : 1;
		u64 aq_err                           : 1;
	} cn;
};

static inline u64 NIXX_AF_AQ_STATUS(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_AQ_STATUS(void)
{
	return 0x420;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_avg_delay
 *
 * NIX AF Queue Average Delay Register
 */
union nixx_af_avg_delay {
	u64 u;
	struct nixx_af_avg_delay_s {
		u64 avg_dly                          : 19;
		u64 reserved_19_23                   : 5;
		u64 avg_timer                        : 16;
		u64 reserved_40_63                   : 24;
	} s;
	/* struct nixx_af_avg_delay_s cn; */
};

static inline u64 NIXX_AF_AVG_DELAY(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_AVG_DELAY(void)
{
	return 0xe0;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_bar2_alias#
 *
 * NIX Admin Function  BAR2 Alias Registers These registers alias to the
 * NIX BAR2 registers for the PF and function selected by
 * NIX_AF_BAR2_SEL[PF_FUNC].  Internal: Not implemented. Placeholder for
 * bug33464.
 */
union nixx_af_bar2_aliasx {
	u64 u;
	struct nixx_af_bar2_aliasx_s {
		u64 data                             : 64;
	} s;
	/* struct nixx_af_bar2_aliasx_s cn; */
};

static inline u64 NIXX_AF_BAR2_ALIASX(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_BAR2_ALIASX(u64 a)
{
	return 0x9100000 + 8 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_bar2_sel
 *
 * NIX Admin Function BAR2 Select Register This register configures BAR2
 * accesses from the NIX_AF_BAR2_ALIAS() registers in BAR0. Internal: Not
 * implemented. Placeholder for bug33464.
 */
union nixx_af_bar2_sel {
	u64 u;
	struct nixx_af_bar2_sel_s {
		u64 alias_pf_func                    : 16;
		u64 alias_ena                        : 1;
		u64 reserved_17_63                   : 47;
	} s;
	/* struct nixx_af_bar2_sel_s cn; */
};

static inline u64 NIXX_AF_BAR2_SEL(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_BAR2_SEL(void)
{
	return 0x9000000;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_blk_rst
 *
 * NIX AF Block Reset Register
 */
union nixx_af_blk_rst {
	u64 u;
	struct nixx_af_blk_rst_s {
		u64 rst                              : 1;
		u64 reserved_1_62                    : 62;
		u64 busy                             : 1;
	} s;
	/* struct nixx_af_blk_rst_s cn; */
};

static inline u64 NIXX_AF_BLK_RST(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_BLK_RST(void)
{
	return 0xb0;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_cfg
 *
 * NIX AF General Configuration Register
 */
union nixx_af_cfg {
	u64 u;
	struct nixx_af_cfg_s {
		u64 force_cond_clk_en                : 1;
		u64 force_rx_gbl_clk_en              : 1;
		u64 force_rx_strm_clk_en             : 1;
		u64 force_cqm_clk_en                 : 1;
		u64 force_seb_clk_en                 : 1;
		u64 force_sqm_clk_en                 : 1;
		u64 force_pse_clk_en                 : 1;
		u64 reserved_7                       : 1;
		u64 af_be                            : 1;
		u64 calibrate_x2p                    : 1;
		u64 force_intf_clk_en                : 1;
		u64 reserved_11_63                   : 53;
	} s;
	/* struct nixx_af_cfg_s cn; */
};

static inline u64 NIXX_AF_CFG(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_CFG(void)
{
	return 0;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_cint_delay
 *
 * NIX AF Completion Interrupt Delay Register
 */
union nixx_af_cint_delay {
	u64 u;
	struct nixx_af_cint_delay_s {
		u64 cint_dly                         : 10;
		u64 reserved_10_15                   : 6;
		u64 cint_timer                       : 16;
		u64 reserved_32_63                   : 32;
	} s;
	/* struct nixx_af_cint_delay_s cn; */
};

static inline u64 NIXX_AF_CINT_DELAY(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_CINT_DELAY(void)
{
	return 0xf0;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_cint_timer#
 *
 * NIX AF Completion Interrupt Timer Registers
 */
union nixx_af_cint_timerx {
	u64 u;
	struct nixx_af_cint_timerx_s {
		u64 expir_time                       : 16;
		u64 cint                             : 7;
		u64 reserved_23                      : 1;
		u64 lf                               : 8;
		u64 active                           : 1;
		u64 reserved_33_63                   : 31;
	} s;
	/* struct nixx_af_cint_timerx_s cn; */
};

static inline u64 NIXX_AF_CINT_TIMERX(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_CINT_TIMERX(u64 a)
{
	return 0x1a40 + 0x40000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_const
 *
 * NIX AF Constants Register This register contains constants for
 * software discovery.
 */
union nixx_af_const {
	u64 u;
	struct nixx_af_const_s {
		u64 cgx_lmac_channels                : 8;
		u64 cgx_lmacs                        : 4;
		u64 num_cgx                          : 4;
		u64 lbk_channels                     : 8;
		u64 num_lbk                          : 4;
		u64 num_sdp                          : 4;
		u64 reserved_32_47                   : 16;
		u64 links                            : 8;
		u64 intfs                            : 4;
		u64 reserved_60_63                   : 4;
	} s;
	/* struct nixx_af_const_s cn; */
};

static inline u64 NIXX_AF_CONST(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_CONST(void)
{
	return 0x20;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_const1
 *
 * NIX AF Constants 1 Register This register contains constants for
 * software discovery.
 */
union nixx_af_const1 {
	u64 u;
	struct nixx_af_const1_s {
		u64 sdp_channels                     : 12;
		u64 rx_bpids                         : 12;
		u64 lf_tx_stats                      : 8;
		u64 lf_rx_stats                      : 8;
		u64 lso_format_fields                : 8;
		u64 lso_formats                      : 8;
		u64 reserved_56_63                   : 8;
	} s;
	/* struct nixx_af_const1_s cn; */
};

static inline u64 NIXX_AF_CONST1(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_CONST1(void)
{
	return 0x28;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_const2
 *
 * NIX AF Constants 2 Register This register contains constants for
 * software discovery.
 */
union nixx_af_const2 {
	u64 u;
	struct nixx_af_const2_s {
		u64 lfs                              : 12;
		u64 qints                            : 12;
		u64 cints                            : 12;
		u64 reserved_36_63                   : 28;
	} s;
	/* struct nixx_af_const2_s cn; */
};

static inline u64 NIXX_AF_CONST2(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_CONST2(void)
{
	return 0x30;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_const3
 *
 * NIX AF Constants 2 Register This register contains constants for
 * software discovery.
 */
union nixx_af_const3 {
	u64 u;
	struct nixx_af_const3_s {
		u64 sq_ctx_log2bytes                 : 4;
		u64 rq_ctx_log2bytes                 : 4;
		u64 cq_ctx_log2bytes                 : 4;
		u64 rsse_log2bytes                   : 4;
		u64 mce_log2bytes                    : 4;
		u64 qint_log2bytes                   : 4;
		u64 cint_log2bytes                   : 4;
		u64 dyno_log2bytes                   : 4;
		u64 reserved_32_63                   : 32;
	} s;
	/* struct nixx_af_const3_s cn; */
};

static inline u64 NIXX_AF_CONST3(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_CONST3(void)
{
	return 0x38;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_cq_const
 *
 * NIX AF CQ Constants Register This register contains constants for
 * software discovery.
 */
union nixx_af_cq_const {
	u64 u;
	struct nixx_af_cq_const_s {
		u64 queues_per_lf                    : 24;
		u64 reserved_24_63                   : 40;
	} s;
	/* struct nixx_af_cq_const_s cn; */
};

static inline u64 NIXX_AF_CQ_CONST(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_CQ_CONST(void)
{
	return 0x48;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_cqm_bp_test
 *
 * INTERNAL: NIX AF CQM Backpressure Test Registers
 */
union nixx_af_cqm_bp_test {
	u64 u;
	struct nixx_af_cqm_bp_test_s {
		u64 lfsr_freq                        : 12;
		u64 reserved_12_15                   : 4;
		u64 bp_cfg                           : 24;
		u64 enable                           : 12;
		u64 reserved_52_63                   : 12;
	} s;
	/* struct nixx_af_cqm_bp_test_s cn; */
};

static inline u64 NIXX_AF_CQM_BP_TEST(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_CQM_BP_TEST(void)
{
	return 0x48c0;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_cqm_eco
 *
 * INTERNAL: AF CQM ECO Register
 */
union nixx_af_cqm_eco {
	u64 u;
	struct nixx_af_cqm_eco_s {
		u64 eco_rw                           : 64;
	} s;
	/* struct nixx_af_cqm_eco_s cn; */
};

static inline u64 NIXX_AF_CQM_ECO(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_CQM_ECO(void)
{
	return 0x590;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_csi_eco
 *
 * INTERNAL: AF CSI ECO Register
 */
union nixx_af_csi_eco {
	u64 u;
	struct nixx_af_csi_eco_s {
		u64 eco_rw                           : 64;
	} s;
	/* struct nixx_af_csi_eco_s cn; */
};

static inline u64 NIXX_AF_CSI_ECO(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_CSI_ECO(void)
{
	return 0x580;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_err_int
 *
 * NIX Admin Function Error Interrupt Register
 */
union nixx_af_err_int {
	u64 u;
	struct nixx_af_err_int_s {
		u64 rx_mcast_data_fault              : 1;
		u64 rx_mirror_data_fault             : 1;
		u64 rx_mcast_wqe_fault               : 1;
		u64 rx_mirror_wqe_fault              : 1;
		u64 rx_mce_fault                     : 1;
		u64 rx_mce_list_err                  : 1;
		u64 rx_unmapped_pf_func              : 1;
		u64 reserved_7_11                    : 5;
		u64 aq_door_err                      : 1;
		u64 aq_res_fault                     : 1;
		u64 aq_inst_fault                    : 1;
		u64 reserved_15_63                   : 49;
	} s;
	/* struct nixx_af_err_int_s cn; */
};

static inline u64 NIXX_AF_ERR_INT(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_ERR_INT(void)
{
	return 0x180;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_err_int_ena_w1c
 *
 * NIX Admin Function Error Interrupt Enable Clear Register This register
 * clears interrupt enable bits.
 */
union nixx_af_err_int_ena_w1c {
	u64 u;
	struct nixx_af_err_int_ena_w1c_s {
		u64 rx_mcast_data_fault              : 1;
		u64 rx_mirror_data_fault             : 1;
		u64 rx_mcast_wqe_fault               : 1;
		u64 rx_mirror_wqe_fault              : 1;
		u64 rx_mce_fault                     : 1;
		u64 rx_mce_list_err                  : 1;
		u64 rx_unmapped_pf_func              : 1;
		u64 reserved_7_11                    : 5;
		u64 aq_door_err                      : 1;
		u64 aq_res_fault                     : 1;
		u64 aq_inst_fault                    : 1;
		u64 reserved_15_63                   : 49;
	} s;
	/* struct nixx_af_err_int_ena_w1c_s cn; */
};

static inline u64 NIXX_AF_ERR_INT_ENA_W1C(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_ERR_INT_ENA_W1C(void)
{
	return 0x198;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_err_int_ena_w1s
 *
 * NIX Admin Function Error Interrupt Enable Set Register This register
 * sets interrupt enable bits.
 */
union nixx_af_err_int_ena_w1s {
	u64 u;
	struct nixx_af_err_int_ena_w1s_s {
		u64 rx_mcast_data_fault              : 1;
		u64 rx_mirror_data_fault             : 1;
		u64 rx_mcast_wqe_fault               : 1;
		u64 rx_mirror_wqe_fault              : 1;
		u64 rx_mce_fault                     : 1;
		u64 rx_mce_list_err                  : 1;
		u64 rx_unmapped_pf_func              : 1;
		u64 reserved_7_11                    : 5;
		u64 aq_door_err                      : 1;
		u64 aq_res_fault                     : 1;
		u64 aq_inst_fault                    : 1;
		u64 reserved_15_63                   : 49;
	} s;
	/* struct nixx_af_err_int_ena_w1s_s cn; */
};

static inline u64 NIXX_AF_ERR_INT_ENA_W1S(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_ERR_INT_ENA_W1S(void)
{
	return 0x190;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_err_int_w1s
 *
 * NIX Admin Function Error Interrupt Set Register This register sets
 * interrupt bits.
 */
union nixx_af_err_int_w1s {
	u64 u;
	struct nixx_af_err_int_w1s_s {
		u64 rx_mcast_data_fault              : 1;
		u64 rx_mirror_data_fault             : 1;
		u64 rx_mcast_wqe_fault               : 1;
		u64 rx_mirror_wqe_fault              : 1;
		u64 rx_mce_fault                     : 1;
		u64 rx_mce_list_err                  : 1;
		u64 rx_unmapped_pf_func              : 1;
		u64 reserved_7_11                    : 5;
		u64 aq_door_err                      : 1;
		u64 aq_res_fault                     : 1;
		u64 aq_inst_fault                    : 1;
		u64 reserved_15_63                   : 49;
	} s;
	/* struct nixx_af_err_int_w1s_s cn; */
};

static inline u64 NIXX_AF_ERR_INT_W1S(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_ERR_INT_W1S(void)
{
	return 0x188;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_expr_tx_fifo_status
 *
 * INTERNAL: NIX AF Express Transmit FIFO Status Register  Internal:
 * 802.3br frame preemption/express path is defeatured. Old definition:
 * Status of FIFO which transmits express packets to CGX and LBK.
 */
union nixx_af_expr_tx_fifo_status {
	u64 u;
	struct nixx_af_expr_tx_fifo_status_s {
		u64 count                            : 12;
		u64 reserved_12_63                   : 52;
	} s;
	/* struct nixx_af_expr_tx_fifo_status_s cn; */
};

static inline u64 NIXX_AF_EXPR_TX_FIFO_STATUS(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_EXPR_TX_FIFO_STATUS(void)
{
	return 0x640;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_gen_int
 *
 * NIX AF General Interrupt Register
 */
union nixx_af_gen_int {
	u64 u;
	struct nixx_af_gen_int_s {
		u64 rx_mcast_drop                    : 1;
		u64 rx_mirror_drop                   : 1;
		u64 reserved_2                       : 1;
		u64 tl1_drain                        : 1;
		u64 smq_flush_done                   : 1;
		u64 reserved_5_63                    : 59;
	} s;
	/* struct nixx_af_gen_int_s cn; */
};

static inline u64 NIXX_AF_GEN_INT(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_GEN_INT(void)
{
	return 0x160;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_gen_int_ena_w1c
 *
 * NIX AF General Interrupt Enable Clear Register This register clears
 * interrupt enable bits.
 */
union nixx_af_gen_int_ena_w1c {
	u64 u;
	struct nixx_af_gen_int_ena_w1c_s {
		u64 rx_mcast_drop                    : 1;
		u64 rx_mirror_drop                   : 1;
		u64 reserved_2                       : 1;
		u64 tl1_drain                        : 1;
		u64 smq_flush_done                   : 1;
		u64 reserved_5_63                    : 59;
	} s;
	/* struct nixx_af_gen_int_ena_w1c_s cn; */
};

static inline u64 NIXX_AF_GEN_INT_ENA_W1C(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_GEN_INT_ENA_W1C(void)
{
	return 0x178;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_gen_int_ena_w1s
 *
 * NIX AF General Interrupt Enable Set Register This register sets
 * interrupt enable bits.
 */
union nixx_af_gen_int_ena_w1s {
	u64 u;
	struct nixx_af_gen_int_ena_w1s_s {
		u64 rx_mcast_drop                    : 1;
		u64 rx_mirror_drop                   : 1;
		u64 reserved_2                       : 1;
		u64 tl1_drain                        : 1;
		u64 smq_flush_done                   : 1;
		u64 reserved_5_63                    : 59;
	} s;
	/* struct nixx_af_gen_int_ena_w1s_s cn; */
};

static inline u64 NIXX_AF_GEN_INT_ENA_W1S(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_GEN_INT_ENA_W1S(void)
{
	return 0x170;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_gen_int_w1s
 *
 * NIX AF General Interrupt Set Register This register sets interrupt
 * bits.
 */
union nixx_af_gen_int_w1s {
	u64 u;
	struct nixx_af_gen_int_w1s_s {
		u64 rx_mcast_drop                    : 1;
		u64 rx_mirror_drop                   : 1;
		u64 reserved_2                       : 1;
		u64 tl1_drain                        : 1;
		u64 smq_flush_done                   : 1;
		u64 reserved_5_63                    : 59;
	} s;
	/* struct nixx_af_gen_int_w1s_s cn; */
};

static inline u64 NIXX_AF_GEN_INT_W1S(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_GEN_INT_W1S(void)
{
	return 0x168;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_lf#_cfg
 *
 * NIX AF Local Function Configuration Registers
 */
union nixx_af_lfx_cfg {
	u64 u;
	struct nixx_af_lfx_cfg_s {
		u64 npa_pf_func                      : 16;
		u64 sso_pf_func                      : 16;
		u64 be                               : 1;
		u64 xqe_size                         : 2;
		u64 reserved_35_63                   : 29;
	} s;
	/* struct nixx_af_lfx_cfg_s cn; */
};

static inline u64 NIXX_AF_LFX_CFG(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_LFX_CFG(u64 a)
{
	return 0x4000 + 0x20000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_lf#_cints_base
 *
 * NIX AF Local Function Completion Interrupts Base Address Registers
 * This register specifies the base AF IOVA of LF's completion interrupt
 * context table in NDC/LLC/DRAM. The table consists of
 * NIX_AF_CONST2[CINTS] contiguous NIX_CINT_HW_S structures.  After
 * writing to this register, software should read it back to ensure that
 * the write has completed before accessing any NIX_LF_CINT()_*
 * registers.
 */
union nixx_af_lfx_cints_base {
	u64 u;
	struct nixx_af_lfx_cints_base_s {
		u64 reserved_0_6                     : 7;
		u64 addr                             : 46;
		u64 reserved_53_63                   : 11;
	} s;
	/* struct nixx_af_lfx_cints_base_s cn; */
};

static inline u64 NIXX_AF_LFX_CINTS_BASE(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_LFX_CINTS_BASE(u64 a)
{
	return 0x4130 + 0x20000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_lf#_cints_cfg
 *
 * NIX AF Local Function Completion Interrupts Configuration Registers
 * This register controls access to the LF's completion interrupt context
 * table in NDC/LLC/DRAM. The table consists of NIX_AF_CONST2[CINTS]
 * contiguous NIX_CINT_HW_S structures. The size of each structure is 1
 * \<\< NIX_AF_CONST3[CINT_LOG2BYTES].  After writing to this register,
 * software should read it back to ensure that the write has completed
 * before accessing any NIX_LF_CINT()_* registers.
 */
union nixx_af_lfx_cints_cfg {
	u64 u;
	struct nixx_af_lfx_cints_cfg_s {
		u64 reserved_0_19                    : 20;
		u64 way_mask                         : 16;
		u64 caching                          : 1;
		u64 reserved_37_63                   : 27;
	} s;
	/* struct nixx_af_lfx_cints_cfg_s cn; */
};

static inline u64 NIXX_AF_LFX_CINTS_CFG(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_LFX_CINTS_CFG(u64 a)
{
	return 0x4120 + 0x20000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_lf#_cqs_base
 *
 * NIX AF Local Function Completion Queues Base Address Register This
 * register specifies the base AF IOVA of the LF's CQ context table. The
 * table consists of NIX_AF_LF()_CQS_CFG[MAX_QUEUESM1]+1 contiguous
 * NIX_CQ_CTX_S structures.
 */
union nixx_af_lfx_cqs_base {
	u64 u;
	struct nixx_af_lfx_cqs_base_s {
		u64 reserved_0_6                     : 7;
		u64 addr                             : 46;
		u64 reserved_53_63                   : 11;
	} s;
	/* struct nixx_af_lfx_cqs_base_s cn; */
};

static inline u64 NIXX_AF_LFX_CQS_BASE(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_LFX_CQS_BASE(u64 a)
{
	return 0x4070 + 0x20000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_lf#_cqs_cfg
 *
 * NIX AF Local Function Completion Queues Configuration Register This
 * register configures completion queues in the LF.
 */
union nixx_af_lfx_cqs_cfg {
	u64 u;
	struct nixx_af_lfx_cqs_cfg_s {
		u64 max_queuesm1                     : 20;
		u64 way_mask                         : 16;
		u64 caching                          : 1;
		u64 reserved_37_63                   : 27;
	} s;
	/* struct nixx_af_lfx_cqs_cfg_s cn; */
};

static inline u64 NIXX_AF_LFX_CQS_CFG(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_LFX_CQS_CFG(u64 a)
{
	return 0x4060 + 0x20000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_lf#_lock#
 *
 * NIX AF Local Function Lockdown Registers Internal: The NIX lockdown
 * depth of 32 bytes is shallow compared to 96 bytes for NIC and meant
 * for outer MAC and/or VLAN (optionally preceded by a small number of
 * skip bytes). NPC's MCAM can be used for deeper protocol-aware
 * lockdown.
 */
union nixx_af_lfx_lockx {
	u64 u;
	struct nixx_af_lfx_lockx_s {
		u64 data                             : 32;
		u64 bit_ena                          : 32;
	} s;
	/* struct nixx_af_lfx_lockx_s cn; */
};

static inline u64 NIXX_AF_LFX_LOCKX(u64 a, u64 b)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_LFX_LOCKX(u64 a, u64 b)
{
	return 0x4300 + 0x20000 * a + 8 * b;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_lf#_qints_base
 *
 * NIX AF Local Function Queue Interrupts Base Address Registers This
 * register specifies the base AF IOVA of LF's queue interrupt context
 * table in NDC/LLC/DRAM. The table consists of NIX_AF_CONST2[QINTS]
 * contiguous NIX_QINT_HW_S structures.  After writing to this register,
 * software should read it back to ensure that the write has completed
 * before accessing any NIX_LF_QINT()_* registers.
 */
union nixx_af_lfx_qints_base {
	u64 u;
	struct nixx_af_lfx_qints_base_s {
		u64 reserved_0_6                     : 7;
		u64 addr                             : 46;
		u64 reserved_53_63                   : 11;
	} s;
	/* struct nixx_af_lfx_qints_base_s cn; */
};

static inline u64 NIXX_AF_LFX_QINTS_BASE(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_LFX_QINTS_BASE(u64 a)
{
	return 0x4110 + 0x20000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_lf#_qints_cfg
 *
 * NIX AF Local Function Queue Interrupts Configuration Registers This
 * register controls access to the LF's queue interrupt context table in
 * NDC/LLC/DRAM. The table consists of NIX_AF_CONST2[QINTS] contiguous
 * NIX_QINT_HW_S structures. The size of each structure is 1 \<\<
 * NIX_AF_CONST3[QINT_LOG2BYTES].  After writing to this register,
 * software should read it back to ensure that the write has completed
 * before accessing any NIX_LF_QINT()_* registers.
 */
union nixx_af_lfx_qints_cfg {
	u64 u;
	struct nixx_af_lfx_qints_cfg_s {
		u64 reserved_0_19                    : 20;
		u64 way_mask                         : 16;
		u64 caching                          : 1;
		u64 reserved_37_63                   : 27;
	} s;
	/* struct nixx_af_lfx_qints_cfg_s cn; */
};

static inline u64 NIXX_AF_LFX_QINTS_CFG(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_LFX_QINTS_CFG(u64 a)
{
	return 0x4100 + 0x20000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_lf#_rqs_base
 *
 * NIX AF Local Function Receive Queues Base Address Register This
 * register specifies the base AF IOVA of the LF's RQ context table. The
 * table consists of NIX_AF_LF()_RQS_CFG[MAX_QUEUESM1]+1 contiguous
 * NIX_RQ_CTX_S structures.
 */
union nixx_af_lfx_rqs_base {
	u64 u;
	struct nixx_af_lfx_rqs_base_s {
		u64 reserved_0_6                     : 7;
		u64 addr                             : 46;
		u64 reserved_53_63                   : 11;
	} s;
	/* struct nixx_af_lfx_rqs_base_s cn; */
};

static inline u64 NIXX_AF_LFX_RQS_BASE(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_LFX_RQS_BASE(u64 a)
{
	return 0x4050 + 0x20000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_lf#_rqs_cfg
 *
 * NIX AF Local Function Receive Queues Configuration Register This
 * register configures receive queues in the LF.
 */
union nixx_af_lfx_rqs_cfg {
	u64 u;
	struct nixx_af_lfx_rqs_cfg_s {
		u64 max_queuesm1                     : 20;
		u64 way_mask                         : 16;
		u64 caching                          : 1;
		u64 reserved_37_63                   : 27;
	} s;
	/* struct nixx_af_lfx_rqs_cfg_s cn; */
};

static inline u64 NIXX_AF_LFX_RQS_CFG(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_LFX_RQS_CFG(u64 a)
{
	return 0x4040 + 0x20000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_lf#_rss_base
 *
 * NIX AF Local Function Receive Size Scaling Table Base Address Register
 * This register specifies the base AF IOVA of the RSS table per LF. The
 * table is present when NIX_AF_LF()_RSS_CFG[ENA] is set and consists of
 * 1 \<\< (NIX_AF_LF()_RSS_CFG[SIZE] + 8) contiguous NIX_RSSE_S
 * structures, where the size of each structure is 1 \<\<
 * NIX_AF_CONST3[RSSE_LOG2BYTES]. See NIX_AF_LF()_RSS_GRP().
 */
union nixx_af_lfx_rss_base {
	u64 u;
	struct nixx_af_lfx_rss_base_s {
		u64 reserved_0_6                     : 7;
		u64 addr                             : 46;
		u64 reserved_53_63                   : 11;
	} s;
	/* struct nixx_af_lfx_rss_base_s cn; */
};

static inline u64 NIXX_AF_LFX_RSS_BASE(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_LFX_RSS_BASE(u64 a)
{
	return 0x40d0 + 0x20000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_lf#_rss_cfg
 *
 * NIX AF Local Function Receive Size Scaling Table Configuration
 * Register See NIX_AF_LF()_RSS_BASE and NIX_AF_LF()_RSS_GRP().
 */
union nixx_af_lfx_rss_cfg {
	u64 u;
	struct nixx_af_lfx_rss_cfg_s {
		u64 size                             : 4;
		u64 ena                              : 1;
		u64 adder_is_tag_lsb                 : 1;
		u64 reserved_6_19                    : 14;
		u64 way_mask                         : 16;
		u64 caching                          : 1;
		u64 reserved_37_63                   : 27;
	} s;
	struct nixx_af_lfx_rss_cfg_cn96xxp1 {
		u64 size                             : 4;
		u64 ena                              : 1;
		u64 reserved_5_19                    : 15;
		u64 way_mask                         : 16;
		u64 caching                          : 1;
		u64 reserved_37_63                   : 27;
	} cn96xxp1;
	/* struct nixx_af_lfx_rss_cfg_s cn96xxp3; */
	/* struct nixx_af_lfx_rss_cfg_cn96xxp1 cnf95xx; */
};

static inline u64 NIXX_AF_LFX_RSS_CFG(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_LFX_RSS_CFG(u64 a)
{
	return 0x40c0 + 0x20000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_lf#_rss_grp#
 *
 * NIX AF Local Function Receive Side Scaling Group Registers A receive
 * packet targets a LF's RSS group when its NIX_RX_ACTION_S[OP] =
 * NIX_RX_ACTIONOP_E::RSS, or its target multicast list has an entry with
 * NIX_RX_MCE_S[OP] = NIX_RX_MCOP_E::RSS. The RSS group index (this
 * register's last index) is NIX_RX_ACTION_S[INDEX] or
 * NIX_RX_MCE_S[INDEX].  The RSS computation is as follows: * The
 * packet's flow_tag (see NIX_LF_RX_SECRET()) and RSS group are used to
 * select a NIX_RSSE_S entry in the LF's RSS table (see [SIZEM1]). *
 * NIX_RSSE_S selects the packet's destination RQ.
 */
union nixx_af_lfx_rss_grpx {
	u64 u;
	struct nixx_af_lfx_rss_grpx_s {
		u64 offset                           : 11;
		u64 reserved_11_15                   : 5;
		u64 sizem1                           : 3;
		u64 reserved_19_63                   : 45;
	} s;
	/* struct nixx_af_lfx_rss_grpx_s cn; */
};

static inline u64 NIXX_AF_LFX_RSS_GRPX(u64 a, u64 b)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_LFX_RSS_GRPX(u64 a, u64 b)
{
	return 0x4600 + 0x20000 * a + 8 * b;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_lf#_rx_cfg
 *
 * NIX AF Local Function Receive Configuration Register
 */
union nixx_af_lfx_rx_cfg {
	u64 u;
	struct nixx_af_lfx_rx_cfg_s {
		u64 reserved_0_31                    : 32;
		u64 drop_re                          : 1;
		u64 lenerr_en                        : 1;
		u64 ip6_udp_opt                      : 1;
		u64 dis_apad                         : 1;
		u64 csum_il4                         : 1;
		u64 csum_ol4                         : 1;
		u64 len_il4                          : 1;
		u64 len_il3                          : 1;
		u64 len_ol4                          : 1;
		u64 len_ol3                          : 1;
		u64 reserved_42_63                   : 22;
	} s;
	struct nixx_af_lfx_rx_cfg_cn96xxp1 {
		u64 reserved_0_31                    : 32;
		u64 reserved_32                      : 1;
		u64 lenerr_en                        : 1;
		u64 ip6_udp_opt                      : 1;
		u64 dis_apad                         : 1;
		u64 csum_il4                         : 1;
		u64 csum_ol4                         : 1;
		u64 len_il4                          : 1;
		u64 len_il3                          : 1;
		u64 len_ol4                          : 1;
		u64 len_ol3                          : 1;
		u64 reserved_42_63                   : 22;
	} cn96xxp1;
	/* struct nixx_af_lfx_rx_cfg_s cn96xxp3; */
	/* struct nixx_af_lfx_rx_cfg_s cnf95xx; */
};

static inline u64 NIXX_AF_LFX_RX_CFG(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_LFX_RX_CFG(u64 a)
{
	return 0x40a0 + 0x20000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_lf#_rx_ipsec_cfg0
 *
 * INTERNAL: NIX AF LF Receive IPSEC Configuration Registers  Internal:
 * Not used; no IPSEC fast-path.
 */
union nixx_af_lfx_rx_ipsec_cfg0 {
	u64 u;
	struct nixx_af_lfx_rx_ipsec_cfg0_s {
		u64 lenm1_max                        : 14;
		u64 reserved_14_15                   : 2;
		u64 sa_pow2_size                     : 4;
		u64 tag_const                        : 24;
		u64 tt                               : 2;
		u64 defcpt                           : 1;
		u64 hshcpt                           : 1;
		u64 reserved_48_63                   : 16;
	} s;
	/* struct nixx_af_lfx_rx_ipsec_cfg0_s cn; */
};

static inline u64 NIXX_AF_LFX_RX_IPSEC_CFG0(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_LFX_RX_IPSEC_CFG0(u64 a)
{
	return 0x4140 + 0x20000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_lf#_rx_ipsec_cfg1
 *
 * INTERNAL: NIX AF LF Receive IPSEC Security Association Configuration
 * Register  Internal: Not used; no IPSEC fast-path.
 */
union nixx_af_lfx_rx_ipsec_cfg1 {
	u64 u;
	struct nixx_af_lfx_rx_ipsec_cfg1_s {
		u64 sa_idx_max                       : 32;
		u64 sa_idx_w                         : 5;
		u64 reserved_37_63                   : 27;
	} s;
	/* struct nixx_af_lfx_rx_ipsec_cfg1_s cn; */
};

static inline u64 NIXX_AF_LFX_RX_IPSEC_CFG1(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_LFX_RX_IPSEC_CFG1(u64 a)
{
	return 0x4148 + 0x20000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_lf#_rx_ipsec_dyno_base
 *
 * INTERNAL: NIX AF LF Receive IPSEC Dynamic Ordering Base Address
 * Registers  Internal: Not used; no IPSEC fast-path.
 */
union nixx_af_lfx_rx_ipsec_dyno_base {
	u64 u;
	struct nixx_af_lfx_rx_ipsec_dyno_base_s {
		u64 reserved_0_6                     : 7;
		u64 addr                             : 46;
		u64 reserved_53_63                   : 11;
	} s;
	/* struct nixx_af_lfx_rx_ipsec_dyno_base_s cn; */
};

static inline u64 NIXX_AF_LFX_RX_IPSEC_DYNO_BASE(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_LFX_RX_IPSEC_DYNO_BASE(u64 a)
{
	return 0x4158 + 0x20000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_lf#_rx_ipsec_dyno_cfg
 *
 * INTERNAL: NIX AF LF Receive IPSEC Dynamic Ordering Base Address
 * Registers  Internal: Not used; no IPSEC fast-path.
 */
union nixx_af_lfx_rx_ipsec_dyno_cfg {
	u64 u;
	struct nixx_af_lfx_rx_ipsec_dyno_cfg_s {
		u64 dyno_idx_w                       : 4;
		u64 dyno_ena                         : 1;
		u64 reserved_5_19                    : 15;
		u64 way_mask                         : 16;
		u64 caching                          : 1;
		u64 reserved_37_63                   : 27;
	} s;
	/* struct nixx_af_lfx_rx_ipsec_dyno_cfg_s cn; */
};

static inline u64 NIXX_AF_LFX_RX_IPSEC_DYNO_CFG(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_LFX_RX_IPSEC_DYNO_CFG(u64 a)
{
	return 0x4150 + 0x20000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_lf#_rx_ipsec_sa_base
 *
 * INTERNAL: NIX AF LF Receive IPSEC Security Association Base Address
 * Register  Internal: Not used; no IPSEC fast-path.
 */
union nixx_af_lfx_rx_ipsec_sa_base {
	u64 u;
	struct nixx_af_lfx_rx_ipsec_sa_base_s {
		u64 reserved_0_6                     : 7;
		u64 addr                             : 46;
		u64 reserved_53_63                   : 11;
	} s;
	/* struct nixx_af_lfx_rx_ipsec_sa_base_s cn; */
};

static inline u64 NIXX_AF_LFX_RX_IPSEC_SA_BASE(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_LFX_RX_IPSEC_SA_BASE(u64 a)
{
	return 0x4170 + 0x20000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_lf#_rx_stat#
 *
 * NIX AF Local Function Receive Statistics Registers The last dimension
 * indicates which statistic, and is enumerated by NIX_STAT_LF_RX_E.
 */
union nixx_af_lfx_rx_statx {
	u64 u;
	struct nixx_af_lfx_rx_statx_s {
		u64 stat                             : 48;
		u64 reserved_48_63                   : 16;
	} s;
	/* struct nixx_af_lfx_rx_statx_s cn; */
};

static inline u64 NIXX_AF_LFX_RX_STATX(u64 a, u64 b)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_LFX_RX_STATX(u64 a, u64 b)
{
	return 0x4500 + 0x20000 * a + 8 * b;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_lf#_rx_vtag_type#
 *
 * NIX AF Local Function Receive Vtag Type Registers These registers
 * specify optional Vtag (e.g. VLAN, E-TAG) actions for received packets.
 * Indexed by NIX_RX_VTAG_ACTION_S[VTAG*_TYPE].
 */
union nixx_af_lfx_rx_vtag_typex {
	u64 u;
	struct nixx_af_lfx_rx_vtag_typex_s {
		u64 size                             : 1;
		u64 reserved_1_3                     : 3;
		u64 strip                            : 1;
		u64 capture                          : 1;
		u64 reserved_6_63                    : 58;
	} s;
	/* struct nixx_af_lfx_rx_vtag_typex_s cn; */
};

static inline u64 NIXX_AF_LFX_RX_VTAG_TYPEX(u64 a, u64 b)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_LFX_RX_VTAG_TYPEX(u64 a, u64 b)
{
	return 0x4200 + 0x20000 * a + 8 * b;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_lf#_sqs_base
 *
 * NIX AF Local Function Send Queues Base Address Register This register
 * specifies the base AF IOVA of the LF's SQ context table. The table
 * consists of NIX_AF_LF()_SQS_CFG[MAX_QUEUESM1]+1 contiguous
 * NIX_SQ_CTX_HW_S structures.
 */
union nixx_af_lfx_sqs_base {
	u64 u;
	struct nixx_af_lfx_sqs_base_s {
		u64 reserved_0_6                     : 7;
		u64 addr                             : 46;
		u64 reserved_53_63                   : 11;
	} s;
	/* struct nixx_af_lfx_sqs_base_s cn; */
};

static inline u64 NIXX_AF_LFX_SQS_BASE(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_LFX_SQS_BASE(u64 a)
{
	return 0x4030 + 0x20000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_lf#_sqs_cfg
 *
 * NIX AF Local Function Send Queues Configuration Register This register
 * configures send queues in the LF.
 */
union nixx_af_lfx_sqs_cfg {
	u64 u;
	struct nixx_af_lfx_sqs_cfg_s {
		u64 max_queuesm1                     : 20;
		u64 way_mask                         : 16;
		u64 caching                          : 1;
		u64 reserved_37_63                   : 27;
	} s;
	/* struct nixx_af_lfx_sqs_cfg_s cn; */
};

static inline u64 NIXX_AF_LFX_SQS_CFG(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_LFX_SQS_CFG(u64 a)
{
	return 0x4020 + 0x20000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_lf#_tx_cfg
 *
 * NIX AF Local Function Transmit Configuration Register
 */
union nixx_af_lfx_tx_cfg {
	u64 u;
	struct nixx_af_lfx_tx_cfg_s {
		u64 vlan0_ins_etype                  : 16;
		u64 vlan1_ins_etype                  : 16;
		u64 send_tstmp_ena                   : 1;
		u64 lock_viol_cqe_ena                : 1;
		u64 lock_ena                         : 1;
		u64 reserved_35_63                   : 29;
	} s;
	/* struct nixx_af_lfx_tx_cfg_s cn; */
};

static inline u64 NIXX_AF_LFX_TX_CFG(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_LFX_TX_CFG(u64 a)
{
	return 0x4080 + 0x20000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_lf#_tx_cfg2
 *
 * NIX AF Local Function Transmit Configuration Register
 */
union nixx_af_lfx_tx_cfg2 {
	u64 u;
	struct nixx_af_lfx_tx_cfg2_s {
		u64 lmt_ena                          : 1;
		u64 reserved_1_63                    : 63;
	} s;
	/* struct nixx_af_lfx_tx_cfg2_s cn; */
};

static inline u64 NIXX_AF_LFX_TX_CFG2(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_LFX_TX_CFG2(u64 a)
{
	return 0x4028 + 0x20000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_lf#_tx_parse_cfg
 *
 * NIX AF Local Function Transmit Parse Configuration Register
 */
union nixx_af_lfx_tx_parse_cfg {
	u64 u;
	struct nixx_af_lfx_tx_parse_cfg_s {
		u64 pkind                            : 6;
		u64 reserved_6_63                    : 58;
	} s;
	/* struct nixx_af_lfx_tx_parse_cfg_s cn; */
};

static inline u64 NIXX_AF_LFX_TX_PARSE_CFG(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_LFX_TX_PARSE_CFG(u64 a)
{
	return 0x4090 + 0x20000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_lf#_tx_stat#
 *
 * NIX AF Local Function Transmit Statistics Registers The last dimension
 * indicates which statistic, and is enumerated by NIX_STAT_LF_TX_E.
 */
union nixx_af_lfx_tx_statx {
	u64 u;
	struct nixx_af_lfx_tx_statx_s {
		u64 stat                             : 48;
		u64 reserved_48_63                   : 16;
	} s;
	/* struct nixx_af_lfx_tx_statx_s cn; */
};

static inline u64 NIXX_AF_LFX_TX_STATX(u64 a, u64 b)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_LFX_TX_STATX(u64 a, u64 b)
{
	return 0x4400 + 0x20000 * a + 8 * b;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_lf#_tx_status
 *
 * NIX AF LF Transmit Status Register
 */
union nixx_af_lfx_tx_status {
	u64 u;
	struct nixx_af_lfx_tx_status_s {
		u64 sq_ctx_err                       : 1;
		u64 reserved_1_63                    : 63;
	} s;
	/* struct nixx_af_lfx_tx_status_s cn; */
};

static inline u64 NIXX_AF_LFX_TX_STATUS(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_LFX_TX_STATUS(u64 a)
{
	return 0x4180 + 0x20000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_lf_rst
 *
 * NIX Admin Function LF Reset Register
 */
union nixx_af_lf_rst {
	u64 u;
	struct nixx_af_lf_rst_s {
		u64 lf                               : 8;
		u64 reserved_8_11                    : 4;
		u64 exec                             : 1;
		u64 reserved_13_63                   : 51;
	} s;
	/* struct nixx_af_lf_rst_s cn; */
};

static inline u64 NIXX_AF_LF_RST(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_LF_RST(void)
{
	return 0x150;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_lso_cfg
 *
 * NIX AF Large Send Offload Configuration Register
 */
union nixx_af_lso_cfg {
	u64 u;
	struct nixx_af_lso_cfg_s {
		u64 tcp_lsf                          : 16;
		u64 tcp_msf                          : 16;
		u64 tcp_fsf                          : 16;
		u64 reserved_48_62                   : 15;
		u64 enable                           : 1;
	} s;
	/* struct nixx_af_lso_cfg_s cn; */
};

static inline u64 NIXX_AF_LSO_CFG(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_LSO_CFG(void)
{
	return 0xa8;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_lso_format#_field#
 *
 * NIX AF Large Send Offload Format Field Registers These registers
 * specify LSO packet modification formats. Each format may modify up to
 * eight packet fields with the following constraints: * If fewer than
 * eight fields are modified, [ALG] must be NIX_LSOALG_E::NOP in the
 * unused field registers. * Modified fields must be specified in
 * contiguous field registers starting with NIX_AF_LSO_FORMAT()_FIELD(0).
 * * Modified fields cannot overlap. * Multiple fields with the same
 * [LAYER] value must be specified in ascending [OFFSET] order. * Fields
 * in different layers must be specified in ascending [LAYER] order.
 */
union nixx_af_lso_formatx_fieldx {
	u64 u;
	struct nixx_af_lso_formatx_fieldx_s {
		u64 offset                           : 8;
		u64 layer                            : 2;
		u64 reserved_10_11                   : 2;
		u64 sizem1                           : 2;
		u64 reserved_14_15                   : 2;
		u64 alg                              : 3;
		u64 reserved_19_63                   : 45;
	} s;
	/* struct nixx_af_lso_formatx_fieldx_s cn; */
};

static inline u64 NIXX_AF_LSO_FORMATX_FIELDX(u64 a, u64 b)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_LSO_FORMATX_FIELDX(u64 a, u64 b)
{
	return 0x1b00 + 0x10000 * a + 8 * b;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_mark_format#_ctl
 *
 * NIX AF Packet Marking Format Registers Describes packet marking
 * calculations for YELLOW and for NIX_COLORRESULT_E::RED_SEND packets.
 * NIX_SEND_EXT_S[MARKFORM] selects the CSR used for the packet
 * descriptor.  All the packet marking offset calculations assume big-
 * endian bits within a byte.  For example, if NIX_SEND_EXT_S[MARKPTR] is
 * 3 and [OFFSET] is 5 and the packet is YELLOW, the NIX marking hardware
 * would do this:  _  byte[3]\<2:0\> |=   [Y_VAL]\<3:1\> _
 * byte[3]\<2:0\> &= ~[Y_MASK]\<3:1\> _  byte[4]\<7\>   |=   [Y_VAL]\<0\>
 * _  byte[4]\<7\>   &= ~[Y_MASK]\<0\>  where byte[3] is the third byte
 * in the packet, and byte[4] the fourth.  For another example, if
 * NIX_SEND_EXT_S[MARKPTR] is 3 and [OFFSET] is 0 and the packet is
 * NIX_COLORRESULT_E::RED_SEND,  _   byte[3]\<7:4\> |=   [R_VAL]\<3:0\> _
 * byte[3]\<7:4\> &= ~[R_MASK]\<3:0\>
 */
union nixx_af_mark_formatx_ctl {
	u64 u;
	struct nixx_af_mark_formatx_ctl_s {
		u64 r_val                            : 4;
		u64 r_mask                           : 4;
		u64 y_val                            : 4;
		u64 y_mask                           : 4;
		u64 offset                           : 3;
		u64 reserved_19_63                   : 45;
	} s;
	/* struct nixx_af_mark_formatx_ctl_s cn; */
};

static inline u64 NIXX_AF_MARK_FORMATX_CTL(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_MARK_FORMATX_CTL(u64 a)
{
	return 0x900 + 0x40000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_mc_mirror_const
 *
 * NIX AF Multicast/Mirror Constants Register This register contains
 * constants for software discovery.
 */
union nixx_af_mc_mirror_const {
	u64 u;
	struct nixx_af_mc_mirror_const_s {
		u64 buf_size                         : 16;
		u64 reserved_16_63                   : 48;
	} s;
	/* struct nixx_af_mc_mirror_const_s cn; */
};

static inline u64 NIXX_AF_MC_MIRROR_CONST(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_MC_MIRROR_CONST(void)
{
	return 0x98;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_mdq#_cir
 *
 * NIX AF Meta Descriptor Queue Committed Information Rate Registers This
 * register has the same bit fields as NIX_AF_TL1()_CIR.
 */
union nixx_af_mdqx_cir {
	u64 u;
	struct nixx_af_mdqx_cir_s {
		u64 enable                           : 1;
		u64 rate_mantissa                    : 8;
		u64 rate_exponent                    : 4;
		u64 rate_divider_exponent            : 4;
		u64 reserved_17_28                   : 12;
		u64 burst_mantissa                   : 8;
		u64 burst_exponent                   : 4;
		u64 reserved_41_63                   : 23;
	} s;
	/* struct nixx_af_mdqx_cir_s cn; */
};

static inline u64 NIXX_AF_MDQX_CIR(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_MDQX_CIR(u64 a)
{
	return 0x1420 + 0x10000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_mdq#_md_debug
 *
 * NIX AF Meta Descriptor Queue Meta Descriptor State Debug Registers
 * This register provides access to the meta descriptor at the front of
 * the MDQ. An MDQ can hold up to 8 packet meta descriptors (PMD) and one
 * flush meta descriptor (FMD).
 */
union nixx_af_mdqx_md_debug {
	u64 u;
	struct nixx_af_mdqx_md_debug_s {
		u64 pkt_len                          : 16;
		u64 red_algo_override                : 2;
		u64 shp_dis                          : 1;
		u64 reserved_19                      : 1;
		u64 shp_chg                          : 9;
		u64 reserved_29_31                   : 3;
		u64 sqm_pkt_id                       : 13;
		u64 reserved_45_60                   : 16;
		u64 md_type                          : 2;
		u64 reserved_63                      : 1;
	} s;
	/* struct nixx_af_mdqx_md_debug_s cn; */
};

static inline u64 NIXX_AF_MDQX_MD_DEBUG(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_MDQX_MD_DEBUG(u64 a)
{
	return 0x14c0 + 0x10000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_mdq#_parent
 *
 * NIX AF Meta Descriptor Queue Topology Registers
 */
union nixx_af_mdqx_parent {
	u64 u;
	struct nixx_af_mdqx_parent_s {
		u64 reserved_0_15                    : 16;
		u64 parent                           : 9;
		u64 reserved_25_63                   : 39;
	} s;
	/* struct nixx_af_mdqx_parent_s cn; */
};

static inline u64 NIXX_AF_MDQX_PARENT(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_MDQX_PARENT(u64 a)
{
	return 0x1480 + 0x10000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_mdq#_pir
 *
 * NIX AF Meta Descriptor Queue Peak Information Rate Registers This
 * register has the same bit fields as NIX_AF_TL1()_CIR.
 */
union nixx_af_mdqx_pir {
	u64 u;
	struct nixx_af_mdqx_pir_s {
		u64 enable                           : 1;
		u64 rate_mantissa                    : 8;
		u64 rate_exponent                    : 4;
		u64 rate_divider_exponent            : 4;
		u64 reserved_17_28                   : 12;
		u64 burst_mantissa                   : 8;
		u64 burst_exponent                   : 4;
		u64 reserved_41_63                   : 23;
	} s;
	/* struct nixx_af_mdqx_pir_s cn; */
};

static inline u64 NIXX_AF_MDQX_PIR(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_MDQX_PIR(u64 a)
{
	return 0x1430 + 0x10000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_mdq#_pointers
 *
 * INTERNAL: NIX AF Meta Descriptor 4 Linked List Pointers Debug Register
 * This register has the same bit fields as NIX_AF_TL4()_POINTERS.
 */
union nixx_af_mdqx_pointers {
	u64 u;
	struct nixx_af_mdqx_pointers_s {
		u64 next                             : 9;
		u64 reserved_9_15                    : 7;
		u64 prev                             : 9;
		u64 reserved_25_63                   : 39;
	} s;
	/* struct nixx_af_mdqx_pointers_s cn; */
};

static inline u64 NIXX_AF_MDQX_POINTERS(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_MDQX_POINTERS(u64 a)
{
	return 0x1460 + 0x10000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_mdq#_ptr_fifo
 *
 * INTERNAL: NIX Meta Descriptor Queue Pointer FIFO State Debug Registers
 */
union nixx_af_mdqx_ptr_fifo {
	u64 u;
	struct nixx_af_mdqx_ptr_fifo_s {
		u64 tail                             : 4;
		u64 head                             : 4;
		u64 p_con                            : 1;
		u64 reserved_9_63                    : 55;
	} s;
	/* struct nixx_af_mdqx_ptr_fifo_s cn; */
};

static inline u64 NIXX_AF_MDQX_PTR_FIFO(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_MDQX_PTR_FIFO(u64 a)
{
	return 0x14d0 + 0x10000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_mdq#_sched_state
 *
 * NIX AF Meta Descriptor Queue Scheduling Control State Registers This
 * register has the same bit fields as NIX_AF_TL2()_SCHED_STATE.
 */
union nixx_af_mdqx_sched_state {
	u64 u;
	struct nixx_af_mdqx_sched_state_s {
		u64 rr_count                         : 25;
		u64 reserved_25_63                   : 39;
	} s;
	/* struct nixx_af_mdqx_sched_state_s cn; */
};

static inline u64 NIXX_AF_MDQX_SCHED_STATE(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_MDQX_SCHED_STATE(u64 a)
{
	return 0x1440 + 0x10000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_mdq#_schedule
 *
 * NIX AF Meta Descriptor Queue Scheduling Control Registers This
 * register has the same bit fields as NIX_AF_TL2()_SCHEDULE.
 */
union nixx_af_mdqx_schedule {
	u64 u;
	struct nixx_af_mdqx_schedule_s {
		u64 rr_quantum                       : 24;
		u64 prio                             : 4;
		u64 reserved_28_63                   : 36;
	} s;
	/* struct nixx_af_mdqx_schedule_s cn; */
};

static inline u64 NIXX_AF_MDQX_SCHEDULE(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_MDQX_SCHEDULE(u64 a)
{
	return 0x1400 + 0x10000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_mdq#_shape
 *
 * NIX AF Meta Descriptor Queue Shaping Control Registers This register
 * has the same bit fields as NIX_AF_TL3()_SHAPE.
 */
union nixx_af_mdqx_shape {
	u64 u;
	struct nixx_af_mdqx_shape_s {
		u64 adjust                           : 9;
		u64 red_algo                         : 2;
		u64 red_disable                      : 1;
		u64 yellow_disable                   : 1;
		u64 reserved_13_23                   : 11;
		u64 length_disable                   : 1;
		u64 schedule_list                    : 2;
		u64 reserved_27_63                   : 37;
	} s;
	/* struct nixx_af_mdqx_shape_s cn; */
};

static inline u64 NIXX_AF_MDQX_SHAPE(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_MDQX_SHAPE(u64 a)
{
	return 0x1410 + 0x10000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_mdq#_shape_state
 *
 * NIX AF Meta Descriptor Queue Shaping State Registers This register has
 * the same bit fields as NIX_AF_TL2()_SHAPE_STATE. This register must
 * not be written during normal operation.
 */
union nixx_af_mdqx_shape_state {
	u64 u;
	struct nixx_af_mdqx_shape_state_s {
		u64 cir_accum                        : 26;
		u64 pir_accum                        : 26;
		u64 color                            : 2;
		u64 reserved_54_63                   : 10;
	} s;
	/* struct nixx_af_mdqx_shape_state_s cn; */
};

static inline u64 NIXX_AF_MDQX_SHAPE_STATE(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_MDQX_SHAPE_STATE(u64 a)
{
	return 0x1450 + 0x10000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_mdq#_sw_xoff
 *
 * NIX AF Meta Descriptor Controlled XOFF Registers This register has the
 * same bit fields as NIX_AF_TL1()_SW_XOFF
 */
union nixx_af_mdqx_sw_xoff {
	u64 u;
	struct nixx_af_mdqx_sw_xoff_s {
		u64 xoff                             : 1;
		u64 drain                            : 1;
		u64 reserved_2                       : 1;
		u64 drain_irq                        : 1;
		u64 reserved_4_63                    : 60;
	} s;
	/* struct nixx_af_mdqx_sw_xoff_s cn; */
};

static inline u64 NIXX_AF_MDQX_SW_XOFF(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_MDQX_SW_XOFF(u64 a)
{
	return 0x1470 + 0x10000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_mdq_const
 *
 * NIX AF Meta Descriptor Queue Constants Register This register contains
 * constants for software discovery.
 */
union nixx_af_mdq_const {
	u64 u;
	struct nixx_af_mdq_const_s {
		u64 count                            : 16;
		u64 reserved_16_63                   : 48;
	} s;
	/* struct nixx_af_mdq_const_s cn; */
};

static inline u64 NIXX_AF_MDQ_CONST(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_MDQ_CONST(void)
{
	return 0x90;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_ndc_cfg
 *
 * NIX AF General Configuration Register
 */
union nixx_af_ndc_cfg {
	u64 u;
	struct nixx_af_ndc_cfg_s {
		u64 ndc_ign_pois                     : 1;
		u64 byp_sq                           : 1;
		u64 byp_sqb                          : 1;
		u64 byp_cqs                          : 1;
		u64 byp_cints                        : 1;
		u64 byp_dyno                         : 1;
		u64 byp_mce                          : 1;
		u64 byp_rqc                          : 1;
		u64 byp_rsse                         : 1;
		u64 byp_mc_data                      : 1;
		u64 byp_mc_wqe                       : 1;
		u64 byp_mr_data                      : 1;
		u64 byp_mr_wqe                       : 1;
		u64 byp_qints                        : 1;
		u64 reserved_14_63                   : 50;
	} s;
	/* struct nixx_af_ndc_cfg_s cn; */
};

static inline u64 NIXX_AF_NDC_CFG(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_NDC_CFG(void)
{
	return 0x18;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_ndc_rx_sync
 *
 * NIX AF Receive NDC Sync Register Used to synchronize the NIX receive
 * NDC (NDC_IDX_E::NIX()_RX).
 */
union nixx_af_ndc_rx_sync {
	u64 u;
	struct nixx_af_ndc_rx_sync_s {
		u64 lf                               : 8;
		u64 reserved_8_11                    : 4;
		u64 exec                             : 1;
		u64 reserved_13_63                   : 51;
	} s;
	/* struct nixx_af_ndc_rx_sync_s cn; */
};

static inline u64 NIXX_AF_NDC_RX_SYNC(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_NDC_RX_SYNC(void)
{
	return 0x3e0;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_ndc_tx_sync
 *
 * NIX AF NDC_TX Sync Register Used to synchronize the NIX transmit NDC
 * (NDC_IDX_E::NIX()_TX).
 */
union nixx_af_ndc_tx_sync {
	u64 u;
	struct nixx_af_ndc_tx_sync_s {
		u64 lf                               : 8;
		u64 reserved_8_11                    : 4;
		u64 exec                             : 1;
		u64 reserved_13_63                   : 51;
	} s;
	/* struct nixx_af_ndc_tx_sync_s cn; */
};

static inline u64 NIXX_AF_NDC_TX_SYNC(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_NDC_TX_SYNC(void)
{
	return 0x3f0;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_norm_tx_fifo_status
 *
 * NIX AF Normal Transmit FIFO Status Register Status of FIFO which
 * transmits normal packets to CGX and LBK.
 */
union nixx_af_norm_tx_fifo_status {
	u64 u;
	struct nixx_af_norm_tx_fifo_status_s {
		u64 count                            : 12;
		u64 reserved_12_63                   : 52;
	} s;
	/* struct nixx_af_norm_tx_fifo_status_s cn; */
};

static inline u64 NIXX_AF_NORM_TX_FIFO_STATUS(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_NORM_TX_FIFO_STATUS(void)
{
	return 0x648;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_pq#_dbg_arb_link_exp
 *
 * INTERNAL: NIX AF PQ Arb Link EXPRESS Debug Register
 */
union nixx_af_pqx_dbg_arb_link_exp {
	u64 u;
	struct nixx_af_pqx_dbg_arb_link_exp_s {
		u64 req                              : 1;
		u64 act_c_con                        : 1;
		u64 cnt                              : 2;
		u64 reserved_4_5                     : 2;
		u64 rr_mask                          : 1;
		u64 reserved_7_63                    : 57;
	} s;
	/* struct nixx_af_pqx_dbg_arb_link_exp_s cn; */
};

static inline u64 NIXX_AF_PQX_DBG_ARB_LINK_EXP(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_PQX_DBG_ARB_LINK_EXP(u64 a)
{
	return 0xce8 + 0x10000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_pq#_dbg_arb_link_nrm
 *
 * INTERNAL: NIX AF PQ Arb Link NORMAL Debug Register
 */
union nixx_af_pqx_dbg_arb_link_nrm {
	u64 u;
	struct nixx_af_pqx_dbg_arb_link_nrm_s {
		u64 req                              : 1;
		u64 act_c_con                        : 1;
		u64 cnt                              : 2;
		u64 reserved_4_5                     : 2;
		u64 rr_mask                          : 1;
		u64 reserved_7_63                    : 57;
	} s;
	/* struct nixx_af_pqx_dbg_arb_link_nrm_s cn; */
};

static inline u64 NIXX_AF_PQX_DBG_ARB_LINK_NRM(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_PQX_DBG_ARB_LINK_NRM(u64 a)
{
	return 0xce0 + 0x10000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_pq#_dbg_arb_link_sdp
 *
 * INTERNAL: NIX AF PQ Arb Link SDP Debug Register
 */
union nixx_af_pqx_dbg_arb_link_sdp {
	u64 u;
	struct nixx_af_pqx_dbg_arb_link_sdp_s {
		u64 req                              : 1;
		u64 act_c_con                        : 1;
		u64 cnt                              : 2;
		u64 reserved_4_5                     : 2;
		u64 rr_mask                          : 1;
		u64 reserved_7_63                    : 57;
	} s;
	/* struct nixx_af_pqx_dbg_arb_link_sdp_s cn; */
};

static inline u64 NIXX_AF_PQX_DBG_ARB_LINK_SDP(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_PQX_DBG_ARB_LINK_SDP(u64 a)
{
	return 0xcf0 + 0x10000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_pq_arb_crd_rdy_debug
 *
 * INTERNAL: NIX AF PQ_ARB Node Credit Ready Registers  NIX AF PQ ARB
 * Credit ready register
 */
union nixx_af_pq_arb_crd_rdy_debug {
	u64 u;
	struct nixx_af_pq_arb_crd_rdy_debug_s {
		u64 node_crd_rdy                     : 28;
		u64 reserved_28_63                   : 36;
	} s;
	/* struct nixx_af_pq_arb_crd_rdy_debug_s cn; */
};

static inline u64 NIXX_AF_PQ_ARB_CRD_RDY_DEBUG(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_PQ_ARB_CRD_RDY_DEBUG(void)
{
	return 0xf10;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_pq_arb_dwrr_msk_debug
 *
 * INTERNAL: NIX AF PQ_ARB DWRR mask set read only debug Registers
 */
union nixx_af_pq_arb_dwrr_msk_debug {
	u64 u;
	struct nixx_af_pq_arb_dwrr_msk_debug_s {
		u64 node_dwrr_mask_set               : 28;
		u64 reserved_28_63                   : 36;
	} s;
	/* struct nixx_af_pq_arb_dwrr_msk_debug_s cn; */
};

static inline u64 NIXX_AF_PQ_ARB_DWRR_MSK_DEBUG(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_PQ_ARB_DWRR_MSK_DEBUG(void)
{
	return 0xf30;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_pq_arb_node_gnt_debug
 *
 * INTERNAL: NIX AF PQ_ARB Node Grant vector Registers
 */
union nixx_af_pq_arb_node_gnt_debug {
	u64 u;
	struct nixx_af_pq_arb_node_gnt_debug_s {
		u64 node_grant_vec                   : 28;
		u64 reserved_28_63                   : 36;
	} s;
	/* struct nixx_af_pq_arb_node_gnt_debug_s cn; */
};

static inline u64 NIXX_AF_PQ_ARB_NODE_GNT_DEBUG(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_PQ_ARB_NODE_GNT_DEBUG(void)
{
	return 0xf20;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_pq_arb_node_req_debug
 *
 * INTERNAL: NIX AF PQ_ARB Node Request Debug Registers  NIX AF PQ ARB
 * Node Request Debug register
 */
union nixx_af_pq_arb_node_req_debug {
	u64 u;
	struct nixx_af_pq_arb_node_req_debug_s {
		u64 node_req                         : 28;
		u64 reserved_28_63                   : 36;
	} s;
	/* struct nixx_af_pq_arb_node_req_debug_s cn; */
};

static inline u64 NIXX_AF_PQ_ARB_NODE_REQ_DEBUG(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_PQ_ARB_NODE_REQ_DEBUG(void)
{
	return 0xf00;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_pq_arb_shape_vld_dbg
 *
 * INTERNAL: NIX AF PQ_ARB shape valid set Register
 */
union nixx_af_pq_arb_shape_vld_dbg {
	u64 u;
	struct nixx_af_pq_arb_shape_vld_dbg_s {
		u64 node_shape_vld_set               : 28;
		u64 reserved_28_63                   : 36;
	} s;
	/* struct nixx_af_pq_arb_shape_vld_dbg_s cn; */
};

static inline u64 NIXX_AF_PQ_ARB_SHAPE_VLD_DBG(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_PQ_ARB_SHAPE_VLD_DBG(void)
{
	return 0xf40;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_pq_dbg_arb_0
 *
 * INTERNAL: NIX AF PQ Arb Debug 0 Register
 */
union nixx_af_pq_dbg_arb_0 {
	u64 u;
	struct nixx_af_pq_dbg_arb_0_s {
		u64 rr_mask_clr                      : 1;
		u64 reserved_1_63                    : 63;
	} s;
	/* struct nixx_af_pq_dbg_arb_0_s cn; */
};

static inline u64 NIXX_AF_PQ_DBG_ARB_0(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_PQ_DBG_ARB_0(void)
{
	return 0xcf8;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_pq_lnk_#_dwrr_msk_dbg
 *
 * INTERNAL: NIX AF PQ_ARB Physical Link DWRR MASK Registers
 */
union nixx_af_pq_lnk_x_dwrr_msk_dbg {
	u64 u;
	struct nixx_af_pq_lnk_x_dwrr_msk_dbg_s {
		u64 link_dwrr_mask_set               : 28;
		u64 reserved_28_63                   : 36;
	} s;
	/* struct nixx_af_pq_lnk_x_dwrr_msk_dbg_s cn; */
};

static inline u64 NIXX_AF_PQ_LNK_X_DWRR_MSK_DBG(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_PQ_LNK_X_DWRR_MSK_DBG(u64 a)
{
	return 0x1100 + 0x10000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_pse_400_rate_divider
 *
 * INTERNAL: NIX AF PSE 400 Rate Divider Register
 */
union nixx_af_pse_400_rate_divider {
	u64 u;
	struct nixx_af_pse_400_rate_divider_s {
		u64 rate_div_cfg                     : 9;
		u64 reserved_9_63                    : 55;
	} s;
	/* struct nixx_af_pse_400_rate_divider_s cn; */
};

static inline u64 NIXX_AF_PSE_400_RATE_DIVIDER(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_PSE_400_RATE_DIVIDER(void)
{
	return 0x830;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_pse_active_cycles_pc
 *
 * NIX AF Active Cycles Register These registers are indexed by the
 * conditional clock domain number.
 */
union nixx_af_pse_active_cycles_pc {
	u64 u;
	struct nixx_af_pse_active_cycles_pc_s {
		u64 act_cyc                          : 64;
	} s;
	/* struct nixx_af_pse_active_cycles_pc_s cn; */
};

static inline u64 NIXX_AF_PSE_ACTIVE_CYCLES_PC(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_PSE_ACTIVE_CYCLES_PC(void)
{
	return 0x8c0;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_pse_bp_test0
 *
 * INTERNAL: NIX AF PSE Backpressure Test 0 Register
 */
union nixx_af_pse_bp_test0 {
	u64 u;
	struct nixx_af_pse_bp_test0_s {
		u64 lfsr_freq                        : 12;
		u64 reserved_12_63                   : 52;
	} s;
	struct nixx_af_pse_bp_test0_cn96xxp1 {
		u64 lfsr_freq                        : 12;
		u64 reserved_12_15                   : 4;
		u64 bp_cfg                           : 8;
		u64 reserved_24_59                   : 36;
		u64 enable                           : 4;
	} cn96xxp1;
	struct nixx_af_pse_bp_test0_cn96xxp3 {
		u64 lfsr_freq                        : 12;
		u64 reserved_12_15                   : 4;
		u64 reserved_16_19                   : 4;
		u64 bp_cfg                           : 12;
		u64 reserved_32_55                   : 24;
		u64 reserved_56_57                   : 2;
		u64 enable                           : 6;
	} cn96xxp3;
	/* struct nixx_af_pse_bp_test0_cn96xxp1 cnf95xxp1; */
	struct nixx_af_pse_bp_test0_cnf95xxp2 {
		u64 lfsr_freq                        : 12;
		u64 reserved_12_15                   : 4;
		u64 bp_cfg                           : 8;
		u64 reserved_24_31                   : 8;
		u64 reserved_32_55                   : 24;
		u64 reserved_56_59                   : 4;
		u64 enable                           : 4;
	} cnf95xxp2;
};

static inline u64 NIXX_AF_PSE_BP_TEST0(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_PSE_BP_TEST0(void)
{
	return 0x840;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_pse_bp_test1
 *
 * INTERNAL: NIX AF PSE Backpressure Test 1 Register
 */
union nixx_af_pse_bp_test1 {
	u64 u;
	struct nixx_af_pse_bp_test1_s {
		u64 lfsr_freq                        : 12;
		u64 reserved_12_15                   : 4;
		u64 bp_cfg                           : 10;
		u64 reserved_26_63                   : 38;
	} s;
	struct nixx_af_pse_bp_test1_cn96xxp1 {
		u64 lfsr_freq                        : 12;
		u64 reserved_12_15                   : 4;
		u64 bp_cfg                           : 8;
		u64 reserved_24_59                   : 36;
		u64 enable                           : 4;
	} cn96xxp1;
	struct nixx_af_pse_bp_test1_cn96xxp3 {
		u64 lfsr_freq                        : 12;
		u64 reserved_12_15                   : 4;
		u64 bp_cfg                           : 10;
		u64 reserved_26_31                   : 6;
		u64 reserved_32_55                   : 24;
		u64 reserved_56_58                   : 3;
		u64 enable                           : 5;
	} cn96xxp3;
	/* struct nixx_af_pse_bp_test1_cn96xxp1 cnf95xxp1; */
	struct nixx_af_pse_bp_test1_cnf95xxp2 {
		u64 lfsr_freq                        : 12;
		u64 reserved_12_15                   : 4;
		u64 bp_cfg                           : 8;
		u64 reserved_24_31                   : 8;
		u64 reserved_32_55                   : 24;
		u64 reserved_56_59                   : 4;
		u64 enable                           : 4;
	} cnf95xxp2;
};

static inline u64 NIXX_AF_PSE_BP_TEST1(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_PSE_BP_TEST1(void)
{
	return 0x850;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_pse_bp_test2
 *
 * INTERNAL: NIX AF PSE Backpressure Test 2 Register
 */
union nixx_af_pse_bp_test2 {
	u64 u;
	struct nixx_af_pse_bp_test2_s {
		u64 lfsr_freq                        : 12;
		u64 reserved_12_15                   : 4;
		u64 bp_cfg                           : 10;
		u64 reserved_26_63                   : 38;
	} s;
	struct nixx_af_pse_bp_test2_cn96xxp1 {
		u64 lfsr_freq                        : 12;
		u64 reserved_12_15                   : 4;
		u64 bp_cfg                           : 8;
		u64 reserved_24_59                   : 36;
		u64 enable                           : 4;
	} cn96xxp1;
	struct nixx_af_pse_bp_test2_cn96xxp3 {
		u64 lfsr_freq                        : 12;
		u64 reserved_12_15                   : 4;
		u64 bp_cfg                           : 10;
		u64 reserved_26_31                   : 6;
		u64 reserved_32_55                   : 24;
		u64 reserved_56_58                   : 3;
		u64 enable                           : 5;
	} cn96xxp3;
	/* struct nixx_af_pse_bp_test2_cn96xxp1 cnf95xxp1; */
	struct nixx_af_pse_bp_test2_cnf95xxp2 {
		u64 lfsr_freq                        : 12;
		u64 reserved_12_15                   : 4;
		u64 bp_cfg                           : 8;
		u64 reserved_24_31                   : 8;
		u64 reserved_32_55                   : 24;
		u64 reserved_56_59                   : 4;
		u64 enable                           : 4;
	} cnf95xxp2;
};

static inline u64 NIXX_AF_PSE_BP_TEST2(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_PSE_BP_TEST2(void)
{
	return 0x860;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_pse_bp_test3
 *
 * INTERNAL: NIX AF PSE Backpressure Test 3 Register
 */
union nixx_af_pse_bp_test3 {
	u64 u;
	struct nixx_af_pse_bp_test3_s {
		u64 lfsr_freq                        : 12;
		u64 reserved_12_15                   : 4;
		u64 bp_cfg                           : 10;
		u64 reserved_26_63                   : 38;
	} s;
	struct nixx_af_pse_bp_test3_cn96xxp1 {
		u64 lfsr_freq                        : 12;
		u64 reserved_12_15                   : 4;
		u64 bp_cfg                           : 8;
		u64 reserved_24_59                   : 36;
		u64 enable                           : 4;
	} cn96xxp1;
	struct nixx_af_pse_bp_test3_cn96xxp3 {
		u64 lfsr_freq                        : 12;
		u64 reserved_12_15                   : 4;
		u64 bp_cfg                           : 10;
		u64 reserved_26_31                   : 6;
		u64 reserved_32_55                   : 24;
		u64 reserved_56_58                   : 3;
		u64 enable                           : 5;
	} cn96xxp3;
	/* struct nixx_af_pse_bp_test3_cn96xxp1 cnf95xxp1; */
	struct nixx_af_pse_bp_test3_cnf95xxp2 {
		u64 lfsr_freq                        : 12;
		u64 reserved_12_15                   : 4;
		u64 bp_cfg                           : 8;
		u64 reserved_24_31                   : 8;
		u64 reserved_32_55                   : 24;
		u64 reserved_56_59                   : 4;
		u64 enable                           : 4;
	} cnf95xxp2;
};

static inline u64 NIXX_AF_PSE_BP_TEST3(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_PSE_BP_TEST3(void)
{
	return 0x870;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_pse_channel_level
 *
 * NIX AF PSE Channel Level Register
 */
union nixx_af_pse_channel_level {
	u64 u;
	struct nixx_af_pse_channel_level_s {
		u64 bp_level                         : 1;
		u64 reserved_1_63                    : 63;
	} s;
	/* struct nixx_af_pse_channel_level_s cn; */
};

static inline u64 NIXX_AF_PSE_CHANNEL_LEVEL(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_PSE_CHANNEL_LEVEL(void)
{
	return 0x800;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_pse_const
 *
 * NIX AF PSE Constants Register This register contains constants for
 * software discovery.
 */
union nixx_af_pse_const {
	u64 u;
	struct nixx_af_pse_const_s {
		u64 levels                           : 4;
		u64 reserved_4_7                     : 4;
		u64 mark_formats                     : 8;
		u64 reserved_16_63                   : 48;
	} s;
	/* struct nixx_af_pse_const_s cn; */
};

static inline u64 NIXX_AF_PSE_CONST(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_PSE_CONST(void)
{
	return 0x60;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_pse_eco
 *
 * INTERNAL: AF PSE ECO Register
 */
union nixx_af_pse_eco {
	u64 u;
	struct nixx_af_pse_eco_s {
		u64 eco_rw                           : 64;
	} s;
	/* struct nixx_af_pse_eco_s cn; */
};

static inline u64 NIXX_AF_PSE_ECO(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_PSE_ECO(void)
{
	return 0x5d0;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_pse_expr_bp_test
 *
 * INTERNAL: NIX AF PSE Express Backpressure Test Register  Internal:
 * 802.3br frame preemption/express path is defeatured.
 */
union nixx_af_pse_expr_bp_test {
	u64 u;
	struct nixx_af_pse_expr_bp_test_s {
		u64 lfsr_freq                        : 12;
		u64 reserved_12_15                   : 4;
		u64 bp_cfg                           : 32;
		u64 enable                           : 16;
	} s;
	/* struct nixx_af_pse_expr_bp_test_s cn; */
};

static inline u64 NIXX_AF_PSE_EXPR_BP_TEST(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_PSE_EXPR_BP_TEST(void)
{
	return 0x890;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_pse_norm_bp_test
 *
 * INTERNAL: NIX AF PSE Normal Backpressure Test Register
 */
union nixx_af_pse_norm_bp_test {
	u64 u;
	struct nixx_af_pse_norm_bp_test_s {
		u64 lfsr_freq                        : 12;
		u64 reserved_12_15                   : 4;
		u64 bp_cfg                           : 32;
		u64 reserved_48_63                   : 16;
	} s;
	struct nixx_af_pse_norm_bp_test_cn96xxp1 {
		u64 lfsr_freq                        : 12;
		u64 reserved_12_15                   : 4;
		u64 bp_cfg                           : 32;
		u64 enable                           : 16;
	} cn96xxp1;
	struct nixx_af_pse_norm_bp_test_cn96xxp3 {
		u64 lfsr_freq                        : 12;
		u64 reserved_12_15                   : 4;
		u64 bp_cfg                           : 12;
		u64 reserved_28_57                   : 30;
		u64 enable                           : 6;
	} cn96xxp3;
	/* struct nixx_af_pse_norm_bp_test_cn96xxp1 cnf95xx; */
};

static inline u64 NIXX_AF_PSE_NORM_BP_TEST(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_PSE_NORM_BP_TEST(void)
{
	return 0x880;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_pse_shaper_cfg
 *
 * NIX AF PSE Shaper Configuration Register
 */
union nixx_af_pse_shaper_cfg {
	u64 u;
	struct nixx_af_pse_shaper_cfg_s {
		u64 red_send_as_yellow               : 1;
		u64 color_aware                      : 1;
		u64 reserved_2_63                    : 62;
	} s;
	/* struct nixx_af_pse_shaper_cfg_s cn; */
};

static inline u64 NIXX_AF_PSE_SHAPER_CFG(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_PSE_SHAPER_CFG(void)
{
	return 0x810;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_ras
 *
 * NIX AF RAS Interrupt Register This register is intended for delivery
 * of RAS events to the SCP, so should be ignored by OS drivers.
 */
union nixx_af_ras {
	u64 u;
	struct nixx_af_ras_s {
		u64 rx_mce_poison                    : 1;
		u64 rx_mcast_wqe_poison              : 1;
		u64 rx_mirror_wqe_poison             : 1;
		u64 rx_mcast_data_poison             : 1;
		u64 rx_mirror_data_poison            : 1;
		u64 reserved_5_31                    : 27;
		u64 aq_ctx_poison                    : 1;
		u64 aq_res_poison                    : 1;
		u64 aq_inst_poison                   : 1;
		u64 reserved_35_63                   : 29;
	} s;
	/* struct nixx_af_ras_s cn; */
};

static inline u64 NIXX_AF_RAS(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_RAS(void)
{
	return 0x1a0;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_ras_ena_w1c
 *
 * NIX AF RAS Interrupt Enable Clear Register This register clears
 * interrupt enable bits.
 */
union nixx_af_ras_ena_w1c {
	u64 u;
	struct nixx_af_ras_ena_w1c_s {
		u64 rx_mce_poison                    : 1;
		u64 rx_mcast_wqe_poison              : 1;
		u64 rx_mirror_wqe_poison             : 1;
		u64 rx_mcast_data_poison             : 1;
		u64 rx_mirror_data_poison            : 1;
		u64 reserved_5_31                    : 27;
		u64 aq_ctx_poison                    : 1;
		u64 aq_res_poison                    : 1;
		u64 aq_inst_poison                   : 1;
		u64 reserved_35_63                   : 29;
	} s;
	/* struct nixx_af_ras_ena_w1c_s cn; */
};

static inline u64 NIXX_AF_RAS_ENA_W1C(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_RAS_ENA_W1C(void)
{
	return 0x1b8;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_ras_ena_w1s
 *
 * NIX AF RAS Interrupt Enable Set Register This register sets interrupt
 * enable bits.
 */
union nixx_af_ras_ena_w1s {
	u64 u;
	struct nixx_af_ras_ena_w1s_s {
		u64 rx_mce_poison                    : 1;
		u64 rx_mcast_wqe_poison              : 1;
		u64 rx_mirror_wqe_poison             : 1;
		u64 rx_mcast_data_poison             : 1;
		u64 rx_mirror_data_poison            : 1;
		u64 reserved_5_31                    : 27;
		u64 aq_ctx_poison                    : 1;
		u64 aq_res_poison                    : 1;
		u64 aq_inst_poison                   : 1;
		u64 reserved_35_63                   : 29;
	} s;
	/* struct nixx_af_ras_ena_w1s_s cn; */
};

static inline u64 NIXX_AF_RAS_ENA_W1S(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_RAS_ENA_W1S(void)
{
	return 0x1b0;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_ras_w1s
 *
 * NIX AF RAS Interrupt Set Register This register sets interrupt bits.
 */
union nixx_af_ras_w1s {
	u64 u;
	struct nixx_af_ras_w1s_s {
		u64 rx_mce_poison                    : 1;
		u64 rx_mcast_wqe_poison              : 1;
		u64 rx_mirror_wqe_poison             : 1;
		u64 rx_mcast_data_poison             : 1;
		u64 rx_mirror_data_poison            : 1;
		u64 reserved_5_31                    : 27;
		u64 aq_ctx_poison                    : 1;
		u64 aq_res_poison                    : 1;
		u64 aq_inst_poison                   : 1;
		u64 reserved_35_63                   : 29;
	} s;
	/* struct nixx_af_ras_w1s_s cn; */
};

static inline u64 NIXX_AF_RAS_W1S(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_RAS_W1S(void)
{
	return 0x1a8;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_reb_bp_test#
 *
 * INTERNAL: NIX AF REB Backpressure Test Registers
 */
union nixx_af_reb_bp_testx {
	u64 u;
	struct nixx_af_reb_bp_testx_s {
		u64 lfsr_freq                        : 12;
		u64 reserved_12_15                   : 4;
		u64 bp_cfg                           : 8;
		u64 reserved_24_47                   : 24;
		u64 enable                           : 4;
		u64 reserved_52_63                   : 12;
	} s;
	/* struct nixx_af_reb_bp_testx_s cn; */
};

static inline u64 NIXX_AF_REB_BP_TESTX(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_REB_BP_TESTX(u64 a)
{
	return 0x4840 + 0x10000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_rq_const
 *
 * NIX AF RQ Constants Register This register contains constants for
 * software discovery.
 */
union nixx_af_rq_const {
	u64 u;
	struct nixx_af_rq_const_s {
		u64 queues_per_lf                    : 24;
		u64 reserved_24_63                   : 40;
	} s;
	/* struct nixx_af_rq_const_s cn; */
};

static inline u64 NIXX_AF_RQ_CONST(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_RQ_CONST(void)
{
	return 0x50;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_rqm_bp_test
 *
 * INTERNAL: NIX AF REB Backpressure Test Registers
 */
union nixx_af_rqm_bp_test {
	u64 u;
	struct nixx_af_rqm_bp_test_s {
		u64 lfsr_freq                        : 12;
		u64 reserved_12_15                   : 4;
		u64 bp_cfg                           : 16;
		u64 reserved_32_47                   : 16;
		u64 enable                           : 8;
		u64 reserved_56_63                   : 8;
	} s;
	/* struct nixx_af_rqm_bp_test_s cn; */
};

static inline u64 NIXX_AF_RQM_BP_TEST(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_RQM_BP_TEST(void)
{
	return 0x4880;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_rqm_eco
 *
 * INTERNAL: AF RQM ECO Register
 */
union nixx_af_rqm_eco {
	u64 u;
	struct nixx_af_rqm_eco_s {
		u64 eco_rw                           : 64;
	} s;
	/* struct nixx_af_rqm_eco_s cn; */
};

static inline u64 NIXX_AF_RQM_ECO(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_RQM_ECO(void)
{
	return 0x5a0;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_rvu_int
 *
 * NIX AF RVU Interrupt Register This register contains RVU error
 * interrupt summary bits.
 */
union nixx_af_rvu_int {
	u64 u;
	struct nixx_af_rvu_int_s {
		u64 unmapped_slot                    : 1;
		u64 reserved_1_63                    : 63;
	} s;
	/* struct nixx_af_rvu_int_s cn; */
};

static inline u64 NIXX_AF_RVU_INT(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_RVU_INT(void)
{
	return 0x1c0;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_rvu_int_ena_w1c
 *
 * NIX AF RVU Interrupt Enable Clear Register This register clears
 * interrupt enable bits.
 */
union nixx_af_rvu_int_ena_w1c {
	u64 u;
	struct nixx_af_rvu_int_ena_w1c_s {
		u64 unmapped_slot                    : 1;
		u64 reserved_1_63                    : 63;
	} s;
	/* struct nixx_af_rvu_int_ena_w1c_s cn; */
};

static inline u64 NIXX_AF_RVU_INT_ENA_W1C(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_RVU_INT_ENA_W1C(void)
{
	return 0x1d8;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_rvu_int_ena_w1s
 *
 * NIX AF RVU Interrupt Enable Set Register This register sets interrupt
 * enable bits.
 */
union nixx_af_rvu_int_ena_w1s {
	u64 u;
	struct nixx_af_rvu_int_ena_w1s_s {
		u64 unmapped_slot                    : 1;
		u64 reserved_1_63                    : 63;
	} s;
	/* struct nixx_af_rvu_int_ena_w1s_s cn; */
};

static inline u64 NIXX_AF_RVU_INT_ENA_W1S(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_RVU_INT_ENA_W1S(void)
{
	return 0x1d0;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_rvu_int_w1s
 *
 * NIX AF RVU Interrupt Set Register This register sets interrupt bits.
 */
union nixx_af_rvu_int_w1s {
	u64 u;
	struct nixx_af_rvu_int_w1s_s {
		u64 unmapped_slot                    : 1;
		u64 reserved_1_63                    : 63;
	} s;
	/* struct nixx_af_rvu_int_w1s_s cn; */
};

static inline u64 NIXX_AF_RVU_INT_W1S(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_RVU_INT_W1S(void)
{
	return 0x1c8;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_rvu_lf_cfg_debug
 *
 * NIX Privileged LF Configuration Debug Register This debug register
 * allows software to lookup the reverse mapping from VF/PF slot to LF.
 * The forward mapping is programmed with NIX_PRIV_LF()_CFG.
 */
union nixx_af_rvu_lf_cfg_debug {
	u64 u;
	struct nixx_af_rvu_lf_cfg_debug_s {
		u64 lf                               : 12;
		u64 lf_valid                         : 1;
		u64 exec                             : 1;
		u64 reserved_14_15                   : 2;
		u64 slot                             : 8;
		u64 pf_func                          : 16;
		u64 reserved_40_63                   : 24;
	} s;
	/* struct nixx_af_rvu_lf_cfg_debug_s cn; */
};

static inline u64 NIXX_AF_RVU_LF_CFG_DEBUG(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_RVU_LF_CFG_DEBUG(void)
{
	return 0x8000030;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_rx_active_cycles_pc#
 *
 * NIX AF Active Cycles Register These registers are indexed by the
 * conditional clock domain number.
 */
union nixx_af_rx_active_cycles_pcx {
	u64 u;
	struct nixx_af_rx_active_cycles_pcx_s {
		u64 act_cyc                          : 64;
	} s;
	/* struct nixx_af_rx_active_cycles_pcx_s cn; */
};

static inline u64 NIXX_AF_RX_ACTIVE_CYCLES_PCX(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_RX_ACTIVE_CYCLES_PCX(u64 a)
{
	return 0x4800 + 0x10000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_rx_bpid#_status
 *
 * NIX AF Receive Backpressure ID Status Registers
 */
union nixx_af_rx_bpidx_status {
	u64 u;
	struct nixx_af_rx_bpidx_status_s {
		u64 aura_cnt                         : 32;
		u64 cq_cnt                           : 32;
	} s;
	/* struct nixx_af_rx_bpidx_status_s cn; */
};

static inline u64 NIXX_AF_RX_BPIDX_STATUS(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_RX_BPIDX_STATUS(u64 a)
{
	return 0x1a20 + 0x20000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_rx_cfg
 *
 * NIX AF Receive Configuration Register
 */
union nixx_af_rx_cfg {
	u64 u;
	struct nixx_af_rx_cfg_s {
		u64 cbp_ena                          : 1;
		u64 reserved_1_63                    : 63;
	} s;
	/* struct nixx_af_rx_cfg_s cn; */
};

static inline u64 NIXX_AF_RX_CFG(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_RX_CFG(void)
{
	return 0xd0;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_rx_chan#_cfg
 *
 * NIX AF Receive Channel Configuration Registers
 */
union nixx_af_rx_chanx_cfg {
	u64 u;
	struct nixx_af_rx_chanx_cfg_s {
		u64 bpid                             : 9;
		u64 reserved_9_15                    : 7;
		u64 bp_ena                           : 1;
		u64 sw_xoff                          : 1;
		u64 imp                              : 1;
		u64 reserved_19_63                   : 45;
	} s;
	/* struct nixx_af_rx_chanx_cfg_s cn; */
};

static inline u64 NIXX_AF_RX_CHANX_CFG(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_RX_CHANX_CFG(u64 a)
{
	return 0x1a30 + 0x8000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_rx_cpt#_credit
 *
 * INTERNAL: NIX AF Receive CPT Credit Register  Internal: Not used; no
 * IPSEC fast-path.
 */
union nixx_af_rx_cptx_credit {
	u64 u;
	struct nixx_af_rx_cptx_credit_s {
		u64 inst_cred_cnt                    : 22;
		u64 reserved_22_63                   : 42;
	} s;
	/* struct nixx_af_rx_cptx_credit_s cn; */
};

static inline u64 NIXX_AF_RX_CPTX_CREDIT(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_RX_CPTX_CREDIT(u64 a)
{
	return 0x360 + 8 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_rx_cpt#_inst_qsel
 *
 * INTERNAL: NIX AF Receive CPT Instruction Queue Select Register
 * Internal: Not used; no IPSEC fast-path.
 */
union nixx_af_rx_cptx_inst_qsel {
	u64 u;
	struct nixx_af_rx_cptx_inst_qsel_s {
		u64 slot                             : 8;
		u64 pf_func                          : 16;
		u64 reserved_24_63                   : 40;
	} s;
	/* struct nixx_af_rx_cptx_inst_qsel_s cn; */
};

static inline u64 NIXX_AF_RX_CPTX_INST_QSEL(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_RX_CPTX_INST_QSEL(u64 a)
{
	return 0x320 + 8 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_rx_def_iip4
 *
 * NIX AF Receive Inner IPv4 Header Definition Register Defines layer
 * information in NPC_RESULT_S to identify an inner IPv4 header.
 * Typically the same as NPC_AF_PCK_DEF_IIP4.
 */
union nixx_af_rx_def_iip4 {
	u64 u;
	struct nixx_af_rx_def_iip4_s {
		u64 ltype_mask                       : 4;
		u64 ltype_match                      : 4;
		u64 lid                              : 3;
		u64 reserved_11_63                   : 53;
	} s;
	/* struct nixx_af_rx_def_iip4_s cn; */
};

static inline u64 NIXX_AF_RX_DEF_IIP4(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_RX_DEF_IIP4(void)
{
	return 0x220;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_rx_def_iip6
 *
 * NIX AF Receive Inner IPv6 Header Definition Register Defines layer
 * information in NPC_RESULT_S to identify an inner IPv6 header.
 */
union nixx_af_rx_def_iip6 {
	u64 u;
	struct nixx_af_rx_def_iip6_s {
		u64 ltype_mask                       : 4;
		u64 ltype_match                      : 4;
		u64 lid                              : 3;
		u64 reserved_11_63                   : 53;
	} s;
	/* struct nixx_af_rx_def_iip6_s cn; */
};

static inline u64 NIXX_AF_RX_DEF_IIP6(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_RX_DEF_IIP6(void)
{
	return 0x240;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_rx_def_ipsec#
 *
 * INTERNAL: NIX AF Receive IPSEC Header Definition Registers  Internal:
 * Not used; no IPSEC fast-path.
 */
union nixx_af_rx_def_ipsecx {
	u64 u;
	struct nixx_af_rx_def_ipsecx_s {
		u64 ltype_mask                       : 4;
		u64 ltype_match                      : 4;
		u64 lid                              : 3;
		u64 reserved_11                      : 1;
		u64 spi_offset                       : 4;
		u64 spi_nz                           : 1;
		u64 reserved_17_63                   : 47;
	} s;
	/* struct nixx_af_rx_def_ipsecx_s cn; */
};

static inline u64 NIXX_AF_RX_DEF_IPSECX(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_RX_DEF_IPSECX(u64 a)
{
	return 0x2b0 + 8 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_rx_def_isctp
 *
 * NIX AF Receive Inner SCTP Header Definition Register Defines layer
 * information in NPC_RESULT_S to identify an inner SCTP header.
 */
union nixx_af_rx_def_isctp {
	u64 u;
	struct nixx_af_rx_def_isctp_s {
		u64 ltype_mask                       : 4;
		u64 ltype_match                      : 4;
		u64 lid                              : 3;
		u64 reserved_11_63                   : 53;
	} s;
	/* struct nixx_af_rx_def_isctp_s cn; */
};

static inline u64 NIXX_AF_RX_DEF_ISCTP(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_RX_DEF_ISCTP(void)
{
	return 0x2a0;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_rx_def_itcp
 *
 * NIX AF Receive Inner TCP Header Definition Register Defines layer
 * information in NPC_RESULT_S to identify an inner TCP header.
 */
union nixx_af_rx_def_itcp {
	u64 u;
	struct nixx_af_rx_def_itcp_s {
		u64 ltype_mask                       : 4;
		u64 ltype_match                      : 4;
		u64 lid                              : 3;
		u64 reserved_11_63                   : 53;
	} s;
	/* struct nixx_af_rx_def_itcp_s cn; */
};

static inline u64 NIXX_AF_RX_DEF_ITCP(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_RX_DEF_ITCP(void)
{
	return 0x260;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_rx_def_iudp
 *
 * NIX AF Receive Inner UDP Header Definition Register Defines layer
 * information in NPC_RESULT_S to identify an inner UDP header.
 */
union nixx_af_rx_def_iudp {
	u64 u;
	struct nixx_af_rx_def_iudp_s {
		u64 ltype_mask                       : 4;
		u64 ltype_match                      : 4;
		u64 lid                              : 3;
		u64 reserved_11_63                   : 53;
	} s;
	/* struct nixx_af_rx_def_iudp_s cn; */
};

static inline u64 NIXX_AF_RX_DEF_IUDP(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_RX_DEF_IUDP(void)
{
	return 0x280;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_rx_def_oip4
 *
 * NIX AF Receive Outer IPv4 Header Definition Register Defines layer
 * information in NPC_RESULT_S to identify an outer IPv4 L3 header.
 * Typically the same as NPC_AF_PCK_DEF_OIP4.
 */
union nixx_af_rx_def_oip4 {
	u64 u;
	struct nixx_af_rx_def_oip4_s {
		u64 ltype_mask                       : 4;
		u64 ltype_match                      : 4;
		u64 lid                              : 3;
		u64 reserved_11_63                   : 53;
	} s;
	/* struct nixx_af_rx_def_oip4_s cn; */
};

static inline u64 NIXX_AF_RX_DEF_OIP4(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_RX_DEF_OIP4(void)
{
	return 0x210;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_rx_def_oip6
 *
 * NIX AF Receive Outer IPv6 Header Definition Register Defines layer
 * information in NPC_RESULT_S to identify an outer IPv6 header.
 * Typically the same as NPC_AF_PCK_DEF_OIP6.
 */
union nixx_af_rx_def_oip6 {
	u64 u;
	struct nixx_af_rx_def_oip6_s {
		u64 ltype_mask                       : 4;
		u64 ltype_match                      : 4;
		u64 lid                              : 3;
		u64 reserved_11_63                   : 53;
	} s;
	/* struct nixx_af_rx_def_oip6_s cn; */
};

static inline u64 NIXX_AF_RX_DEF_OIP6(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_RX_DEF_OIP6(void)
{
	return 0x230;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_rx_def_ol2
 *
 * NIX AF Receive Outer L2 Header Definition Register Defines layer
 * information in NPC_RESULT_S to identify an outer L2/Ethernet header.
 * Typically the same as NPC_AF_PCK_DEF_OL2.
 */
union nixx_af_rx_def_ol2 {
	u64 u;
	struct nixx_af_rx_def_ol2_s {
		u64 ltype_mask                       : 4;
		u64 ltype_match                      : 4;
		u64 lid                              : 3;
		u64 reserved_11_63                   : 53;
	} s;
	/* struct nixx_af_rx_def_ol2_s cn; */
};

static inline u64 NIXX_AF_RX_DEF_OL2(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_RX_DEF_OL2(void)
{
	return 0x200;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_rx_def_osctp
 *
 * NIX AF Receive Outer SCTP Header Definition Register Defines layer
 * information in NPC_RESULT_S to identify an outer SCTP header.
 */
union nixx_af_rx_def_osctp {
	u64 u;
	struct nixx_af_rx_def_osctp_s {
		u64 ltype_mask                       : 4;
		u64 ltype_match                      : 4;
		u64 lid                              : 3;
		u64 reserved_11_63                   : 53;
	} s;
	/* struct nixx_af_rx_def_osctp_s cn; */
};

static inline u64 NIXX_AF_RX_DEF_OSCTP(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_RX_DEF_OSCTP(void)
{
	return 0x290;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_rx_def_otcp
 *
 * NIX AF Receive Outer TCP Header Definition Register Defines layer
 * information in NPC_RESULT_S to identify an outer TCP header.
 */
union nixx_af_rx_def_otcp {
	u64 u;
	struct nixx_af_rx_def_otcp_s {
		u64 ltype_mask                       : 4;
		u64 ltype_match                      : 4;
		u64 lid                              : 3;
		u64 reserved_11_63                   : 53;
	} s;
	/* struct nixx_af_rx_def_otcp_s cn; */
};

static inline u64 NIXX_AF_RX_DEF_OTCP(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_RX_DEF_OTCP(void)
{
	return 0x250;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_rx_def_oudp
 *
 * NIX AF Receive Outer UDP Header Definition Register Defines layer
 * information in NPC_RESULT_S to identify an outer UDP header.
 */
union nixx_af_rx_def_oudp {
	u64 u;
	struct nixx_af_rx_def_oudp_s {
		u64 ltype_mask                       : 4;
		u64 ltype_match                      : 4;
		u64 lid                              : 3;
		u64 reserved_11_63                   : 53;
	} s;
	/* struct nixx_af_rx_def_oudp_s cn; */
};

static inline u64 NIXX_AF_RX_DEF_OUDP(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_RX_DEF_OUDP(void)
{
	return 0x270;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_rx_flow_key_alg#_field#
 *
 * NIX AF Receive Flow Key Algorithm Field Registers A flow key algorithm
 * defines how the 40-byte FLOW_KEY is formed from the received packet
 * header. FLOW_KEY is formed using up to five header fields (this
 * register's last index) with up to 16 bytes per field. Header fields
 * must not overlap in FLOW_KEY.  The algorithm (index {a} (ALG) of these
 * registers) is selected by NIX_RX_ACTION_S[FLOW_KEY_ALG] from the
 * packet's NPC_RESULT_S[ACTION].  Internal: 40-byte FLOW_KEY is wide
 * enough to support an IPv6 5-tuple that includes a VXLAN/GENEVE/NVGRE
 * tunnel ID, e.g: _ Source IP: 16B. _ Dest IP: 16B. _ Source port: 2B. _
 * Dest port: 2B. _ Tunnel VNI/VSI: 3B. _ Total: 39B.
 */
union nixx_af_rx_flow_key_algx_fieldx {
	u64 u;
	struct nixx_af_rx_flow_key_algx_fieldx_s {
		u64 key_offset                       : 6;
		u64 ln_mask                          : 1;
		u64 fn_mask                          : 1;
		u64 hdr_offset                       : 8;
		u64 bytesm1                          : 5;
		u64 lid                              : 3;
		u64 reserved_24                      : 1;
		u64 ena                              : 1;
		u64 sel_chan                         : 1;
		u64 ltype_mask                       : 4;
		u64 ltype_match                      : 4;
		u64 reserved_35_63                   : 29;
	} s;
	/* struct nixx_af_rx_flow_key_algx_fieldx_s cn; */
};

static inline u64 NIXX_AF_RX_FLOW_KEY_ALGX_FIELDX(u64 a, u64 b)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_RX_FLOW_KEY_ALGX_FIELDX(u64 a, u64 b)
{
	return 0x1800 + 0x40000 * a + 8 * b;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_rx_ipsec_gen_cfg
 *
 * INTERNAL: NIX AF Receive IPSEC General Configuration Register
 * Internal: Not used; no IPSEC fast-path.
 */
union nixx_af_rx_ipsec_gen_cfg {
	u64 u;
	struct nixx_af_rx_ipsec_gen_cfg_s {
		u64 param2                           : 16;
		u64 param1                           : 16;
		u64 opcode                           : 16;
		u64 egrp                             : 3;
		u64 reserved_51_63                   : 13;
	} s;
	/* struct nixx_af_rx_ipsec_gen_cfg_s cn; */
};

static inline u64 NIXX_AF_RX_IPSEC_GEN_CFG(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_RX_IPSEC_GEN_CFG(void)
{
	return 0x300;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_rx_link#_cfg
 *
 * NIX AF Receive Link Configuration Registers Index enumerated by
 * NIX_LINK_E.
 */
union nixx_af_rx_linkx_cfg {
	u64 u;
	struct nixx_af_rx_linkx_cfg_s {
		u64 minlen                           : 16;
		u64 maxlen                           : 16;
		u64 reserved_32_63                   : 32;
	} s;
	/* struct nixx_af_rx_linkx_cfg_s cn; */
};

static inline u64 NIXX_AF_RX_LINKX_CFG(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_RX_LINKX_CFG(u64 a)
{
	return 0x540 + 0x10000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_rx_link#_sl#_spkt_cnt
 *
 * INTERNAL: NIX Receive Software Sync Link Packet Count Registers  For
 * diagnostic use only for debug of NIX_AF_RX_SW_SYNC[ENA] function. LINK
 * index is enumerated by NIX_LINK_E. For the internal multicast/mirror
 * link (NIX_LINK_E::MC), SL index is zero for multicast replay, one for
 * mirror replay. SL index one is reserved for all other links.
 * Internal: 802.3br frame preemption/express path is defeatured. Old
 * definition of SL index: SL index is zero for non-express packets, one
 * for express packets. For the internal NIX_LINK_E::MC, SL index is zero
 * for multicast replay, one for mirror replay.
 */
union nixx_af_rx_linkx_slx_spkt_cnt {
	u64 u;
	struct nixx_af_rx_linkx_slx_spkt_cnt_s {
		u64 in_cnt                           : 20;
		u64 reserved_20_31                   : 12;
		u64 out_cnt                          : 20;
		u64 reserved_52_63                   : 12;
	} s;
	/* struct nixx_af_rx_linkx_slx_spkt_cnt_s cn; */
};

static inline u64 NIXX_AF_RX_LINKX_SLX_SPKT_CNT(u64 a, u64 b)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_RX_LINKX_SLX_SPKT_CNT(u64 a, u64 b)
{
	return 0x500 + 0x10000 * a + 8 * b;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_rx_link#_wrr_cfg
 *
 * NIX AF Receive Link Weighted Round Robin Configuration Registers Index
 * enumerated by NIX_LINK_E.
 */
union nixx_af_rx_linkx_wrr_cfg {
	u64 u;
	struct nixx_af_rx_linkx_wrr_cfg_s {
		u64 weight                           : 8;
		u64 reserved_8_63                    : 56;
	} s;
	/* struct nixx_af_rx_linkx_wrr_cfg_s cn; */
};

static inline u64 NIXX_AF_RX_LINKX_WRR_CFG(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_RX_LINKX_WRR_CFG(u64 a)
{
	return 0x560 + 0x10000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_rx_mcast_base
 *
 * NIX AF Receive Multicast/Mirror Table Base Address Register This
 * register specifies the base AF IOVA of the receive multicast/mirror
 * table in NDC/LLC/DRAM. The table consists of 1 \<\<
 * (NIX_AF_RX_MCAST_CFG[SIZE] + 8) contiguous NIX_RX_MCE_S structures.
 * The size of each structure is 1 \<\< NIX_AF_CONST3[MCE_LOG2BYTES].
 * The table contains multicast/mirror replication lists. Each list
 * consists of linked entries with NIX_RX_MCE_S[EOL] = 1 in the last
 * entry. All lists must reside within the table size specified by
 * NIX_AF_RX_MCAST_CFG[SIZE]. A mirror replication list will typically
 * consist of two entries, but that is not checked or enforced by
 * hardware.  A receive packet is multicast when the action returned by
 * NPC has NIX_RX_ACTION_S[OP] = NIX_RX_ACTIONOP_E::MCAST. A receive
 * packet is mirrored when the action returned by NPC has
 * NIX_RX_ACTION_S[OP] = NIX_RX_ACTIONOP_E::MIRROR. In both cases,
 * NIX_RX_ACTION_S[INDEX] specifies the index of the replication list's
 * first NIX_RX_MCE_S in the table, and a linked entry with
 * NIX_RX_MCE_S[EOL] = 1 indicates the end of list.  If a mirrored flow
 * is part of a multicast replication list, software should include the
 * two mirror entries in that list.  Internal: A multicast list may have
 * multiple entries for the same LF (e.g. for future RoCE/IB multicast).
 */
union nixx_af_rx_mcast_base {
	u64 u;
	struct nixx_af_rx_mcast_base_s {
		u64 reserved_0_6                     : 7;
		u64 addr                             : 46;
		u64 reserved_53_63                   : 11;
	} s;
	/* struct nixx_af_rx_mcast_base_s cn; */
};

static inline u64 NIXX_AF_RX_MCAST_BASE(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_RX_MCAST_BASE(void)
{
	return 0x100;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_rx_mcast_buf_base
 *
 * NIX AF Receive Multicast Buffer Base Address Register This register
 * specifies the base AF IOVA of the receive multicast buffers in
 * NDC/LLC/DRAM. These buffers are used to temporarily store packets
 * whose action returned by NPC has NIX_RX_ACTION_S[OP] =
 * NIX_RX_ACTIONOP_E::MCAST. The number of buffers is configured by
 * NIX_AF_RX_MCAST_BUF_CFG[SIZE].  If the number of free buffers is
 * insufficient for a received multicast packet, hardware tail drops the
 * packet and sets NIX_AF_GEN_INT[RX_MCAST_DROP].  Hardware prioritizes
 * the processing of RX mirror packets over RX multicast packets.
 */
union nixx_af_rx_mcast_buf_base {
	u64 u;
	struct nixx_af_rx_mcast_buf_base_s {
		u64 reserved_0_6                     : 7;
		u64 addr                             : 46;
		u64 reserved_53_63                   : 11;
	} s;
	/* struct nixx_af_rx_mcast_buf_base_s cn; */
};

static inline u64 NIXX_AF_RX_MCAST_BUF_BASE(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_RX_MCAST_BUF_BASE(void)
{
	return 0x120;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_rx_mcast_buf_cfg
 *
 * NIX AF Receive Multicast Buffer Configuration Register See
 * NIX_AF_RX_MCAST_BUF_BASE.
 */
union nixx_af_rx_mcast_buf_cfg {
	u64 u;
	struct nixx_af_rx_mcast_buf_cfg_s {
		u64 size                             : 4;
		u64 way_mask                         : 16;
		u64 caching                          : 1;
		u64 reserved_21_23                   : 3;
		u64 npc_replay_pkind                 : 6;
		u64 reserved_30_31                   : 2;
		u64 free_buf_level                   : 11;
		u64 reserved_43_61                   : 19;
		u64 busy                             : 1;
		u64 ena                              : 1;
	} s;
	struct nixx_af_rx_mcast_buf_cfg_cn96xxp1 {
		u64 size                             : 4;
		u64 way_mask                         : 16;
		u64 caching                          : 1;
		u64 reserved_21_23                   : 3;
		u64 npc_replay_pkind                 : 6;
		u64 reserved_30_31                   : 2;
		u64 free_buf_level                   : 11;
		u64 reserved_43_61                   : 19;
		u64 reserved_62                      : 1;
		u64 ena                              : 1;
	} cn96xxp1;
	/* struct nixx_af_rx_mcast_buf_cfg_s cn96xxp3; */
	struct nixx_af_rx_mcast_buf_cfg_cnf95xxp1 {
		u64 size                             : 4;
		u64 way_mask                         : 16;
		u64 caching                          : 1;
		u64 reserved_21_23                   : 3;
		u64 npc_replay_pkind                 : 6;
		u64 reserved_30_31                   : 2;
		u64 free_buf_level                   : 11;
		u64 reserved_43_62                   : 20;
		u64 ena                              : 1;
	} cnf95xxp1;
	/* struct nixx_af_rx_mcast_buf_cfg_s cnf95xxp2; */
};

static inline u64 NIXX_AF_RX_MCAST_BUF_CFG(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_RX_MCAST_BUF_CFG(void)
{
	return 0x130;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_rx_mcast_cfg
 *
 * NIX AF Receive Multicast/Mirror Table Configuration Register See
 * NIX_AF_RX_MCAST_BASE.
 */
union nixx_af_rx_mcast_cfg {
	u64 u;
	struct nixx_af_rx_mcast_cfg_s {
		u64 size                             : 4;
		u64 max_list_lenm1                   : 8;
		u64 reserved_12_19                   : 8;
		u64 way_mask                         : 16;
		u64 caching                          : 1;
		u64 reserved_37_63                   : 27;
	} s;
	/* struct nixx_af_rx_mcast_cfg_s cn; */
};

static inline u64 NIXX_AF_RX_MCAST_CFG(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_RX_MCAST_CFG(void)
{
	return 0x110;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_rx_mirror_buf_base
 *
 * NIX AF Receive Mirror Buffer Base Address Register This register
 * specifies the base AF IOVA of the receive mirror buffers in
 * NDC/LLC/DRAM. These buffers are used to temporarily store packets
 * whose action returned by NPC has NIX_RX_ACTION_S[OP] =
 * NIX_RX_ACTIONOP_E::MIRROR. The number of buffers is configured by
 * NIX_AF_RX_MIRROR_BUF_CFG[SIZE].  If the number of free buffers is
 * insufficient for a received multicast packet, hardware tail drops the
 * packet and sets NIX_AF_GEN_INT[RX_MIRROR_DROP].  Hardware prioritizes
 * the processing of RX mirror packets over RX multicast packets.
 */
union nixx_af_rx_mirror_buf_base {
	u64 u;
	struct nixx_af_rx_mirror_buf_base_s {
		u64 reserved_0_6                     : 7;
		u64 addr                             : 46;
		u64 reserved_53_63                   : 11;
	} s;
	/* struct nixx_af_rx_mirror_buf_base_s cn; */
};

static inline u64 NIXX_AF_RX_MIRROR_BUF_BASE(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_RX_MIRROR_BUF_BASE(void)
{
	return 0x140;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_rx_mirror_buf_cfg
 *
 * NIX AF Receive Mirror Buffer Configuration Register See
 * NIX_AF_RX_MIRROR_BUF_BASE.
 */
union nixx_af_rx_mirror_buf_cfg {
	u64 u;
	struct nixx_af_rx_mirror_buf_cfg_s {
		u64 size                             : 4;
		u64 way_mask                         : 16;
		u64 caching                          : 1;
		u64 reserved_21_23                   : 3;
		u64 npc_replay_pkind                 : 6;
		u64 reserved_30_31                   : 2;
		u64 free_buf_level                   : 11;
		u64 reserved_43_61                   : 19;
		u64 busy                             : 1;
		u64 ena                              : 1;
	} s;
	struct nixx_af_rx_mirror_buf_cfg_cn96xxp1 {
		u64 size                             : 4;
		u64 way_mask                         : 16;
		u64 caching                          : 1;
		u64 reserved_21_23                   : 3;
		u64 npc_replay_pkind                 : 6;
		u64 reserved_30_31                   : 2;
		u64 free_buf_level                   : 11;
		u64 reserved_43_61                   : 19;
		u64 reserved_62                      : 1;
		u64 ena                              : 1;
	} cn96xxp1;
	/* struct nixx_af_rx_mirror_buf_cfg_s cn96xxp3; */
	struct nixx_af_rx_mirror_buf_cfg_cnf95xxp1 {
		u64 size                             : 4;
		u64 way_mask                         : 16;
		u64 caching                          : 1;
		u64 reserved_21_23                   : 3;
		u64 npc_replay_pkind                 : 6;
		u64 reserved_30_31                   : 2;
		u64 free_buf_level                   : 11;
		u64 reserved_43_62                   : 20;
		u64 ena                              : 1;
	} cnf95xxp1;
	/* struct nixx_af_rx_mirror_buf_cfg_s cnf95xxp2; */
};

static inline u64 NIXX_AF_RX_MIRROR_BUF_CFG(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_RX_MIRROR_BUF_CFG(void)
{
	return 0x148;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_rx_npc_mc_drop
 *
 * NIX AF Multicast Drop Statistics Register The counter increments for
 * every dropped MC packet marked by the NPC.
 */
union nixx_af_rx_npc_mc_drop {
	u64 u;
	struct nixx_af_rx_npc_mc_drop_s {
		u64 stat                             : 48;
		u64 reserved_48_63                   : 16;
	} s;
	/* struct nixx_af_rx_npc_mc_drop_s cn; */
};

static inline u64 NIXX_AF_RX_NPC_MC_DROP(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_RX_NPC_MC_DROP(void)
{
	return 0x4710;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_rx_npc_mc_rcv
 *
 * NIX AF Multicast Receive Statistics Register The counter increments
 * for every received MC packet marked by the NPC.
 */
union nixx_af_rx_npc_mc_rcv {
	u64 u;
	struct nixx_af_rx_npc_mc_rcv_s {
		u64 stat                             : 48;
		u64 reserved_48_63                   : 16;
	} s;
	/* struct nixx_af_rx_npc_mc_rcv_s cn; */
};

static inline u64 NIXX_AF_RX_NPC_MC_RCV(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_RX_NPC_MC_RCV(void)
{
	return 0x4700;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_rx_npc_mirror_drop
 *
 * NIX AF Mirror Drop Statistics Register The counter increments for
 * every dropped MIRROR packet marked by the NPC.
 */
union nixx_af_rx_npc_mirror_drop {
	u64 u;
	struct nixx_af_rx_npc_mirror_drop_s {
		u64 stat                             : 48;
		u64 reserved_48_63                   : 16;
	} s;
	/* struct nixx_af_rx_npc_mirror_drop_s cn; */
};

static inline u64 NIXX_AF_RX_NPC_MIRROR_DROP(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_RX_NPC_MIRROR_DROP(void)
{
	return 0x4730;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_rx_npc_mirror_rcv
 *
 * NIX AF Mirror Receive Statistics Register The counter increments for
 * every received MIRROR packet marked by the NPC.
 */
union nixx_af_rx_npc_mirror_rcv {
	u64 u;
	struct nixx_af_rx_npc_mirror_rcv_s {
		u64 stat                             : 48;
		u64 reserved_48_63                   : 16;
	} s;
	/* struct nixx_af_rx_npc_mirror_rcv_s cn; */
};

static inline u64 NIXX_AF_RX_NPC_MIRROR_RCV(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_RX_NPC_MIRROR_RCV(void)
{
	return 0x4720;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_rx_sw_sync
 *
 * NIX AF Receive Software Sync Register
 */
union nixx_af_rx_sw_sync {
	u64 u;
	struct nixx_af_rx_sw_sync_s {
		u64 ena                              : 1;
		u64 reserved_1_63                    : 63;
	} s;
	/* struct nixx_af_rx_sw_sync_s cn; */
};

static inline u64 NIXX_AF_RX_SW_SYNC(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_RX_SW_SYNC(void)
{
	return 0x550;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_sdp_hw_xoff#
 *
 * NIX AF SDP Transmit Link Hardware Controlled XOFF Registers .
 */
union nixx_af_sdp_hw_xoffx {
	u64 u;
	struct nixx_af_sdp_hw_xoffx_s {
		u64 chan_xoff                        : 64;
	} s;
	/* struct nixx_af_sdp_hw_xoffx_s cn; */
};

static inline u64 NIXX_AF_SDP_HW_XOFFX(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_SDP_HW_XOFFX(u64 a)
{
	return 0xac0 + 8 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_sdp_link_credit
 *
 * NIX AF Transmit Link SDP Credit Register This register tracks SDP link
 * credits.
 */
union nixx_af_sdp_link_credit {
	u64 u;
	struct nixx_af_sdp_link_credit_s {
		u64 reserved_0                       : 1;
		u64 cc_enable                        : 1;
		u64 cc_packet_cnt                    : 10;
		u64 cc_unit_cnt                      : 20;
		u64 reserved_32_62                   : 31;
		u64 pse_pkt_id_lmt                   : 1;
	} s;
	struct nixx_af_sdp_link_credit_cn96xx {
		u64 reserved_0                       : 1;
		u64 cc_enable                        : 1;
		u64 cc_packet_cnt                    : 10;
		u64 cc_unit_cnt                      : 20;
		u64 reserved_32_62                   : 31;
		u64 reserved_63                      : 1;
	} cn96xx;
	/* struct nixx_af_sdp_link_credit_s cnf95xx; */
};

static inline u64 NIXX_AF_SDP_LINK_CREDIT(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_SDP_LINK_CREDIT(void)
{
	return 0xa40;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_sdp_sw_xoff#
 *
 * INTERNAL: NIX AF SDP Transmit Link Software Controlled XOFF Registers
 * Internal: Defeatured registers. Software should use
 * NIX_AF_TL4()_SW_XOFF registers instead.
 */
union nixx_af_sdp_sw_xoffx {
	u64 u;
	struct nixx_af_sdp_sw_xoffx_s {
		u64 chan_xoff                        : 64;
	} s;
	/* struct nixx_af_sdp_sw_xoffx_s cn; */
};

static inline u64 NIXX_AF_SDP_SW_XOFFX(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_SDP_SW_XOFFX(u64 a)
{
	return 0xa60 + 8 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_sdp_tx_fifo_status
 *
 * NIX AF SDP Transmit FIFO Status Register Status of FIFO which
 * transmits packets to SDP.
 */
union nixx_af_sdp_tx_fifo_status {
	u64 u;
	struct nixx_af_sdp_tx_fifo_status_s {
		u64 count                            : 12;
		u64 reserved_12_63                   : 52;
	} s;
	/* struct nixx_af_sdp_tx_fifo_status_s cn; */
};

static inline u64 NIXX_AF_SDP_TX_FIFO_STATUS(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_SDP_TX_FIFO_STATUS(void)
{
	return 0x650;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_seb_active_cycles_pc#
 *
 * NIX AF Active Cycles Register These registers are indexed by the
 * conditional clock domain number.
 */
union nixx_af_seb_active_cycles_pcx {
	u64 u;
	struct nixx_af_seb_active_cycles_pcx_s {
		u64 act_cyc                          : 64;
	} s;
	/* struct nixx_af_seb_active_cycles_pcx_s cn; */
};

static inline u64 NIXX_AF_SEB_ACTIVE_CYCLES_PCX(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_SEB_ACTIVE_CYCLES_PCX(u64 a)
{
	return 0x6c0 + 8 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_seb_bp_test
 *
 * INTERNAL: NIX AF SEB Backpressure Test Register
 */
union nixx_af_seb_bp_test {
	u64 u;
	struct nixx_af_seb_bp_test_s {
		u64 lfsr_freq                        : 12;
		u64 reserved_12_15                   : 4;
		u64 bp_cfg                           : 14;
		u64 reserved_30_47                   : 18;
		u64 enable                           : 7;
		u64 reserved_55_63                   : 9;
	} s;
	/* struct nixx_af_seb_bp_test_s cn; */
};

static inline u64 NIXX_AF_SEB_BP_TEST(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_SEB_BP_TEST(void)
{
	return 0x630;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_seb_cfg
 *
 * NIX SEB Configuration Register
 */
union nixx_af_seb_cfg {
	u64 u;
	struct nixx_af_seb_cfg_s {
		u64 sg_ndc_sel                       : 1;
		u64 reserved_1_63                    : 63;
	} s;
	/* struct nixx_af_seb_cfg_s cn; */
};

static inline u64 NIXX_AF_SEB_CFG(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_SEB_CFG(void)
{
	return 0x5f0;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_seb_eco
 *
 * INTERNAL: AF SEB ECO Register
 */
union nixx_af_seb_eco {
	u64 u;
	struct nixx_af_seb_eco_s {
		u64 eco_rw                           : 64;
	} s;
	/* struct nixx_af_seb_eco_s cn; */
};

static inline u64 NIXX_AF_SEB_ECO(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_SEB_ECO(void)
{
	return 0x5c0;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_seb_pipe_bp_test#
 *
 * INTERNAL: NIX AF SEB Pipe Backpressure Test Registers
 */
union nixx_af_seb_pipe_bp_testx {
	u64 u;
	struct nixx_af_seb_pipe_bp_testx_s {
		u64 lfsr_freq                        : 12;
		u64 reserved_12_15                   : 4;
		u64 bp_cfg                           : 24;
		u64 reserved_40_47                   : 8;
		u64 enable                           : 12;
		u64 reserved_60_63                   : 4;
	} s;
	/* struct nixx_af_seb_pipe_bp_testx_s cn; */
};

static inline u64 NIXX_AF_SEB_PIPE_BP_TESTX(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_SEB_PIPE_BP_TESTX(u64 a)
{
	return 0x600 + 0x10 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_seb_pipeb_bp_test#
 *
 * INTERNAL: NIX AF SEB Pipe Backpressure Test Registers
 */
union nixx_af_seb_pipeb_bp_testx {
	u64 u;
	struct nixx_af_seb_pipeb_bp_testx_s {
		u64 lfsr_freq                        : 12;
		u64 reserved_12_15                   : 4;
		u64 bp_cfg                           : 18;
		u64 reserved_34_47                   : 14;
		u64 enable                           : 9;
		u64 reserved_57_63                   : 7;
	} s;
	/* struct nixx_af_seb_pipeb_bp_testx_s cn; */
};

static inline u64 NIXX_AF_SEB_PIPEB_BP_TESTX(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_SEB_PIPEB_BP_TESTX(u64 a)
{
	return 0x608 + 0x10 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_seb_wd_tick_divider
 *
 * INTERNAL: NIX AF SEB TSTMP Watchdog Tick Divider Register
 */
union nixx_af_seb_wd_tick_divider {
	u64 u;
	struct nixx_af_seb_wd_tick_divider_s {
		u64 tick_div_cfg                     : 7;
		u64 reserved_7_63                    : 57;
	} s;
	/* struct nixx_af_seb_wd_tick_divider_s cn; */
};

static inline u64 NIXX_AF_SEB_WD_TICK_DIVIDER(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_SEB_WD_TICK_DIVIDER(void)
{
	return 0x6f0;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_smq#_cfg
 *
 * NIX AF SQM PSE Queue Configuration Registers
 */
union nixx_af_smqx_cfg {
	u64 u;
	struct nixx_af_smqx_cfg_s {
		u64 minlen                           : 7;
		u64 desc_shp_ctl_dis                 : 1;
		u64 maxlen                           : 16;
		u64 lf                               : 7;
		u64 reserved_31_35                   : 5;
		u64 max_vtag_ins                     : 3;
		u64 rr_minlen                        : 9;
		u64 express                          : 1;
		u64 flush                            : 1;
		u64 enq_xoff                         : 1;
		u64 pri_thr                          : 6;
		u64 reserved_57_63                   : 7;
	} s;
	/* struct nixx_af_smqx_cfg_s cn; */
};

static inline u64 NIXX_AF_SMQX_CFG(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_SMQX_CFG(u64 a)
{
	return 0x700 + 0x10000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_smq#_head
 *
 * NIX AF SQM SMQ Head Register These registers track the head of the SMQ
 * linked list.
 */
union nixx_af_smqx_head {
	u64 u;
	struct nixx_af_smqx_head_s {
		u64 sq_idx                           : 20;
		u64 valid                            : 1;
		u64 reserved_21_63                   : 43;
	} s;
	/* struct nixx_af_smqx_head_s cn; */
};

static inline u64 NIXX_AF_SMQX_HEAD(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_SMQX_HEAD(u64 a)
{
	return 0x710 + 0x10000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_smq#_nxt_head
 *
 * NIX AF SQM SMQ Next Head Register These registers track the next head
 * of the SMQ linked list.
 */
union nixx_af_smqx_nxt_head {
	u64 u;
	struct nixx_af_smqx_nxt_head_s {
		u64 sq_idx                           : 20;
		u64 valid                            : 1;
		u64 reserved_21_63                   : 43;
	} s;
	/* struct nixx_af_smqx_nxt_head_s cn; */
};

static inline u64 NIXX_AF_SMQX_NXT_HEAD(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_SMQX_NXT_HEAD(u64 a)
{
	return 0x740 + 0x10000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_smq#_status
 *
 * NIX AF SQM SMQ Status Register These registers track the status of the
 * SMQ FIFO.
 */
union nixx_af_smqx_status {
	u64 u;
	struct nixx_af_smqx_status_s {
		u64 level                            : 7;
		u64 reserved_7_63                    : 57;
	} s;
	/* struct nixx_af_smqx_status_s cn; */
};

static inline u64 NIXX_AF_SMQX_STATUS(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_SMQX_STATUS(u64 a)
{
	return 0x730 + 0x10000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_smq#_tail
 *
 * NIX AF SQM SMQ Head Register These registers track the tail of SMQ
 * linked list.
 */
union nixx_af_smqx_tail {
	u64 u;
	struct nixx_af_smqx_tail_s {
		u64 sq_idx                           : 20;
		u64 valid                            : 1;
		u64 reserved_21_63                   : 43;
	} s;
	/* struct nixx_af_smqx_tail_s cn; */
};

static inline u64 NIXX_AF_SMQX_TAIL(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_SMQX_TAIL(u64 a)
{
	return 0x720 + 0x10000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_sq_const
 *
 * NIX AF SQ Constants Register This register contains constants for
 * software discovery.
 */
union nixx_af_sq_const {
	u64 u;
	struct nixx_af_sq_const_s {
		u64 queues_per_lf                    : 24;
		u64 smq_depth                        : 10;
		u64 sqb_size                         : 16;
		u64 reserved_50_63                   : 14;
	} s;
	/* struct nixx_af_sq_const_s cn; */
};

static inline u64 NIXX_AF_SQ_CONST(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_SQ_CONST(void)
{
	return 0x40;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_sqm_active_cycles_pc
 *
 * NIX AF SQM Active Cycles Register These registers are indexed by the
 * conditional clock domain number.
 */
union nixx_af_sqm_active_cycles_pc {
	u64 u;
	struct nixx_af_sqm_active_cycles_pc_s {
		u64 act_cyc                          : 64;
	} s;
	/* struct nixx_af_sqm_active_cycles_pc_s cn; */
};

static inline u64 NIXX_AF_SQM_ACTIVE_CYCLES_PC(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_SQM_ACTIVE_CYCLES_PC(void)
{
	return 0x770;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_sqm_bp_test#
 *
 * INTERNAL: NIX AF SQM Backpressure Test Register
 */
union nixx_af_sqm_bp_testx {
	u64 u;
	struct nixx_af_sqm_bp_testx_s {
		u64 lfsr_freq                        : 12;
		u64 reserved_12_15                   : 4;
		u64 bp_cfg                           : 8;
		u64 reserved_24_59                   : 36;
		u64 enable                           : 4;
	} s;
	/* struct nixx_af_sqm_bp_testx_s cn; */
};

static inline u64 NIXX_AF_SQM_BP_TESTX(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_SQM_BP_TESTX(u64 a)
{
	return 0x760 + 0x10000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_sqm_dbg_ctl_status
 *
 * NIX AF SQM Debug Register This register is for SQM diagnostic use
 * only.
 */
union nixx_af_sqm_dbg_ctl_status {
	u64 u;
	struct nixx_af_sqm_dbg_ctl_status_s {
		u64 tm1                              : 8;
		u64 tm2                              : 1;
		u64 tm3                              : 4;
		u64 tm4                              : 1;
		u64 tm5                              : 1;
		u64 tm6                              : 1;
		u64 tm7                              : 4;
		u64 tm8                              : 1;
		u64 tm9                              : 1;
		u64 tm10                             : 1;
		u64 tm11                             : 1;
		u64 tm12                             : 1;
		u64 tm13                             : 1;
		u64 reserved_26_63                   : 38;
	} s;
	struct nixx_af_sqm_dbg_ctl_status_cn96xxp1 {
		u64 tm1                              : 8;
		u64 tm2                              : 1;
		u64 tm3                              : 4;
		u64 tm4                              : 1;
		u64 tm5                              : 1;
		u64 tm6                              : 1;
		u64 tm7                              : 4;
		u64 tm8                              : 1;
		u64 tm9                              : 1;
		u64 reserved_22_63                   : 42;
	} cn96xxp1;
	/* struct nixx_af_sqm_dbg_ctl_status_s cn96xxp3; */
	/* struct nixx_af_sqm_dbg_ctl_status_cn96xxp1 cnf95xxp1; */
	struct nixx_af_sqm_dbg_ctl_status_cnf95xxp2 {
		u64 tm1                              : 8;
		u64 tm2                              : 1;
		u64 tm3                              : 4;
		u64 tm4                              : 1;
		u64 tm5                              : 1;
		u64 tm6                              : 1;
		u64 tm7                              : 4;
		u64 tm8                              : 1;
		u64 tm9                              : 1;
		u64 reserved_22                      : 1;
		u64 reserved_23                      : 1;
		u64 reserved_24                      : 1;
		u64 reserved_25                      : 1;
		u64 reserved_26_63                   : 38;
	} cnf95xxp2;
};

static inline u64 NIXX_AF_SQM_DBG_CTL_STATUS(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_SQM_DBG_CTL_STATUS(void)
{
	return 0x750;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_sqm_eco
 *
 * INTERNAL: AF SQM ECO Register
 */
union nixx_af_sqm_eco {
	u64 u;
	struct nixx_af_sqm_eco_s {
		u64 eco_rw                           : 64;
	} s;
	/* struct nixx_af_sqm_eco_s cn; */
};

static inline u64 NIXX_AF_SQM_ECO(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_SQM_ECO(void)
{
	return 0x5b0;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_status
 *
 * NIX AF General Status Register
 */
union nixx_af_status {
	u64 u;
	struct nixx_af_status_s {
		u64 blk_busy                         : 10;
		u64 calibrate_done                   : 1;
		u64 reserved_11_15                   : 5;
		u64 calibrate_status                 : 15;
		u64 reserved_31_63                   : 33;
	} s;
	/* struct nixx_af_status_s cn; */
};

static inline u64 NIXX_AF_STATUS(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_STATUS(void)
{
	return 0x10;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_tcp_timer
 *
 * NIX TCP Timer Register
 */
union nixx_af_tcp_timer {
	u64 u;
	struct nixx_af_tcp_timer_s {
		u64 dur_counter                      : 16;
		u64 lf_counter                       : 8;
		u64 reserved_24_31                   : 8;
		u64 duration                         : 16;
		u64 reserved_48_62                   : 15;
		u64 ena                              : 1;
	} s;
	/* struct nixx_af_tcp_timer_s cn; */
};

static inline u64 NIXX_AF_TCP_TIMER(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_TCP_TIMER(void)
{
	return 0x1e0;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_tl1#_cir
 *
 * NIX AF Transmit Level 1 Committed Information Rate Register
 */
union nixx_af_tl1x_cir {
	u64 u;
	struct nixx_af_tl1x_cir_s {
		u64 enable                           : 1;
		u64 rate_mantissa                    : 8;
		u64 rate_exponent                    : 4;
		u64 rate_divider_exponent            : 4;
		u64 reserved_17_28                   : 12;
		u64 burst_mantissa                   : 8;
		u64 burst_exponent                   : 4;
		u64 reserved_41_63                   : 23;
	} s;
	/* struct nixx_af_tl1x_cir_s cn; */
};

static inline u64 NIXX_AF_TL1X_CIR(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_TL1X_CIR(u64 a)
{
	return 0xc20 + 0x10000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_tl1#_dropped_bytes
 *
 * NIX AF Transmit Level 1 Dropped Bytes Registers This register has the
 * same bit fields as NIX_AF_TL1()_GREEN_BYTES.
 */
union nixx_af_tl1x_dropped_bytes {
	u64 u;
	struct nixx_af_tl1x_dropped_bytes_s {
		u64 count                            : 48;
		u64 reserved_48_63                   : 16;
	} s;
	/* struct nixx_af_tl1x_dropped_bytes_s cn; */
};

static inline u64 NIXX_AF_TL1X_DROPPED_BYTES(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_TL1X_DROPPED_BYTES(u64 a)
{
	return 0xd30 + 0x10000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_tl1#_dropped_packets
 *
 * NIX AF Transmit Level 1 Dropped Packets Registers This register has
 * the same bit fields as NIX_AF_TL1()_GREEN_PACKETS.
 */
union nixx_af_tl1x_dropped_packets {
	u64 u;
	struct nixx_af_tl1x_dropped_packets_s {
		u64 count                            : 40;
		u64 reserved_40_63                   : 24;
	} s;
	/* struct nixx_af_tl1x_dropped_packets_s cn; */
};

static inline u64 NIXX_AF_TL1X_DROPPED_PACKETS(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_TL1X_DROPPED_PACKETS(u64 a)
{
	return 0xd20 + 0x10000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_tl1#_green
 *
 * INTERNAL: NIX Transmit Level 1 Green State Debug Register
 */
union nixx_af_tl1x_green {
	u64 u;
	struct nixx_af_tl1x_green_s {
		u64 tail                             : 8;
		u64 reserved_8_9                     : 2;
		u64 head                             : 8;
		u64 reserved_18_19                   : 2;
		u64 active_vec                       : 20;
		u64 rr_active                        : 1;
		u64 reserved_41_63                   : 23;
	} s;
	/* struct nixx_af_tl1x_green_s cn; */
};

static inline u64 NIXX_AF_TL1X_GREEN(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_TL1X_GREEN(u64 a)
{
	return 0xc90 + 0x10000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_tl1#_green_bytes
 *
 * NIX AF Transmit Level 1 Green Sent Bytes Registers
 */
union nixx_af_tl1x_green_bytes {
	u64 u;
	struct nixx_af_tl1x_green_bytes_s {
		u64 count                            : 48;
		u64 reserved_48_63                   : 16;
	} s;
	/* struct nixx_af_tl1x_green_bytes_s cn; */
};

static inline u64 NIXX_AF_TL1X_GREEN_BYTES(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_TL1X_GREEN_BYTES(u64 a)
{
	return 0xd90 + 0x10000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_tl1#_green_packets
 *
 * NIX AF Transmit Level 1 Green Sent Packets Registers
 */
union nixx_af_tl1x_green_packets {
	u64 u;
	struct nixx_af_tl1x_green_packets_s {
		u64 count                            : 40;
		u64 reserved_40_63                   : 24;
	} s;
	/* struct nixx_af_tl1x_green_packets_s cn; */
};

static inline u64 NIXX_AF_TL1X_GREEN_PACKETS(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_TL1X_GREEN_PACKETS(u64 a)
{
	return 0xd80 + 0x10000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_tl1#_md_debug0
 *
 * NIX AF Transmit Level 1 Meta Descriptor Debug 0 Registers
 * NIX_AF_TL1()_MD_DEBUG0, NIX_AF_TL1()_MD_DEBUG1, NIX_AF_TL1()_MD_DEBUG2
 * and NIX_AF_TL1()_MD_DEBUG3 provide access to the TLn queue meta
 * descriptor. A TLn queue can hold up to two packet meta descriptors
 * (PMD) and one flush meta descriptor (FMD): * PMD0 state is accessed
 * with [PMD0_VLD], [PMD0_LENGTH] and NIX_AF_TL1()_MD_DEBUG1. * PMD1 is
 * accessed with [PMD1_VLD], [PMD1_LENGTH] and NIX_AF_TL1()_MD_DEBUG2. *
 * FMD is accessed with NIX_AF_TL1()_MD_DEBUG3.
 */
union nixx_af_tl1x_md_debug0 {
	u64 u;
	struct nixx_af_tl1x_md_debug0_s {
		u64 pmd0_length                      : 16;
		u64 pmd1_length                      : 16;
		u64 pmd0_vld                         : 1;
		u64 pmd1_vld                         : 1;
		u64 reserved_34_45                   : 12;
		u64 drain_pri                        : 1;
		u64 drain                            : 1;
		u64 c_con                            : 1;
		u64 p_con                            : 1;
		u64 reserved_50_51                   : 2;
		u64 child                            : 10;
		u64 reserved_62                      : 1;
		u64 pmd_count                        : 1;
	} s;
	/* struct nixx_af_tl1x_md_debug0_s cn96xxp1; */
	struct nixx_af_tl1x_md_debug0_cn96xxp3 {
		u64 pmd0_length                      : 16;
		u64 reserved_16_31                   : 16;
		u64 pmd0_vld                         : 1;
		u64 reserved_33                      : 1;
		u64 reserved_34_45                   : 12;
		u64 reserved_46                      : 1;
		u64 reserved_47                      : 1;
		u64 c_con                            : 1;
		u64 p_con                            : 1;
		u64 reserved_50_51                   : 2;
		u64 child                            : 10;
		u64 reserved_62                      : 1;
		u64 reserved_63                      : 1;
	} cn96xxp3;
	/* struct nixx_af_tl1x_md_debug0_s cnf95xx; */
};

static inline u64 NIXX_AF_TL1X_MD_DEBUG0(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_TL1X_MD_DEBUG0(u64 a)
{
	return 0xcc0 + 0x10000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_tl1#_md_debug1
 *
 * NIX AF Transmit Level 1 Meta Descriptor Debug 1 Registers Packet meta
 * descriptor 0 debug. See NIX_AF_TL1()_MD_DEBUG0.
 */
union nixx_af_tl1x_md_debug1 {
	u64 u;
	struct nixx_af_tl1x_md_debug1_s {
		u64 reserved_0_5                     : 6;
		u64 red_algo_override                : 2;
		u64 cir_dis                          : 1;
		u64 pir_dis                          : 1;
		u64 adjust                           : 9;
		u64 uid                              : 4;
		u64 reserved_23                      : 1;
		u64 bubble                           : 1;
		u64 color                            : 2;
		u64 pse_pkt_id                       : 9;
		u64 reserved_36                      : 1;
		u64 tx_pkt_p2x                       : 2;
		u64 sqm_pkt_id                       : 13;
		u64 mdq_idx                          : 10;
		u64 reserved_62                      : 1;
		u64 vld                              : 1;
	} s;
	struct nixx_af_tl1x_md_debug1_cn96xxp1 {
		u64 reserved_0_5                     : 6;
		u64 red_algo_override                : 2;
		u64 cir_dis                          : 1;
		u64 pir_dis                          : 1;
		u64 adjust                           : 9;
		u64 uid                              : 4;
		u64 drain                            : 1;
		u64 bubble                           : 1;
		u64 color                            : 2;
		u64 pse_pkt_id                       : 9;
		u64 reserved_36                      : 1;
		u64 tx_pkt_p2x                       : 2;
		u64 sqm_pkt_id                       : 13;
		u64 mdq_idx                          : 10;
		u64 reserved_62                      : 1;
		u64 vld                              : 1;
	} cn96xxp1;
	struct nixx_af_tl1x_md_debug1_cn96xxp3 {
		u64 reserved_0_5                     : 6;
		u64 red_algo_override                : 2;
		u64 cir_dis                          : 1;
		u64 pir_dis                          : 1;
		u64 adjust                           : 9;
		u64 reserved_19_22                   : 4;
		u64 flush                            : 1;
		u64 bubble                           : 1;
		u64 color                            : 2;
		u64 pse_pkt_id                       : 9;
		u64 reserved_36                      : 1;
		u64 tx_pkt_p2x                       : 2;
		u64 sqm_pkt_id                       : 13;
		u64 mdq_idx                          : 10;
		u64 reserved_62                      : 1;
		u64 vld                              : 1;
	} cn96xxp3;
	/* struct nixx_af_tl1x_md_debug1_cn96xxp1 cnf95xx; */
};

static inline u64 NIXX_AF_TL1X_MD_DEBUG1(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_TL1X_MD_DEBUG1(u64 a)
{
	return 0xcc8 + 0x10000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_tl1#_md_debug2
 *
 * NIX AF Transmit Level 1 Meta Descriptor Debug 2 Registers Packet meta
 * descriptor 1 debug. See NIX_AF_TL1()_MD_DEBUG0.
 */
union nixx_af_tl1x_md_debug2 {
	u64 u;
	struct nixx_af_tl1x_md_debug2_s {
		u64 reserved_0_5                     : 6;
		u64 red_algo_override                : 2;
		u64 cir_dis                          : 1;
		u64 pir_dis                          : 1;
		u64 adjust                           : 9;
		u64 uid                              : 4;
		u64 reserved_23                      : 1;
		u64 bubble                           : 1;
		u64 color                            : 2;
		u64 pse_pkt_id                       : 9;
		u64 reserved_36                      : 1;
		u64 tx_pkt_p2x                       : 2;
		u64 sqm_pkt_id                       : 13;
		u64 mdq_idx                          : 10;
		u64 reserved_62                      : 1;
		u64 vld                              : 1;
	} s;
	struct nixx_af_tl1x_md_debug2_cn96xxp1 {
		u64 reserved_0_5                     : 6;
		u64 red_algo_override                : 2;
		u64 cir_dis                          : 1;
		u64 pir_dis                          : 1;
		u64 adjust                           : 9;
		u64 uid                              : 4;
		u64 drain                            : 1;
		u64 bubble                           : 1;
		u64 color                            : 2;
		u64 pse_pkt_id                       : 9;
		u64 reserved_36                      : 1;
		u64 tx_pkt_p2x                       : 2;
		u64 sqm_pkt_id                       : 13;
		u64 mdq_idx                          : 10;
		u64 reserved_62                      : 1;
		u64 vld                              : 1;
	} cn96xxp1;
	struct nixx_af_tl1x_md_debug2_cn96xxp3 {
		u64 reserved_0_5                     : 6;
		u64 red_algo_override                : 2;
		u64 cir_dis                          : 1;
		u64 pir_dis                          : 1;
		u64 adjust                           : 9;
		u64 reserved_19_22                   : 4;
		u64 flush                            : 1;
		u64 bubble                           : 1;
		u64 color                            : 2;
		u64 pse_pkt_id                       : 9;
		u64 reserved_36                      : 1;
		u64 tx_pkt_p2x                       : 2;
		u64 sqm_pkt_id                       : 13;
		u64 mdq_idx                          : 10;
		u64 reserved_62                      : 1;
		u64 vld                              : 1;
	} cn96xxp3;
	/* struct nixx_af_tl1x_md_debug2_cn96xxp1 cnf95xx; */
};

static inline u64 NIXX_AF_TL1X_MD_DEBUG2(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_TL1X_MD_DEBUG2(u64 a)
{
	return 0xcd0 + 0x10000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_tl1#_md_debug3
 *
 * NIX AF Transmit Level 1 Meta Descriptor Debug 3 Registers Flush meta
 * descriptor debug. See NIX_AF_TL1()_MD_DEBUG0.
 */
union nixx_af_tl1x_md_debug3 {
	u64 u;
	struct nixx_af_tl1x_md_debug3_s {
		u64 reserved_0_36                    : 37;
		u64 tx_pkt_p2x                       : 2;
		u64 sqm_pkt_id                       : 13;
		u64 mdq_idx                          : 10;
		u64 reserved_62                      : 1;
		u64 vld                              : 1;
	} s;
	/* struct nixx_af_tl1x_md_debug3_s cn96xxp1; */
	struct nixx_af_tl1x_md_debug3_cn96xxp3 {
		u64 reserved_0_36                    : 37;
		u64 reserved_37_38                   : 2;
		u64 reserved_39_51                   : 13;
		u64 reserved_52_61                   : 10;
		u64 reserved_62                      : 1;
		u64 reserved_63                      : 1;
	} cn96xxp3;
	/* struct nixx_af_tl1x_md_debug3_s cnf95xx; */
};

static inline u64 NIXX_AF_TL1X_MD_DEBUG3(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_TL1X_MD_DEBUG3(u64 a)
{
	return 0xcd8 + 0x10000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_tl1#_red
 *
 * INTERNAL: NIX Transmit Level 1 Red State Debug Register  This register
 * has the same bit fields as NIX_AF_TL1()_YELLOW.
 */
union nixx_af_tl1x_red {
	u64 u;
	struct nixx_af_tl1x_red_s {
		u64 tail                             : 8;
		u64 reserved_8_9                     : 2;
		u64 head                             : 8;
		u64 reserved_18_63                   : 46;
	} s;
	/* struct nixx_af_tl1x_red_s cn; */
};

static inline u64 NIXX_AF_TL1X_RED(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_TL1X_RED(u64 a)
{
	return 0xcb0 + 0x10000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_tl1#_red_bytes
 *
 * NIX AF Transmit Level 1 Red Sent Bytes Registers This register has the
 * same bit fields as NIX_AF_TL1()_GREEN_BYTES.
 */
union nixx_af_tl1x_red_bytes {
	u64 u;
	struct nixx_af_tl1x_red_bytes_s {
		u64 count                            : 48;
		u64 reserved_48_63                   : 16;
	} s;
	/* struct nixx_af_tl1x_red_bytes_s cn; */
};

static inline u64 NIXX_AF_TL1X_RED_BYTES(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_TL1X_RED_BYTES(u64 a)
{
	return 0xd50 + 0x10000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_tl1#_red_packets
 *
 * NIX AF Transmit Level 1 Red Sent Packets Registers This register has
 * the same bit fields as NIX_AF_TL1()_GREEN_PACKETS.
 */
union nixx_af_tl1x_red_packets {
	u64 u;
	struct nixx_af_tl1x_red_packets_s {
		u64 count                            : 40;
		u64 reserved_40_63                   : 24;
	} s;
	/* struct nixx_af_tl1x_red_packets_s cn; */
};

static inline u64 NIXX_AF_TL1X_RED_PACKETS(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_TL1X_RED_PACKETS(u64 a)
{
	return 0xd40 + 0x10000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_tl1#_schedule
 *
 * NIX AF Transmit Level 1 Scheduling Control Register
 */
union nixx_af_tl1x_schedule {
	u64 u;
	struct nixx_af_tl1x_schedule_s {
		u64 rr_quantum                       : 24;
		u64 reserved_24_63                   : 40;
	} s;
	/* struct nixx_af_tl1x_schedule_s cn; */
};

static inline u64 NIXX_AF_TL1X_SCHEDULE(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_TL1X_SCHEDULE(u64 a)
{
	return 0xc00 + 0x10000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_tl1#_shape
 *
 * NIX AF Transmit Level 1 Shaping Control Register
 */
union nixx_af_tl1x_shape {
	u64 u;
	struct nixx_af_tl1x_shape_s {
		u64 adjust                           : 9;
		u64 reserved_9_23                    : 15;
		u64 length_disable                   : 1;
		u64 reserved_25_63                   : 39;
	} s;
	struct nixx_af_tl1x_shape_cn {
		u64 adjust                           : 9;
		u64 reserved_9_17                    : 9;
		u64 reserved_18_23                   : 6;
		u64 length_disable                   : 1;
		u64 reserved_25_63                   : 39;
	} cn;
};

static inline u64 NIXX_AF_TL1X_SHAPE(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_TL1X_SHAPE(u64 a)
{
	return 0xc10 + 0x10000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_tl1#_shape_state
 *
 * NIX AF Transmit Level 1 Shape State Register This register must not be
 * written during normal operation.
 */
union nixx_af_tl1x_shape_state {
	u64 u;
	struct nixx_af_tl1x_shape_state_s {
		u64 cir_accum                        : 26;
		u64 reserved_26_51                   : 26;
		u64 color                            : 1;
		u64 reserved_53_63                   : 11;
	} s;
	/* struct nixx_af_tl1x_shape_state_s cn; */
};

static inline u64 NIXX_AF_TL1X_SHAPE_STATE(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_TL1X_SHAPE_STATE(u64 a)
{
	return 0xc50 + 0x10000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_tl1#_sw_xoff
 *
 * NIX AF Transmit Level 1 Software Controlled XOFF Registers
 */
union nixx_af_tl1x_sw_xoff {
	u64 u;
	struct nixx_af_tl1x_sw_xoff_s {
		u64 xoff                             : 1;
		u64 drain                            : 1;
		u64 reserved_2                       : 1;
		u64 drain_irq                        : 1;
		u64 reserved_4_63                    : 60;
	} s;
	/* struct nixx_af_tl1x_sw_xoff_s cn; */
};

static inline u64 NIXX_AF_TL1X_SW_XOFF(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_TL1X_SW_XOFF(u64 a)
{
	return 0xc70 + 0x10000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_tl1#_topology
 *
 * NIX AF Transmit Level 1 Topology Registers
 */
union nixx_af_tl1x_topology {
	u64 u;
	struct nixx_af_tl1x_topology_s {
		u64 reserved_0                       : 1;
		u64 rr_prio                          : 4;
		u64 reserved_5_31                    : 27;
		u64 prio_anchor                      : 8;
		u64 reserved_40_63                   : 24;
	} s;
	/* struct nixx_af_tl1x_topology_s cn; */
};

static inline u64 NIXX_AF_TL1X_TOPOLOGY(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_TL1X_TOPOLOGY(u64 a)
{
	return 0xc80 + 0x10000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_tl1#_yellow
 *
 * INTERNAL: NIX Transmit Level 1 Yellow State Debug Register
 */
union nixx_af_tl1x_yellow {
	u64 u;
	struct nixx_af_tl1x_yellow_s {
		u64 tail                             : 8;
		u64 reserved_8_9                     : 2;
		u64 head                             : 8;
		u64 reserved_18_63                   : 46;
	} s;
	/* struct nixx_af_tl1x_yellow_s cn; */
};

static inline u64 NIXX_AF_TL1X_YELLOW(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_TL1X_YELLOW(u64 a)
{
	return 0xca0 + 0x10000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_tl1#_yellow_bytes
 *
 * NIX AF Transmit Level 1 Yellow Sent Bytes Registers This register has
 * the same bit fields as NIX_AF_TL1()_GREEN_BYTES.
 */
union nixx_af_tl1x_yellow_bytes {
	u64 u;
	struct nixx_af_tl1x_yellow_bytes_s {
		u64 count                            : 48;
		u64 reserved_48_63                   : 16;
	} s;
	/* struct nixx_af_tl1x_yellow_bytes_s cn; */
};

static inline u64 NIXX_AF_TL1X_YELLOW_BYTES(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_TL1X_YELLOW_BYTES(u64 a)
{
	return 0xd70 + 0x10000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_tl1#_yellow_packets
 *
 * NIX AF Transmit Level 1 Yellow Sent Packets Registers This register
 * has the same bit fields as NIX_AF_TL1()_GREEN_PACKETS.
 */
union nixx_af_tl1x_yellow_packets {
	u64 u;
	struct nixx_af_tl1x_yellow_packets_s {
		u64 count                            : 40;
		u64 reserved_40_63                   : 24;
	} s;
	/* struct nixx_af_tl1x_yellow_packets_s cn; */
};

static inline u64 NIXX_AF_TL1X_YELLOW_PACKETS(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_TL1X_YELLOW_PACKETS(u64 a)
{
	return 0xd60 + 0x10000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_tl1_const
 *
 * NIX AF Transmit Level 1 Constants Register This register contains
 * constants for software discovery.
 */
union nixx_af_tl1_const {
	u64 u;
	struct nixx_af_tl1_const_s {
		u64 count                            : 16;
		u64 reserved_16_63                   : 48;
	} s;
	/* struct nixx_af_tl1_const_s cn; */
};

static inline u64 NIXX_AF_TL1_CONST(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_TL1_CONST(void)
{
	return 0x70;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_tl2#_cir
 *
 * NIX AF Transmit Level 2 Committed Information Rate Registers This
 * register has the same bit fields as NIX_AF_TL1()_CIR.
 */
union nixx_af_tl2x_cir {
	u64 u;
	struct nixx_af_tl2x_cir_s {
		u64 enable                           : 1;
		u64 rate_mantissa                    : 8;
		u64 rate_exponent                    : 4;
		u64 rate_divider_exponent            : 4;
		u64 reserved_17_28                   : 12;
		u64 burst_mantissa                   : 8;
		u64 burst_exponent                   : 4;
		u64 reserved_41_63                   : 23;
	} s;
	/* struct nixx_af_tl2x_cir_s cn; */
};

static inline u64 NIXX_AF_TL2X_CIR(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_TL2X_CIR(u64 a)
{
	return 0xe20 + 0x10000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_tl2#_green
 *
 * INTERNAL: NIX Transmit Level 2 Green State Debug Register  This
 * register has the same bit fields as NIX_AF_TL1()_GREEN.
 */
union nixx_af_tl2x_green {
	u64 u;
	struct nixx_af_tl2x_green_s {
		u64 tail                             : 8;
		u64 reserved_8_9                     : 2;
		u64 head                             : 8;
		u64 reserved_18_19                   : 2;
		u64 active_vec                       : 20;
		u64 rr_active                        : 1;
		u64 reserved_41_63                   : 23;
	} s;
	/* struct nixx_af_tl2x_green_s cn; */
};

static inline u64 NIXX_AF_TL2X_GREEN(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_TL2X_GREEN(u64 a)
{
	return 0xe90 + 0x10000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_tl2#_md_debug0
 *
 * NIX AF Transmit Level 2 Meta Descriptor Debug 0 Registers See
 * NIX_AF_TL1()_MD_DEBUG0
 */
union nixx_af_tl2x_md_debug0 {
	u64 u;
	struct nixx_af_tl2x_md_debug0_s {
		u64 pmd0_length                      : 16;
		u64 pmd1_length                      : 16;
		u64 pmd0_vld                         : 1;
		u64 pmd1_vld                         : 1;
		u64 reserved_34_45                   : 12;
		u64 drain_pri                        : 1;
		u64 drain                            : 1;
		u64 c_con                            : 1;
		u64 p_con                            : 1;
		u64 reserved_50_51                   : 2;
		u64 child                            : 10;
		u64 reserved_62                      : 1;
		u64 pmd_count                        : 1;
	} s;
	/* struct nixx_af_tl2x_md_debug0_s cn96xxp1; */
	struct nixx_af_tl2x_md_debug0_cn96xxp3 {
		u64 pmd0_length                      : 16;
		u64 reserved_16_31                   : 16;
		u64 pmd0_vld                         : 1;
		u64 reserved_33                      : 1;
		u64 reserved_34_45                   : 12;
		u64 reserved_46                      : 1;
		u64 reserved_47                      : 1;
		u64 c_con                            : 1;
		u64 p_con                            : 1;
		u64 reserved_50_51                   : 2;
		u64 child                            : 10;
		u64 reserved_62                      : 1;
		u64 reserved_63                      : 1;
	} cn96xxp3;
	/* struct nixx_af_tl2x_md_debug0_s cnf95xx; */
};

static inline u64 NIXX_AF_TL2X_MD_DEBUG0(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_TL2X_MD_DEBUG0(u64 a)
{
	return 0xec0 + 0x10000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_tl2#_md_debug1
 *
 * NIX AF Transmit Level 2 Meta Descriptor Debug 1 Registers Packet meta
 * descriptor 0 debug. See NIX_AF_TL1()_MD_DEBUG0.
 */
union nixx_af_tl2x_md_debug1 {
	u64 u;
	struct nixx_af_tl2x_md_debug1_s {
		u64 reserved_0_5                     : 6;
		u64 red_algo_override                : 2;
		u64 cir_dis                          : 1;
		u64 pir_dis                          : 1;
		u64 adjust                           : 9;
		u64 uid                              : 4;
		u64 reserved_23                      : 1;
		u64 bubble                           : 1;
		u64 color                            : 2;
		u64 pse_pkt_id                       : 9;
		u64 reserved_36                      : 1;
		u64 tx_pkt_p2x                       : 2;
		u64 sqm_pkt_id                       : 13;
		u64 mdq_idx                          : 10;
		u64 reserved_62                      : 1;
		u64 vld                              : 1;
	} s;
	struct nixx_af_tl2x_md_debug1_cn96xxp1 {
		u64 reserved_0_5                     : 6;
		u64 red_algo_override                : 2;
		u64 cir_dis                          : 1;
		u64 pir_dis                          : 1;
		u64 adjust                           : 9;
		u64 uid                              : 4;
		u64 drain                            : 1;
		u64 bubble                           : 1;
		u64 color                            : 2;
		u64 pse_pkt_id                       : 9;
		u64 reserved_36                      : 1;
		u64 tx_pkt_p2x                       : 2;
		u64 sqm_pkt_id                       : 13;
		u64 mdq_idx                          : 10;
		u64 reserved_62                      : 1;
		u64 vld                              : 1;
	} cn96xxp1;
	struct nixx_af_tl2x_md_debug1_cn96xxp3 {
		u64 reserved_0_5                     : 6;
		u64 red_algo_override                : 2;
		u64 cir_dis                          : 1;
		u64 pir_dis                          : 1;
		u64 adjust                           : 9;
		u64 reserved_19_22                   : 4;
		u64 flush                            : 1;
		u64 bubble                           : 1;
		u64 color                            : 2;
		u64 pse_pkt_id                       : 9;
		u64 reserved_36                      : 1;
		u64 tx_pkt_p2x                       : 2;
		u64 sqm_pkt_id                       : 13;
		u64 mdq_idx                          : 10;
		u64 reserved_62                      : 1;
		u64 vld                              : 1;
	} cn96xxp3;
	/* struct nixx_af_tl2x_md_debug1_cn96xxp1 cnf95xx; */
};

static inline u64 NIXX_AF_TL2X_MD_DEBUG1(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_TL2X_MD_DEBUG1(u64 a)
{
	return 0xec8 + 0x10000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_tl2#_md_debug2
 *
 * NIX AF Transmit Level 2 Meta Descriptor Debug 2 Registers Packet meta
 * descriptor 1 debug. See NIX_AF_TL1()_MD_DEBUG0.
 */
union nixx_af_tl2x_md_debug2 {
	u64 u;
	struct nixx_af_tl2x_md_debug2_s {
		u64 reserved_0_5                     : 6;
		u64 red_algo_override                : 2;
		u64 cir_dis                          : 1;
		u64 pir_dis                          : 1;
		u64 adjust                           : 9;
		u64 uid                              : 4;
		u64 reserved_23                      : 1;
		u64 bubble                           : 1;
		u64 color                            : 2;
		u64 pse_pkt_id                       : 9;
		u64 reserved_36                      : 1;
		u64 tx_pkt_p2x                       : 2;
		u64 sqm_pkt_id                       : 13;
		u64 mdq_idx                          : 10;
		u64 reserved_62                      : 1;
		u64 vld                              : 1;
	} s;
	struct nixx_af_tl2x_md_debug2_cn96xxp1 {
		u64 reserved_0_5                     : 6;
		u64 red_algo_override                : 2;
		u64 cir_dis                          : 1;
		u64 pir_dis                          : 1;
		u64 adjust                           : 9;
		u64 uid                              : 4;
		u64 drain                            : 1;
		u64 bubble                           : 1;
		u64 color                            : 2;
		u64 pse_pkt_id                       : 9;
		u64 reserved_36                      : 1;
		u64 tx_pkt_p2x                       : 2;
		u64 sqm_pkt_id                       : 13;
		u64 mdq_idx                          : 10;
		u64 reserved_62                      : 1;
		u64 vld                              : 1;
	} cn96xxp1;
	struct nixx_af_tl2x_md_debug2_cn96xxp3 {
		u64 reserved_0_5                     : 6;
		u64 red_algo_override                : 2;
		u64 cir_dis                          : 1;
		u64 pir_dis                          : 1;
		u64 adjust                           : 9;
		u64 reserved_19_22                   : 4;
		u64 flush                            : 1;
		u64 bubble                           : 1;
		u64 color                            : 2;
		u64 pse_pkt_id                       : 9;
		u64 reserved_36                      : 1;
		u64 tx_pkt_p2x                       : 2;
		u64 sqm_pkt_id                       : 13;
		u64 mdq_idx                          : 10;
		u64 reserved_62                      : 1;
		u64 vld                              : 1;
	} cn96xxp3;
	/* struct nixx_af_tl2x_md_debug2_cn96xxp1 cnf95xx; */
};

static inline u64 NIXX_AF_TL2X_MD_DEBUG2(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_TL2X_MD_DEBUG2(u64 a)
{
	return 0xed0 + 0x10000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_tl2#_md_debug3
 *
 * NIX AF Transmit Level 2 Meta Descriptor Debug 3 Registers Flush meta
 * descriptor debug. See NIX_AF_TL1()_MD_DEBUG0.
 */
union nixx_af_tl2x_md_debug3 {
	u64 u;
	struct nixx_af_tl2x_md_debug3_s {
		u64 reserved_0_36                    : 37;
		u64 tx_pkt_p2x                       : 2;
		u64 sqm_pkt_id                       : 13;
		u64 mdq_idx                          : 10;
		u64 reserved_62                      : 1;
		u64 vld                              : 1;
	} s;
	/* struct nixx_af_tl2x_md_debug3_s cn96xxp1; */
	struct nixx_af_tl2x_md_debug3_cn96xxp3 {
		u64 reserved_0_36                    : 37;
		u64 reserved_37_38                   : 2;
		u64 reserved_39_51                   : 13;
		u64 reserved_52_61                   : 10;
		u64 reserved_62                      : 1;
		u64 reserved_63                      : 1;
	} cn96xxp3;
	/* struct nixx_af_tl2x_md_debug3_s cnf95xx; */
};

static inline u64 NIXX_AF_TL2X_MD_DEBUG3(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_TL2X_MD_DEBUG3(u64 a)
{
	return 0xed8 + 0x10000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_tl2#_parent
 *
 * NIX AF Transmit Level 2 Parent Registers
 */
union nixx_af_tl2x_parent {
	u64 u;
	struct nixx_af_tl2x_parent_s {
		u64 reserved_0_15                    : 16;
		u64 parent                           : 5;
		u64 reserved_21_63                   : 43;
	} s;
	/* struct nixx_af_tl2x_parent_s cn; */
};

static inline u64 NIXX_AF_TL2X_PARENT(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_TL2X_PARENT(u64 a)
{
	return 0xe88 + 0x10000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_tl2#_pir
 *
 * NIX AF Transmit Level 2 Peak Information Rate Registers This register
 * has the same bit fields as NIX_AF_TL1()_CIR.
 */
union nixx_af_tl2x_pir {
	u64 u;
	struct nixx_af_tl2x_pir_s {
		u64 enable                           : 1;
		u64 rate_mantissa                    : 8;
		u64 rate_exponent                    : 4;
		u64 rate_divider_exponent            : 4;
		u64 reserved_17_28                   : 12;
		u64 burst_mantissa                   : 8;
		u64 burst_exponent                   : 4;
		u64 reserved_41_63                   : 23;
	} s;
	/* struct nixx_af_tl2x_pir_s cn; */
};

static inline u64 NIXX_AF_TL2X_PIR(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_TL2X_PIR(u64 a)
{
	return 0xe30 + 0x10000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_tl2#_pointers
 *
 * INTERNAL: NIX Transmit Level 2 Linked List Pointers Debug Register
 */
union nixx_af_tl2x_pointers {
	u64 u;
	struct nixx_af_tl2x_pointers_s {
		u64 next                             : 8;
		u64 reserved_8_15                    : 8;
		u64 prev                             : 8;
		u64 reserved_24_63                   : 40;
	} s;
	/* struct nixx_af_tl2x_pointers_s cn; */
};

static inline u64 NIXX_AF_TL2X_POINTERS(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_TL2X_POINTERS(u64 a)
{
	return 0xe60 + 0x10000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_tl2#_red
 *
 * INTERNAL: NIX Transmit Level 2 Red State Debug Register  This register
 * has the same bit fields as NIX_AF_TL1()_RED.
 */
union nixx_af_tl2x_red {
	u64 u;
	struct nixx_af_tl2x_red_s {
		u64 tail                             : 8;
		u64 reserved_8_9                     : 2;
		u64 head                             : 8;
		u64 reserved_18_63                   : 46;
	} s;
	/* struct nixx_af_tl2x_red_s cn; */
};

static inline u64 NIXX_AF_TL2X_RED(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_TL2X_RED(u64 a)
{
	return 0xeb0 + 0x10000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_tl2#_sched_state
 *
 * NIX AF Transmit Level 2 Scheduling Control State Registers
 */
union nixx_af_tl2x_sched_state {
	u64 u;
	struct nixx_af_tl2x_sched_state_s {
		u64 rr_count                         : 25;
		u64 reserved_25_63                   : 39;
	} s;
	/* struct nixx_af_tl2x_sched_state_s cn; */
};

static inline u64 NIXX_AF_TL2X_SCHED_STATE(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_TL2X_SCHED_STATE(u64 a)
{
	return 0xe40 + 0x10000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_tl2#_schedule
 *
 * NIX AF Transmit Level 2 Scheduling Control Registers
 */
union nixx_af_tl2x_schedule {
	u64 u;
	struct nixx_af_tl2x_schedule_s {
		u64 rr_quantum                       : 24;
		u64 prio                             : 4;
		u64 reserved_28_63                   : 36;
	} s;
	/* struct nixx_af_tl2x_schedule_s cn; */
};

static inline u64 NIXX_AF_TL2X_SCHEDULE(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_TL2X_SCHEDULE(u64 a)
{
	return 0xe00 + 0x10000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_tl2#_shape
 *
 * NIX AF Transmit Level 2 Shaping Control Registers
 */
union nixx_af_tl2x_shape {
	u64 u;
	struct nixx_af_tl2x_shape_s {
		u64 adjust                           : 9;
		u64 red_algo                         : 2;
		u64 red_disable                      : 1;
		u64 yellow_disable                   : 1;
		u64 reserved_13_23                   : 11;
		u64 length_disable                   : 1;
		u64 schedule_list                    : 2;
		u64 reserved_27_63                   : 37;
	} s;
	/* struct nixx_af_tl2x_shape_s cn; */
};

static inline u64 NIXX_AF_TL2X_SHAPE(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_TL2X_SHAPE(u64 a)
{
	return 0xe10 + 0x10000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_tl2#_shape_state
 *
 * NIX AF Transmit Level 2 Shape State Registers This register must not
 * be written during normal operation.
 */
union nixx_af_tl2x_shape_state {
	u64 u;
	struct nixx_af_tl2x_shape_state_s {
		u64 cir_accum                        : 26;
		u64 pir_accum                        : 26;
		u64 color                            : 2;
		u64 reserved_54_63                   : 10;
	} s;
	/* struct nixx_af_tl2x_shape_state_s cn; */
};

static inline u64 NIXX_AF_TL2X_SHAPE_STATE(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_TL2X_SHAPE_STATE(u64 a)
{
	return 0xe50 + 0x10000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_tl2#_sw_xoff
 *
 * NIX AF Transmit Level 2 Software Controlled XOFF Registers This
 * register has the same bit fields as NIX_AF_TL1()_SW_XOFF.
 */
union nixx_af_tl2x_sw_xoff {
	u64 u;
	struct nixx_af_tl2x_sw_xoff_s {
		u64 xoff                             : 1;
		u64 drain                            : 1;
		u64 reserved_2                       : 1;
		u64 drain_irq                        : 1;
		u64 reserved_4_63                    : 60;
	} s;
	/* struct nixx_af_tl2x_sw_xoff_s cn; */
};

static inline u64 NIXX_AF_TL2X_SW_XOFF(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_TL2X_SW_XOFF(u64 a)
{
	return 0xe70 + 0x10000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_tl2#_topology
 *
 * NIX AF Transmit Level 2 Topology Registers
 */
union nixx_af_tl2x_topology {
	u64 u;
	struct nixx_af_tl2x_topology_s {
		u64 reserved_0                       : 1;
		u64 rr_prio                          : 4;
		u64 reserved_5_31                    : 27;
		u64 prio_anchor                      : 8;
		u64 reserved_40_63                   : 24;
	} s;
	/* struct nixx_af_tl2x_topology_s cn; */
};

static inline u64 NIXX_AF_TL2X_TOPOLOGY(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_TL2X_TOPOLOGY(u64 a)
{
	return 0xe80 + 0x10000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_tl2#_yellow
 *
 * INTERNAL: NIX Transmit Level 2 Yellow State Debug Register  This
 * register has the same bit fields as NIX_AF_TL1()_YELLOW.
 */
union nixx_af_tl2x_yellow {
	u64 u;
	struct nixx_af_tl2x_yellow_s {
		u64 tail                             : 8;
		u64 reserved_8_9                     : 2;
		u64 head                             : 8;
		u64 reserved_18_63                   : 46;
	} s;
	/* struct nixx_af_tl2x_yellow_s cn; */
};

static inline u64 NIXX_AF_TL2X_YELLOW(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_TL2X_YELLOW(u64 a)
{
	return 0xea0 + 0x10000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_tl2_const
 *
 * NIX AF Transmit Level 2 Constants Register This register contains
 * constants for software discovery.
 */
union nixx_af_tl2_const {
	u64 u;
	struct nixx_af_tl2_const_s {
		u64 count                            : 16;
		u64 reserved_16_63                   : 48;
	} s;
	/* struct nixx_af_tl2_const_s cn; */
};

static inline u64 NIXX_AF_TL2_CONST(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_TL2_CONST(void)
{
	return 0x78;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_tl3#_cir
 *
 * NIX AF Transmit Level 3 Committed Information Rate Registers This
 * register has the same bit fields as NIX_AF_TL1()_CIR.
 */
union nixx_af_tl3x_cir {
	u64 u;
	struct nixx_af_tl3x_cir_s {
		u64 enable                           : 1;
		u64 rate_mantissa                    : 8;
		u64 rate_exponent                    : 4;
		u64 rate_divider_exponent            : 4;
		u64 reserved_17_28                   : 12;
		u64 burst_mantissa                   : 8;
		u64 burst_exponent                   : 4;
		u64 reserved_41_63                   : 23;
	} s;
	/* struct nixx_af_tl3x_cir_s cn; */
};

static inline u64 NIXX_AF_TL3X_CIR(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_TL3X_CIR(u64 a)
{
	return 0x1020 + 0x10000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_tl3#_green
 *
 * INTERNAL: NIX Transmit Level 3 Green State Debug Register
 */
union nixx_af_tl3x_green {
	u64 u;
	struct nixx_af_tl3x_green_s {
		u64 tail                             : 9;
		u64 reserved_9                       : 1;
		u64 head                             : 9;
		u64 reserved_19                      : 1;
		u64 active_vec                       : 20;
		u64 rr_active                        : 1;
		u64 reserved_41_63                   : 23;
	} s;
	/* struct nixx_af_tl3x_green_s cn; */
};

static inline u64 NIXX_AF_TL3X_GREEN(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_TL3X_GREEN(u64 a)
{
	return 0x1090 + 0x10000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_tl3#_md_debug0
 *
 * NIX AF Transmit Level 3 Meta Descriptor Debug 0 Registers See
 * NIX_AF_TL1()_MD_DEBUG0.
 */
union nixx_af_tl3x_md_debug0 {
	u64 u;
	struct nixx_af_tl3x_md_debug0_s {
		u64 pmd0_length                      : 16;
		u64 pmd1_length                      : 16;
		u64 pmd0_vld                         : 1;
		u64 pmd1_vld                         : 1;
		u64 reserved_34_45                   : 12;
		u64 drain_pri                        : 1;
		u64 drain                            : 1;
		u64 c_con                            : 1;
		u64 p_con                            : 1;
		u64 reserved_50_51                   : 2;
		u64 child                            : 10;
		u64 reserved_62                      : 1;
		u64 pmd_count                        : 1;
	} s;
	/* struct nixx_af_tl3x_md_debug0_s cn96xxp1; */
	struct nixx_af_tl3x_md_debug0_cn96xxp3 {
		u64 pmd0_length                      : 16;
		u64 reserved_16_31                   : 16;
		u64 pmd0_vld                         : 1;
		u64 reserved_33                      : 1;
		u64 reserved_34_45                   : 12;
		u64 reserved_46                      : 1;
		u64 reserved_47                      : 1;
		u64 c_con                            : 1;
		u64 p_con                            : 1;
		u64 reserved_50_51                   : 2;
		u64 child                            : 10;
		u64 reserved_62                      : 1;
		u64 reserved_63                      : 1;
	} cn96xxp3;
	/* struct nixx_af_tl3x_md_debug0_s cnf95xx; */
};

static inline u64 NIXX_AF_TL3X_MD_DEBUG0(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_TL3X_MD_DEBUG0(u64 a)
{
	return 0x10c0 + 0x10000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_tl3#_md_debug1
 *
 * NIX AF Transmit Level 3 Meta Descriptor Debug 1 Registers Packet meta
 * descriptor 0 debug. See NIX_AF_TL1()_MD_DEBUG0.
 */
union nixx_af_tl3x_md_debug1 {
	u64 u;
	struct nixx_af_tl3x_md_debug1_s {
		u64 reserved_0_5                     : 6;
		u64 red_algo_override                : 2;
		u64 cir_dis                          : 1;
		u64 pir_dis                          : 1;
		u64 adjust                           : 9;
		u64 uid                              : 4;
		u64 reserved_23                      : 1;
		u64 bubble                           : 1;
		u64 color                            : 2;
		u64 pse_pkt_id                       : 9;
		u64 reserved_36                      : 1;
		u64 tx_pkt_p2x                       : 2;
		u64 sqm_pkt_id                       : 13;
		u64 mdq_idx                          : 10;
		u64 reserved_62                      : 1;
		u64 vld                              : 1;
	} s;
	struct nixx_af_tl3x_md_debug1_cn96xxp1 {
		u64 reserved_0_5                     : 6;
		u64 red_algo_override                : 2;
		u64 cir_dis                          : 1;
		u64 pir_dis                          : 1;
		u64 adjust                           : 9;
		u64 uid                              : 4;
		u64 drain                            : 1;
		u64 bubble                           : 1;
		u64 color                            : 2;
		u64 pse_pkt_id                       : 9;
		u64 reserved_36                      : 1;
		u64 tx_pkt_p2x                       : 2;
		u64 sqm_pkt_id                       : 13;
		u64 mdq_idx                          : 10;
		u64 reserved_62                      : 1;
		u64 vld                              : 1;
	} cn96xxp1;
	struct nixx_af_tl3x_md_debug1_cn96xxp3 {
		u64 reserved_0_5                     : 6;
		u64 red_algo_override                : 2;
		u64 cir_dis                          : 1;
		u64 pir_dis                          : 1;
		u64 adjust                           : 9;
		u64 reserved_19_22                   : 4;
		u64 flush                            : 1;
		u64 bubble                           : 1;
		u64 color                            : 2;
		u64 pse_pkt_id                       : 9;
		u64 reserved_36                      : 1;
		u64 tx_pkt_p2x                       : 2;
		u64 sqm_pkt_id                       : 13;
		u64 mdq_idx                          : 10;
		u64 reserved_62                      : 1;
		u64 vld                              : 1;
	} cn96xxp3;
	/* struct nixx_af_tl3x_md_debug1_cn96xxp1 cnf95xx; */
};

static inline u64 NIXX_AF_TL3X_MD_DEBUG1(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_TL3X_MD_DEBUG1(u64 a)
{
	return 0x10c8 + 0x10000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_tl3#_md_debug2
 *
 * NIX AF Transmit Level 3 Meta Descriptor Debug 2 Registers Packet meta
 * descriptor 1 debug. See NIX_AF_TL1()_MD_DEBUG0.
 */
union nixx_af_tl3x_md_debug2 {
	u64 u;
	struct nixx_af_tl3x_md_debug2_s {
		u64 reserved_0_5                     : 6;
		u64 red_algo_override                : 2;
		u64 cir_dis                          : 1;
		u64 pir_dis                          : 1;
		u64 adjust                           : 9;
		u64 uid                              : 4;
		u64 reserved_23                      : 1;
		u64 bubble                           : 1;
		u64 color                            : 2;
		u64 pse_pkt_id                       : 9;
		u64 reserved_36                      : 1;
		u64 tx_pkt_p2x                       : 2;
		u64 sqm_pkt_id                       : 13;
		u64 mdq_idx                          : 10;
		u64 reserved_62                      : 1;
		u64 vld                              : 1;
	} s;
	struct nixx_af_tl3x_md_debug2_cn96xxp1 {
		u64 reserved_0_5                     : 6;
		u64 red_algo_override                : 2;
		u64 cir_dis                          : 1;
		u64 pir_dis                          : 1;
		u64 adjust                           : 9;
		u64 uid                              : 4;
		u64 drain                            : 1;
		u64 bubble                           : 1;
		u64 color                            : 2;
		u64 pse_pkt_id                       : 9;
		u64 reserved_36                      : 1;
		u64 tx_pkt_p2x                       : 2;
		u64 sqm_pkt_id                       : 13;
		u64 mdq_idx                          : 10;
		u64 reserved_62                      : 1;
		u64 vld                              : 1;
	} cn96xxp1;
	struct nixx_af_tl3x_md_debug2_cn96xxp3 {
		u64 reserved_0_5                     : 6;
		u64 red_algo_override                : 2;
		u64 cir_dis                          : 1;
		u64 pir_dis                          : 1;
		u64 adjust                           : 9;
		u64 reserved_19_22                   : 4;
		u64 flush                            : 1;
		u64 bubble                           : 1;
		u64 color                            : 2;
		u64 pse_pkt_id                       : 9;
		u64 reserved_36                      : 1;
		u64 tx_pkt_p2x                       : 2;
		u64 sqm_pkt_id                       : 13;
		u64 mdq_idx                          : 10;
		u64 reserved_62                      : 1;
		u64 vld                              : 1;
	} cn96xxp3;
	/* struct nixx_af_tl3x_md_debug2_cn96xxp1 cnf95xx; */
};

static inline u64 NIXX_AF_TL3X_MD_DEBUG2(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_TL3X_MD_DEBUG2(u64 a)
{
	return 0x10d0 + 0x10000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_tl3#_md_debug3
 *
 * NIX AF Transmit Level 3 Meta Descriptor Debug 3 Registers Flush meta
 * descriptor debug. See NIX_AF_TL1()_MD_DEBUG0.
 */
union nixx_af_tl3x_md_debug3 {
	u64 u;
	struct nixx_af_tl3x_md_debug3_s {
		u64 reserved_0_36                    : 37;
		u64 tx_pkt_p2x                       : 2;
		u64 sqm_pkt_id                       : 13;
		u64 mdq_idx                          : 10;
		u64 reserved_62                      : 1;
		u64 vld                              : 1;
	} s;
	/* struct nixx_af_tl3x_md_debug3_s cn96xxp1; */
	struct nixx_af_tl3x_md_debug3_cn96xxp3 {
		u64 reserved_0_36                    : 37;
		u64 reserved_37_38                   : 2;
		u64 reserved_39_51                   : 13;
		u64 reserved_52_61                   : 10;
		u64 reserved_62                      : 1;
		u64 reserved_63                      : 1;
	} cn96xxp3;
	/* struct nixx_af_tl3x_md_debug3_s cnf95xx; */
};

static inline u64 NIXX_AF_TL3X_MD_DEBUG3(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_TL3X_MD_DEBUG3(u64 a)
{
	return 0x10d8 + 0x10000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_tl3#_parent
 *
 * NIX AF Transmit Level 3 Parent Registers
 */
union nixx_af_tl3x_parent {
	u64 u;
	struct nixx_af_tl3x_parent_s {
		u64 reserved_0_15                    : 16;
		u64 parent                           : 8;
		u64 reserved_24_63                   : 40;
	} s;
	/* struct nixx_af_tl3x_parent_s cn; */
};

static inline u64 NIXX_AF_TL3X_PARENT(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_TL3X_PARENT(u64 a)
{
	return 0x1088 + 0x10000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_tl3#_pir
 *
 * NIX AF Transmit Level 3 Peak Information Rate Registers This register
 * has the same bit fields as NIX_AF_TL1()_CIR.
 */
union nixx_af_tl3x_pir {
	u64 u;
	struct nixx_af_tl3x_pir_s {
		u64 enable                           : 1;
		u64 rate_mantissa                    : 8;
		u64 rate_exponent                    : 4;
		u64 rate_divider_exponent            : 4;
		u64 reserved_17_28                   : 12;
		u64 burst_mantissa                   : 8;
		u64 burst_exponent                   : 4;
		u64 reserved_41_63                   : 23;
	} s;
	/* struct nixx_af_tl3x_pir_s cn; */
};

static inline u64 NIXX_AF_TL3X_PIR(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_TL3X_PIR(u64 a)
{
	return 0x1030 + 0x10000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_tl3#_pointers
 *
 * INTERNAL: NIX Transmit Level 3 Linked List Pointers Debug Register
 * This register has the same bit fields as NIX_AF_TL2()_POINTERS.
 */
union nixx_af_tl3x_pointers {
	u64 u;
	struct nixx_af_tl3x_pointers_s {
		u64 next                             : 8;
		u64 reserved_8_15                    : 8;
		u64 prev                             : 8;
		u64 reserved_24_63                   : 40;
	} s;
	/* struct nixx_af_tl3x_pointers_s cn; */
};

static inline u64 NIXX_AF_TL3X_POINTERS(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_TL3X_POINTERS(u64 a)
{
	return 0x1060 + 0x10000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_tl3#_red
 *
 * INTERNAL: NIX Transmit Level 3 Red State Debug Register  This register
 * has the same bit fields as NIX_AF_TL3()_YELLOW.
 */
union nixx_af_tl3x_red {
	u64 u;
	struct nixx_af_tl3x_red_s {
		u64 tail                             : 9;
		u64 reserved_9                       : 1;
		u64 head                             : 9;
		u64 reserved_19_63                   : 45;
	} s;
	/* struct nixx_af_tl3x_red_s cn; */
};

static inline u64 NIXX_AF_TL3X_RED(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_TL3X_RED(u64 a)
{
	return 0x10b0 + 0x10000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_tl3#_sched_state
 *
 * NIX AF Transmit Level 3 Scheduling Control State Registers This
 * register has the same bit fields as NIX_AF_TL2()_SCHED_STATE.
 */
union nixx_af_tl3x_sched_state {
	u64 u;
	struct nixx_af_tl3x_sched_state_s {
		u64 rr_count                         : 25;
		u64 reserved_25_63                   : 39;
	} s;
	/* struct nixx_af_tl3x_sched_state_s cn; */
};

static inline u64 NIXX_AF_TL3X_SCHED_STATE(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_TL3X_SCHED_STATE(u64 a)
{
	return 0x1040 + 0x10000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_tl3#_schedule
 *
 * NIX AF Transmit Level 3 Scheduling Control Registers This register has
 * the same bit fields as NIX_AF_TL2()_SCHEDULE.
 */
union nixx_af_tl3x_schedule {
	u64 u;
	struct nixx_af_tl3x_schedule_s {
		u64 rr_quantum                       : 24;
		u64 prio                             : 4;
		u64 reserved_28_63                   : 36;
	} s;
	/* struct nixx_af_tl3x_schedule_s cn; */
};

static inline u64 NIXX_AF_TL3X_SCHEDULE(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_TL3X_SCHEDULE(u64 a)
{
	return 0x1000 + 0x10000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_tl3#_shape
 *
 * NIX AF Transmit Level 3 Shaping Control Registers
 */
union nixx_af_tl3x_shape {
	u64 u;
	struct nixx_af_tl3x_shape_s {
		u64 adjust                           : 9;
		u64 red_algo                         : 2;
		u64 red_disable                      : 1;
		u64 yellow_disable                   : 1;
		u64 reserved_13_23                   : 11;
		u64 length_disable                   : 1;
		u64 schedule_list                    : 2;
		u64 reserved_27_63                   : 37;
	} s;
	/* struct nixx_af_tl3x_shape_s cn; */
};

static inline u64 NIXX_AF_TL3X_SHAPE(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_TL3X_SHAPE(u64 a)
{
	return 0x1010 + 0x10000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_tl3#_shape_state
 *
 * NIX AF Transmit Level 3 Shaping State Registers This register has the
 * same bit fields as NIX_AF_TL2()_SHAPE_STATE. This register must not be
 * written during normal operation.
 */
union nixx_af_tl3x_shape_state {
	u64 u;
	struct nixx_af_tl3x_shape_state_s {
		u64 cir_accum                        : 26;
		u64 pir_accum                        : 26;
		u64 color                            : 2;
		u64 reserved_54_63                   : 10;
	} s;
	/* struct nixx_af_tl3x_shape_state_s cn; */
};

static inline u64 NIXX_AF_TL3X_SHAPE_STATE(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_TL3X_SHAPE_STATE(u64 a)
{
	return 0x1050 + 0x10000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_tl3#_sw_xoff
 *
 * NIX AF Transmit Level 3 Software Controlled XOFF Registers This
 * register has the same bit fields as NIX_AF_TL1()_SW_XOFF
 */
union nixx_af_tl3x_sw_xoff {
	u64 u;
	struct nixx_af_tl3x_sw_xoff_s {
		u64 xoff                             : 1;
		u64 drain                            : 1;
		u64 reserved_2                       : 1;
		u64 drain_irq                        : 1;
		u64 reserved_4_63                    : 60;
	} s;
	/* struct nixx_af_tl3x_sw_xoff_s cn; */
};

static inline u64 NIXX_AF_TL3X_SW_XOFF(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_TL3X_SW_XOFF(u64 a)
{
	return 0x1070 + 0x10000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_tl3#_topology
 *
 * NIX AF Transmit Level 3 Topology Registers
 */
union nixx_af_tl3x_topology {
	u64 u;
	struct nixx_af_tl3x_topology_s {
		u64 reserved_0                       : 1;
		u64 rr_prio                          : 4;
		u64 reserved_5_31                    : 27;
		u64 prio_anchor                      : 9;
		u64 reserved_41_63                   : 23;
	} s;
	/* struct nixx_af_tl3x_topology_s cn; */
};

static inline u64 NIXX_AF_TL3X_TOPOLOGY(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_TL3X_TOPOLOGY(u64 a)
{
	return 0x1080 + 0x10000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_tl3#_yellow
 *
 * INTERNAL: NIX Transmit Level 3 Yellow State Debug Register
 */
union nixx_af_tl3x_yellow {
	u64 u;
	struct nixx_af_tl3x_yellow_s {
		u64 tail                             : 9;
		u64 reserved_9                       : 1;
		u64 head                             : 9;
		u64 reserved_19_63                   : 45;
	} s;
	/* struct nixx_af_tl3x_yellow_s cn; */
};

static inline u64 NIXX_AF_TL3X_YELLOW(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_TL3X_YELLOW(u64 a)
{
	return 0x10a0 + 0x10000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_tl3_const
 *
 * NIX AF Transmit Level 3 Constants Register This register contains
 * constants for software discovery.
 */
union nixx_af_tl3_const {
	u64 u;
	struct nixx_af_tl3_const_s {
		u64 count                            : 16;
		u64 reserved_16_63                   : 48;
	} s;
	/* struct nixx_af_tl3_const_s cn; */
};

static inline u64 NIXX_AF_TL3_CONST(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_TL3_CONST(void)
{
	return 0x80;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_tl3_tl2#_bp_status
 *
 * NIX AF Transmit Level 3/2 Backpressure Status Registers
 */
union nixx_af_tl3_tl2x_bp_status {
	u64 u;
	struct nixx_af_tl3_tl2x_bp_status_s {
		u64 hw_xoff                          : 1;
		u64 reserved_1_63                    : 63;
	} s;
	/* struct nixx_af_tl3_tl2x_bp_status_s cn; */
};

static inline u64 NIXX_AF_TL3_TL2X_BP_STATUS(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_TL3_TL2X_BP_STATUS(u64 a)
{
	return 0x1610 + 0x10000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_tl3_tl2#_cfg
 *
 * NIX AF Transmit Level 3/2 Configuration Registers
 */
union nixx_af_tl3_tl2x_cfg {
	u64 u;
	struct nixx_af_tl3_tl2x_cfg_s {
		u64 express                          : 1;
		u64 reserved_1_63                    : 63;
	} s;
	/* struct nixx_af_tl3_tl2x_cfg_s cn; */
};

static inline u64 NIXX_AF_TL3_TL2X_CFG(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_TL3_TL2X_CFG(u64 a)
{
	return 0x1600 + 0x10000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_tl3_tl2#_link#_cfg
 *
 * NIX AF Transmit Level 3/2 Link Configuration Registers These registers
 * specify the links and associated channels that a given TL3 or TL2
 * queue (depending on NIX_AF_PSE_CHANNEL_LEVEL[BP_LEVEL]) can transmit
 * on. Each TL3/TL2 queue can be enabled to transmit on and be
 * backpressured by one or more links and associated channels. The last
 * index (LINK) is enumerated by NIX_LINK_E.
 */
union nixx_af_tl3_tl2x_linkx_cfg {
	u64 u;
	struct nixx_af_tl3_tl2x_linkx_cfg_s {
		u64 relchan                          : 8;
		u64 reserved_8_11                    : 4;
		u64 ena                              : 1;
		u64 bp_ena                           : 1;
		u64 reserved_14_63                   : 50;
	} s;
	/* struct nixx_af_tl3_tl2x_linkx_cfg_s cn; */
};

static inline u64 NIXX_AF_TL3_TL2X_LINKX_CFG(u64 a, u64 b)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_TL3_TL2X_LINKX_CFG(u64 a, u64 b)
{
	return 0x1700 + 0x10000 * a + 8 * b;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_tl4#_bp_status
 *
 * NIX AF Transmit Level 4 Backpressure Status Registers
 */
union nixx_af_tl4x_bp_status {
	u64 u;
	struct nixx_af_tl4x_bp_status_s {
		u64 hw_xoff                          : 1;
		u64 reserved_1_63                    : 63;
	} s;
	/* struct nixx_af_tl4x_bp_status_s cn; */
};

static inline u64 NIXX_AF_TL4X_BP_STATUS(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_TL4X_BP_STATUS(u64 a)
{
	return 0xb00 + 0x10000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_tl4#_cir
 *
 * NIX AF Transmit Level 4 Committed Information Rate Registers This
 * register has the same bit fields as NIX_AF_TL1()_CIR.
 */
union nixx_af_tl4x_cir {
	u64 u;
	struct nixx_af_tl4x_cir_s {
		u64 enable                           : 1;
		u64 rate_mantissa                    : 8;
		u64 rate_exponent                    : 4;
		u64 rate_divider_exponent            : 4;
		u64 reserved_17_28                   : 12;
		u64 burst_mantissa                   : 8;
		u64 burst_exponent                   : 4;
		u64 reserved_41_63                   : 23;
	} s;
	/* struct nixx_af_tl4x_cir_s cn; */
};

static inline u64 NIXX_AF_TL4X_CIR(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_TL4X_CIR(u64 a)
{
	return 0x1220 + 0x10000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_tl4#_green
 *
 * INTERNAL: NIX Transmit Level 4 Green State Debug Register  This
 * register has the same bit fields as NIX_AF_TL3()_GREEN.
 */
union nixx_af_tl4x_green {
	u64 u;
	struct nixx_af_tl4x_green_s {
		u64 tail                             : 9;
		u64 reserved_9                       : 1;
		u64 head                             : 9;
		u64 reserved_19                      : 1;
		u64 active_vec                       : 20;
		u64 rr_active                        : 1;
		u64 reserved_41_63                   : 23;
	} s;
	/* struct nixx_af_tl4x_green_s cn; */
};

static inline u64 NIXX_AF_TL4X_GREEN(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_TL4X_GREEN(u64 a)
{
	return 0x1290 + 0x10000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_tl4#_md_debug0
 *
 * NIX AF Transmit Level 4 Meta Descriptor Debug 0 Registers See
 * NIX_AF_TL1()_MD_DEBUG0.
 */
union nixx_af_tl4x_md_debug0 {
	u64 u;
	struct nixx_af_tl4x_md_debug0_s {
		u64 pmd0_length                      : 16;
		u64 pmd1_length                      : 16;
		u64 pmd0_vld                         : 1;
		u64 pmd1_vld                         : 1;
		u64 reserved_34_45                   : 12;
		u64 drain_pri                        : 1;
		u64 drain                            : 1;
		u64 c_con                            : 1;
		u64 p_con                            : 1;
		u64 reserved_50_51                   : 2;
		u64 child                            : 10;
		u64 reserved_62                      : 1;
		u64 pmd_count                        : 1;
	} s;
	/* struct nixx_af_tl4x_md_debug0_s cn96xxp1; */
	struct nixx_af_tl4x_md_debug0_cn96xxp3 {
		u64 pmd0_length                      : 16;
		u64 reserved_16_31                   : 16;
		u64 pmd0_vld                         : 1;
		u64 reserved_33                      : 1;
		u64 reserved_34_45                   : 12;
		u64 reserved_46                      : 1;
		u64 reserved_47                      : 1;
		u64 c_con                            : 1;
		u64 p_con                            : 1;
		u64 reserved_50_51                   : 2;
		u64 child                            : 10;
		u64 reserved_62                      : 1;
		u64 reserved_63                      : 1;
	} cn96xxp3;
	/* struct nixx_af_tl4x_md_debug0_s cnf95xx; */
};

static inline u64 NIXX_AF_TL4X_MD_DEBUG0(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_TL4X_MD_DEBUG0(u64 a)
{
	return 0x12c0 + 0x10000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_tl4#_md_debug1
 *
 * NIX AF Transmit Level 4 Meta Descriptor Debug 1 Registers Packet meta
 * descriptor 0 debug. See NIX_AF_TL1()_MD_DEBUG0.
 */
union nixx_af_tl4x_md_debug1 {
	u64 u;
	struct nixx_af_tl4x_md_debug1_s {
		u64 reserved_0_5                     : 6;
		u64 red_algo_override                : 2;
		u64 cir_dis                          : 1;
		u64 pir_dis                          : 1;
		u64 adjust                           : 9;
		u64 uid                              : 4;
		u64 reserved_23                      : 1;
		u64 bubble                           : 1;
		u64 color                            : 2;
		u64 pse_pkt_id                       : 9;
		u64 reserved_36                      : 1;
		u64 tx_pkt_p2x                       : 2;
		u64 sqm_pkt_id                       : 13;
		u64 mdq_idx                          : 10;
		u64 reserved_62                      : 1;
		u64 vld                              : 1;
	} s;
	struct nixx_af_tl4x_md_debug1_cn96xxp1 {
		u64 reserved_0_5                     : 6;
		u64 red_algo_override                : 2;
		u64 cir_dis                          : 1;
		u64 pir_dis                          : 1;
		u64 adjust                           : 9;
		u64 uid                              : 4;
		u64 drain                            : 1;
		u64 bubble                           : 1;
		u64 color                            : 2;
		u64 pse_pkt_id                       : 9;
		u64 reserved_36                      : 1;
		u64 tx_pkt_p2x                       : 2;
		u64 sqm_pkt_id                       : 13;
		u64 mdq_idx                          : 10;
		u64 reserved_62                      : 1;
		u64 vld                              : 1;
	} cn96xxp1;
	struct nixx_af_tl4x_md_debug1_cn96xxp3 {
		u64 reserved_0_5                     : 6;
		u64 red_algo_override                : 2;
		u64 cir_dis                          : 1;
		u64 pir_dis                          : 1;
		u64 adjust                           : 9;
		u64 reserved_19_22                   : 4;
		u64 flush                            : 1;
		u64 bubble                           : 1;
		u64 color                            : 2;
		u64 pse_pkt_id                       : 9;
		u64 reserved_36                      : 1;
		u64 tx_pkt_p2x                       : 2;
		u64 sqm_pkt_id                       : 13;
		u64 mdq_idx                          : 10;
		u64 reserved_62                      : 1;
		u64 vld                              : 1;
	} cn96xxp3;
	/* struct nixx_af_tl4x_md_debug1_cn96xxp1 cnf95xx; */
};

static inline u64 NIXX_AF_TL4X_MD_DEBUG1(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_TL4X_MD_DEBUG1(u64 a)
{
	return 0x12c8 + 0x10000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_tl4#_md_debug2
 *
 * NIX AF Transmit Level 4 Meta Descriptor Debug 2 Registers Packet meta
 * descriptor 1 debug. See NIX_AF_TL1()_MD_DEBUG0.
 */
union nixx_af_tl4x_md_debug2 {
	u64 u;
	struct nixx_af_tl4x_md_debug2_s {
		u64 reserved_0_5                     : 6;
		u64 red_algo_override                : 2;
		u64 cir_dis                          : 1;
		u64 pir_dis                          : 1;
		u64 adjust                           : 9;
		u64 uid                              : 4;
		u64 reserved_23                      : 1;
		u64 bubble                           : 1;
		u64 color                            : 2;
		u64 pse_pkt_id                       : 9;
		u64 reserved_36                      : 1;
		u64 tx_pkt_p2x                       : 2;
		u64 sqm_pkt_id                       : 13;
		u64 mdq_idx                          : 10;
		u64 reserved_62                      : 1;
		u64 vld                              : 1;
	} s;
	struct nixx_af_tl4x_md_debug2_cn96xxp1 {
		u64 reserved_0_5                     : 6;
		u64 red_algo_override                : 2;
		u64 cir_dis                          : 1;
		u64 pir_dis                          : 1;
		u64 adjust                           : 9;
		u64 uid                              : 4;
		u64 drain                            : 1;
		u64 bubble                           : 1;
		u64 color                            : 2;
		u64 pse_pkt_id                       : 9;
		u64 reserved_36                      : 1;
		u64 tx_pkt_p2x                       : 2;
		u64 sqm_pkt_id                       : 13;
		u64 mdq_idx                          : 10;
		u64 reserved_62                      : 1;
		u64 vld                              : 1;
	} cn96xxp1;
	struct nixx_af_tl4x_md_debug2_cn96xxp3 {
		u64 reserved_0_5                     : 6;
		u64 red_algo_override                : 2;
		u64 cir_dis                          : 1;
		u64 pir_dis                          : 1;
		u64 adjust                           : 9;
		u64 reserved_19_22                   : 4;
		u64 flush                            : 1;
		u64 bubble                           : 1;
		u64 color                            : 2;
		u64 pse_pkt_id                       : 9;
		u64 reserved_36                      : 1;
		u64 tx_pkt_p2x                       : 2;
		u64 sqm_pkt_id                       : 13;
		u64 mdq_idx                          : 10;
		u64 reserved_62                      : 1;
		u64 vld                              : 1;
	} cn96xxp3;
	/* struct nixx_af_tl4x_md_debug2_cn96xxp1 cnf95xx; */
};

static inline u64 NIXX_AF_TL4X_MD_DEBUG2(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_TL4X_MD_DEBUG2(u64 a)
{
	return 0x12d0 + 0x10000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_tl4#_md_debug3
 *
 * NIX AF Transmit Level 4 Meta Descriptor Debug 3 Registers Flush meta
 * descriptor debug. See NIX_AF_TL1()_MD_DEBUG0.
 */
union nixx_af_tl4x_md_debug3 {
	u64 u;
	struct nixx_af_tl4x_md_debug3_s {
		u64 reserved_0_36                    : 37;
		u64 tx_pkt_p2x                       : 2;
		u64 sqm_pkt_id                       : 13;
		u64 mdq_idx                          : 10;
		u64 reserved_62                      : 1;
		u64 vld                              : 1;
	} s;
	/* struct nixx_af_tl4x_md_debug3_s cn96xxp1; */
	struct nixx_af_tl4x_md_debug3_cn96xxp3 {
		u64 reserved_0_36                    : 37;
		u64 reserved_37_38                   : 2;
		u64 reserved_39_51                   : 13;
		u64 reserved_52_61                   : 10;
		u64 reserved_62                      : 1;
		u64 reserved_63                      : 1;
	} cn96xxp3;
	/* struct nixx_af_tl4x_md_debug3_s cnf95xx; */
};

static inline u64 NIXX_AF_TL4X_MD_DEBUG3(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_TL4X_MD_DEBUG3(u64 a)
{
	return 0x12d8 + 0x10000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_tl4#_parent
 *
 * NIX AF Transmit Level 4 Parent Registers
 */
union nixx_af_tl4x_parent {
	u64 u;
	struct nixx_af_tl4x_parent_s {
		u64 reserved_0_15                    : 16;
		u64 parent                           : 8;
		u64 reserved_24_63                   : 40;
	} s;
	/* struct nixx_af_tl4x_parent_s cn; */
};

static inline u64 NIXX_AF_TL4X_PARENT(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_TL4X_PARENT(u64 a)
{
	return 0x1288 + 0x10000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_tl4#_pir
 *
 * NIX AF Transmit Level 4 Peak Information Rate Registers This register
 * has the same bit fields as NIX_AF_TL1()_CIR.
 */
union nixx_af_tl4x_pir {
	u64 u;
	struct nixx_af_tl4x_pir_s {
		u64 enable                           : 1;
		u64 rate_mantissa                    : 8;
		u64 rate_exponent                    : 4;
		u64 rate_divider_exponent            : 4;
		u64 reserved_17_28                   : 12;
		u64 burst_mantissa                   : 8;
		u64 burst_exponent                   : 4;
		u64 reserved_41_63                   : 23;
	} s;
	/* struct nixx_af_tl4x_pir_s cn; */
};

static inline u64 NIXX_AF_TL4X_PIR(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_TL4X_PIR(u64 a)
{
	return 0x1230 + 0x10000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_tl4#_pointers
 *
 * INTERNAL: NIX Transmit Level 4 Linked List Pointers Debug Register
 * This register has the same bit fields as NIX_AF_TL2()_POINTERS.
 */
union nixx_af_tl4x_pointers {
	u64 u;
	struct nixx_af_tl4x_pointers_s {
		u64 next                             : 9;
		u64 reserved_9_15                    : 7;
		u64 prev                             : 9;
		u64 reserved_25_63                   : 39;
	} s;
	/* struct nixx_af_tl4x_pointers_s cn; */
};

static inline u64 NIXX_AF_TL4X_POINTERS(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_TL4X_POINTERS(u64 a)
{
	return 0x1260 + 0x10000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_tl4#_red
 *
 * INTERNAL: NIX Transmit Level 4 Red State Debug Register  This register
 * has the same bit fields as NIX_AF_TL3()_YELLOW.
 */
union nixx_af_tl4x_red {
	u64 u;
	struct nixx_af_tl4x_red_s {
		u64 tail                             : 9;
		u64 reserved_9                       : 1;
		u64 head                             : 9;
		u64 reserved_19_63                   : 45;
	} s;
	/* struct nixx_af_tl4x_red_s cn; */
};

static inline u64 NIXX_AF_TL4X_RED(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_TL4X_RED(u64 a)
{
	return 0x12b0 + 0x10000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_tl4#_sched_state
 *
 * NIX AF Transmit Level 4 Scheduling Control State Registers This
 * register has the same bit fields as NIX_AF_TL2()_SCHED_STATE.
 */
union nixx_af_tl4x_sched_state {
	u64 u;
	struct nixx_af_tl4x_sched_state_s {
		u64 rr_count                         : 25;
		u64 reserved_25_63                   : 39;
	} s;
	/* struct nixx_af_tl4x_sched_state_s cn; */
};

static inline u64 NIXX_AF_TL4X_SCHED_STATE(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_TL4X_SCHED_STATE(u64 a)
{
	return 0x1240 + 0x10000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_tl4#_schedule
 *
 * NIX AF Transmit Level 4 Scheduling Control Registers This register has
 * the same bit fields as NIX_AF_TL2()_SCHEDULE.
 */
union nixx_af_tl4x_schedule {
	u64 u;
	struct nixx_af_tl4x_schedule_s {
		u64 rr_quantum                       : 24;
		u64 prio                             : 4;
		u64 reserved_28_63                   : 36;
	} s;
	/* struct nixx_af_tl4x_schedule_s cn; */
};

static inline u64 NIXX_AF_TL4X_SCHEDULE(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_TL4X_SCHEDULE(u64 a)
{
	return 0x1200 + 0x10000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_tl4#_sdp_link_cfg
 *
 * NIX AF Transmit Level 4 Link Configuration Registers These registers
 * specify which TL4 queues transmit to and are optionally backpressured
 * by SDP.
 */
union nixx_af_tl4x_sdp_link_cfg {
	u64 u;
	struct nixx_af_tl4x_sdp_link_cfg_s {
		u64 relchan                          : 8;
		u64 reserved_8_11                    : 4;
		u64 ena                              : 1;
		u64 bp_ena                           : 1;
		u64 reserved_14_63                   : 50;
	} s;
	/* struct nixx_af_tl4x_sdp_link_cfg_s cn; */
};

static inline u64 NIXX_AF_TL4X_SDP_LINK_CFG(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_TL4X_SDP_LINK_CFG(u64 a)
{
	return 0xb10 + 0x10000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_tl4#_shape
 *
 * NIX AF Transmit Level 4 Shaping Control Registers This register has
 * the same bit fields as NIX_AF_TL2()_SHAPE.
 */
union nixx_af_tl4x_shape {
	u64 u;
	struct nixx_af_tl4x_shape_s {
		u64 adjust                           : 9;
		u64 red_algo                         : 2;
		u64 red_disable                      : 1;
		u64 yellow_disable                   : 1;
		u64 reserved_13_23                   : 11;
		u64 length_disable                   : 1;
		u64 schedule_list                    : 2;
		u64 reserved_27_63                   : 37;
	} s;
	/* struct nixx_af_tl4x_shape_s cn; */
};

static inline u64 NIXX_AF_TL4X_SHAPE(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_TL4X_SHAPE(u64 a)
{
	return 0x1210 + 0x10000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_tl4#_shape_state
 *
 * NIX AF Transmit Level 4 Shaping State Registers This register has the
 * same bit fields as NIX_AF_TL2()_SHAPE_STATE. This register must not be
 * written during normal operation.
 */
union nixx_af_tl4x_shape_state {
	u64 u;
	struct nixx_af_tl4x_shape_state_s {
		u64 cir_accum                        : 26;
		u64 pir_accum                        : 26;
		u64 color                            : 2;
		u64 reserved_54_63                   : 10;
	} s;
	/* struct nixx_af_tl4x_shape_state_s cn; */
};

static inline u64 NIXX_AF_TL4X_SHAPE_STATE(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_TL4X_SHAPE_STATE(u64 a)
{
	return 0x1250 + 0x10000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_tl4#_sw_xoff
 *
 * NIX AF Transmit Level 4 Software Controlled XOFF Registers This
 * register has the same bit fields as NIX_AF_TL1()_SW_XOFF
 */
union nixx_af_tl4x_sw_xoff {
	u64 u;
	struct nixx_af_tl4x_sw_xoff_s {
		u64 xoff                             : 1;
		u64 drain                            : 1;
		u64 reserved_2                       : 1;
		u64 drain_irq                        : 1;
		u64 reserved_4_63                    : 60;
	} s;
	/* struct nixx_af_tl4x_sw_xoff_s cn; */
};

static inline u64 NIXX_AF_TL4X_SW_XOFF(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_TL4X_SW_XOFF(u64 a)
{
	return 0x1270 + 0x10000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_tl4#_topology
 *
 * NIX AF Transmit Level 4 Topology Registers
 */
union nixx_af_tl4x_topology {
	u64 u;
	struct nixx_af_tl4x_topology_s {
		u64 reserved_0                       : 1;
		u64 rr_prio                          : 4;
		u64 reserved_5_31                    : 27;
		u64 prio_anchor                      : 9;
		u64 reserved_41_63                   : 23;
	} s;
	/* struct nixx_af_tl4x_topology_s cn; */
};

static inline u64 NIXX_AF_TL4X_TOPOLOGY(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_TL4X_TOPOLOGY(u64 a)
{
	return 0x1280 + 0x10000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_tl4#_yellow
 *
 * INTERNAL: NIX Transmit Level 4 Yellow State Debug Register  This
 * register has the same bit fields as NIX_AF_TL3()_YELLOW
 */
union nixx_af_tl4x_yellow {
	u64 u;
	struct nixx_af_tl4x_yellow_s {
		u64 tail                             : 9;
		u64 reserved_9                       : 1;
		u64 head                             : 9;
		u64 reserved_19_63                   : 45;
	} s;
	/* struct nixx_af_tl4x_yellow_s cn; */
};

static inline u64 NIXX_AF_TL4X_YELLOW(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_TL4X_YELLOW(u64 a)
{
	return 0x12a0 + 0x10000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_tl4_const
 *
 * NIX AF Transmit Level 4 Constants Register This register contains
 * constants for software discovery.
 */
union nixx_af_tl4_const {
	u64 u;
	struct nixx_af_tl4_const_s {
		u64 count                            : 16;
		u64 reserved_16_63                   : 48;
	} s;
	/* struct nixx_af_tl4_const_s cn; */
};

static inline u64 NIXX_AF_TL4_CONST(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_TL4_CONST(void)
{
	return 0x88;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_tx_link#_expr_credit
 *
 * INTERNAL: NIX AF Transmit Link Express Credit Registers  Internal:
 * 802.3br frame preemption/express path is defeatured. Old definition:
 * These registers track credits per link for express packets that may
 * potentially preempt normal packets. Link index enumerated by
 * NIX_LINK_E.
 */
union nixx_af_tx_linkx_expr_credit {
	u64 u;
	struct nixx_af_tx_linkx_expr_credit_s {
		u64 reserved_0                       : 1;
		u64 cc_enable                        : 1;
		u64 cc_packet_cnt                    : 10;
		u64 cc_unit_cnt                      : 20;
		u64 reserved_32_63                   : 32;
	} s;
	/* struct nixx_af_tx_linkx_expr_credit_s cn; */
};

static inline u64 NIXX_AF_TX_LINKX_EXPR_CREDIT(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_TX_LINKX_EXPR_CREDIT(u64 a)
{
	return 0xa10 + 0x10000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_tx_link#_hw_xoff
 *
 * NIX AF Transmit Link Hardware Controlled XOFF Registers Link index
 * enumerated by NIX_LINK_E.
 */
union nixx_af_tx_linkx_hw_xoff {
	u64 u;
	struct nixx_af_tx_linkx_hw_xoff_s {
		u64 chan_xoff                        : 64;
	} s;
	/* struct nixx_af_tx_linkx_hw_xoff_s cn; */
};

static inline u64 NIXX_AF_TX_LINKX_HW_XOFF(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_TX_LINKX_HW_XOFF(u64 a)
{
	return 0xa30 + 0x10000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_tx_link#_norm_credit
 *
 * NIX AF Transmit Link Normal Credit Registers These registers track
 * credits per link for normal packets sent to CGX and LBK. Link index
 * enumerated by NIX_LINK_E.
 */
union nixx_af_tx_linkx_norm_credit {
	u64 u;
	struct nixx_af_tx_linkx_norm_credit_s {
		u64 reserved_0                       : 1;
		u64 cc_enable                        : 1;
		u64 cc_packet_cnt                    : 10;
		u64 cc_unit_cnt                      : 20;
		u64 reserved_32_63                   : 32;
	} s;
	/* struct nixx_af_tx_linkx_norm_credit_s cn; */
};

static inline u64 NIXX_AF_TX_LINKX_NORM_CREDIT(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_TX_LINKX_NORM_CREDIT(u64 a)
{
	return 0xa00 + 0x10000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_tx_link#_sw_xoff
 *
 * INTERNAL: NIX AF Transmit Link Software Controlled XOFF Registers
 * Link index enumerated by NIX_LINK_E. Internal: Defeatured registers.
 * Software should instead use NIX_AF_TL3()_SW_XOFF registers when
 * NIX_AF_PSE_CHANNEL_LEVEL[BP_LEVEL] is set and NIX_AF_TL2()_SW_XOFF
 * registers when NIX_AF_PSE_CHANNEL_LEVEL[BP_LEVEL] is clear.
 */
union nixx_af_tx_linkx_sw_xoff {
	u64 u;
	struct nixx_af_tx_linkx_sw_xoff_s {
		u64 chan_xoff                        : 64;
	} s;
	/* struct nixx_af_tx_linkx_sw_xoff_s cn; */
};

static inline u64 NIXX_AF_TX_LINKX_SW_XOFF(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_TX_LINKX_SW_XOFF(u64 a)
{
	return 0xa20 + 0x10000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_tx_mcast#
 *
 * NIX AF Transmit Multicast Registers These registers access transmit
 * multicast table entries used to specify multicast replication lists.
 * Each list consists of linked entries with [EOL] = 1 in the last entry.
 * A transmit packet is multicast when the action returned by NPC has
 * NIX_TX_ACTION_S[OP] = NIX_TX_ACTIONOP_E::MCAST. NIX_TX_ACTION_S[INDEX]
 * points to the start of the multicast replication list, and [EOL] = 1
 * indicates the end of list.
 */
union nixx_af_tx_mcastx {
	u64 u;
	struct nixx_af_tx_mcastx_s {
		u64 channel                          : 12;
		u64 eol                              : 1;
		u64 reserved_13_15                   : 3;
		u64 next                             : 16;
		u64 reserved_32_63                   : 32;
	} s;
	/* struct nixx_af_tx_mcastx_s cn; */
};

static inline u64 NIXX_AF_TX_MCASTX(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_TX_MCASTX(u64 a)
{
	return 0x1900 + 0x8000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_tx_npc_capture_config
 *
 * NIX AF Transmit NPC Response Capture Configuration Register Configures
 * the NPC response capture logic for transmit packets. When enabled,
 * allows NPC responses for selected packets to be captured in
 * NIX_AF_TX_NPC_CAPTURE_INFO and NIX_AF_TX_NPC_CAPTURE_RESP().
 */
union nixx_af_tx_npc_capture_config {
	u64 u;
	struct nixx_af_tx_npc_capture_config_s {
		u64 en                               : 1;
		u64 continuous                       : 1;
		u64 lso_segnum_en                    : 1;
		u64 sqe_id_en                        : 1;
		u64 sq_id_en                         : 1;
		u64 lf_id_en                         : 1;
		u64 reserved_6_11                    : 6;
		u64 lso_segnum                       : 8;
		u64 sqe_id                           : 16;
		u64 sq_id                            : 20;
		u64 lf_id                            : 8;
	} s;
	/* struct nixx_af_tx_npc_capture_config_s cn; */
};

static inline u64 NIXX_AF_TX_NPC_CAPTURE_CONFIG(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_TX_NPC_CAPTURE_CONFIG(void)
{
	return 0x660;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_tx_npc_capture_info
 *
 * NIX AF Transmit NPC Response Capture Information Register This
 * register contains captured NPC response information for a transmit
 * packet. See NIX_AF_TX_NPC_CAPTURE_CONFIG.
 */
union nixx_af_tx_npc_capture_info {
	u64 u;
	struct nixx_af_tx_npc_capture_info_s {
		u64 vld                              : 1;
		u64 reserved_1_11                    : 11;
		u64 lso_segnum                       : 8;
		u64 sqe_id                           : 16;
		u64 sq_id                            : 20;
		u64 lf_id                            : 8;
	} s;
	/* struct nixx_af_tx_npc_capture_info_s cn; */
};

static inline u64 NIXX_AF_TX_NPC_CAPTURE_INFO(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_TX_NPC_CAPTURE_INFO(void)
{
	return 0x668;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_tx_npc_capture_resp#
 *
 * NIX AF Transmit NPC Capture Response Registers These registers contain
 * the captured NPC response for a transmit packet when
 * NIX_AF_TX_NPC_CAPTURE_INFO[VLD] is set. See also
 * NIX_AF_TX_NPC_CAPTURE_CONFIG.
 */
union nixx_af_tx_npc_capture_respx {
	u64 u;
	struct nixx_af_tx_npc_capture_respx_s {
		u64 data                             : 64;
	} s;
	/* struct nixx_af_tx_npc_capture_respx_s cn; */
};

static inline u64 NIXX_AF_TX_NPC_CAPTURE_RESPX(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_TX_NPC_CAPTURE_RESPX(u64 a)
{
	return 0x680 + 8 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_tx_tstmp_cfg
 *
 * NIX AF Transmit Timestamp Configuration Register
 */
union nixx_af_tx_tstmp_cfg {
	u64 u;
	struct nixx_af_tx_tstmp_cfg_s {
		u64 tstmp_wd_period                  : 4;
		u64 reserved_4_7                     : 4;
		u64 express                          : 16;
		u64 reserved_24_63                   : 40;
	} s;
	/* struct nixx_af_tx_tstmp_cfg_s cn; */
};

static inline u64 NIXX_AF_TX_TSTMP_CFG(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_TX_TSTMP_CFG(void)
{
	return 0xc0;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_tx_vtag_def#_ctl
 *
 * NIX AF Transmit Vtag Definition Control Registers The transmit Vtag
 * definition table specifies Vtag layers (e.g. VLAN, E-TAG) to
 * optionally insert or replace in the TX packet header. Indexed by
 * NIX_TX_VTAG_ACTION_S[VTAG*_DEF].
 */
union nixx_af_tx_vtag_defx_ctl {
	u64 u;
	struct nixx_af_tx_vtag_defx_ctl_s {
		u64 size                             : 1;
		u64 reserved_1_63                    : 63;
	} s;
	/* struct nixx_af_tx_vtag_defx_ctl_s cn; */
};

static inline u64 NIXX_AF_TX_VTAG_DEFX_CTL(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_TX_VTAG_DEFX_CTL(u64 a)
{
	return 0x1a00 + 0x10000 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_af_tx_vtag_def#_data
 *
 * NIX AF Transmit Vtag Definition Data Registers See
 * NIX_AF_TX_VTAG_DEF()_CTL.
 */
union nixx_af_tx_vtag_defx_data {
	u64 u;
	struct nixx_af_tx_vtag_defx_data_s {
		u64 data                             : 64;
	} s;
	/* struct nixx_af_tx_vtag_defx_data_s cn; */
};

static inline u64 NIXX_AF_TX_VTAG_DEFX_DATA(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_AF_TX_VTAG_DEFX_DATA(u64 a)
{
	return 0x1a10 + 0x10000 * a;
}

/**
 * Register (RVU_PFVF_BAR2) nix#_lf_cfg
 *
 * NIX LF Configuration Register
 */
union nixx_lf_cfg {
	u64 u;
	struct nixx_lf_cfg_s {
		u64 tcp_timer_int_ena                : 1;
		u64 reserved_1_63                    : 63;
	} s;
	/* struct nixx_lf_cfg_s cn; */
};

static inline u64 NIXX_LF_CFG(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_LF_CFG(void)
{
	return 0x100;
}

/**
 * Register (RVU_PFVF_BAR2) nix#_lf_cint#_cnt
 *
 * NIX LF Completion Interrupt Count Registers
 */
union nixx_lf_cintx_cnt {
	u64 u;
	struct nixx_lf_cintx_cnt_s {
		u64 ecount                           : 32;
		u64 qcount                           : 16;
		u64 reserved_48_63                   : 16;
	} s;
	/* struct nixx_lf_cintx_cnt_s cn; */
};

static inline u64 NIXX_LF_CINTX_CNT(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_LF_CINTX_CNT(u64 a)
{
	return 0xd00 + 0x1000 * a;
}

/**
 * Register (RVU_PFVF_BAR2) nix#_lf_cint#_ena_w1c
 *
 * NIX LF Completion Interrupt Enable Clear Registers This register
 * clears interrupt enable bits.
 */
union nixx_lf_cintx_ena_w1c {
	u64 u;
	struct nixx_lf_cintx_ena_w1c_s {
		u64 intr                             : 1;
		u64 reserved_1_63                    : 63;
	} s;
	/* struct nixx_lf_cintx_ena_w1c_s cn; */
};

static inline u64 NIXX_LF_CINTX_ENA_W1C(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_LF_CINTX_ENA_W1C(u64 a)
{
	return 0xd50 + 0x1000 * a;
}

/**
 * Register (RVU_PFVF_BAR2) nix#_lf_cint#_ena_w1s
 *
 * NIX LF Completion Interrupt Enable Set Registers This register sets
 * interrupt enable bits.
 */
union nixx_lf_cintx_ena_w1s {
	u64 u;
	struct nixx_lf_cintx_ena_w1s_s {
		u64 intr                             : 1;
		u64 reserved_1_63                    : 63;
	} s;
	/* struct nixx_lf_cintx_ena_w1s_s cn; */
};

static inline u64 NIXX_LF_CINTX_ENA_W1S(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_LF_CINTX_ENA_W1S(u64 a)
{
	return 0xd40 + 0x1000 * a;
}

/**
 * Register (RVU_PFVF_BAR2) nix#_lf_cint#_int
 *
 * NIX LF Completion Interrupt Registers
 */
union nixx_lf_cintx_int {
	u64 u;
	struct nixx_lf_cintx_int_s {
		u64 intr                             : 1;
		u64 reserved_1_63                    : 63;
	} s;
	/* struct nixx_lf_cintx_int_s cn; */
};

static inline u64 NIXX_LF_CINTX_INT(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_LF_CINTX_INT(u64 a)
{
	return 0xd20 + 0x1000 * a;
}

/**
 * Register (RVU_PFVF_BAR2) nix#_lf_cint#_int_w1s
 *
 * NIX LF Completion Interrupt Set Registers This register sets interrupt
 * bits.
 */
union nixx_lf_cintx_int_w1s {
	u64 u;
	struct nixx_lf_cintx_int_w1s_s {
		u64 intr                             : 1;
		u64 reserved_1_63                    : 63;
	} s;
	/* struct nixx_lf_cintx_int_w1s_s cn; */
};

static inline u64 NIXX_LF_CINTX_INT_W1S(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_LF_CINTX_INT_W1S(u64 a)
{
	return 0xd30 + 0x1000 * a;
}

/**
 * Register (RVU_PFVF_BAR2) nix#_lf_cint#_wait
 *
 * NIX LF Completion Interrupt Count Registers
 */
union nixx_lf_cintx_wait {
	u64 u;
	struct nixx_lf_cintx_wait_s {
		u64 ecount_wait                      : 32;
		u64 qcount_wait                      : 16;
		u64 time_wait                        : 8;
		u64 reserved_56_63                   : 8;
	} s;
	/* struct nixx_lf_cintx_wait_s cn; */
};

static inline u64 NIXX_LF_CINTX_WAIT(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_LF_CINTX_WAIT(u64 a)
{
	return 0xd10 + 0x1000 * a;
}

/**
 * Register (RVU_PFVF_BAR2) nix#_lf_cq_op_door
 *
 * NIX LF CQ Doorbell Operation Register A write to this register
 * dequeues CQEs from a CQ ring within the LF. A read is RAZ.  RSL
 * accesses to this register are RAZ/WI.
 */
union nixx_lf_cq_op_door {
	u64 u;
	struct nixx_lf_cq_op_door_s {
		u64 count                            : 16;
		u64 reserved_16_31                   : 16;
		u64 cq                               : 20;
		u64 reserved_52_63                   : 12;
	} s;
	/* struct nixx_lf_cq_op_door_s cn; */
};

static inline u64 NIXX_LF_CQ_OP_DOOR(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_LF_CQ_OP_DOOR(void)
{
	return 0xb30;
}

/**
 * Register (RVU_PFVF_BAR2) nix#_lf_cq_op_int
 *
 * NIX LF Completion Queue Interrupt Operation Register A 64-bit atomic
 * load-and-add to this register reads CQ interrupts and interrupt
 * enables. A write optionally sets or clears interrupts and interrupt
 * enables. A read is RAZ.  RSL accesses to this register are RAZ/WI.
 */
union nixx_lf_cq_op_int {
	u64 u;
	struct nixx_lf_cq_op_int_s {
		u64 cq_err_int                       : 8;
		u64 cq_err_int_ena                   : 8;
		u64 reserved_16_41                   : 26;
		u64 op_err                           : 1;
		u64 setop                            : 1;
		u64 cq                               : 20;
	} s;
	/* struct nixx_lf_cq_op_int_s cn; */
};

static inline u64 NIXX_LF_CQ_OP_INT(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_LF_CQ_OP_INT(void)
{
	return 0xb00;
}

/**
 * Register (RVU_PFVF_BAR2) nix#_lf_cq_op_status
 *
 * NIX LF Completion Queue Status Operation Register A 64-bit atomic
 * load-and-add to this register reads NIX_CQ_CTX_S[HEAD,TAIL]. The
 * atomic write data has format NIX_OP_Q_WDATA_S and selects the CQ
 * within LF.  All other accesses to this register (e.g. reads and
 * writes) are RAZ/WI.  RSL accesses to this register are RAZ/WI.
 */
union nixx_lf_cq_op_status {
	u64 u;
	struct nixx_lf_cq_op_status_s {
		u64 tail                             : 20;
		u64 head                             : 20;
		u64 reserved_40_45                   : 6;
		u64 cq_err                           : 1;
		u64 reserved_47_62                   : 16;
		u64 op_err                           : 1;
	} s;
	/* struct nixx_lf_cq_op_status_s cn; */
};

static inline u64 NIXX_LF_CQ_OP_STATUS(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_LF_CQ_OP_STATUS(void)
{
	return 0xb40;
}

/**
 * Register (RVU_PFVF_BAR2) nix#_lf_err_int
 *
 * NIX LF Error Interrupt Register
 */
union nixx_lf_err_int {
	u64 u;
	struct nixx_lf_err_int_s {
		u64 sqb_fault                        : 1;
		u64 sq_ctx_fault                     : 1;
		u64 rq_ctx_fault                     : 1;
		u64 cq_ctx_fault                     : 1;
		u64 reserved_4                       : 1;
		u64 rsse_fault                       : 1;
		u64 ipsec_dyno_fault                 : 1;
		u64 sq_disabled                      : 1;
		u64 sq_oor                           : 1;
		u64 send_jump_fault                  : 1;
		u64 send_sg_fault                    : 1;
		u64 rq_disabled                      : 1;
		u64 rq_oor                           : 1;
		u64 rx_wqe_fault                     : 1;
		u64 rss_err                          : 1;
		u64 reserved_15_19                   : 5;
		u64 dyno_err                         : 1;
		u64 reserved_21_23                   : 3;
		u64 cq_disabled                      : 1;
		u64 cq_oor                           : 1;
		u64 reserved_26_27                   : 2;
		u64 qint_fault                       : 1;
		u64 cint_fault                       : 1;
		u64 reserved_30_63                   : 34;
	} s;
	/* struct nixx_lf_err_int_s cn; */
};

static inline u64 NIXX_LF_ERR_INT(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_LF_ERR_INT(void)
{
	return 0x220;
}

/**
 * Register (RVU_PFVF_BAR2) nix#_lf_err_int_ena_w1c
 *
 * NIX LF Error Interrupt Enable Clear Register This register clears
 * interrupt enable bits.
 */
union nixx_lf_err_int_ena_w1c {
	u64 u;
	struct nixx_lf_err_int_ena_w1c_s {
		u64 sqb_fault                        : 1;
		u64 sq_ctx_fault                     : 1;
		u64 rq_ctx_fault                     : 1;
		u64 cq_ctx_fault                     : 1;
		u64 reserved_4                       : 1;
		u64 rsse_fault                       : 1;
		u64 ipsec_dyno_fault                 : 1;
		u64 sq_disabled                      : 1;
		u64 sq_oor                           : 1;
		u64 send_jump_fault                  : 1;
		u64 send_sg_fault                    : 1;
		u64 rq_disabled                      : 1;
		u64 rq_oor                           : 1;
		u64 rx_wqe_fault                     : 1;
		u64 rss_err                          : 1;
		u64 reserved_15_19                   : 5;
		u64 dyno_err                         : 1;
		u64 reserved_21_23                   : 3;
		u64 cq_disabled                      : 1;
		u64 cq_oor                           : 1;
		u64 reserved_26_27                   : 2;
		u64 qint_fault                       : 1;
		u64 cint_fault                       : 1;
		u64 reserved_30_63                   : 34;
	} s;
	/* struct nixx_lf_err_int_ena_w1c_s cn; */
};

static inline u64 NIXX_LF_ERR_INT_ENA_W1C(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_LF_ERR_INT_ENA_W1C(void)
{
	return 0x230;
}

/**
 * Register (RVU_PFVF_BAR2) nix#_lf_err_int_ena_w1s
 *
 * NIX LF Error Interrupt Enable Set Register This register sets
 * interrupt enable bits.
 */
union nixx_lf_err_int_ena_w1s {
	u64 u;
	struct nixx_lf_err_int_ena_w1s_s {
		u64 sqb_fault                        : 1;
		u64 sq_ctx_fault                     : 1;
		u64 rq_ctx_fault                     : 1;
		u64 cq_ctx_fault                     : 1;
		u64 reserved_4                       : 1;
		u64 rsse_fault                       : 1;
		u64 ipsec_dyno_fault                 : 1;
		u64 sq_disabled                      : 1;
		u64 sq_oor                           : 1;
		u64 send_jump_fault                  : 1;
		u64 send_sg_fault                    : 1;
		u64 rq_disabled                      : 1;
		u64 rq_oor                           : 1;
		u64 rx_wqe_fault                     : 1;
		u64 rss_err                          : 1;
		u64 reserved_15_19                   : 5;
		u64 dyno_err                         : 1;
		u64 reserved_21_23                   : 3;
		u64 cq_disabled                      : 1;
		u64 cq_oor                           : 1;
		u64 reserved_26_27                   : 2;
		u64 qint_fault                       : 1;
		u64 cint_fault                       : 1;
		u64 reserved_30_63                   : 34;
	} s;
	/* struct nixx_lf_err_int_ena_w1s_s cn; */
};

static inline u64 NIXX_LF_ERR_INT_ENA_W1S(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_LF_ERR_INT_ENA_W1S(void)
{
	return 0x238;
}

/**
 * Register (RVU_PFVF_BAR2) nix#_lf_err_int_w1s
 *
 * NIX LF Error Interrupt Set Register This register sets interrupt bits.
 */
union nixx_lf_err_int_w1s {
	u64 u;
	struct nixx_lf_err_int_w1s_s {
		u64 sqb_fault                        : 1;
		u64 sq_ctx_fault                     : 1;
		u64 rq_ctx_fault                     : 1;
		u64 cq_ctx_fault                     : 1;
		u64 reserved_4                       : 1;
		u64 rsse_fault                       : 1;
		u64 ipsec_dyno_fault                 : 1;
		u64 sq_disabled                      : 1;
		u64 sq_oor                           : 1;
		u64 send_jump_fault                  : 1;
		u64 send_sg_fault                    : 1;
		u64 rq_disabled                      : 1;
		u64 rq_oor                           : 1;
		u64 rx_wqe_fault                     : 1;
		u64 rss_err                          : 1;
		u64 reserved_15_19                   : 5;
		u64 dyno_err                         : 1;
		u64 reserved_21_23                   : 3;
		u64 cq_disabled                      : 1;
		u64 cq_oor                           : 1;
		u64 reserved_26_27                   : 2;
		u64 qint_fault                       : 1;
		u64 cint_fault                       : 1;
		u64 reserved_30_63                   : 34;
	} s;
	/* struct nixx_lf_err_int_w1s_s cn; */
};

static inline u64 NIXX_LF_ERR_INT_W1S(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_LF_ERR_INT_W1S(void)
{
	return 0x228;
}

/**
 * Register (RVU_PFVF_BAR2) nix#_lf_gint
 *
 * NIX LF General Interrupt Register
 */
union nixx_lf_gint {
	u64 u;
	struct nixx_lf_gint_s {
		u64 drop                             : 1;
		u64 tcp_timer                        : 1;
		u64 reserved_2_63                    : 62;
	} s;
	/* struct nixx_lf_gint_s cn; */
};

static inline u64 NIXX_LF_GINT(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_LF_GINT(void)
{
	return 0x200;
}

/**
 * Register (RVU_PFVF_BAR2) nix#_lf_gint_ena_w1c
 *
 * NIX LF General Interrupt Enable Clear Register This register clears
 * interrupt enable bits.
 */
union nixx_lf_gint_ena_w1c {
	u64 u;
	struct nixx_lf_gint_ena_w1c_s {
		u64 drop                             : 1;
		u64 tcp_timer                        : 1;
		u64 reserved_2_63                    : 62;
	} s;
	/* struct nixx_lf_gint_ena_w1c_s cn; */
};

static inline u64 NIXX_LF_GINT_ENA_W1C(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_LF_GINT_ENA_W1C(void)
{
	return 0x210;
}

/**
 * Register (RVU_PFVF_BAR2) nix#_lf_gint_ena_w1s
 *
 * NIX LF General Interrupt Enable Set Register This register sets
 * interrupt enable bits.
 */
union nixx_lf_gint_ena_w1s {
	u64 u;
	struct nixx_lf_gint_ena_w1s_s {
		u64 drop                             : 1;
		u64 tcp_timer                        : 1;
		u64 reserved_2_63                    : 62;
	} s;
	/* struct nixx_lf_gint_ena_w1s_s cn; */
};

static inline u64 NIXX_LF_GINT_ENA_W1S(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_LF_GINT_ENA_W1S(void)
{
	return 0x218;
}

/**
 * Register (RVU_PFVF_BAR2) nix#_lf_gint_w1s
 *
 * NIX LF General Interrupt Set Register This register sets interrupt
 * bits.
 */
union nixx_lf_gint_w1s {
	u64 u;
	struct nixx_lf_gint_w1s_s {
		u64 drop                             : 1;
		u64 tcp_timer                        : 1;
		u64 reserved_2_63                    : 62;
	} s;
	/* struct nixx_lf_gint_w1s_s cn; */
};

static inline u64 NIXX_LF_GINT_W1S(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_LF_GINT_W1S(void)
{
	return 0x208;
}

/**
 * Register (RVU_PFVF_BAR2) nix#_lf_mnq_err_dbg
 *
 * NIX LF Meta-descriptor Enqueue Error Debug Register This register
 * captures debug info for an error detected during send meta-descriptor
 * enqueue from an SQ to an SMQ. Hardware sets [VALID] when the debug
 * info is captured, and subsequent errors are not captured until
 * software clears [VALID] by writing a one to it.
 */
union nixx_lf_mnq_err_dbg {
	u64 u;
	struct nixx_lf_mnq_err_dbg_s {
		u64 errcode                          : 8;
		u64 sq                               : 20;
		u64 sqe_id                           : 16;
		u64 valid                            : 1;
		u64 reserved_45_63                   : 19;
	} s;
	/* struct nixx_lf_mnq_err_dbg_s cn; */
};

static inline u64 NIXX_LF_MNQ_ERR_DBG(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_LF_MNQ_ERR_DBG(void)
{
	return 0x270;
}

/**
 * Register (RVU_PFVF_BAR2) nix#_lf_op_ipsec_dyno_cnt
 *
 * INTERNAL: NIX LF IPSEC Dynamic Ordering Counter Operation Register
 * Internal: Not used; no IPSEC fast-path. All accesses are RAZ/WI.
 */
union nixx_lf_op_ipsec_dyno_cnt {
	u64 u;
	struct nixx_lf_op_ipsec_dyno_cnt_s {
		u64 count                            : 32;
		u64 reserved_32_46                   : 15;
		u64 storeop                          : 1;
		u64 dyno_sel                         : 15;
		u64 op_err                           : 1;
	} s;
	/* struct nixx_lf_op_ipsec_dyno_cnt_s cn; */
};

static inline u64 NIXX_LF_OP_IPSEC_DYNO_CNT(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_LF_OP_IPSEC_DYNO_CNT(void)
{
	return 0x980;
}

/**
 * Register (RVU_PFVF_BAR2) nix#_lf_op_send#
 *
 * NIX LF Send Operation Registers An LMTST (or large store from CPT) to
 * this address enqueues one or more SQEs to a send queue.
 * NIX_SEND_HDR_S[SQ] in the first SQE selects the send queue.The maximum
 * size of each SQE is specified by NIX_SQ_CTX_S[MAX_SQE_SIZE].  A read
 * to this address is RAZ.  An RSL access to this address will fault.
 * The endianness of the instruction write data is controlled by
 * NIX_AF_LF()_CFG[BE].  When a NIX_SEND_JUMP_S is not present in the
 * SQE, the SQE consists of the entire send descriptor.  When a
 * NIX_SEND_JUMP_S is present in the SQE, the SQE must contain exactly
 * the portion of the send descriptor up to and including the
 * NIX_SEND_JUMP_S, and the remainder of the send descriptor must be at
 * LF IOVA NIX_SEND_JUMP_S[ADDR] in LLC/DRAM.  Software must ensure that
 * all LLC/DRAM locations that will be referenced by NIX while processing
 * this descriptor, including all packet data and post-jump
 * subdescriptors contain the latest updates before issuing the LMTST. A
 * DMB instruction may be required prior to the LMTST to ensure this. A
 * DMB following the LMTST may be useful if SQ descriptor ordering
 * matters and more than one CPU core is simultaneously enqueueing to the
 * same SQ.
 */
union nixx_lf_op_sendx {
	u64 u;
	struct nixx_lf_op_sendx_s {
		u64 data                             : 64;
	} s;
	/* struct nixx_lf_op_sendx_s cn; */
};

static inline u64 NIXX_LF_OP_SENDX(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_LF_OP_SENDX(u64 a)
{
	return 0x800 + 8 * a;
}

/**
 * Register (RVU_PFVF_BAR2) nix#_lf_qint#_cnt
 *
 * NIX LF Queue Interrupt Count Registers
 */
union nixx_lf_qintx_cnt {
	u64 u;
	struct nixx_lf_qintx_cnt_s {
		u64 count                            : 22;
		u64 reserved_22_63                   : 42;
	} s;
	/* struct nixx_lf_qintx_cnt_s cn; */
};

static inline u64 NIXX_LF_QINTX_CNT(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_LF_QINTX_CNT(u64 a)
{
	return 0xc00 + 0x1000 * a;
}

/**
 * Register (RVU_PFVF_BAR2) nix#_lf_qint#_ena_w1c
 *
 * NIX LF Queue Interrupt Enable Clear Registers This register clears
 * interrupt enable bits.
 */
union nixx_lf_qintx_ena_w1c {
	u64 u;
	struct nixx_lf_qintx_ena_w1c_s {
		u64 intr                             : 1;
		u64 reserved_1_63                    : 63;
	} s;
	/* struct nixx_lf_qintx_ena_w1c_s cn; */
};

static inline u64 NIXX_LF_QINTX_ENA_W1C(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_LF_QINTX_ENA_W1C(u64 a)
{
	return 0xc30 + 0x1000 * a;
}

/**
 * Register (RVU_PFVF_BAR2) nix#_lf_qint#_ena_w1s
 *
 * NIX LF Queue Interrupt Enable Set Registers This register sets
 * interrupt enable bits.
 */
union nixx_lf_qintx_ena_w1s {
	u64 u;
	struct nixx_lf_qintx_ena_w1s_s {
		u64 intr                             : 1;
		u64 reserved_1_63                    : 63;
	} s;
	/* struct nixx_lf_qintx_ena_w1s_s cn; */
};

static inline u64 NIXX_LF_QINTX_ENA_W1S(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_LF_QINTX_ENA_W1S(u64 a)
{
	return 0xc20 + 0x1000 * a;
}

/**
 * Register (RVU_PFVF_BAR2) nix#_lf_qint#_int
 *
 * NIX LF Queue Interrupt Registers
 */
union nixx_lf_qintx_int {
	u64 u;
	struct nixx_lf_qintx_int_s {
		u64 intr                             : 1;
		u64 reserved_1_63                    : 63;
	} s;
	/* struct nixx_lf_qintx_int_s cn; */
};

static inline u64 NIXX_LF_QINTX_INT(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_LF_QINTX_INT(u64 a)
{
	return 0xc10 + 0x1000 * a;
}

/**
 * Register (RVU_PFVF_BAR2) nix#_lf_qint#_int_w1s
 *
 * INTERNAL: NIX LF Queue Interrupt Set Registers
 */
union nixx_lf_qintx_int_w1s {
	u64 u;
	struct nixx_lf_qintx_int_w1s_s {
		u64 intr                             : 1;
		u64 reserved_1_63                    : 63;
	} s;
	/* struct nixx_lf_qintx_int_w1s_s cn; */
};

static inline u64 NIXX_LF_QINTX_INT_W1S(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_LF_QINTX_INT_W1S(u64 a)
{
	return 0xc18 + 0x1000 * a;
}

/**
 * Register (RVU_PFVF_BAR2) nix#_lf_ras
 *
 * NIX LF RAS Interrupt Register
 */
union nixx_lf_ras {
	u64 u;
	struct nixx_lf_ras_s {
		u64 sqb_poison                       : 1;
		u64 sq_ctx_poison                    : 1;
		u64 rq_ctx_poison                    : 1;
		u64 cq_ctx_poison                    : 1;
		u64 reserved_4                       : 1;
		u64 rsse_poison                      : 1;
		u64 ipsec_dyno_poison                : 1;
		u64 send_jump_poison                 : 1;
		u64 send_sg_poison                   : 1;
		u64 qint_poison                      : 1;
		u64 cint_poison                      : 1;
		u64 reserved_11_63                   : 53;
	} s;
	/* struct nixx_lf_ras_s cn; */
};

static inline u64 NIXX_LF_RAS(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_LF_RAS(void)
{
	return 0x240;
}

/**
 * Register (RVU_PFVF_BAR2) nix#_lf_ras_ena_w1c
 *
 * NIX LF RAS Interrupt Enable Clear Register This register clears
 * interrupt enable bits.
 */
union nixx_lf_ras_ena_w1c {
	u64 u;
	struct nixx_lf_ras_ena_w1c_s {
		u64 sqb_poison                       : 1;
		u64 sq_ctx_poison                    : 1;
		u64 rq_ctx_poison                    : 1;
		u64 cq_ctx_poison                    : 1;
		u64 reserved_4                       : 1;
		u64 rsse_poison                      : 1;
		u64 ipsec_dyno_poison                : 1;
		u64 send_jump_poison                 : 1;
		u64 send_sg_poison                   : 1;
		u64 qint_poison                      : 1;
		u64 cint_poison                      : 1;
		u64 reserved_11_63                   : 53;
	} s;
	/* struct nixx_lf_ras_ena_w1c_s cn; */
};

static inline u64 NIXX_LF_RAS_ENA_W1C(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_LF_RAS_ENA_W1C(void)
{
	return 0x250;
}

/**
 * Register (RVU_PFVF_BAR2) nix#_lf_ras_ena_w1s
 *
 * NIX LF RAS Interrupt Enable Set Register This register sets interrupt
 * enable bits.
 */
union nixx_lf_ras_ena_w1s {
	u64 u;
	struct nixx_lf_ras_ena_w1s_s {
		u64 sqb_poison                       : 1;
		u64 sq_ctx_poison                    : 1;
		u64 rq_ctx_poison                    : 1;
		u64 cq_ctx_poison                    : 1;
		u64 reserved_4                       : 1;
		u64 rsse_poison                      : 1;
		u64 ipsec_dyno_poison                : 1;
		u64 send_jump_poison                 : 1;
		u64 send_sg_poison                   : 1;
		u64 qint_poison                      : 1;
		u64 cint_poison                      : 1;
		u64 reserved_11_63                   : 53;
	} s;
	/* struct nixx_lf_ras_ena_w1s_s cn; */
};

static inline u64 NIXX_LF_RAS_ENA_W1S(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_LF_RAS_ENA_W1S(void)
{
	return 0x258;
}

/**
 * Register (RVU_PFVF_BAR2) nix#_lf_ras_w1s
 *
 * NIX LF RAS Interrupt Set Register This register sets interrupt bits.
 */
union nixx_lf_ras_w1s {
	u64 u;
	struct nixx_lf_ras_w1s_s {
		u64 sqb_poison                       : 1;
		u64 sq_ctx_poison                    : 1;
		u64 rq_ctx_poison                    : 1;
		u64 cq_ctx_poison                    : 1;
		u64 reserved_4                       : 1;
		u64 rsse_poison                      : 1;
		u64 ipsec_dyno_poison                : 1;
		u64 send_jump_poison                 : 1;
		u64 send_sg_poison                   : 1;
		u64 qint_poison                      : 1;
		u64 cint_poison                      : 1;
		u64 reserved_11_63                   : 53;
	} s;
	/* struct nixx_lf_ras_w1s_s cn; */
};

static inline u64 NIXX_LF_RAS_W1S(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_LF_RAS_W1S(void)
{
	return 0x248;
}

/**
 * Register (RVU_PFVF_BAR2) nix#_lf_rq_op_drop_octs
 *
 * NIX LF Receive Queue Dropped Octets Operation Register A 64-bit atomic
 * load-and-add to this register reads NIX_RQ_CTX_S[DROP_OCTS]. The
 * atomic write data has format NIX_OP_Q_WDATA_S and selects the RQ
 * within LF.  All other accesses to this register (e.g. reads and
 * writes) are RAZ/WI.  RSL accesses to this register are RAZ/WI.
 */
union nixx_lf_rq_op_drop_octs {
	u64 u;
	struct nixx_lf_rq_op_drop_octs_s {
		u64 cnt                              : 48;
		u64 reserved_48_62                   : 15;
		u64 op_err                           : 1;
	} s;
	/* struct nixx_lf_rq_op_drop_octs_s cn; */
};

static inline u64 NIXX_LF_RQ_OP_DROP_OCTS(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_LF_RQ_OP_DROP_OCTS(void)
{
	return 0x930;
}

/**
 * Register (RVU_PFVF_BAR2) nix#_lf_rq_op_drop_pkts
 *
 * NIX LF Receive Queue Dropped Packets Operation Register A 64-bit
 * atomic load-and-add to this register reads NIX_RQ_CTX_S[DROP_PKTS].
 * The atomic write data has format NIX_OP_Q_WDATA_S and selects the RQ
 * within LF.  All other accesses to this register (e.g. reads and
 * writes) are RAZ/WI.  RSL accesses to this register are RAZ/WI.
 */
union nixx_lf_rq_op_drop_pkts {
	u64 u;
	struct nixx_lf_rq_op_drop_pkts_s {
		u64 cnt                              : 48;
		u64 reserved_48_62                   : 15;
		u64 op_err                           : 1;
	} s;
	/* struct nixx_lf_rq_op_drop_pkts_s cn; */
};

static inline u64 NIXX_LF_RQ_OP_DROP_PKTS(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_LF_RQ_OP_DROP_PKTS(void)
{
	return 0x940;
}

/**
 * Register (RVU_PFVF_BAR2) nix#_lf_rq_op_int
 *
 * NIX LF Receive Queue Interrupt Operation Register A 64-bit atomic
 * load-and-add to this register reads RQ interrupts and interrupt
 * enables. A 64-bit write optionally sets or clears interrupts and
 * interrupt enables.  All other accesses to this register (e.g. reads,
 * 128-bit accesses) are RAZ/WI.  RSL accesses to this register are
 * RAZ/WI.
 */
union nixx_lf_rq_op_int {
	u64 u;
	struct nixx_lf_rq_op_int_s {
		u64 rq_int                           : 8;
		u64 rq_int_ena                       : 8;
		u64 reserved_16_41                   : 26;
		u64 op_err                           : 1;
		u64 setop                            : 1;
		u64 rq                               : 20;
	} s;
	/* struct nixx_lf_rq_op_int_s cn; */
};

static inline u64 NIXX_LF_RQ_OP_INT(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_LF_RQ_OP_INT(void)
{
	return 0x900;
}

/**
 * Register (RVU_PFVF_BAR2) nix#_lf_rq_op_octs
 *
 * NIX LF Receive Queue Octets Operation Register A 64-bit atomic load-
 * and-add to this register reads NIX_RQ_CTX_S[OCTS]. The atomic write
 * data has format NIX_OP_Q_WDATA_S and selects the RQ within LF.  All
 * other accesses to this register (e.g. reads and writes) are RAZ/WI.
 * RSL accesses to this register are RAZ/WI.
 */
union nixx_lf_rq_op_octs {
	u64 u;
	struct nixx_lf_rq_op_octs_s {
		u64 cnt                              : 48;
		u64 reserved_48_62                   : 15;
		u64 op_err                           : 1;
	} s;
	/* struct nixx_lf_rq_op_octs_s cn; */
};

static inline u64 NIXX_LF_RQ_OP_OCTS(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_LF_RQ_OP_OCTS(void)
{
	return 0x910;
}

/**
 * Register (RVU_PFVF_BAR2) nix#_lf_rq_op_pkts
 *
 * NIX LF Receive Queue Packets Operation Register A 64-bit atomic load-
 * and-add to this register reads NIX_RQ_CTX_S[PKTS]. The atomic write
 * data has format NIX_OP_Q_WDATA_S and selects the RQ within LF.  All
 * other accesses to this register (e.g. reads and writes) are RAZ/WI.
 * RSL accesses to this register are RAZ/WI.
 */
union nixx_lf_rq_op_pkts {
	u64 u;
	struct nixx_lf_rq_op_pkts_s {
		u64 cnt                              : 48;
		u64 reserved_48_62                   : 15;
		u64 op_err                           : 1;
	} s;
	/* struct nixx_lf_rq_op_pkts_s cn; */
};

static inline u64 NIXX_LF_RQ_OP_PKTS(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_LF_RQ_OP_PKTS(void)
{
	return 0x920;
}

/**
 * Register (RVU_PFVF_BAR2) nix#_lf_rq_op_re_pkts
 *
 * NIX LF Receive Queue Errored Packets Operation Register A 64-bit
 * atomic load-and-add to this register reads NIX_RQ_CTX_S[RE_PKTS]. The
 * atomic write data has format NIX_OP_Q_WDATA_S and selects the RQ
 * within LF.  All other accesses to this register (e.g. reads and
 * writes) are RAZ/WI.  RSL accesses to this register are RAZ/WI.
 */
union nixx_lf_rq_op_re_pkts {
	u64 u;
	struct nixx_lf_rq_op_re_pkts_s {
		u64 cnt                              : 48;
		u64 reserved_48_62                   : 15;
		u64 op_err                           : 1;
	} s;
	/* struct nixx_lf_rq_op_re_pkts_s cn; */
};

static inline u64 NIXX_LF_RQ_OP_RE_PKTS(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_LF_RQ_OP_RE_PKTS(void)
{
	return 0x950;
}

/**
 * Register (RVU_PFVF_BAR2) nix#_lf_rx_secret#
 *
 * NIX LF Receive Secret Key Registers
 */
union nixx_lf_rx_secretx {
	u64 u;
	struct nixx_lf_rx_secretx_s {
		u64 key                              : 64;
	} s;
	/* struct nixx_lf_rx_secretx_s cn; */
};

static inline u64 NIXX_LF_RX_SECRETX(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_LF_RX_SECRETX(u64 a)
{
	return 0 + 8 * a;
}

/**
 * Register (RVU_PFVF_BAR2) nix#_lf_rx_stat#
 *
 * NIX LF Receive Statistics Registers The last dimension indicates which
 * statistic, and is enumerated by NIX_STAT_LF_RX_E.
 */
union nixx_lf_rx_statx {
	u64 u;
	struct nixx_lf_rx_statx_s {
		u64 stat                             : 48;
		u64 reserved_48_63                   : 16;
	} s;
	/* struct nixx_lf_rx_statx_s cn; */
};

static inline u64 NIXX_LF_RX_STATX(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_LF_RX_STATX(u64 a)
{
	return 0x400 + 8 * a;
}

/**
 * Register (RVU_PFVF_BAR2) nix#_lf_send_err_dbg
 *
 * NIX LF Send Error Debug Register This register captures debug info an
 * error detected on packet send after a meta-descriptor is granted by
 * PSE. Hardware sets [VALID] when the debug info is captured, and
 * subsequent errors are not captured until software clears [VALID] by
 * writing a one to it.
 */
union nixx_lf_send_err_dbg {
	u64 u;
	struct nixx_lf_send_err_dbg_s {
		u64 errcode                          : 8;
		u64 sq                               : 20;
		u64 sqe_id                           : 16;
		u64 valid                            : 1;
		u64 reserved_45_63                   : 19;
	} s;
	/* struct nixx_lf_send_err_dbg_s cn; */
};

static inline u64 NIXX_LF_SEND_ERR_DBG(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_LF_SEND_ERR_DBG(void)
{
	return 0x280;
}

/**
 * Register (RVU_PFVF_BAR2) nix#_lf_sq_op_drop_octs
 *
 * NIX LF Send Queue Dropped Octets Operation Register A 64-bit atomic
 * load-and-add to this register reads NIX_SQ_CTX_S[DROP_OCTS]. The
 * atomic write data has format NIX_OP_Q_WDATA_S and selects the SQ
 * within LF.  All other accesses to this register (e.g. reads and
 * writes) are RAZ/WI.  RSL accesses to this register are RAZ/WI.
 */
union nixx_lf_sq_op_drop_octs {
	u64 u;
	struct nixx_lf_sq_op_drop_octs_s {
		u64 cnt                              : 48;
		u64 reserved_48_62                   : 15;
		u64 op_err                           : 1;
	} s;
	/* struct nixx_lf_sq_op_drop_octs_s cn; */
};

static inline u64 NIXX_LF_SQ_OP_DROP_OCTS(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_LF_SQ_OP_DROP_OCTS(void)
{
	return 0xa40;
}

/**
 * Register (RVU_PFVF_BAR2) nix#_lf_sq_op_drop_pkts
 *
 * NIX LF Send Queue Dropped Packets Operation Register A 64-bit atomic
 * load-and-add to this register reads NIX_SQ_CTX_S[DROP_PKTS]. The
 * atomic write data has format NIX_OP_Q_WDATA_S and selects the SQ
 * within LF.  All other accesses to this register (e.g. reads and
 * writes) are RAZ/WI.  RSL accesses to this register are RAZ/WI.
 */
union nixx_lf_sq_op_drop_pkts {
	u64 u;
	struct nixx_lf_sq_op_drop_pkts_s {
		u64 cnt                              : 48;
		u64 reserved_48_62                   : 15;
		u64 op_err                           : 1;
	} s;
	/* struct nixx_lf_sq_op_drop_pkts_s cn; */
};

static inline u64 NIXX_LF_SQ_OP_DROP_PKTS(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_LF_SQ_OP_DROP_PKTS(void)
{
	return 0xa50;
}

/**
 * Register (RVU_PFVF_BAR2) nix#_lf_sq_op_err_dbg
 *
 * NIX LF SQ Operation Error Debug Register This register captures debug
 * info for an error detected on LMT store to NIX_LF_OP_SEND() or when a
 * NIX_LF_SQ_OP_* register is accessed. Hardware sets [VALID] when the
 * debug info is captured, and subsequent errors are not captured until
 * software clears [VALID] by writing a one to it.
 */
union nixx_lf_sq_op_err_dbg {
	u64 u;
	struct nixx_lf_sq_op_err_dbg_s {
		u64 errcode                          : 8;
		u64 sq                               : 20;
		u64 sqe_id                           : 16;
		u64 valid                            : 1;
		u64 reserved_45_63                   : 19;
	} s;
	/* struct nixx_lf_sq_op_err_dbg_s cn; */
};

static inline u64 NIXX_LF_SQ_OP_ERR_DBG(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_LF_SQ_OP_ERR_DBG(void)
{
	return 0x260;
}

/**
 * Register (RVU_PFVF_BAR2) nix#_lf_sq_op_int
 *
 * NIX LF Send Queue Interrupt Operation Register A 64-bit atomic load-
 * and-add to this register reads SQ interrupts, interrupt enables and
 * XOFF status. A write optionally sets or clears interrupts, interrupt
 * enables and XOFF status. A read is RAZ.  RSL accesses to this register
 * are RAZ/WI.
 */
union nixx_lf_sq_op_int {
	u64 u;
	struct nixx_lf_sq_op_int_s {
		u64 sq_int                           : 8;
		u64 sq_int_ena                       : 8;
		u64 xoff                             : 1;
		u64 reserved_17_41                   : 25;
		u64 op_err                           : 1;
		u64 setop                            : 1;
		u64 sq                               : 20;
	} s;
	/* struct nixx_lf_sq_op_int_s cn; */
};

static inline u64 NIXX_LF_SQ_OP_INT(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_LF_SQ_OP_INT(void)
{
	return 0xa00;
}

/**
 * Register (RVU_PFVF_BAR2) nix#_lf_sq_op_octs
 *
 * NIX LF Send Queue Octets Operation Register A 64-bit atomic load-and-
 * add to this register reads NIX_SQ_CTX_S[OCTS]. The atomic write data
 * has format NIX_OP_Q_WDATA_S and selects the SQ within LF.  All other
 * accesses to this register (e.g. reads and writes) are RAZ/WI.  RSL
 * accesses to this register are RAZ/WI.
 */
union nixx_lf_sq_op_octs {
	u64 u;
	struct nixx_lf_sq_op_octs_s {
		u64 cnt                              : 48;
		u64 reserved_48_62                   : 15;
		u64 op_err                           : 1;
	} s;
	/* struct nixx_lf_sq_op_octs_s cn; */
};

static inline u64 NIXX_LF_SQ_OP_OCTS(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_LF_SQ_OP_OCTS(void)
{
	return 0xa10;
}

/**
 * Register (RVU_PFVF_BAR2) nix#_lf_sq_op_pkts
 *
 * NIX LF Send Queue Packets Operation Register A 64-bit atomic load-and-
 * add to this register reads NIX_SQ_CTX_S[PKTS]. The atomic write data
 * has format NIX_OP_Q_WDATA_S and selects the SQ within LF.  All other
 * accesses to this register (e.g. reads and writes) are RAZ/WI.  RSL
 * accesses to this register are RAZ/WI.
 */
union nixx_lf_sq_op_pkts {
	u64 u;
	struct nixx_lf_sq_op_pkts_s {
		u64 cnt                              : 48;
		u64 reserved_48_62                   : 15;
		u64 op_err                           : 1;
	} s;
	/* struct nixx_lf_sq_op_pkts_s cn; */
};

static inline u64 NIXX_LF_SQ_OP_PKTS(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_LF_SQ_OP_PKTS(void)
{
	return 0xa20;
}

/**
 * Register (RVU_PFVF_BAR2) nix#_lf_sq_op_status
 *
 * NIX LF Send Queue Status Operation Register A 64-bit atomic load-and-
 * add to this register reads status fields in NIX_SQ_CTX_S. The atomic
 * write data has format NIX_OP_Q_WDATA_S and selects the SQ within LF.
 * Completion of the load-and-add operation also ensures that all
 * previously issued LMT stores to NIX_LF_OP_SEND() have completed.  All
 * other accesses to this register (e.g. reads and writes) are RAZ/WI.
 * RSL accesses to this register are RAZ/WI.
 */
union nixx_lf_sq_op_status {
	u64 u;
	struct nixx_lf_sq_op_status_s {
		u64 sqb_count                        : 16;
		u64 reserved_16_19                   : 4;
		u64 head_offset                      : 6;
		u64 reserved_26_27                   : 2;
		u64 tail_offset                      : 6;
		u64 reserved_34_62                   : 29;
		u64 op_err                           : 1;
	} s;
	struct nixx_lf_sq_op_status_cn {
		u64 sqb_count                        : 16;
		u64 reserved_16_19                   : 4;
		u64 head_offset                      : 6;
		u64 reserved_26_27                   : 2;
		u64 tail_offset                      : 6;
		u64 reserved_34_35                   : 2;
		u64 reserved_36_62                   : 27;
		u64 op_err                           : 1;
	} cn;
};

static inline u64 NIXX_LF_SQ_OP_STATUS(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_LF_SQ_OP_STATUS(void)
{
	return 0xa30;
}

/**
 * Register (RVU_PFVF_BAR2) nix#_lf_tx_stat#
 *
 * NIX LF Transmit Statistics Registers The last dimension indicates
 * which statistic, and is enumerated by NIX_STAT_LF_TX_E.
 */
union nixx_lf_tx_statx {
	u64 u;
	struct nixx_lf_tx_statx_s {
		u64 stat                             : 48;
		u64 reserved_48_63                   : 16;
	} s;
	/* struct nixx_lf_tx_statx_s cn; */
};

static inline u64 NIXX_LF_TX_STATX(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_LF_TX_STATX(u64 a)
{
	return 0x300 + 8 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_priv_af_int_cfg
 *
 * NIX Privileged Admin Function Interrupt Configuration Register
 */
union nixx_priv_af_int_cfg {
	u64 u;
	struct nixx_priv_af_int_cfg_s {
		u64 msix_offset                      : 11;
		u64 reserved_11                      : 1;
		u64 msix_size                        : 8;
		u64 reserved_20_63                   : 44;
	} s;
	/* struct nixx_priv_af_int_cfg_s cn; */
};

static inline u64 NIXX_PRIV_AF_INT_CFG(void)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_PRIV_AF_INT_CFG(void)
{
	return 0x8000000;
}

/**
 * Register (RVU_PF_BAR0) nix#_priv_lf#_cfg
 *
 * NIX Privileged Local Function Configuration Registers These registers
 * allow each NIX local function (LF) to be provisioned to a VF/PF for
 * RVU. See also NIX_AF_RVU_LF_CFG_DEBUG.  Software should read this
 * register after write to ensure that the LF is mapped to [PF_FUNC]
 * before issuing transactions to the mapped PF and function.  [SLOT]
 * must be zero.  Internal: Hardware ignores [SLOT] and always assumes
 * 0x0.
 */
union nixx_priv_lfx_cfg {
	u64 u;
	struct nixx_priv_lfx_cfg_s {
		u64 slot                             : 8;
		u64 pf_func                          : 16;
		u64 reserved_24_62                   : 39;
		u64 ena                              : 1;
	} s;
	/* struct nixx_priv_lfx_cfg_s cn; */
};

static inline u64 NIXX_PRIV_LFX_CFG(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_PRIV_LFX_CFG(u64 a)
{
	return 0x8000010 + 0x100 * a;
}

/**
 * Register (RVU_PF_BAR0) nix#_priv_lf#_int_cfg
 *
 * NIX Privileged LF Interrupt Configuration Registers
 */
union nixx_priv_lfx_int_cfg {
	u64 u;
	struct nixx_priv_lfx_int_cfg_s {
		u64 msix_offset                      : 11;
		u64 reserved_11                      : 1;
		u64 msix_size                        : 8;
		u64 reserved_20_63                   : 44;
	} s;
	/* struct nixx_priv_lfx_int_cfg_s cn; */
};

static inline u64 NIXX_PRIV_LFX_INT_CFG(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NIXX_PRIV_LFX_INT_CFG(u64 a)
{
	return 0x8000020 + 0x100 * a;
}

#endif /* __CSRS_NIX_H__ */
