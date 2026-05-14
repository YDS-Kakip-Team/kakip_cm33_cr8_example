/***********************************************************************************************************************
 * File Name    : console.h
 * Description  : UART console output utility for Kakip MCU examples.
 **********************************************************************************************************************/
/*
 * Copyright (c) 2020 - 2024 Renesas Electronics Corporation and/or its affiliates
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CONSOLE_H_
#define CONSOLE_H_

#include "hal_data.h"

fsp_err_t console_init(void);
void console_print(const char *fmt, ...);
bool console_has_data(void);
uint32_t console_read(uint8_t *buf, uint32_t len);
void console_deinit(void);

#endif /* CONSOLE_H_ */
