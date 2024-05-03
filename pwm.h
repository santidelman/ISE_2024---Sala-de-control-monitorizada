#ifndef _PWM_H_
#define _PWM_H_

#include "RTE_Device.h"
#include "stm32f4xx_hal.h"
#include "cmsis_os2.h" 

#define senalPWM			 0x00000002U

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define ZUMB_PIN		GPIO_PIN_3
#define ZUMB_PORT 	GPIOA
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
void Timer5OC_Init(uint32_t prescaler, uint32_t period, uint32_t pulso);
void Init_Zumbador(void);
int Init_ThreadPWM(void);
void ThreadPWM(void *argument); 
void TimerPWM_Callback(void const *arg);
int Init_TimerPWM(void);

#endif
