#if defined(CONFIG_BCM_KF_XT_MATCH_LAYER7)
#ifndef _XT_LAYER7_H
#define _XT_LAYER7_H

#if 0
#define MAX_PATTERN_LEN 8192
#define MAX_PROTOCOL_LEN 256
#else
#define MAX_PATTERN_LEN 256
#define MAX_PROTOCOL_LEN 64
#endif

struct xt_layer7_info {
    char protocol[MAX_PROTOCOL_LEN];
    char pattern[MAX_PATTERN_LEN];
    u_int8_t invert;
    u_int8_t pkt;
};

#endif /* _XT_LAYER7_H */
#endif
