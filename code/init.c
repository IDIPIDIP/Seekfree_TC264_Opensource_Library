/*
 * int.c
 *
 *  Created on: 2026年2月27日
 *      Author: IDIPIDIP
 */

#include "zf_common_typedef.h"
#include "zf_common_headfile.h"

int L_PWM_P=50;//左电机PWM频率
int L_PWM_K=5000;//左电机PWM占空比 =PWM_K/10000
int R_PWM_P=50;//右电机PWM频率
int R_PWM_K=5000;//右电机PWM占空比 =PWM_K/10000
int l_encoder;//左后编码器计数值
int r_encoder;//右后编码器计数值
int f_encoder;//前轮编码器计数值


void init()
{
    pwm_init(ATOM0_CH0_P21_2,L_PWM_P,L_PWM_K);
    pwm_init(ATOM1_CH1_P21_3,R_PWM_P,R_PWM_K);
    encoder_dir_init(TIM2_ENCODER, TIM2_ENCODER_CH1_P33_7, TIM2_ENCODER_CH2_P00_8);//左后编码器
    encoder_dir_init(TIM3_ENCODER, TIM3_ENCODER_CH1_P02_6, TIM3_ENCODER_CH2_P02_7);//右后编码器
    encoder_dir_init(TIM4_ENCODER, TIM4_ENCODER_CH1_P02_8, TIM4_ENCODER_CH2_P00_9);//前轮编码器
    encoder_clear_count(TIM2_ENCODER);
    encoder_clear_count(TIM3_ENCODER);
    encoder_clear_count(TIM4_ENCODER);
    pit_ms_init(CCU60_CH0, 10);//定时器0，10ms中断一次，用于获取编码器和按键扫描
    gnss_init(TAU1201);
    tft180_init();
    tft180_set_font(TFT180_6X8_FONT);
    key_init(10);//按键扫描周期10ms
    

}
