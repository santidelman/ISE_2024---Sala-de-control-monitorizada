#ifndef __RTC_H
#define __RTC_H

#include "stm32f4xx_hal.h"
#include <stdio.h>
#include "LCD.h"
#include <time.h>
#include "rl_net.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define RTC_ASYNCH_PREDIV  0x7F   
#define RTC_SYNCH_PREDIV   0x00FF 
/* Private macro -------------------------------------------------------------*/
/* Private variable ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
void RTC_Init											(void);
void RTC_Config										(void);
void RTC_Show_TimeDate						(void);
void RTC_Set_Alarm								(void);

#endif /* __RTC_H */
