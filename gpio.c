/**
  ******************************************************************************
  * File Name          : gpio.c
  * Description        : This file provides code for the configuration
  *                      of all used GPIO pins.
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
#include "gpio.h"

/**
 * Inicializa los pines GPIO necesarios para el funcionamiento del módulo RC522.
 */
void RC522_GPIO_Init(void)
{
  static GPIO_InitTypeDef GPIO_InitStruct_RC522;
	
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  
	
  __HAL_RCC_GPIOA_CLK_ENABLE();
  GPIO_InitStruct_RC522.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct_RC522.Pull = GPIO_PULLUP;
  GPIO_InitStruct_RC522.Speed = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct_RC522.Pin = GPIO_PIN_15;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct_RC522);
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, GPIO_PIN_SET);
  
  __HAL_RCC_GPIOB_CLK_ENABLE();
  GPIO_InitStruct_RC522.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct_RC522.Pull = GPIO_PULLUP;
  GPIO_InitStruct_RC522.Speed = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct_RC522.Pin = GPIO_PIN_4;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct_RC522);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_SET);
	
  __HAL_RCC_GPIOF_CLK_ENABLE();
  GPIO_InitStruct_RC522.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct_RC522.Pull = GPIO_PULLUP;
  GPIO_InitStruct_RC522.Speed = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct_RC522.Pin = GPIO_PIN_12;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct_RC522);
  HAL_GPIO_WritePin(GPIOF, GPIO_PIN_12, GPIO_PIN_SET);


}
