/*
 * The wlcsm kernel module
 *
 * Copyright (C) 2024, Broadcom. All Rights Reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * <<Broadcom-WL-IPTag/Open:>
 *
 * $Id: wlcsm_netlink.c 838017 2024-03-19 22:03:08Z $
 */

/**
 *  @file wlcsm_netlink.c
 *  @brief handling netlink socket packets
 *
 *  As the hooking point, user space APP and kernel space use netlink socket
 *  to communicate with each other.User APP sends/receives packets on netlink
 *  socket.
 *
 * */
#include <linux/socket.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/rtnetlink.h>
#include <linux/netlink.h>
#include <net/sock.h>
#include <linux/in.h>
#include <linux/if.h>
#include <linux/if_vlan.h>
#include <linux/timer.h>

#include "wlcsm_linux.h"
#include "wlcsm_nvram.h"
#include "bcmendian.h"

#ifdef WLCSM_DEBUG
#include <stdarg.h>
#endif // endif
static struct sock *nl_sk = NULL;
static PROCESS_REG_LIST g_process_list;
static unsigned  int g_mngr_pid=0;

#ifdef WLCSM_DEBUG
static void wlcsm_dump_buf(char *buf, int len)
{
    int i;
    printk("========================KRNEL BPEELA START===================================\n");
    for(i =0; i < len; i++) {
        printk("%02x", (unsigned char)buf[i]);
        if(!((i+1)%2))
            printk(" ");
        if(!((i+1)%16))
            printk("\n");
    }
    printk("\n");
    printk("=======================KRNL BPEELA END====================================\n");
}
#endif // endif
/******************************************************************//**
 *@brief sending message to App
 *
 * ###here is an example #
 *
 *~~~~{.c}
 *	wlcsm_sendup_response(msg_hdr->type,msg_hdr+1,msg_hdr->len,nlh->nlmsg_pid);
 *~~~~
 *
 * @param[in] type 	message type
 * @param[in] pData 	pointer to the real carrying data
 * @param[in] len  	len of carraying data.
 * @param[in] len  	len of carraying data.
 *
 **************************************************************************/
int WLCSM_NOINSTR_FUNC
wlcsm_sendup_response(t_WLCSM_MSGTYPES type,char *pData, int len,int topid,int frompid)
{
    struct nlmsghdr *nlh;
    char *ptr = NULL;
    struct sk_buff *new_skb;
    int buf_size;

    buf_size = sizeof(t_WLCSM_MSG_HDR) + len;
    /*  new_skb buffer=netlinkHeader +wlcsmmsgHeader+ pData */
    new_skb  = alloc_skb(NLMSG_SPACE(buf_size), GFP_ATOMIC);

    if(!new_skb) {
        printk("br_netlink_wlcsm.c:%d %s() errr no mem\n", __LINE__, __FUNCTION__);
        return 0;
    } else
        skb_put(new_skb, NLMSG_SPACE(buf_size));

    /* fill in header */
    nlh = (struct nlmsghdr *)new_skb->data;
    nlh->nlmsg_len = NLMSG_SPACE(buf_size);
    nlh->nlmsg_pid = frompid;
    nlh->nlmsg_flags = 0;

    /* fill in msg header */
    ptr = NLMSG_DATA(nlh);
    ((t_WLCSM_MSG_HDR *)ptr)->type = type;
    ((t_WLCSM_MSG_HDR *)ptr)->len = len;

    /* fill in data */
    ptr += sizeof(t_WLCSM_MSG_HDR);
    if(len)
        memcpy(ptr, pData, len);

    NETLINK_CB(new_skb).portid = topid;
    NETLINK_CB(new_skb).dst_group = (topid==0);

    if(topid==0)
        netlink_broadcast(nl_sk, new_skb,0 ,1, GFP_KERNEL);
    else
        netlink_unicast(nl_sk, new_skb, topid, MSG_DONTWAIT);

    return 1;
} /* wlcsm_sendup_notification */

int WLCSM_NOINSTR_FUNC
wlcsm_sendup_notification(t_WLCSM_MSGTYPES type,u8 *pData, int len,unsigned int frompid)
{
    return wlcsm_sendup_response(type,pData,len,0,frompid);
} /* wlcsm_sendup_notification */

