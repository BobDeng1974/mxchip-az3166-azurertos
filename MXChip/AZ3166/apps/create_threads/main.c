/* This is a small demo of the high-performance ThreadX kernel.  It includes examples of eight
   threads of different priorities, using a message queue, semaphore, mutex, event flags group,
   byte pool, and block pool.  */


/* Includes */
#include <stdio.h>
#include <tx_api.h>

/* Private macro */
#define MESSAGE1   "  Express Logic's   "
#define MESSAGE2   "     ThreadX        "
#define MESSAGE3   "    running on      "
#define MESSAGE4   " STM3240_41_G-EVAL  "
#define MESSAGE5   "    built with      "
#define MESSAGE6   " Atollic TrueSTUDIO "
#define MESSAGEb   "                    "


/* ThreadX definitions.  */
#define DEMO_STACK_SIZE         1024
#define SPARE                   1024
#define DEMO_QUEUE_SIZE         100
#define DEMO_BLOCK_POOL_SIZE    100
#define NUMBER_TASK             9
#define DEMO_BYTE_POOL_SIZE     SPARE+ DEMO_STACK_SIZE+ (NUMBER_TASK*DEMO_STACK_SIZE) + (2*DEMO_QUEUE_SIZE) + DEMO_BLOCK_POOL_SIZE
char pool[DEMO_BYTE_POOL_SIZE];


/* Define the ThreadX object control blocks...  */


typedef struct PRIVATE_TX_OBJ
{
TX_QUEUE                queue_0;
TX_SEMAPHORE            semaphore_0;
TX_MUTEX                mutex_0;
TX_EVENT_FLAGS_GROUP    event_flags_0;
} PRIVATE_TX_OBJ;

TX_THREAD               thread[NUMBER_TASK];
TX_THREAD               thread_init;
PRIVATE_TX_OBJ          tx_obj;
TX_BYTE_POOL            byte_pool_0;
TX_BLOCK_POOL           block_pool_0;



/* Define the counters used in the demo application...  */

ULONG                   thread_0_counter;
ULONG                   thread_1_counter;
ULONG                   thread_2_counter;
ULONG                   thread_3_counter;
ULONG                   thread_4_counter;
ULONG                   thread_5_counter;
ULONG                   thread_6_counter;
ULONG                   thread_7_counter;
ULONG                   thread_1_messages_sent;
ULONG                   thread_2_messages_received;
ULONG                   thread_info_counter;

int                     error_count = 0;


/* Define thread prototypes.  */

void    thread_0_entry(ULONG thread_input);
void    thread_1_entry(ULONG thread_input);
void    thread_2_entry(ULONG thread_input);
void    thread_3_and_4_entry(ULONG thread_input);
void    thread_5_entry(ULONG thread_input);
void    thread_6_and_7_entry(ULONG thread_input);
void    thread_init_entry(ULONG thread_input);

/**
**===========================================================================
**
**  Abstract: main program
**
**===========================================================================
*/
int main(void)
{

  tx_kernel_enter();

  return -1;

}


/* Define what the initial system looks like.  */

void    tx_application_define(void *first_unused_memory)
{

CHAR    *pointer;
UINT    status;


    /* Create a byte memory pool from which to allocate the thread stacks.  */
    status = tx_byte_pool_create(&byte_pool_0, "byte pool 0", pool, DEMO_BYTE_POOL_SIZE);
    if (status != TX_SUCCESS){
    error_count++;
    }

    /* Allocate the stack for thread 1.  */
    status = tx_byte_allocate(&byte_pool_0, (VOID **) &pointer, DEMO_STACK_SIZE, TX_NO_WAIT);
    if (status)
        error_count++;

    /* .  */
    status = tx_thread_create(&thread_init, "thread i", thread_init_entry, 0,
            pointer, DEMO_STACK_SIZE, 1, 1, TX_NO_TIME_SLICE, TX_AUTO_START);
    if (status)
        error_count++;

}

