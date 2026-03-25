/*
 * Copyright (c) 2020 - 2024 Renesas Electronics Corporation and/or its affiliates
 * 
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "i2c_pmic.h"

/*******************************************************************************************************************//**
 * @addtogroup r_riic_master_pmic
 * @{
 **********************************************************************************************************************/

#if defined(BSP_FEATURE_BSP_HAS_CM33BOOT_SUPPORT) && (BSP_FEATURE_BSP_HAS_CM33BOOT_SUPPORT == 1)

/*
 * Global Variables
 */

/* Reading I2C call back event through i2c_Master callback */
static volatile i2c_master_event_t i2c_event = I2C_MASTER_EVENT_ABORTED;

/*
 * private function declarations
 */
static fsp_err_t pmic_read_reg(uint8_t reg_address, uint8_t * reg_data);
static fsp_err_t pmic_write_reg(uint8_t reg_address, uint8_t reg_data);
static fsp_err_t validate_i2c_event(void);

/* PMIC Register Definitions */
 #define PMIC_HW_REV_ADDR          (0xF8)
 #define PMIC_MPIO3_CONFIG_ADDR    (0x8D)
 #define PMIC_MPIO3_CONFIG_DATA    (0x00)
 #define PMIC_BLOCK_EN_ADDR        (0x6C)

/*******************************************************************************************************************//**
 *  @brief       Change PMIC status from SLEEP to ACTIVE
 *  @param[IN]   None
 *  @retval      FSP_SUCCESS                  Successful transition to ACTIVE
 *  @retval      Any Other Error code apart from FSP_SUCCESS is  Unsuccessful open or start
 **********************************************************************************************************************/
fsp_err_t pmic_sleep_to_active (void)
{
    uint8_t   reg_address;
    uint8_t   reg_data = RESET_VALUE;
    fsp_err_t err      = FSP_SUCCESS;

    /* opening RIIC master module */
    g_i2c_master0.p_api->open(g_i2c_master0.p_ctrl, g_i2c_master0.p_cfg);

    /* handle error */
    if (FSP_SUCCESS != err)
    {
        APP_ERR_PRINT("** R_RIIC_MASTER_Open API failed ** \r\n");

        return err;
    }

    reg_address = PMIC_HW_REV_ADDR;
    pmic_read_reg(reg_address, &reg_data);
    APP_PRINT("HW REV addr:%x, data:%x\r\n", reg_address, reg_data);

    reg_address = PMIC_MPIO3_CONFIG_ADDR;
    pmic_read_reg(reg_address, &reg_data);
    APP_PRINT("MPIO3 Config addr:%x, data:%x\r\n", reg_address, reg_data);

    reg_address = PMIC_BLOCK_EN_ADDR;
    pmic_read_reg(reg_address, &reg_data);
    APP_PRINT("Block EN addr:%x, data:%x\r\n", reg_address, reg_data);

    APP_PRINT("Write to MPIO3 Config\r\n");

    reg_address = PMIC_MPIO3_CONFIG_ADDR;
    reg_data    = PMIC_MPIO3_CONFIG_DATA;
    pmic_write_reg(reg_address, reg_data);

    reg_address = PMIC_MPIO3_CONFIG_ADDR;
    pmic_read_reg(reg_address, &reg_data);
    APP_PRINT("MPIO3 Config addr:%x, data:%x\r\n", reg_address, reg_data);

    /* close open modules */
    g_i2c_master0.p_api->close(g_i2c_master0.p_ctrl);

    if (FSP_SUCCESS != err)
    {
        APP_ERR_PRINT("** R_RIIC_MASTER_Close API failed ** \r\n");
    }

    return err;
}

/*******************************************************************************************************************//**
 *  @brief     Read PMIC Device Register
 *
 *  @param[in] reg_address                  register address to read
 *  @param[in] reg_data
 *
 *  @retval    FSP_SUCCESS                  on successful I2C transaction
 *             FSP_ERR_INVALID_POINTER      if invalid parameters passed
 *             err                          whichever occurs on either API or on I2C event failure
 **********************************************************************************************************************/
