/*------------------------------------------------------------------------------
 * MDK Middleware - Component ::Network
 * Copyright (c) 2004-2019 Arm Limited (or its affiliates). All rights reserved.
 *------------------------------------------------------------------------------
 * Name:    HTTP_Server.c
 * Purpose: HTTP Server example
 *----------------------------------------------------------------------------*/

#include <stdio.h>

#include "main.h"
#include "rl_net.h"                     // Keil.MDK-Pro::Network:CORE

#include "LEDS.h"
#include "LCD.h"
#include "ADC.h"
#include "RTC.h"
#include "SNTP.h"
#include "BUTTON.h"
#include "RC522.h"
#include "pwm.h"

// Main stack size must be multiple of 8 Bytes
#define APP_MAIN_STK_SZ (1024U)
uint64_t app_main_stk[APP_MAIN_STK_SZ / 8];
const osThreadAttr_t app_main_attr = {
  .stack_mem  = &app_main_stk[0],
  .stack_size = sizeof(app_main_stk)
};

//extern uint16_t AD_in          (uint32_t ch);
//extern uint8_t  get_button     (void);
extern void     netDHCP_Notify (uint32_t if_num, uint8_t option, const uint8_t *val, uint32_t len);

extern bool LEDrun;
extern char lcd_text[2][20+1];
extern bool alarm;
extern bool sntp_hour;
extern RTC_HandleTypeDef RtcHandle;
extern RTC_AlarmTypeDef RtcAlarm;
extern ADC_HandleTypeDef hadc;

extern osThreadId_t TID_Display;
extern osThreadId_t TID_Led;

bool LEDrun;
bool LCDrun = false;
char lcd_text[2][20+1] = { "LCD line 1",
                           "LCD line 2" };
uint8_t rgb = 0;

/* Thread IDs */
osThreadId_t TID_Display;
osThreadId_t TID_Led;
osThreadId_t TID_AlarmRTC;

/* Thread declarations */
static void BlinkLed (void *arg);
static void Display  (void *arg);

__NO_RETURN void app_main (void *arg);
													 
/* Read analog inputs */
//uint16_t AD_in (uint32_t ch)
//{
//	int32_t val = 0;
//	if(ch == 13) 
//	{
//		ADC_StartConversion(); 
//		while(ADC_ConversionDone()!=0);
//		val = ADC_GetValue();
//	} 
//  return ((uint16_t)val);
//}

uint32_t AD_in (uint32_t ch) {//FUNCION MODIFICADA PARA USAR EN NUESTRA PLACA
	int32_t value=ADC_getVoltage(&hadc , ch);//get values from channel 10->ADC123_IN10
  return ((uint16_t)value);
}

/* IP address change notification */
void netDHCP_Notify (uint32_t if_num, uint8_t option, const uint8_t *val, uint32_t len) {

  (void)if_num;
  (void)val;
  (void)len;

  if (option == NET_DHCP_OPTION_IP_ADDRESS) {
    /* IP address change, trigger LCD update */
    osThreadFlagsSet (TID_Display, 0x01);
  }
}

/*----------------------------------------------------------------------------
  Thread 'Display': LCD display handler
 *---------------------------------------------------------------------------*/
static __NO_RETURN void Display (void *arg)
{
  while(1){
		LCD_clean();
		RTC_Show_TimeDate();
		osDelay(1000);
  }
}

/*----------------------------------------------------------------------------
  Thread 'BlinkLed': Blink the LEDs on an eval board
 *---------------------------------------------------------------------------*/
static __NO_RETURN void BlinkLed (void *arg) {
  (void)arg;
																
  while(1) {
		if(rgb == 0)			HAL_GPIO_WritePin(MBED_RGB_PORT, MBED_RGB_BLUE_PIN, GPIO_PIN_RESET);
  }
}

/*----------------------------------------------------------------------------
  Main Thread 'main': Run Network
 *---------------------------------------------------------------------------*/
__NO_RETURN void app_main (void *arg) {
  (void)arg;
  
  LCD_Initialize();
	LCD_reset();
	LCD_init();
	LCD_clean();
	
	LED_Initialize();
//	ADC_Initialize();
//	GPIO_Init_ButtonBlue();
//	Init_ThreadPulsacion();
	Init_Thread_RC522();
	RC522_Timer_Init();
	
	Init_Zumbador();
	Init_ThreadPWM();

  netInitialize ();
	
	osDelay(4000);
	Get_Time_SNTP();
	RTC_Init();
	RTC_Timer_Init();

  TID_Led     = osThreadNew (BlinkLed, NULL, NULL);
  TID_Display = osThreadNew (Display,  NULL, NULL);

  osThreadExit();
}
