#if defined(CONFIG_BCM_KF_XT_TARGET_DC)
/*
* published by the free software foundation
*/
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/netfilter/x_tables.h>
#include <net/netfilter/nf_conntrack.h>
#include <linux/string.h>
#include <net/sock.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/ctype.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION("ddc: dpi data collection module");

//#define DDC_DEBUG 
struct proc_dir_entry *proc_dpi_dir = NULL;
struct proc_dir_entry *proc_dpi_entry = NULL;
#define PROC_DPI_FILE_SZ        (1024)
#define DPI_DIRECTORY   "dpi"
#define DPI_PROC_FILE_NAME      "http"
#define DPI_INFO_BUF_LEN        (1024)
static char* pdpi_buf = NULL;
static unsigned int total_bytes = 0;

//#define round_up(x, y) (((x) + (y) - 1) & ~((y) - 1))

typedef enum
{
    DPI_PROTO_INVALID = 0,
    DPI_PROTO_HTTP,
    DPI_PROTO_FTP,
    DPI_PROTO_POP3,
    DPI_PROTO_TELNET,
    DPI_PROTO_MAX,
}DPI_PROTOCOL;

#define MAX_HOST_LENGTH 64
#define MAX_SEARCH_KEY_LEN  128
static char host[MAX_HOST_LENGTH]        = {0};
//static char referer[MAX_HOST_LENGTH]   = {0}; 
static char search_key[MAX_SEARCH_KEY_LEN] = {0};

//static char http_ver[16] ={0};
static char http_date[48] = {0};
static char http_charset[16] = {0};

#define PARSE_DATE      (1<<0)
#define PARSE_CHARSET      (1<<1)
#define DATE_VALID      (1 <<16)
#define CHARSET_VALID      (1 <<17)

static unsigned int  bparse_flag = 0;

typedef enum
{
    METHOD_GET,
    METHOD_POST,
    METHOD_HEAD,
    METHOD_PUT,
    METHOD_DELETE,
    METHOD_TRACE,
    METHOD_OPTIONS,
    METHOD_CONNECT,
    METHOD_PATCH,
    METHOD_MAX,    
}http_method;

struct dpi_packet_info
{
    char * ip_hdr;
    char * tcp_hdr;
    char * http_hdr;
    char * host;
    char * referer;
    char * key_end;     //pointer to the last character of searching key
    http_method method;
    int src_ptn_idx;
};

typedef enum
{
    SESSION_STATE_EMPTY,
    SESSION_STATE_NEW,
    SESSION_STATE_CONFIRMED,
    SESSION_STATE_SEARCHING,
    SESSION_STATE_MAX,
}session_state;

struct http_session
{
   session_state state;
   int              index;          //tell the updator the index of the session
   int              update_cnt;         //record how many http packet comes
   char *        keyword;
   //bool         record_key;
   int              keycnt;
   char         host_name[MAX_HOST_LENGTH];
   // add member later: mac, first access time, when to leave, and so on
};

struct srch_ptn
{
    int ptn_len;
    char * ptn;
};

static struct srch_ptn wld_ptn[]=   //for ns?, i? and v?
{
    {5, "word="},
    {0, NULL},
};

static struct srch_ptn search_ptn[]=     //search?
{
    {2, "q="},
    {5, "word="},
    {4, "key="},
    {0, NULL},
};

static struct srch_ptn sq[]=   //s?
{
    {3, "wd="},
    {0, NULL},
};

static struct srch_ptn kw[]=   //s?
{
    {3, "kw="},
    {0, NULL},
};


static struct srch_ptn q[]=   //?
{
    {2, "q="},
    {0, NULL},
};

struct search_mkptn
{
    int marklen;
    char* mark;
    struct srch_ptn *wdp;
};
/*
to make it simple, only 2. one is active page, the other is inactive
*/
#define SESSION_POOL_MAX        2  
#define UPDATE_CNT_THRESHOLD 4

