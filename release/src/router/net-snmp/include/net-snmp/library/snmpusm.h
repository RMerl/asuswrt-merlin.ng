/*
 * snmpusm.h
 *
 * Header file for USM support.
 *
 * Portions of this file are copyrighted by:
 * Copyright (c) 2016 VMware, Inc. All rights reserved.
 * Use is subject to license terms specified in the COPYING file
 * distributed with the Net-SNMP package.
 */

#ifndef SNMPUSM_H
#define SNMPUSM_H

#include <net-snmp/library/callback.h>

#ifdef __cplusplus
extern          "C" {
#endif

#define WILDCARDSTRING "*"

    /*
     * General.
     */
#define USM_MAX_ID_LENGTH		1024    /* In bytes. */
#define USM_MAX_SALT_LENGTH		128     /* In BITS. */
#define USM_DES_SALT_LENGTH		64      /* In BITS. */
#define USM_AES_SALT_LENGTH		128     /* In BITS. */
#define USM_MAX_KEYEDHASH_LENGTH	128     /* In BITS. */

#define USM_TIME_WINDOW			150
#define USM_MD5_AND_SHA_AUTH_LEN        12      /* bytes */
#define USM_HMAC128SHA224_AUTH_LEN      16      /* OPTIONAL */
#define USM_HMAC192SHA256_AUTH_LEN      24      /* MUST */
#define USM_HMAC256SHA384_AUTH_LEN      32      /* OPTIONAL */
#define USM_HMAC384SHA512_AUTH_LEN      48      /* SHOULD */
#define USM_MAX_AUTHSIZE                USM_HMAC384SHA512_AUTH_LEN

#define USM_SEC_MODEL_NUMBER            SNMP_SEC_MODEL_USM

    /*
     * Structures.
     */
    struct usmStateReference;

    /*
     * struct usmUser: a structure to represent a given user in a list 
     */
    /*
     * Note: Any changes made to this structure need to be reflected in
     * the following functions: 
     */

    struct usmUser;
    struct usmUser {
        u_int          flags;
        u_char         *engineID;
        size_t          engineIDLen;
        char           *name;
        char           *secName;
        oid            *cloneFrom;
        size_t          cloneFromLen;
        oid            *authProtocol;
        size_t          authProtocolLen;
        u_char         *authKey;
        size_t          authKeyLen;
        u_char         *authKeyKu;
        size_t          authKeyKuLen;
        oid            *privProtocol;
        size_t          privProtocolLen;
        u_char         *privKeyKu;
        size_t          privKeyKuLen;
        u_char         *privKey;
        size_t          privKeyLen;
        u_char         *userPublicString;
        size_t          userPublicStringLen;
        int             userStatus;
        int             userStorageType;
       /* these are actually DH * pointers but only if openssl is avail. */
        void           *usmDHUserAuthKeyChange;
        void           *usmDHUserPrivKeyChange;
        struct usmUser *next;
        struct usmUser *prev;
    };

#define USMUSER_FLAG_KEEP_MASTER_KEY             0x01


    /*
     * Prototypes.
     */
    NETSNMP_IMPORT
    int             usm_extend_user_kul(struct usmUser *user,
                                        u_int privKeyBufSize);
    NETSNMP_IMPORT
    struct usmUser *usm_get_userList(void);
    NETSNMP_IMPORT
    struct usmUser *usm_get_user(const u_char *engineID, size_t engineIDLen,
                                 const char *name);
    NETSNMP_IMPORT
    struct usmUser *usm_add_user(struct usmUser *user);
    NETSNMP_IMPORT
    struct usmUser *usm_free_user(struct usmUser *user);
    NETSNMP_IMPORT
    struct usmUser *usm_create_user(void);
    NETSNMP_IMPORT
    struct usmUser *usm_cloneFrom_user(struct usmUser *from,
                                       struct usmUser *to);
    NETSNMP_IMPORT
    struct usmUser *usm_remove_user(struct usmUser *user);
    NETSNMP_IMPORT
    void            usm_parse_config_usmUser(const char *token,
                                             char *line);
    NETSNMP_IMPORT
    void            usm_set_user_password(struct usmUser *user,
                                          const char *token, char *line);
    void            init_usm(void);
    NETSNMP_IMPORT
    void            init_usm_conf(const char *app);
    NETSNMP_IMPORT
    void            shutdown_usm(void);
    NETSNMP_IMPORT
    int             usm_lookup_auth_type(const char *str);
    NETSNMP_IMPORT
    const char     *usm_lookup_auth_str(int value);
    NETSNMP_IMPORT
    oid            *usm_get_auth_oid(int auth_type, size_t *oid_len);
    NETSNMP_IMPORT
    int             usm_lookup_priv_type(const char *str);
    NETSNMP_IMPORT
    const char     *usm_lookup_priv_str(int value);
    NETSNMP_IMPORT
    oid            *usm_get_priv_oid(int priv_type, size_t *oid_len);


#define USM_CREATE_USER_AUTH_DFLT -1
#define USM_CREATE_USER_AUTH_NONE NETSNMP_USMAUTH_NONE
#define USM_CREATE_USER_AUTH_MD5  NETSNMP_USMAUTH_HMACMD5
#define USM_CREATE_USER_AUTH_SHA1 NETSNMP_USMAUTH_HMACSHA1
#define USM_CREATE_USER_AUTH_SHA  USM_CREATE_USER_AUTH_SHA1
#define USM_CREATE_USER_AUTH_SHA512  NETSNMP_USMAUTH_HMAC384SHA512
#define USM_CREATE_USER_AUTH_SHA384  NETSNMP_USMAUTH_HMAC256SHA384
#define USM_CREATE_USER_AUTH_SHA256  NETSNMP_USMAUTH_HMAC192SHA256
#define USM_CREATE_USER_AUTH_SHA224  NETSNMP_USMAUTH_HMAC128SHA224

    /** flags for variants fo priv algorithsm */
#define USM_DES_FLAG_3                      0x000100

#define USM_AES_FLAG_192                    0x000100
#define USM_AES_FLAG_256                    0x000200

#define USM_AES_REEDER_FLAG                 0x030000
#define USM_AES_FLAG_CISCO                  0x100000

#define USM_PRIV_MASK_ALG                   0x0000ff
#define USM_PRIV_MASK_VARIANT               0x00ff00

#define USM_CREATE_USER_PRIV_DFLT          -1
#define USM_CREATE_USER_PRIV_NONE           0

#define USM_CREATE_USER_PRIV_DES            0x01
#define USM_CREATE_USER_PRIV_3DES           \
    (USM_CREATE_USER_PRIV_DES | USM_DES_FLAG_3)

#define USM_CREATE_USER_PRIV_AES            0x02
#define USM_CREATE_USER_PRIV_AES192         \
    (USM_CREATE_USER_PRIV_AES | USM_AES_FLAG_192)
#define USM_CREATE_USER_PRIV_AES256         \
    (USM_CREATE_USER_PRIV_AES | USM_AES_FLAG_256)

#define USM_CREATE_USER_PRIV_AES192_CISCO   \
    (USM_CREATE_USER_PRIV_AES | USM_AES_FLAG_192 | USM_AES_FLAG_CISCO \
     | USM_AES_REEDER_FLAG)
#define USM_CREATE_USER_PRIV_AES256_CISCO   \
    (USM_CREATE_USER_PRIV_AES | USM_AES_FLAG_256 | USM_AES_FLAG_CISCO \
     | USM_AES_REEDER_FLAG)


    NETSNMP_IMPORT
    int             usm_create_user_from_session(netsnmp_session * session);
    NETSNMP_IMPORT
    void            usm_parse_create_usmUser(const char *token,
                                             char *line);
    NETSNMP_IMPORT
    const oid      *get_default_authtype(size_t *);
    NETSNMP_IMPORT
    const oid      *get_default_privtype(size_t *);

#ifdef __cplusplus
}
#endif
#endif                          /* SNMPUSM_H */
