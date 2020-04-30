## asuswrt-merlin New Gen for K3(version 382.xx and higher)

This project forks form Asuswrt-Merlin, the goal is to fit for PHICOMM-K3.


## features

1.Based entirely on asuswrt-merlin.ng.

2.Fixed kernel boot & gpio for K3, no error in TTL.

3.Transplant CFE code from src-rt-7.x.main to K3, router will not crash.

4.Adding K3 screen control util, screen is full function now.

5.Some extra CN & TW translation.


## how to build

Added model RT-K3, use "make rt-k3" to compile and gen trx image.


## Credits

ASUS

[RMerl](https://github.com/RMerl/) for makeing up asuswrt-merlin.ng

Lostlonger for research transplant from merlin to K3

[MerlinRdev](https://github.com/MerlinRdev/) for sharing K3-merlin.ng source code

[Updateing](https://github.com/Updateing/) for making screen usable

