#if defined(CONFIG_BCM_KF_NETFILTER)
/*
<:copyright-BRCM:2011:DUAL/GPL:standard

   Copyright (c) 2011 Broadcom 
   All Rights Reserved

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License, version 2, as published by
the Free Software Foundation (the "GPL").

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.


A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.

:>
*/

/* PT for IP connection tracking. */
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/ip.h>
#include <linux/ctype.h>
#include <net/checksum.h>
#include <net/udp.h>
#include <net/tcp.h>

#include <net/netfilter/nf_conntrack_core.h>
#include <net/netfilter/nf_conntrack_helper.h>
#include <net/netfilter/nf_nat_helper.h>
#include <linux/netfilter/nf_conntrack_pt.h>
#if defined(CONFIG_BCM_KF_RUNNER)
#if defined(CONFIG_BCM_RDPA) || defined(CONFIG_BCM_RDPA_MODULE)
#include <net/bl_ops.h>
#endif /* CONFIG_BCM_RUNNER */
#endif /* CONFIG_BCM_KF_RUNNER */

#define PT_PROTO_TCP 	1
#define PT_PROTO_UDP 	2
#define PT_PROTO_ALL 	(PT_PROTO_TCP|PT_PROTO_UDP)

/*
** Parameters passed from insmod.
** outport[25]={proto,a1,a2,proto,b1,b2,proto,c1,c2.............}
** inport[25]={proton,a1,a2,proto,b1,b2,proto,c1,c2.............}
** iface[25]={interface1, interface2, interface3................}
** insmod pt.o outport=0,100,200,1,6000,7000 
**             inport=0,300,400,2,800,900
**             iface=eth0,ppp0
**	       entries=2
**	       timeout=180	
** where number tells us how many entries user entered
** where 1 means tcp
** where 2 means udp 
** where 0 means both
*/
static unsigned short outport[PT_MAX_ENTRIES*3];
static unsigned short inport[PT_MAX_ENTRIES*3];
static char *iface[PT_MAX_ENTRIES];
static int entries;
static unsigned timeout = PT_TIMEOUT;
static unsigned outport_c;
static unsigned inport_c;
static unsigned iface_c;
unsigned short invalid_config = 0;

module_param_array(outport, ushort, &outport_c, 0);
module_param_array(inport, ushort, &inport_c, 0);
module_param_array(iface, charp, &iface_c, 0);
module_param(entries, int, 0);
module_param(timeout, uint, 0);

static void trigger_ports(struct nf_conn *ct, int dir, int idx)
{
	__be16 port;
	unsigned short iport, iproto;
	struct nf_conntrack_expect *exp;
    	struct nf_conntrack_expect *exp2;

	/* Setup expectations */
	for (iport = inport[idx*3+1]; iport <= inport[idx*3+2]; iport++) {
		port = htons(iport);
		if ((exp = nf_ct_expect_alloc(ct)) == NULL) {
			pr_debug("nf_nat_pt: nf_ct_expect_alloc() error\n");
			return;
		}
		if (inport[idx*3] == PT_PROTO_TCP)
			iproto = IPPROTO_TCP;
		else if (inport[idx*3] == PT_PROTO_UDP)
			iproto = IPPROTO_UDP;
		else {
            		if ((exp2 = nf_ct_expect_alloc(ct)) == NULL) {
                		pr_debug("nf_nat_pt: "
					 "nf_ct_expect_alloc() error\n");
                		return;
            		}	
            		iproto = IPPROTO_TCP;
			nf_ct_expect_init(exp2, NF_CT_EXPECT_CLASS_DEFAULT,
					  AF_INET, NULL,
					  &ct->tuplehash[!dir].tuple.dst.u3,
					  iproto, NULL, &port);
            		exp2->expectfn = nf_nat_follow_master;
            		exp2->flags = NF_CT_EXPECT_PERMANENT;
            		exp2->saved_proto.all = port;
            		exp2->dir = !dir;
            		if(nf_ct_expect_related(exp2) == 0) {
                		pr_debug("nf_nat_pt: expect incoming "
					 "connection to %pI4:%hu %s\n",
					 &exp2->tuple.dst.u3.ip, iport,
                       	 		 iproto == IPPROTO_TCP? "tcp" : "udp");
            		} else {
                		pr_debug("nf_nat_pt: failed to expect incoming "
					 "connection to %pI4:%hu %s\n",
					 &exp2->tuple.dst.u3.ip, iport,
                       	 		 iproto == IPPROTO_TCP? "tcp" : "udp");
            		}
            		nf_ct_expect_put(exp2);
            
            		iproto = IPPROTO_UDP;
        	}

		nf_ct_expect_init(exp, NF_CT_EXPECT_CLASS_DEFAULT,
				  AF_INET, NULL,
				  &ct->tuplehash[!dir].tuple.dst.u3,
				  iproto, NULL, &port);
       		exp->expectfn = nf_nat_follow_master;
       		exp->flags = NF_CT_EXPECT_PERMANENT;
            	exp->saved_proto.all = port;
            	exp->dir = !dir;
		if(nf_ct_expect_related(exp) == 0) {
			pr_debug("nf_nat_pt: expect incoming connection to "
			       	 "%pI4:%hu %s\n", &exp->tuple.dst.u3.ip, iport,
			       	 iproto == IPPROTO_TCP? "tcp" : "udp");
		} else {
			pr_debug("nf_nat_pt: failed to expect incoming "
				 "connection to %pI4:%hu %s\n",
			       	 &exp->tuple.dst.u3.ip, iport,
			       	 iproto == IPPROTO_TCP? "tcp" : "udp");
		}
		nf_ct_expect_put(exp);

	}
#if defined(CONFIG_BCM_KF_RUNNER)
#if defined(CONFIG_BCM_RDPA) || defined(CONFIG_BCM_RDPA_MODULE)
#if defined(CONFIG_BCM_RUNNER_RG) || defined(CONFIG_BCM_RUNNER_RG_MODULE)
	BL_OPS(net_netfilter_xt_PORTTRIG_trigger_new (ct, ntohl(ct->tuplehash[!dir].tuple.src.u3.ip), ntohl(ct->tuplehash[!dir].tuple.dst.u3.ip),
		inport[idx*3+1], inport[idx*3+2], inport[idx*3]));
#endif /* CONFIG_BCM_RUNNER_RG || CONFIG_BCM_RUNNER_RG_MODULE */
#endif /* CONFIG_BCM_RUNNER */
#endif /* CONFIG_BCM_KF_RUNNER */
}

