## asuswrt-merlin New Gen for K3(version 382.xx and higher)

This project forks form Asuswrt-Merlin, the goal is to fit for PHICOMM-K3.


## features

1.Based entirely on asuswrt-merlin.ng.

2.Fixed kernel boot & gpio for K3, no error in TTL.

3.Transplant CFE code from src-rt-7.x.main to K3, router will not crash.

4.Adding K3 screen control util, screen is full function now.

5.Some extra CN & TW translation.

## Toolchains to be prepared in advance

Asuswrt-Merlin toolchains - https://github.com/RMerl/am-toolchains

## how to build

You can modify cfe MAC address to your own in rt-k3_nvram.txt

(release/src-rt-7.14.114.x/src/cfe/build/broadcom/bcm947xx/compressed/rt-k3_nvram.txt)

New firmware flash will copy MAC from orignal CFE on the fisrt startup, it takes effect only when flash cfe separately.

where to modify:

(LAN MAC)	et0macaddr=00:11:22:33:44:55 -> et0macaddr=XX:XX:XX:XX:XX:XX

(2.4G MAC)	1:macaddr=00:11:22:33:44:66 -> 1:macaddr=XX:XX:XX:XX:XX:XX

(5G MAC)	2:macaddr=00:11:22:33:44:77 -> 2:macaddr=XX:XX:XX:XX:XX:XX

XX:XX:XX:XX:XX:XX means your own MAC address of LAN, 2.4G, 5G(They are generally different).

---------------

Added model RT-K3, use "make rt-k3" in src-rt-7.14.114.x/src to compile and gen trx image.

Use "make" to compile cfe separately in:

/release/src-rt-7.14.114.x/src/cfe/

or

/release/src-rt-7.14.114.x/src/cfe/build/broadcom/bcm947xx/

---------------

Output folder:

TRX image: release/src-rt-7.14.114.x/src/image/RT-K3_xxx.xx_x.trx

CFE image: release/src-rt-7.14.114.x/src/cfe/cfe_rt-k3.bin

(cfe_rt-k3.bin will be copied to /rom/cfe in TRX image if exsit)


## Additional pack(put into release/src/router/)

K3screenctrl - https://github.com/ghostnup/k3screenctrl


## Credits

ASUS

[RMerl](https://github.com/RMerl/) for makeing up asuswrt-merlin.ng

Lostlonger for research transplant from merlin to K3

[MerlinRdev](https://github.com/MerlinRdev/) for sharing K3-merlin.ng source code

[Updateing](https://github.com/Updateing/) for making screen usable

## 打赏我(buy me a beer)

![alipay](https://wx1.sbimg.cn/2020/05/28/alipay.jpg)
![wechat](https://wx1.sbimg.cn/2020/05/28/wechat.jpg)
