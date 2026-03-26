/*
 * Copyright (c) 2020 - 2024 Renesas Electronics Corporation and/or its affiliates
 * 
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include "hal_data.h"
#include "ca55_start.h"

/* Macro to confirm bits of CA55_RESET0 to CA55_RESET16 */
#define CA55_PRV_CPG_RSTMON0_ALLCA55_BITS                      (0x0001FFFFU)

/* Macro to confirm that 14 to 16 are in reset release state and 0 to 13 are still in reset state */
#define CA55_PRV_CPG_RSTMON0_RELEASE_CA55_14_TO_16             (0x00003FFFU)

/* Macro to confirm that all 0 to 16 are reset release */
#define CA55_PRV_CPG_RSTMON0_RELEASE_CA55_ALL                  (0x00000000U)

/* Macro to set bits of CA55_RESET0 to CA55_RESET13 */
#define CR8_PRV_CPG_RST_0_CA55_0_TO_13                         ((0x3FFFU << R_CPG_CPG_RST_0_RSTB0_WEN_Pos) | \
                                                                (0x3FFFU << R_CPG_CPG_RST_0_RSTB0_Pos))

/* Macro to set bits of CA55_RESET14 to CA55_RESET16 */
#define CR8_PRV_CPG_RST_1_CA55_14_TO_16                        ((0x7U << R_CPG_CPG_RST_1_RSTB0_WEN_Pos) | \
                                                                (0x7U << R_CPG_CPG_RST_1_RSTB0_Pos))

#define CA55_PRV_CPG_LP_CA55_CTL1_BITS_MASK                    (0x000003FFU)
#define CA55_PRV_CPG_LP_CA55_CTL1_CLUSTER_POWR_ON_REQ_START    ((0U << R_CPG_CPG_LP_CA55_CTL1_CLUSTERPACCEPT_Pos) |   \
                                                                (0x48U << R_CPG_CPG_LP_CA55_CTL1_CLUSTERPSTATE_Pos) | \
                                                                (1U << R_CPG_CPG_LP_CA55_CTL1_CLUSTERPREQ_Pos))
#define CA55_PRV_CPG_LP_CA55_CTL1_CLUSTER_POWR_ON_REQ_ACK      ((1U << R_CPG_CPG_LP_CA55_CTL1_CLUSTERPACCEPT_Pos) |   \
                                                                (0x48U << R_CPG_CPG_LP_CA55_CTL1_CLUSTERPSTATE_Pos) | \
                                                                (1U << R_CPG_CPG_LP_CA55_CTL1_CLUSTERPREQ_Pos))
#define CA55_PRV_CPG_LP_CA55_CTL1_CLUSTER_POWR_ON_REQ_STOP     ((1U << R_CPG_CPG_LP_CA55_CTL1_CLUSTERPACCEPT_Pos) |   \
                                                                (0x48U << R_CPG_CPG_LP_CA55_CTL1_CLUSTERPSTATE_Pos) | \
                                                                (0U << R_CPG_CPG_LP_CA55_CTL1_CLUSTERPREQ_Pos))
#define CA55_PRV_CPG_LP_CA55_CTL1_CLUSTER_POWR_ON_STAT         ((0U << R_CPG_CPG_LP_CA55_CTL1_CLUSTERPACCEPT_Pos) |   \
                                                                (0x48U << R_CPG_CPG_LP_CA55_CTL1_CLUSTERPSTATE_Pos) | \
                                                                (0U << R_CPG_CPG_LP_CA55_CTL1_CLUSTERPREQ_Pos))

#define CA55_PRV_CPG_LP_CA55_CTL2_CORE0_BITS_MASK              (0x000001FFU)
#define CA55_PRV_CPG_LP_CA55_CTL2_CORE0_POWER_ON_REQ_START     ((0U << R_CPG_CPG_LP_CA55_CTL2_COREPACCEPT0_Pos) |   \
                                                                (0x08U << R_CPG_CPG_LP_CA55_CTL2_COREPSTATE0_Pos) | \
                                                                (1U << R_CPG_CPG_LP_CA55_CTL2_COREPREQ0_Pos))
#define CA55_PRV_CPG_LP_CA55_CTL2_CORE0_POWER_ON_REQ_ACK       ((1U << R_CPG_CPG_LP_CA55_CTL2_COREPACCEPT0_Pos) |   \
                                                                (0x08U << R_CPG_CPG_LP_CA55_CTL2_COREPSTATE0_Pos) | \
                                                                (1U << R_CPG_CPG_LP_CA55_CTL2_COREPREQ0_Pos))
#define CA55_PRV_CPG_LP_CA55_CTL2_CORE0_POWER_ON_REQ_STOP      ((1U << R_CPG_CPG_LP_CA55_CTL2_COREPACCEPT0_Pos) |   \
                                                                (0x08U << R_CPG_CPG_LP_CA55_CTL2_COREPSTATE0_Pos) | \
                                                                (0U << R_CPG_CPG_LP_CA55_CTL2_COREPREQ0_Pos))