void    thread_init_entry(ULONG thread_input){

CHAR    *pointer;
UINT    status;


    /* Put system definition stuff in here, e.g. thread creates and other assorted
       create information.  */

    /* Allocate the stack for thread 0.  */
    status = tx_byte_allocate(&byte_pool_0, (VOID **) &pointer, DEMO_STACK_SIZE, TX_NO_WAIT);
    if (status != TX_SUCCESS){
        error_count++;
    }

    /* Create the main thread.  */
    status = tx_thread_create(&thread[0], "thread 0", thread_0_entry, 0,
            pointer, DEMO_STACK_SIZE,
            1, 1, TX_NO_TIME_SLICE, TX_AUTO_START);
    if (status){
        error_count++;
    }

    /* Allocate the stack for thread 1.  */
    status = tx_byte_allocate(&byte_pool_0, (VOID **) &pointer, DEMO_STACK_SIZE, TX_NO_WAIT);
    if (status != TX_SUCCESS){
        error_count++;
    }

    /* Create threads 1 and 2. These threads pass information through a ThreadX
       message queue.  It is also interesting to note that these threads have a time
       slice.  */
    status = tx_thread_create(&thread[1], "thread 1", thread_1_entry, 1,
            pointer, DEMO_STACK_SIZE,
        16, 16, 4, TX_AUTO_START);
    if (status){
        error_count++;
    }

    /* Allocate the stack for thread 2.  */
    status = tx_byte_allocate(&byte_pool_0, (VOID **) &pointer, DEMO_STACK_SIZE, TX_NO_WAIT);
    if (status != TX_SUCCESS){
        error_count++;
    }

    status = tx_thread_create(&thread[2], "thread 2", thread_2_entry, 2,
            pointer, DEMO_STACK_SIZE,
            16, 16, 4, TX_AUTO_START);
    if (status){
        error_count++;
    }
    /* Allocate the stack for thread 3.  */
    status = tx_byte_allocate(&byte_pool_0, (VOID **) &pointer, DEMO_STACK_SIZE, TX_NO_WAIT);
    if (status != TX_SUCCESS){
        error_count++;
    }

    /* Create threads 3 and 4.  These threads compete for a ThreadX counting semaphore.
       An interesting thing here is that both threads share the same instruction area.  */
    status = tx_thread_create(&thread[3], "thread 3", thread_3_and_4_entry, 3,
            pointer, DEMO_STACK_SIZE,
            8, 8, TX_NO_TIME_SLICE, TX_AUTO_START);
    if (status){
        error_count++;
    }


    /* Allocate the stack for thread 4.  */
    status = tx_byte_allocate(&byte_pool_0, (VOID **) &pointer, DEMO_STACK_SIZE, TX_NO_WAIT);
    if (status != TX_SUCCESS){
        error_count++;
    }

    status = tx_thread_create(&thread[4], "thread 4", thread_3_and_4_entry, 4,
            pointer, DEMO_STACK_SIZE,
            8, 8, TX_NO_TIME_SLICE, TX_AUTO_START);
    if (status){
        error_count++;
    }

    /* Allocate the stack for thread 5.  */
    status = tx_byte_allocate(&byte_pool_0, (VOID **) &pointer, DEMO_STACK_SIZE, TX_NO_WAIT);
    if (status != TX_SUCCESS){
        error_count++;
    }

    /* Create thread 5.  This thread simply pends on an event flag which will be set
       by thread_0.  */
    status = tx_thread_create(&thread[5], "thread 5", thread_5_entry, 5,
            pointer, DEMO_STACK_SIZE,
            4, 4, TX_NO_TIME_SLICE, TX_AUTO_START);
    if (status){
        error_count++;
    }


    /* Allocate the stack for thread 6.  */
    status = tx_byte_allocate(&byte_pool_0, (VOID **) &pointer, DEMO_STACK_SIZE, TX_NO_WAIT);
    if (status != TX_SUCCESS){
        error_count++;
    }

    /* Create threads 6 and 7.  These threads compete for a ThreadX mutex.  */
    status = tx_thread_create(&thread[6], "thread 6", thread_6_and_7_entry, 6,
            pointer, DEMO_STACK_SIZE,
            8, 8, TX_NO_TIME_SLICE, TX_AUTO_START);
    if (status){
        error_count++;
    }


    /* Allocate the stack for thread 7.  */
    status = tx_byte_allocate(&byte_pool_0, (VOID **) &pointer, DEMO_STACK_SIZE, TX_NO_WAIT);
    if (status != TX_SUCCESS){
        error_count++;
    }

    status = tx_thread_create(&thread[7], "thread 7", thread_6_and_7_entry, 7,
            pointer, DEMO_STACK_SIZE,
            8, 8, TX_NO_TIME_SLICE, TX_AUTO_START);
    if (status){
        error_count++;
    }

    /* Allocate the message queue.  */
    status = tx_byte_allocate(&byte_pool_0, (VOID **) &pointer, DEMO_QUEUE_SIZE*sizeof(ULONG), TX_NO_WAIT);
    if (status != TX_SUCCESS){
        error_count++;
    }

    /* Create the message queue shared by threads 1 and 2.  */
    status = tx_queue_create(&tx_obj.queue_0, "queue 0", TX_1_ULONG, pointer, DEMO_QUEUE_SIZE*sizeof(ULONG));
    if (status != TX_SUCCESS){
        error_count++;
    }

    /* Create the semaphore used by threads 3 and 4.  */
    status = tx_semaphore_create(&tx_obj.semaphore_0, "semaphore 0", 1);
    if (status != TX_SUCCESS){
        error_count++;
    }

    /* Create the event flags group used by threads 1 and 5.  */
    status = tx_event_flags_create(&tx_obj.event_flags_0, "event flags 0");
    if (status != TX_SUCCESS){
        error_count++;
    }

    /* Create the mutex used by thread 6 and 7 without priority inheritance.  */
    status = tx_mutex_create(&tx_obj.mutex_0, "mutex 0", TX_NO_INHERIT);
    if (status != TX_SUCCESS){
        error_count++;
    }

    /* Allocate the memory for a small block pool.  */
    status = tx_byte_allocate(&byte_pool_0, (VOID **) &pointer, DEMO_BLOCK_POOL_SIZE, TX_NO_WAIT);
    if (status != TX_SUCCESS){
        error_count++;
    }

    /* Create a block memory pool to allocate a message buffer from.  */
    status = tx_block_pool_create(&block_pool_0, "block pool 0", sizeof(ULONG), pointer, DEMO_BLOCK_POOL_SIZE);
    if (status != TX_SUCCESS){
        error_count++;
    }

    /* Allocate a block and release the block memory.  */
    status = tx_block_allocate(&block_pool_0, (VOID **) &pointer, TX_NO_WAIT);
    if (status != TX_SUCCESS){
        error_count++;
    }

    /* Release the block back to the pool.  */
    status = tx_block_release(pointer);
    if (status != TX_SUCCESS){
        error_count++;
    }

    if (error_count){
        while (1){}
    }
}



