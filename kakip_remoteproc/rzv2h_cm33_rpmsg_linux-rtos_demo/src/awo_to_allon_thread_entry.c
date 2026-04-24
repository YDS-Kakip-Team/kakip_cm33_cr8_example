/*
* Copyright (c) 2020 - 2024 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
*/

#include "hal_data.h"
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "awo_to_allon_thread_entry.h"
#include "pd_axi_on.h"
#include "i2c_pmic.h"
#include "pll_init.h"
#include "xspi_init.h"
#include "ca55_start.h"
#include "cr8_start.h"

uint32_t g_is_pd_allon = 0;

/* AWO to ALLON Thread entry function */
void awo_to_allon_thread_entry (__attribute__((unused)) void * pvParameters)
{
#if defined(BSP_FEATURE_BSP_HAS_CM33BOOT_SUPPORT) && (BSP_FEATURE_BSP_HAS_CM33BOOT_SUPPORT == 1)
    pd_all_on_preproc();

    /* Initialization for CM33 coldboot */
    if (R_SYSC->SYS_LSI_MODE_b.STAT_BOOTSELECTER == 0)
    {
        /* PD_AWO -> PD_ALL_ON */
        pd_all_on();
    }
#endif

    /* Allow access to IP beyond AXI */
    pd_all_on_postproc_axi();

#if defined(BSP_FEATURE_BSP_HAS_CM33BOOT_SUPPORT) && (BSP_FEATURE_BSP_HAS_CM33BOOT_SUPPORT == 1)
    pd_all_on_postproc();

 #if BSP_CFG_MCU_CA55_CLOCK_UP && !BSP_CFG_MCU_LAUNCH_CA55

    /* reset of CA55(Core0) */
    assert_reset_ca55();

    /* set the clock frequency of CA55(core0) to 1.8GHz */
    pll_reboot_ca55_at_1_8GHz();
 #endif
 #if BSP_CFG_MCU_CLOCK_INIT
    pll_init_other();
 #endif

    xspi_open();

 #if BSP_CFG_MCU_LAUNCH_CR8

    /* Release reset of CR8(Core0, Core1) and load dummy program to SRAM(RCPU) */
    release_reset_cr8();
 #endif
 #if BSP_CFG_MCU_LAUNCH_CA55

    /* Load CA55 program to SRAM(ACPU) */
    load_ca55_prog();

    xspi_close();

    /* Release reset of CA55(Core0) */
    release_reset_ca55();
 #elif BSP_CFG_MCU_CA55_CLOCK_UP

    /* Release reset of CA55(Core0) */
    release_reset_ca55();
 #endif
#endif

    g_is_pd_allon = 1;

    vTaskDelete(NULL);
}
