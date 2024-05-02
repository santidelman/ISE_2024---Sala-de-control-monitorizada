/*
 * RC522.C
 *
 *  Created on: Apr 19, 2024
 *      Author: alcid
 */


#include "RC522.h"

uint8_t CardUID[5];  // almacena el buffer del codigo de rfid

//SPI init
extern ARM_DRIVER_SPI Driver_SPI3;
static ARM_DRIVER_SPI* SPI3drv = &Driver_SPI3;

osTimerId_t tim_RC522;
static uint32_t exec;

osThreadId_t TID_RC522;

/**
 * Función de temporizador para activar la bandera de evento de lectura de RC522.
 * @param arg Argumento de la función de temporizador (no utilizado).
 */
static void Timer_RC522_read(void const *arg)
{
  osThreadFlagsSet(TID_RC522, RC522_readUID);
}

/**
 * Inicializa el temporizador para la lectura periódica de RC522.
 * @return 0 si la inicialización es exitosa, -1 si hay un error.
 */
int RC522_Timer_Init(void)
{
	osStatus_t status;                            // function return status
  // Create periodic timer
  exec = 2U;
  tim_RC522 = osTimerNew((osTimerFunc_t)&Timer_RC522_read, osTimerPeriodic, &exec, NULL);
  if (tim_RC522 != NULL) {  // Periodic timer created
    status = osTimerStart(tim_RC522, 1000U);            
    if (status != osOK) {
      return -1;
    }
	}
	return 0;
}

///* Thread declarations */
static void Thread_RC522 (void *arg);

/**
 * Inicializa el hilo para manejar la lectura del RFID RC522.
 * @return 0 si la inicialización es exitosa, -1 si hay un error.
 */
int Init_Thread_RC522 (void) 
{
  TID_RC522 = osThreadNew(Thread_RC522, NULL, NULL);
  if (TID_RC522 == NULL) {
    return(-1);
  }
  return(0);
}

/**
 * Hilo que maneja la lectura del RFID RC522.
 * @param arg Argumento del hilo (no utilizado).
 */
static __NO_RETURN void Thread_RC522 (void *arg) {
		(void)arg;
		char buffer[20];
	
		RC522_Initialize();
	
		while(1) {
			osThreadFlagsWait(RC522_readUID, osFlagsWaitAny, osWaitForever);
			if(TM_MFRC522_Check(CardUID) == MI_OK){
				
				LCD_clean();
				sprintf(buffer, "%02X %02X %02X %02X %02X", CardUID[0], CardUID[1], CardUID[2], CardUID[3], CardUID[4]);
				LCD_WriteSentence(buffer,1);
				
			} else {	// MI_ERR
				printf("Error\n");
			}
		}
}

/**
* Inicializacion del RFID. 
*			- Configuracion del SPI y de los puertos GPIO. 
*			- Inicializa el modulo RC522
*/
void RC522_Initialize(void){
	RC522_GPIO_Init();
	RC522_SPI3_Init();
	TM_MFRC522_Init();
}

/**
*	Inicializa el módulo MFRC522.
*		- Realiza un reinicio del módulo.
*		- Configura los registros de temporización y ganancia de la señal.
*		- Activa la antena del módulo.
*/
static void TM_MFRC522_Init (void) {
	
	TM_MFRC522_Reset();
	
	TM_MFRC522_WriteRegister(MFRC522_REG_T_MODE, 0x8D);
	TM_MFRC522_WriteRegister(MFRC522_REG_T_PRESCALER, 0x3E);
	TM_MFRC522_WriteRegister(MFRC522_REG_T_RELOAD_L, 30);           
	TM_MFRC522_WriteRegister(MFRC522_REG_T_RELOAD_H, 0);

	/* 48dB gain */
	TM_MFRC522_WriteRegister(MFRC522_REG_RF_CFG, 0x70);
	
	TM_MFRC522_WriteRegister(MFRC522_REG_TX_AUTO, 0x40);
	TM_MFRC522_WriteRegister(MFRC522_REG_MODE, 0x3D);

	TM_MFRC522_AntennaOn();		//Open the antenna
	
}


/**
* Realiza la verificación de la presencia de una tarjeta RFID.
* @param id Puntero a un arreglo donde se almacenará el identificador de la tarjeta si se encuentra.
* @return El estado de la operación, representado por el tipo de dato TM_MFRC522_Status_t.
*		- Si se encuentra una tarjeta, devuelve MI_OK.
*		- Si no encuentra una tarjeta, devuelve MI_ERR
*/
static TM_MFRC522_Status_t TM_MFRC522_Check(uint8_t* id) {
	TM_MFRC522_Status_t status;
	//Find cards, return card type
	status = TM_MFRC522_Request(PICC_REQIDL, id);		
	if (status == MI_OK) {	//si se detecto correctamente una tarjeta
		//Card detected
		//Anti-collision, return card serial number 4 bytes
		status = TM_MFRC522_Anticoll(id);	//por si acaso hay colision
	}
	TM_MFRC522_Halt();			//Command card into hibernation 

	return status;
}

