#pragma once

#include "defines.h"

ssize_t inet_add_option(uint16_t eth_type, void *buf, size_t len,
                int proto, const void *optbuf, size_t optlen);
void    inet_checksum(uint16_t eth_type, void *buf, size_t len);

int raw_ip_opt_parse(int argc, char *argv[], uint8_t *type, uint8_t *len,
        uint8_t *buff, int buff_len);
int raw_ip6_opt_parse(int argc, char *argv[], uint8_t *proto, int *len,
        uint8_t *buff, int buff_len);
