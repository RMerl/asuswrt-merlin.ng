""" Runs all unit tests for the netsnmp package.   """
# Copyright (c) 2006 Andy Gross.  See LICENSE.txt for details.

import os
import unittest
import netsnmp

def snmp_dest(**kwargs):
    """Return information about how to communicate with snmpd"""
    dest = {
        'Version':    1,
        'DestHost':   'localhost:' + os.environ.get("SNMP_SNMPD_PORT", 161),
        'Community':  'public',
    }
    for key, value in kwargs.iteritems():
        dest[key] = value
    return dest

def setup_v1():
    return netsnmp.Session(**snmp_dest())

def setup_v2():
    sess = netsnmp.Session(**snmp_dest(Version=2))
    sess.UseEnums = 1
    sess.UseLongNames = 1
    return sess

def setup_v3():
    sess = netsnmp.Session(**snmp_dest(Version=3,
                                       SecLevel='authPriv',
                                       SecName='initial',
                                       PrivPass='priv_pass',
                                       AuthPass='auth_pass'))
    sess.UseSprintValue = 1
    return sess

class BasicTests(unittest.TestCase):
    """Basic unit tests for the Net-SNMP Python interface"""
    def test_varbind_creation(self):
        var = netsnmp.Varbind('sysDescr.0')
        self.assertEqual(var.tag, 'sysDescr')
        self.assertEqual(var.iid, '0')

        var = netsnmp.Varbind('sysDescr', '0')
        self.assertEqual(var.tag, 'sysDescr')
        self.assertEqual(var.iid, '0')

        var = netsnmp.Varbind(
            '.iso.org.dod.internet.mgmt.mib-2.system.sysDescr', '0')
        self.assertEqual(var.tag,
                         '.iso.org.dod.internet.mgmt.mib-2.system.sysDescr')
        self.assertEqual(var.iid, '0')

        var = netsnmp.Varbind(
            '.iso.org.dod.internet.mgmt.mib-2.system.sysDescr.0')
        self.assertEqual(var.tag,
                         '.iso.org.dod.internet.mgmt.mib-2.system.sysDescr.0')
        self.assertEqual(var.iid, None)

        var = netsnmp.Varbind('.1.3.6.1.2.1.1.1.0')
        self.assertEqual(var.tag, '.1.3.6.1.2.1.1.1.0')
        self.assertEqual(var.iid, '')

    def test_v1_get(self):
        print "\n"
        print "---v1 GET tests -------------------------------------\n"
        var = netsnmp.Varbind('.1.3.6.1.2.1.1.1', '0')
        res = netsnmp.snmpget(var, **snmp_dest())

        print "v1 snmpget result: ", res, "\n"
        self.assertEqual(len(res), 1)

        print "v1 get var: ", var.tag, var.iid, "=", var.val, '(', var.type, ')'
        self.assertEqual(var.tag, 'sysDescr')
        self.assertEqual(var.iid, '0')
        self.assertEqual(var.val, res[0])
        self.assertEqual(var.type, 'OCTETSTR')

    def test_v1_getnext(self):
        print "\n"
        print "---v1 GETNEXT tests-------------------------------------\n"
        var = netsnmp.Varbind('.1.3.6.1.2.1.1.1', '0')
        res = netsnmp.snmpgetnext(var, **snmp_dest())

        print "v1 snmpgetnext result: ", res, "\n"
        self.assertEqual(len(res), 1)

        print "v1 getnext var: ", var.tag, var.iid, "=", var.val, '(', var.type, ')'
        self.assertTrue(var.tag is not None)
        self.assertTrue(var.iid is not None)
        self.assertTrue(var.val is not None)
        self.assertTrue(var.type is not None)

    def test_v1_set(self):
        print "\n"
        print "---v1 SET tests-------------------------------------\n"
        var = netsnmp.Varbind('sysLocation', '0', 'my new location')
        res = netsnmp.snmpset(var, **snmp_dest())

        print "v1 snmpset result: ", res, "\n"
        self.assertEqual(res, 1)

        print "v1 set var: ", var.tag, var.iid, "=", var.val, '(', var.type, ')'
        self.assertEqual(var.tag, 'sysLocation')
        self.assertEqual(var.iid, '0')
        self.assertEqual(var.val, 'my new location')
        self.assertTrue(var.type is None)

    def test_v1_walk(self):
        print "\n"
        print "---v1 walk tests-------------------------------------\n"
        varlist = netsnmp.VarList(netsnmp.Varbind('system'))

        print "v1 varlist walk in: "
        for var in varlist:
            print "  ", var.tag, var.iid, "=", var.val, '(', var.type, ')'

        res = netsnmp.snmpwalk(varlist, **snmp_dest())
        print "v1 snmpwalk result: ", res, "\n"
        self.assertTrue(len(res) > 0)

        for var in varlist:
            print var.tag, var.iid, "=", var.val, '(', var.type, ')'

    def test_v1_walk_2(self):
        print "\n"
        print "---v1 walk 2-------------------------------------\n"

        print "v1 varbind walk in: "
        var = netsnmp.Varbind('system')
        self.assertEqual(var.tag, 'system')
        self.assertEqual(var.iid, '')
        self.assertEqual(var.val, None)
        self.assertEqual(var.type, None)
        res = netsnmp.snmpwalk(var, **snmp_dest())
        print "v1 snmpwalk result (should be = orig): ", res, "\n"
        self.assertTrue(len(res) > 0)

        print var.tag, var.iid, "=", var.val, '(', var.type, ')'
        self.assertEqual(var.tag, 'system')
        self.assertEqual(var.iid, '')
        self.assertEqual(var.val, None)
        self.assertEqual(var.type, None)

    def test_v1_mv_get(self):
        print "\n"
        print "---v1 multi-varbind test-------------------------------------\n"
        sess = setup_v1()

        varlist = netsnmp.VarList(netsnmp.Varbind('sysUpTime', 0),
                                  netsnmp.Varbind('sysContact', 0),
                                  netsnmp.Varbind('sysLocation', 0))
        vals = sess.get(varlist)
        print "v1 sess.get result: ", vals, "\n"
        self.assertTrue(len(vals) > 0)

        for var in varlist:
            print var.tag, var.iid, "=", var.val, '(', var.type, ')'

        vals = sess.getnext(varlist)
        print "v1 sess.getnext result: ", vals, "\n"
        self.assertTrue(len(vals) > 0)

        for var in varlist:
            print var.tag, var.iid, "=", var.val, '(', var.type, ')'

        varlist = netsnmp.VarList(netsnmp.Varbind('sysUpTime'),
                                  netsnmp.Varbind('sysORLastChange'),
                                  netsnmp.Varbind('sysORID'),
                                  netsnmp.Varbind('sysORDescr'),
                                  netsnmp.Varbind('sysORUpTime'))

        vals = sess.getbulk(2, 8, varlist)
        print "v1 sess.getbulk result: ", vals, "\n"
        self.assertEqual(vals, None) # GetBulk is not supported for v1

        for var in varlist:
            print var.tag, var.iid, "=", var.val, '(', var.type, ')'

    def test_v1_set_2(self):
        print "\n"
        print "---v1 set2-------------------------------------\n"

        sess = setup_v1()
        varlist = netsnmp.VarList(
            netsnmp.Varbind('sysLocation', '0', 'my newer location'))
        res = sess.set(varlist)
        print "v1 sess.set result: ", res, "\n"

    def test_v1_walk_3(self):
        print "\n"
        print "---v1 walk3-------------------------------------\n"

        sess = setup_v1()
        varlist = netsnmp.VarList(netsnmp.Varbind('system'))

        vals = sess.walk(varlist)
        print "v1 sess.walk result: ", vals, "\n"
        self.assertTrue(len(vals) > 0)

        for var in varlist:
            print "  ", var.tag, var.iid, "=", var.val, '(', var.type, ')'

    def test_v2c_get(self):
        print "\n"
        print "---v2c get-------------------------------------\n"

        sess = setup_v2()
        varlist = netsnmp.VarList(netsnmp.Varbind('sysUpTime', 0),
                                  netsnmp.Varbind('sysContact', 0),
                                  netsnmp.Varbind('sysLocation', 0))
        vals = sess.get(varlist)
        print "v2 sess.get result: ", vals, "\n"
        self.assertEqual(len(vals), 3)

    def test_v2c_getnext(self):
        print "\n"
        print "---v2c getnext-------------------------------------\n"

        sess = setup_v2()
        varlist = netsnmp.VarList(netsnmp.Varbind('sysUpTime', 0),
                                  netsnmp.Varbind('sysContact', 0),
                                  netsnmp.Varbind('sysLocation', 0))
        for var in varlist:
            print var.tag, var.iid, "=", var.val, '(', var.type, ')'
        print "\n"

        vals = sess.getnext(varlist)
        print "v2 sess.getnext result: ", vals, "\n"
        self.assertTrue(len(vals) > 0)

        for var in varlist:
            print var.tag, var.iid, "=", var.val, '(', var.type, ')'
        print "\n"

    def test_v2c_getbulk(self):
        print "\n"
        print "---v2c getbulk-------------------------------------\n"

        sess = setup_v2()
        varlist = netsnmp.VarList(netsnmp.Varbind('sysUpTime'),
                                  netsnmp.Varbind('sysORLastChange'),
                                  netsnmp.Varbind('sysORID'),
                                  netsnmp.Varbind('sysORDescr'),
                                  netsnmp.Varbind('sysORUpTime'))

        vals = sess.getbulk(2, 8, varlist)
        print "v2 sess.getbulk result: ", vals, "\n"
        self.assertTrue(len(vals) > 0)

        for var in varlist:
            print var.tag, var.iid, "=", var.val, '(', var.type, ')'
        print "\n"

    def test_v2c_set(self):
        print "\n"
        print "---v2c set-------------------------------------\n"

        sess = setup_v2()

        varlist = netsnmp.VarList(
            netsnmp.Varbind('sysLocation', '0', 'my even newer location'))

        res = sess.set(varlist)
        print "v2 sess.set result: ", res, "\n"
        self.assertEqual(res, 1)

    def test_v2c_walk(self):
        print "\n"
        print "---v2c walk-------------------------------------\n"

        sess = setup_v2()

        varlist = netsnmp.VarList(netsnmp.Varbind('system'))

        vals = sess.walk(varlist)
        print "v2 sess.walk result: ", vals, "\n"
        self.assertTrue(len(vals) > 0)

        for var in varlist:
            print "  ", var.tag, var.iid, "=", var.val, '(', var.type, ')'

    def test_v3_get(self):
        print "\n"
        sess = setup_v3();
        varlist = netsnmp.VarList(netsnmp.Varbind('sysUpTime', 0),
                                  netsnmp.Varbind('sysContact', 0),
                                  netsnmp.Varbind('sysLocation', 0))
        print "---v3 get-------------------------------------\n"
        vals = sess.get(varlist)
        print "v3 sess.get result: ", vals, "\n"
        self.assertTrue(len(vals) > 0)

        for var in varlist:
            print var.tag, var.iid, "=", var.val, '(', var.type, ')'
        print "\n"

    def test_v3_getnext(self):
        print "\n"
        print "---v3 getnext-------------------------------------\n"

        sess = setup_v3();
        varlist = netsnmp.VarList(netsnmp.Varbind('sysUpTime', 0),
                                  netsnmp.Varbind('sysContact', 0),
                                  netsnmp.Varbind('sysLocation', 0))
        vals = sess.getnext(varlist)
        print "v3 sess.getnext result: ", vals, "\n"
        self.assertTrue(len(vals) > 0)

        for var in varlist:
            print var.tag, var.iid, "=", var.val, '(', var.type, ')'
        print "\n"

    def test_v3_getbulk(self):
        sess = setup_v3();
        varlist = netsnmp.VarList(netsnmp.Varbind('sysUpTime'),
                                  netsnmp.Varbind('sysORLastChange'),
                                  netsnmp.Varbind('sysORID'),
                                  netsnmp.Varbind('sysORDescr'),
                                  netsnmp.Varbind('sysORUpTime'))

        vals = sess.getbulk(2, 8, varlist)
        print "v3 sess.getbulk result: ", vals, "\n"
        self.assertTrue(len(vals) > 0)

        for var in varlist:
            print var.tag, var.iid, "=", var.val, '(', var.type, ')'
        print "\n"

    def test_v3_set(self):
        print "\n"
        print "---v3 set-------------------------------------\n"

        sess = setup_v3();
        varlist = netsnmp.VarList(
            netsnmp.Varbind('sysLocation', '0', 'my final destination'))
        res = sess.set(varlist)
        print "v3 sess.set result: ", res, "\n"
        self.assertEqual(res, 1)

    def test_v3_walk(self):
        print "\n"
        print "---v3 walk-------------------------------------\n"
        sess = setup_v3();
        varlist = netsnmp.VarList(netsnmp.Varbind('system'))

        vals = sess.walk(varlist)
        print "v3 sess.walk result: ", vals, "\n"
        self.assertTrue(len(vals) > 0)

        for var in varlist:
            print "  ", var.tag, var.iid, "=", var.val, '(', var.type, ')'