/**
* Compara dos identificadores de tarjeta RFID.
* @param CardID Puntero al primer identificador de tarjeta.
* @param CompareID Puntero al segundo identificador de tarjeta.
* @return true si los identificadores son iguales, false en caso contrario.
*/
bool TM_MFRC522_Compare(uint8_t* CardID, uint8_t* CompareID) {
	uint8_t i;
	for (i = 0; i < 5; i++) {
		if (CardID[i] != CompareID[i]) {
			return false;
		}
	}
	return true;
}

/**
* Escribe un valor en un registro del módulo MFRC522.
* @param addr Dirección del registro a escribir.
* @param val Valor a escribir en el registro.
*/
static void TM_MFRC522_WriteRegister(uint8_t addr, uint8_t val) {
	static uint8_t tx_data[2];
	
	//CS low
	HAL_GPIO_WritePin(RC522_CS_PORT, RC522_CS_PIN, GPIO_PIN_RESET);
	
	//send address and data
	tx_data[0] = (addr << 1) & 0x7E;
	tx_data[1] = val;
	
	SPI3drv->Send(tx_data, 2);
	osThreadFlagsWait(SPI3_transfer_complete, osFlagsWaitAny, osWaitForever);
	
	//CS high
	HAL_GPIO_WritePin(RC522_CS_PORT, RC522_CS_PIN, GPIO_PIN_SET);
}

/**
* Lee el valor de un registro del módulo MFRC522.
* @param addr Dirección del registro a leer.
* @return El valor leído desde el registro.
*/
static uint8_t TM_MFRC522_ReadRegister(uint8_t addr) {
	static uint8_t rx_data;
	
	//CS low
	HAL_GPIO_WritePin(RC522_CS_PORT, RC522_CS_PIN, GPIO_PIN_RESET);
	
	//send address and data
	uint8_t tx_data = ((addr << 1) & 0x7E) | 0x80; 	
	
	SPI3drv->Send(&tx_data, 1);
	osThreadFlagsWait(SPI3_transfer_complete, osFlagsWaitAny, osWaitForever);
	SPI3drv->Receive(&rx_data, 1);
	osThreadFlagsWait(SPI3_transfer_complete, osFlagsWaitAny, osWaitForever);
	
	//CS high
	HAL_GPIO_WritePin(RC522_CS_PORT, RC522_CS_PIN, GPIO_PIN_SET);
	return rx_data;	
}

/**
* Establece bits específicos en un registro del módulo MFRC522.
* @param reg Dirección del registro a modificar.
* @param mask Máscara de bits a establecer.
*/
static void TM_MFRC522_SetBitMask(uint8_t reg, uint8_t mask) {
	TM_MFRC522_WriteRegister(reg, TM_MFRC522_ReadRegister(reg) | mask);
}

/**
* Borra bits específicos en un registro del módulo MFRC522.
* @param reg Dirección del registro a modificar.
* @param mask Máscara de bits a borrar.
*/
static void TM_MFRC522_ClearBitMask(uint8_t reg, uint8_t mask){
	TM_MFRC522_WriteRegister(reg, TM_MFRC522_ReadRegister(reg) & (~mask));
} 


/**
* Activa la antena del módulo MFRC522 si no está activada.
*/
static void TM_MFRC522_AntennaOn(void) {
	uint8_t temp;

	temp = TM_MFRC522_ReadRegister(MFRC522_REG_TX_CONTROL);
	if (!(temp & 0x03)) {
		TM_MFRC522_SetBitMask(MFRC522_REG_TX_CONTROL, 0x03);
	}
}

/**
* Desactiva la antena del módulo MFRC522.
*/
static void TM_MFRC522_AntennaOff(void) {
	TM_MFRC522_ClearBitMask(MFRC522_REG_TX_CONTROL, 0x03);
}

/**
* Reinicia el módulo MFRC522.
*/
static void TM_MFRC522_Reset(void) {
	TM_MFRC522_WriteRegister(MFRC522_REG_COMMAND, PCD_RESETPHASE);
}

