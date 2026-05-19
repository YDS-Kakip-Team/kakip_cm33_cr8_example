/***********************************************************************************************************************
 * File Name    : console.c
 * Description  : UART console output utility for Kakip MCU examples.
 **********************************************************************************************************************/
/*
 * Copyright (c) 2020 - 2024 Renesas Electronics Corporation and/or its affiliates
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "console.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include <stdbool.h>

#define CONSOLE_BUF_SIZE  (256u)
#define CONSOLE_TIMEOUT   (UINT32_MAX)
#define RESET_VALUE       (0u)

#define RX_RING_SIZE      (64u)

/* Flag for user callback */
static volatile uint8_t g_uart_event = RESET_VALUE;

/* RX ring buffer */
static volatile uint8_t g_rx_ring[RX_RING_SIZE];
static volatile uint32_t g_rx_head = 0;
static volatile uint32_t g_rx_tail = 0;

/*******************************************************************************************************************//**
 * @brief       Initialize UART console.
 * @retval      FSP_SUCCESS     Upon successful open
 * @retval      Any Other Error code apart from FSP_SUCCESS
 **********************************************************************************************************************/
fsp_err_t console_init(void)
{
    fsp_err_t err = R_SCI_B_UART_Open(&g_uart_ctrl, &g_uart_cfg);
    if (FSP_SUCCESS != err)
    {
        return err;
    }
    return FSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief       Print formatted string to UART console.
 * @param[in]   fmt    Format string (printf-style)
 **********************************************************************************************************************/
void console_print(const char *fmt, ...)
{
    char buf[CONSOLE_BUF_SIZE];
    va_list args;
    uint64_t timeout;

    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    uint8_t len = (uint8_t)strlen(buf);
    if (len == 0)
    {
        return;
    }

    g_uart_event = RESET_VALUE;
    fsp_err_t err = R_SCI_B_UART_Write(&g_uart_ctrl, (uint8_t *)buf, len);
    if (FSP_SUCCESS != err)
    {
        return;
    }

    /* Wait for transfer complete */
    timeout = CONSOLE_TIMEOUT;
    while ((UART_EVENT_TX_COMPLETE != g_uart_event) && (--timeout))
    {
        /* wait */
    }
}

/*******************************************************************************************************************//**
 * @brief       Check if RX data is available.
 * @retval      true    Data available
 * @retval      false   No data
 **********************************************************************************************************************/
bool console_has_data(void)
{
    return (g_rx_head != g_rx_tail);
}

/*******************************************************************************************************************//**
 * @brief       Read received data from RX ring buffer.
 * @param[in]   buf    Buffer to store data
 * @param[in]   len    Maximum bytes to read
 * @retval      Number of bytes actually read
 **********************************************************************************************************************/
uint32_t console_read(uint8_t *buf, uint32_t len)
{
    uint32_t count = 0;
    while ((count < len) && (g_rx_head != g_rx_tail))
    {
        buf[count++] = (uint8_t)g_rx_ring[g_rx_tail];
        g_rx_tail = (g_rx_tail + 1u) % RX_RING_SIZE;
    }
    return count;
}

/*******************************************************************************************************************//**
 * @brief       Deinitialize UART console.
 **********************************************************************************************************************/
void console_deinit(void)
{
    R_SCI_B_UART_Close(&g_uart_ctrl);
}

/*******************************************************************************************************************//**
 * @brief       UART callback for console. Set this in FSP configuration.
 **********************************************************************************************************************/
void g_uart_callback(uart_callback_args_t *p_args)
{
    if (NULL != p_args)
    {
        g_uart_event = (uint8_t)p_args->event;

        if (UART_EVENT_RX_CHAR == p_args->event)
        {
            uint32_t next = (g_rx_head + 1u) % RX_RING_SIZE;
            if (next != g_rx_tail)
            {
                g_rx_ring[g_rx_head] = (uint8_t)p_args->data;
                g_rx_head = next;
            }
        }
    }
}
