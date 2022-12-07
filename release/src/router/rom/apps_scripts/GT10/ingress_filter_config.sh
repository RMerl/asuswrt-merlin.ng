#!/bin/sh

bs /b/c port/index=wan0 ingress_filter[etype_pppoe_d]={enabled=yes,action=host}
bs /b/c port/index=wan0 ingress_filter[etype_pppoe_s]={enabled=yes,action=host}

bs /b/c port/index=wan0 ingress_filter[dhcp]={enabled=yes,action=host}
bs /b/c port/index=lan0 ingress_filter[dhcp]={enabled=yes,action=host}
bs /b/c port/index=lan1 ingress_filter[dhcp]={enabled=yes,action=host}
bs /b/c port/index=lan2 ingress_filter[dhcp]={enabled=yes,action=host}
bs /b/c port/index=wlan0 ingress_filter[dhcp]={enabled=yes,action=host}
bs /b/c port/index=wlan1 ingress_filter[dhcp]={enabled=yes,action=host}
bs /b/c port/index=wlan2 ingress_filter[dhcp]={enabled=yes,action=host}

bs /b/c port/index=wan0 ingress_filter[etype_arp]={enabled=yes,action=host}
bs /b/c port/index=lan0 ingress_filter[etype_arp]={enabled=yes,action=host}
bs /b/c port/index=lan1 ingress_filter[etype_arp]={enabled=yes,action=host}
bs /b/c port/index=lan2 ingress_filter[etype_arp]={enabled=yes,action=host}
bs /b/c port/index=wlan0 ingress_filter[etype_arp]={enabled=yes,action=host}
bs /b/c port/index=wlan1 ingress_filter[etype_arp]={enabled=yes,action=host}
bs /b/c port/index=wlan2 ingress_filter[etype_arp]={enabled=yes,action=host}

bs /b/c port/index=wan0 ingress_filter[ip_frag]={enabled=yes,action=host}
bs /b/c port/index=lan0 ingress_filter[ip_frag]={enabled=yes,action=host}
bs /b/c port/index=lan1 ingress_filter[ip_frag]={enabled=yes,action=host}
bs /b/c port/index=lan2 ingress_filter[ip_frag]={enabled=yes,action=host}
bs /b/c port/index=wlan0 ingress_filter[ip_frag]={enabled=yes,action=host}
bs /b/c port/index=wlan1 ingress_filter[ip_frag]={enabled=yes,action=host}
bs /b/c port/index=wlan2 ingress_filter[ip_frag]={enabled=yes,action=host}

bs /b/c port/index=wan0 ingress_filter[hdr_err]={enabled=yes,action=host}
bs /b/c port/index=lan0 ingress_filter[hdr_err]={enabled=yes,action=host}
bs /b/c port/index=lan1 ingress_filter[hdr_err]={enabled=yes,action=host}
bs /b/c port/index=lan2 ingress_filter[hdr_err]={enabled=yes,action=host}
bs /b/c port/index=wlan0 ingress_filter[hdr_err]={enabled=yes,action=host}
bs /b/c port/index=wlan1 ingress_filter[hdr_err]={enabled=yes,action=host}
bs /b/c port/index=wlan2 ingress_filter[hdr_err]={enabled=yes,action=host}

bs /b/c port/index=wan0 ingress_filter[bcast]={enabled=yes,action=host}
bs /b/c port/index=lan0 ingress_filter[bcast]={enabled=yes,action=host}
bs /b/c port/index=lan1 ingress_filter[bcast]={enabled=yes,action=host}
bs /b/c port/index=lan2 ingress_filter[bcast]={enabled=yes,action=host}
bs /b/c port/index=wlan0 ingress_filter[bcast]={enabled=yes,action=host}
bs /b/c port/index=wlan1 ingress_filter[bcast]={enabled=yes,action=host}
bs /b/c port/index=wlan2 ingress_filter[bcast]={enabled=yes,action=host}

bs /b/c port/index=lan0 ingress_filter[mcast]={enabled=yes,action=host}
bs /b/c port/index=lan1 ingress_filter[mcast]={enabled=yes,action=host}
bs /b/c port/index=lan2 ingress_filter[mcast]={enabled=yes,action=host}

bs /b/c port/index=lan0 ingress_filter[ip_mcast_control]={enabled=yes,action=host}
bs /b/c port/index=lan1 ingress_filter[ip_mcast_control]={enabled=yes,action=host}
bs /b/c port/index=lan2 ingress_filter[ip_mcast_control]={enabled=yes,action=host}

bs /b/c port/index=lan0 ingress_filter[igmp]={enabled=yes,action=host}
bs /b/c port/index=lan1 ingress_filter[igmp]={enabled=yes,action=host}
bs /b/c port/index=lan2 ingress_filter[igmp]={enabled=yes,action=host}
bs /b/c port/index=wlan0 ingress_filter[igmp]={enabled=yes,action=host}
bs /b/c port/index=wlan1 ingress_filter[igmp]={enabled=yes,action=host}
bs /b/c port/index=wlan2 ingress_filter[igmp]={enabled=yes,action=host}

bs /b/c port/index=lan0 ingress_filter[mld]={enabled=yes,action=host}
bs /b/c port/index=lan1 ingress_filter[mld]={enabled=yes,action=host}
bs /b/c port/index=lan2 ingress_filter[mld]={enabled=yes,action=host}
bs /b/c port/index=wlan0 ingress_filter[mld]={enabled=yes,action=host}
bs /b/c port/index=wlan1 ingress_filter[mld]={enabled=yes,action=host}
bs /b/c port/index=wlan2 ingress_filter[mld]={enabled=yes,action=host}

bs /b/c port/index=lan0 ingress_filter[etype_802_1ag_cfm]={enabled=yes,action=host}
bs /b/c port/index=lan1 ingress_filter[etype_802_1ag_cfm]={enabled=yes,action=host}
bs /b/c port/index=lan2 ingress_filter[etype_802_1ag_cfm]={enabled=yes,action=host}
bs /b/c port/index=wlan0 ingress_filter[etype_802_1ag_cfm]={enabled=yes,action=host}
bs /b/c port/index=wlan1 ingress_filter[etype_802_1ag_cfm]={enabled=yes,action=host}
bs /b/c port/index=wlan2 ingress_filter[etype_802_1ag_cfm]={enabled=yes,action=host}

bs /b/c port/index=lan0 ingress_filter[etype_udef_0]={enabled=yes,action=host}
bs /b/c port/index=lan1 ingress_filter[etype_udef_0]={enabled=yes,action=host}
bs /b/c port/index=lan2 ingress_filter[etype_udef_0]={enabled=yes,action=host}
bs /b/c port/index=wlan0 ingress_filter[etype_udef_0]={enabled=yes,action=host}
bs /b/c port/index=wlan1 ingress_filter[etype_udef_0]={enabled=yes,action=host}
bs /b/c port/index=wlan2 ingress_filter[etype_udef_0]={enabled=yes,action=host}