#ifdef WLCSM_DEBUG
unsigned int g_debugdaemon_pid=0;
static WLCSM_INLINE int
isprint(unsigned char c)
{
    return (c>31 && c<127);
}

void  WLCSM_NOINSTR_FUNC
wlcsm_print(const char *fmt, ...)
{
    char msg[512]= {'\0'};
    va_list ap;
    int n=0;
    va_start(ap,fmt);
    n=vsnprintf(msg,512,fmt,ap);
    va_end(ap);
    if(n>0 && n<512) {
        if( g_debugdaemon_pid) {
            wlcsm_sendup_response(WLCSM_MSG_DEBUG_LOGMESSAGE,msg,n+1,g_debugdaemon_pid,0);
        } else
            printk("%s",msg);
    }
}

static void WLCSM_INLINE
inter_print(char *msg)
{
    if( g_debugdaemon_pid) {
        wlcsm_sendup_response(WLCSM_MSG_DEBUG_LOGMESSAGE,msg,strlen(msg)+1,g_debugdaemon_pid,0);
    } else
        printk("%s",msg);
    msg[0]='\0';
}

static void WLCSM_INLINE
print_banner(char *msg,unsigned char c, int length,int isHeader)
{
    int i=0;
    for (i=0 ; i<3; i++ )
        if(isHeader)
            msg[i]='[';
        else
            msg[i]=']';
    for (i=3 ; i<length; i++ )
        msg[i]=c;
    msg[length]='\n';
    msg[length+1]='\0';
    inter_print(msg);
}

void WLCSM_NOINSTR_FUNC
wlcsm_hexdump_ascii(const char *title,unsigned char c, const unsigned char *buf, unsigned int len)
{
    int i, llen;
    const unsigned char *pos = buf;
    const int line_len = 16;
    char msg[512]= {'\0'};
    sprintf(msg,"\n%s - (data len=%lu):\n", title, (unsigned long) len);
    inter_print(msg);
    print_banner(msg,c,20,1);

    while (len) {
        msg[0]='\0';
        llen = len > line_len ? line_len : len;
        sprintf(msg,"     ");
        for (i = 0; i < llen; i++)
            sprintf(msg+strlen(msg),"%02x ", pos[i]);
        for (i = llen; i < line_len; i++)
            sprintf(msg+strlen(msg),"   ");
        sprintf(msg+strlen(msg),"|   ");
        for (i = 0; i < llen; i++) {
            if (isprint(pos[i]))
                sprintf(msg+strlen(msg),"%c", pos[i]);
            else
                sprintf(msg+strlen(msg),"*");
        }
        for (i = llen; i < line_len; i++)
            sprintf(msg+strlen(msg)," ");

        sprintf(msg+strlen(msg),"\n");
        inter_print(msg);
        pos += llen;
        len -= llen;
    }
    print_banner(msg,c,30,0);
}

void wlcsm_dump_pkt(char *prompt,void* pktdata,int pktlen,unsigned int match_destlast4,unsigned int match_srclast4)
{
    if(g_WLCSM_TRACE_LEVEL & WLCSM_TRACE_PKT)  {
        int srcmatch=(match_srclast4==0||(match_srclast4 && ntoh32(*(unsigned int *)(pktdata+8))==match_srclast4));
        int dstmatch=(match_destlast4==0|| (match_destlast4 && ntoh32(*(unsigned int *)(pktdata+2))==match_destlast4));
        if(srcmatch && dstmatch)
            wlcsm_hexdump_ascii(prompt,'-',pktdata,pktlen);
    }
}

#endif // endif

static int			busy_pids = 0;
static struct mutex		busy_mutex;
static struct timer_list	busy_timer;
static void
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 15, 0))
busy_timer_task(struct timer_list *timer)
#else
busy_timer_task(unsigned long int data)
#endif /* LINUX_VERSION_CODE >= 4.15.0 */
{
	mutex_lock(&busy_mutex);
	printk("### %s: reset busy pid %d!\n", __FUNCTION__, busy_pids);
	busy_pids = 0;
	mutex_unlock(&busy_mutex);
}

#define NLS_XFR 1
#ifdef NLS_XFR

#ifdef CONFIG_NLS
#include <linux/nls.h>

static char *NLS_NVRAM_U2C = "asusnlsu2c";
static char *NLS_NVRAM_C2U = "asusnlsc2u";
__u16 unibuf[1024];
char codebuf[1024], tmpbuf[1024];
#endif

