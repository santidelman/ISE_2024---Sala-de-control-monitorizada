#include "LCD.h"
#include "Arial12x12.h"

/* Private typedef -----------------------------------------------------------*/
GPIO_InitTypeDef GPIO_InitStruct_LCD;
TIM_HandleTypeDef TIM7_Handler;
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variable ---------------------------------------------------------*/
unsigned char buffer[512];
uint32_t positionL1 = 0;
uint32_t positionL2 = 256;
/* Manejadores ---------------------------------------------------------------*/
extern ARM_DRIVER_SPI Driver_SPI1;
ARM_DRIVER_SPI* SPIdrv = &Driver_SPI1;

void LCD_reset(void)
{
	delay(20);
	HAL_GPIO_WritePin(LCD_RESET_PORT, LCD_RESET_PIN, GPIO_PIN_RESET);
	delay(1000);
	HAL_GPIO_WritePin(LCD_RESET_PORT, LCD_RESET_PIN, GPIO_PIN_SET);
}

void delay(uint32_t n_microsegundos)
{
	__HAL_RCC_TIM7_CLK_ENABLE(); 							
	TIM7_Handler.Instance = TIM7;
	TIM7_Handler.Init.Prescaler = 83;
	TIM7_Handler.Init.Period = n_microsegundos - 1;
	
	HAL_TIM_Base_Init(&TIM7_Handler);
	HAL_TIM_Base_Start(&TIM7_Handler);
	
	while(TIM7 -> CNT < n_microsegundos - 1);
	HAL_TIM_Base_DeInit(&TIM7_Handler);
	HAL_TIM_Base_Stop(&TIM7_Handler);
}

void LCD_Initialize(void)
{
	SPIdrv -> Initialize(NULL);
	SPIdrv -> PowerControl(ARM_POWER_FULL);
	SPIdrv -> Control(ARM_SPI_MODE_MASTER | ARM_SPI_CPOL1_CPHA1 | ARM_SPI_MSB_LSB | ARM_SPI_DATA_BITS(8),1000000);
	
	/* Configuracion de la señal RESET como salida*/
	__HAL_RCC_GPIOA_CLK_ENABLE();
	GPIO_InitStruct_LCD.Pin = LCD_RESET_PIN;
	GPIO_InitStruct_LCD.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct_LCD.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(LCD_RESET_PORT, &GPIO_InitStruct_LCD);
	
	/* Configuracion de la señal CS como salida*/
	__HAL_RCC_GPIOD_CLK_ENABLE();
	GPIO_InitStruct_LCD.Pin = LCD_CS_PIN;
	GPIO_InitStruct_LCD.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct_LCD.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(LCD_CS_PORT, &GPIO_InitStruct_LCD);
	
	/* Configuracion de la señal A0 como salida*/
	__HAL_RCC_GPIOF_CLK_ENABLE();
	GPIO_InitStruct_LCD.Pin = LCD_A0_PIN;
	GPIO_InitStruct_LCD.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct_LCD.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(LCD_A0_PORT, &GPIO_InitStruct_LCD);
		
	HAL_GPIO_WritePin(LCD_RESET_PORT, LCD_RESET_PIN, GPIO_PIN_SET);
	HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_SET);
	HAL_GPIO_WritePin(LCD_A0_PORT, LCD_A0_PIN, GPIO_PIN_SET);
	
}
	
void LCD_wr_data(unsigned char data)
{
	ARM_SPI_STATUS status;
	
	HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LCD_A0_PORT, LCD_A0_PIN, GPIO_PIN_SET);
	SPIdrv -> Send(&data, sizeof(data));
	do{
		status = SPIdrv -> GetStatus();
	}while(status.busy);
	HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_SET);
}

void LCD_wr_cmd(unsigned char cmd)
{
	ARM_SPI_STATUS status;
	
	HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LCD_A0_PORT, LCD_A0_PIN, GPIO_PIN_RESET);
	SPIdrv -> Send(&cmd, sizeof(cmd));
	do{
		status = SPIdrv -> GetStatus();
	}while(status.busy);
	HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_SET);
}

