/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __ADC_H
#define __ADC_H

/* Defines -------------------------------------------------------------------*/
#define RESOLUTION_12B 4096U
#define VREF 3.3f

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "cmsis_os2.h"
#include "math.h"

/* Function prototypes -------------------------------------------------------*/
//int Init_Thread_pot(void);
//void Thread_pot (void *argument);
void ADC1_pins_F429ZI_config(void);  //Configuracion de los pines
int ADC_Init_Single_Conversion(ADC_HandleTypeDef *, ADC_TypeDef  *);  //Configuracion del ADC
uint32_t ADC_getVoltage(ADC_HandleTypeDef * , uint32_t );  //lectura del voltaje convertido a un float
float ratio_to_ppm(float ratio);
float calculate_CO_ppm(uint32_t adc_value);

#endif /* __ADC_H */
