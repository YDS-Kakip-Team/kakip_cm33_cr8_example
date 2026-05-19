/***********************************************************************************************************************
 * File Name    : can_fd_ep.h
 * Description  : Contains declarations of data structures and functions used in hal_entry.c.
 **********************************************************************************************************************/
/*
 * Copyright (c) 2020 - 2024 Renesas Electronics Corporation and/or its affiliates
 * 
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CAN_FD_EP_H_
#define CAN_FD_EP_H_

#define CAN_MAILBOX_NUMBER_0            (0U)               //mail box number
#define CAN_CLASSIC_FRAME_DATA_BYTES    (8U)               //Data Length code for classic frame
#define CAN_FD_DATA_LENGTH_CODE         (16)               //Data Length code for FD frame
#define CAN_ID                          (0x1100)           //ID to be updated in transmit frame
/* Acceptance filter array parameters */
#define CANFD_FILTER_ID                 (0x00001000)
#define MASK_ID                         (0x1FFFF000)
#define MASK_ID_MODE                    (1)
#undef BUFFER_SIZE_UP
#define BUFFER_SIZE_UP                  (2048)
#define ZERO                            (0U)               //Array Index value
#define NULL_CHAR                       ('\0')             //MACRO for null character
#define WAIT_TIME                       (500000U)             //Wait time value
#define EP_INFO                        "\nCAN FD Example: Press any key to start CAN transmission."\
                                       "\nCH0 <-> CH3: Classic CAN (up to 8B) + CAN FD (up to 64B).\n"

void canfd_operation(void);
void can_read_operation(void);
void canfd_deinit(void);


#endif /* CAN_FD_EP_H_ */