/* PDF Diapositivas B2_LCD128x32 -> pagina 10 Y 11*/
void LCD_init(void)
{
	LCD_wr_cmd(0xAE);  //Comand(1)  -> Display OFF: 0xAE, Display ON: 0xAF
	LCD_wr_cmd(0xA2);  //Comand(11) -> Voltage bias ratio 1/9: 0xA2, Voltage bias ratio 1/7: 0xA3
	LCD_wr_cmd(0xA0);  //Comand(8)  -> Display RAM adress NORMAL: 0xA0, REVERSE: 0xA1
	LCD_wr_cmd(0xC8);  //Comand(1)  -> Display OFF: 0xAE, Display ON: 0xAF**********
	LCD_wr_cmd(0x22);  //Comand(17) -> Internar Resistor ratio (Rb/Ra) 0x2n -> n = Rb/Ra = 2 (nuestro caso)
	LCD_wr_cmd(0x2F);  //Comand(16) -> Power control set: 0x2F power on
	LCD_wr_cmd(0x40);  //Comand(2)  -> Display start line: 0x4n -> n: numero de linea = 0 (nuestro caso)
	LCD_wr_cmd(0xAF);  //Comand(1)  -> Display OFF: 0xAE, Display ON: 0xAF
	LCD_wr_cmd(0x81);  //Comand(18) -> Contrast
	LCD_wr_cmd(0x17);  //Comand(18) -> Contrasr value: 0x3F maximo contraste
	LCD_wr_cmd(0xA4);  //Comand(10) -> Display all points NORMAL: 0xA4, ALL POINTS ON: 0xA5
	LCD_wr_cmd(0xA6);  //Comand(9)  -> Display NORMAL: 0xA6, REVERSE: 0xA7
}

void LCD_update(void)
{
	int i;
	// PAGINA 0
	LCD_wr_cmd(0x00);
	LCD_wr_cmd(0x10);
	LCD_wr_cmd(0xB0);
	for(i = 0; i < 128; i++){
		LCD_wr_data(buffer[i]);
	}
	
	// PAGINA 1
	LCD_wr_cmd(0x00);
	LCD_wr_cmd(0x10);
	LCD_wr_cmd(0xB1);
	for(i = 128; i < 256; i++){
		LCD_wr_data(buffer[i]);
	}
	
	// PAGINA 2 
	LCD_wr_cmd(0x00);
	LCD_wr_cmd(0x10);
	LCD_wr_cmd(0xB2);
	for(i = 256; i < 384; i++){
		LCD_wr_data(buffer[i]);
	}
	
	// PAGINA 3
	LCD_wr_cmd(0x00);
	LCD_wr_cmd(0x10);
	LCD_wr_cmd(0xB3);
	for(i = 384; i < 512; i++){
		LCD_wr_data(buffer[i]);
	}
	
}

// Dibujamos un cuadrado de 8x8 en el LCD, en la pagina 0
void LCD_Cuadrado(void)
{
	buffer[0] = 0xFF;
	buffer[1] = 0x81;
	buffer[2] = 0x81;
	buffer[3] = 0x81;
	buffer[4] = 0x81;
	buffer[5] = 0x81;
	buffer[6] = 0x81;
	buffer[7] = 0xFF;
}

// Dibujamos una A de 8x8 en el LCD, en la pagina 2
void LCD_LetraA(void)
{
	buffer[256] = 0xFF;
	buffer[257] = 0x11;
	buffer[258] = 0x11;
	buffer[259] = 0x11;
	buffer[260] = 0x11;
	buffer[261] = 0x11;
	buffer[262] = 0x11;
	buffer[263] = 0xFF;
}

void LCD_symbolToLocalBuffer_L1(uint8_t symbol)
{
	uint8_t i, value1, value2;
	uint16_t offset = 0;
	
	offset = 25 * (symbol - ' ');
	
	for(i=0; i<12; i++){
		value1 = Arial12x12[offset+i*2+1];
		value2 = Arial12x12[offset+i*2+2];
		
		buffer[i+positionL1] = value1;
		buffer[i+128+positionL1] = value2;
	}
	positionL1 = positionL1 + Arial12x12[offset];
}

void LCD_symbolToLocalBuffer_L2(uint8_t symbol)
{
	uint8_t i, value1, value2;
	uint16_t offset = 0;
	
	offset = 25 * (symbol - ' ');
	
	for(i=0; i<12; i++){
		value1 = Arial12x12[offset+i*2+1];
		value2 = Arial12x12[offset+i*2+2];
		
		buffer[i+positionL2] = value1;
		buffer[i+128+positionL2] = value2;
	}
	positionL2 = positionL2 + Arial12x12[offset];
}

void LCD_WriteSentence(char *frase, uint8_t line)
{
	uint8_t i;
	if(line == 1){
		for(i=0; i<strlen(frase); i++){
			if(positionL1+12 < 127)
				LCD_symbolToLocalBuffer_L1(frase[i]);
		}
	}
	if(line == 2){
		for(i=0; i<strlen(frase); i++){
			if(positionL2+12 < 383)
				LCD_symbolToLocalBuffer_L2(frase[i]);
		}
	}
	positionL2 = 256;
	positionL1 = 0;
	LCD_update();
}

void LCD_clean(void)
{
	memset(buffer,0,512);
	LCD_update();
}
