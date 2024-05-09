#include "W25Q32.h"
#include "gpio.h"
#include "spi.h"

osThreadId_t TID_W25Q32;

extern ARM_DRIVER_SPI Driver_SPI3;
static ARM_DRIVER_SPI* SPI3drv = &Driver_SPI3;

int status_tranferencia =  0 ;
ARM_SPI_STATUS stat;

MedidaAmbiental medida_leida;

uint8_t data[sizeof(MedidaAmbiental)];
	
void Thread_FLASH (void *argument);                   // thread function
 
int Init_FLASH (void) {
 
  TID_W25Q32 = osThreadNew(Thread_FLASH, NULL, NULL);
  if (TID_W25Q32 == NULL) {
    return(-1);
  }
 
  return(0);
}
 
void Thread_FLASH(void *argument) {
    MedidaAmbiental medida = {osKernelGetTickCount(), 27.8f, 32.3f, 5.3f};
    uint8_t medida_bytes[sizeof(MedidaAmbiental)];
    memcpy(medida_bytes, &medida, sizeof(MedidaAmbiental));
    
    uint32_t page = 0;
    uint16_t offset = 0;
    uint32_t medida_address = (page * 256) + offset;
    
    // guarda la medida 
    W25Q32_WriteMedidaAmbiental(medida_address, &medida);
    
    // lee la medida 
    
    W25Q32_ReadMedidaAmbiental(medida_address, &medida_leida);
    
    printf("Medida leída:\n");
    printf("Timestamp: %u\n", medida_leida.timestamp);
    printf("Temperatura: %.1f °C\n", medida_leida.temperatura);
    printf("Humedad: %.1f %%RH\n", medida_leida.humedad);
    printf("CO: %.1f ppm\n", medida_leida.co);

    while (1) {
        osThreadYield();  // suspend thread
    }
}



////////////////////////////////////////////////////////////////////////////////////////



// RESTABLECER LA MEMORIA
void W25Q_Reset (void)
{
	uint8_t tData[2]; //Buffer para almacenar las instrucciones
	tData[0] = 0x66; // Almacenamos el comando de reinicio habilitacion
	tData[1] = 0x99; // Almacenamos el comando de reinicio 
		
	HAL_GPIO_WritePin(CS_W25Q32_PORT, CS_W25Q32_PIN, GPIO_PIN_RESET);
	SPI3drv->Send(tData, sizeof(tData));
  
}


void W25Q32_WriteEnable(void) {
    uint8_t cmd = 0x06; // comando WE
    HAL_GPIO_WritePin(CS_W25Q32_PORT, CS_W25Q32_PIN, GPIO_PIN_RESET); 
    SPI3drv->Send(&cmd, 1);
    while (SPI3drv->GetStatus().busy); // espera a que termine la tx
    HAL_GPIO_WritePin(CS_W25Q32_PORT, CS_W25Q32_PIN, GPIO_PIN_SET);
}


uint32_t W25Q32_ReadID(void) {
    uint8_t cmd[] = {0x9F, 0x00, 0x00, 0x00};
    uint8_t id[3] = {0, 0, 0};
    HAL_GPIO_WritePin(CS_W25Q32_PORT, CS_W25Q32_PIN, GPIO_PIN_RESET);
    SPI3drv->Send(cmd, 1);
    SPI3drv->Receive(&id, 3); // 3 bytes de ID
    while (SPI3drv->GetStatus().busy); // espera a que termine la rx
    HAL_GPIO_WritePin(CS_W25Q32_PORT, CS_W25Q32_PIN, GPIO_PIN_SET);
    return (id[0] << 16) | (id[1] << 8) | id[2];
}


