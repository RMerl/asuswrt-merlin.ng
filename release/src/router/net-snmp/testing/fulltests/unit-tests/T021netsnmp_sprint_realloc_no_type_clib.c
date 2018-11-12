/* HEADER Test sprint_realloc_variable with -OQ */

/*
 * Check that sprint_realloc_variable does not crash when it prints
 * a variable with NULL type, while MIG says it's different type (INTEGER, ...).
 * We will use MIB definition of nlmLogVariableTable, which has a column
 * for most of the MIB types.
 */
static oid objid[] = {
        1, 3, 6, 1, 2, 1, 92, 1, 3, 2, 1, 4
};

u_char *buf;
size_t buf_len, out_len;
int buf_overflow, i;
char mibdir[PATH_MAX];
int ret;

snprintf(mibdir, sizeof(mibdir), "%s/%s", ABS_SRCDIR, "mibs");
netsnmp_ds_set_string(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_MIBDIRS, mibdir);
netsnmp_ds_set_boolean(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_QUICKE_PRINT, TRUE);

init_snmp("T021");

netsnmp_variable_list variable;
variable.next_variable = NULL;
variable.name = objid;
variable.name_length = sizeof(objid) / sizeof(objid[0]);
variable.type = ASN_NULL;
variable.val.integer = NULL;
variable.val_len = 0;


buf = NULL;
buf_len = out_len = buf_overflow = 0;

/* Try to format variable with nlmLogVariableEntry.4 - nlmLogVariableEntry.11
 * OIDs and with NULL type.
 */
for (i=4; i<=11; i++) {
    buf = NULL;
    buf_len = out_len = buf_overflow = 0;
    objid[11] = i;
    ret = sprint_realloc_variable(&buf, &buf_len, &out_len, 1, objid,
            sizeof(objid) / sizeof(objid[0]), &variable);
    OKF(ret == 1, ("sprint_realloc_variable for %d returned %d: %s, expected 1",
            i, ret, buf));
    free(buf);
}

snmp_shutdown("T021");
