#include "bme680.h"

/*----------------------------------------------------------------------------
 *                         Salidas -> MsgQueue                               *
 *---------------------------------------------------------------------------*/
//osMessageQueueId_t mid_MsgQueue_BME680;
//MSGQUEUE_BME680_t localObject;


/*----------------------------------------------------------------------------
 *                      VARIABLES PARA TESTEAR EL MODULO                     *
 *---------------------------------------------------------------------------*/
float temp = 0;
float hum = 0;
float pres = 0;
float gas = 0;
int IAQ = 0;
int error;

/*----------------------------------------------------------------------------
 *                              Thread BME680                                *
 *---------------------------------------------------------------------------*/
osThreadId_t tid_ThBME680;                        // thread BME680 id

void Th_BME680 (void *argument);                   // thread function

int Init_Th_BME680 (void)
{
  tid_ThBME680 = osThreadNew(Th_BME680, NULL, NULL);
  
  if (tid_ThBME680 == NULL)
  {
    return(-1);
  }
  
  //mid_MsgQueue_BME680 = osMessageQueueNew(SIZE_MSGQUEUE_BME680 , sizeof(MSGQUEUE_BME680_t), NULL);
//  if (mid_MsgQueue_BME680 == NULL) {
//    return (-1);
//  }
 
  return(0);
}


void Th_BME680 (void *argument)
{
  uint8_t reg_data;
  
  BME680_Init(); // Se inicializa el sensor y el periferico I2C

  while (1)
  { // se vuelve a configurar solo la temperatura porque las medidas de humedad y gas dependen de ella
  // Se configura el sobremuestreo de temperatura (x4) y se selecciona el modo forzado
  reg_data = 0x6D;
  BME680_I2C_Write(BME680_I2C_ADDR, 0x74, &reg_data, 1);
    
  osDelay(800); //Se espera a que se estabilice la medida
    
  if((BME680_IsMeasuring(BME680_I2C_ADDR)) == 0) // cuando haya terminado de medir, que lea el valor T, H, e IAQ
  {
    temp = Get_temperature();
    hum = Get_humidity();
    gas = Get_gas_resistance();
    IAQ = Get_IAQ();
  }

  osDelay(1000); //esperamos 1 seg para volver a realizar una medida
  }
}


/*----------------------------------------------------------------------------
 *                              I2C Driver                                   *
 *---------------------------------------------------------------------------*/
extern ARM_DRIVER_I2C            Driver_I2C1;
static ARM_DRIVER_I2C *I2Cdrv = &Driver_I2C1;


/*----------------------------------------------------------------------------
 *                         BME680 Sensor Initialize                          *
 *---------------------------------------------------------------------------*/

void BME680_Init (void)
{
  uint8_t reg_data,id;
  
  /* Inicializar periférico I2C */
  I2Cdrv-> Initialize(NULL); // Se inicializa el periferico I2C
  I2Cdrv->PowerControl(ARM_POWER_FULL); // Power up the I2C peripheral
  I2Cdrv->Control(ARM_I2C_BUS_SPEED, ARM_I2C_BUS_SPEED_FAST);
  I2Cdrv->Control(ARM_I2C_BUS_CLEAR, 0);
  
  // Se chequea el ID del sensor
  BME680_I2C_Read(BME680_I2C_ADDR, 0xD0, &id, 1);
  if (id != 0x61)
  {
    error = 1;
  }
  
  // Se configura el sobremuestreo de temperatura (x4) y se selecciona el modo forzado
  reg_data = 0x6D;
  BME680_I2C_Write(BME680_I2C_ADDR, 0x74, &reg_data, 1);
  
  // Se configura el sobremuestreo de la humedad (x4)
  reg_data = 0x03;
  BME680_I2C_Write(BME680_I2C_ADDR, 0x72, &reg_data,1);
  
  // Filtro IIR --> Se selecciona el coeficiente para el filtro
  //reg_data = 0x08; // respuesta rapida pero con algo de ruido
  reg_data = 0x04; // medida mas precisa pero mas lento
  BME680_I2C_Write(BME680_I2C_ADDR, 0x75, &reg_data,1);
  
  // Se configuran los parametros para medir gas y se habilita la medicion
  BME680_Gas_Config();
  osDelay(200);
}

 
/*----------------------------------------------------------------------------
 *                         FUNCIONES SENSOR BME680                           *
 *---------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------
 *                 Funcion Calculo Indice Calidad del Aire                   *
 *---------------------------------------------------------------------------*/
