/* HEADER Testing snmp_set_var_value() */

{
    static const signed char values[] = { -128, -23, -1, 1, 42, 127, 0 };
    int i;

    for (i = 0; values[i]; i++) {
        netsnmp_variable_list variable;
        int ec;

        memset(&variable, 0, sizeof(variable));
        variable.next_variable = NULL;
        variable.type = ASN_INTEGER;
        variable.val.string = NULL;
        variable.val.integer = NULL;
        variable.val_len = 0;

        ec = snmp_set_var_value( &variable, &values[i], 1);

        OK(ec == 0, "snmp_set_var_value() should succeed");
        OKF(*variable.val.integer == values[i],
            ("%ld =?= %d", *variable.val.integer, values[i]));
    }
}
