#if defined(CONFIG_BCM_KF_NETFILTER)
/*
<:copyright-BRCM:2011:DUAL/GPL:standard

   Copyright (c) 2011 Broadcom 
   All Rights Reserved

Unless you and Broadcom execute a separate written software license
agreement governing use of this software, this software is licensed
to you under the terms of the GNU General Public License version 2
(the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
with the following added to such license:

   As a special exception, the copyright holders of this software give
   you permission to link this software with independent modules, and
   to copy and distribute the resulting executable under terms of your
   choice, provided that you also meet, for each linked independent
   module, the terms and conditions of the license of that module.
   An independent module is a module which is not derived from this
   software.  The special exception does not apply to any modifications
   of the software.

Not withstanding the above, under no circumstances may you combine
this software in any way with any other Broadcom software provided
under a license other than the GPL, without Broadcom's express prior
written consent.

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

#define PT_PROTO_TCP    1
#define PT_PROTO_UDP    2
#define PT_PROTO_ALL    (PT_PROTO_TCP|PT_PROTO_UDP)

/* 
// add/del entry 
- add trigger port (tcp, 100, 200) open port (udp, 300, 400) on interface eth0.1
- add trigger port (udp, 6000, 6000) open port (both, 8000, 8001) on interface ppp0
# echo a 1 100 200 2 300 400 eth0.1 > /proc/net/nat_pt
# echo a 1 6000 6000 3 8000 8001 ppp0 > /proc/net/nat_pt

// Read entry
# cat /proc/net/nat_pt
idx: trigger port(protocol,start,end) : open port(protocol,start,end)  intf
  0:                1  100  200               2  300  400  eth0.1
  1:                1 6000 6000               3 8000 8001  ppp0
total_outport_cnt=102, total_inport_cnt=103, max_idx_used=2

// data structure
     outport={1,100,200,2,6000,6000, ...} 
     inport={2,300,400,3,8000,8001, ...}
     iface={eth0.1,ppp0, ...}
*/
static unsigned short outport[PT_MAX_ENTRIES*3];
static unsigned short inport[PT_MAX_ENTRIES*3];
static char *iface[PT_MAX_ENTRIES];

// sum of the outports of all configuration entries <= PT_MAX_PORTS
// sum of the inports of one configuration entry <= PT_MAX_EXPECTED
static int total_inport_cnt;
static int total_outport_cnt;
static int max_idx_used;    // max entry index is currently used

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
    for (i = 0; i < max_idx_used; i++) {
        /* Look for matched port range */
        if (!(oproto & outport[i*3]) || (oport < outport[i*3+1]) ||
            (oport > outport[i*3+2]))
                continue;

        /* If interface specified, they must match */
        if (iface[i] && strcmp(iface[i], skb_dst(skb)->dev->name))
            continue;
        trigger_ports(ct, dir, i);
    }

    return NF_ACCEPT;
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

static struct nf_conntrack_expect_policy pt_exp_policy = {
    .max_expected   = PT_MAX_EXPECTED,
    .timeout    = PT_TIMEOUT,
};


static struct nf_conntrack_helper *helper_ptr[PT_MAX_ENTRIES]; // each entry may have up to range*2(tcp/udp) helper structures
static int helper_cnt[PT_MAX_ENTRIES];
static int nat_pt_helper_register(int idx)
{
    struct nf_conntrack_helper *h;
    int port, helper_num=0;
    int ret=0;

    if(helper_cnt[idx] > 0) {
        printk("nf_nat_pt: entry %d is occupied\n", idx);
        return 1;
    }

    helper_cnt[idx] = 0;
    // calculate how many helper structures need for this entry
    for (port = outport[idx*3+1]; port <= outport[idx*3+2]; port++ ) {
        /* Don't register known ports */
        if (check_port(port, outport[idx*3])) {
           printk("nf_nat_pt: cannot register port %hu "
                  "(already registered by other module)\n",
                  port);
           continue;
        }
        if(outport[idx*3] == PT_PROTO_TCP || outport[idx*3] == PT_PROTO_UDP)
            helper_num++;
        else if (outport[idx*3] == PT_PROTO_ALL) {
            helper_num+=2;
        }
        else {
            printk("nf_nat_pt: unkown protocol !\n");
            return 1;
        }
    }

    if ((h = kzalloc(helper_num*sizeof(*h), GFP_KERNEL)) == NULL) {
        printk("nf_nat_pt: OOM\n");
        return -ENOMEM;
    }
    helper_ptr[idx] = h;
    helper_cnt[idx] = helper_num;

    for (port = outport[idx*3+1]; port <= outport[idx*3+2]; port++ ) {

        if (check_port(port, outport[idx*3])) 
           continue;
		   
        snprintf(h->name, NF_CT_HELPER_NAME_LEN, "pt-%d-%d", port, inport[idx*3+1]);
        h->me = THIS_MODULE;
        h->expect_policy = &pt_exp_policy;
        h->expect_class_max = 1;
        if (outport[idx*3] == PT_PROTO_TCP) {
            h->tuple.dst.protonum = IPPROTO_TCP;
        } else if (outport[idx*3] == PT_PROTO_UDP) {
            h->tuple.dst.protonum = IPPROTO_UDP;
        } else if (outport[idx*3] == PT_PROTO_ALL) {
            h->tuple.dst.protonum = IPPROTO_TCP;
            h->tuple.src.u.all = htons(port);
            h->tuple.src.l3num = AF_INET;
            h->help = help;

            if ((ret = nf_conntrack_helper_register(h)) < 0) {
                printk("nf_nat_pt: register helper error\n");
                return ret;
            }
            total_outport_cnt++;
            h++;

            snprintf(h->name, NF_CT_HELPER_NAME_LEN, "pt-%d-%d", port, inport[idx*3+1]);    
            h->me = THIS_MODULE;
            h->expect_policy = &pt_exp_policy;
            h->expect_class_max = 1;
            h->tuple.dst.protonum = IPPROTO_UDP;
        }
        h->tuple.src.u.all = htons(port);
        h->tuple.src.l3num = AF_INET;
        h->help = help;
        if ((ret = nf_conntrack_helper_register(h)) < 0) {
            pr_debug("nf_nat_pt: register helper error\n");
            return ret;
        }
        total_outport_cnt++;
        h++;
    }           
    return 0;
}

