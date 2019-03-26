
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  ** This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * COPYRIGHT(c) 2019 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f4xx_hal.h"
#include "adc.h"
#include "dma.h"
#include "i2c.h"
#include "i2s.h"
#include "iwdg.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* USER CODE BEGIN Includes */
#include <string.h>
#include "I2C.h"
#include "math.h"
#include "I2C_LCD.h"
/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

/* USER CODE END PFP */

/* USER CODE BEGIN 0 */
void init_timing();
void get_distance();
void udelay(uint32_t useconds);

typedef enum state_t {                                                                               // the struct with states for rangefinder
	 IDLE_S,
	 TRIGGERING_S,
	 WAITING_FOR_ECHO_START_S,
	 WAITING_FOR_ECHO_STOP_S,
	 TRIG_NOT_WENT_LOW_S,
	 ECHO_TIMEOUT_S,
	 ECHO_NOT_WENT_LOW_S,
	 READING_DATA_S,
	 ERROR_S
} state_t;

char str[18];
uint32_t ADC_BUF[2];                                                                                 // the buffer to store the data from adc
uint8_t information[4] = {0, 0, 0, 0};
int last_id = -1;

int level = -1;                                                                                      // the level of water
int lighting = -1;                                                                                   // the current lighting level
int humidity = -1;																					 // the current humidity level
int previous_lcd_reload_period = 0;                                                                  // the last time display was reloaded
int previous_data_updated_period = 0;                                                                // the last time the data on display was updated
int ip_first = 0, ip_second = 0, ip_third = 0, ip_fourth = 0;

volatile state_t state = IDLE_S;                                                                     //  checks rangefinder state
volatile uint32_t echo_start;                                                                        //  the time when signal starts
volatile uint32_t echo_finish;                                                                       //  the time when signal ends
volatile uint32_t measured_time;                                                                     //  the time that signal travels
volatile uint32_t tim10_overflows = 0;                                                               //  the variable to null 10th timer

#define SIZE_OF_CHECKED_DATA                    10000                                                // the number of values to get the middle one
#define MINIMUM_LIGHTING                        0                                                    // the minimum adc lighting level
#define MAXIMUM_LIGHTING                        1200                                                 // the maximum adc lighting value
#define MINIMUM_HUMIDITY                        0                                                    // the minimum adc humidity value
#define MAXIMUM_HUMIDITY                        3100                                                 // the maximum adc humidity value
#define USE_TIM10_TIMING                        1                                                    // defined to use the 10th time
#define TIME_TO_UPDATE_LCD_DATA                 1000												 // the period to update the data on lcd (milliseconds)
#define TIME_TO_RELOAD_LCD                      10000												 // the period to fully reload lcd (milliseconds)
#define SYSTEM_CORE_CLOCK_IN_MHZ                (SystemCoreClock / 1000000)							 // the system core frequency
#define LCD_NOT_RELOADED_PERIOD                 (HAL_GetTick() - previous_lcd_reload_period)         // the period of time while display wasn't reloaded
#define LCD_DATA_NOT_UPDATED_PERIOD             (HAL_GetTick() - previous_data_updated_period)       // the period of time while data on display wasn't updated

// the function scale the value of lighting and humidity from adc to scale from 0 to 100
uint32_t convert(uint32_t value, uint32_t max, uint32_t min){
	value = value - min;
	if (value <= 0)
		return 0;

	uint32_t step = (uint32_t)(max - min) / 100;
	uint32_t converted = (uint32_t)(value / step);

	if(converted > 100) return 100;
	if(converted < 0) return 0;

	return converted;
}

// the function get the arithmetic mean of the data, using the SIZE_OF_CHECKED_DATA
// as the total number of observed data
uint32_t get_middle_value(uint32_t* array){
	uint32_t middle_value = 0;
	uint32_t sum = 0;

	for(int i = 0; i < SIZE_OF_CHECKED_DATA; i++){
		sum += *(array + i);
	}

	middle_value = (uint32_t)(sum / SIZE_OF_CHECKED_DATA);

	return middle_value;
}