void
asusnls_u2c(char *buf)
{
#ifdef CONFIG_NLS
	char *codepage;
	char *xfrstr;
	struct nls_table *nls;
	int ret, len, i, charlen;

	strcpy(codebuf, buf);
	codepage = codebuf + strlen(NLS_NVRAM_U2C);
	if ((xfrstr = strchr(codepage, '_'))) {
		*xfrstr = '\0';
		xfrstr++;
		nls = load_nls(codepage);
		if (!nls) {
			printk("NLS table is null!\n");
		} else {
			len = 0;
			if ((ret = utf8s_to_utf16s(xfrstr, strlen(xfrstr), UTF16_HOST_ENDIAN, unibuf, 1024))) {
				for (i = 0; (i < ret) && unibuf[i]; i++) {
					charlen = nls->uni2char(unibuf[i], &buf[len], NLS_MAX_CHARSET_SIZE);
					if (charlen > 0) {
						len += charlen;
					} else {
						strcpy(buf, "");
						unload_nls(nls);
						return;
					}
				}
				buf[len] = 0;
			}
			unload_nls(nls);
			if (!len) {
				printk("cannot xfr from utf8 to %s\n", codepage);
				strcpy(buf, "");
			}
		}
	} else {
		strcpy(buf, "");
	}
#endif
}

void
asusnls_c2u(char *buf)
{
#ifdef CONFIG_NLS
	char *codepage;
	char *xfrstr;
	struct nls_table *nls;
	int len, i, charlen, ret;

	strcpy(codebuf, buf);
	codepage = codebuf + strlen(NLS_NVRAM_C2U);
	if ((xfrstr = strchr(codepage, '_'))) {
		*xfrstr = '\0';
		xfrstr++;

		strcpy(buf, "");
		nls = load_nls(codepage);
		if (!nls) {
			printk("NLS table is null!\n");
		} else {
			len = strlen(xfrstr);
			for (i = 0; len && *xfrstr; i++, xfrstr += charlen, len -= charlen) {   /* string to unicode */
				charlen = nls->char2uni(xfrstr, len, &unibuf[i]);
				if (charlen < 1) {
					strcpy(buf ,"");
					unload_nls(nls);
					return;
				}
			}
			unibuf[i] = 0;
			ret = utf16s_to_utf8s(unibuf, i, UTF16_HOST_ENDIAN, buf, 1024);  /* unicode to utf-8, 1024 is size of array unibuf */
			buf[ret] = 0;
			unload_nls(nls);
			if (!ret) {
				printk("cannot xfr from %s to utf8\n", codepage);
				strcpy(buf, "");
			}
		}
	} else {
		strcpy(buf, "");
	}
#endif
}

char *
nvram_xfr(const char *buf)
{
	strcpy(tmpbuf, buf);

	if (strncmp(tmpbuf, NLS_NVRAM_U2C, strlen(NLS_NVRAM_U2C)) == 0)
		asusnls_u2c(tmpbuf);
	else if (strncmp(buf, NLS_NVRAM_C2U, strlen(NLS_NVRAM_C2U)) == 0)
		asusnls_c2u(tmpbuf);
	else
		strcpy(tmpbuf, "");

	return tmpbuf;
}
#endif // NLS_XFR

#ifdef CONFIG_DUMP_PREV_OOPS_MSG
extern void dump_previous_oops(void);
#endif

