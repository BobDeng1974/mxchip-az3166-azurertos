/**************************************************************************/
/*                                                                        */
/*       Copyright (c) Microsoft Corporation. All rights reserved.        */
/*                                                                        */
/**************************************************************************/

#include <nx_api.h>
#include "wifi.h"
#include "nx_wifi.h"
#include <nxd_dns.h>
#include <nx_secure_tls_api.h>
#include <stm32f4xx_hal.h>

/* Include the demo.  */
extern VOID sample_entry(NX_IP *ip_ptr, NX_PACKET_POOL *pool_ptr, NX_DNS *dns_ptr);

/* Define the thread for running Azure demo on ThreadX (X-Ware IoT Platform).  */
#ifndef SAMPLE_THREAD_STACK_SIZE
#define SAMPLE_THREAD_STACK_SIZE        (4096)
#endif /* SAMPLE_THREAD_STACK_SIZE  */

#ifndef SAMPLE_THREAD_PRIORITY
#define SAMPLE_THREAD_PRIORITY          (4)
#endif /* SAMPLE_THREAD_PRIORITY  */

/* Define the memory area for the sample thread.  */
static UCHAR sample_thread_stack[SAMPLE_THREAD_STACK_SIZE];

/* Define the prototypes for the sample thread.  */
static TX_THREAD sample_thread;
static void sample_thread_entry(ULONG parameter);

/* Define the default thread priority, stack size, etc. The user can override this 
   via -D command line option or via project settings.  */

#ifndef SAMPLE_PACKET_COUNT
#define SAMPLE_PACKET_COUNT             (20)
#endif /* SAMPLE_PACKET_COUNT  */

#ifndef SAMPLE_PACKET_SIZE
#define SAMPLE_PACKET_SIZE              (1200)  /* Set the default value to 1200 since WIFI payload size (ES_WIFI_PAYLOAD_SIZE) is 1200.  */
#endif /* SAMPLE_PACKET_SIZE  */

#define SAMPLE_POOL_SIZE                ((SAMPLE_PACKET_SIZE + sizeof(NX_PACKET)) * SAMPLE_PACKET_COUNT)

/* Define the stack/cache for ThreadX.  */
static UCHAR sample_pool_stack[SAMPLE_POOL_SIZE];

/* Define the prototypes for ThreadX.  */
static NX_IP                            ip_0;
static NX_PACKET_POOL                   pool_0;
static NX_DNS     				    	dns_client;

/* Include the board setup.  */
extern void board_setup();

static UINT	dns_create();

/* Define main entry point.  */
int main(void)
{
    /* Setup platform. */
    board_setup();
    
    /* Enter the ThreadX kernel.  */
    tx_kernel_enter();
}

/* Define what the initial system looks like.  */
void    tx_application_define(void *first_unused_memory)
{

UINT    status;

    /* Initialize the NetX system.  */
    nx_system_initialize();

    /* Create a packet pool.  */
    status = nx_packet_pool_create(&pool_0, "NetX Main Packet Pool", SAMPLE_PACKET_SIZE,
                                   sample_pool_stack , SAMPLE_POOL_SIZE);
    
    /* Check for pool creation error.  */
    if (status)
    {
        printf("PACKET POOL CREATE FAIL.\r\n");
        return;
    }
        
    /* Create an IP instance.  */
     status = nx_ip_create(&ip_0, "NetX IP Instance 0", 0, 0,
                           &pool_0, NULL, NULL, NULL, 0);

    /* Check for IP create errors.  */
    if (status)
    {
	  	printf("IP CREATE FAIL.\r\n");
        return;
    }
    
    /* Initialize THREADX Wifi.  */
    status = nx_wifi_initialize(&ip_0, &pool_0);
    
    /* Check status.  */
    if (status)
    {
        printf("WIFI INITIALIZE FAIL.\r\n");
        return;
    }     
    
    /* Initialize TLS.  */
    nx_secure_tls_initialize();
    
    /* Create Threadx Azure thread. */
    status = tx_thread_create(&sample_thread, "Sample Thread",
                              sample_thread_entry, 0,
                              sample_thread_stack, SAMPLE_THREAD_STACK_SIZE,
                              SAMPLE_THREAD_PRIORITY, SAMPLE_THREAD_PRIORITY, 
                              TX_NO_TIME_SLICE, TX_AUTO_START);    
    
    /* Check status.  */
    if (status)
    {
        printf("Azure Demo Thread Create Failed.\r\n");
    }
}

/* Define THREADX Azure SDK thread entry.  */
void sample_thread_entry(ULONG parameter)
{

    /* Ceate dns.  */
    if (dns_create())
    {
        printf("DNS CREATE FAIL.\r\n");
        return;
    }
        
    /* Start sample.  */
    sample_entry(&ip_0, &pool_0, &dns_client);
}

static UINT	dns_create(ULONG dns_server_address)
{
      
UINT	status; 
UCHAR   dns_address_1[4]; 
UCHAR   dns_address_2[4];
 
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
    status = nx_dns_packet_pool_set(&dns_client, &pool_0);
	if (status)
    {
        return(status);
    }
#endif /* NX_DNS_CLIENT_USER_CREATE_PACKET_POOL */  

    /* Get the DNS address.  */
    if (WIFI_GetDNS_Address(dns_address_1, dns_address_2) != WIFI_STATUS_OK)
    {
        nx_dns_delete(&dns_client);
        return(NX_NOT_SUCCESSFUL);
    }
        
    /* Add an IPv4 server address to the Client list. */
    status = nx_dns_server_add(&dns_client, IP_ADDRESS(dns_address_1[0], dns_address_1[1], dns_address_1[2], dns_address_1[3]));
	if (status)
    {
        nx_dns_delete(&dns_client);
        return(status);
    }
	
	return(NX_SUCCESS);
}