static fsp_err_t pmic_read_reg (uint8_t reg_address, uint8_t * reg_data)
{
    fsp_err_t err = FSP_SUCCESS;

    /* Parameter checking */
    if (NULL == reg_data)
    {
        err = FSP_ERR_INVALID_POINTER;
        APP_ERR_PRINT("** NULL Pointer check fail ** \r\n");

        return err;
    }

    /* Send register address to PMIC */
    err = g_i2c_master0.p_api->write(g_i2c_master0.p_ctrl, &reg_address, ONE_BYTE, true);

    /* handle error */
    if (FSP_SUCCESS != err)
    {
        APP_ERR_PRINT("** R_RIIC_MASTER_Write API failed ** \r\n");
    }
    else
    {
        err = validate_i2c_event();

        /* handle error */
        if (FSP_ERR_TRANSFER_ABORTED == err)
        {
            APP_ERR_PRINT("** Device register address, I2C write failed ** \r\n");
        }
        else
        {
            /* Read only when I2C write and its callback event is successful */
            err = g_i2c_master0.p_api->read(g_i2c_master0.p_ctrl, reg_data, ONE_BYTE, false);

            /* handle error */
            if (FSP_SUCCESS != err)
            {
                APP_ERR_PRINT("** R_RIIC_MASTER_Read API failed ** \r\n");

                // Do nothing, the error is returned in the end
            }
            else
            {
                err = validate_i2c_event();

                /* handle error */
                if (FSP_ERR_TRANSFER_ABORTED == err)
                {
                    APP_ERR_PRINT("** Device register read,  I2C read failed ** \r\n");
                }
            }
        }
    }

    /* On successful I2C transaction with PMOD-ACL return from here */
    return err;
}

/*******************************************************************************************************************//**
 *  @brief     Write PMIC Device Register
 *
 *  @param[in] reg_address                  register address to read
 *  @param[in] reg_data
 *
 *  @retval    FSP_SUCCESS                  on successful I2C transaction
 *             FSP_ERR_INVALID_POINTER      if invalid parameters passed
 *             err                          whichever occurs on either API or on I2C event failure
 **********************************************************************************************************************/
static fsp_err_t pmic_write_reg (uint8_t reg_address, uint8_t reg_data)
{
    fsp_err_t err = FSP_SUCCESS;
    uint8_t   buf[2];

    buf[0] = reg_address;
    buf[1] = reg_data;

    /* Send register address to PMIC */
    err = g_i2c_master0.p_api->write(g_i2c_master0.p_ctrl, &buf[0], 2, false);

    /* handle error */
    if (FSP_SUCCESS != err)
    {
        APP_ERR_PRINT("** R_RIIC_MASTER_Write API failed ** \r\n");
    }
    else
    {
        err = validate_i2c_event();

        /* handle error */
        if (FSP_ERR_TRANSFER_ABORTED == err)
        {
            APP_ERR_PRINT("** Device register address, I2C write failed ** \r\n");
        }
    }

    /* On successful I2C transaction with PMOD-ACL return from here */
    return err;
}

/*******************************************************************************************************************//**
 *  @brief      User callback function
 *  @param[in]  p_args
 *  @retval None
 **********************************************************************************************************************/
void i2c_master_callback (i2c_master_callback_args_t * p_args)
{
    if (NULL != p_args)
    {
        /* capture callback event for validating the i2c transfer event*/
        i2c_event = p_args->event;
    }
}

/*******************************************************************************************************************//**
 *  @brief       Validate i2c receive/transmit  based on required write read operation
 *
 *  @param[in]   None
 *
 *  @retval      FSP_SUCCESS                       successful event receiving returns FSP_SUCCESS
 *               FSP_ERR_TRANSFER_ABORTED          Either on timeout elapsed or received callback event is
 *                                                 I2C_MASTER_EVENT_ABORTED
 **********************************************************************************************************************/
static fsp_err_t validate_i2c_event (void)
{
    uint64_t local_time_out = UINT64_MAX;

    /* resetting call back event capture variable */
    i2c_event = (i2c_master_event_t) RESET_VALUE;

    do
    {
        /* This is to avoid infinite  loop */
        --local_time_out;

        if (RESET_VALUE == local_time_out)
        {
            return FSP_ERR_TRANSFER_ABORTED;
        }
    } while (i2c_event == RESET_VALUE);

    if (i2c_event != I2C_MASTER_EVENT_ABORTED)
    {
        i2c_event = (i2c_master_event_t) RESET_VALUE; // Make sure this is always Reset before return
        return FSP_SUCCESS;
    }

    i2c_event = (i2c_master_event_t) RESET_VALUE;     // Make sure this is always Reset before return
    return FSP_ERR_TRANSFER_ABORTED;
}

#endif                                                /*defined(BSP_FEATURE_BSP_HAS_CM33BOOT_SUPPORT) && (BSP_FEATURE_BSP_HAS_CM33BOOT_SUPPORT == 1)*/

/*******************************************************************************************************************//**
 * @} (end addtogroup r_riic_master_pmic)
 **********************************************************************************************************************/
