#ifndef NX_USER_H
#define NX_USER_H

// #ifndef TX_MAX_PRIORITIES
// #define TX_MAX_PRIORITIES                       32  
// #endif

// Disable the weird includes in some of the source files
#ifndef NX_DISABLE_INCLUDE_SOURCE_CODE
#define NX_DISABLE_INCLUDE_SOURCE_CODE
#endif

extern UINT nx_rand16( void );
#define NX_RAND                         nx_rand16

#define NX_DNS_CLIENT_USER_CREATE_PACKET_POOL
#define NXD_MQTT_MAX_TOPIC_NAME_LENGTH  200
#define NXD_MQTT_MAX_MESSAGE_LENGTH     200

#define NX_SECURE_ENABLE

#define NX_ASSERT_FAIL for(;;){}

/* Symbols for Wiced.  */

/* This define specifies the size of the physical packet header. The default value is 16 (based on
   a typical 16-byte Ethernet header).  */
#define NX_PHYSICAL_HEADER              (14 + 12 + 18)

/* This define specifies the size of the physical packet trailer and is typically used to reserve storage
   for things like Ethernet CRCs, etc.  */
#define NX_PHYSICAL_TRAILER             (0)

#define NX_LINK_PTP_SEND                51      /* Precision Time Protocol */

#endif