int Get_IAQ (void)
{
  int humidity_score, gas_score;
  int air_quality_score; // Indice de calidad del aire
//  char* IAQ_texto; // Indoor Air Quality (con texto)
  
  humidity_score = GetHumidityScore();
  gas_score = GetGasScore();
  
  // Se combinan los resultados para poner el valor de IAQ (0-100% donde 100% es buena calidad del aire)
  air_quality_score = humidity_score + gas_score;

  // Calculo del indice de calidad del aire
  air_quality_score = (100 - air_quality_score) * 5;
  
  // ESTO LO PONDRIA PARA QUE SE VISUALICE EN EL LCD COMO TEXTO
//  if (air_quality_score >= 301 && air_quality_score <= 500)
//  {
//    IAQ_texto = "Peligroso";
//  }
//  else if (air_quality_score >= 201 && air_quality_score <= 300)
//  {
//    IAQ_texto = "No saludable";
//  }
//  else if (air_quality_score >= 151 && air_quality_score <= 200 )
//  {
//    IAQ_texto = "Malo";
//  }
//  else if (air_quality_score >= 101 && air_quality_score <= 150 )
//  {
//    IAQ_texto = "Un poco malo";
//  }
//  else if (air_quality_score >=  51 && air_quality_score <= 100 )
//  {
//    IAQ_texto = "Moderado";
//  }
//  else if (air_quality_score >=  00 && air_quality_score <=  50 )
//  {
//    IAQ_texto = "Bueno";
//  }
  
  return air_quality_score;
}


/*----------------------------------------------------------------------------
 *                  Funcion Calculo Indice Humedad Relativa                  *
 *---------------------------------------------------------------------------*/
int GetHumidityScore (void) //Calculate humidity contribution to IAQ index
{
  int humidity_score; // Indice de humedad relativa
  float hum_reference = 40;
  
  float current_humidity = Get_humidity();
  
  if (current_humidity >= 38 && current_humidity <= 42) // Humedad dentro del rango optimo (+/- 5% respecto de la referencia)
  {
    humidity_score = 0.25 * 100; // Humedad relativa del 25%
  }
  else // Humedad por debajo del rango optimo
  {
    if (current_humidity < 38) // Si la humedad leida esta por debajo del 38%
    {
      humidity_score = 0.25 / hum_reference * current_humidity * 100; 
    }
    else // Humedad por encima del 42%
    {
      humidity_score = ((-0.25 / (100 - hum_reference) * current_humidity) + 0.416666) * 100;
    }
  }
  
  return humidity_score;
}


/*----------------------------------------------------------------------------
 *                      Funcion Calculo Indice Gases                         *
 *---------------------------------------------------------------------------*/
int GetGasScore (void) // Calculate Gas contribution to IAQ index
{
  int gas_score; // Indice de gases presentes en el aire
  float gas_reference;
  int gas_lower_limit = 10000; // Limite mala calidad del aire
  int gas_upper_limit = 300000; // Limite buena calidad del aire

  gas_reference = Get_gas_resistance() * 1000000; // Se multiplica por 1 millon porque gas_reference esta en ppm
  
  //Calculate gas contribution to IAQ index
  gas_score = (0.75 / (gas_upper_limit - gas_lower_limit) * gas_reference - (gas_lower_limit * (0.75 / (gas_upper_limit - gas_lower_limit)))) * 100.00;
  
  if (gas_score > 75)
  {
    gas_score = 75; // Sometimes gas readings can go outside of expected scale maximum
  }
  
  if (gas_score <  0)
  {
    gas_score = 0;  // Sometimes gas readings can go outside of expected scale minimum
  }
  
  return gas_score;
}


/*----------------------------------------------------------------------------
 *   Funcion que comprueba si ya ha terminado de medir todos los parametros  *
 *---------------------------------------------------------------------------*/
uint8_t BME680_IsMeasuring (uint8_t addr)
{
  uint8_t status;
  BME680_I2C_Read(addr, 0x1D, &status, 1); // 0x1D --> Registro MEAS_STATUS_0
  
  return (status & 0x20);
}


/*----------------------------------------------------------------------------
 *                        Funcion Calculo Temperatura                        *
 *---------------------------------------------------------------------------*/