void W25Q32_WaitForWriteEnd(void) {
    uint8_t cmd = 0x05;  // comando de lectura de registro de estado
    uint8_t status = 0x01;

    HAL_GPIO_WritePin(CS_W25Q32_PORT, CS_W25Q32_PIN, GPIO_PIN_RESET);
    SPI3drv->Send(&cmd, 1);  

    do {
        SPI3drv->Receive(&status, 1); 
    } while (status & 0x01);  // mientras se lea un 1 en el registro de estado es que está ocupado

    HAL_GPIO_WritePin(CS_W25Q32_PORT, CS_W25Q32_PIN, GPIO_PIN_SET);
}




void W25Q32_WriteData(uint32_t addr, uint8_t *data, uint32_t size) {
    W25Q32_WriteEnable();  // habilita la escritura 

    uint8_t cmd[4 + size];
    cmd[0] = 0x02;  // comando de escritura de página
    cmd[1] = (addr >> 16) & 0xFF;  // dir de memoria alta
    cmd[2] = (addr >> 8) & 0xFF;   // dir de memoria media
    cmd[3] = addr & 0xFF;          // dir de memoria baja

    for (uint32_t i = 0; i < size; i++) {
        cmd[4 + i] = data[i];  
    }

    HAL_GPIO_WritePin(CS_W25Q32_PORT, CS_W25Q32_PIN, GPIO_PIN_RESET);  
    SPI3drv->Send(cmd, 4 + size); 
    while (SPI3drv->GetStatus().busy);  // espera a que termine la tx
    HAL_GPIO_WritePin(CS_W25Q32_PORT, CS_W25Q32_PIN, GPIO_PIN_SET);

    
    W25Q32_WaitForWriteEnd();	// espera hasta que la memoria esté lista para otra op
}


void W25Q32_ReadData(uint32_t addr, uint8_t *buffer, uint32_t size) {
    uint8_t cmd[4] = {0x03, (addr >> 16) & 0xFF, (addr >> 8) & 0xFF, addr & 0xFF};
    HAL_GPIO_WritePin(CS_W25Q32_PORT, CS_W25Q32_PIN, GPIO_PIN_RESET);
    SPI3drv->Send(cmd, 4); 
    SPI3drv->Receive(buffer, size); 
    while (SPI3drv->GetStatus().busy); // espera a que termine la rx
    HAL_GPIO_WritePin(CS_W25Q32_PORT, CS_W25Q32_PIN, GPIO_PIN_SET);
}


void W25Q32_EraseSector(uint32_t addr) {
    W25Q32_WriteEnable(); //habilita la escritura
    uint8_t cmd[4] = {0x20, (addr >> 16) & 0xFF, (addr >> 8) & 0xFF, addr & 0xFF};
    HAL_GPIO_WritePin(CS_W25Q32_PORT, CS_W25Q32_PIN, GPIO_PIN_RESET);
    SPI3drv->Send(cmd, 4); 
    while (SPI3drv->GetStatus().busy); // espera a que termine la tx
    HAL_GPIO_WritePin(CS_W25Q32_PORT, CS_W25Q32_PIN, GPIO_PIN_SET);
    osDelay(400); // espera a que se borre el sector
}


void W25Q32_WriteMedidaAmbiental(uint32_t addr, MedidaAmbiental* medida) {
    
    memcpy(data, medida, sizeof(MedidaAmbiental));
    W25Q32_WriteData(addr, data, sizeof(MedidaAmbiental));
}

void W25Q32_WriteRegistroAcceso(uint32_t addr, RegistroAcceso* registro) {
    uint8_t data[sizeof(RegistroAcceso)];
    memcpy(data, registro, sizeof(RegistroAcceso));
    W25Q32_WriteData(addr, data, sizeof(RegistroAcceso));
}


void W25Q32_ReadMedidaAmbiental(uint32_t addr, MedidaAmbiental* medida) {
    
    W25Q32_ReadData(addr, data, sizeof(MedidaAmbiental));
    memcpy(medida, data, sizeof(MedidaAmbiental));
}