static inline void WLCSM_NOINSTR_FUNC
wlcsm_nl_rcv_skb(struct sk_buff *skb)
{
    struct nlmsghdr *nlh = (struct nlmsghdr *)skb->data;
    int ret, tmp_pid;
    unsigned short type;
    char *data;
    t_WLCSM_MSG_HDR *msg_hdr;
    char *p = NULL;

    if (skb->len >= NLMSG_SPACE(0)) {
        if (nlh->nlmsg_len < sizeof(*nlh) || skb->len < nlh->nlmsg_len)
            return;

        msg_hdr = (t_WLCSM_MSG_HDR *)NLMSG_DATA(nlh);
	data = (char *)(msg_hdr + 1);

        switch(msg_hdr->type) {

        case WLCSM_MSG_GETWL_VAR: {
            if(g_mngr_pid)
                wlcsm_sendup_response(msg_hdr->type,(char *)(msg_hdr+1),msg_hdr->len,g_mngr_pid,nlh->nlmsg_pid);
            else
                wlcsm_sendup_response(msg_hdr->type,(char *)(msg_hdr+1),msg_hdr->len,nlh->nlmsg_pid,0);
            break;
        }
        case WLCSM_MSG_GETWL_VAR_RESP:
        case WLCSM_MSG_GETWL_VAR_RESP_DONE: {
            if(g_mngr_pid) {
                WLCSM_TRACE(WLCSM_TRACE_LOG," g_mngr_pid is:%d, to pid:%d,from pid:%d \r\n",g_mngr_pid,msg_hdr->pid,nlh->nlmsg_pid);
                wlcsm_sendup_response(msg_hdr->type,(char *)(msg_hdr+1),msg_hdr->len,msg_hdr->pid,nlh->nlmsg_pid);
            }
            break;
        }
        case WLCSM_MSG_SETWL_VAR: {
            if(g_mngr_pid)
                wlcsm_sendup_response(msg_hdr->type,(char *)(msg_hdr+1),msg_hdr->len,g_mngr_pid,nlh->nlmsg_pid);
            else
                wlcsm_sendup_response(msg_hdr->type,(char *)(msg_hdr+1),msg_hdr->len,nlh->nlmsg_pid,0);
            break;
        }

        case WLCSM_MSG_SETWL_VAR_RESP: {
            if(g_mngr_pid) {
                wlcsm_sendup_response(msg_hdr->type,(char *)(msg_hdr+1),msg_hdr->len,msg_hdr->pid,nlh->nlmsg_pid);
            }
            break;
        }

        case WLCSM_MSG_REGISTER: {
            t_WLCSM_MSG_REGISTER *reg=(t_WLCSM_MSG_REGISTER *)(msg_hdr+1);
            PROCESS_REG_LIST *process=kmalloc(sizeof(PROCESS_REG_LIST),GFP_KERNEL);
            if(process) {
                process->pid= reg->pid;
                memcpy(process->name,reg->process,reg->len);
                if(!strcmp(reg->process,"wlmngr"))  g_mngr_pid=reg->pid;
#ifdef WLCSM_DEBUG
                else if(!strcmp(reg->process,"wldebug")) {
                    printk("JXUJXU:%s:%d   set debug to %d \r\n",__FUNCTION__,__LINE__,reg->pid );
                    g_debugdaemon_pid=reg->pid;
                } else if(!strcmp(reg->process,"wlnodebug")) {
                    printk("JXUJXU:%s:%d   set debug to 0 \r\n",__FUNCTION__,__LINE__ );
                    g_debugdaemon_pid=0;
                }
#endif // endif
                list_add(&(process->list),&(g_process_list.list));
            }
            wlcsm_sendup_response(msg_hdr->type,(char *)(msg_hdr+1),msg_hdr->len,nlh->nlmsg_pid,0);
            break;
        }

#ifdef WLCSM_DEBUG
        case WLCSM_MSG_NVRAM_SETTRACE: {
            unsigned int level= *(unsigned int *)(msg_hdr+1);
            unsigned int action= (level>>30)&0x3;
            if(action==2)
                g_WLCSM_TRACE_LEVEL|=level;
            else if(action==1)
                g_WLCSM_TRACE_LEVEL &= (~level);
            else
                g_WLCSM_TRACE_LEVEL=level;
            printk("---:%s:%d  kernel DEBUG:%08x \r\n",__FUNCTION__,__LINE__,g_WLCSM_TRACE_LEVEL);
            wlcsm_sendup_response(msg_hdr->type,(char *)(msg_hdr+1),msg_hdr->len,nlh->nlmsg_pid,0);
        }
        break;

        /* userpace debug daemon which is repsonsible for forwarding message to network log */
        case WLCSM_MSG_DEBUGPID_REG: {
            g_debugdaemon_pid= *(unsigned int *)(msg_hdr+1);
            wlcsm_sendup_response(msg_hdr->type,(char *)(msg_hdr+1),msg_hdr->len,nlh->nlmsg_pid,0);
        }
        break;

        case WLCSM_MSG_DEBUG_LOGMESSAGE: {
            if(g_debugdaemon_pid) {
                wlcsm_sendup_response(msg_hdr->type,(char *)(msg_hdr+1),msg_hdr->len,g_debugdaemon_pid,0);
                wlcsm_sendup_response(msg_hdr->type,(char *)(msg_hdr+1),2,nlh->nlmsg_pid,0);
            } else
                wlcsm_sendup_response(msg_hdr->type,(char *)(msg_hdr+1),1,nlh->nlmsg_pid,0);
        }
        break;
#endif // endif
        case WLCSM_MSG_NVRAM_SET: {
		mutex_lock(&busy_mutex);
		tmp_pid = busy_pids;
		if (tmp_pid > 0) {
			mutex_unlock(&busy_mutex);
			wlcsm_sendup_response(WLCSM_MSG_NVRAM_GETALL_BUSY, (char *)&tmp_pid,
				sizeof(tmp_pid), nlh->nlmsg_pid, 0);
			break;
		}
		ret = wlcsm_nvram_set(data, msg_hdr->len);
		mutex_unlock(&busy_mutex);
		type = (ret == WLCSM_SUCCESS) ? msg_hdr->type : WLCSM_MSG_NVRAM_SET_ERR;
		wlcsm_sendup_response(type, data, msg_hdr->len, nlh->nlmsg_pid, 0);
		if (ret == WLCSM_SUCCESS)
			wlcsm_sendup_notification(type, data, msg_hdr->len, nlh->nlmsg_pid);
		break;
	}
        case WLCSM_MSG_NVRAM_UNSET: {
		mutex_lock(&busy_mutex);
		tmp_pid = busy_pids;
		if (tmp_pid > 0) {
			mutex_unlock(&busy_mutex);
			wlcsm_sendup_response(WLCSM_MSG_NVRAM_GETALL_BUSY, (char *)&tmp_pid,
				sizeof(tmp_pid), nlh->nlmsg_pid, 0);
			break;
		}
		wlcsm_nvram_unset((char *)(msg_hdr+1));
		mutex_unlock(&busy_mutex);
		wlcsm_sendup_response(msg_hdr->type,(char *)(msg_hdr+1),msg_hdr->len,nlh->nlmsg_pid,0);
		wlcsm_sendup_notification(msg_hdr->type,(char *)(msg_hdr+1),msg_hdr->len,nlh->nlmsg_pid);
		break;
	}
        case WLCSM_MSG_NVRAM_COMMIT:
            wlcsm_sendup_response(msg_hdr->type,(char *)(msg_hdr+1),msg_hdr->len,nlh->nlmsg_pid,0);
            wlcsm_sendup_notification(msg_hdr->type,(char *)(msg_hdr+1),msg_hdr->len,nlh->nlmsg_pid);
            break;
        case WLCSM_MSG_NVRAM_COMMIT_REQD: {
		int count = wlcsm_nvram_setcount_get();

		wlcsm_sendup_response(msg_hdr->type, (char *)&count, sizeof(count), nlh->nlmsg_pid, 0);
		break;
	}
        case WLCSM_MSG_NVRAM_COMMIT_REQD_CLEAR: {
		wlcsm_nvram_setcount_clear();
		break;
	}
        case WLCSM_MSG_NVRAM_GET: {
            char *buffer=NULL;
            int len;
            buffer=kmalloc(WLCSM_NAMEVALUEPAIR_MAX, GFP_KERNEL);
            if (buffer) {
               len=wlcsm_nvram_get((char *)(msg_hdr+1),buffer);
               if(len) {
                   wlcsm_sendup_response(msg_hdr->type,buffer,len,nlh->nlmsg_pid,0);
               } else
                   wlcsm_sendup_response(msg_hdr->type,(char *)(msg_hdr+1),0,nlh->nlmsg_pid,0);
               kfree(buffer);
            }
            else {
               WLCSM_TRACE(WLCSM_TRACE_ERR,"--:%s:%d kmalloc failed \r\n",__FUNCTION__,__LINE__ );
               wlcsm_sendup_response(WLCSM_MSG_NVRAM_GET_ERR, data, 0, nlh->nlmsg_pid, 0);
            }
        }
        break;
	case WLCSM_MSG_NVRAM_XFR:
		if ((p = nvram_xfr((char *)(msg_hdr+1)))) {
			strcpy(buffer, p);
			wlcsm_sendup_response(msg_hdr->type,buffer,strlen(buffer),nlh->nlmsg_pid,0);
		} else {
			wlcsm_sendup_response(msg_hdr->type,(char *)(msg_hdr+1),0,nlh->nlmsg_pid,0);
		}
	break;
#ifdef CONFIG_DUMP_PREV_OOPS_MSG
	case WLCSM_MSG_DUMP_PREV_OOPS:
		dump_previous_oops();
		wlcsm_sendup_response(msg_hdr->type,(char *)(msg_hdr+1),msg_hdr->len,nlh->nlmsg_pid,0);
		wlcsm_sendup_notification(msg_hdr->type,(char *)(msg_hdr+1),msg_hdr->len,nlh->nlmsg_pid);
	break;
#endif
        case WLCSM_MSG_NVRAM_GETALL: {
            int len = 0, retlen = 0;
            char *bigbuffer=NULL;

#ifdef OLD_GETALL
            sscanf((char *)(msg_hdr+1),"%d",&len);
#else
	    len = MAX_ALLOWED_SIZE + WLCSM_NAMEVALUEPAIR_MAX;
#endif /* OLD_GETALL */
            bigbuffer=kmalloc(len,GFP_KERNEL);
            if(bigbuffer) {
#ifdef OLD_GETALL
                char *tempptr = bigbuffer;

                WLCSM_TRACE(WLCSM_TRACE_LOG,"---:%s:%d  asking buffer len is :0x%x \r\n",__FUNCTION__,__LINE__,len );
                retlen = wlcsm_nvram_getall(bigbuffer, MAX_ALLOWED_SIZE, 0);
                len=retlen<len?retlen:len;
                WLCSM_TRACE(WLCSM_TRACE_LOG,"---:%s:%d  return buffer filled len is :0x%x  and len is:0x%x\n",__FUNCTION__,__LINE__,retlen,len );
                do {
                    if (len > MAX_ALLOWED_SIZE) {
                        wlcsm_sendup_response(msg_hdr->type, bigbuffer, MAX_ALLOWED_SIZE,
				nlh->nlmsg_pid, 0);
                        bigbuffer += MAX_ALLOWED_SIZE;

                    } else {
                        WLCSM_TRACE(WLCSM_TRACE_LOG,"---:%s:%d  send GETALL_DONE!!! \r\n",__FUNCTION__,__LINE__ );
                        wlcsm_sendup_response(WLCSM_MSG_NVRAM_GETALL_DONE,bigbuffer,len,nlh->nlmsg_pid,0);
                    }

                    len -= MAX_ALLOWED_SIZE;

                } while(len>0);

                kfree(tempptr);
#else
		/* XXX merged from wlcsm_nl_rcv_skb() from 504L02
		 * in bcmdrivers/broadcom/char/wlcsm_ext/impl1/src/wlcsm_netlink.c
		 * The pos is assigned as index in common_nvram_kernel_getall().
		 */
		int pos = ((int *)data)[1];

		mutex_lock(&busy_mutex);
		if (pos == 0) {
			if (busy_pids == 0) {
				busy_timer.expires = jiffies + msecs_to_jiffies(500);
				add_timer(&busy_timer);
			} else {
				mod_timer(&busy_timer, jiffies + msecs_to_jiffies(500));
				printk("%s: busy %d mod_timer 500ms for pid %d ###\n",
					__FUNCTION__, busy_pids, nlh->nlmsg_pid);
			}
			busy_pids++;
		}
		retlen = wlcsm_nvram_getall(bigbuffer, MAX_ALLOWED_SIZE, pos);
		if (retlen < MAX_ALLOWED_SIZE) {
			busy_pids--;
			if (busy_pids == 0) {
				del_timer_sync(&busy_timer);
			} else if (busy_pids < 0) {
				printk("%s: busy_pids %d!\n", __FUNCTION__, busy_pids);
			}
			if (retlen < 0) {
				type = WLCSM_MSG_NVRAM_GETALL_ERR;
				retlen = 0;
			} else
				type = WLCSM_MSG_NVRAM_GETALL_DONE;
		} else {
			type = msg_hdr->type;
			if (retlen > MAX_ALLOWED_SIZE) {
				printk("%s: retlen %d > %zu!\n", __FUNCTION__,
					retlen, MAX_ALLOWED_SIZE);
				retlen = MAX_ALLOWED_SIZE;
			}
		}
		mutex_unlock(&busy_mutex);
		wlcsm_sendup_response(type, bigbuffer, retlen, nlh->nlmsg_pid, 0);
		kfree(bigbuffer);
#endif /* OLD_GETALL */
            } else
		wlcsm_sendup_response(WLCSM_MSG_NVRAM_GETALL_ERR, data, 0, nlh->nlmsg_pid, 0);
        }
        break;
#ifndef OLD_GETALL
        case WLCSM_MSG_NVRAM_GETALL_DONE: {
		mutex_lock(&busy_mutex);
		printk("%s: dec busy_pids %d for pid %d!\n", __FUNCTION__,
			busy_pids, nlh->nlmsg_pid);
		if (busy_pids > 0)
			busy_pids--;
		if (busy_pids == 0)
			del_timer_sync(&busy_timer);
		mutex_unlock(&busy_mutex);
		break;
	}
#endif /* OLD_GETALL */
        default:
            WLCSM_TRACE(WLCSM_TRACE_ERR,"--:%s:%d  UNKNOWN COMMAND \r\n",__FUNCTION__,__LINE__ );
            break;
        }
    }
    return;
} /* wlcsm_nl_rcv_skb */

