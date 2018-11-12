/* HEADER Testing duplicate handling in binary OID array */

/* Much copied from T012 */
static const char test_name[] = "binary-array-of-OIDs-duplicate-test";
oid o1 = 1;
oid o2 = 2;
oid o3 = 6;
oid o4 = 8;
oid o5 = 9;
oid ox = 7;
oid oy = 10;
netsnmp_index i1, i2, i3, i4, i5, ix, iy, *ip;
netsnmp_index *b[] = { &i4, &i2, &i3, &i1, &i5 };
netsnmp_container *c;
int i;

init_snmp(test_name);

c = netsnmp_container_get_binary_array();
c->compare = netsnmp_compare_netsnmp_index;
netsnmp_binary_array_options_set(c, 1,
				 CONTAINER_KEY_ALLOW_DUPLICATES);

i1.oids = &o1;
i2.oids = &o2;
i3.oids = &o3;
i4.oids = &o4;
i5.oids = &o5;
ix.oids = &ox;
iy.oids = &oy;
i1.len = i2.len = i3.len = i4.len = i5.len = ix.len = iy.len = 1;

for (i = 0; i < sizeof(b)/sizeof(b[0]); ++i)
    CONTAINER_INSERT(c, b[i]);

#define MAX_ROUNDS 6
/* Insert some duplicates of i4; also insert a duplicate of
 * i1 to move the contents of the array around. */
for (i = 0; i < MAX_ROUNDS; ++i) {
    switch (i) {
	case 0:
	    /* First round: no insert */
	    break;
	case 1:
	case 2:
	case 4:
	case 5:
	    /* Insert another duplicate of our target object */
	    CONTAINER_INSERT(c, &i4);
	    break;
	case 3:
	    /* Insert a dulicate of an earlier OID, so that it
	     * changes the binary search behavior */
	    CONTAINER_INSERT(c, &i1);
	    break;
    }
    /* Primary requirement: getnext returns the next value! */
    ip = CONTAINER_FIND(c, &i4);
    OKF(ip, ("FIND returned a value"));
    OKF(c->compare(&i4, ip) == 0,
        ("FIND returned oid %" NETSNMP_PRIo "d", ip->oids[0]));
    ip = CONTAINER_NEXT(c, &i4);
    OKF(ip, ("NEXT returned a value"));
    OKF(c->compare(&i5, ip) == 0,
        ("NEXT returned index 5 = %" NETSNMP_PRIo "d", i5.oids[0]));
}

while ((ip = CONTAINER_FIRST(c)))
  CONTAINER_REMOVE(c, ip);
CONTAINER_FREE(c);

snmp_shutdown(test_name);