static struct dpi_packet_info pkt_info;
//static struct http_session  session[SESSION_POOL_MAX];
//static int active = 0;
//static int last_update = 0;

const char* search_engine[] = 
{
    "www.baidu.com",
    "www.google.com",
    "www.google.com.hk",
    NULL,
};

//put in right sequence!
const struct search_mkptn search_pat[] =   //profiled search pattern: mark and pattern pairs
{   
    {7, "search?", search_ptn},
    {2, "s?", sq},
    {2, "f?", kw},
    {2, "v?", wld_ptn},
    {2, "i?", wld_ptn},
    {2, "ns?", wld_ptn},
    {3, "?q=", q},
    {5, "word?", wld_ptn},
};

#define SEARCH_PATTERN_NBR (sizeof(search_pat)/sizeof(struct search_mkptn))

typedef enum
{
    RECORD_PCONLINE = 1,
    RECORD_GATEWAY_STATE,
    RECORD_FLOW,
    RECORD_NETWORK_PERF,
    RECORD_URL,
    RECORD_MAIL,
    RECORD_APP,
    RECORD_MAX,
}record_type;

typedef enum
{   
    ACTION_WEB,
    ACTION_SEARCH,
    ACTION_MAX,
}action_type;

typedef enum
{
    CHARSET_INVALID,
    CHARSET_UTF_8,
    CHARSET_GBK,
    CHARSET_GB2312,
    //add other members
    CHARSET_MAX,
}charset;

struct host_key_record
{
    /*  PC online =1;
    *   gateway state
    *   flow
    *   network performance
    *   url
    *   mail
    *   app
    */
    unsigned short rcdtype;     
    unsigned short item_len;    //dont change the order of first two member       
    unsigned char actiontype;       /*web =1, search =2*/
    unsigned char url_len;
    unsigned char mac[6];
    char date[32];    
    unsigned char charset;
    unsigned char unused;
    unsigned short key_len;
    char data[];
};

struct host_key_record *phn_rcd = NULL;  /*pointer of host-name record*/

DEFINE_SPINLOCK(ddc_lock);


#if 1
static inline int isPrintable(char c)
{
    if((c>= 0x20) && (c<=0x7E))
        return 1;
     else
        return 0;
}

/*len = 16. for last line, len<16*/
static void printLine(const char * buf, unsigned int len)
{
    unsigned int i;
    unsigned char c;
    for(i=0; i<len; i++)
    {
        c = *(buf+i);
        printk("%02x ", c);
    }
    //allignment
    printk("    ");
    if(len <16)
    {
        for(i=0; i<16-len; i++)
            printk("   ");
    }
    for(i=0; i<len; i++)
    {
        c = *(buf+i);
        if(isPrintable(c))
            printk("%c", c);
        else
            printk(".");
    }
    printk("\n");
}

/*
*   Brief description: dump packet content to console
*   start: start address of the packet
*   end: end address of the packet
*/
void ddc_printPacket( char* start,  char* end)
{
    unsigned int  len;
    char *buf = start;
    printk("\n Total Len = %d\n", (unsigned int)(end-start));
    while(buf < end)
    {
        if(buf + 16 < end)
            len = 16;
        else
            len = (unsigned int)(end -buf);
        printLine(buf, len);
        buf+=len;
    }
}
#endif

static inline bool utl_in_range(const char* start, const char * end, const char * ptr)
{
    if(ptr >= start && ptr < end)
    {
        return true;
    }
    return false;    
}
static inline char* skipIpheader(char *ip_hdr)
{
    unsigned int iphdr_len;
    char * iphdr = ip_hdr;
    pkt_info.ip_hdr = ip_hdr;
    iphdr_len = ((*iphdr) & 0x0F)<<2;

    return iphdr+ iphdr_len;
}