/**
* Realiza una solicitud al módulo MFRC522 para detectar una tarjeta RFID.
* @param reqMode Modo de solicitud de tarjeta (por ejemplo, PICC_REQIDL).
* @param TagType Puntero al tipo de tarjeta detectada.
* @return El estado de la operación, representado por el tipo de dato TM_MFRC522_Status_t.
*          - Si se detecta una tarjeta correctamente, devuelve MI_OK.
*          - Si ocurre un error o no se detecta ninguna tarjeta, devuelve MI_ERR.
*/
static TM_MFRC522_Status_t TM_MFRC522_Request(uint8_t reqMode, uint8_t* TagType) {
	TM_MFRC522_Status_t status;  
	uint16_t backBits;			//The received data bits

	TM_MFRC522_WriteRegister(MFRC522_REG_BIT_FRAMING, 0x07);		//TxLastBists = BitFramingReg[2..0]	???

	TagType[0] = reqMode;
	status = TM_MFRC522_ToCard(PCD_TRANSCEIVE, TagType, 1, TagType, &backBits);

	if ((status != MI_OK) || (backBits != 0x10)) {    
		status = MI_ERR;
	}

	return status;
}

/**
* Realiza una operación de comunicación con una tarjeta RFID.
* @param command Comando a ejecutar (por ejemplo, PCD_AUTHENT o PCD_TRANSCEIVE).
* @param sendData Puntero a los datos que se enviarán a la tarjeta.
* @param sendLen Longitud de los datos a enviar.
* @param backData Puntero donde se almacenarán los datos recibidos de la tarjeta.
* @param backLen Puntero donde se almacenará la longitud de los datos recibidos.
* @return El estado de la operación, representado por el tipo de dato TM_MFRC522_Status_t.
*          - Si la operación se realiza correctamente, devuelve MI_OK.
*          - Si ocurre un error, devuelve MI_ERR.
*/
static TM_MFRC522_Status_t TM_MFRC522_ToCard(uint8_t command, uint8_t* sendData, uint8_t sendLen, uint8_t* backData, uint16_t* backLen) {
	TM_MFRC522_Status_t status = MI_ERR;
	uint8_t irqEn = 0x00;			//para MFRC522_REG_COMM_IE_N
	uint8_t waitIRq = 0x00;
	uint8_t lastBits;
	uint8_t n;
	uint16_t i;

	switch (command) {
		case PCD_AUTHENT: {
			irqEn = 0x12;					
			waitIRq = 0x10;			
			break;
		}
		case PCD_TRANSCEIVE: {
			irqEn = 0x77;					
			waitIRq = 0x30;
			break;
		}
		default:
			break;
	}

	TM_MFRC522_WriteRegister(MFRC522_REG_COMM_IE_N, irqEn | 0x80);	
	TM_MFRC522_ClearBitMask(MFRC522_REG_COMM_IRQ, 0x80);	
	TM_MFRC522_SetBitMask(MFRC522_REG_FIFO_LEVEL, 0x80);	

	TM_MFRC522_WriteRegister(MFRC522_REG_COMMAND, PCD_IDLE);		

	//Writing data to the FIFO
	for (i = 0; i < sendLen; i++) {   
		TM_MFRC522_WriteRegister(MFRC522_REG_FIFO_DATA, sendData[i]);   
	}

	//Execute the command
	TM_MFRC522_WriteRegister(MFRC522_REG_COMMAND, command);
	if (command == PCD_TRANSCEIVE) {    
		TM_MFRC522_SetBitMask(MFRC522_REG_BIT_FRAMING, 0x80);		//StartSend=1,transmission of data starts  
	}   

	//Waiting to receive data to complete
	i = 2000;	//i according to the clock frequency adjustment, the operator M1 card maximum waiting time 25ms???
	do {
		//CommIrqReg[7..0]
		//Set1 TxIRq RxIRq IdleIRq HiAlerIRq LoAlertIRq ErrIRq TimerIRq
		n = TM_MFRC522_ReadRegister(MFRC522_REG_COMM_IRQ);
		i--;
	} while ((i!=0) && !(n&0x01) && !(n&waitIRq));

	TM_MFRC522_ClearBitMask(MFRC522_REG_BIT_FRAMING, 0x80);			//StartSend=0

	if (i != 0)  {
		if (!(TM_MFRC522_ReadRegister(MFRC522_REG_ERROR) & 0x1B)) {		
			status = MI_OK;
			if (n & irqEn & 0x01) {   
				status = MI_NOTAGERR;			
			}

			if (command == PCD_TRANSCEIVE) {
				n = TM_MFRC522_ReadRegister(MFRC522_REG_FIFO_LEVEL);	
				lastBits = TM_MFRC522_ReadRegister(MFRC522_REG_CONTROL) & 0x07;	
				if (lastBits) {   
					*backLen = (n - 1) * 8 + lastBits;   
				} else {   														
					*backLen = n * 8;   									
				}																			

				if (n == 0) {							
					n = 1;    
				}
				if (n > MFRC522_MAX_LEN) {   
					n = MFRC522_MAX_LEN;   
				}

				//Reading the received data in FIFO
				for (i = 0; i < n; i++) {   
					backData[i] = TM_MFRC522_ReadRegister(MFRC522_REG_FIFO_DATA);    
				}
			}
		} else {   
			status = MI_ERR;  
		}
	}

	return status;
}

