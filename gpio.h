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

/**SPI3 GPIO Configuration
*    PA15     ------> SPI3_NSS o SPI3_CS
*    PF12    ------> SPI3_RESET
*/
#define RC522_CS_PIN         	GPIO_PIN_15
#define RC522_CS_PORT     		GPIOA 
#define RC522_RST_PIN      		GPIO_PIN_12
#define RC522_RST_PORT     		GPIOF 

void RC522_GPIO_Init(void);

#endif
