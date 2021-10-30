#!/bin/sh

# Prerouting
nft list chain ip nat MINIUPNPD
# Postrouting
nft list chain ip nat MINIUPNPD-POSTROUTING
# Filter
nft list chain inet filter MINIUPNPD