int
wlcsm_module_init(void)
{
	int ret;
	struct socket *sock;

	/* first to try to detect if netlink socket already exists as
	 * now we probably have two wireless drivers,one is wl and the
	 * other is DHD
	 */
	INIT_LIST_HEAD(&(g_process_list.list));
	ret = sock_create(AF_NETLINK, SOCK_RAW, NETLINK_WLCSM, &sock);
	if (ret < 0) {
		struct netlink_kernel_cfg cfg = {
			.input = wlcsm_nl_rcv_skb,
		};

		printk(KERN_INFO "Initializing WLCSM Module\n");
		nl_sk = netlink_kernel_create(&init_net, NETLINK_WLCSM, &cfg);
		if (nl_sk == NULL) {
			printk("WLCSM: failure to create kernel netlink socket\n");
			return -ENOMEM;
		}
	} else {
		sock_release(sock);
		printk(KERN_INFO "WLCSM Module has already been loaded\n");
		return 0;
	}
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 15, 0)
	timer_setup(&busy_timer, &busy_timer_task, 0);
#else
	init_timer(&busy_timer);
	busy_timer.data = (unsigned long)NULL;
	busy_timer.function = &busy_timer_task;
#endif /* LINUX_VERSION_CODE */
	mutex_init(&busy_mutex);
	printk(KERN_INFO "WLCSM Module loaded successfully\n");
	return 0;
} /* wlcsm_module_init */

