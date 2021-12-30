// This example is for Nucleo-F411 board connecting with AHT10 Module by I2C1 @ PB8&PB9
// Send out reading data using UART2 port; can be received by Arduino Serial Port Monitor...
// State Machine of AHT10 @ AHT10.graphml(yEd)
//                                                  2021 Dec, by Frank Hsiung

#include "main.h"
#include <string.h>
#include <stdio.h>

I2C_HandleTypeDef hi2c1;
UART_HandleTypeDef huart2;
HAL_StatusTypeDef status;

const char *statusArray[] = {"HAL_OK", "HAL_ERROR", "HAL_BUSY", "HAL_TIMEOUT"};
const uint32_t tickWait = 1000; // read data every tickWait mini-second

static uint32_t tickEnd;                           // for State Machine Time-out
static uint8_t buf_UART[64];                       // report information through UART2
static uint8_t buf_I2C_Tx[3] = {0xE1, 0x08, 0x00}; // Initialization: datasheet is not clear here!
static uint8_t buf_I2C_Rx[6];                      // Rx Buffer for UART Communicastion

void s_AHT10_Init(void); // state machine
void s_AHT10_InitDelay(void);
void s_AHT10_IssueMeasureCmd(void);
void s_AHT10_MeasurementDelay(void);
void (*ps_AHT10)(void); // state machine functional pointer of AHT10

void s_AHT10_Init(void)
{
    status = HAL_I2C_Master_Transmit(&hi2c1, 0x70, buf_I2C_Tx, 3, HAL_MAX_DELAY); // Issue Init Command

    strcpy((char *)buf_UART, statusArray[status]); // Report Initialization Status
    HAL_UART_Transmit(&huart2, buf_UART, strlen((char *)buf_UART), HAL_MAX_DELAY);
    strcpy((char *)buf_UART, " @ s_AHT10_Init State\r\n\0");
    HAL_UART_Transmit(&huart2, buf_UART, strlen((char *)buf_UART), HAL_MAX_DELAY);

    tickEnd = HAL_GetTick() + tickWait; // set None Blocking Delay for Initialization
    ps_AHT10 = s_AHT10_InitDelay;       // change state
}

void s_AHT10_InitDelay(void)
{
    if (HAL_GetTick() > tickEnd)
    {
        strcpy((char *)buf_UART, "Start Reading AHT10 Data\r\n\0");
        HAL_UART_Transmit(&huart2, buf_UART, strlen((char *)buf_UART), HAL_MAX_DELAY);

        buf_I2C_Tx[0] = 0xAC;               // rewrite Measurement Command Buffer
        buf_I2C_Tx[1] = 0x33;               // set once only
        ps_AHT10 = s_AHT10_IssueMeasureCmd; // change state
    }
}

void s_AHT10_IssueMeasureCmd(void)
{ // issue measurement command
    status = HAL_I2C_Master_Transmit(&hi2c1, 0x70, buf_I2C_Tx, 3, HAL_MAX_DELAY);
    if (status != HAL_OK)
    {
        strcpy((char *)buf_UART, statusArray[status]);
        HAL_UART_Transmit(&huart2, buf_UART, strlen((char *)buf_UART), HAL_MAX_DELAY);
        strcpy((char *)buf_UART, " @ s_AHT10_IssueMeasureCmd State\r\n\0");
        HAL_UART_Transmit(&huart2, buf_UART, strlen((char *)buf_UART), HAL_MAX_DELAY);
    }
    else
    {
        tickEnd = HAL_GetTick() + tickWait;  // None Blocking Delay
        ps_AHT10 = s_AHT10_MeasurementDelay; // change state
    }
}

void s_AHT10_MeasurementDelay(void)
{
    if (HAL_GetTick() > tickEnd)
    {
        status = HAL_I2C_Master_Receive(&hi2c1, 0x70, buf_I2C_Rx, 6, HAL_MAX_DELAY);
        if (status == HAL_OK)
        { // ------------------------- calculate Temperature ---------------------------------------
            strcpy((char *)buf_UART, " ％RH\r\nTemperature = \0");
            HAL_UART_Transmit(&huart2, buf_UART, strlen((char *)buf_UART), HAL_MAX_DELAY);
            // restructure to 20-bit data
            uint32_t x = ((uint32_t)(buf_I2C_Rx[3] & 0x0F) << 16) | ((uint32_t)buf_I2C_Rx[4] << 8) | buf_I2C_Rx[5];
            float tmp = (float)x * 0.000191 - 50;   // formula in datasheet
            sprintf((char *)buf_UART, "%.2f", tmp); // output truncation
            HAL_UART_Transmit(&huart2, buf_UART, strlen((char *)buf_UART), HAL_MAX_DELAY);
            // ----------------------- calculate Humidity ------------------------------------------
            strcpy((char *)buf_UART, " ℃\tHumidity = \0");
            HAL_UART_Transmit(&huart2, buf_UART, strlen((char *)buf_UART), HAL_MAX_DELAY);
            // restructure to 20-bit data
            x = ((uint32_t)buf_I2C_Rx[1] << 12) | ((uint32_t)buf_I2C_Rx[2] << 4) | (buf_I2C_Rx[3] >> 4);
            tmp = (float)x * 0.000095;              // formula in datasheet
            sprintf((char *)buf_UART, "%.2f", tmp); // output truncation
            HAL_UART_Transmit(&huart2, buf_UART, strlen((char *)buf_UART), HAL_MAX_DELAY);

            ps_AHT10 = s_AHT10_IssueMeasureCmd; // change state
        }
        else
        { // report exception status
            strcpy((char *)buf_UART, statusArray[status]);
            HAL_UART_Transmit(&huart2, buf_UART, strlen((char *)buf_UART), HAL_MAX_DELAY);
            strcpy((char *)buf_UART, " @ s_AHT10_MeasurementDelay State\r\n\0");
            HAL_UART_Transmit(&huart2, buf_UART, strlen((char *)buf_UART), HAL_MAX_DELAY);
        }
    }
}
