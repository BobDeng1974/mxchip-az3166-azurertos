#ifndef WIFI_INIT_H
#define WIFI_INIT_H

#include <nx_api.h>
#include <nxd_dns.h>

/* Define the prototypes for ThreadX.  */
static NX_PACKET_POOL                   nx_pool[2]; /* 0=TX, 1=RX. */
static NX_IP                            ip_0;
static NX_DNS                           dns_client;

#endif /* WIFI_INIT_H */