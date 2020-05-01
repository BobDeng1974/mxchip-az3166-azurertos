// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

// Copyright (c) Express Logic.  All rights reserved.
// Please contact support@expresslogic.com for any questions or use the support portal at www.rtos.com


#include <tx_api.h>
#include <nx_api.h>
#include <nxd_dns.h>
#include <nx_secure_tls_api.h>
#include <wwd_network_constants.h>
#include <optimize_size.h>

#ifndef THREADX_SNTP_DISABLE
#include <nxd_sntp_client.h>
#endif /* THREADX_SNTP_DISABLE */

// extern void threadx_azure_sdk_initialize(void);
// extern void threadx_azure_sdk_deinitialize(void);
extern void (*platform_driver_get())(NX_IP_DRIVER *);
extern UINT wifi_network_join(void *pools);

/* Define the default thread priority, stack size, etc. The user can override this 
   via -D command line option or via project settings.  */

#ifndef THREADX_IP_STACK_SIZE
#define THREADX_IP_STACK_SIZE           (2048)
#endif /* THREADX_IP_STACK_SIZE  */

#ifndef THREADX_TX_PACKET_COUNT
#define THREADX_TX_PACKET_COUNT         (32)
#endif /* THREADX_TX_PACKET_COUNT  */

#ifndef THREADX_RX_PACKET_COUNT
#define THREADX_RX_PACKET_COUNT         (16)
#endif /* THREADX_RX_PACKET_COUNT  */

#ifndef THREADX_PACKET_SIZE
#define THREADX_PACKET_SIZE             (WICED_LINK_MTU)
#endif /* THREADX_PACKET_SIZE  */

#define THREADX_TX_POOL_SIZE            ((THREADX_PACKET_SIZE + sizeof(NX_PACKET)) * THREADX_TX_PACKET_COUNT)
#define THREADX_RX_POOL_SIZE            ((THREADX_PACKET_SIZE + sizeof(NX_PACKET)) * THREADX_RX_PACKET_COUNT)

#ifndef THREADX_ARP_CACHE_SIZE
#define THREADX_ARP_CACHE_SIZE          512
#endif /* THREADX_ARP_CACHE_SIZE  */


/* Define the address of SNTP Server. If not defined, use DNS module to resolve the host name THREADX_SNTP_SERVER_NAME.  */
/*
#define THREADX_SNTP_SERVER_ADDRESS     IP_ADDRESS(118, 190, 21, 209)
*/

#ifndef THREADX_SNTP_SERVER_NAME
#define THREADX_SNTP_SERVER_NAME        "0.pool.ntp.org"    /* SNTP Server.  */
#endif /* THREADX_SNTP_SERVER_NAME */

#ifndef THREADX_SNTP_SYNC_MAX
#define THREADX_SNTP_SYNC_MAX           3
#endif /* THREADX_SNTP_SYNC_MAX */

#ifndef THREADX_SNTP_UPDATE_MAX
#define THREADX_SNTP_UPDATE_MAX         10
#endif /* THREADX_SNTP_UPDATE_MAX */

/* Default time. GMT: Monday, June 1, 2020 12:00:00 AM. Epoch timestamp: 1590969601.  */
#ifndef THREADX_SYSTEM_TIME 
#define THREADX_SYSTEM_TIME             1590969601
#endif /* THREADX_SYSTEM_TIME  */

/* Seconds between Unix Epoch (1/1/1970) and NTP Epoch (1/1/1999) */
#define THREADX_UNIX_TO_NTP_EPOCH_SECOND 0x83AA7E80

/* Define the stack/cache for ThreadX.  */ 
static UCHAR threadx_ip_stack[THREADX_IP_STACK_SIZE];
static UCHAR threadx_tx_pool_stack[THREADX_TX_POOL_SIZE];
static UCHAR threadx_rx_pool_stack[THREADX_RX_POOL_SIZE];
static UCHAR threadx_arp_cache_area[THREADX_ARP_CACHE_SIZE];