/* Define the test threads.  */

void    thread_0_entry(ULONG thread_input)
{

UINT    status;


    /* This thread simply sits in while-forever-sleep loop.  */
    while(1)
    {

        /* Increment the thread counter.  */
        thread_0_counter++;

        /* Sleep for 10 ticks.  */
        tx_thread_sleep(10);

        /* Set event flag 0 to wakeup thread 5.  */
        status =  tx_event_flags_set(&tx_obj.event_flags_0, 0x1, TX_OR);

        /* Check status.  */
        if (status != TX_SUCCESS)
            break;
    }
}


void    thread_1_entry(ULONG thread_input)
{

UINT    status;


    /* This thread simply sends messages to a queue shared by thread 2.  */
    while(1)
    {

        /* Increment the thread counter.  */
        thread_1_counter++;

        /* Send message to queue 0.  */
        status =  tx_queue_send(&tx_obj.queue_0, &thread_1_messages_sent, TX_WAIT_FOREVER);

        /* Check completion status.  */
        if (status != TX_SUCCESS)
            break;

        /* Increment the message sent.  */
        thread_1_messages_sent++;
    }
}


void    thread_2_entry(ULONG thread_input)
{

ULONG   received_message;
UINT    status;


    /* This thread retrieves messages placed on the queue by thread 1.  */
    while(1)
    {

        /* Increment the thread counter.  */
        thread_2_counter++;

        /* Retrieve a message from the queue.  */
        status = tx_queue_receive(&tx_obj.queue_0, &received_message, TX_WAIT_FOREVER);

        /* Check completion status and make sure the message is what we
           expected.  */
        if ((status != TX_SUCCESS) || (received_message != thread_2_messages_received))
            break;

        /* Otherwise, all is okay.  Increment the received message count.  */
        thread_2_messages_received++;
    }
}