static int nat_pt_helper_unregister(int idx)
{
    int i, cnt;
    struct nf_conntrack_helper *h;

    if(helper_cnt[idx] == 0) {
        printk("nf_nat_pt: entry %d is not valid \n", idx);
        return 1;
    }

    cnt = helper_cnt[idx];
    for (i = 0, h = helper_ptr[idx] ; i < cnt; i++, h++) {
        if (hlist_unhashed(&h->hnode))  /* Not registered */
            continue;
        nf_conntrack_helper_unregister(h);
        total_outport_cnt--;
    }
    kfree(helper_ptr[idx]);
    helper_cnt[idx] = 0;
    return 0;
}

ssize_t nat_pt_read_proc(struct file *file, char __user *buf, size_t size, loff_t *offset)
//static int nat_pt_read_proc(char* page, char ** start, off_t off, int count, int* eof, void * data)
{
    int i;
    char *p, *np;

    if(*offset !=0)
    {
        return 0;
    }

    p = np = buf;
    np += sprintf(p, "idx: trigger port(protocol,start,end) : open port(protocol,start,end)        intf\n");
    p = np;
    for (i = 0; i < max_idx_used; i++) {
        if( (outport[i*3] & (PT_PROTO_TCP|PT_PROTO_UDP)) != 0)
            np += sprintf(p, "%3d:             %2d %6d %6d              %2d %6d %6d                %s\n", 
                 i, outport[i*3], outport[i*3+1], outport[i*3+2], inport[i*3], inport[i*3+1], inport[i*3+2], iface[i]);
        p = np;
    }
    np += sprintf(p, "total_outport_cnt=%d, total_inport_cnt=%d\n", total_outport_cnt, total_inport_cnt);
 
    return (np - buf);
}

