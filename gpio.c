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
#include "spi.h"

void GPIO_SPI_Init(void){
  
  static GPIO_InitTypeDef GPIO_InitStruct_SPI;
	
  __HAL_RCC_GPIOF_CLK_ENABLE();
  GPIO_InitStruct_SPI.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct_SPI.Pull = GPIO_PULLUP;
  GPIO_InitStruct_SPI.Speed = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct_SPI.Pin = RST_RC522_PIN;
  HAL_GPIO_Init(RST_RC522_PORT, &GPIO_InitStruct_SPI);
  HAL_GPIO_WritePin(RST_RC522_PORT, RST_RC522_PIN, GPIO_PIN_SET);
	
	/*nCS RFID*/    //PA15
	__HAL_RCC_GPIOA_CLK_ENABLE();
  GPIO_InitStruct_SPI.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct_SPI.Pull = GPIO_PULLUP;
  GPIO_InitStruct_SPI.Speed = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct_SPI.Pin = CS_RC522_PIN;
  HAL_GPIO_Init(CS_RC522_PORT, &GPIO_InitStruct_SPI);
  HAL_GPIO_WritePin(CS_RC522_PORT, CS_RC522_PIN, GPIO_PIN_SET);
	
  /*nCS FLASH*/    //PG0
  __HAL_RCC_GPIOG_CLK_ENABLE();
  GPIO_InitStruct_SPI.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct_SPI.Pull = GPIO_PULLUP;
      GPIO_InitStruct_SPI.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct_SPI.Pin = CS_W25Q32_PIN;
  HAL_GPIO_Init(CS_W25Q32_PORT, &GPIO_InitStruct_SPI);
  HAL_GPIO_WritePin(CS_W25Q32_PORT, CS_W25Q32_PIN, GPIO_PIN_SET);
  
  /*MOSI*/    //PC12
  __HAL_RCC_GPIOF_CLK_ENABLE();
  GPIO_InitStruct_SPI.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct_SPI.Pull = GPIO_PULLUP;
  GPIO_InitStruct_SPI.Speed = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct_SPI.Pin = MOSI_PIN;
  HAL_GPIO_Init(MOSI_PORT, &GPIO_InitStruct_SPI);
  HAL_GPIO_WritePin(MOSI_PORT, MOSI_PIN, GPIO_PIN_SET);
  
  /*MISO*/   //SPI_MISO -- SPI_B_MISO  PB4
  __HAL_RCC_GPIOB_CLK_ENABLE();
  GPIO_InitStruct_SPI.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct_SPI.Pull = GPIO_PULLUP;
  GPIO_InitStruct_SPI.Speed = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct_SPI.Pin = MISO_PIN;
  HAL_GPIO_Init(MISO_PORT, &GPIO_InitStruct_SPI);
  HAL_GPIO_WritePin(MISO_PORT, MISO_PIN, GPIO_PIN_SET);
}