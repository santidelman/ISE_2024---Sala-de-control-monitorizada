#ifndef __BME680_H
#define __BME680_H

#include <stdio.h>
#include <string.h>
#include "math.h"
#include "cmsis_os2.h"
#include "Driver_I2C.h"


#define BME680_I2C_ADDR       0x76        // Direccion I2C --> SDIO conectado a masa
#define SIZE_MSGQUEUE_BME680    1         // Se puede quitar si no se utilizan colas

//Header file only contains datatypes and functions that we made available for other modules
typedef struct // Esto se puede quitar si finalmente no se utilizan colas de mensajes
{
  float temperatura;
  float humedad;
  float gas;
  int IAQ;
} MSGQUEUE_BME680_t;

extern int Init_Th_BME680 (void);

void BME680_Init (void);
void BME680_Gas_Config (void);
uint8_t BME680_IsMeasuring (uint8_t addr);

float Get_temperature (void);
//float Get_pressure (void);
float Get_humidity (void);
float Get_gas_resistance (void);

int GetHumidityScore (void);
int GetGasScore (void);
int Get_IAQ (void);

uint8_t calc_res_heat (void); 

void BME680_I2C_Write (uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint32_t len);
void BME680_I2C_Read (uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint32_t len);

#endif /* __BME680_H */

