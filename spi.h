/**
  ******************************************************************************
  * File Name          : SPI.h
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
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SPI_H
#define __SPI_H

/* Includes ------------------------------------------------------------------*/
#include "RTE_Device.h"
#include "Driver_SPI.h"
#include "stm32f4xx_hal.h"
#include "cmsis_os2.h" s
#include <stdio.h>
#include <stdbool.h>
#include "string.h"
#include "RC522.h"
#include "gpio.h"


void SPI3_Init(void);
static void SPI_callback(uint32_t event);

#endif