/* FIXME: This should be in userspace.  Later. */
static int help(struct sk_buff *skb, unsigned int protoff,
		struct nf_conn *ct, enum ip_conntrack_info ctinfo)
{
	int dir = CTINFO2DIR(ctinfo);
	unsigned short oport, oproto;
	int i;

	if ((nfct_help(ct))->expecting[NF_CT_EXPECT_CLASS_DEFAULT]) {
		/* Already triggered */
		return NF_ACCEPT;
	}

	/* We care only NATed outgoing packets */
	if (!(ct->status & IPS_SRC_NAT))
		return NF_ACCEPT;
	
	/* Get out protocol and port */
	if (nf_ct_protonum(ct) == IPPROTO_TCP) {
		/* Don't do anything until TCP connection is established */
		if (ctinfo != IP_CT_ESTABLISHED &&
		    ctinfo != IP_CT_ESTABLISHED + IP_CT_IS_REPLY)
		    	return NF_ACCEPT;
		oproto = PT_PROTO_TCP;
		oport = ntohs(ct->tuplehash[dir].tuple.dst.u.tcp.port);
	} else if(ct->tuplehash[dir].tuple.dst.protonum == IPPROTO_UDP) {
		oproto = PT_PROTO_UDP;
		oport = ntohs(ct->tuplehash[dir].tuple.dst.u.udp.port);
	} else /* Care only TCP and UDP */
		return NF_ACCEPT;
	
	for (i = 0; i < entries; i++) {
		/* Look for matched port range */
		if (!(oproto & outport[i*3]) || (oport < outport[i*3+1]) ||
		    (oport > outport[i*3+2]))
		    	continue;

		/* If interface specified, they must match */
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,2,0)
		if (iface[i] && strcmp(iface[i], skb->dst->dev->name))
#else
		if (iface[i] && strcmp(iface[i], skb_dst(skb)->dev->name))
#endif
			continue;

		trigger_ports(ct, dir, i);
	}

        return NF_ACCEPT;
}

static struct nf_conntrack_helper *pt;
static int valid_ports;

/* Not __exit: called from init() */
static void fini(void)
{
	int i;
	struct nf_conntrack_helper *h;

	for (i = 0, h = pt; i < valid_ports; i++, h++) {
		if (hlist_unhashed(&h->hnode))  /* Not registered */
			break;
		pr_debug("nf_nat_pt: unregister helper for port %hu\n",
		       	 ntohs(pt[i].tuple.src.u.all));
		nf_conntrack_helper_unregister(h);
	}
}

/*
** Becareful with ports that are registered already.
** ftp:21
** irc:6667
** tftp:69
** snmp: 161,162
** talk: 517,518
** h323: 1720
** sip: 5060
** pptp: 1723
** http: 80
*/
static int check_port(unsigned short port, unsigned short proto)
{
	if(proto & PT_PROTO_TCP) {
		if (port == 21 || port == 6667 || port == 1720 ||
		    port == 1723 || port == 80)
			return 1;
	}
	
	if(proto & PT_PROTO_UDP) {
		if (port == 69 || port == 161 || port == 162 || port == 517 ||
		    port == 518 || port == 5060)
			return 1;
	}

	return 0;
}

