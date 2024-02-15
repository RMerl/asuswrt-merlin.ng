/* dnsmasq is Copyright (c) 2000-2024 Simon Kelley
   
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 dated June, 1991, or
   (at your option) version 3 dated 29 June, 2007.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/* If you modify this list, please keep the labels in metrics.c in sync. */
enum {
  METRIC_DNS_CACHE_INSERTED,
  METRIC_DNS_CACHE_LIVE_FREED,
  METRIC_DNS_QUERIES_FORWARDED,
  METRIC_DNS_AUTH_ANSWERED,
  METRIC_DNS_LOCAL_ANSWERED,
  METRIC_DNS_STALE_ANSWERED,
  METRIC_DNS_UNANSWERED_QUERY,
  METRIC_CRYPTO_HWM,
  METRIC_SIG_FAIL_HWM,
  METRIC_WORK_HWM,
  METRIC_BOOTP,
  METRIC_PXE,
  METRIC_DHCPACK,
  METRIC_DHCPDECLINE,
  METRIC_DHCPDISCOVER,
  METRIC_DHCPINFORM,
  METRIC_DHCPNAK,
  METRIC_DHCPOFFER,
  METRIC_DHCPRELEASE,
  METRIC_DHCPREQUEST,
  METRIC_NOANSWER,
  METRIC_LEASES_ALLOCATED_4,
  METRIC_LEASES_PRUNED_4,
  METRIC_LEASES_ALLOCATED_6,
  METRIC_LEASES_PRUNED_6,
  METRIC_TCP_CONNECTIONS,
  
  __METRIC_MAX,
};

const char* get_metric_name(int);
void clear_metrics(void);
