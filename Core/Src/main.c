/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "usb_device.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "usbd_hid.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */
extern USBD_HandleTypeDef hUsbDeviceFS;

typedef struct
{
	uint8_t modifier;
	uint8_t reserved;
	uint8_t keykode1;
	uint8_t keykode2;
	uint8_t keykode3;
	uint8_t keykode4;
	uint8_t keykode5;
	uint8_t keykode6;
} keyboard_HID_t;

keyboard_HID_t kbhid = {0,0,0,0,0,0,0,0};
uint8_t cl = 0;
uint8_t press_matrix[8][9] = {0};
uint16_t kb_matrix[8][9] = {0};
uint8_t lshift = 0;
uint8_t rshift = 0;
uint8_t lalt = 0;
uint8_t ralt = 0;
uint8_t lctrl = 0;
uint8_t rctrl = 0;
uint8_t lwin = 0;
uint8_t rwin = 0;
uint8_t fn = 0;

//    0   1   2   3   4       5       6       7       8
//0: "3","5","7","9","-",  "BKSP","P.SCR",     "",     "1",
//1: "2","4","6","8","0",     "=",  "INS",     "",   "ESC",
//2: "e","t","u","o","[",    "\\","PAUSE", "LALT",     "q",
//3: "w","r","y","i","p",     "]",  "DEL","SPACE",   "TAB",
//4: "s","f","h","k",";", "ENTER","RIGHT",   "FN",     "~",
//5: "a","d","g","j","l",     "'","LCTRL","RCTRL","CAPS.L",
//6: "x","v","n",",","/",    "UP", "DOWN", "RALT",   "WIN",
//7: "z","c","b","m",".","LSHIFT", "LEFT","SPACE","RSHIFT" 

uint8_t val_matrix[8][9] = 
  { 0x20, 0x22, 0x24, 0x26, 0x2D, 0x2A, 0x46, 0x00, 0x1E,
	  0x1F, 0x21, 0x23, 0x25, 0x27, 0x2E, 0x49, 0x00, 0x29,
	  0x08, 0x17, 0x18, 0x12, 0x2F, 0x32, 0x48, 0xE2, 0x14,
	  0x1A, 0x15, 0x1C, 0x0C, 0x13, 0x30, 0x4C, 0x00, 0x2B,
	  0x16, 0x09, 0x0B, 0x0E, 0x33, 0x28, 0x4F, 0x00, 0x35,
	  0x04, 0x07, 0x0A, 0x0D, 0x0F, 0x34, 0xE0, 0xE4, 0x39,
	  0x1B, 0x19, 0x11, 0x36, 0x38, 0x52, 0x51, 0xE6, 0xE3,
	  0x1D, 0x06, 0x05, 0x10, 0x37, 0xE1, 0x50, 0x2C, 0xE5};

uint8_t fn_matrix[8][9] = 
  { 0x3C, 0x3E, 0x40, 0x42, 0x44, 0x2A, 0x81, 0x00, 0x3A,
	  0x3B, 0x3D, 0x3F, 0x41, 0x43, 0x45, 0x49, 0x00, 0x35,
	  0x08, 0x17, 0x18, 0x12, 0x2F, 0x32, 0x80, 0x00, 0x14,
	  0x1A, 0x15, 0x1C, 0x0C, 0x13, 0x30, 0x4C, 0x00, 0x2B,
	  0x16, 0x09, 0x0B, 0x0E, 0x33, 0x28, 0x4D, 0x00, 0x35,
	  0x04, 0x07, 0x0A, 0x0D, 0x0F, 0x34, 0x00, 0x00, 0x39,
	  0x1B, 0x19, 0x11, 0x36, 0x38, 0x4B, 0x4E, 0x00, 0xE7,
	  0x1D, 0x06, 0x05, 0x10, 0x37, 0x00, 0x4A, 0x2C, 0x00};
	

char str_matrix[8][9][1]=
  {"3","5","7","9","-","\x08","\x07", "",   "1",
   "2","4","6","8","0",   "=",    "", "","\x1B",
   "e","t","u","o","[",  "\\",    "", "",   "q",
   "w","r","y","i","p",   "]","\x7f"," ",  "\t",
   "s","f","h","k",";",  "\n",    "", "",   "`",
   "a","d","g","j","l",   "'",    "", "",    "",
   "x","v","n",",","/",    "",    "", "",    "",
   "z","c","b","m",".",    "",    ""," ",    "" };
char shift_matrix[8][9][1]=
  {"#","%","&","(","_","\x08","\x07", "",   "!",
   "2","$","^","*",")",   "+",    "", "","\x1B",
   "E","T","U","O","{",   "|",    "", "",   "Q",
   "W","R","Y","I","P",   "}","\x7f"," ",  "\t",
   "S","F","H","K",":",  "\n",    "", "",   "~",
   "A","D","G","J","L",  "\"",    "", "",    "",
   "X","V","N","<","?",    "",    "", "",    "",
   "Z","C","B","M",">",    "",    ""," ",    "" };

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void clear_columns(){
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);
}