static inline char* skipTcpheader(const char * tcp_hdr)
{
    unsigned int tcphdr_len;
    char *tcphdr = (char *)tcp_hdr;
    pkt_info.tcp_hdr = (char *)tcp_hdr;
    tcphdr_len =  ((*(tcphdr + 12))  & 0xF0) >> 2;

    return (tcphdr + tcphdr_len);
}

static inline bool pktdata_match(const char * src, const char * dst, int len)
{
    int i;
    for(i=0; i<len; i++)
    {
        if(src[i] != dst[i])
            return false;
    }
    return true;
}

static void extract_http_method(const char * http_hdr)
{
    if(http_hdr == NULL || *http_hdr == 0)
    {
        pkt_info.method = METHOD_MAX;
        return;
    }

    if(pktdata_match(http_hdr, "GET /", 5))
        pkt_info.method = METHOD_GET;
    else if(pktdata_match(http_hdr, "POST /", 6))
        pkt_info.method = METHOD_POST;
    #if 0    
    else if(pktdata_match(http_hdr, "HEAD /", 6))
        pkt_info.method = METHOD_HEAD;  
    else if(pktdata_match(http_hdr, "PUT /", 5))
        pkt_info.method = METHOD_PUT;    
    else if(pktdata_match(http_hdr, "DELETE /", 8))
        pkt_info.method = METHOD_POST;        
    else if(pktdata_match(http_hdr, "TRACE /", 7))
        pkt_info.method = METHOD_TRACE; 
    else if(pktdata_match(http_hdr, "OPTIONS /", 9))
        pkt_info.method = METHOD_OPTIONS;     
    else if(pktdata_match(http_hdr, "CONNECT /", 9))
        pkt_info.method = METHOD_CONNECT;  
    else if(pktdata_match(http_hdr, "PATCH /", 7))
        pkt_info.method = METHOD_PATCH;     
   #endif     
    else
        pkt_info.method = METHOD_MAX;
}
static void locateHTTPHeader(struct sk_buff * skb)
{
    char *proto_hdr = (char *)skb->data;
    #if 0
    if((*proto_hdr >>4) == 4)
    {
        printk("IPv4 packet\n");
    }
    #endif
    
    proto_hdr = skipIpheader(proto_hdr);
    proto_hdr = skipTcpheader(proto_hdr);  //later add code to check UDP header

    if(proto_hdr <=(char *)skb->tail)
    {
        pkt_info.http_hdr = proto_hdr;
    }
    else
    {
        pkt_info.http_hdr = NULL;   //just tcp ack packet
    } 
}

/*
    changed to char * left, char * right, char * keyword, char* start, int mode
    left              |          right
    right mode: start ->

*/
static char * find_keywords_r(const char* right, const char *keyword, char *start)
{
    char *  ptr = start;
    char * arch = NULL;
    int len = strlen(keyword);     

    while((ptr +len) <= (right +1))
    {
        if(*ptr == *keyword)
        {
            if(pktdata_match(ptr, keyword, len))
            {
                arch = ptr;
                return arch;
            }
        }
        ptr++;
    }

    return arch;
}

#if 0
/*search from right to the left
    caller should give left edge, keyword and start = right -len +1
*/
static char * find_keywords_l(const char* left, const char *keyword, char *start)
{
    char *  ptr = start;
    char * arch = NULL;
    int len = strlen(keyword);     

    while(ptr >=left)
    {
        if(*ptr == *keyword)
        {
            if(pktdata_match(ptr, keyword, len))
            {
                arch = ptr;
                return arch;
            }
        }
        ptr--;
    }

    return arch;
}

/*
*   only search/record user's searching keyword for some profiled search engine
*/
static bool match_search_engine(char* host_name)
{   
    int i = 0;

    //search profiled search engine
    const char * phost = search_engine[i];  
    while(phost !=NULL)
    {
        if(strcmp(phost, host_name) == 0)
            return true;
        phost = search_engine[++i];
    }

    //later add code to search configured search engine
    return false;
}
#endif

