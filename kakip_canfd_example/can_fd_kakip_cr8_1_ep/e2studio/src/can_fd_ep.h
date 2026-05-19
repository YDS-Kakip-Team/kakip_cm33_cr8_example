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
#define WAIT_TIME                       (500U)             //Wait time value
#define EP_INFO                        "\n This Example Project demonstrates CAN FD operations on Renesas RZ MCUs using 1 RZ/V board."\
                                       "\nOn pressing any key on the RTT Viewer, data is transmitted from one channel to other."\
                                       "\nOn 1st transmission, Channel 0 transmits data to Channel 3. Channel 3 displays the received data."\
                                       "\nOn 2nd transmission, Channel 3 transmits updated data to Channel 0 as ACK. Channel 0 displays the received data,"\
                                       "\nthen Channel 0 changes CAN frame to CANFD frame and updates data."\
                                       "\nOn 3rd transmission, Channel 0 transmits updated data to Channel 3. Channel 3 displays the received data,"\
                                       "\nthen Channel 3 changes CAN frame to CANFD frame and updates data to transmits back to Channel 0 as ACK."\
                                       "\nOn 4th transmission, Channel 3 transmits updated data to Channel 0 as ACK. Channel 0 displays the received data.\n"

void canfd_operation(void);
void can_read_operation(void);
void canfd_deinit(void);


#endif /* CAN_FD_EP_H_ */
