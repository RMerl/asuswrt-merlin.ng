/* HEADER Testing netsnmp_sprint_realloc_objid_tree() */

static const oid objid1[] = {
    1, 3, 6, 1, 2, 1, 4, 32, 1, 5, 1, 1, 4, 127, 0, 0, 0, 8
};
static const oid objid2[] = {
    1, 3, 6, 1, 2, 1, 4, 32, 1, 5, 1, 2, 16, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 1, 128
};
static const oid objid3[] = {
    1, 3, 6, 1, 2, 1, 4, 32, 1, 5, 1, 1, 4, 255, 255, 255, 255, 8
};
static const oid objid4[] = {
    1, 3, 6, 1, 2, 1, 4, 32, 1, 5, 1, 2, 16, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 128
};
static const oid objid5[] = {
    1, 3, 6, 1, 2, 1, 4, 32, 1, 5, 1, 1, 4, 256, 256, 256, 256, 8
};
static const oid objid6[] = {
    1, 3, 6, 1, 2, 1, 4, 32, 1, 5, 1, 2, 16, 256, 256, 256, 256, 256, 256, 256,
    256, 256, 256, 256, 256, 256, 256, 256, 256, 128
};
static const oid objid7[] = {
    1, 3, 6, 1, 2, 1, 4, 32, 1, 5, 1, 1, 4, -1, -1, -1, -1, 8
};
static const oid objid8[] = {
    1, 3, 6, 1, 2, 1, 4, 32, 1, 5, 1, 2, 16, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, 128

};
struct objid_data { oid const *oid; int len; const char *str; };
static const struct objid_data objid_array[] = {
    { objid1, sizeof(objid1) / sizeof(objid1[0]),
      "IP-MIB::ipAddressPrefixOrigin.1.ipv4.\"127.0.0.0\".8" },
    { objid2, sizeof(objid2) / sizeof(objid2[0]),
      "IP-MIB::ipAddressPrefixOrigin.1.ipv6."
      "\"00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:01\".128" },
    { objid3, sizeof(objid3) / sizeof(objid3[0]),
      "IP-MIB::ipAddressPrefixOrigin.1.ipv4.\"255.255.255.255\".8" },
    { objid4, sizeof(objid4) / sizeof(objid4[0]),
      "IP-MIB::ipAddressPrefixOrigin.1.ipv6."
      "\"ff:ff:ff:ff:ff:ff:ff:ff:ff:ff:ff:ff:ff:ff:ff:ff\".128" },
    { objid5, sizeof(objid5) / sizeof(objid5[0]),
      "IP-MIB::ipAddressPrefixOrigin.1.ipv4.\"....\".8" },
    { objid6, sizeof(objid6) / sizeof(objid6[0]),
      "IP-MIB::ipAddressPrefixOrigin.1.ipv6.\"................\".128" },
    { objid7, sizeof(objid7) / sizeof(objid7[0]),
      "IP-MIB::ipAddressPrefixOrigin.1.ipv4.\"....\".8" },
    { objid8, sizeof(objid8) / sizeof(objid8[0]),
      "IP-MIB::ipAddressPrefixOrigin.1.ipv6.\"................\".128" },
};
struct tree *tree;
char *buf;
size_t buf_len, out_len;
int buf_overflow, i;
char mibdir[PATH_MAX];

snprintf(mibdir, sizeof(mibdir), "%s/%s", ABS_SRCDIR, "mibs");
netsnmp_ds_set_string(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_MIBDIRS, mibdir);

init_snmp("T020");

for (i = 0; i < sizeof(objid_array) / sizeof(objid_array[0]); i++) {
    buf = NULL;
    buf_len = out_len = buf_overflow = 0;
    tree = netsnmp_sprint_realloc_objid_tree((u_char **) &buf, &buf_len,
                                             &out_len, 1/*allow_realloc*/,
                                             &buf_overflow,
                                             objid_array[i].oid,
                                             objid_array[i].len);
    OK(!!objid_array[i].str == !!tree,
       "netsnmp_sprint_realloc_objid_tree() return value");
    if (objid_array[i].str && tree) {
        OKF(strcmp(objid_array[i].str, buf) == 0,
            ("Mismatch: expected %s but got %s", objid_array[i].str,
             buf ? (const char *)buf : "(NULL)"));
    }
    free(buf);
}

snmp_shutdown("T020");