/* Define the prototypes for ThreadX.  */
static NX_PACKET_POOL                   nx_pool[2]; /* 0=TX, 1=RX.  */
static NX_IP                            ip_0;
static NX_DNS                           dns_client;
NX_DNS                                  *_threadx_dns_client_created_ptr;


#ifndef THREADX_SNTP_DISABLE
NX_SNTP_CLIENT                          sntp_client;
#endif /* THREADX_SNTP_DISABLE */

/* Threadx time offset for agenttime_threadx.c.  */
ULONG                                   _threadx_time_offset;

#ifndef THREADX_DHCP_DISABLE

#include <nxd_dhcp_client.h>
static NX_DHCP                          dhcp_client;
static void                             wait_dhcp(void);

#define THREADX_IPV4_ADDRESS            IP_ADDRESS(0, 0, 0, 0)
#define THREADX_IPV4_MASK               IP_ADDRESS(0, 0, 0, 0)

#else

#ifndef THREADX_IPV4_ADDRESS
//#define THREADX_IPV4_ADDRESS          IP_ADDRESS(192, 168, 100, 33)
#error "SYMBOL THREADX_IPV4_ADDRESS must be defined. This symbol specifies the IP address of device. "

#endif /* THREADX_IPV4_ADDRESS */
#ifndef THREADX_IPV4_MASK
//#define THREADX_IPV4_MASK             0xFFFFFF00UL
#error "SYMBOL THREADX_IPV4_MASK must be defined. This symbol specifies the IP address mask of device. "
#endif /* IPV4_MASK */

#ifndef THREADX_GATEWAY_ADDRESS
//#define THREADX_GATEWAY_ADDRESS       IP_ADDRESS(192, 168, 100, 1)
#error "SYMBOL THREADX_GATEWAY_ADDRESS must be defined. This symbol specifies the gateway address for routing. "
#endif /* THREADX_GATEWAY_ADDRESS */

#ifndef THREADX_DNS_SERVER_ADDRESS
//#define THREADX_DNS_SERVER_ADDRESS      IP_ADDRESS(192, 168, 100, 1)
#error "SYMBOL THREADX_DNS_SERVER_ADDRESS must be defined. This symbol specifies the dns server address for routing. "
#endif /* THREADX_DNS_SERVER_ADDRESS */

#endif /* THREADX_DHCP_DISABLE  */

static UINT dns_create();

#ifndef THREADX_SNTP_DISABLE
static UINT sntp_time_sync();
#endif /* THREADX_SNTP_DISABLE */

