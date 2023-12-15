/* auto gen  app protocols list definition */
#ifndef __APP_PROTOCOLS_LIST_H
#define __APP_PROTOCOLS_LIST_H

#define APP_NAME_LEN 128
#define PROTOCOL_FAMILIY_LEN 5

typedef struct protocol_name_s {
  int id;
  int port;
  char protocol[PROTOCOL_FAMILIY_LEN + 1];
  char name[APP_NAME_LEN + 1];
  int map_cat_id;
  int map_app_id;
} protocol_name_t;

protocol_name_t protocol_list [] = {
  { 50000, 21, "TCP", "FTP", 0, 0},
  { 10021, 22, "TCP", "SSH Secure Shell", 10, 21},
  { 10018, 23, "TCP", "Telnet", 10, 18},
  { 50003, 69, "UDP", "TFTP", 0, 0},
  { 13222, 80, "TCP", "HTTP", 13, 222},
  { 50005, 110, "TCP", "POP Mail", 0, 0},
  { 50006, 111, "UDP", "NFS", 0, 0},
  { 9002, 123, "UDP", "Network Time Protocol", 9, 2},
  { 50008, 137, "UDP", "Windows Networking", 0, 0},
  { 50009, 143, "TCP", "IMAP Mail", 0, 0},
  { 20185, 443, "TCP", "TLS/SSL", 20, 185},
  { 9110, 445, "TCP", "Windows SMB", 9, 110},
  { 50012, 500, "UDP", "IPsec", 0, 0},
  { 50013, 554, "UDP", "Real Time Streaming Protocol", 0, 0},
  { 50014, 1433, "TCP", "Microsoft SQL Server", 0, 0},
  { 50015, 1521, "TCP", "Oracle", 0, 0},
  { 50016, 1720, "TCP", "H.323", 0, 0},
  { 50017, 1883, "TCP", "MQTT", 0, 0},
  { 50018, 3306, "TCP", "MySQL", 0, 0},
  { 50019, 3389, "TCP", "Remote Desktop Protocol", 0, 0},
  { 20172, 3478, "UDP", "STUN", 20, 172},
  { 50021, 4500, "UDP", "IPsec", 0, 0},
  { 50022, 5355, "UDP", "RTP", 0, 0},
  { 50023, 6379, "TCP", "Redis", 0, 0},
  { 50024, 8080, "TCP", "Web Application", 0, 0},
  { 50025, 8443, "TCP", "HTTPS", 0, 0},
  { 50026, 9418, "TCP", "Git", 0, 0},
  { 50027, 17500, "UDP", "Dropbox", 0, 0},
};
#endif