/*
*   extract host name from the HTTP GET packet and save this info into 
*   the array host[]. 
*/
static bool extract_http_host(const char * left, const char * right, char * start)
{
    int i =0;
    char * ptr = start;
    char * http_host = NULL;

    if(!utl_in_range(left, right, ptr))
        return false;

    http_host = find_keywords_r(right, "Host: ", ptr);
    if(http_host == NULL)   return false;

    /*
    now we dont consider the status of HTTP response. actually when the 
    return code is 301, which means the host has been moved permanently. 
    then we should extract the valid host name from "Location: xxxx\r\d"
    */
    http_host +=6;          //skip "Host: "
    pkt_info.host = http_host;
    while((http_host <=right) 
                && (*http_host != 0x0d) 
                && (*http_host != 0x0a))
    {
        host[i++] = *http_host;
        http_host++;
        if(i>=MAX_HOST_LENGTH) break;
    }
    host[i] = '\0';    
    return true;
}

static void update_search_key(struct sk_buff *skb, char * host_name, char * key)
{
    unsigned short key_len = round_up(strlen(key), 4);
    unsigned short url_len =  round_up(strlen(host_name), 4);
    unsigned short item_len = key_len + url_len +  round_up(sizeof(struct host_key_record), 4);
    
    phn_rcd = (struct host_key_record *)(pdpi_buf + total_bytes);
    
    if(!utl_in_range(pdpi_buf, pdpi_buf+DPI_INFO_BUF_LEN, (char*)phn_rcd + item_len))
    {
        //debug
        pr_debug("out of dpi buffer\n");
        return;
    }

    phn_rcd->rcdtype = RECORD_URL;
    phn_rcd->actiontype = 2; //search
    phn_rcd->url_len = url_len;
    phn_rcd->charset = CHARSET_INVALID;
    phn_rcd->key_len = key_len;
    phn_rcd->item_len =item_len;
    phn_rcd->date[0] = '\0';    //now date is invalid
    memcpy(phn_rcd->mac, skb_mac_header(skb)+6, 6);

    strcpy(phn_rcd->data, host_name);
    strcpy(phn_rcd->data + url_len, key);

    total_bytes += item_len;

    //debug
    //printk("total bytes: %d\n", total_bytes);
    //ddc_printPacket( (char*)pdpi_buf, (char*)pdpi_buf +  total_bytes);    
}

static void update_charset(char * charset)
{
    char * ptr = charset + 8;       //skip "charset="
    
    if(!utl_in_range(pdpi_buf, pdpi_buf+DPI_INFO_BUF_LEN, (char*)phn_rcd))
        return;
        
    if(strcmp(ptr, "utf-8") == 0)
        phn_rcd->charset = CHARSET_UTF_8;
    else if(strcmp(ptr, "gbk") == 0)
        phn_rcd->charset = CHARSET_GBK;
    else if(strcmp(ptr, "gb2312") == 0)
        phn_rcd->charset = CHARSET_GB2312;
    else
        phn_rcd->charset = CHARSET_INVALID;

    //debug
   //ddc_printPacket( (char*)pdpi_buf, (char*)pdpi_buf +  total_bytes);
}