float Get_temperature (void)
{
  // Variables para calcular la temperatura segun BOSCH
  float t_fine, temp_comp, var1, var2;
  uint16_t par_t1, par_t2;
  uint8_t par_t3;
  uint8_t par_t[2];
  uint8_t buff_t[3]; // buffer temperatura
  int32_t temp_adc;

  // Se lee el valor del registro de temperatura (temp_xlsb | temp_lsb | temp_msb) --> 0x24[7:4] | 0x23 | 0x22
  BME680_I2C_Read(BME680_I2C_ADDR, 0x22, buff_t, 3); // 0x22 -> TEMP_MSB
  
  // Read temperature ADC value
  temp_adc = (buff_t[0] << 12) | (buff_t[1] << 4) | (buff_t[2] >> 4);
  
  // Se leen los parametros de calibracion necesarios para el calculo de la temperatura
  BME680_I2C_Read(BME680_I2C_ADDR, 0xE9, par_t, 2); // 0xE9 -> par_t1 LSB
  par_t1 = (par_t[1]<<8)| par_t[0];
  BME680_I2C_Read(BME680_I2C_ADDR, 0x8A, par_t, 2); // 0x8A -> par_t2 LSB
  par_t2 = (par_t[1]<<8)| par_t[0];
  BME680_I2C_Read(BME680_I2C_ADDR, 0x8C, &par_t3, 1); // 0x8C -> par_t3
  
  // Calculo de las variables var1 y var2
  var1 = ((((float)temp_adc / 16384.0f) - ((float)par_t1 / 1024.0f)) * ((float)par_t2));
  var2 = (((((float)temp_adc / 131072.0f) - ((float)par_t1 / 8192.0f)) * (((float)temp_adc / 131072.0f) - ((float)par_t1 / 8192.0f))) * ((float)par_t3 * 16.0f));

  // t_fine value -> Para compensar la temperatura
  t_fine = (var1 + var2);

  // Temperatura compensada
  temp_comp = ((t_fine) / 5120.0f);

  return temp_comp;
}


/*----------------------------------------------------------------------------
 *                      Funcion Calculo Humedad Relativa                     *
 *---------------------------------------------------------------------------*/
float Get_humidity (void)
{
  float hum_comp, var1, var2, var3, var4, temp_comp; //t_fine
  uint16_t par_h1, par_h2, hum_adc;
  uint8_t par_h[8];
  uint8_t buff_h[2]; // buffer humedad
  
  // Se lee el valor del registro de humedad (hum_lsb | hum_msb) --> 0x26 | 0x25
  BME680_I2C_Read(BME680_I2C_ADDR, 0x25, buff_h, 2); // 0x25 -> HUM_MSB
  
  // Read humidity ADC value
  hum_adc = (buff_h[0]<< 8) | buff_h[1];
  
  // Se leen los parametros de calibracion necesarios para el calculo de la humedad
  BME680_I2C_Read(BME680_I2C_ADDR, 0xE1, par_h, 8); // 0xE1 -> par_h2 MSB
  par_h1 = ((uint16_t) par_h[2] << 4) | ((uint16_t)par_h[1] & 0x0F); 
  par_h2 = ((uint16_t)par_h[0] << 4) | ((uint16_t)par_h[1] >> 4);
  
  // Datos de temperatura compensados
  temp_comp = Get_temperature();
  var1 = (float)((float)hum_adc) - (((float)par_h1 * 16.0f) + (((float)par_h[3] / 2.0f) * temp_comp));
  var2 = var1 * ((float)(((float)par_h2 / 262144.0f) * (1.0f + (((float)par_h[4] / 16384.0f) * temp_comp) + (((float)par_h[5] / 1048576.0f) * temp_comp * temp_comp))));
  var3 = (float)par_h[6] / 16384.0f;
  var4 = (float)par_h[7] / 2097152.0f;
  
  // Humedad relativa compensada
  hum_comp = var2 + ((var3 + (var4 * temp_comp)) * var2 * var2);
  
  if (hum_comp > 100.0f) // La humedad relativa puede ser del 100% como maximo
  {
    hum_comp = 100.0f;
  }
  else if (hum_comp < 0.0f) // La humedad relativa puede ser del 0% como minimo
  {
    hum_comp = 0.0f;
  }

  return hum_comp;
}


/*----------------------------------------------------------------------------
 *                       Funcion Calculo Gases en Aire                       *
 *---------------------------------------------------------------------------*/
float Get_gas_resistance(void)
{
  int16_t gas_adc;
  uint8_t res_g[2]; // buffer gases 
  uint8_t range_sw_err, gas_range;
  float gas_res;
  float var1, var2,var3;
  
  const float lookup_k1_range[16] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, -0.8f, 0.0f, 0.0f, -0.2f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f};
  const float lookup_k2_range[16] = {0.0f, 0.0f, 0.0f, 0.0f, 0.1f, 0.7f, 0.0f, -0.8f, -0.1f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
  
  // Se lee el valor del registro de presion atmosferica (gas_r_lsb | gas_r_msb) --> 0x2B | 0x2A
  BME680_I2C_Read(BME680_I2C_ADDR, 0x2A, res_g, 2); // 0x2A -> GAS_R_MSB
    
  // Read gas ADC value
  gas_adc = ((uint16_t)res_g[1] >> 6) | ((uint16_t)res_g[0] << 2);
  
  // Se leen los parametros necesarios para el calculo de la presencia de gases en el aire
  gas_range = res_g[1] & 0x0F;
  BME680_I2C_Read(BME680_I2C_ADDR, 0x04, &range_sw_err, 1);

  // Calculo de las variables var1, var2 y var3
  var1 = (1340.0f + (5.0f *range_sw_err));
  var2 = (var1) * (1.0f + lookup_k1_range[gas_range] / 100.0f);
  var3 = 1.0f + (lookup_k2_range[gas_range] / 100.0f);
    
  // Resistencia del sensor de gas calculada
  gas_res = 1.0f / (float)(var3 * (0.000000125f) * (float)gas_range * ((((float)gas_adc- 512.0f) / var2) + 1.0f));
  gas_res = gas_res / 1000000; // En ppm

  return gas_res;
}