void W25Q32_ReadRegistroAcceso(uint32_t addr, RegistroAcceso* registro) {
    uint8_t data[sizeof(RegistroAcceso)];
    W25Q32_ReadData(addr, data, sizeof(RegistroAcceso));
    memcpy(registro, data, sizeof(RegistroAcceso));
}






//void W25Q_FastRead (uint32_t startPage, uint8_t offset, uint32_t size, uint8_t *rData)
//{
//	uint8_t tData[6]; //Buffer para almacenar los datos
//	uint32_t memAddr = (startPage*256) + offset; // calculamos la direccion de lectura
//	
//	if (numBLOCK < 1024)
//	{
//	tData[0] = 0x0B; // Almacenamos el comando de habilitadacion de la lectura rapida
//	tData[1] = (memAddr>>16)&0xFF; // Direcion de la memoria de 24 bits  MSB
//	tData[2] = (memAddr>>8)&0xFF; // Direcion de la memoria de 24 bits 
//	tData[3] = (memAddr)&0xFF; // Direcion de la memoria de 24 bits 	 LSB
//	tData[4] = 0;	// Dummy clock
//	}
//	else
//	{
//	tData[0] = 0x0B; // Almacenamos el comando de habilitadacion de la lectura rapida
//	tData[1] = (memAddr>>24)&0xFF; // Direcion de la memoria de 24 bits  MSB
//	tData[2] = (memAddr>>16)&0xFF; // Direcion de la memoria de 24 bits  
//	tData[3] = (memAddr>>8)&0xFF; // Direcion de la memoria de 24 bits 
//	tData[4] = (memAddr)&0xFF; // Direcion de la memoria de 24 bits 	 LSB
//	tData[5] = 0;	// Dummy clock
//	}
//	
//	 //csLOW();
//	HAL_GPIO_WritePin(CS_W25Q32_PORT, CS_W25Q32_PIN, GPIO_PIN_RESET);
//		if (numBLOCK < 1024)
//	{
//		//SPI_Write(tData,5);
//		SPI3drv->Send(tData, sizeof(tData));
//	}
//	else
//	{
//    //SPI_Write(tData,5);
//		SPI3drv->Send(tData, sizeof(tData));
////		  do{
////    stat = SPI3drv->GetStatus();
////  }while(stat.busy);
//			
//	}
//	
//	//SPI_Read(rData,size);
//	SPI3drv->Receive(rData, size);
//	 // do{
//   // stat = SPI3drv->GetStatus();
// // }while(stat.busy);
//		
//	 //csHIGH();
//	HAL_GPIO_WritePin(CS_W25Q32_PORT, CS_W25Q32_PIN, GPIO_PIN_SET);
//}







//void write_disable(void)
//{
//	uint8_t tData = 0x04;  // Desabilizacion de escritura
//		HAL_GPIO_WritePin  (CS_W25Q32_PORT, CS_W25Q32_PIN, GPIO_PIN_RESET); //csLOW();
//	//SPI_Write(&tData, 1);
//	SPI3drv->Send(&tData, sizeof(tData));
//  osThreadFlagsWait(TRANSFER_COMPLETE, osFlagsWaitAny, osWaitForever);
//	//HAL_GPIO_WritePin  (CS_W25Q32_PORT, CS_W25Q32_PIN, GPIO_PIN_SET); //csHIGH();
//	HAL_Delay (5);
//}



//void W25Q_Borrar_Sector (uint16_t numsector)
//{
//	uint8_t tData[6]; //Datos que se enviaran
//	uint32_t memAddr = numsector*16*256; //Calculamos la direccion de la memoria usando el numero del sector Numero de sector * 16 paginas * 256 bytes
//	
//	write_enable();
//	
//		if (numBLOCK < 1024)
//	{
//	tData[0] = 0x20; // Almacenamos el comando de habilitadacion de borrar sector
//	tData[1] = (memAddr>>16)&0xFF; // Direcion de la memoria de 24 bits  MSB
//	tData[2] = (memAddr>>8)&0xFF; // Direcion de la memoria de 24 bits 
//	tData[3] = (memAddr)&0xFF; // Direcion de la memoria de 24 bits 	 LSB