/**
* Realiza una operación de anticolisión con una tarjeta RFID.
* @param serNum Puntero donde se almacenará el número de serie de la tarjeta.
* @return El estado de la operación, representado por el tipo de dato TM_MFRC522_Status_t.
*          - Si la operación se realiza correctamente, devuelve MI_OK.
*          - Si ocurre un error, devuelve MI_ERR.
*/
static TM_MFRC522_Status_t TM_MFRC522_Anticoll(uint8_t* serNum) {
	TM_MFRC522_Status_t status;
	uint8_t i;
	uint8_t serNumCheck = 0;
	uint16_t unLen;

	TM_MFRC522_WriteRegister(MFRC522_REG_BIT_FRAMING, 0x00);		//TxLastBists = BitFramingReg[2..0]
	
	serNum[0] = PICC_ANTICOLL;
	serNum[1] = 0x20;
	status = TM_MFRC522_ToCard(PCD_TRANSCEIVE, serNum, 2, serNum, &unLen);

	if (status == MI_OK) {
		//Check card serial number
		for (i = 0; i < 4; i++) {   
			serNumCheck ^= serNum[i];
		}
		if (serNumCheck != serNum[i]) {   
			status = MI_ERR;    
		}
	}
	return status;
} 

/**
* Calcula el CRC (Cyclic Redundancy Check) de los datos proporcionados.
* @param pIndata Puntero a los datos de entrada.
* @param len Longitud de los datos de entrada.
* @param pOutData Puntero donde se almacenará el resultado del cálculo CRC.
*/
static void TM_MFRC522_CalculateCRC(uint8_t*  pIndata, uint8_t len, uint8_t* pOutData) {
	uint8_t i, n;

	TM_MFRC522_ClearBitMask(MFRC522_REG_DIV_IRQ, 0x04);			//CRCIrq = 0
	TM_MFRC522_SetBitMask(MFRC522_REG_FIFO_LEVEL, 0x80);			//Clear the FIFO pointer
	//Write_MFRC522(CommandReg, PCD_IDLE);

	//Writing data to the FIFO	
	for (i = 0; i < len; i++) {   
		TM_MFRC522_WriteRegister(MFRC522_REG_FIFO_DATA, *(pIndata+i));   
	}
	TM_MFRC522_WriteRegister(MFRC522_REG_COMMAND, PCD_CALCCRC);

	//Wait CRC calculation is complete
	i = 0xFF;
	do {
		n = TM_MFRC522_ReadRegister(MFRC522_REG_DIV_IRQ);
		i--;
	} while ((i!=0) && !(n&0x04));			//CRCIrq = 1

	//Read CRC calculation result
	pOutData[0] = TM_MFRC522_ReadRegister(MFRC522_REG_CRC_RESULT_L);
	pOutData[1] = TM_MFRC522_ReadRegister(MFRC522_REG_CRC_RESULT_M);
}

/**
* Selecciona una tarjeta RFID identificada por su número de serie.
* @param serNum Puntero al número de serie de la tarjeta.
* @return El tamaño de la tarjeta seleccionada.
*/
static uint8_t TM_MFRC522_SelectTag(uint8_t* serNum) {
	uint8_t i;
	TM_MFRC522_Status_t status;
	uint8_t size;
	uint16_t recvBits;
	uint8_t buffer[9]; 

	buffer[0] = PICC_SElECTTAG;
	buffer[1] = 0x70;
	for (i = 0; i < 5; i++) {
		buffer[i+2] = *(serNum+i);
	}
	TM_MFRC522_CalculateCRC(buffer, 7, &buffer[7]);		//??
	status = TM_MFRC522_ToCard(PCD_TRANSCEIVE, buffer, 9, buffer, &recvBits);

	if ((status == MI_OK) && (recvBits == 0x18)) {   
		size = buffer[0]; 
	} else {   
		size = 0;    
	}

	return size;
}

