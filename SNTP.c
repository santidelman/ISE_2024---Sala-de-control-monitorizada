#include "SNTP.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variable ----------------------------------------------------------*/

extern RTC_HandleTypeDef RtcHandle;
extern RTC_DateTypeDef sdatestructureget;
extern RTC_TimeTypeDef stimestructureget; 
extern bool LCDrun;
extern bool hour_summer;

uint32_t sec = 0;
bool sntp_hour = false;
struct tm SNTP;
const NET_ADDR4 ntp_server = { NET_ADDR_IP4, 0, 150, 214, 94, 10 };
static void time_callback (uint32_t seconds, uint32_t seconds_fraction);
osThreadId_t tid_ThreadSNTP;

//osTimerId_t tim_RTC_SNTP;
//static uint32_t exec;

//static void Timer_sync_RTC_SNTP(void const *arg)
//{
//  Get_Time_SNTP();
//	sntp_hour = true;
//	LCDrun = false;
//}

//int RTC_Timer_Init(void)
//{
//	osStatus_t status;                            // function return status
//  // Create periodic timer
//  exec = 2U;
//  tim_RTC_SNTP = osTimerNew((osTimerFunc_t)&Timer_sync_RTC_SNTP, osTimerPeriodic, &exec, NULL);
//  if (tim_RTC_SNTP != NULL) {  // Periodic timer created
//    status = osTimerStart(tim_RTC_SNTP, 180000U);            
//    if (status != osOK) {
//      return -1;
//    }
//	}
//	return 0;
//}

void Get_Time_SNTP (void) 
{
  if (netSNTPc_GetTime ((NET_ADDR *)&ntp_server, time_callback) == netOK) {
    LCD_WriteSentence("SNTP request sent",1);
  }
  else {
    LCD_WriteSentence("SNTP ERROR",1);
  }
}
 
static void time_callback (uint32_t seconds, uint32_t seconds_fraction)
{
  if (seconds == 0) {
    LCD_WriteSentence("Server SNTP ERROR",2);
  }
  else {
		if(!hour_summer)	sec = seconds + 3600;
		else 	sec = seconds + 7200;
    LCD_WriteSentence("Server SNTP",2);
    SNTP = *localtime(&sec);
    RTC_Show_SNTP_TimeDate(); 
  }
}

void RTC_Show_SNTP_TimeDate(void)
{
  /*##-1- Configure the Date SNTP #################################################*/
  sdatestructureget.Year = SNTP.tm_year - 100;
  sdatestructureget.Month = SNTP.tm_mon + 1;
  sdatestructureget.Date = SNTP.tm_mday;
	HAL_RTC_SetDate(&RtcHandle,&sdatestructureget,RTC_FORMAT_BIN);

  /*##-2- Configure the Time SNTP #################################################*/
  stimestructureget.Hours = SNTP.tm_hour;
  stimestructureget.Minutes = SNTP.tm_min;
  stimestructureget.Seconds = SNTP.tm_sec;
  stimestructureget.TimeFormat = RTC_HOURFORMAT_24;
  stimestructureget.DayLightSaving = RTC_DAYLIGHTSAVING_NONE ;
  stimestructureget.StoreOperation = RTC_STOREOPERATION_RESET;
	HAL_RTC_SetTime(&RtcHandle,&stimestructureget,RTC_FORMAT_BIN);
	
	printf(" Hora del sistema:  %.2d:%.2d:%.2d\n", stimestructureget.Hours, stimestructureget.Minutes, stimestructureget.Seconds);
	printf(" Fecha del sistema: %.2d-%.2d-%.2d\n", sdatestructureget.Date, sdatestructureget.Month, 2000 + sdatestructureget.Year);
}