//			HAL_GPIO_WritePin  (CS_W25Q32_PORT, CS_W25Q32_PIN, GPIO_PIN_RESET); //csLOW();
//		//SPI_Write(tData, 4);
//		SPI3drv->Send(tData, sizeof(tData));
//    osThreadFlagsWait(TRANSFER_COMPLETE, osFlagsWaitAny, osWaitForever);
//		HAL_GPIO_WritePin  (CS_W25Q32_PORT, CS_W25Q32_PIN, GPIO_PIN_SET); //csHIGH();
//	}
//	else
//	{
//	tData[0] = 0x20; // Almacenamos el comando de habilitadacion de borrar sector
//	tData[1] = (memAddr>>24)&0xFF; // Direcion de la memoria de 24 bits  MSB
//	tData[2] = (memAddr>>16)&0xFF; // Direcion de la memoria de 24 bits  
//	tData[3] = (memAddr>>8)&0xFF; // Direcion de la memoria de 24 bits 
//	tData[4] = (memAddr)&0xFF; // Direcion de la memoria de 24 bits 	 LSB
//	
//			HAL_GPIO_WritePin  (CS_W25Q32_PORT, CS_W25Q32_PIN, GPIO_PIN_RESET); //csLOW();
//		//SPI_Write(tData, 5);
//		SPI3drv->Send(tData, sizeof(tData));
//    osThreadFlagsWait(TRANSFER_COMPLETE, osFlagsWaitAny, osWaitForever);
//		HAL_GPIO_WritePin  (CS_W25Q32_PORT, CS_W25Q32_PIN, GPIO_PIN_SET); //csHIGH();
//	}
//	
//	HAL_Delay (450); //Tiempo requerido para borrar un sector 400ms pag 63 data (tSE)
//	write_disable();
//}




