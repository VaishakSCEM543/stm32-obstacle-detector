/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
#include <stdio.h>
/* USER CODE END Includes */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* ===== PIN DEFINITIONS (BASED ON YOUR CONNECTIONS) ===== */
#define LED_PORT        GPIOB
#define LED_PIN         GPIO_PIN_4     // External LED on PB4

#define BUZZER_PORT     GPIOB
#define BUZZER_PIN      GPIO_PIN_5     // Buzzer on PB5

#define LDR_PORT        GPIOB
#define LDR_PIN         GPIO_PIN_10    // LDR D0 on PB10 (Digital)

#define TRIG_PORT       GPIOB
#define TRIG_PIN        GPIO_PIN_3     // Ultrasonic TRIG on PB3

#define ECHO_PORT       GPIOA
#define ECHO_PIN        GPIO_PIN_10    // Ultrasonic ECHO on PA10

#define DHT_PORT        GPIOA
#define DHT_PIN         GPIO_PIN_0     // DHT DATA on PA0

/* Thresholds */
#define DIST_ALERT_CM   15
#define DIST_WARN_CM    30

/* USER CODE END PD */

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
float distance_cm = 0;
uint8_t lightDetected = 0;

uint8_t dht_temp = 0;
uint8_t dht_hum  = 0;
uint8_t dht_ok   = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);

/* USER CODE BEGIN PFP */
static void delay_us(uint16_t us);
static void DWT_Init(void);

static float Ultrasonic_ReadDistanceCM(void);

static void DHT_SetPinOutput(void);
static void DHT_SetPinInput(void);
static uint8_t DHT_Read(uint8_t *temperature, uint8_t *humidity);

static void LED_ON(void);
static void LED_OFF(void);
static void BUZZER_ON(void);
static void BUZZER_OFF(void);
/* USER CODE END PFP */

/* USER CODE BEGIN 0 */

/* ✅ printf redirect to UART2 */
int _write(int file, char *ptr, int len)
{
  HAL_UART_Transmit(&huart2, (uint8_t*)ptr, len, HAL_MAX_DELAY);
  return len;
}

/* microsecond delay using DWT */
static void delay_us(uint16_t us)
{
  uint32_t start = DWT->CYCCNT;
  uint32_t ticks = (SystemCoreClock / 1000000) * us;
  while ((DWT->CYCCNT - start) < ticks);
}

/* Enable DWT cycle counter (needed for delay_us) */
static void DWT_Init(void)
{
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  DWT->CYCCNT = 0;
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

/* ✅ ACTIVE-LOW LED / BUZZER helpers */
static void LED_OFF(void)    { HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_SET); }
static void LED_ON(void)     { HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_RESET); }

static void BUZZER_OFF(void) { HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_SET); }
static void BUZZER_ON(void)  { HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_RESET); }

/* Ultrasonic distance */
static float Ultrasonic_ReadDistanceCM(void)
{
  uint32_t t1 = 0, t2 = 0;
  uint32_t timeout = 60000;

  /* Trigger pulse */
  HAL_GPIO_WritePin(TRIG_PORT, TRIG_PIN, GPIO_PIN_RESET);
  delay_us(2);
  HAL_GPIO_WritePin(TRIG_PORT, TRIG_PIN, GPIO_PIN_SET);
  delay_us(10);
  HAL_GPIO_WritePin(TRIG_PORT, TRIG_PIN, GPIO_PIN_RESET);

  /* Wait for ECHO HIGH */
  uint32_t startWait = HAL_GetTick();
  while (HAL_GPIO_ReadPin(ECHO_PORT, ECHO_PIN) == GPIO_PIN_RESET)
  {
    if ((HAL_GetTick() - startWait) > 50) return -1;
  }

  t1 = DWT->CYCCNT;

  /* Wait for ECHO LOW */
  while (HAL_GPIO_ReadPin(ECHO_PORT, ECHO_PIN) == GPIO_PIN_SET)
  {
    if (timeout-- == 0) return -1;
  }

  t2 = DWT->CYCCNT;

  uint32_t diff_cycles = (t2 - t1);
  float time_us = (float)diff_cycles / (SystemCoreClock / 1000000.0f);
  float dist = (time_us * 0.0343f) / 2.0f;

  return dist;
}

/* DHT pin direction control */
static void DHT_SetPinOutput(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin = DHT_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(DHT_PORT, &GPIO_InitStruct);
}

static void DHT_SetPinInput(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin = DHT_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(DHT_PORT, &GPIO_InitStruct);
}

/* DHT Read: returns 1 if OK, 0 if fail */
static uint8_t DHT_Read(uint8_t *temperature, uint8_t *humidity)
{
  uint8_t data[5] = {0};

  /* Start signal */
  DHT_SetPinOutput();
  HAL_GPIO_WritePin(DHT_PORT, DHT_PIN, GPIO_PIN_RESET);
  HAL_Delay(18);
  HAL_GPIO_WritePin(DHT_PORT, DHT_PIN, GPIO_PIN_SET);
  delay_us(30);
  DHT_SetPinInput();

  /* Response check */
  uint32_t wait = 0;
  while (HAL_GPIO_ReadPin(DHT_PORT, DHT_PIN) == GPIO_PIN_SET)
  {
    if (wait++ > 150) return 0;
    delay_us(1);
  }

  wait = 0;
  while (HAL_GPIO_ReadPin(DHT_PORT, DHT_PIN) == GPIO_PIN_RESET)
  {
    if (wait++ > 150) return 0;
    delay_us(1);
  }

  wait = 0;
  while (HAL_GPIO_ReadPin(DHT_PORT, DHT_PIN) == GPIO_PIN_SET)
  {
    if (wait++ > 150) return 0;
    delay_us(1);
  }

  /* Read 40 bits */
  for (int i = 0; i < 40; i++)
  {
    while (HAL_GPIO_ReadPin(DHT_PORT, DHT_PIN) == GPIO_PIN_RESET);

    uint32_t t = 0;
    while (HAL_GPIO_ReadPin(DHT_PORT, DHT_PIN) == GPIO_PIN_SET)
    {
      t++;
      delay_us(1);
      if (t > 255) break;
    }

    data[i/8] <<= 1;
    if (t > 40)
      data[i/8] |= 1;
  }

  if ((uint8_t)(data[0] + data[1] + data[2] + data[3]) != data[4])
    return 0;

  *humidity = data[0];
  *temperature = data[2];
  return 1;
}

