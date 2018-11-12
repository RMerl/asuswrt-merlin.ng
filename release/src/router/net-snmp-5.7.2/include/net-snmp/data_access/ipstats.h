/*
 * ipstats data access header
 *
 * $Id$
 */
#ifndef NETSNMP_ACCESS_IPSTATS_H
#define NETSNMP_ACCESS_IPSTATS_H

# ifdef __cplusplus
extern          "C" {
#endif

#define IPSYSTEMSTATSTABLE_HCINRECEIVES       1
#define IPSYSTEMSTATSTABLE_HCINOCTETS         2
#define IPSYSTEMSTATSTABLE_INHDRERRORS        3
#define IPSYSTEMSTATSTABLE_HCINNOROUTES       4 
#define IPSYSTEMSTATSTABLE_INADDRERRORS       5
#define IPSYSTEMSTATSTABLE_INUNKNOWNPROTOS    6
#define IPSYSTEMSTATSTABLE_INTRUNCATEDPKTS    7
#define IPSYSTEMSTATSTABLE_HCINFORWDATAGRAMS  8 
#define IPSYSTEMSTATSTABLE_REASMREQDS         9
#define IPSYSTEMSTATSTABLE_REASMOKS           10
#define IPSYSTEMSTATSTABLE_REASMFAILS         11
#define IPSYSTEMSTATSTABLE_INDISCARDS         12
#define IPSYSTEMSTATSTABLE_HCINDELIVERS       13
#define IPSYSTEMSTATSTABLE_HCOUTREQUESTS      14
#define IPSYSTEMSTATSTABLE_HCOUTNOROUTES      15
#define IPSYSTEMSTATSTABLE_HCOUTFORWDATAGRAMS 16
#define IPSYSTEMSTATSTABLE_HCOUTDISCARDS      17
#define IPSYSTEMSTATSTABLE_HCOUTFRAGREQDS     18
#define IPSYSTEMSTATSTABLE_HCOUTFRAGOKS       19
#define IPSYSTEMSTATSTABLE_HCOUTFRAGFAILS     20
#define IPSYSTEMSTATSTABLE_HCOUTFRAGCREATES   21
#define IPSYSTEMSTATSTABLE_HCOUTTRANSMITS     22
#define IPSYSTEMSTATSTABLE_HCOUTOCTETS        23
#define IPSYSTEMSTATSTABLE_HCINMCASTPKTS      24
#define IPSYSTEMSTATSTABLE_HCINMCASTOCTETS    25
#define IPSYSTEMSTATSTABLE_HCOUTMCASTPKTS     26
#define IPSYSTEMSTATSTABLE_HCOUTMCASTOCTETS   27
#define IPSYSTEMSTATSTABLE_HCINBCASTPKTS      28
#define IPSYSTEMSTATSTABLE_HCOUTBCASTPKTS     29
#define IPSYSTEMSTATSTABLE_DISCONTINUITYTIME  30
#define IPSYSTEMSTATSTABLE_REFRESHRATE        31
    
#define IPSYSTEMSTATSTABLE_LAST IPSYSTEMSTATSTABLE_REFRESHRATE
    
/**---------------------------------------------------------------------*/
/*
 * structure definitions
 */

/*
 * netsnmp_ipstats_entry
 */
typedef struct netsnmp_ipstats_s {

   /* Columns of ipStatsTable. Some of them are HC for computation of the 
    * other columns, when underlying OS does not provide them.
    * Always fill at least 32 bits, the table is periodically polled -> 32 bit
    * overflow shall be detected and 64 bit value should be computed automatically. */
   struct counter64 HCInReceives;
   struct counter64 HCInOctets;
   u_long          InHdrErrors;
   struct counter64 HCInNoRoutes; 
   u_long          InAddrErrors;
   u_long          InUnknownProtos;
   u_long          InTruncatedPkts;
   
   /* optional, can be computed from HCInNoRoutes and HCOutForwDatagrams */
   struct counter64 HCInForwDatagrams; 
   
   u_long          ReasmReqds;
   u_long          ReasmOKs;
   u_long          ReasmFails;
   u_long          InDiscards;
   struct counter64 HCInDelivers;
   struct counter64 HCOutRequests;
   struct counter64 HCOutNoRoutes;
   struct counter64 HCOutForwDatagrams;
   struct counter64 HCOutDiscards;
   
   /* optional, can be computed from HCOutFragOKs + HCOutFragFails*/
   struct counter64 HCOutFragReqds;
   struct counter64 HCOutFragOKs;
   struct counter64 HCOutFragFails;
   struct counter64 HCOutFragCreates;
   
   /* optional, can be computed from 
    * HCOutRequests +HCOutForwDatagrams + HCOutFragCreates
    * - HCOutFragReqds - HCOutNoRoutes  - HCOutDiscards */
   struct counter64 HCOutTransmits;
   
   struct counter64 HCOutOctets;
   struct counter64 HCInMcastPkts;
   struct counter64 HCInMcastOctets;
   struct counter64 HCOutMcastPkts;
   struct counter64 HCOutMcastOctets;
   struct counter64 HCInBcastPkts;
   struct counter64 HCOutBcastPkts;

   /* Array of available columns.*/
   int             columnAvail[IPSYSTEMSTATSTABLE_LAST+1];
} netsnmp_ipstats;


# ifdef __cplusplus
}
#endif

#endif /* NETSNMP_ACCESS_IPSTATS_H */