// the function gets values of lighting and humidity from adc applying to them
// get_middle_value() function
uint32_t* get_data(){
	  uint32_t adcResultsLighting[SIZE_OF_CHECKED_DATA] = {0};
	  uint32_t adcResultsHumidity[SIZE_OF_CHECKED_DATA] = {0};

	  for(int i = 0; i < SIZE_OF_CHECKED_DATA; i++){
		  HAL_ADC_Start_DMA(&hadc1, (uint32_t*)ADC_BUF, 2);
		  HAL_ADC_Start_IT(&hadc1);
		  adcResultsLighting[i] = ADC_BUF[0];
		  adcResultsHumidity[i] = ADC_BUF[1];
	  }

	  static uint32_t middleValues[2];
	  middleValues[0] = get_middle_value(adcResultsLighting);
	  middleValues[1] = get_middle_value(adcResultsHumidity);

	  return middleValues;
}

// the function to print the
// main data on it
void initialize_and_update_display(int addr){
	//LCD_Print(0, "  Flower - Station  ");
	LCD_Init(addr<<1, hi2c3);
	init_timing();
	LCD_ADDR = addr<<1;
	HAL_GPIO_WritePin(TRIG_GPIO_Port, TRIG_Pin, GPIO_PIN_RESET);
	LCD_Clear();
	LCD_PPrint(0, 0, "Light: ");
	LCD_PPrint(1, 0,  "Humid: ");
	//LCD_Print(3, "Water level:       %");
}


// the function gets converted lighting and humidity and stores them
// to the glodal variables
void get_lighting_and_huminity(){
	uint32_t* data = get_data();
	lighting = convert(data[0], MAXIMUM_LIGHTING, MINIMUM_LIGHTING);
	humidity = convert(data[1], MAXIMUM_HUMIDITY, MINIMUM_HUMIDITY);
}


// the function gets the level of water using the formula which
// depends on the height of water container
void get_water_level(){
	get_distance();
	uint32_t distance = measured_time/58;
	level = ceill((11.5 - distance) * 8.7);
	if(level > 100) level = 100;
	if(level < 0) level = 0;
}


// the function gets distance using busy loops
void get_distance(){
	HAL_GPIO_WritePin(TRIG_GPIO_Port, TRIG_Pin, GPIO_PIN_SET);
	udelay(16);
	HAL_GPIO_WritePin(TRIG_GPIO_Port, TRIG_Pin, GPIO_PIN_RESET);

	state = WAITING_FOR_ECHO_START_S;
	while (state == WAITING_FOR_ECHO_START_S && state != ERROR_S){}
	while (state == WAITING_FOR_ECHO_STOP_S && state != ERROR_S ){}
}


// the function sends data to esp8266 using UART protocol
void send_data_to_esp(){
	uint32_t buffer_data = lighting * 5000 + level;
	char buffer[11];
	snprintf(buffer, sizeof buffer, "%lu\n", buffer_data);
	HAL_UART_Transmit(&huart1, buffer, strlen(buffer), HAL_MAX_DELAY);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Instance == TIM10){ ++tim10_overflows; }
}

void TIM10_reinit()
{
	 HAL_TIM_Base_Stop(&htim10);
	 __HAL_TIM_SET_PRESCALER( &htim10, (SYSTEM_CORE_CLOCK_IN_MHZ - 1) );
	 __HAL_TIM_SET_COUNTER( &htim10, 0 );
	 tim10_overflows = 0;
	 HAL_TIM_Base_Start_IT(&htim10);
}

uint32_t get_tim10_us()
{
	 __HAL_TIM_DISABLE_IT(&htim10, TIM_IT_UPDATE);
	 //__disable_irq();
	 uint32_t res = tim10_overflows * 10000 + __HAL_TIM_GET_COUNTER(&htim10);
	 //__enable_irq();
	 __HAL_TIM_ENABLE_IT(&htim10, TIM_IT_UPDATE);
	 return res;
}

void udelay_TIM10(uint32_t useconds) {
	 uint32_t before = get_tim10_us();
	 while( get_tim10_us() < before + useconds){}
}

void init_timing()
{
	#ifdef  USE_HAL_DELAY_AND_ASM
		return;
	#elif defined USE_DWT_TIMING
		DWT_Init();
	#elif defined USE_TIM10_TIMING
		TIM10_reinit();
	#else
		#error "Unknown timing method."
	#endif
}

uint32_t get_us()
{
	#ifdef  USE_HAL_DELAY_AND_ASM
		return 1000*HAL_GetTick();
	#elif defined USE_DWT_TIMING
		return get_DWT_us();
	#elif defined USE_TIM10_TIMING
		return get_tim10_us();
	#else
		#error "Unknown timing method."
	#endif
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	 if (GPIO_Pin == ECHOI_Pin)
	 {
		 switch (state) {
		 case WAITING_FOR_ECHO_START_S: {
			 echo_start = get_us();
			 state = WAITING_FOR_ECHO_STOP_S;
			 break;
		 }
		 case WAITING_FOR_ECHO_STOP_S: {
			 echo_finish = get_us();
			 measured_time = echo_finish - echo_start;
			 state = READING_DATA_S;
			 break;
		 }
		 default:
			 state = ERROR_S;
		 }
	 }
}