void wlcsm_module_exit(void)
{

    if(nl_sk) {
        sock_release(nl_sk->sk_socket);
        WLCSM_TRACE(WLCSM_TRACE_LOG,"---:%s:%d remove WLCSM  \r\n",__FUNCTION__,__LINE__ );
    }
    del_timer(&busy_timer);

} /* wlcsm_module_exit */

void WLCSM_NOINSTR_FUNC
__cyg_profile_func_enter(void *this_fn, void *call_site)
{
    WLCSM_TRACE(WLCSM_TRACE_FUNC,"  enter:%p	from:%p\r\n",this_fn,call_site );
}

void WLCSM_NOINSTR_FUNC
__cyg_profile_func_exit(void *this_fn, void *call_site)
{
    WLCSM_TRACE(WLCSM_TRACE_FUNC,"  exit:%p	from:%p\r\n",this_fn,call_site );
}

EXPORT_SYMBOL(wlcsm_nvram_k_set);
EXPORT_SYMBOL(wlcsm_nvram_k_get);
EXPORT_SYMBOL(wlcsm_nvram_getall);

#ifdef WLCSM_DEBUG
EXPORT_SYMBOL(wlcsm_print);
EXPORT_SYMBOL(__cyg_profile_func_exit);
EXPORT_SYMBOL(__cyg_profile_func_enter);
EXPORT_SYMBOL(wlcsm_dump_pkt);
EXPORT_SYMBOL(wlcsm_hexdump_ascii);
EXPORT_SYMBOL(g_WLCSM_TRACE_LEVEL);
#endif // endif
