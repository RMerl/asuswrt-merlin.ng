#ifndef _CIDR_H_

uint32_t ipv4_cidr2mask_host(uint8_t cidr);
uint32_t ipv4_cidr2mask_net(uint8_t cidr);
void ipv6_cidr2mask_host(uint8_t cidr, uint32_t *res);
void ipv6_cidr2mask_net(uint8_t cidr, uint32_t *res);
void ipv6_addr2addr_host(uint32_t *addr, uint32_t *res);

#endif