void udelay(uint32_t useconds)
{
	#ifdef  USE_HAL_DELAY_AND_ASM
	 	 udelay_asm(useconds);
	#elif defined USE_DWT_TIMING
	 	 udelay_DWT(useconds);
	#elif defined USE_TIM10_TIMING
	 	 udelay_TIM10(useconds);
	#else
		#error "Unknown timing method."
	#endif
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  *
  * @retval None
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
//	extern void initialise_monitor_handles(void);
//	initialise_monitor_handles();
  /* USER CODE END 1 */

  /* MCU Configuration----------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_11, 1);
  MX_DMA_Init();
  MX_I2C1_Init();
  MX_I2S2_Init();
  MX_I2S3_Init();
  MX_SPI1_Init();
  MX_ADC1_Init();
  MX_TIM3_Init();
  MX_I2C3_Init();
  MX_TIM10_Init();
  MX_TIM11_Init();
  MX_IWDG_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  HAL_ADC_Start_DMA(&hadc1, (uint32_t*)ADC_BUF, 2);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);



  int carr = 0x3f;
  initialize_and_update_display(carr);
  LCD_PPrint(0, 0, "Ip is loading...");
  int watered = 0;
  int pump_started = 0;
  int time_start = HAL_GetTick();
  int level_updated = HAL_GetTick();

  uint8_t str[20];
  HAL_UART_Receive_IT(&huart1,(uint8_t*) str, 20);
  //search_I2C_bus_without_semihosting(hi2c3);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	get_lighting_and_huminity();

	if(HAL_GetTick() - time_start <= 10000){
		HAL_UART_Receive_IT(&huart1, (uint8_t*) str, 20);
		if(LCD_DATA_NOT_UPDATED_PERIOD > 1000){
			char str_lcd[] = "                    ";
			int i = 0;
			while(*(str + i) != (uint8_t)'-'){
				*(str_lcd + i) = *(str + i);
				i++;
			}
			LCD_PPrint(0, 0, str_lcd);
			previous_data_updated_period = HAL_GetTick();
		}
	}else{
		if(LCD_DATA_NOT_UPDATED_PERIOD > TIME_TO_UPDATE_LCD_DATA){
				LCD_PPrintInt(0, 9, lighting, 10);
				LCD_PPrintInt(1, 9, humidity, 10);
				previous_data_updated_period = HAL_GetTick();
			}

			if(LCD_NOT_RELOADED_PERIOD > TIME_TO_RELOAD_LCD){
				LCD_Clear();
				initialize_and_update_display(carr);
				previous_lcd_reload_period = HAL_GetTick();
			}
	}

	if(HAL_GetTick() - level_updated > 3000){
		get_water_level();
		level_updated = HAL_GetTick();
	}

	if(level < 15){
		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_11, 1);
	}
	else{
		if (15 > humidity && humidity >= 0 && watered == 0){
			HAL_GPIO_WritePin(GPIOD, GPIO_PIN_11, 0);
			pump_started = HAL_GetTick();
			watered = 1;
		}
		if (watered && HAL_GetTick() - pump_started > 5000){
			HAL_GPIO_WritePin(GPIOD, GPIO_PIN_11, 1);
		}
		if (watered && HAL_GetTick() - pump_started > 10000){
			watered = 0;
		}
	}

	TIM3->CCR1 = 1000 - lighting * 10; // PWM

	send_data_to_esp();

	HAL_IWDG_Refresh(&hiwdg);

  /* USER CODE END WHILE */

  /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */

}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct;

    /**Configure the main internal regulator output voltage 
    */
  __HAL_RCC_PWR_CLK_ENABLE();

  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 192;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 8;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_I2S;
  PeriphClkInitStruct.PLLI2S.PLLI2SN = 200;
  PeriphClkInitStruct.PLLI2S.PLLI2SM = 5;
  PeriphClkInitStruct.PLLI2S.PLLI2SR = 2;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure the Systick interrupt time 
    */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    /**Configure the Systick 
    */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  file: The file name as string.
  * @param  line: The line in file as a number.
  * @retval None
  */
void _Error_Handler(char *file, int line)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  while(1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