/*----------------------------------------------------------------------------
 *                   Funcion de escritura del SENSOR BME680                  *
 *---------------------------------------------------------------------------*/
void BME680_I2C_Write (uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint32_t len)
{
  uint32_t status;
  uint8_t buffer[len + 1];
  buffer[0] = reg_addr;
  
  for (uint32_t i = 0; i < len; i++)
  {
    buffer[i + 1] = data[i];
  }
  
  status = I2Cdrv->MasterTransmit(dev_addr, buffer, len + 1, true);
  
  while (I2Cdrv->GetStatus().busy);
  
  if (status != ARM_I2C_EVENT_TRANSFER_DONE)
  {
    error = 2;
  }
}


/*----------------------------------------------------------------------------
 *                    Funcion de lectura del SENSOR BME680                   *
 *---------------------------------------------------------------------------*/
void BME680_I2C_Read (uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint32_t len)
{
  uint32_t status;
  
  status = I2Cdrv->MasterTransmit(dev_addr, &reg_addr, 1, true);
  
  while (I2Cdrv->GetStatus().busy);
  
  if (status != ARM_I2C_EVENT_TRANSFER_DONE)
  {
    error = 3;
  }
  
  status = I2Cdrv->MasterReceive(dev_addr, data, len, false);
  
  if (status != ARM_I2C_EVENT_TRANSFER_DONE)
  {
    error = 4;
  }
  
  while (I2Cdrv->GetStatus().busy);
}


/*----------------------------------------------------------------------------
 *          Funcion que configura los parametros iniciales del gas           *
 *---------------------------------------------------------------------------*/
void BME680_Gas_Config (void)
{
  uint8_t reg_data;
  
  // Se configura el tiempo de calentamiento del sensor -> 100 ms
  reg_data = 0x59;
  BME680_I2C_Write(BME680_I2C_ADDR, 0x64, &reg_data, 1);
  
  // Se configura la resistencia de calentamiento
  reg_data = calc_res_heat();
  BME680_I2C_Write(BME680_I2C_ADDR, 0x5A, &reg_data, 1);
  
  // Se habilita la lectura de gases: run --> 1 y nb_conv = 0
  reg_data = 0x10;
  BME680_I2C_Write(BME680_I2C_ADDR, 0x71, &reg_data, 1);
}
 

/*----------------------------------------------------------------------------
 *      Funcion que calcula el valor de la Tª de la resistencia del gas      *
 *---------------------------------------------------------------------------*/
uint8_t calc_res_heat (void)
{
  uint8_t res_heat_x;
  float temp_comp, var1,var2,var3, var4, var5;
  uint8_t par_g[4];
  int16_t par_g2;
  uint8_t res_heat_range_reg, res_heat_range, res_heat_val;
  
  // Se leen los parametros de calibracion  necesarios para el calculo de la temperatura en la resistencia
  BME680_I2C_Read(BME680_I2C_ADDR, 0xEB, par_g, 4); // 0xEB -> par_g2 LSB
  par_g2 = (par_g[1] << 8)| par_g[0];
  BME680_I2C_Read(BME680_I2C_ADDR, 0x00, &res_heat_val, 1); // 0x00 -> res_heat_val
  BME680_I2C_Read(BME680_I2C_ADDR, 0x02, &res_heat_range_reg, 1); // 0x02 -> res_heat_range
  
  res_heat_range = (res_heat_range_reg & 0x30) >> 4;
  temp_comp = Get_temperature();

  // Calculo de las variables var1, var2, var3, var4 y var5
  var1 = (((float)par_g[2] / (16.0f)) + 49.0f);
  var2 = ((((float)par_g2 / (32768.0f)) * (0.0005f)) + 0.00235f);
  var3 = ((float)par_g[3] / (1024.0f));
  var4 = (var1 * (1.0f + (var2 * (float)250)));
  var5 = (var4 + (var3 * (float)temp_comp));
  
  // Temperatura de la resistencia calculada
  res_heat_x = (uint8_t)(3.4f * ((var5 * (4 / (4 + (float)res_heat_range)) * (1 / (1 + ((float)res_heat_val * 0.002f)))) - 25));

  return res_heat_x;
}
