#ifndef __SNTP_H
#define __SNTP_H

#include "stm32f4xx_hal.h"
#include "cmsis_os2.h"
#include <stdio.h>
#include "LCD.h"
#include <time.h>
#include "rl_net.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variable ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
int RTC_Timer_Init								(void);
void Get_Time_SNTP                (void);
void RTC_Show_SNTP_TimeDate       (void);

#endif /* __SNTP_H */