void    thread_3_and_4_entry(ULONG thread_input)
{

UINT    status;


    /* This function is executed from thread 3 and thread 4.  As the loop
       below shows, these function compete for ownership of semaphore_0.  */
    while(1)
    {

        /* Increment the thread counter.  */
        if (thread_input == 3)
            thread_3_counter++;
        else
            thread_4_counter++;

        /* Get the semaphore with suspension.  */
        status =  tx_semaphore_get(&tx_obj.semaphore_0, TX_WAIT_FOREVER);

        /* Check status.  */
        if (status != TX_SUCCESS)
            break;

        /* Sleep for 2 ticks to hold the semaphore.  */
        tx_thread_sleep(2);

        /* Release the semaphore.  */
        status =  tx_semaphore_put(&tx_obj.semaphore_0);

        /* Check status.  */
        if (status != TX_SUCCESS)
            break;
    }
}


void    thread_5_entry(ULONG thread_input)
{

UINT    status;
ULONG   actual_flags;


    /* This thread simply waits for an event in a forever loop.  */
    while(1)
    {

        /* Increment the thread counter.  */
        thread_5_counter++;

        /* Wait for event flag 0.  */
        status =  tx_event_flags_get(&tx_obj.event_flags_0, 0x1, TX_OR_CLEAR,
                                                &actual_flags, TX_WAIT_FOREVER);

        /* Check status.  */
        if ((status != TX_SUCCESS) || (actual_flags != 0x1))
            break;
    }
}


void    thread_6_and_7_entry(ULONG thread_input)
{

UINT    status;


    /* This function is executed from thread 6 and thread 7.  As the loop
       below shows, these function compete for ownership of mutex_0.  */
    while(1)
    {

        /* Increment the thread counter.  */
        if (thread_input == 6)
            thread_6_counter++;
        else
            thread_7_counter++;

        /* Get the mutex with suspension.  */
        status =  tx_mutex_get(&tx_obj.mutex_0, TX_WAIT_FOREVER);

        /* Check status.  */
        if (status != TX_SUCCESS)
            break;

        /* Get the mutex again with suspension.  This shows
           that an owning thread may retrieve the mutex it
           owns multiple times.  */
        status =  tx_mutex_get(&tx_obj.mutex_0, TX_WAIT_FOREVER);

        /* Check status.  */
        if (status != TX_SUCCESS)
            break;

        /* Sleep for 2 ticks to hold the mutex.  */
        tx_thread_sleep(2);

        /* Release the mutex.  */
        status =  tx_mutex_put(&tx_obj.mutex_0);

        /* Check status.  */
        if (status != TX_SUCCESS)
            break;

        /* Release the mutex again.  This will actually
           release ownership since it was obtained twice.  */
        status =  tx_mutex_put(&tx_obj.mutex_0);

        /* Check status.  */
        if (status != TX_SUCCESS)
            break;
    }
}