int platform_init(void)
{

UINT    status;
ULONG   ip_address;
ULONG   network_mask;
ULONG   gateway_address;


    /* Initialize the NetX system.  */
    nx_system_initialize();

    /* Create a packet pool.  */
    status = nx_packet_pool_create(&nx_pool[0], "NetX TX Packet Pool", THREADX_PACKET_SIZE,
                                   threadx_tx_pool_stack , THREADX_TX_POOL_SIZE);    
    
    /* Check for pool creation error.  */
    if (status)
    {
      /* LogError */
        printf("THREADX platform initialize fail: PACKET POOL CREATE FAIL.");
        return(MU_FAILURE);
    }
    
    status = nx_packet_pool_create(&nx_pool[1], "NetX RX Packet Pool", THREADX_PACKET_SIZE,
                                   threadx_rx_pool_stack , THREADX_RX_POOL_SIZE);
    
    /* Check for pool creation error.  */
    if (status)
    {
        nx_packet_pool_delete(&nx_pool[0]);
        /* LogError */
        printf("THREADX platform initialize fail: PACKET POOL CREATE FAIL.");
        return(MU_FAILURE);
    }
    
    /* Join Wifi network.  */
    if (wifi_network_join(&nx_pool))
    {
        nx_packet_pool_delete(&nx_pool[0]);
        nx_packet_pool_delete(&nx_pool[1]);
        /* LogError */
        printf("THREADX platform initialize fail: WIFI Join FAIL.");
        return(MU_FAILURE);
    }

    /* Create an IP instance for the DHCP Client. The rest of the DHCP Client set up is handled
       by the client thread entry function.  */
     status = nx_ip_create(&ip_0, "NetX IP Instance 0", THREADX_IPV4_ADDRESS, THREADX_IPV4_MASK,
                           &nx_pool[0], platform_driver_get(), (UCHAR*)threadx_ip_stack, THREADX_IP_STACK_SIZE, 1);

    /* Check for IP create errors.  */
    if (status)
    {
        nx_packet_pool_delete(&nx_pool[0]);
        nx_packet_pool_delete(&nx_pool[1]);
        /* LogError */
        printf("THREADX platform initialize fail: IP CREATE FAIL.");
        return(MU_FAILURE);
    }

    /* Enable ARP and supply ARP cache memory for IP Instance 0.  */
    status = nx_arp_enable(&ip_0, (VOID *)threadx_arp_cache_area, THREADX_ARP_CACHE_SIZE);

    /* Check for ARP enable errors.  */
    if (status)
    {
        nx_ip_delete(&ip_0);
        nx_packet_pool_delete(&nx_pool[0]);
        nx_packet_pool_delete(&nx_pool[1]);
        /* LogError */
        printf("THREADX platform initialize fail: ARP ENABLE FAIL.");
        return(MU_FAILURE);
    }

    /* Enable ICMP traffic.  */
    status = nx_icmp_enable(&ip_0);

    /* Check for ICMP enable errors.  */
    if (status)
    {
        nx_ip_delete(&ip_0);
        nx_packet_pool_delete(&nx_pool[0]);
        nx_packet_pool_delete(&nx_pool[1]);
        /* LogError */
        printf("THREADX platform initialize fail: ICMP ENABLE FAIL.");
        return(MU_FAILURE);
    }

    /* Enable TCP traffic.  */
    status = nx_tcp_enable(&ip_0);

    /* Check for TCP enable errors.  */
    if (status)
    {
        nx_ip_delete(&ip_0);
        nx_packet_pool_delete(&nx_pool[0]);
        nx_packet_pool_delete(&nx_pool[1]);
        /* LogError */
        printf("THREADX platform initialize fail: TCP ENABLE FAIL.");
        return(MU_FAILURE);
    }

    /* Enable UDP traffic.  */
    status = nx_udp_enable(&ip_0);

    /* Check for UDP enable errors.  */
    if (status)
    {
        nx_ip_delete(&ip_0);
        nx_packet_pool_delete(&nx_pool[0]);
        nx_packet_pool_delete(&nx_pool[1]);
        /* LogError */
        printf("THREADX platform initialize fail: UDP ENABLE FAIL.");
        return(MU_FAILURE);
    }

#ifndef THREADX_DHCP_DISABLE
    wait_dhcp();
#else
    nx_ip_gateway_address_set(&ip_0, THREADX_GATEWAY_ADDRESS);
#endif /* THREADX_DHCP_DISABLE  */

    /* Get IP address and gateway address. */
    nx_ip_address_get(&ip_0, &ip_address, &network_mask);
    nx_ip_gateway_address_get(&ip_0, &gateway_address);

    /* Output IP address and gateway address. */
    /* LogInfo */
    printf("IP address: %lu.%lu.%lu.%lu",
           (ip_address >> 24),
           (ip_address >> 16 & 0xFF),
           (ip_address >> 8 & 0xFF),
           (ip_address & 0xFF));
    printf("Mask: %lu.%lu.%lu.%lu",
           (network_mask >> 24),
           (network_mask >> 16 & 0xFF),
           (network_mask >> 8 & 0xFF),
           (network_mask & 0xFF));
    printf("Gateway: %lu.%lu.%lu.%lu",
           (gateway_address >> 24),
           (gateway_address >> 16 & 0xFF),
           (gateway_address >> 8 & 0xFF),
           (gateway_address & 0xFF));

    /* Ceate dns.  */
    status = dns_create();

    /* Check for DNS create errors.  */
    if (status)
    {
        nx_dhcp_delete(&dhcp_client);
        nx_ip_delete(&ip_0);
        nx_packet_pool_delete(&nx_pool[0]);
        nx_packet_pool_delete(&nx_pool[1]);
        /* LogError */
        printf("THREADX platform initialize fail: DNS CREATE FAIL.");
        return(MU_FAILURE);
    }

#ifndef THREADX_SNTP_DISABLE
    for (UINT i = 0; i < THREADX_SNTP_SYNC_MAX; i++)
    {

        /* Start SNTP to sync the local time.  */
        status = sntp_time_sync();

        /* Check status.  */
        if(status == NX_SUCCESS)
            break;
    }

    /* Check status.  */
    if (status)
    {
        /* LogInfo */
        printf("SNTP Time Sync failed.");
        printf("Set Time to default value: THREADX_SYSTEM_TIME.");
        _threadx_time_offset = THREADX_SYSTEM_TIME;
    }
    else
    {
      /* LogInfo */
        printf("SNTP Time Sync successfully.");
    }
#else
    _threadx_time_offset = THREADX_SYSTEM_TIME;
#endif /* THREADX_SNTP_DISABLE */

    /* Initialize TLS.  */
    nx_secure_tls_initialize();

    /* Initialize ThreadX Azure SDK.  */
    // threadx_azure_sdk_initialize();

    return (0);
}


