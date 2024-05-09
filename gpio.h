/**
  ******************************************************************************
  * File Name          : gpio.h
  * Description        : This file contains all the functions prototypes for
  *                      the gpio
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
#ifndef __GPIO_H
#define __GPIO_H


/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Defines -------------------------------------------------------------------*/
/**SPI3 GPIO Configuration
*    PB4     ------> SPI3_MISO
*    PC12    ------> SPI3_MOSI
*    PB3     ------> SPI3_SCK
*/
#define SCK_PIN         	GPIO_PIN_3
#define SCK_PORT    			GPIOB
#define MISO_PIN      		GPIO_PIN_4
#define MISO_PORT   			GPIOB
#define MOSI_PIN         	GPIO_PIN_12
#define MOSI_PORT        	GPIOC 

/**Aditional SPI GPIO Configuration
*    PF12    ------> RST_RC522
*    PA15    ------> nCS_RC522
*    PG0     ------> nCS_W25Q32
*/
#define RST_RC522_PIN     GPIO_PIN_12
#define RST_RC522_PORT    GPIOF
#define CS_RC522_PIN      GPIO_PIN_15
#define CS_RC522_PORT     GPIOA
#define CS_W25Q32_PIN     GPIO_PIN_0
#define CS_W25Q32_PORT    GPIOG

void GPIO_SPI_Init(void);

#endif
