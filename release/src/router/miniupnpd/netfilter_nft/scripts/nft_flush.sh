#!/bin/sh

nft flush chain ip nat MINIUPNPD
nft flush chain ip nat MINIUPNPD-POSTROUTING
nft flush chain inet filter MINIUPNPD
