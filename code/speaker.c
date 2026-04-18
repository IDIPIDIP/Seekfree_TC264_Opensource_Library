#include "zf_common_headfile.h"

//控制发声频率与时间

void speaker_control(uint32 freq, uint32 duration_ms)
{
    pwm_init(speaker_pwm_channel, freq, PWM_DUTY_MAX / 2); // 设置频率和占空比
    system_delay_ms(duration_ms); // 持续时间
    pwm_set_duty(speaker_pwm_channel, 0); // 停止发声
}