void platform_deinit(void)
{

    /* Cleanup the resource.  */
    // threadx_azure_sdk_deinitialize();

    /* Cleanup TLS.  */
    tx_mutex_delete(&_nx_secure_tls_protection);

    /* Cleanup DNS.  */
    nx_dns_delete(&dns_client);

    /* Cleanup DHCP.  */
    nx_dhcp_delete(&dhcp_client);

    /* Cleanup IP and packet pool.  */
    nx_ip_delete(&ip_0);
    nx_packet_pool_delete(&nx_pool[0]);
    nx_packet_pool_delete(&nx_pool[1]);
}

#ifndef THREADX_DHCP_DISABLE
static void wait_dhcp(void)
{

ULONG   actual_status;

    /* LogInfo */
    printf("DHCP In Progress...");

    /* Create the DHCP instance.  */
    nx_dhcp_create(&dhcp_client, &ip_0, "dhcp_client");

    /* Start the DHCP Client.  */
    nx_dhcp_start(&dhcp_client);

    /* Wait util address is solved. */
    nx_ip_status_check(&ip_0, NX_IP_ADDRESS_RESOLVED, &actual_status, NX_WAIT_FOREVER);
}
#endif /* THREADX_DHCP_DISABLE  */


static UINT dns_create()
{
      
UINT    status;
ULONG   dns_server_address[3];
UINT    dns_server_address_size = 12;
 
    /* Create a DNS instance for the Client.  Note this function will create
       the DNS Client packet pool for creating DNS message packets intended
       for querying its DNS server. */
    status = nx_dns_create(&dns_client, &ip_0, (UCHAR *)"DNS Client");
    if (status)
    {
        return(status);
    }

    /* Is the DNS client configured for the host application to create the pecket pool? */
#ifdef NX_DNS_CLIENT_USER_CREATE_PACKET_POOL   

    /* Yes, use the packet pool created above which has appropriate payload size
       for DNS messages. */
    status = nx_dns_packet_pool_set(&dns_client, ip_0.nx_ip_default_packet_pool);
    if (status)
    {
        nx_dns_delete(&dns_client);
        return(status);
    }
#endif /* NX_DNS_CLIENT_USER_CREATE_PACKET_POOL */  

#ifndef THREADX_DHCP_DISABLE
    /* Retrieve DNS server address.  */
    nx_dhcp_interface_user_option_retrieve(&dhcp_client, 0, NX_DHCP_OPTION_DNS_SVR, (UCHAR *)(dns_server_address), &dns_server_address_size); 
#else
    dns_server_address[0] = THREADX_DNS_SERVER_ADDRESS;
#endif    
    
    /* Add an IPv4 server address to the Client list. */
    status = nx_dns_server_add(&dns_client, dns_server_address[0]);
    if (status)
    {
        nx_dns_delete(&dns_client);
        return(status);
    }
    
    /* Record the dns client, it will be used in socketio_threadx.c  */
    _threadx_dns_client_created_ptr = &dns_client;
    
    /* Output DNS Server address.  */
    /* LogInfo */
    printf("DNS Server address: %lu.%lu.%lu.%lu\r\n",
           (dns_server_address[0] >> 24),
           (dns_server_address[0] >> 16 & 0xFF),
           (dns_server_address[0] >> 8 & 0xFF),
           (dns_server_address[0] & 0xFF));
    
    return(0);
}

