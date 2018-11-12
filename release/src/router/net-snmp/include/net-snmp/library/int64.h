#ifndef INT64_INCLUDED
#define INT64_INCLUDED

#ifdef __cplusplus
extern          "C" {
#endif

#ifndef NETSNMP_REMOVE_U64
    /*
     * Note: using the U64 typedef is deprecated because this typedef conflicts
     * with a typedef with the same name defined in the Perl header files.
     */
    typedef struct counter64 U64;
#endif

#define I64CHARSZ 21

    void            divBy10(struct counter64, struct counter64 *,
                            unsigned int *);
    void            multBy10(struct counter64, struct counter64 *);
    void            incrByU16(struct counter64 *, unsigned int);
    void            incrByU32(struct counter64 *, unsigned int);
    NETSNMP_IMPORT
    void            zeroU64(struct counter64 *);
    int             isZeroU64(const struct counter64 *);
    NETSNMP_IMPORT
    void            printU64(char *, const struct counter64 *);
    NETSNMP_IMPORT
    void            printI64(char *, const struct counter64 *);
    int             read64(struct counter64 *, const char *);
    NETSNMP_IMPORT
    void            u64Subtract(const struct counter64 *pu64one,
                                const struct counter64 *pu64two,
                                struct counter64 *pu64out);
    void            u64Incr(struct counter64 *pu64out,
                            const struct counter64 *pu64one);
    void            u64UpdateCounter(struct counter64 *pu64out,
                                     const struct counter64 *pu64one,
                                     const struct counter64 *pu64two);
    void            u64Copy(struct counter64 *pu64one,
                            const struct counter64 *pu64two);

    int             netsnmp_c64_check_for_32bit_wrap(struct counter64 *old_val,
                                                     struct counter64 *new_val,
                                                     int adjust);
    NETSNMP_IMPORT
    int             netsnmp_c64_check32_and_update(struct counter64 *prev_val,
                                                   struct counter64 *new_val,
                                                   struct counter64 *old_prev_val,
                                                   int *need_wrap_check);

#ifdef __cplusplus
}
#endif
#endif