//// page - Pagina que queremos escribir
//// offsset - Desplazamiento para la pagina de inicio varia de 0 a 255
//// size - Tamaño de los datos que queremos leer (el tamaño debe estar en bytes)
//// *data - Puntero de la matriz para los datos 
//// Funcion que escribe los datos mientras que tambien borra el sector, limpia todo el sector antes de escribir en el
//void W25Q_Write_Clean (uint32_t page, uint16_t offset, uint32_t size, uint8_t *data)
//	{
//	
//		uint8_t tData[266]; // Array de 266 bytes para enviar la instuccion, la direccion y los datos (solo se pueden escribir 256 bytes a la vez)
//		uint32_t startPage = page; // Pagina desde donde comenzamos a escribir datos
//		uint32_t endPage = startPage + ((size+offset-1)/256); // Pagina final donde  escribiremos datos
//		uint32_t numPages = endPage - startPage + 1; // Numero de paginas en las que escribiremos datos
//		
//		//  Antes de escribir los datos en la pagina debemos borrarlos. Borando un sector
//		
//		
//		// CAlcularemos el sector de inicio segun la pagina de inicio. Cada sector contiene 16 paginas
//		uint16_t startSector = startPage/16;
//		// CAlcularemos el sector de final segun la pagina de inicio. Cada sector contiene 16 paginas
//		uint16_t endSector = endPage/16;
//		// CAlcularemos el numero de sectores para borrar. Cada sector contiene 16 paginas
//		uint16_t numSectors = endSector - startSector + 1;
//		
//		//Se ejcutara tantas veces como sectores tengamos que borrar
//		for (uint16_t i=0; i<numSectors; i++)
//		{
//			W25Q_Borrar_Sector(startSector++);
//		}
//		
//		
//		uint32_t dataPosition = 0; // Seguimiento de las posicion de los datos dentro del puntero de datos
//		
//		// ESCRIBIMOS LOS DATOS
//		uint16_t bytesremainig = 0;
//		uint32_t index;
//		uint16_t bytestosend;
//		
//		for (uint32_t i=0; i<numPages; i++)
//		{
//			uint32_t memAddr = (startPage*256)+offset; // Calcularemos la direccion de memoria segun el numero de pagina y desplazamiento
//			bytesremainig = bytestowrite(size, offset);// Calcularemos cuantos bytes tenemos que enviar
//			write_enable();
//			index = 0; //almacenaremos cuandos bytes se han almacenado en el buffer
//			
//			
//			//Copiamos la instruccion y la direccion de memoria en el buffer
//					if (numBLOCK < 1024)
//						{
//							tData[0] = 0x02; // Progama de paginas
//							tData[1] = (memAddr>>16)&0xFF; // Direcion de la memoria de 24 bits  MSB
//							tData[2] = (memAddr>>8)&0xFF; // Direcion de la memoria de 24 bits 
//							tData[3] = (memAddr)&0xFF; // Direcion de la memoria de 24 bits 	 LSB
//							index = 4;
//						}
//					else
//						{
//							tData[0] = 0x12; // Almacenamos el comando de habilitadacion de borrar sector
//							tData[1] = (memAddr>>24)&0xFF; // Direcion de la memoria de 24 bits  MSB
//							tData[2] = (memAddr>>16)&0xFF; // Direcion de la memoria de 24 bits  
//							tData[3] = (memAddr>>8)&0xFF; // Direcion de la memoria de 24 bits 
//							tData[4] = (memAddr)&0xFF; // Direcion de la memoria de 24 bits 	 LSB
//							index = 5;
//						}
//			
//						bytestosend = bytesremainig + index;// Total de bytes que tenemos que enviar		
//		
//		
//				for (uint16_t i=0; i<bytesremainig; i++)
//		{		
//			tData[index++] = data[i+dataPosition];	
//		}
//		
//		if (bytestosend > 250)
//		{
//				HAL_GPIO_WritePin  (CS_W25Q32_PORT, CS_W25Q32_PIN, GPIO_PIN_RESET); //csLOW();
//			//SPI_Write(tData, 100);
//			SPI3drv->Send(tData, sizeof(tData));
//      osThreadFlagsWait(TRANSFER_COMPLETE, osFlagsWaitAny, osWaitForever);
//			//SPI_Write(tData+100, bytestosend-1);
//			SPI3drv->Send(tData+100, bytestosend-1);
//      osThreadFlagsWait(TRANSFER_COMPLETE, osFlagsWaitAny, osWaitForever);
//			HAL_GPIO_WritePin  (CS_W25Q32_PORT, CS_W25Q32_PIN, GPIO_PIN_SET); //csHIGH();
//		}
//		else
//			{
//				HAL_GPIO_WritePin  (CS_W25Q32_PORT, CS_W25Q32_PIN, GPIO_PIN_RESET); //csLOW();
//	//SPI_Write(tData, bytestosend);
//				SPI3drv->Send(tData, bytestosend);
//        osThreadFlagsWait(TRANSFER_COMPLETE, osFlagsWaitAny, osWaitForever);
//			HAL_GPIO_WritePin  (CS_W25Q32_PORT, CS_W25Q32_PIN, GPIO_PIN_SET); //csHIGH();
//		}			

//		
//		startPage++;
//		offset=0;
//		size = size - bytesremainig;
//		dataPosition = dataPosition + bytesremainig;
//		HAL_Delay (5); //Tiempo requerido para programar una pagina pag 63 data (tPP)
//		write_disable();
//		}
//		
//	}

