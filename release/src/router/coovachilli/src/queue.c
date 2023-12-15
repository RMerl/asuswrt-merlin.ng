/* -*- mode: c; c-basic-offset: 2 -*- */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <linux/types.h>
#include <linux/netfilter.h>
#include <libnetfilter_queue/libnetfilter_queue.h>

/* returns packet id */
static u_int32_t print_pkt (struct nfq_data *tb) {
  struct nfqnl_msg_packet_hdr *ph;
  u_int32_t mark,ifi; 
  int id = 0;
  char *data;
  int ret;
  
  ph = nfq_get_msg_packet_hdr(tb);
  if (ph){
    id = ntohl(ph->packet_id);
    printf("hw_protocol=0x%04x hook=%u id=%u ",
	   ntohs(ph->hw_protocol), ph->hook, id);
  }
  
  mark = nfq_get_nfmark(tb);
  if (mark)
    printf("mark=%u ", mark);
  
  ifi = nfq_get_indev(tb);
  if (ifi)
    printf("indev=%u ", ifi);
  
  ifi = nfq_get_outdev(tb);
  if (ifi)
    printf("outdev=%u ", ifi);
  
  ret = nfq_get_payload(tb, &data);
  if (ret >= 0)
    printf("payload_len=%d ", ret);
  
  fputc('\n', stdout);
  
  return id;
}


static int cb(struct nfq_q_handle *qh, struct nfgenmsg *nfmsg,
              struct nfq_data *nfa, void *data) {
  u_int32_t id = print_pkt(nfa);
  printf("entering callback\n");
  return nfq_set_verdict(qh, id, NF_ACCEPT, 0, NULL);
}

static  struct nfq_handle *h;
static  struct nfq_q_handle *qh;
/*static  struct nfnl_handle *nh;*/
static  int fd;

int q_setup() {
  printf("opening library handle\n");
  h = nfq_open();
  if (!h) {
    fprintf(stderr, "error during nfq_open()\n");
    exit(1);
  }
  
  printf("unbinding existing nf_queue handler for AF_INET (if any)\n");
  if (nfq_unbind_pf(h, AF_INET) < 0) {
    fprintf(stderr, "error during nfq_unbind_pf()\n");
    exit(1);
  }
  
  printf("binding nfnetlink_queue as nf_queue handler for AF_INET\n");
  if (nfq_bind_pf(h, AF_INET) < 0) {
    fprintf(stderr, "error during nfq_bind_pf()\n");
    exit(1);
  }
  
  printf("binding this socket to queue '0'\n");
  qh = nfq_create_queue(h,  0, &cb, NULL);
  if (!qh) {
    fprintf(stderr, "error during nfq_create_queue()\n");
    exit(1);
  }
  
  printf("setting copy_packet mode\n");
  if (nfq_set_mode(qh, NFQNL_COPY_PACKET, 0xffff) < 0) {
    fprintf(stderr, "can't set packet_copy mode\n");
    exit(1);
  }
  
  fd = nfq_fd(h);

  return 0;
}

int main(int argc, char **argv) {
  int rv;
  char buf[4096] __attribute__ ((aligned));

  q_setup();
  
  while ((rv = recv(fd, buf, sizeof(buf), 0)) && rv >= 0) {
    printf("pkt received\n");
    nfq_handle_packet(h, buf, rv);
  }
  
  printf("unbinding from queue 0\n");
  nfq_destroy_queue(qh);
  
#ifdef INSANE
        /* normally, applications SHOULD NOT issue this command, since
         * it detaches other programs/sockets from AF_INET, too ! */
  printf("unbinding from AF_INET\n");
  nfq_unbind_pf(h, AF_INET);
#endif
  
  printf("closing library handle\n");
  nfq_close(h);
  
  exit(0);
}
