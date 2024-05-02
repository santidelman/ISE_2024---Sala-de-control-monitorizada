#ifndef _LCD_H_
#define _LCD_H_

#include "RTE_Device.h"
#include "Driver_SPI.h"
#include "stdio.h"
#include "string.h"
#include "stm32f4xx_hal.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define LCD_RESET_PIN      GPIO_PIN_6
#define LCD_RESET_PORT     GPIOA
#define LCD_CS_PIN         GPIO_PIN_14
#define LCD_CS_PORT        GPIOD
#define LCD_A0_PIN         GPIO_PIN_13
#define LCD_A0_PORT        GPIOF
/* Private macro -------------------------------------------------------------*/
/* Private variable ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
void LCD_reset										(void);
void delay												(uint32_t n_microsegundos);
void LCD_Initialize								(void);
void LCD_wr_data									(unsigned char data);
void LCD_wr_cmd										(unsigned char cmd);
void LCD_init											(void);
void LCD_update										(void);
void LCD_Cuadrado									(void);
void LCD_LetraA										(void);
void LCD_symbolToLocalBuffer_L1		(uint8_t symbol);
void LCD_symbolToLocalBuffer_L2		(uint8_t symbol);
void LCD_WriteSentence						(char *frase, uint8_t line);
void LCD_clean										(void);

#endif