/*total 29 bytes.*/
static void update_date(char * date)
{
    if(!utl_in_range(pdpi_buf, pdpi_buf+DPI_INFO_BUF_LEN, (char*)phn_rcd))
        return;
    strcpy(phn_rcd->date, date);   //have enough space

    //debug
    //ddc_printPacket( (char*)pdpi_buf, (char*)pdpi_buf +  total_bytes);
}
static void update_host_name(struct sk_buff *skb, char * host_name)
{
    unsigned short url_len, item_len;
    phn_rcd = (struct host_key_record *)(pdpi_buf + total_bytes);

    url_len = round_up(strlen(host_name), 4);
    item_len = url_len + round_up(sizeof(struct host_key_record), 4);    

    if(!utl_in_range(pdpi_buf, pdpi_buf+DPI_INFO_BUF_LEN, (char*)phn_rcd+item_len))
    {   
        pr_debug("out of dpi buffer\n");
        return;
    }
    
    phn_rcd->rcdtype = RECORD_URL;
    phn_rcd->actiontype = 1;    //web
    phn_rcd->url_len = url_len;
    phn_rcd->charset = CHARSET_INVALID;
    
    memcpy(phn_rcd->mac, skb_mac_header(skb)+6, 6);
    phn_rcd->date[0] = '\0';    //now date is invalid
    phn_rcd->item_len = item_len;
    
    strcpy(phn_rcd->data, host_name);
    total_bytes += phn_rcd->item_len ;
    phn_rcd->key_len = 0;
    
    //debug
    //ddc_printPacket( (char*)pdpi_buf, (char*)pdpi_buf +  total_bytes);
    return;
}

static bool check_search_pattern(const char * start)
{
    int i =0;
    char * ptr = (char *)start;
    struct search_mkptn * ptn = NULL;

    //skip the additional '/'
    while(i<10) //max we what skip. tunable
    {
        if(*ptr ==0x2F)  //'/'
        {
            ptr++;
            goto PATTERN_SEARCH;
        }
        i++;
        ptr++;            
    }
    ptr = (char *)start;        //reset

    /*later add code to read additional pattern configured through ctms
    */
PATTERN_SEARCH:  
    i=0;
    while(i<SEARCH_PATTERN_NBR)
    {
        ptn = (struct search_mkptn *)(&search_pat[i]);
        if(pktdata_match(ptr, ptn->mark, ptn->marklen))
        {
            pkt_info.src_ptn_idx = i;
            return true;
        }
        i++;
    };
    return false;
}

static bool extract_search_word(const char* start, const char* end)
{
    char * ptr = (char *)start;
    struct srch_ptn * ptrn = NULL;

    int i = 0;   
    
    if(pkt_info.src_ptn_idx < SEARCH_PATTERN_NBR)    
    {
        ptrn = search_pat[pkt_info.src_ptn_idx].wdp;
    }
    else
    {
        return false;
    }

    while(ptr <=(end-ptrn[i].ptn_len+1) && (*ptr!='\r') && (*ptr!='\n')&& (*ptr!=' '))
    {   
        while(ptrn[i].ptn != NULL)
        {
            if(pktdata_match(ptr, ptrn[i].ptn, ptrn[i].ptn_len))
            {
                ptr +=ptrn[i].ptn_len;
                goto CPY_SEARCH_KEY;
            }
            i++;
        }
        ptr++;
        i=0;
    }
    return false;
    
CPY_SEARCH_KEY:
    i =0;
    while((*ptr !='&') && (*ptr !=' ') && (i <(MAX_SEARCH_KEY_LEN-1)))
    {
        search_key[i++]  = *ptr;
        ptr++;
    }
    search_key[i] = '\0';
    pkt_info.key_end = ptr;
    return (i>0)? true : false;
}

static bool extract_http_date(const char * left, const char * right, const char * start)
{
    int i =0;
    char * ptr = (char *)start;

    while(ptr <=(right-3))
    {
        if(pktdata_match(ptr, "Date", 4))
        {
            goto CPY_DATE;
        }
        ptr++;
    }
    http_date[0] ='\0';
    return false;

CPY_DATE:
    while((*ptr!=0x0d) &&(*ptr!=0x0a))
    {
        http_date[i++] = *ptr++;
    }
    http_date[i] ='\0';
    return true;
}

static bool extract_http_charset(const char * left, const char * right, const char * start)
{
    int i =0;
    char * ptr = (char *)start;

    while(ptr <=(right-6))
    {
        if(pktdata_match(ptr, "charset", 7))
        {
            goto CPY_CHARSET;
        }
        ptr++;
    }
    http_charset[0] ='\0';
    return false;

CPY_CHARSET:
    while((*ptr!=0x0d) &&(*ptr!=0x0a))
    {
        http_charset[i++] = isascii(*ptr)? tolower(*ptr) : *ptr;
        ptr++;
    }
    http_charset[i] ='\0';
    return true;
}