#define CA55_PRV_CPG_LP_CA55_CTL2_CORE0_POWER_ON_STAT          ((0U << R_CPG_CPG_LP_CA55_CTL2_COREPACCEPT0_Pos) |   \
                                                                (0x08U << R_CPG_CPG_LP_CA55_CTL2_COREPSTATE0_Pos) | \
                                                                (0U << R_CPG_CPG_LP_CA55_CTL2_COREPREQ0_Pos))

void release_reset_ca55 (void)
{
    /* Set CA55 Core0 Reset vector address. */
    R_SYSC->SYS_ACPU_CFG_RVAL0 = CA55_RESET_VECTOR_ADDRESS;

    /* 1: Confirm that all signals of CA55 are in the reset state. */
    FSP_HARDWARE_REGISTER_WAIT((CA55_PRV_CPG_RSTMON0_ALLCA55_BITS & R_CPG->CPG_RSTMON_0),
                               CA55_PRV_CPG_RSTMON0_ALLCA55_BITS);

    /* 2: Request output "Transition destination power mode for Cluster : ON" */
    R_CPG->CPG_LP_CA55_CTL1 = CA55_PRV_CPG_LP_CA55_CTL1_CLUSTER_POWR_ON_REQ_START;

    /* 3: Request output "Transition destination power mode for core 0 : ON" */
    R_CPG->CPG_LP_CA55_CTL2 = CA55_PRV_CPG_LP_CA55_CTL2_CORE0_POWER_ON_REQ_START;

    /* 4: Release reset of CA55_RESET14-16 */
    R_CPG->CPG_RST_1 = CR8_PRV_CPG_RST_1_CA55_14_TO_16;

    /* 5: Wait until released reset of CA55_RESET14-16 */
    FSP_HARDWARE_REGISTER_WAIT((CA55_PRV_CPG_RSTMON0_ALLCA55_BITS & R_CPG->CPG_RSTMON_0),
                               CA55_PRV_CPG_RSTMON0_RELEASE_CA55_14_TO_16);

    /* 6: Release reset of CA55_RESET0-13 */
    R_CPG->CPG_RST_0 = CR8_PRV_CPG_RST_0_CA55_0_TO_13;

    /* 7: Wait until released reset of CA55_RESET0-13 */
    FSP_HARDWARE_REGISTER_WAIT((CA55_PRV_CPG_RSTMON0_ALLCA55_BITS & R_CPG->CPG_RSTMON_0),
                               CA55_PRV_CPG_RSTMON0_RELEASE_CA55_ALL);

    /* 8: Wait until "Power mode transition acceptance for Cluster : Transition accepted" */
    FSP_HARDWARE_REGISTER_WAIT((CA55_PRV_CPG_LP_CA55_CTL1_BITS_MASK & R_CPG->CPG_LP_CA55_CTL1),
                               CA55_PRV_CPG_LP_CA55_CTL1_CLUSTER_POWR_ON_REQ_ACK);

    /* 9: Wait until "Power mode transition acceptance for core 0 : Transition accepted" */
    FSP_HARDWARE_REGISTER_WAIT((CA55_PRV_CPG_LP_CA55_CTL2_CORE0_BITS_MASK & R_CPG->CPG_LP_CA55_CTL2),
                               CA55_PRV_CPG_LP_CA55_CTL2_CORE0_POWER_ON_REQ_ACK);

    /* 10: Request stopped "Power mode transition request control for Cluster" */
    R_CPG->CPG_LP_CA55_CTL1 = CA55_PRV_CPG_LP_CA55_CTL1_CLUSTER_POWR_ON_REQ_STOP;

    /* 11: Wait until "Power mode transition acceptance for Cluster : Transition accepted" */
    FSP_HARDWARE_REGISTER_WAIT((CA55_PRV_CPG_LP_CA55_CTL1_BITS_MASK & R_CPG->CPG_LP_CA55_CTL1),
                               CA55_PRV_CPG_LP_CA55_CTL1_CLUSTER_POWR_ON_STAT);

    /* 12: Request stopped "Power mode transition request control for core 0" */
    R_CPG->CPG_LP_CA55_CTL2 = CA55_PRV_CPG_LP_CA55_CTL2_CORE0_POWER_ON_REQ_STOP;

    /* 13: Wait until "Power mode transition acceptance for core 0 : Transition accepted" */
    FSP_HARDWARE_REGISTER_WAIT((CA55_PRV_CPG_LP_CA55_CTL2_CORE0_BITS_MASK & R_CPG->CPG_LP_CA55_CTL2),
                               CA55_PRV_CPG_LP_CA55_CTL2_CORE0_POWER_ON_STAT);

    /* Release reset of SYC_0_RESET */
    R_BSP_MODULE_RSTOFF(FSP_IP_SYC, 0);
}

void load_ca55_prog (void)
{
    memcpy((void *) (CA55_RESET_VECTOR_ADDRESS),
           (void *) (RZV2H_SPIROM_BL2_BASE + BL2_BOOTPARM_SIZE),
           RZV2H_BL2_SIZE_MAX);
}
