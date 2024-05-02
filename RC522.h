#ifndef __RC522_H
#define __RC522_H

#include "stdint.h"
#include "gpio.h"
#include "spi.h"

#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "stm32f4xx_hal.h"
#include <stdio.h>
#include <stdbool.h>
#include "string.h"
#include "LCD.h"

typedef unsigned char uchar;
#define MAX_CLIENTS_SIZE 10
#define ID_SIZE 5
typedef struct {
    uchar array[ID_SIZE];
} UcharArray;
typedef enum {
	MI_OK = 0,
	MI_NOTAGERR,
	MI_ERR
} TM_MFRC522_Status_t;

typedef struct{
  uint8_t id[5];
} MSGQUEUE_RC522_t;

//MF522 Command word
#define PCD_IDLE              0x00               //NO action; Cancel the current command
#define PCD_AUTHENT           0x0E               //Authentication Key
#define PCD_RECEIVE           0x08               //Receive Data
#define PCD_TRANSMIT          0x04               //Transmit data
#define PCD_TRANSCEIVE        0x0C               //Transmit and receive data,
#define PCD_RESETPHASE        0x0F               //Reset
#define PCD_CALCCRC           0x03               //CRC Calculate

// Mifare_One card command word
#define PICC_REQIDL          0x26               // find the antenna area does not enter hibernation
#define PICC_REQALL          0x52               // find all the cards antenna area
#define PICC_ANTICOLL        0x93               // anti-collision
#define PICC_SElECTTAG       0x93               // election card
#define PICC_AUTHENT1A       0x60               // authentication key A
#define PICC_AUTHENT1B       0x61               // authentication key B
#define PICC_READ            0x30               // Read Block
#define PICC_WRITE           0xA0               // write block
#define PICC_DECREMENT       0xC0               // debit
#define PICC_INCREMENT       0xC1               // reuint8_tge
#define PICC_RESTORE         0xC2               // transfer block data to the buffer
#define PICC_TRANSFER        0xB0               // save the data in the buffer
#define PICC_HALT            0x50               // Sleep

/* MFRC522 Registers */
//Page 0: Command and Status
#define MFRC522_REG_COMMAND					0x01 
#define MFRC522_REG_COMM_IE_N				0x02  
#define MFRC522_REG_COMM_IRQ				0x04 
#define MFRC522_REG_DIV_IRQ					0x05
#define MFRC522_REG_ERROR						0x06 
#define MFRC522_REG_STATUS2					0x08 
#define MFRC522_REG_FIFO_DATA				0x09
#define MFRC522_REG_FIFO_LEVEL			0x0A
#define MFRC522_REG_CONTROL					0x0C
#define MFRC522_REG_BIT_FRAMING			0x0D

//Page 1: Command 
#define MFRC522_REG_MODE						0x11
#define MFRC522_REG_TX_CONTROL			0x14
#define MFRC522_REG_TX_AUTO					0x15

//Page 2: CFG   
#define MFRC522_REG_CRC_RESULT_M		0x21
#define MFRC522_REG_CRC_RESULT_L		0x22
#define MFRC522_REG_RF_CFG					0x26
#define MFRC522_REG_T_MODE					0x2A
#define MFRC522_REG_T_PRESCALER			0x2B
#define MFRC522_REG_T_RELOAD_H			0x2C
#define MFRC522_REG_T_RELOAD_L			0x2D

#define MFRC522_MAX_LEN							16

#define SPI3_transfer_complete			0x01
#define RC522_readUID								0x02

static void Timer_RC522_read											(void const *arg);
int RC522_Timer_Init															(void);
int Init_Thread_RC522															(void);
void RC522_Initialize															(void);

static void TM_MFRC522_Init												(void);
static TM_MFRC522_Status_t TM_MFRC522_Check				(uint8_t* id);
bool TM_MFRC522_Compare														(uint8_t* CardID, uint8_t* CompareID);
static void TM_MFRC522_WriteRegister							(uint8_t addr, uint8_t val);
static uint8_t TM_MFRC522_ReadRegister						(uint8_t addr);
static void TM_MFRC522_SetBitMask									(uint8_t reg, uint8_t mask);
static void TM_MFRC522_ClearBitMask								(uint8_t reg, uint8_t mask);
static void TM_MFRC522_AntennaOn									(void);
static void TM_MFRC522_AntennaOff									(void);
static void TM_MFRC522_Reset											(void);
static TM_MFRC522_Status_t TM_MFRC522_Request			(uint8_t reqMode, uint8_t* TagType);
static TM_MFRC522_Status_t TM_MFRC522_ToCard			(uint8_t command, uint8_t* sendData, uint8_t sendLen, uint8_t* backData, uint16_t* backLen);
static TM_MFRC522_Status_t TM_MFRC522_Anticoll		(uint8_t* serNum);
static void TM_MFRC522_CalculateCRC								(uint8_t*  pIndata, uint8_t len, uint8_t* pOutData);
static uint8_t TM_MFRC522_SelectTag								(uint8_t* serNum);
static TM_MFRC522_Status_t TM_MFRC522_Auth				(uint8_t authMode, uint8_t BlockAddr, uint8_t* Sectorkey, uint8_t* serNum);
static TM_MFRC522_Status_t TM_MFRC522_Read				(uint8_t blockAddr, uint8_t* recvData);
static TM_MFRC522_Status_t TM_MFRC522_Write				(uint8_t blockAddr, uint8_t* writeData);
static void TM_MFRC522_Halt(void);

#endif