static void http_handler_ds(struct sk_buff *skb)
{
    if(bparse_flag & (PARSE_DATE |PARSE_CHARSET))
    {
        locateHTTPHeader(skb);
    }
    
    if(bparse_flag & PARSE_DATE)
    {
        if(pkt_info.http_hdr && pktdata_match(pkt_info.http_hdr,"HTTP/",5))
        {
            if(extract_http_date(pkt_info.http_hdr, (const char *)skb->tail, pkt_info.http_hdr ))
            {
                bparse_flag &= ~PARSE_DATE;
                //printk("access time: %s\n", http_date+6); //just skip "Date: 
                update_date(http_date+6);
            }
        }
    }        

    /*
    * parese character set is just a demo
    */
    if(bparse_flag & PARSE_CHARSET)
    {
        if(pkt_info.http_hdr && pktdata_match(pkt_info.http_hdr,"HTTP/",5))     
        {
            if(extract_http_charset(pkt_info.http_hdr, (const char *)skb->tail, pkt_info.http_hdr + 7))
            {
                bparse_flag &= ~PARSE_CHARSET;
                bparse_flag |= CHARSET_VALID;
                //printk("%s\n", http_charset);
                update_charset(http_charset);
            }
        }
    }
}
static void http_handler_us(struct sk_buff *skb)
{
    char* ptr;
    bool spkt = false;
    locateHTTPHeader(skb);
    extract_http_method(pkt_info.http_hdr);
        
    if(pkt_info.method == METHOD_GET)
    {        
        if(pkt_info.http_hdr && pktdata_match(pkt_info.http_hdr + 6, "HTTP/", 5))
        {
            bparse_flag |= PARSE_DATE | PARSE_CHARSET;
            if(extract_http_host((char *)skb->data, (char *) skb->tail, pkt_info.http_hdr + 15))
            {      
                //printk("\n\nHost: %s\n", host);  //debug
                update_host_name(skb, host);
            }
        }    

        /*  now all web sites will be examined to extract searching key word. 
      *  later add code to extract key word only from specified web site based
      *  on ctms config
      */
        ptr = pkt_info.http_hdr + 5;      //skip "GET /"
        spkt = check_search_pattern(ptr);
        if(spkt)
        {
            if(extract_search_word(pkt_info.http_hdr + 5, (char *) skb->tail))  //skip space
            {
                extract_http_host((char *)skb->data, (char *) skb->tail, pkt_info.key_end ? pkt_info.key_end : (char *)skb->data);
                //printk("search key: %s\n", search_key);   //debug
                update_search_key(skb, host, search_key);
                bparse_flag |= PARSE_DATE;
            }
        }  
    }
    //add function to process other method, such as POST to parse webmail

}

static unsigned int
dc_tg(struct sk_buff *skb, const struct xt_action_param *par)
{       
    struct nf_conn * conntrack = NULL;
    enum ip_conntrack_info ctinfo;

   
    conntrack = nf_ct_get(skb, &ctinfo);
    if(unlikely(conntrack == NULL))
    {                
        return XT_CONTINUE;
    }
    
    memset(&pkt_info, 0, sizeof(pkt_info));
    
    spin_lock_bh(&ddc_lock); 

    if(((skb_dst(skb)->flags & DST_NOXFRM) && (skb->dev->priv_flags & IFF_WANDEV)) ||
       (!(skb_dst(skb)->flags & DST_NOXFRM) && (skb_dst(skb)->dev->priv_flags & IFF_WANDEV)))
    {
        /*NOTE: all http protocol pattern file should start with "http"
       now only handle http packet (only http rule is inserted)
    */
        if(pktdata_match(conntrack->layer7.app_proto, "http", 4))   
            http_handler_us(skb);
        //add other protocol handler
    }
    else
    {  
        if(pktdata_match(conntrack->layer7.app_proto, "http", 4))   
            http_handler_ds(skb);         
    }    
    spin_unlock_bh(&ddc_lock);    
    
    return XT_CONTINUE;
}