#ifndef THREADX_SNTP_DISABLE

/* Sync up the local time.  */
static UINT sntp_time_sync()
{

UINT    status;
UINT    server_status;
ULONG   sntp_server_address;
UINT    i;


    /* LogInfo */
    printf("SNTP Time Sync...");

#ifndef THREADX_SNTP_SERVER_ADDRESS
    /* Look up an IPv4 address over IPv4. */
    status = nx_dns_host_by_name_get(_threadx_dns_client_created_ptr, (UCHAR *)THREADX_SNTP_SERVER_NAME, &sntp_server_address, 5*NX_IP_PERIODIC_RATE);

    /* Check status.  */
    if (status)
    {
        return(status);
    }
#else
    sntp_server_address = THREADX_SNTP_SERVER_ADDRESS;
#endif

    /* Create the SNTP Client to run in broadcast mode.. */
    status =  nx_sntp_client_create(&sntp_client, &ip_0, 0, ip_0.nx_ip_default_packet_pool,  
                                    NX_NULL,
                                    NX_NULL,
                                    NX_NULL /* no random_number_generator callback */);

    /* Check status.  */
    if (status)
    {
        return(status);
    }

    /* Use the IPv4 service to initialize the Client and set the IPv4 SNTP server. */
    status = nx_sntp_client_initialize_unicast(&sntp_client, sntp_server_address);

    /* Check status.  */
    if (status)
    {
        nx_sntp_client_delete(&sntp_client);
        return(status);
    }

    /* Set local time to 0 */
    status = nx_sntp_client_set_local_time(&sntp_client, 0, 0);

    /* Check status.  */
    if (status)
    {
        nx_sntp_client_delete(&sntp_client);
        return(status);
    }

    /* Run Unicast client */
    status = nx_sntp_client_run_unicast(&sntp_client);

    /* Check status.  */
    if (status)
    {
        nx_sntp_client_stop(&sntp_client);
        nx_sntp_client_delete(&sntp_client);
        return(status);
    }

    /* Wait till updates are received */
    for (i = 0; i < THREADX_SNTP_UPDATE_MAX; i++)
    {

        /* First verify we have a valid SNTP service running. */
        status = nx_sntp_client_receiving_updates(&sntp_client, &server_status);

        /* Check status.  */
        if ((status == NX_SUCCESS) && (server_status == NX_TRUE))
        {

            /* Server status is good. Now get the Client local time. */

            ULONG sntp_seconds, sntp_fraction;
            ULONG threadx_time_in_second;

            /* Get the local time.  */
            status = nx_sntp_client_get_local_time(&sntp_client, &sntp_seconds, &sntp_fraction, NX_NULL);

            /* Check status.  */
            if (status != NX_SUCCESS)
            {
                continue;
            }

            /* Get the time in second in local system.  */
            threadx_time_in_second = tx_time_get() / TX_TIMER_TICKS_PER_SECOND;

            /* Convert to Unix epoch and minus the local current time.  */
            _threadx_time_offset = (sntp_seconds - threadx_time_in_second - THREADX_UNIX_TO_NTP_EPOCH_SECOND);

            /* Time sync successfully.  */

            /* Stop and delete SNTP.  */
            nx_sntp_client_stop(&sntp_client);
            nx_sntp_client_delete(&sntp_client);

            return(NX_SUCCESS);
        }

        /* Sleep.  */
        tx_thread_sleep(50);
    }

    /* Time sync failed.  */

    /* Stop and delete SNTP.  */
    nx_sntp_client_stop(&sntp_client);
    nx_sntp_client_delete(&sntp_client);

    /* Return success.  */
    return(NX_NOT_SUCCESSFUL);
}
#endif /* THREADX_SNTP_DISABLE */
