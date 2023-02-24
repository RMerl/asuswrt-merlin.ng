/* auto gen  app protocols list definition */
#ifndef __APP_PROTOCOLS_LIST_H
#define __APP_PROTOCOLS_LIST_H

#define APP_NAME_LEN 128
#define PROTOCOL_FAMILIY_LEN 5

typedef struct protocol_name_s {
  int port;
  char protocol[PROTOCOL_FAMILIY_LEN + 1];
  char name[APP_NAME_LEN + 1];
} protocol_name_t;

protocol_name_t protocol_list [] = {
  { 20, "TCP", "FTP Data"},
  { 22, "TCP", "SSH Secure Shell"},
  { 23, "TCP", "Telnet"},
  { 43, "TCP", "Whois"},
  { 69, "UDP", "TFTP"},
  { 80, "TCP", "HTTP"},
  { 88, "TCP", "Kerberos"},
  { 110, "TCP", "POP Mail"},
  { 123, "UDP", "Network Time Protocol"},
  { 137, "UDP", "Windows Networking"},
  { 143, "TCP", "IMAP Mail"},
  { 177, "UDP", "X-Windows Display"},
  { 443, "TCP", "HTTPS"},
  { 445, "TCP", "Windows SMB"},
  { 502, "TCP", "Modbus"},
  { 546, "UDP", "DHCP IPv6"},
  { 548, "TCP", "Apple File Services"},
  { 554, "TCP", "Real Time Streaming Protocol"},
  { 587, "TCP", "Secure SMTP Mail"},
  { 873, "TCP", "Rsync"},
  { 903, "TCP", "VMware"},
  { 993, "TCP", "Secure IMAP Mail"},
  { 995, "TCP", "Secure POP Mail"},
  { 1080, "TCP", "SOCKS5"},
  { 1119, "TCP", "StarCraft"},
  { 1194, "UDP", "OpenVPN"},
  { 1352, "TCP", "Lotus Notes"},
  { 1433, "TCP", "Microsoft SQL Server"},
  { 1521, "TCP", "Oracle"},
  { 1720, "TCP", "H.323"},
  { 1723, "TCP", "PPTP"},
  { 1900, "UDP", "SSDP"},
  { 2000, "TCP", "Skinny"},
  { 2055, "UDP", "NetFlow"},
  { 2604, "TCP", "OSPF"},
  { 3306, "TCP", "MySQL"},
  { 3389, "TCP", "Remote Desktop Protocol"},
  { 3478, "UDP", "STUN"},
  { 3544, "UDP", "Teredo IPv6"},
  { 3868, "TCP", "Diameter"},
  { 4000, "TCP", "FIX"},
  { 4500, "UDP", "IPsec"},
  { 4506, "TCP", "ZeroMQ"},
  { 5222, "TCP", "WhatsApp Chat"},
  { 5353, "UDP", "Multicast DNS"},
  { 5355, "UDP", "RTP"},
  { 5632, "UDP", "pcAnywhere"},
  { 6000, "UDP", "EAQ"},
  { 6379, "TCP", "Redis"},
  { 7680, "TCP", "Pando Media Booster"},
  { 8009, "TCP", "Apache JServ"},
  { 8080, "TCP", "Ookla Speedtest"},
  { 8081, "TCP", "HTTP Web Proxy Connect"},
  { 8612, "UDP", "Canon BJNP"},
  { 9418, "TCP", "Git"},
  { 9982, "TCP", "Skype for Business"},
  { 10000, "TCP", "Cisco VPN"},
  { 10001, "UDP", "Ubiquiti AirControl 2"},
  { 10002, "UDP", "VHUA"},
  { 11095, "TCP", "Nest Protect"},
  { 11211, "TCP", "Memcached"},
  { 17500, "UDP", "Dropbox"},
  { 27005, "UDP", "csgo"},
};
#endif
