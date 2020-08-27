#pragma once

#define XT_DNS_MAXSIZE 255
#define XT_DNS_LABEL_MAXSIZE 63
#define DNS_PORT 53

/* DNS matching stuff */
struct xt_dns {
    bool qr;                       /* QR Bit */
    unsigned opcode : 4;           /* OPCODE */
    bool aa;                       /* AA Bit */
    bool tc;                       /* TC Bit */
    bool rd;                       /* RD Bit */
    bool ra;                       /* RA Bit */
    bool ad;                       /* AD Bit */
    bool cd;                       /* CD Bit */
    unsigned rcode : 4;            /* RCODE */
    uint8_t qname[XT_DNS_MAXSIZE]; /* Qname */
    uint16_t qtype;                /* QTYPE */

    bool rmatch;     /* reverse match */
    uint8_t maxsize; /* max size */

    uint16_t invflags; /* Inverse Flags */
    uint16_t setflags; /* Set Confitional flag */

    uint8_t qname_size;
};

#define XT_DNS_FLAG_QR 0x0001
#define XT_DNS_FLAG_OPCODE 0x0002
#define XT_DNS_FLAG_AA 0x0004
#define XT_DNS_FLAG_TC 0x0008
#define XT_DNS_FLAG_RD 0x0010
#define XT_DNS_FLAG_RA 0x0020
#define XT_DNS_FLAG_AD 0x0040
#define XT_DNS_FLAG_CD 0x0080
#define XT_DNS_FLAG_RCODE 0x0100
#define XT_DNS_FLAG_QNAME 0x0200
#define XT_DNS_FLAG_QTYPE 0x0400
#define XT_DNS_FLAG_RMATCH 0x0800
#define XT_DNS_FLAG_QNAME_MAXSIZE 0x1000
#define XT_DNS_FLAG_MASK 0x1FFF
