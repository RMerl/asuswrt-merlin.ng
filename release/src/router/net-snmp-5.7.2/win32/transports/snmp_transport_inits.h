#ifdef NETSNMP_TRANSPORT_UDP_DOMAIN
netsnmp_udp_base_ctor();
netsnmp_udp_ctor();
#endif
#ifdef NETSNMP_TRANSPORT_TCP_DOMAIN
netsnmp_tcp_ctor();
#endif
#ifdef NETSNMP_TRANSPORT_ALIAS_DOMAIN
netsnmp_alias_ctor();
#endif
#ifdef NETSNMP_TRANSPORT_UDPIPV6_DOMAIN
netsnmp_udpipv6_ctor();
#endif
#ifdef NETSNMP_TRANSPORT_TCPIPV6_DOMAIN
netsnmp_tcpipv6_ctor();
#endif
#ifdef NETSNMP_TRANSPORT_TLSBASE_DOMAIN
netsnmp_tlsbase_ctor();
#endif
#ifdef NETSNMP_TRANSPORT_DTLSUDP_DOMAIN
netsnmp_dtlsudp_ctor();
#endif
#ifdef NETSNMP_TRANSPORT_TLSTCP_DOMAIN
netsnmp_tlstcp_ctor();
#endif
#ifdef NETSNMP_TRANSPORT_STD_DOMAIN
netsnmp_std_ctor();
#endif
