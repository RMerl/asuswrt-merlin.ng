/*
 *  Template MIB group interface - var_route.h
 *
 */
#ifndef _MIBGROUP_VAR_ROUTE_H
#define _MIBGROUP_VAR_ROUTE_H

config_require(mibII/ip)
#ifdef solaris2
config_require(kernel_sunos5)
#endif

#if defined(HAVE_IPHLPAPI_H)
#include <iphlpapi.h>
     extern PMIB_IPFORWARDROW route_row;
#endif
     extern int      create_flag;

     void            init_var_route(void);

     extern FindVarMethod var_ipRouteEntry;

#if !defined(hpux11) && !defined(solaris2)
     RTENTRY **netsnmp_get_routes(size_t *out_numroutes);
#endif

#endif                          /* _MIBGROUP_VAR_ROUTE_H */
