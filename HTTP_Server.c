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
	static char buf[24];
  while(1){
		/* PRACTICA 1*/
		if(LCDrun){
			osDelay(1000);
			LCD_clean();
			sprintf (buf, "%-20s", lcd_text[0]);
			LCD_WriteSentence(buf,1);
			sprintf (buf, "%-20s", lcd_text[1]);
			LCD_WriteSentence(buf,2);
		} else {
		/* PRACTICA 2*/
			osDelay(1000);
//			LCD_clean();
//			RTC_Show_TimeDate();
		}
  }
}

/*----------------------------------------------------------------------------
  Thread 'BlinkLed': Blink the LEDs on an eval board
 *---------------------------------------------------------------------------*/
static __NO_RETURN void BlinkLed (void *arg) {
  const uint8_t led_val[16] = { 0x48,0x88,0x84,0x44,0x42,0x22,0x21,0x11,
                                0x12,0x0A,0x0C,0x14,0x18,0x28,0x30,0x50 };
  (void)arg;
																
  uint32_t cnt = 0U;
	int ticks_5s, ticks_2s = 0;
	GPIO_PinState on_off;
                                
  LEDrun = true;
																
  while(1) {
    /* PRACTICA 1*/
		if (LEDrun == true) {
      LED_SetOut (led_val[cnt]);
      if (++cnt >= sizeof(led_val)) {
        cnt = 0U;
      }
    }			
    /* PRACTICA 2*/
		if(alarm == true){
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0,on_off);
			ticks_5s++;
			if(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_0) == GPIO_PIN_SET)	on_off = GPIO_PIN_RESET;
			else 																										on_off = GPIO_PIN_SET;
			if(ticks_5s == 50){
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
				ticks_5s = 0;
				alarm = false; 
			}
		}
		if(sntp_hour){
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14,on_off);
			ticks_2s++;
			if(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_14) == GPIO_PIN_SET)	on_off = GPIO_PIN_RESET;
			else 																										on_off = GPIO_PIN_SET;
			if(ticks_2s == 20){
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);
				ticks_2s = 0;
				sntp_hour = false; 
			}
		}
    osDelay(100);		
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

  netInitialize ();
	
	osDelay(5000);
//	RTC_Init();
//	Get_Time_SNTP();
//	RTC_Timer_Init();

  TID_Led     = osThreadNew (BlinkLed, NULL, NULL);
  TID_Display = osThreadNew (Display,  NULL, NULL);

  osThreadExit();
}
