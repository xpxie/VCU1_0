#ifndef __BSP_ENCODER_H
#define __BSP_ENCODER_H
#include "sys.h"

//  #define FRONT_LEFT_CH1					GPIO_Pin_0
//  #define FRONT_LEFT_CH2					GPIO_Pin_1
//  #define FRONT_LEFT_PORT				GPIOA
//  #define FRONT_LEFT_CLK					RCC_APB2Periph_GPIOA
//  #define FRONT_LEFT_TIMER				TIM2
//  #define FRONT_LEFT_TIMER_CLK			RCC_APB1Periph_TIM2

//  #define FRONT_RIGHT_CH1				GPIO_Pin_4
//  #define FRONT_RIGHT_CH2				GPIO_Pin_5
//  #define FRONT_RIGHT_PORT				GPIOB
//  #define FRONT_RIGHT_CLK				RCC_APB2Periph_GPIOB
//  #define FRONT_RIGHT_TIMER				TIM3
//  #define FRONT_RIGHT_TIMER_CLK			RCC_APB1Periph_TIM3

//  #define BACK_LEFT_CH1					GPIO_Pin_6
//  #define BACK_LEFT_CH2					GPIO_Pin_7
//  #define BACK_LEFT_PORT					GPIOB
//  #define BACK_LEFT_CLK					RCC_APB2Periph_GPIOB
//  #define BACK_LEFT_TIMER				TIM4
//  #define BACK_LEFT_TIMER_CLK			RCC_APB1Periph_TIM4

//  #define BACK_RIGHT_CH1					GPIO_Pin_10
//  #define BACK_RIGHT_CH2					GPIO_Pin_11
//  #define BACK_RIGHT_PORT				GPIOH
//  #define BACK_RIGHT_CLK					RCC_APB2Periph_GPIOH
//  #define BACK_RIGHT_TIMER				TIM5
//  #define BACK_RIGHT_TIMER_CLK			RCC_APB1Periph_TIM5


// extern int16_t gEncoder[4];

// void BSP_EncoderInit(void);
void BSP_EncoderRead(void);


 void MX_TIM2_Init(void);
 void MX_TIM3_Init(void);
 void MX_TIM4_Init(void);
 void MX_TIM5_Init(void);

extern int16_t acc[4];
#endif
