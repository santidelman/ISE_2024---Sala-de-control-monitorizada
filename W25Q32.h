#ifndef __W25Qxx_H
#define __W25Qxx_H

#define TRANSFER_COMPLETE 					0x99              //flag que se utiliza en SPI para saber que una transferencia se ha completado
//#define csLOW() HAL_GPIO_WritePIN (GPIOA, GPIO_PIN_15, GPIO_PIN_RESET) // REVISAR PINES
//#define csHIGH() HAL_GPIO_WritePIN (GPIOA, GPIO_PIN_15, GPIO_PIN_SET) // REVISAR PINES


#define numBLOCK 32   // numero de bloques para una memoria de 16 Mb

#include "stm32f4xx_hal.h"
#include "Driver_SPI.h"
#include <stdio.h>
#include "cmsis_os2.h"   
#include <stdint.h>

// Estructura para medidas ambientales
typedef struct {
    uint32_t timestamp;  // Unix timestamp
    float temperatura;   // Temperatura
    float humedad;       // Humedad
    float co;            // Nivel de CO
} MedidaAmbiental;

// Estructura para registros de acceso
typedef struct {
    uint32_t timestamp;  // Unix timestamp
    uint32_t id;         // ID de 8 dígitos
    char tipo;           // 'E' para entrada, 'S' para salida
} RegistroAcceso;


int Init_FLASH (void);
void Thread_FLASH(void *argument);
void W25Q_Reset (void);
void W25Q32_WriteEnable(void);
uint32_t W25Q32_ReadID(void);
void W25Q32_WaitForWriteEnd(void);
void W25Q32_WriteData(uint32_t addr, uint8_t *data, uint32_t size);
void W25Q32_ReadData(uint32_t addr, uint8_t *buffer, uint32_t size);
void W25Q32_EraseSector(uint32_t addr);
void W25Q32_WriteMedidaAmbiental(uint32_t addr, MedidaAmbiental* medida);
void W25Q32_WriteRegistroAcceso(uint32_t addr, RegistroAcceso* registro);
void W25Q32_ReadMedidaAmbiental(uint32_t addr, MedidaAmbiental* medida);
void W25Q32_ReadRegistroAcceso(uint32_t addr, RegistroAcceso* registro);

#endif
