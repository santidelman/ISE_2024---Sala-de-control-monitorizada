#include "RTC.h"

/* Private typedef -----------------------------------------------------------*/
RTC_HandleTypeDef RtcHandle;
RTC_DateTypeDef sdatestructure;
RTC_TimeTypeDef stimestructure;
RTC_DateTypeDef sdatestructureget;
RTC_TimeTypeDef stimestructureget;
RTC_AlarmTypeDef RtcAlarm; 
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variable ----------------------------------------------------------*/
char showtime[20];
char showdate[20];
bool alarm = false;

void RTC_Init(void)
{
	/*##-1- Configure the RTC peripheral #######################################*/
  /* Configure RTC prescaler and RTC data registers */
  /* RTC configured as follows:
      - Hour Format    = Format 24
      - Asynch Prediv  = Value according to source clock
      - Synch Prediv   = Value according to source clock
      - OutPut         = Output Disable
      - OutPutPolarity = High Polarity
      - OutPutType     = Open Drain */ 
	RtcHandle.Instance = RTC; 
  RtcHandle.Init.HourFormat = RTC_HOURFORMAT_24;
  RtcHandle.Init.AsynchPrediv = RTC_ASYNCH_PREDIV;
  RtcHandle.Init.SynchPrediv = RTC_SYNCH_PREDIV;
  RtcHandle.Init.OutPut = RTC_OUTPUT_DISABLE;
  RtcHandle.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  RtcHandle.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  __HAL_RTC_RESET_HANDLE_STATE(&RtcHandle);
	HAL_RTC_Init(&RtcHandle);
	
	/*##-2- Check if Data stored in BackUp register1: No Need to reconfigure RTC#*/
  /* Read the Back Up Register 1 Data */
  if (HAL_RTCEx_BKUPRead(&RtcHandle, RTC_BKP_DR1) != 0x32F2)
  {
    /* Configure RTC Calendar */
    RTC_Config();
  }
	
	HAL_NVIC_EnableIRQ(RTC_Alarm_IRQn);
	RTC_Set_Alarm();
	RTC_Show_TimeDate();
}

void RTC_Config(void)
{
  /*##-1- Configure the Date #################################################*/
  /* Set Date: January 1th 0000 */
  sdatestructure.Year = 0x00;
  sdatestructure.Month = RTC_MONTH_JANUARY;
  sdatestructure.Date = 0x01;
	HAL_RTC_SetDate(&RtcHandle,&sdatestructure,RTC_FORMAT_BIN);

  /*##-2- Configure the Time #################################################*/
  /* Set Time: 00:00:00 */
  stimestructure.Hours = 0x00;
  stimestructure.Minutes = 0x00;
  stimestructure.Seconds = 0x00;
  stimestructure.TimeFormat = RTC_HOURFORMAT_24;
  stimestructure.DayLightSaving = RTC_DAYLIGHTSAVING_NONE ;
  stimestructure.StoreOperation = RTC_STOREOPERATION_RESET;
	HAL_RTC_SetTime(&RtcHandle,&stimestructure,RTC_FORMAT_BIN);
}

/**
  * @brief  Display the current time and date.
  * @param  showtime : pointer to buffer
  * @param  showdate : pointer to buffer
  * @retval None
  */
void RTC_Show_TimeDate(void)
{
  /* Get the RTC current Time */
  HAL_RTC_GetTime(&RtcHandle, &stimestructureget, RTC_FORMAT_BIN);
  /* Get the RTC current Date */
  HAL_RTC_GetDate(&RtcHandle, &sdatestructureget, RTC_FORMAT_BIN);
  /* Display time Format : hh:mm:ss */
  sprintf(showtime, "%.2d:%.2d:%.2d", stimestructureget.Hours, stimestructureget.Minutes, stimestructureget.Seconds);
  /* Display date Format : mm-dd-yy */
  sprintf(showdate, "%.2d-%.2d-%.2d", sdatestructureget.Date, sdatestructureget.Month, 2000 + sdatestructureget.Year);
	
	LCD_WriteSentence(showtime,1);
	LCD_WriteSentence(showdate,2);
}

void HAL_RTC_MspInit(RTC_HandleTypeDef *hrtc)
{
  RCC_OscInitTypeDef        RCC_OscInitStruct;
  RCC_PeriphCLKInitTypeDef  PeriphClkInitStruct;

  /*##-1- Enables the PWR Clock and Enables access to the backup domain ###################################*/
  /* To change the source clock of the RTC feature (LSE, LSI), You have to:
     - Enable the power clock using __HAL_RCC_PWR_CLK_ENABLE()
     - Enable write access using HAL_PWR_EnableBkUpAccess() function before to 
       configure the RTC clock source (to be done once after reset).
     - Reset the Back up Domain using __HAL_RCC_BACKUPRESET_FORCE() and 
       __HAL_RCC_BACKUPRESET_RELEASE().
     - Configure the needed RTc clock source */
  __HAL_RCC_PWR_CLK_ENABLE();
  HAL_PWR_EnableBkUpAccess();

  
  //##-2- Configure LSE as RTC clock source ###################################/
  RCC_OscInitStruct.OscillatorType =  RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
	HAL_RCC_OscConfig(&RCC_OscInitStruct);
  
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
  PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
	HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct);
  
  //##-3- Enable RTC peripheral Clocks #######################################/
  /* Enable RTC Clock */
  __HAL_RCC_RTC_ENABLE();
}

void HAL_RTC_AlarmAEventCallback(RTC_HandleTypeDef *hrtc)
{
	alarm = true;
//	RTC_Set_Alarm();
}

void RTC_Alarm_IRQHandler(void)
{
  HAL_RTC_AlarmIRQHandler(&RtcHandle);
}

void RTC_Set_Alarm(void)
{
  RtcAlarm.AlarmTime.Hours = 0x00;
  RtcAlarm.AlarmTime.Minutes = 0x01;
	RtcAlarm.AlarmTime.Seconds = 0x00;
	RtcAlarm.AlarmTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
	RtcAlarm.Alarm = RTC_ALARM_A;
	RtcAlarm.AlarmTime.StoreOperation = RTC_STOREOPERATION_SET;
	RtcAlarm.AlarmMask = RTC_ALARMMASK_DATEWEEKDAY | RTC_ALARMMASK_HOURS | RTC_ALARMMASK_MINUTES;
	RtcAlarm.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_DATE;
	RtcAlarm.AlarmDateWeekDay = 0x01;
	
	HAL_RTC_SetAlarm_IT(&RtcHandle, &RtcAlarm, RTC_FORMAT_BIN);
}
