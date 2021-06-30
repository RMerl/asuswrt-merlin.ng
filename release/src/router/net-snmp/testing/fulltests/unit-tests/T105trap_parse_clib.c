/* HEADER Parsing of an SNMP trap with no varbinds */
netsnmp_pdu pdu;
int rc;
static u_char trap_pdu[] = {
    /* Sequence with length of 0x2d = 45 bytes. */
    [ 0] = 0x30, [ 1] = 0x82, [ 2] = 0x00, [ 3] = 0x2d,
    /* version = INTEGER 0 */
    [ 4] = 0x02, [ 5] = 0x01, [ 6] = 0x00,
    /* community = public (OCTET STRING 0x70 0x75 0x62 0x6c 0x69 0x63) */
    [ 7] = 0x04, [ 8] = 0x06, [ 9] = 0x70, [10] = 0x75,
    [11] = 0x62, [12] = 0x6c, [13] = 0x69, [14] = 0x63,
    /* SNMP_MSG_TRAP; 32 bytes. */
    [15] = 0xa4, [16] = 0x20,
    /* enterprise = OBJECT IDENTIFIER .1.3.6.1.6.3.1.1.5 = snmpTraps */
    [17] = 0x06, [18] = 0x08,
    [19] = 0x2b, [20] = 0x06, [21] = 0x01, [22] = 0x06,
    [23] = 0x03, [24] = 0x01, [25] = 0x01, [26] = 0x05,
    /* agent-addr = ASN_IPADDRESS 192.168.1.34 */
    [27] = 0x40, [28] = 0x04, [29] = 0xc0, [30] = 0xa8,
    [31] = 0x01, [32] = 0x22,
    /* generic-trap = INTEGER 0 */
    [33] = 0x02, [34] = 0x01, [35] = 0x00,
    /* specific-trap = INTEGER 0 */
    [36] = 0x02, [37] = 0x01, [38] = 0x00,
    /* ASN_TIMETICKS 0x117f243a */
    [39] = 0x43, [40] = 0x04, [41] = 0x11, [42] = 0x7f,
    [43] = 0x24, [44] = 0x3a,
    /* varbind list */
    [45] = 0x30, [46] = 0x82, [47] = 0x00, [48] = 0x00,
};
static size_t trap_pdu_length = sizeof(trap_pdu);
netsnmp_session session;

snmp_set_do_debugging(TRUE);
debug_register_tokens("dumpv_recv,dumpv_send,asn,recv");
memset(&session, 0, sizeof(session));
snmp_sess_init(&session);
memset(&pdu, 0, sizeof(pdu));
rc = snmp_parse(NULL, &session, &pdu, trap_pdu, trap_pdu_length);

OKF((rc == 0), ("Parsing of a trap PDU"));
