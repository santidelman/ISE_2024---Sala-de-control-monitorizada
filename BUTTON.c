#include "BUTTON.h"

/* Private typedef -----------------------------------------------------------*/
GPIO_InitTypeDef GPIO_InitStruct_Button;
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variable ----------------------------------------------------------*/
osThreadId_t tid_ThreadPulsacion;  

bool hour_summer = false;

void GPIO_Init_ButtonBlue(void)
{
	__HAL_RCC_GPIOC_CLK_ENABLE();
	GPIO_InitStruct_Button.Pin = BUTTON_BLUE;
	GPIO_InitStruct_Button.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct_Button.Pull = GPIO_PULLDOWN;

	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct_Button);
	HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
}
void EXTI15_10_IRQHandler(void)
{
	HAL_GPIO_EXTI_IRQHandler(BUTTON_BLUE);
}
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
//	RTC_Config();
//	osThreadFlagsSet(tid_ThreadPulsacion,0x01);
}

int Init_ThreadPulsacion(void)
{
	
  tid_ThreadPulsacion = osThreadNew(ThreadPulsacion, NULL, NULL);
  if (tid_ThreadPulsacion == NULL) {
    return(-1);
  }
  return(0);
}

 
void ThreadPulsacion(void *argument)
{
	while(1){
		osThreadFlagsWait(0x01, osFlagsWaitAny, osWaitForever);
		if (HAL_GPIO_ReadPin(GPIOC,BUTTON_BLUE) == GPIO_PIN_SET) 	hour_summer = true;
		else 																											hour_summer = false;
		Get_Time_SNTP();
	}
}
