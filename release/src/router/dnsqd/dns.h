#ifndef __DNS_H__
#define __DNS_H__

/******************************************************************************/
/* DNS header bit flags.                                                      */
/******************************************************************************/
#define DNS_HDR_QR                                0x80
#define DNS_HDR_OPCODE                            0x78
#define DNS_HDR_AA                                0x04
#define DNS_HDR_TC                                0x02
#define DNS_HDR_RD                                0x01
#define DNS_HDR_RA                                0x80
#define DNS_HDR_RC                                0x0F

#define DNS_HDR_QR_RESP                           0x80
#define DNS_HDR_RCODE_NO_ERR                      0x0
#define DNS_HDR_RCODE_FORMAT_ERR                  0x1
#define DNS_HDR_RCODE_SERVER_FAILURE              0x2
#define DNS_HDR_RCODE_NAME_ERR                    0x3
#define DNS_HDR_RCODE_NOT_IMPL                    0x4
#define DNS_HDR_RCODE_REFUSED                     0x5


/******************************************************************************/
/* DNS RR types.                                                              */
/******************************************************************************/
#define DNS_RR_TYPE_A                             1
#define DNS_RR_TYPE_NS                            2
#define DNS_RR_TYPE_CNAME                         5
#define DNS_RR_TYPE_SOA                           6
#define DNS_RR_TYPE_PTR                           12
#define DNS_RR_TYPE_MX                            15
#define DNS_RR_TYPE_AAAA                          28

#define DNS_RR_TYPE_A_LEN                         4

/******************************************************************************/
/* DNS RR class.                                                              */
/******************************************************************************/
#define DNS_RR_CLASS_IN                           1

/******************************************************************************/
/* DNS DATA ERROR.                                                              */
/******************************************************************************/
#define DNS_DATA_GENERIC_ERR                      -1
#define DNS_DATA_RESPONSE_TYPE_ERR                -2
#define DNS_DATA_NO_ANSWER_ERR                    -3
#define DNS_DATA_NO_QUESTION_ERR                  -4
#define DNS_DATA_REPLY_ERR                        -5

#define DNS_PAYLOADZ                              512
#define DNS_MAX_NAME_LEN                          253
#define DNS_MAX_LABEL_LEN                         63
#define DNS_MAX_NUM_LABELS                        127
#define DNS_MAX_ANS_RR_NUM                        20
#define DNS_MAX_ANS_RR_LEN                        255

/*
* Masks and constants.
*/
#define QR_MASK                                   0x8000
#define OPCODE_MASK                               0x7800
#define AA_MASK                                   0x0400
#define TC_MASK                                   0x0200
#define RD_MASK                                   0x0100
#define RA_MASK                                   0x8000
#define RCODE_MASK                                0x000F

/* Response Type */
#define Ok_ResponseType                           0
#define FormatError_ResponseType                  1
#define ServerFailure_ResponseType                2
#define NameError_ResponseType                    3
#define NotImplemented_ResponseType               4
#define Refused_ResponseType                      5

/* OPCODE Type */
#define OPCODE_Query                              0
#define OPCODE_IQuery                             1
#define OPCODE_Status                             2
#define OPCODE_Unassigned                         3
#define OPCODE_Notify                             4
#define OPCODE_Update                             5
#define OPCODE_DSO                                6
/******************************************************************************/
/* DNS header structure.                                                      */
/******************************************************************************/
typedef struct _dns_header {
  unsigned short id;
  unsigned short fc;
  unsigned short q_count;
  unsigned short ans_count;
  unsigned short ns_count;
  unsigned short addrec_count;
} dns_header;

typedef struct _dns_answer
{
  int n_rec;
  char recs[DNS_MAX_ANS_RR_NUM][DNS_MAX_ANS_RR_LEN];
  bool isv4[DNS_MAX_ANS_RR_NUM];
} dns_answer;

typedef struct _dns_question
{
  int n_label;
  char name[DNS_MAX_NAME_LEN + 1];
  short q_type;
  short q_class;
  dns_answer ans;
} dns_question;


#ifndef INET6_ADDRSTRLEN
#define INET6_ADDRSTRLEN        48
#endif
typedef struct _dns_msg {
  int isv4;
  char sip[INET6_ADDRSTRLEN];
  unsigned int src_ip;
  struct in6_addr srcv6;
  int sport;
  char dip[INET6_ADDRSTRLEN];
  unsigned int dst_ip;
  struct in6_addr dstv6;
  int dport;
  dns_header header;
  dns_question question;  // support 1 question now
} dns_msg;

typedef enum {
    NFCM_APP = 0,
    DNS_QUERY = 1,
    APP_CLIENT = 2,
    TASK_STATUS = 3,
    APP_SUM = 4,
    DB_MAX = 5
} DNS_DB_TYPE;

typedef enum {
    UNKNOWN_DEV = 0,
    ANDROID_DEV = 1,
    IOS_DEV = 2,
    WINDOWS_DEV = 3,
    UBUNTU_DEV = 4,
    PS_DEV = 5,
    XBOX_DEV = 6,
    NINTENDO_DEV = 7
} DEV_IDENTIFY_TYPE;

#define DEV_TYPE_LEN 20
#define BUF_SIZE 1500
#define APP_CLIENT_DATA_COLLECT_INTERVAL 60
#define FETCH_ARP_MAC_INTERVAL 11
#define MIN(x, y) ((x) <= (y) ? (x) : (y))

#define ENABLE_DNS_BLOCK 0
#define NAME_IN_BLACK_LIST 0
#define NAME_IN_WHITE_LIST 1

#define ONE_K (1024)
#define ONE_M (1048576)  // ONE_K*ONE_K
#define DAY_SEC        (86400)

#endif /* __DNS_H__ */