/**
* Autentica un bloque de memoria de una tarjeta RFID.
* @param authMode Modo de autenticación (por ejemplo, PCD_AUTHENT_A o PCD_AUTHENT_B).
* @param BlockAddr Dirección del bloque de memoria a autenticar.
* @param Sectorkey Puntero a la clave de sector.
* @param serNum Puntero al número de serie de la tarjeta.
* @return El estado de la operación, representado por el tipo de dato TM_MFRC522_Status_t.
*          - Si la autenticación se realiza correctamente, devuelve MI_OK.
*          - Si ocurre un error, devuelve MI_ERR.
*/
static TM_MFRC522_Status_t TM_MFRC522_Auth(uint8_t authMode, uint8_t BlockAddr, uint8_t* Sectorkey, uint8_t* serNum) {
	TM_MFRC522_Status_t status;
	uint16_t recvBits;
	uint8_t i;
	uint8_t buff[12]; 

	//Verify the command block address + sector + password + card serial number
	buff[0] = authMode;
	buff[1] = BlockAddr;
	for (i = 0; i < 6; i++) {    
		buff[i+2] = *(Sectorkey+i);   
	}
	for (i=0; i<4; i++) {    
		buff[i+8] = *(serNum+i);   
	}
	status = TM_MFRC522_ToCard(PCD_AUTHENT, buff, 12, buff, &recvBits);

	if ((status != MI_OK) || (!(TM_MFRC522_ReadRegister(MFRC522_REG_STATUS2) & 0x08))) {   
		status = MI_ERR;   
	}

	return status;
}

/**
* Lee datos de un bloque de memoria de una tarjeta RFID.
* @param blockAddr Dirección del bloque de memoria a leer.
* @param recvData Puntero donde se almacenarán los datos leídos.
* @return El estado de la operación, representado por el tipo de dato TM_MFRC522_Status_t.
*          - Si la lectura se realiza correctamente, devuelve MI_OK.
*          - Si ocurre un error, devuelve MI_ERR.
*/
static TM_MFRC522_Status_t TM_MFRC522_Read(uint8_t blockAddr, uint8_t* recvData) {
	TM_MFRC522_Status_t status;
	uint16_t unLen;

	recvData[0] = PICC_READ;
	recvData[1] = blockAddr;
	TM_MFRC522_CalculateCRC(recvData,2, &recvData[2]);
	status = TM_MFRC522_ToCard(PCD_TRANSCEIVE, recvData, 4, recvData, &unLen);

	if ((status != MI_OK) || (unLen != 0x90)) {
		status = MI_ERR;
	}

	return status;
}

/**
* Escribe datos en un bloque de memoria de una tarjeta RFID.
* @param blockAddr Dirección del bloque de memoria a escribir.
* @param writeData Puntero a los datos que se van a escribir.
* @return El estado de la operación, representado por el tipo de dato TM_MFRC522_Status_t.
*          - Si la escritura se realiza correctamente, devuelve MI_OK.
*          - Si ocurre un error, devuelve MI_ERR.
*/
static TM_MFRC522_Status_t TM_MFRC522_Write(uint8_t blockAddr, uint8_t* writeData) {
	TM_MFRC522_Status_t status;
	uint16_t recvBits;
	uint8_t i;
	uint8_t buff[18]; 

	buff[0] = PICC_WRITE;
	buff[1] = blockAddr;
	TM_MFRC522_CalculateCRC(buff, 2, &buff[2]);
	status = TM_MFRC522_ToCard(PCD_TRANSCEIVE, buff, 4, buff, &recvBits);

	if ((status != MI_OK) || (recvBits != 4) || ((buff[0] & 0x0F) != 0x0A)) {   
		status = MI_ERR;   
	}

	if (status == MI_OK) {
		//Data to the FIFO write 16Byte
		for (i = 0; i < 16; i++) {    
			buff[i] = *(writeData+i);   
		}
		TM_MFRC522_CalculateCRC(buff, 16, &buff[16]);
		status = TM_MFRC522_ToCard(PCD_TRANSCEIVE, buff, 18, buff, &recvBits);

		if ((status != MI_OK) || (recvBits != 4) || ((buff[0] & 0x0F) != 0x0A)) {   
			status = MI_ERR;   
		}
	}

	return status;
}

/**
* Pone en estado de hibernación a una tarjeta RFID.
*/
static void TM_MFRC522_Halt(void) {
	uint16_t unLen;
	uint8_t buff[4]; 

	buff[0] = PICC_HALT;
	buff[1] = 0;
	TM_MFRC522_CalculateCRC(buff, 2, &buff[2]);

	TM_MFRC522_ToCard(PCD_TRANSCEIVE, buff, 4, buff, &unLen);
}
