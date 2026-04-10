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

#define CONSOLE_BUF_SIZE  (256u)
#define CONSOLE_TIMEOUT   (UINT32_MAX)

/* Flag for user callback */
static volatile uint8_t g_uart_event = RESET_VALUE;

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
    }
}
