/**
  ******************************************************************************
  * File Name          : SPI.c
  * Description        : This file provides code for the configuration
  *                      of the SPI instances.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "spi.h"

extern osThreadId_t TID_RC522;

extern ARM_DRIVER_SPI Driver_SPI3;
ARM_DRIVER_SPI* SPI3drv = &Driver_SPI3;

/**
 * Inicializa la interfaz SPI3 para el módulo RC522.
 * Configura los parámetros de la interfaz SPI y controla el pin de reinicio (RST).
 */
void RC522_SPI3_Init(void)
{
	SPI3drv -> Initialize(SPI_callback);
	SPI3drv -> PowerControl(ARM_POWER_FULL);
	SPI3drv -> Control(ARM_SPI_MODE_MASTER | ARM_SPI_CPOL0_CPHA0 | ARM_SPI_MSB_LSB | ARM_SPI_DATA_BITS(8), 1000000);

	HAL_GPIO_WritePin(RC522_RST_PORT, RC522_RST_PIN, GPIO_PIN_RESET);
  osDelay(1);
  HAL_GPIO_WritePin(RC522_RST_PORT, RC522_RST_PIN, GPIO_PIN_SET);
  osDelay(1000);
}

/**
 * Callback de evento de la interfaz SPI.
 * Maneja los eventos de la interfaz SPI3, como la transferencia completa de datos.
 * @param event Evento de la interfaz SPI.
 */
static void SPI_callback(uint32_t event){
    switch (event) {
    case ARM_SPI_EVENT_TRANSFER_COMPLETE:
        osThreadFlagsSet(TID_RC522, SPI3_transfer_complete);
        break;
    case ARM_SPI_EVENT_DATA_LOST:
        /*  Occurs in slave mode when data is requested/sent by master
            but send/receive/transfer operation has not been started
            and indicates that data is lost. Occurs also in master mode
            when driver cannot transfer data fast enough. */
        __breakpoint(0);  /* Error: Call debugger or replace with custom error handling */
        break;
    case ARM_SPI_EVENT_MODE_FAULT:
        /*  Occurs in master mode when Slave Select is deactivated and
            indicates Master Mode Fault. */
        __breakpoint(0);  /* Error: Call debugger or replace with custom error handling */
        break;
    }
}