//uint16_t bytestowrite( uint32_t size, uint16_t offset)
//	{
//		if((size+offset)<256)
//		{
//			return size;
//		}
//		else
//		{
//			return 256-offset;
//		}
//		
//	}
////Calculamos cuantos bytes mas tenemos disponibles en el sector
//uint16_t bytestomodify( uint32_t size, uint16_t offset)
//{
//		if((size+offset)<4096)
//		{
//			return size;
//		}
//		else
//		{
//			return 4096-offset;
//		}
//		
//}



//	
//	// page - Pagina que queremos escribir
//// offsset - Desplazamiento para la pagina de inicio varia de 0 a 255
//// size - Tamaño de los datos que queremos leer (el tamaño debe estar en bytes)
//// *data - Puntero de la matriz para los datos 
//	// Funcion que actualiza los datos de un sector
//void W25Q_Write(uint32_t page, uint16_t offset, uint32_t size, uint8_t *data)
//	{

//		// CAlcularemos el sector de inicio . Cada sector contiene 16 paginas
//		uint16_t startSector = page/16;
//		// CAlcularemos el sector de final segun la pagina. Cada sector contiene 16 paginas
//		uint16_t endSector = page + ((size+offset-1)/256)/16;
//		// CAlcularemos el numero de sectores para borrar. Cada sector contiene 16 paginas
//		uint16_t numSectors = endSector - startSector + 1;
//		
//		uint8_t previusData[4096]; // Almacenamos los datos dl sector
//		uint32_t sectorOffset = ((page%16)*256) + offset; //Calculamos el desplazamiento del sector
//		uint32_t dataIndex = 0;
//		
//		uint32_t startPage;
//		uint16_t bytesRemeaning; // Cuantos Bytes podemos modificar en el sector actual
//		
//		//Se ejcutara tantas veces como sectores tengamos que borrar
//		for (uint16_t i=0; i<numSectors; i++)
//		{
//			startPage = startSector * 16;// CAlculamos la pagina de inicio, pagina donde el sector comienza. Puede ser 0 16 36 48....
//			W25Q_FastRead(startPage, 0, 4096, previusData); // Leemos todo el sector y los almacenamos
//			bytesRemeaning = bytestomodify(size, sectorOffset);
//			
//			//Modificamos los datos almacenados con los nuevos datos
//					for (uint16_t i=0; i<bytesRemeaning; i++)
//					{
//							previusData[i+sectorOffset] = data[i+dataIndex];
//					}	
//		W25Q_Write_Clean(	startPage, 0, 4096, previusData); // Borramos el sector y escribimos en el los datos actualizados
//		startSector++;
//		sectorOffset=0;
//		dataIndex = dataIndex +	bytesRemeaning;	
//		size = size - bytesRemeaning;
//					
//		}

//	}

//	
//	// Funcion para leer un nico byte
//uint8_t W25Q_Read_Byte(uint32_t Addr)
//	{
//			uint8_t tData[5]; //Buffer para almacenar los datos
//			uint8_t rData; //DAtos almacenados despues de la lectura
//	
//	if (numBLOCK < 1024)
//	{
//	tData[0] = 0x03; // Almacenamos el comando de habilitadacion de la lectura
//	tData[1] = (Addr>>16)&0xFF; // Direcion de la memoria de 24 bits  MSB
//	tData[2] = (Addr>>8)&0xFF; // Direcion de la memoria de 24 bits 
//	tData[3] = (Addr)&0xFF; // Direcion de la memoria de 24 bits 	 LSB
//	}
//	else
//	{
//	tData[0] = 0x13; // Almacenamos el comando de habilitadacion de la lectura
//	tData[1] = (Addr>>24)&0xFF; // Direcion de la memoria de 24 bits  MSB
//	tData[2] = (Addr>>16)&0xFF; // Direcion de la memoria de 24 bits  
//	tData[3] = (Addr>>8)&0xFF; // Direcion de la memoria de 24 bits 
//	tData[4] = (Addr)&0xFF; // Direcion de la memoria de 24 bits 	 LSB
//	}
//	
//		HAL_GPIO_WritePin  (CS_W25Q32_PORT, CS_W25Q32_PIN, GPIO_PIN_RESET); //csLOW();
//	
//		if (numBLOCK < 1024)
//	{
//		//SPI_Write(tData,4);
//			SPI3drv->Send(tData, sizeof(tData));
//	}
//	else
//	{
//    //SPI_Write(tData,5);
//			SPI3drv->Send(tData, sizeof(tData));
//	}
//	
//	//SPI_Read(&rData,1);
//	SPI3drv->Receive(&rData, sizeof(tData));
//	HAL_GPIO_WritePin  (CS_W25Q32_PORT, CS_W25Q32_PIN, GPIO_PIN_SET); //csHIGH();
//		
//		return rData;
//	}