static int count_outport(void)
{
	int i;
	unsigned short port;

	for (i = valid_ports = 0; i < entries; i++) {
		for (port = outport[i*3+1]; port <= outport[i*3+2]; port++ ) {
			/* Don't register known ports */
			if (check_port(port, outport[i*3])) {
	    	    		printk("nf_nat_pt: cannot register port %hu "
				       "(already registered by other module)\n",
				       port);
				continue;
			}
            		if(outport[i*3] == PT_PROTO_TCP ||
			   outport[i*3] == PT_PROTO_UDP)
                		valid_ports++;
            		else
                		valid_ports+=2;
		}
	}

	if (valid_ports > PT_MAX_PORTS) {
		printk("nf_nat_pt: Conntrack port forwarding table is full. "
		       "Remaining entries are not processed.\n" );
		invalid_config = 1;
	}

	return valid_ports;
}

static int count_inport(void)
{
   	int i;

   	for( i=0; i<entries; i++) 
   	{
      		if( (inport[i*3+2] - inport[i*3+1] + 1) > PT_MAX_EXPECTED ) 
      		{
         		printk("nf_nat_pt: inport range is greater than "
			       "maximum number %d remaining ports are not "
			       "processed.\n", PT_MAX_EXPECTED);
			invalid_config = 1;
      		}
   	}

   	return 1;
}

static struct nf_conntrack_expect_policy pt_exp_policy = {
	.max_expected	= PT_MAX_EXPECTED,
	.timeout	= PT_TIMEOUT,
};

static int __init init(void)
{
	int i, ret=0;
	unsigned short port;
	struct nf_conntrack_helper *h;

	/* Validate parameters */
	if ((outport_c != inport_c) ||
	    (outport_c < entries * 3) ||
	    (inport_c < entries * 3)) {
	    	printk("nf_nat_pt: parameter numbers don't match\n");
		return -EINVAL;
	}

	/* Allocate memory for helpers */
	if (!count_outport()) {
		printk("nf_nat_pt: no ports specified\n");
		return -EINVAL;
	}

   	/* make sure inport range is less than or equal to PT_MAX_EXPECTED */
   	count_inport();

	if (invalid_config)
	{
		printk("nf_nat_pt: cannot port range larger than %d\n",
		       PT_MAX_PORTS);
		return -EINVAL;
	}

	if ((pt = kzalloc(valid_ports * sizeof(*h), GFP_KERNEL)) == NULL) {
		printk("nf_nat_pt: OOM\n");
		return -ENOMEM;
	}
	h = pt;

	pt_exp_policy.timeout = timeout;
	for (i = 0; i < entries; i++) {
		for (port = outport[i*3+1]; port <= outport[i*3+2]; port++ ) {
			/* Don't register known ports */
			if (check_port(port, outport[i*3]))
				continue;

            snprintf(h->name, NF_CT_HELPER_NAME_LEN, "pt-%d", port);
			h->me = THIS_MODULE;
			h->expect_policy = &pt_exp_policy;
			h->expect_class_max = 1;
			if (outport[i*3] == PT_PROTO_TCP) {
				h->tuple.dst.protonum = IPPROTO_TCP;
			} else if ( outport[i*3] == PT_PROTO_UDP) {
				h->tuple.dst.protonum = IPPROTO_UDP;
			} else {
				/* To keep backward compatibility, we still use
				 * 0 as all protocol for input parameters. Here
				 * we convert it to internal value */
				outport[i*3] = PT_PROTO_ALL;
                		h->tuple.dst.protonum = IPPROTO_TCP;
                		h->tuple.src.u.all = htons(port);
                		h->tuple.src.l3num = AF_INET;
                		h->help = help;
               	 		pr_debug("nf_nat_pt: register helper for "
					 "port %hu for incoming ports "
					 "%hu-%hu\n",
                       	 	 	 port, inport[i*3+1], inport[i*3+2]);
                		if ((ret = nf_conntrack_helper_register(h))
				    < 0) {
                    			printk("nf_nat_pt: register helper "
					       "error\n");
                    			fini();
                    			return ret;
                		}
                		h++;

                        snprintf(h->name, NF_CT_HELPER_NAME_LEN, "pt-%d", port);
                		h->me = THIS_MODULE;
				h->expect_policy = &pt_exp_policy;
				h->expect_class_max = 1;
                		h->tuple.dst.protonum = IPPROTO_UDP;
			}

			h->tuple.src.u.all = htons(port);
			h->tuple.src.l3num = AF_INET;
			h->help = help;

			pr_debug("nf_nat_pt: register helper for port %hu for "
			       	 "incoming ports %hu-%hu\n",
			       	 port, inport[i*3+1], inport[i*3+2]);

            		if ((ret = nf_conntrack_helper_register(h)) < 0) {
           	    		printk("nf_nat_pt: register helper error\n");
                		fini();
                		return ret;
			}
			h++;
		}
	}

	return 0;
}

MODULE_AUTHOR("Eddie Shi <eddieshi@broadcom.com>");
MODULE_DESCRIPTION("Netfilter Conntrack helper for PT");
MODULE_LICENSE("GPL");

module_init(init);
module_exit(fini);
#endif
