#include "gpio.h"
#include "delay.h"


u8 rece_light_state1 = 0;
u8 rece_light_state2 = 0;
u8 send_light_state1 = 0;

u8 left_turn_light_flag , right_turn_light_flag, double_flash_flag , horn_1_second_flag;
u8 BREAK_LIGHT_MASTER, BREAK_LIGHT_SLAVE = 0xFF;

void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct;

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOI_CLK_ENABLE();

  GPIO_InitStruct.Pin = HIGH_LIGHT_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(HIGH_LIGHT_PORT, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = LOW_LIGHT_PIN;
  HAL_GPIO_Init(LOW_LIGHT_PORT, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = LEFT_LIGHT_PIN;
  HAL_GPIO_Init(LEFT_LIGHT_PORT, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = RIGHT_LIGHT_PIN;
  HAL_GPIO_Init(RIGHT_LIGHT_PORT, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = DRIVING_LIGHT_PIN;
  HAL_GPIO_Init(DRIVING_LIGHT_PORT, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = BREAK_LIGHT_PIN;
  HAL_GPIO_Init(BREAK_LIGHT_PORT, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = REAR_LIGHT_PIN;
  HAL_GPIO_Init(REAR_LIGHT_PORT, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = HORN_PIN;
  HAL_GPIO_Init(HORN_PORT, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = WARN_LIGHT_PIN;
  HAL_GPIO_Init(WARN_LIGHT_PORT, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = RESERVE_RELAY_PIN;
  HAL_GPIO_Init(RESERVE_RELAY_PORT, &GPIO_InitStruct);

  HAL_GPIO_WritePin( HIGH_LIGHT_PORT, HIGH_LIGHT_PIN, GPIO_PIN_SET);//电路上光耦导致，引脚为高时，外部输出为低
  HAL_GPIO_WritePin( LOW_LIGHT_PORT, LOW_LIGHT_PIN, GPIO_PIN_SET);
  HAL_GPIO_WritePin( LEFT_LIGHT_PORT, LEFT_LIGHT_PIN, GPIO_PIN_SET);
  HAL_GPIO_WritePin( RIGHT_LIGHT_PORT, RIGHT_LIGHT_PIN, GPIO_PIN_SET);
  HAL_GPIO_WritePin( DRIVING_LIGHT_PORT, DRIVING_LIGHT_PIN, GPIO_PIN_SET);
  HAL_GPIO_WritePin( BREAK_LIGHT_PORT, BREAK_LIGHT_PIN, GPIO_PIN_SET);
  HAL_GPIO_WritePin( REAR_LIGHT_PORT, REAR_LIGHT_PIN, GPIO_PIN_SET);
  HAL_GPIO_WritePin( HORN_PORT, HORN_PIN, GPIO_PIN_SET);
  HAL_GPIO_WritePin( WARN_LIGHT_PORT, WARN_LIGHT_PIN, GPIO_PIN_SET);
  HAL_GPIO_WritePin( RESERVE_RELAY_PORT, RESERVE_RELAY_PIN, GPIO_PIN_SET);


  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;


  GPIO_InitStruct.Pin = BUMPERI4_PIN | BUMPERI5_PIN | BUMPERI6_PIN | BUMPERI7_PIN;
  HAL_GPIO_Init(BUMPERI4_PORT, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = BUMPERE0_PIN | BUMPERE1_PIN;
  HAL_GPIO_Init(BUMPERE0_PORT, &GPIO_InitStruct);

}

void update_light_state(void)
{
  HIGH_LIGHT             = ~(rece_light_state1 & 0x01);      //电路上光耦导致，引脚为高时，外部输出为低
  LOW_LIGHT              = ~((rece_light_state1 >> 1) & 0x01);
  DRIVING_LIGHT          = ~((rece_light_state1 >> 2) & 0x01);
  BREAK_LIGHT_MASTER     = (~((rece_light_state1 >> 3) & 0x01)) & 0x01;
  REAR_LIGHT             = ~((rece_light_state1 >> 4) & 0x01);
  WARN_LIGHT             = ~((rece_light_state1 >> 5) & 0x01);
  left_turn_light_flag   = ((rece_light_state1 >> 6) & 0x01);
  right_turn_light_flag  = ((rece_light_state1 >> 7) & 0x01);
  double_flash_flag      = (rece_light_state2 & 0x01);
  horn_1_second_flag     = ((rece_light_state2 >> 1) & 0x01);
  HORN                   = ~((rece_light_state2 >> 2) & 0x01);
  RESERVE_RELAY          = ~((rece_light_state2 >> 3) & 0x01);
  BREAK_LIGHT = BREAK_LIGHT_MASTER & BREAK_LIGHT_SLAVE;
  complex_light_function();
  if (HORN) horn_1_second();                             //喇叭为低时，才进行判断喇叭是否响一秒
}

void complex_light_function(void)
{
  static u8 left_light_count, rignt_light_count, double_flash_count = 0;
  if (double_flash_flag)  //double flash light
  {
    left_light_count = 0;
    rignt_light_count = 0;
    double_flash_count++;
    if (double_flash_count >= 50)
    {
      LEFT_LIGHT = ~LEFT_LIGHT;
      RIGHT_LIGHT = LEFT_LIGHT;
      double_flash_count = 0;
    }

  } else if (left_turn_light_flag)  //left turn light
  {
    double_flash_count = 0;
    rignt_light_count = 0;
    left_light_count++;
    if (left_light_count >= 50)
    {
      LEFT_LIGHT = ~LEFT_LIGHT;
      left_light_count = 0;
    }

  } else if (right_turn_light_flag) //right turn light
  {
    double_flash_count = 0;
    left_light_count = 0;
    rignt_light_count++;
    if (rignt_light_count >= 50)
    {
      RIGHT_LIGHT = ~RIGHT_LIGHT;
      rignt_light_count = 0;
    }
  }
  else
  {
    left_light_count = 0;
    rignt_light_count = 0;
    double_flash_count = 0;
    LEFT_LIGHT = 1;
    RIGHT_LIGHT = 1;
  }
}




void horn_1_second(void)
{
  static u8 horn_count = 0;
  if (horn_1_second_flag)
  {
    horn_count++;
    HORN = 0;
    if (horn_count >= 20 && horn_count < 40)    //horn 200ms
    {
      HORN  = 1;
    } else if (horn_count >= 40 && horn_count <= 70)  //horn 300ms
    {
      HORN  = 0;
    }
    else if (horn_count >= 70)
    {
      rece_light_state2  = rece_light_state2 & 0xfd;
      horn_count = 0;
      HORN = 1;
    }

  }
  else
  {
    horn_count = 0;
    HORN = 1;
  }
}