//	
//	
//	
//		// Funcion para escribir un nico byte
//void W25Q_Write_Byte(uint32_t Addr, uint8_t data)
//	{
//			uint8_t tData[6]; //Buffer para almacenar los datos
//			uint8_t index; 
//	
//	if (numBLOCK < 1024)
//	{
//	tData[0] = 0x02; // Almacenamos el comando de habilitadacion de la  escritura
//	tData[1] = (Addr>>16)&0xFF; // Direcion de la memoria de 24 bits  MSB
//	tData[2] = (Addr>>8)&0xFF; // Direcion de la memoria de 24 bits 
//	tData[3] = (Addr)&0xFF; // Direcion de la memoria de 24 bits 	 LSB
//	tData[4] = data;
//	index = 5;
//		
//	}
//	else
//	{
//	tData[0] = 0x12; // Almacenamos el comando de habilitadacion de la escritura
//	tData[1] = (Addr>>24)&0xFF; // Direcion de la memoria de 24 bits  MSB
//	tData[2] = (Addr>>16)&0xFF; // Direcion de la memoria de 24 bits  
//	tData[3] = (Addr>>8)&0xFF; // Direcion de la memoria de 24 bits 
//	tData[4] = (Addr)&0xFF; // Direcion de la memoria de 24 bits 	 LSB
//	tData[5] = data;
//	index = 6;
//	}
//	
//	
//	if(W25Q_Read_Byte(Addr) == 0xFF) // Comprobamos que la direccion de memoria esta borrada
//	{
//		write_enable();
//		HAL_GPIO_WritePin  (CS_W25Q32_PORT, CS_W25Q32_PIN, GPIO_PIN_RESET); //csLOW();
//		//SPI_Write(tData, index);
//			SPI3drv->Send(tData, index);
//		HAL_GPIO_WritePin  (CS_W25Q32_PORT, CS_W25Q32_PIN, GPIO_PIN_SET); //csHIGH();
//		HAL_Delay (5); //Tiempo requerido para programar una pagina pag 63 data (tPP)
//		write_disable();
//	}