/* USER CODE END 0 */

int main(void)
{
  HAL_Init();

  /* ✅ Init DWT AFTER clock config (most stable) */
  SystemClock_Config();
  DWT_Init();

  MX_GPIO_Init();
  MX_USART2_UART_Init();

  LED_OFF();
  BUZZER_OFF();

  printf("\r\n✅ SYSTEM STARTED ✅\r\n");
  printf("Distance | LDR | Temp | Hum\r\n");

  uint32_t last_print = 0;

  while (1)
  {
    /* -------- LDR DIGITAL READ -------- */
    uint8_t ldr_raw = HAL_GPIO_ReadPin(LDR_PORT, LDR_PIN);

    /* ✅ Adjust this if your LDR module logic is opposite */
    if (ldr_raw == GPIO_PIN_RESET)
      lightDetected = 0;   // DARK
    else
      lightDetected = 1;   // BRIGHT

    /* -------- ULTRASONIC READ -------- */
    distance_cm = Ultrasonic_ReadDistanceCM();

    /* -------- DHT READ (every 2 sec) -------- */
    static uint32_t last_dht_time = 0;
    if (HAL_GetTick() - last_dht_time >= 2000)
    {
      last_dht_time = HAL_GetTick();
      dht_ok = DHT_Read(&dht_temp, &dht_hum);
    }

    /* ✅ PRINT EVERY 1 SECOND */
    if (HAL_GetTick() - last_print >= 1000)
    {
      last_print = HAL_GetTick();

      if (distance_cm < 0)
        printf("Dist=ERR | ");
      else
        printf("Dist=%.2fcm | ", distance_cm);

      printf("LDR=%s | ", (lightDetected ? "BRIGHT" : "DARK"));

      if (dht_ok)
        printf("Temp=%dC | Hum=%d%%\r\n", dht_temp, dht_hum);
      else
        printf("Temp=-- | Hum=-- (DHT FAIL)\r\n");
    }

    /* -------- OUTPUT LOGIC -------- */
    LED_OFF();
    BUZZER_OFF();

    /* Condition 1: Object very near */
    if (distance_cm > 0 && distance_cm <= DIST_ALERT_CM)
    {
      LED_ON();
      BUZZER_ON();
      HAL_Delay(150);
      LED_OFF();
      BUZZER_OFF();
      HAL_Delay(150);
      continue;
    }

    /* Condition 2: Object near */
    if (distance_cm > 0 && distance_cm <= DIST_WARN_CM)
    {
      LED_ON();
      HAL_Delay(300);
      LED_OFF();
      HAL_Delay(300);
      continue;
    }

    /* Condition 3: Darkness detected */
    if (lightDetected == 0)
    {
      LED_ON();
      HAL_Delay(700);
      LED_OFF();
      HAL_Delay(700);
      continue;
    }

    /* Condition 4: DHT warning (if ok) */
    if (dht_ok)
    {
      if (dht_temp >= 35 || dht_hum >= 80)
      {
        LED_ON();
        BUZZER_ON();
        HAL_Delay(200);
        LED_OFF();
        BUZZER_OFF();
        HAL_Delay(800);
        continue;
      }
    }

    HAL_Delay(50);
  }
}

/* System Clock */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) Error_Handler();

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
    Error_Handler();
}

/* UART2 init */
static void MX_USART2_UART_Init(void)
{
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK) Error_Handler();
}

/* GPIO init */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /* ✅ IMPORTANT: set outputs OFF (ACTIVE LOW => SET = OFF) */
  HAL_GPIO_WritePin(GPIOB, LED_PIN|BUZZER_PIN, GPIO_PIN_SET);
  HAL_GPIO_WritePin(GPIOB, TRIG_PIN, GPIO_PIN_RESET);

  /* LED PB4 */
  GPIO_InitStruct.Pin = LED_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED_PORT, &GPIO_InitStruct);

  /* Buzzer PB5 */
  GPIO_InitStruct.Pin = BUZZER_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(BUZZER_PORT, &GPIO_InitStruct);

  /* TRIG PB3 */
  GPIO_InitStruct.Pin = TRIG_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(TRIG_PORT, &GPIO_InitStruct);

  /* ECHO PA10 */
  GPIO_InitStruct.Pin = ECHO_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(ECHO_PORT, &GPIO_InitStruct);

  /* LDR D0 PB10 */
  GPIO_InitStruct.Pin = LDR_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(LDR_PORT, &GPIO_InitStruct);

  /* DHT PA0 input initially */
  GPIO_InitStruct.Pin = DHT_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(DHT_PORT, &GPIO_InitStruct);
}

void Error_Handler(void)
{
  __disable_irq();
  while (1) {}
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
}
#endif