class SetTests(unittest.TestCase):
    """SNMP set tests for the Net-SNMP Python interface"""
    def testFuncs(self):
        """Test code"""
        print "\n-------------- SET Test Start ----------------------------\n"

        var = netsnmp.Varbind('sysUpTime', '0')
        res = netsnmp.snmpget(var, **snmp_dest())
        print "uptime = ", res[0]
        self.assertEqual(len(res), 1)


        var = netsnmp.Varbind('versionRestartAgent', '0', 1)
        res = netsnmp.snmpset(var, **snmp_dest())
        self.assertEqual(res, 1)

        var = netsnmp.Varbind('sysUpTime', '0')
        res = netsnmp.snmpget(var, **snmp_dest())
        print "uptime = ", res[0]
        self.assertEqual(len(res), 1)

        var = netsnmp.Varbind('nsCacheEntry')
        res = netsnmp.snmpgetnext(var, **snmp_dest())
        print "var = ", var.tag, var.iid, "=", var.val, '(', var.type, ')'
        self.assertEqual(len(res), 1)

        var.val = 65
        res = netsnmp.snmpset(var, **snmp_dest())
        self.assertEqual(res, 1)
        res = netsnmp.snmpget(var, **snmp_dest())
        print "var = ", var.tag, var.iid, "=", var.val, '(', var.type, ')'
        self.assertEqual(len(res), 1)
        self.assertEqual(res[0], '65');

        sess = setup_v1()

        varlist = netsnmp.VarList(
            netsnmp.Varbind('.1.3.6.1.6.3.12.1.2.1.2.116.101.115.116', '', '.1.3.6.1.6.1.1'),
            netsnmp.Varbind('.1.3.6.1.6.3.12.1.2.1.3.116.101.115.116', '', '1234'),
            netsnmp.Varbind('.1.3.6.1.6.3.12.1.2.1.9.116.101.115.116', '', 4))
        res = sess.set(varlist)

        print "res = ", res
        self.assertEqual(res, 1)

        varlist = netsnmp.VarList(netsnmp.Varbind('snmpTargetAddrTDomain'),
                                  netsnmp.Varbind('snmpTargetAddrTAddress'),
                                  netsnmp.Varbind('snmpTargetAddrRowStatus'))

        res = sess.getnext(varlist)
        self.assertEqual(len(res), 3)
        self.assertEqual(varlist[0].tag, 'snmpTargetAddrTDomain')
        self.assertEqual(varlist[0].iid, '116.101.115.116')
        self.assertEqual(varlist[0].val, '.1.3.6.1.6.1.1')
        self.assertEqual(varlist[1].tag, 'snmpTargetAddrTAddress')
        self.assertEqual(varlist[1].iid, '116.101.115.116')
        self.assertEqual(varlist[1].val, '1234')
        self.assertEqual(varlist[2].tag, 'snmpTargetAddrRowStatus')
        self.assertEqual(varlist[2].iid, '116.101.115.116')
        self.assertEqual(varlist[2].val, '3')

        for var in varlist:
            print var.tag, var.iid, "=", var.val, '(', var.type, ')'
        print "\n"

        varlist = netsnmp.VarList(
            netsnmp.Varbind('.1.3.6.1.6.3.12.1.2.1.9.116.101.115.116', '', 6))

        res = sess.set(varlist)

        print "res = ", res
        self.assertEqual(res, 1)

        varlist = netsnmp.VarList(netsnmp.Varbind('snmpTargetAddrTDomain'),
                                  netsnmp.Varbind('snmpTargetAddrTAddress'),
                                  netsnmp.Varbind('snmpTargetAddrRowStatus'))

        res = sess.getnext(varlist)
        self.assertEqual(len(res), 3)
        self.assertNotEqual(varlist[0].tag, 'snmpTargetAddrTDomain')
        self.assertNotEqual(varlist[1].tag, 'snmpTargetAddrTAddress')
        self.assertNotEqual(varlist[2].tag, 'snmpTargetAddrRowStatus')

        for var in varlist:
            print var.tag, var.iid, "=", var.val, '(', var.type, ')'
        print "\n"

        print "\n-------------- SET Test End ----------------------------\n"


if __name__ == '__main__':
    unittest.main()
