/**************************************************************************/
/*                                                                        */
/*            Copyright (c) 1996-2018 by Express Logic Inc.               */
/*                                                                        */
/*  This software is copyrighted by and is the sole property of Express   */
/*  Logic, Inc.  All rights, title, ownership, or other interests         */
/*  in the software remain the property of Express Logic, Inc.  This      */
/*  software may only be used in accordance with the corresponding        */
/*  license agreement.  Any unauthorized use, duplication, transmission,  */
/*  distribution, or disclosure of this software is expressly forbidden.  */
/*                                                                        */
/*  This Copyright notice may not be removed or modified without prior    */
/*  written consent of Express Logic, Inc.                                */
/*                                                                        */
/*  Express Logic, Inc. reserves the right to modify this software        */
/*  without notice.                                                       */
/*                                                                        */
/*  Express Logic, Inc.                     info@expresslogic.com         */
/*  11423 West Bernardo Court               http://www.expresslogic.com   */
/*  San Diego, CA  92127                                                  */
/*                                                                        */
/**************************************************************************/

#include <nx_api.h>
#include <stm32f4xx_hal.h>
#include <board_init.h>
#include <wwd_management.h>
#include <wwd_wifi.h>
#include <wwd_buffer_interface.h>
#include <wwd_network.h>

/* Include the sample for Azure SDK Digitaltwin.  */
extern void mqtt_iothub_run(void);

#define STR_EXPAND(x) #x
#define STR(x) STR_EXPAND(x)

/* Define the default wifi ssid and password. The user can override this 
   via -D command line option or via project settings.  */
#ifndef WIFI_SSID
#error "Symbol WIFI_SSID must be defined."
// #define WIFI_SSID                                       "A@Da"
#endif /* WIFI_SSID  */

#ifndef WIFI_PASSWORD
#error "Symbol WIFI_PASSWORD must be defined."
// #define WIFI_PASSWORD                                   "y7nh5s4n"
#endif /* WIFI_PASSWORD  */

/* WIFI Security, the security types are defined in wwwd_constants.h.  */
#ifndef WIFI_SECURITY
#error "Symbol WIFI_SECURITY must be defined."
// #define WIFI_SECURITY                                   "WICED_SECURITY_WPA2_MIXED_PSK"
#endif /* WIFI_SECURITY  */

/* Country codes are defined in wwwd_constants.h.  */
#ifndef WIFI_COUNTRY
#error "Symbol WIFI_COUNTRY must be defined."
// #define WIFI_COUNTRY                                    "WICED_COUNTRY_CHINA"
#endif /* WIFI_COUNTRY  */


/* Define the helper thread for running Azure SDK on ThreadX (THREADX IoT Platform).  */
#ifndef THREADX_AZURE_SDK_HELPER_THREAD_STACK_SIZE
#define THREADX_AZURE_SDK_HELPER_THREAD_STACK_SIZE      (4096)
#endif /* THREADX_AZURE_SDK_HELPER_THREAD_STACK_SIZE  */

#ifndef THREADX_AZURE_SDK_HELPER_THREAD_PRIORITY
#define THREADX_AZURE_SDK_HELPER_THREAD_PRIORITY        (4)
#endif /* THREADX_AZURE_SDK_HELPER_THREAD_PRIORITY  */

/* Define the memory area for helper thread.  */
UCHAR threadx_azure_sdk_helper_thread_stack[THREADX_AZURE_SDK_HELPER_THREAD_STACK_SIZE];

/* Define the prototypes for helper thread.  */
TX_THREAD threadx_azure_sdk_helper_thread;
void threadx_azure_sdk_helper_thread_entry(ULONG parameter);

/******** Optionally substitute your Ethernet driver here. ***********/
void (*platform_driver_get())(NX_IP_DRIVER *);

/* Define main entry point.  */
int main(void)
{
    /* Enable execution profile.  */
    CoreDebug -> DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT -> CTRL |= DWT_CTRL_CYCCNTENA_Msk;
    
    /* Setup platform. */
    hardware_setup();
    
    /* Enter the ThreadX kernel.  */
    tx_kernel_enter();

    return -1;
}

/* Define what the initial system looks like.  */
void    tx_application_define(void *first_unused_memory)
{

UINT  status;
    
    /* Create THREADX Azure SDK helper thread. */
    status = tx_thread_create(&threadx_azure_sdk_helper_thread, "THREADX Azure SDK Help Thread",
                     threadx_azure_sdk_helper_thread_entry, 0,
                     threadx_azure_sdk_helper_thread_stack, THREADX_AZURE_SDK_HELPER_THREAD_STACK_SIZE,
                     THREADX_AZURE_SDK_HELPER_THREAD_PRIORITY, THREADX_AZURE_SDK_HELPER_THREAD_PRIORITY, 
                     TX_NO_TIME_SLICE, TX_AUTO_START);    
    
    /* Check status.  */
    if (status)
        printf("THREADX Azure SDK Helper Thread Create Fail.\r\n");
}

/* Define THREADX Azure SDK helper thread entry.  */
void threadx_azure_sdk_helper_thread_entry(ULONG parameter)
{
    mqtt_iothub_run();
}

/* Get the network driver.  */
VOID (*platform_driver_get())(NX_IP_DRIVER *)
{
    return(wiced_sta_netx_duo_driver_entry);
}

static const wiced_ssid_t wifi_ssid =
{
    .length = sizeof(STR(WIFI_SSID))-1,
    .value  = STR(WIFI_SSID),
};

/* Join Network.  */
UINT wifi_network_join(void *pools)
{    
    
    /* Set pools for wifi.   */
    wwd_buffer_init(pools);
    
    /* Set country.  */
    if (wwd_management_wifi_on(WIFI_COUNTRY) != WWD_SUCCESS)
    {
        printf("Failed to set WiFI Country!\r\n");
        return(NX_NOT_SUCCESSFUL);
    }

    /* Attempt to join the Wi-Fi network.  */
    printf("Joining: %s\r\n", STR(WIFI_SSID));
    while (wwd_wifi_join(&wifi_ssid, WIFI_SECURITY, (uint8_t*)STR(WIFI_PASSWORD), sizeof(STR(WIFI_PASSWORD))-1, NULL, WWD_STA_INTERFACE) != WWD_SUCCESS)
    {
        printf("Failed to join: %s ... retrying\r\n", STR(WIFI_SSID));
    }
    
    printf("Successfully joined: %s.\r\n", STR(WIFI_SSID));
    
    return(NX_SUCCESS);
}