void toggle_column(){
	if (cl == 0) HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_SET);
	else if (cl == 1) HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_SET);
	else if (cl == 2) HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET);
	else if (cl == 3) HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET);
	else if (cl == 4) HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_SET);
	else if (cl == 5) HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_SET);
	else if (cl == 6) HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);
	else if (cl == 7) HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_SET);
	else if (cl == 8) HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET);
  cl++;
	if (cl > 8) cl = 0;
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

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
  MX_USB_DEVICE_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
		int i = 0;
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
		clear_columns();
		toggle_column();
		
		if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == GPIO_PIN_SET) press_matrix[0][cl] = 1; else press_matrix[0][cl] = 0;
		if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_1) == GPIO_PIN_SET) press_matrix[1][cl] = 1; else press_matrix[1][cl] = 0;
		if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_2) == GPIO_PIN_SET) press_matrix[2][cl] = 1; else press_matrix[2][cl] = 0;
		if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_3) == GPIO_PIN_SET) press_matrix[3][cl] = 1; else press_matrix[3][cl] = 0;
		if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4) == GPIO_PIN_SET) press_matrix[4][cl] = 1; else press_matrix[4][cl] = 0;
		if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_5) == GPIO_PIN_SET) press_matrix[5][cl] = 1; else press_matrix[5][cl] = 0;
		if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_6) == GPIO_PIN_SET) press_matrix[6][cl] = 1; else press_matrix[6][cl] = 0;
		if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_7) == GPIO_PIN_SET) press_matrix[7][cl] = 1; else press_matrix[7][cl] = 0;
		
		//shift = (press_matrix[7][5] == 1) || (press_matrix[7][8] == 1);
		lshift = (press_matrix[7][5] == 1);
		rshift = (press_matrix[7][8] == 1);
		//ctrl = (press_matrix[5][7] == 1) || (press_matrix[6][7] == 1);
		lctrl = (press_matrix[5][6] == 1) ;
		rctrl = (press_matrix[5][7] == 1);
		lalt = (press_matrix[2][7] == 1);
		ralt = (press_matrix[6][7] == 1);
		lwin = (press_matrix[6][8] == 1);
		fn = (press_matrix[4][7] == 1);
		if (fn){
			rwin = lwin;
			lwin = 0;
		}
		
		if (press_matrix[3][7] == 1)
		{
			press_matrix[3][7] = 0;
			press_matrix[7][7] = 1;
		}
		
		for (i=0; i<8; i++){
			if (press_matrix[i][cl] == 1) kb_matrix[i][cl]++; else {
				if (kb_matrix[i][cl] > 0){
					kbhid.keykode1 = 0x00;
  				kbhid.modifier = (lshift << 1) + (rshift << 5) + (lctrl) + (rctrl << 4) + (lalt << 2) + (ralt << 6) + (lwin << 3) + (rwin << 7);
					USBD_HID_SendReport(&hUsbDeviceFS, (uint8_t *) &kbhid, sizeof(kbhid));
					HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
				}
				kb_matrix[i][cl] = 0;
			}
			
			if ((kb_matrix[i][cl] == 1)||(kb_matrix[i][cl] > 50)){
  			//if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13) == GPIO_PIN_RESET) HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET); else HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
				uint8_t tmp = 0;
				if (fn) tmp = fn_matrix[i][cl]; else tmp = val_matrix[i][cl];
				if (tmp != 0){
					HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
					kbhid.keykode1 = tmp;
					kbhid.modifier = (lshift << 1) + (rshift << 5) + (lctrl) + (rctrl << 4) + (lalt << 2) + (ralt << 6) + (lwin << 3) + (rwin << 7);
					USBD_HID_SendReport(&hUsbDeviceFS, (uint8_t *) &kbhid, sizeof(kbhid));
				} //else kb_matrix[i][cl] = 0;
				
				if (lshift || rshift) tmp = shift_matrix[i][cl][0]; else tmp = str_matrix[i][cl][0];
				if (tmp){
					HAL_UART_Transmit(&huart1, &tmp, 1, 100);
				  if (tmp=='\n'){
						tmp = '\r';
  					HAL_UART_Transmit(&huart1, &tmp, 1, 100);
					}
				}
			}
		}
		
		HAL_Delay(5);
		
		if (cl == 0){ 
			//if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13) == GPIO_PIN_RESET) HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET); else HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
		}
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL6;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB;
  PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_PLL;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_3
                          |GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7
                          |GPIO_PIN_8, GPIO_PIN_RESET);

  /*Configure GPIO pin : PC13 */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PA0 PA1 PA2 PA3
                           PA4 PA5 PA6 PA7 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3
                          |GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PB12 PB13 PB14 PB3
                           PB4 PB5 PB6 PB7
                           PB8 */
  GPIO_InitStruct.Pin = GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_3
                          |GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7
                          |GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
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
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
