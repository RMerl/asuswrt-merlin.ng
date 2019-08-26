#!/bin/sh

nft delete chain nat MINIUPNPD
nft delete chain nat MINIUPNPD-POSTROUTING
nft delete chain filter MINIUPNPD