/*
 * internal DPI buffer is 1024 now(tunable). so the count should >=1024.
*/

static int dc_read_proc(struct file *f, char *buf, size_t cnt, loff_t *pos)
{
    int r = 0;
    
    //lock. need to define a lock        
    #if 1
    if(pdpi_buf == NULL)
        return -EFAULT;    

    r = simple_read_from_buffer(buf, cnt, pos, pdpi_buf, total_bytes);
    #else
    //here provide an example of how to parse data in /proc/dpi/http
    char * ptr = pdpi_buf;
    unsigned short items = 0;
    struct host_key_record * prcd;
    
    if(pdpi_buf == NULL)
        return 0;       
    printk("\n--dump contents in dpi buffer--\n");
    while((char *)ptr < (pdpi_buf + total_bytes))
    {
        prcd = (struct host_key_record *)ptr;
        if(prcd->rcdtype == 5)
            printk("RT: URL\n");
        else
            printk("RT: other\n");
        printk("MC: ");
        {
            int i;
            for(i=0; i<6; i++)
            {
                if(i!=5)
                    printk("%02x-", prcd->mac[i]);
                else
                    printk("%02x\n", prcd->mac[i]);
            }
        }
        printk("DT: %s\n", prcd->date);
        printk("HN: %s\n", prcd->data);
        if(prcd->actiontype ==2)
            printk("KY: %s\n", (char *)prcd->data + prcd->url_len);
        
        ptr += prcd->item_len;
        items++;
        printk("\n\n");
    }
    printk("\--total %d items--\n\n", items);
    #endif

    total_bytes = 0;    
    //unlock
    
    return r;
}

static struct file_operations dc_proc_fops = {
    .owner   = THIS_MODULE,
    .read    = dc_read_proc,
};

static void dc_init_proc(void)
{
    proc_dpi_dir = proc_mkdir(DPI_DIRECTORY, NULL);
    if(!proc_dpi_dir)
    {
        return;
    }
    
    proc_dpi_entry = proc_create(DPI_PROC_FILE_NAME, 0644, proc_dpi_dir, &dc_proc_fops);
    if(!proc_dpi_entry)
    {
        remove_proc_entry(DPI_DIRECTORY, NULL);
        return;      
    }
}

static void dc_cleanup_proc(void)
{
    if( proc_dpi_entry )
        remove_proc_entry(DPI_PROC_FILE_NAME, proc_dpi_dir);
    if( proc_dpi_dir )
        remove_proc_entry(DPI_DIRECTORY, NULL);
}


static struct xt_target dc_tg_reg  __read_mostly = 
{
    .name 		= "DC",
    .revision		= 0,
    .family		= NFPROTO_UNSPEC,
    .target		= dc_tg,
    .me		= THIS_MODULE,
};

static int __init dc_tg_init(void)
{    
    pdpi_buf = (char *)kmalloc(DPI_INFO_BUF_LEN, GFP_KERNEL);
    if(pdpi_buf == NULL)
    {
        pr_debug("%s: kmalloc %d bytes failed\n", __FUNCTION__, DPI_INFO_BUF_LEN);
        return -ENOMEM;
    }    
    
    dc_init_proc();
    return xt_register_target(&dc_tg_reg);
}

static void __exit dc_tg_exit(void)
{    
    if(pdpi_buf != NULL)
        kfree(pdpi_buf);
        
    dc_cleanup_proc();
    
    xt_unregister_target(&dc_tg_reg);
}

module_init(dc_tg_init);
module_exit(dc_tg_exit);
#endif
