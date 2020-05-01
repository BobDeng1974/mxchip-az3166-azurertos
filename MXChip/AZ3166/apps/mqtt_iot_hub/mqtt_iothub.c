// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <tx_api.h>

void ThreadAPI_Sleep(unsigned int);
void mqtt_iothub_run(void);
extern int platform_init(void);
extern void platform_deinit(void);

void mqtt_iothub_run(void)
{
    int time_counter = 0;

    if (platform_init() != 0)
    {
        (void)printf("Failed to initialize platform.\r\n");
        return;
    }

    /* Loop to send publish message and wait for command.  */
    while (1)
    {
        /* Send publish message every five seconds.  */
        if ((time_counter % 50) == 0)
        {
            printf("Sending message...");
        }

        /* Sleep 100ms.  */
        ThreadAPI_Sleep(100);

        /* Update the counter.  */
        time_counter++;
    }

    /* Deinit platform.  */
    platform_deinit();
}

/* Process the thread sleep functionality of the SDK.  */

void ThreadAPI_Sleep(unsigned int milliseconds)
{
    UINT ticks;

    /* Change milliseconds to ticks.  */
    ticks = (milliseconds * TX_TIMER_TICKS_PER_SECOND) / 1000;

    /* Check if ticks is zero.  */
    if (ticks == 0)
        ticks = 1;

    tx_thread_sleep(ticks);
}
