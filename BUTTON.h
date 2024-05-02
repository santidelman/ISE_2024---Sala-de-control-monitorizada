#ifndef __BUTTON_H
#define __BUTTON_H

#include "stm32f4xx_hal.h"
#include "cmsis_os2.h"
#include "SNTP.h"


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define BUTTON_BLUE	GPIO_PIN_13
/* Private macro -------------------------------------------------------------*/
/* Private variable ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
void GPIO_Init_ButtonBlue		(void);
int Init_ThreadPulsacion		(void);
void ThreadPulsacion				(void *argument);

#endif /* __BUTTON_H */
