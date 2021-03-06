/**
 ******************************************************************************
 * @file    drv_fan.c
 * @author  MEMS Application Team
 * @version V1.1
 * @date    10-August-2016
 * @brief   drv_fan source file
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "drv.h"
#include "drv_fan.h"

/* Private variables ---------------------------------------------------------*/
extern TIM_HandleTypeDef htim1;
extern e2prom_param_s g_e2promParam;

static uint32_t tickstart;
static uint32_t tickexpire;
static uint8_t turn_off = 0;

/* Private function prototypes -----------------------------------------------*/

/**
  * @brief  The drv_led init.
  */
void Drv_FAN_Init(void)
{  
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);  

  tickstart = HAL_GetTick();
}

void Drv_FAN_Proc(void)
{
  if (turn_off)
  {
    if((HAL_GetTick() - tickstart) > tickexpire)
    {
      turn_off = 0;
      __HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_2, (uint32_t)0);
    }
  }
}

void drv_fan_speed(uint16_t param)
{
  param=param>100?100:param;
  
  __HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_2, (uint32_t)param);
}

void drv_fan_on(void)
{
  turn_off = 0;

  if(getLightType() == SMALL_LIGHT) {

    __HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_2, 80);
  } else {

    __HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_2, 100);
  }
}

void drv_fan_off(uint32_t delay)
{
  turn_off = 1;
  tickexpire = delay;
  tickstart = HAL_GetTick();
}

