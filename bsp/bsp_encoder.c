#include "bsp_encoder.h"



/**       			STM32F429VGT6
 *             		TIMx        CH1     CH2
 * FRONT_LEFT   	TIM2        PA0    	PA1
 * FRONT_RIGHT   	TIM3        PB4  	PB5
 * BACK_LEFT 		TIM4		PB6		PB7
 * BACK_RIGHT		TIM5		PH10	PH11
 */



const int16_t angle_reload = 0;
const int16_t angle_min = -127;
const int16_t angle_max = 128;
int16_t angle_last[4] = {angle_reload, angle_reload, angle_reload, angle_reload};
int16_t angle_cur[4], acc[4];
int16_t round_cnt[4];
double total_angle[4];

TIM_TypeDef* TIMs[4] = {TIM2, TIM3, TIM4, TIM5}; //TIM2,3,4,5

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;
TIM_HandleTypeDef htim5;
// /**
//  * @breif read encoder value and clear register
//  */
void BSP_EncoderRead(void)
{
	uint8_t i = 0;
	for (i = 0; i < 4; i++)
	{
		angle_cur[i] = TIMs[i]->CNT;
		acc[i] = angle_cur[i] - angle_last[i] ;
		if (acc[i]  > angle_max )
		{
			acc[i] -= (angle_max - angle_min+1);
		}
		else if (acc[i] < angle_min) {
			acc[i] += (angle_max - angle_min+1);
		}
		angle_last[i] = angle_cur[i];
		// total_angle[i] = (round_cnt[i] * (angle_max - angle_min) + angle_cur[i])
		//                  * 360.0f / (angle_max - angle_min);
	}
}




/* TIM2 init function */
void MX_TIM2_Init(void)
{

	TIM_Encoder_InitTypeDef sConfig;
	TIM_MasterConfigTypeDef sMasterConfig;



	htim2.Instance = TIM2;
	htim2.Init.Prescaler = 0;
	htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim2.Init.Period = angle_max - angle_min;
	htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	sConfig.EncoderMode = TIM_ENCODERMODE_TI12;					//两个通道均计数
	sConfig.IC1Polarity = TIM_ICPOLARITY_RISING;				//此参数为上升沿触发捕获AB相的值，不是4倍频
	sConfig.IC1Selection = TIM_ICSELECTION_DIRECTTI;			
	sConfig.IC1Prescaler = TIM_ICPSC_DIV1;
	sConfig.IC1Filter = 4;
	sConfig.IC2Polarity = TIM_ICPOLARITY_RISING;
	sConfig.IC2Selection = TIM_ICSELECTION_DIRECTTI;
	sConfig.IC2Prescaler = TIM_ICPSC_DIV1;
	sConfig.IC2Filter = 4;
	HAL_TIM_Encoder_Init(&htim2, &sConfig);

	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig);

	__HAL_TIM_ENABLE_IT(&htim2, TIM_IT_UPDATE);
	__HAL_TIM_CLEAR_IT(&htim2, TIM_IT_UPDATE);
	__HAL_TIM_ENABLE(&htim2);

}

/* TIM3 init function */
void MX_TIM3_Init(void)
{

	TIM_Encoder_InitTypeDef sConfig;
	TIM_MasterConfigTypeDef sMasterConfig;

	htim3.Instance = TIM3;
	htim3.Init.Prescaler = 0;
	htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim3.Init.Period = angle_max - angle_min;
	htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	sConfig.EncoderMode = TIM_ENCODERMODE_TI12;
	sConfig.IC1Polarity = TIM_ICPOLARITY_RISING;
	sConfig.IC1Selection = TIM_ICSELECTION_DIRECTTI;
	sConfig.IC1Prescaler = TIM_ICPSC_DIV1;
	sConfig.IC1Filter = 4;
	sConfig.IC2Polarity = TIM_ICPOLARITY_RISING;
	sConfig.IC2Selection = TIM_ICSELECTION_DIRECTTI;
	sConfig.IC2Prescaler = TIM_ICPSC_DIV1;
	sConfig.IC2Filter = 4;
	HAL_TIM_Encoder_Init(&htim3, &sConfig);

	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig);

	__HAL_TIM_ENABLE_IT(&htim3, TIM_IT_UPDATE);
	__HAL_TIM_CLEAR_IT(&htim3, TIM_IT_UPDATE);
	__HAL_TIM_ENABLE(&htim3);

}

/* TIM4 init function */
void MX_TIM4_Init(void)
{

	TIM_Encoder_InitTypeDef sConfig;
	TIM_MasterConfigTypeDef sMasterConfig;

	htim4.Instance = TIM4;
	htim4.Init.Prescaler = 0;
	htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim4.Init.Period = angle_max - angle_min;
	htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	sConfig.EncoderMode = TIM_ENCODERMODE_TI12;
	sConfig.IC1Polarity = TIM_ICPOLARITY_RISING;
	sConfig.IC1Selection = TIM_ICSELECTION_DIRECTTI;
	sConfig.IC1Prescaler = TIM_ICPSC_DIV1;
	sConfig.IC1Filter = 4;
	sConfig.IC2Polarity = TIM_ICPOLARITY_RISING;
	sConfig.IC2Selection = TIM_ICSELECTION_DIRECTTI;
	sConfig.IC2Prescaler = TIM_ICPSC_DIV1;
	sConfig.IC2Filter = 4;
	HAL_TIM_Encoder_Init(&htim4, &sConfig);

	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig);

	__HAL_TIM_ENABLE_IT(&htim4, TIM_IT_UPDATE);
	__HAL_TIM_CLEAR_IT(&htim4, TIM_IT_UPDATE);
	__HAL_TIM_ENABLE(&htim4);

}

/* TIM5 init function */
void MX_TIM5_Init(void)
{

	TIM_Encoder_InitTypeDef sConfig;
	TIM_MasterConfigTypeDef sMasterConfig;

	htim5.Instance = TIM5;
	htim5.Init.Prescaler = 0 ;
	htim5.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim5.Init.Period = angle_max - angle_min;
	htim5.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	sConfig.EncoderMode = TIM_ENCODERMODE_TI12;
	sConfig.IC1Polarity = TIM_ICPOLARITY_RISING;
	sConfig.IC1Selection = TIM_ICSELECTION_DIRECTTI;
	sConfig.IC1Prescaler = TIM_ICPSC_DIV1;
	sConfig.IC1Filter = 4;
	sConfig.IC2Polarity = TIM_ICPOLARITY_RISING;
	sConfig.IC2Selection = TIM_ICSELECTION_DIRECTTI;
	sConfig.IC2Prescaler = TIM_ICPSC_DIV1;
	sConfig.IC2Filter = 4;
	HAL_TIM_Encoder_Init(&htim5, &sConfig);

	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	HAL_TIMEx_MasterConfigSynchronization(&htim5, &sMasterConfig);

	__HAL_TIM_ENABLE_IT(&htim5, TIM_IT_UPDATE);
	__HAL_TIM_CLEAR_IT(&htim5, TIM_IT_UPDATE);
	__HAL_TIM_ENABLE(&htim5);

}