//	}
//	
//	
//	
//	
//	
//	
//	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	uint8_t tempBytes[4]; // Float ocupa 4 bytes de memoria
//	
//	
//	void W25Q_Write_NUM ( uint32_t page, uint16_t offset, float data)
//	{
//		 float2Bytes (tempBytes, data);// Convertimos el valor float en Bytes
//		
//		W25Q_Write(page, offset, 4, tempBytes);
//		
//	}
//	
//		float W25Q_Read_NUM ( uint32_t page, uint16_t offset)
//	{
//		
//		uint8_t rData[4];
//		W25Q_Read(page, offset, 4, rData);
//		return (Bytes2float(rData));
//		
//	}
//	
//	
//	
//	void float2Bytes (uint8_t *ftoa_bytes_temp, float float_variable)
//	{
//		union {
//			float a;
//				uint8_t bytes[4];
//		} thing;
//		
//		thing.a = float_variable;
//		for (uint8_t i =0 ; i < 4 ; i++)
//			{
//				ftoa_bytes_temp[i] = thing.bytes[i];
//			}
//		
//	}
//	
//		float Bytes2float (uint8_t *ftoa_bytes_temp)
//	{
//		union {
//			float a;
//				uint8_t bytes[4];
//		} thing;
//		
//		for (uint8_t i =0 ; i < 4 ; i++)
//			{
//				thing.bytes[i] = ftoa_bytes_temp[i];
//			}
//		float float_variable = thing.a;
//			return float_variable;
//			
//	}
//	
//	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	
//	
//	
//	void W25Q_Write_32B ( uint32_t page, uint16_t offset, uint32_t size, uint32_t *data)
//	{
//		uint8_t data8[size*4];
//		uint32_t index = 0;
//		
//				for (uint32_t i =0 ; i < size ; i++)
//			{
//				data8[index++] = data[i]&0xFF; //LSB
//				data8[index++] = (data[i]>>8)&0xFF;
//				data8[index++] = (data[i]>>16)&0xFF;
//				data8[index++] = (data[i]>>24)&0xFF;
//			}
//		
//			W25Q_Write(page, offset, index, data8);
//			
//	}
//	
//	
//	
//		void W25Q_Read_32B ( uint32_t page, uint16_t offset, uint32_t size, uint32_t *data)
//	{
//		uint8_t data8[size*4];
//		uint32_t index = 0;
//		uint32_t aux = 0;
//		
//		W25Q_FastRead(page, offset, size*4, data8);
//		
//				for (uint32_t i =0 ; i < size ; i++)
//			{
//				aux = index++;
//				
//				data[i] = (data8[aux]) | (data8[aux]<<8)| (data8[aux]<<16)| (data8[aux]<<24);
//			}
//		
//			
//			
//	}
//	
//	
//	
//	
//	
//	
//	void W25Q_Erase_Sector (uint16_t numsector)
//{
//	uint8_t tData[6];
//	uint32_t memAddr = numsector*16*256;   // Each sector contains 16 pages * 256 bytes

//	write_enable();

//	if (numBLOCK<512)   // Chip Size<256Mb
//	{
//		tData[0] = 0x20;  // Erase sector
//		tData[1] = (memAddr>>16)&0xFF;  // MSB of the memory Address
//		tData[2] = (memAddr>>8)&0xFF;
//		tData[3] = (memAddr)&0xFF; // LSB of the memory Address

//				//csLOW();  // pull the CS LOW
//		HAL_GPIO_WritePin(CS_W25Q32_PORT, CS_W25Q32_PIN, GPIO_PIN_RESET);;
//		//SPI_Write(tData, 4);
//		SPI3drv->Send(tData, sizeof(tData));
//				//csHIGH();  // pull the HIGH
//		HAL_GPIO_WritePin(CS_W25Q32_PORT, CS_W25Q32_PIN, GPIO_PIN_SET);
//	}
//	else  // we use 32bit memory address for chips >= 256Mb
//	{
//		tData[0] = 0x21;  // ERASE Sector with 32bit address
//		tData[1] = (memAddr>>24)&0xFF;
//		tData[2] = (memAddr>>16)&0xFF;
//		tData[3] = (memAddr>>8)&0xFF;
//		tData[4] = memAddr&0xFF;

//		//csLOW();  // pull the CS LOW
//		HAL_GPIO_WritePin(CS_W25Q32_PORT, CS_W25Q32_PIN, GPIO_PIN_RESET);
//		//SPI_Write(tData, 5);
//		SPI3drv->Send(tData, sizeof(tData));
//		//csHIGH();  // pull the HIGH
//		HAL_GPIO_WritePin(CS_W25Q32_PORT, CS_W25Q32_PIN, GPIO_PIN_SET);
//	}

//	HAL_Delay(450);  // 450ms delay for sector erase

//	write_disable();

//}

//	
//	
//	
//	
//	
//	
//	
//	
//	
//	
//	
//	
//	
//	
//	
//	
//	
//	
//	
//	


