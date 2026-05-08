/***********************************************************************************************************************
 * File Name    : sender_task.c
 * Description  : Contains function definition.
 **********************************************************************************************************************/
/*
 * Copyright (c) 2020 - 2024 Renesas Electronics Corporation and/or its affiliates
 * 
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "sender_task.h"
#include "common_utils.h"
#include "freertos_ep.h"
#include "gtm_timer_initialize.h"
#define MODULE_NAME             "FreeRTOS Message Queue & Semaphore"

/* Flag to check Message Queue task completion */
static bool b_suspend_msqQ_task = false;

/* Cycle counter */
static uint32_t g_cycle_count = 0;

/* Sender Task entry function */
/* pvParameters contains TaskHandle_t */
void sender_task_entry(void *pvParameters)
{
    FSP_PARAMETER_NOT_USED (pvParameters);

    /* Variable to store task state */
    eTaskState Task_State = (eTaskState) RESET_VALUE;

    fsp_pack_version_t version = {RESET_VALUE};

    /* Initialize the message to send to Message Queue */
    msg_t message_to_task = {100 , "Sender_Task"};

    /* Initialize UART console */
    fsp_err_t err = console_init();
    if (FSP_SUCCESS != err)
    {
        while (1) { /* UART init failed, halt */ }
    }

    /* version get API for FLEX pack information */
    R_FSP_VersionGet (&version);

    /* Example Project information printed on the UART */
    APP_PRINT(BANNER_INFO, EP_VERSION, version.version_id_b.major, version.version_id_b.minor, version.version_id_b.patch);
    APP_PRINT (EP_INFO);

    /* The rate at which the task waits on the Message availability. */
    TickType_t Queue_wait_ticks = pdMS_TO_TICKS ( WAIT_TIME );

    /* Initialize the variable used by the call to vTaskDelayUntil(). */
    TickType_t last_execution_time = xTaskGetTickCount ();

    /* Check the task state, if in running or ready state only,
     * suspend Semaphore task. Semaphore task will be resumed after
     * Message Queue task has completed execution */
    Task_State = eTaskGetState ( semaphore_task );
    if( (eBlocked == Task_State)  ||  (eRunning == Task_State)  || (eReady == Task_State) )
    {
        vTaskSuspend (semaphore_task);
    }

    APP_PRINT ("\r\n Sender_Task : Starting g_periodic_timer_msgq timer");
    /* Start GTM timer to start sending message to Message Queue periodically at 1sec from GTM callback */
    fsp_err_t fsp_err = gtm_timer_init ( &g_periodic_timer_msgq_ctrl , &g_periodic_timer_msgq_cfg );
    /* Handle error */
    if(FSP_SUCCESS != fsp_err)
    {
        APP_ERR_TRAP(fsp_err);
    }

    while (true)
    {
        /* Check if flag is set, to suspend Message Queue task. If not set,
         * continue sending messages to Message Queue at 500ms periodically. */
        if ( true != b_suspend_msqQ_task )
        {

            /* Send message to back of Message Queue */
            if ( pdPASS == xQueueSendToBack ( g_queue , &message_to_task , Queue_wait_ticks ) )
            {
                APP_PRINT ("\r\n Sender_Task : Message posted on Queue successfully");
                APP_PRINT ("\r\n Sender_Task : Going on delay for 500ms");

                /* Places the task into the Blocked state. The task
                 * will execute every 500ms once */
                vTaskDelayUntil ( &last_execution_time , WAIT_TIME );

                APP_PRINT ("\r\n Sender_Task : After delay of 500ms\r\n");
            }
        }
        else
        {
            /* Flush the message from the Queue before suspending the task */
            if ( pdPASS == xQueueReset (g_queue) )
            {
                APP_PRINT ("\r\nSender_Task : Message Queue reset \r\n");

                /* Suspend Message Queue tasks and resume Semaphore task for Execution */
                fsp_err = R_GTM_Stop (&g_periodic_timer_msgq_ctrl);
                /* Handle error */
                if ( FSP_SUCCESS != fsp_err )
                {
                    if ( FSP_SUCCESS != R_GTM_Close (&g_periodic_timer_msgq_ctrl))
                    {
                        /* Print out in case of error */
                        APP_ERR_PRINT ("\r\nSender_Task : GTM Timer Close API failed\r\n");
                    }
                    /* Print out in case of error */
                    APP_ERR_PRINT ("\r\nSender_Task : GTM Timer stop API failed\r\n");
                    APP_ERR_TRAP (fsp_err);
                }
                APP_PRINT ("\r\nSender_Task : GTM timer stopped\r\n");

                fsp_err = R_GTM_Close (&g_periodic_timer_msgq_ctrl);
                if ( FSP_SUCCESS != fsp_err )
                {
                    /* Print out in case of error */
                    APP_ERR_PRINT ("\r\nSender_Task : GTM Timer Close API failed\r\n");
                    APP_ERR_TRAP (fsp_err);
                }

                vTaskSuspend (receiver_task);
                APP_PRINT ("\r\nSender_Task : Receiver Task Suspended\r\n");

                /* Resume semaphore task and suspend self; semaphore task will resume us for next cycle */
                vTaskResume (semaphore_task);
                APP_PRINT ("\r\nSender_Task : Semaphore Task Resumed, suspending Sender Task\r\n");

                /* Suspends calling task */
                vTaskSuspend (RESET_VALUE);

                /* --- Resumed by Semaphore Task for next cycle --- */
                g_cycle_count++;
                APP_PRINT ("\r\n=== Cycle %lu starting ===\r\n", (unsigned long)g_cycle_count);

                /* Reset flag and restart message queue timer */
                b_suspend_msqQ_task = false;
                last_execution_time = xTaskGetTickCount();

                vTaskResume (receiver_task);

                fsp_err = gtm_timer_init ( &g_periodic_timer_msgq_ctrl , &g_periodic_timer_msgq_cfg );
                if(FSP_SUCCESS != fsp_err)
                {
                    APP_ERR_TRAP(fsp_err);
                }
            }
        }
    }
}


/*******************************************************************************************************************//**
 * @brief      User defined GTM irq callback for Message Queue.
 *
 * @param[in]  timer_callback_args_t Callback function parameter data
 * @retval     None
 **********************************************************************************************************************/
void periodic_timer_msgq_cb(timer_callback_args_t *p_args)
{
    FSP_PARAMETER_NOT_USED (p_args);

    /* Counter to track task suspend count */
    static uint8_t msgQ_counter = RESET_VALUE;

    /* Skip if flagged to suspend */
    if (true == b_suspend_msqQ_task)
    {
        return;
    }

    /* Variable is set to true if priority of unblocked task is higher
     * than the task that was in running state when interrupt occurred */
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    /* Initialize the message to send to Message Queue */
    static msg_t message_to_task = {200 , "GTM Callback"};

    /* If ISR is executed for 10sec then set the flag to suspend task */
    if( TASK_SUSPEND_COUNT == msgQ_counter )
    {
        /* Set flag to suspend Message Queue tasks */
        b_suspend_msqQ_task = true;
        msgQ_counter = RESET_VALUE;
    }
    else
    {
        /* Send message to Message Queue */
        if ( pdPASS ==  xQueueSendToBackFromISR (g_queue , &message_to_task , &xHigherPriorityTaskWoken) )
        {
            /* Increment counter on successfully sending message */
            msgQ_counter++;
        }
    }
    /* A context switch will be performed if xHigherPriorityTaskWoken is equal to pdTRUE. */
    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}
