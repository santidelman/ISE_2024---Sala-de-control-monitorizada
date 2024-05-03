#include "pwm.h"

/* Private typedef -----------------------------------------------------------*/
TIM_HandleTypeDef TIM5_Handler;
TIM_OC_InitTypeDef TIM5_ConfigChannelOC;
GPIO_InitTypeDef GPIO_InitStruct_ZUMB;
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
osTimerId_t tid_TimerPWM;                    
uint32_t exec4; 
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/   

void Timer5OC_Init(uint32_t prescaler, uint32_t period, uint32_t pulso)
{
	/* Pasos para utilizar los Timers -> pagina 1013 y 1014 (manual del HAL y LL)*/
	/* Paso 2A: Inicializar el reloj del TIM*/
	__HAL_RCC_TIM5_CLK_ENABLE(); 
	
	/* Paso 4: Configurar el timer en el modo Output Compare*/
	TIM5_Handler.Instance = TIM5;
	TIM5_Handler.Init.Prescaler = prescaler;
	TIM5_Handler.Init.Period = period;
	HAL_TIM_PWM_Init(&TIM5_Handler);
	/* Paso 4: Configurar el canal del modo Output Compare*/
	/* Para la funcionabilidad de cada uno de los pines vamos a una tabla -> pagina 54 a 70 (DATASHEET)*/
	TIM5_ConfigChannelOC.OCMode = TIM_OCMODE_PWM1;
	TIM5_ConfigChannelOC.Pulse = pulso;
	HAL_TIM_PWM_ConfigChannel(&TIM5_Handler, &TIM5_ConfigChannelOC, TIM_CHANNEL_4);
	
	/* Paso 5: Arrancamos el timer*/
	HAL_TIM_OC_Start(&TIM5_Handler, TIM_CHANNEL_4); 
	
}

void Init_Zumbador(void)
{
	/*-1- Enable GPIO Clock (to be able to program the configuration registers)*/
	__HAL_RCC_GPIOA_CLK_ENABLE();
	
	/*-2- Configure IO in output push-pull mode to drive external LEDs*/
	GPIO_InitStruct_ZUMB.Pin = ZUMB_PIN;
	GPIO_InitStruct_ZUMB.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct_ZUMB.Pull = GPIO_PULLUP;
	GPIO_InitStruct_ZUMB.Alternate = GPIO_AF2_TIM5;
	HAL_GPIO_Init(ZUMB_PORT, &GPIO_InitStruct_ZUMB);
	
	Init_ThreadPWM();
	Init_TimerPWM();
}

/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/

osThreadId_t tid_ThreadPWM;
void ThreadPWM(void *argument);  

int Init_ThreadPWM(void)
{
  tid_ThreadPWM = osThreadNew(ThreadPWM, NULL, NULL);
  if (tid_ThreadPWM == NULL) {
    return(-1);
  }
  return(0);
}
 
void ThreadPWM(void *argument)
{
  while (1) {
		osThreadFlagsWait(senalPWM, osFlagsWaitAny, osWaitForever);
		Timer5OC_Init(999, 83, 41);
		osTimerStart(tid_TimerPWM,100U);		
		osThreadYield(); 
  }
}

/*----------------------------------------------------------------------------
 *      Timer: Sample timer functions
 *---------------------------------------------------------------------------*/
     
// One-Shoot Timer Function
void TimerPWM_Callback(void const *arg)
{
	HAL_TIM_OC_Stop(&TIM5_Handler, TIM_CHANNEL_4);
}

int Init_TimerPWM(void)
{
  // Create one-shoot timer
  exec4 = 1U;
  tid_TimerPWM = osTimerNew((osTimerFunc_t)&TimerPWM_Callback, osTimerOnce, &exec4, NULL);
  return NULL;
}
