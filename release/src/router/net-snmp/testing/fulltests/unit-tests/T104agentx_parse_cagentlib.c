/* HEADER oss-fuzz bug 14502 */
/*
 * Verify whether agentx_parse() does not access any memory outside its
 * bounds for a particular invalid AgentX input. See also
 * https://bugs.chromium.org/p/oss-fuzz/issues/detail?id=14502.
 */

netsnmp_session session;
netsnmp_pdu pdu;
int rc;
static u_char data[] = {
    0x20, 0x08, 0x20, 0x20, 0x20, 0x20, 0xff, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x14, 0x00, 0x00, 0x00, 0x44, 0x00, 0x20, 0x20,
    0x00, 0x20, 0x20, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0x20, 0x20, 0x20, 0x20, 0x20, 0xff, 0x20,
};
static const size_t data_length = sizeof(data);

memset(&session, 0, sizeof(session));
session.version = AGENTX_VERSION_1;

memset(&pdu, 0, sizeof(pdu));

rc = agentx_parse(&session, &pdu, data, data_length);

fprintf(stderr, "rc = %d\n", rc);
fflush(stderr);

OKF(rc != 0, ("Parsing of AgentX data failed"));