static ssize_t nat_pt_write_proc(struct file *f, const char *buf, size_t cnt, loff_t *pos)
//static int nat_pt_write_proc(struct file* file, const char* buf, unsigned long cnt, void *data)
{
    char input[64], intf[16], op;
    int argc, idx;
    int trigger[3], open[3];

    if (copy_from_user(input, buf, cnt) != 0)
        return -EFAULT;

    argc = sscanf(input, "%c %d %d %d %d %d %d %s\n", &op, (int *)&trigger[0], (int *)&trigger[1], (int *)&trigger[2], (int *)&open[0], (int *)&open[1], (int *)&open[2], intf);
    if( argc < 7) {
        printk("nf_nat_pt: wrong number of argument !\n");
        printk("Format : echo op trigger_protocol trigger_start_port trigger_end_port open_protocol open_start_port open_end_port [interface_name] > /proc/net/nf_nat_pt\n");
        printk("         --op a(add), d(del)        --protocol TCP=1, UDP=2, Both=3\n");
        return cnt;
    }

    if((op != 'a') && (op != 'd')) {
        printk("nf_nat_pt: Unknown op=%c --op a(add) or d(del)\n", op);
        return cnt;
    }

    if((trigger[0] > PT_PROTO_ALL) || trigger[0] <= 0) {
        printk("nf_nat_pt: Unknown protocol : TCP=1, UDP=2, Both=3\n");
        return cnt;
    }

    if((trigger[1] > trigger[2]) || (open[1] > open[2]) ) {
        printk("nf_nat_pt: Error (end-start) < 0  \n");
        return cnt;
    }

    // add entry
    if(op == 'a') {
        for(idx = 0; idx < max_idx_used; idx++) {
            // find available entry index
            if(outport[idx*3] == 0) {
                break;
            }
        }
        
        if(idx >= PT_MAX_ENTRIES) {
            printk("nf_nat_pt: index = %d, should be 0~99 !\n", idx);
            return cnt;
        }

        // sum of the outports of all configuration entries <= PT_MAX_PORTS
        if ((total_outport_cnt + (trigger[2] - trigger[1] + 1)) > PT_MAX_PORTS) {
            printk("nf_nat_pt: outport range is greater than maximum number total %d. Total inport used = %d.\n", 
                    PT_MAX_PORTS, total_outport_cnt);
            return cnt;
        }

        // sum of the inports <= PT_MAX_EXPECTED
        if(total_inport_cnt + (open[2] - open[1] + 1) > PT_MAX_EXPECTED) 
        {
            printk("nf_nat_pt: inport range is greater than maximum number total %d. Total inport used = %d.\n", 
                    PT_MAX_EXPECTED, total_inport_cnt);
        return cnt;
        }
        total_inport_cnt += (open[2] - open[1] + 1);

        outport[idx*3] = trigger[0];
        outport[idx*3+1] = trigger[1];
        outport[idx*3+2] = trigger[2];
        inport[idx*3] = open[0];
        inport[idx*3+1] = open[1];
        inport[idx*3+2] = open[2];

        if(argc == 8) {
            if ((iface[idx] = kzalloc(sizeof(intf), GFP_KERNEL)) == NULL) {
                printk("nf_nat_pt: allocate ifname buf error\n");
                return -ENOMEM;
            }
            strcpy(iface[idx], intf);
        } else {
            iface[idx] = NULL;
        }

        nat_pt_helper_register(idx);

        if(idx == max_idx_used)
            max_idx_used++;

        pr_debug("nf_nat_pt: op=%c idx=%d trigger=%4d %4d %4d  open=%4d %4d %4d   %s\n", 
                    op, idx, trigger[0], trigger[1], trigger[2], open[0], open[1], open[2], iface[idx]);

    } else if(op =='d') {
        // remove matched entry
        for(idx=0; idx < max_idx_used; idx++) {

           if((trigger[0] == outport[idx*3]) && (trigger[1] == outport[idx*3+1]) && (trigger[2] == outport[idx*3+2]) &&
               (open[0] == inport[idx*3]) && (open[1] == inport[idx*3+1]) && (open[2] == inport[idx*3+2])) {

                pr_debug("nf_nat_pt: delete index=%d\n", idx);
                total_inport_cnt -= (inport[idx*3+2] - inport[idx*3+1] + 1);

                outport[idx*3] = 0; outport[idx*3+1] = 0; outport[idx*3+2] = 0;
                inport[idx*3] = 0;  inport[idx*3+1] = 0;  inport[idx*3+2] = 0;
                kfree(iface[idx]);
                iface[idx] = NULL;

                nat_pt_helper_unregister(idx);
                break;
            }
        }
        if(idx == max_idx_used) {
            printk("nf_nat_pt: delete fail - op=%c trigger=%d %d %d open=%d %d %d is not found\n", 
                    op, trigger[0], trigger[1], trigger[2], open[0], open[1], open[2]);
        }
        else if(idx == (max_idx_used-1)) {
            max_idx_used--; 
            while(max_idx_used > 0) {
                if(outport[(max_idx_used-1)*3] != 0)
                    break;
                max_idx_used--;
            }
        }
    }
    return cnt;
}

static struct file_operations proc_fops = {
    .owner      = THIS_MODULE,
    .read       = nat_pt_read_proc,
    .write      = nat_pt_write_proc,
};

/* register the proc file */
static void nat_pt_init_proc(void)
{
    struct proc_dir_entry *p;

    p = proc_create("nf_nat_pt", 0644, init_net.proc_net, &proc_fops);
    if (p == NULL)
        printk("nf_nat_pt: create proc - nf_nat_ptfail !\n");

    return;  
}
static void nat_pt_cleanup_proc(void)
{
    remove_proc_entry("nf_nat_pt", init_net.proc_net);
}


static void fini(void)
{
    int i;

    for(i=0; i < max_idx_used; i++) {
        nat_pt_helper_unregister(i);
    }
    nat_pt_cleanup_proc();
}

static int __init init(void)
{
    /* init proc file */
    nat_pt_init_proc();
    total_inport_cnt = 0;
    total_outport_cnt = 0;
    return 0;
}

MODULE_AUTHOR("Eddie Shi <eddieshi@broadcom.com>");
MODULE_DESCRIPTION("Netfilter Conntrack helper for PT");
MODULE_LICENSE("GPL");

module_init(init);
module_exit(fini);
#endif
