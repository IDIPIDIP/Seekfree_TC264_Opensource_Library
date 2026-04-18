#include "zf_common_typedef.h"
#include "zf_common_headfile.h"

//进入mode2时asr_init();// 初始化ASR模块，连接WiFi

#define MODE2_SNAKE_TOTAL_LENGTH_M      (10.0f)  // 蛇形路径总长度，单位 m
#define MODE2_SNAKE_POINT_DS_M          (0.10f)  // 相邻路径点间距，单位 m（越小越平滑）
#define MODE2_SNAKE_AMPLITUDE_M         (0.70f)  // 蛇形横向摆动幅值，单位 m
#define MODE2_SNAKE_WAVELENGTH_M        (2.20f)  // 蛇形波长，单位 m（越小摆动越频繁）
#define MODE2_SNAKE_FORWARD_SPEED_MPS   (1.30f)  // 蛇形前进目标速度，单位 m/s
#define MODE2_SNAKE_REVERSE_SPEED_MPS   (-1.30f) // 蛇形后退目标速度，单位 m/s（负值）
#define MODE2_SNAKE_MIN_POINTS          (20U)    // 路径有效最小点数，低于该值不启动追踪
#define MODE2_SNAKE_RAMP_LENGTH_M       (0.50f)  // 起终段渐入渐出长度，单位 m

// 生成蛇形路径坐标：x_sign=1.0f 生成前进路径，x_sign=-1.0f 生成后退路径
static uint16 mode2_build_snake_xy_path(float x_sign)
{
    uint16 i = 0;
    uint16 point_count = 0;
    float k_omega = 0.0f;
    float x_abs = 0.0f;
    float x = 0.0f;
    float ramp = 1.0f;
    float dist_to_end = 0.0f;

    // 按总长度与采样间距计算离散点数（包含起点）
    point_count = (uint16)(MODE2_SNAKE_TOTAL_LENGTH_M / MODE2_SNAKE_POINT_DS_M) + 1U;
    if(point_count > INTEGRATE_MAX_POINTS)
    {
        point_count = INTEGRATE_MAX_POINTS;
    }

    // 角频率 omega = 2*pi / lambda
    k_omega = 6.2831853f / MODE2_SNAKE_WAVELENGTH_M;
    for(i = 0; i < point_count; i++)
    {
        x_abs = (float)i * MODE2_SNAKE_POINT_DS_M;// x坐标绝对值 从0开始递增

        // 起终段做线性渐入/渐出，避免一开始和结束时转向突变
        dist_to_end = MODE2_SNAKE_TOTAL_LENGTH_M - x_abs;
        ramp = 1.0f;
        if(x_abs < MODE2_SNAKE_RAMP_LENGTH_M)
        {
            ramp = x_abs / MODE2_SNAKE_RAMP_LENGTH_M;
        }
        else if(dist_to_end < MODE2_SNAKE_RAMP_LENGTH_M)
        {
            ramp = dist_to_end / MODE2_SNAKE_RAMP_LENGTH_M;
        }
        if(ramp < 0.0f)
        {
            ramp = 0.0f;
        }

        // x_sign=1 生成前进路径；x_sign=-1 生成后退路径
        x = x_sign * x_abs;
        // y=A*sin(omega*x) 生成蛇形横向摆动，并叠加ramp平滑首尾
        path_track_points_struct.x[i] = x;
        path_track_points_struct.y[i] = MODE2_SNAKE_AMPLITUDE_M * ramp * sinf(k_omega * x_abs);
    }

    path_track_points_struct.point_num = point_count;
    path_track_points_struct.x_offset = path_track_points_struct.x[0];
    path_track_points_struct.y_offset = path_track_points_struct.y[0];
    return point_count;
}

void mode2_1(void)//发车区灯光任务
{
    uint8 mode2_1_flag = 0;
    mode2_1_flag=asr();//key1开始录音，再按一次结束
    if(mode2_1_flag==28)
    {
        dot_matrix_screen_zuozhuan();//点阵屏显示左转图案
    }
    else if(mode2_1_flag==29)
    {
        dot_matrix_screen_youzhuan();//点阵屏显示右转图案
    }
    else if(mode2_1_flag==30)
    {
        dot_matrix_screen_yuanguang();//点阵屏显示远光图案
    }
    else if(mode2_1_flag==31)
    {
        dot_matrix_screen_jinguang();//点阵屏显示近光图案
    }
    else if(mode2_1_flag==32)
    {
        dot_matrix_screen_wudeng();//点阵屏显示雾灯图案
    }
    else if(mode2_1_flag==33)
    {
        dot_matrix_screen_shuangshan();//点阵屏显示双闪图案
    }
    else if(mode2_1_flag==34)
    {
        dot_matrix_screen_cheneideng();//点阵屏显示车内照明灯
    }
    else if(mode2_1_flag==35)
    {
        dot_matrix_screen_yuguaqi();//点阵屏显示雨刷器
    }
}

void mode2_2(void)//鸣笛任务
{
    uint8 mode2_2_flag = 0;
    mode2_2_flag=asr();//key1开始录音，再按一次结束
    if(mode2_2_flag==1)
    {
        speaker_control(1000,1000);//鸣笛一秒钟
    }
    else if(mode2_2_flag==2)
    {
        speaker_control(1000,2000);//鸣笛二秒钟
    }
    else if(mode2_2_flag==3)
    {
        speaker_control(1000,3000);//鸣笛三秒钟
    }
    else if(mode2_2_flag==4)
    {
        speaker_control(1000,1000);//鸣笛两声
        system_delay_ms(1000);
        speaker_control(1000,1000);
    }
    else if(mode2_2_flag==5)
    {
        speaker_control(1000,1000);//鸣笛三声
        system_delay_ms(1000);
        speaker_control(1000,1000);
        system_delay_ms(1000);
        speaker_control(1000,1000);
    }
    else if(mode2_2_flag==6)
    {
        speaker_control(1000,1000);//鸣笛四声
        system_delay_ms(1000);
        speaker_control(1000,1000);
        system_delay_ms(1000);
        speaker_control(1000,1000);
        system_delay_ms(1000);
        speaker_control(1000,1000);
    }
    else if(mode2_2_flag==7)
    {
        speaker_control(1000,1000);//长短鸣笛
        system_delay_ms(1000);
        speaker_control(1000,3000);
        // speaker_control(1000,1000);
        // system_delay_ms(1000);
        // speaker_control(1000,3000);
        // speaker_control(1000,1000);
        // system_delay_ms(1000);
        // speaker_control(1000,3000);
    }
    else if(mode2_2_flag==8)
    {
        speaker_control(1000,500);//急促鸣笛
        system_delay_ms(500);
        speaker_control(1000,500);
        system_delay_ms(500);
        speaker_control(1000,500);
        system_delay_ms(500);
        speaker_control(1000,500);
        system_delay_ms(500);
        speaker_control(1000,500);
        system_delay_ms(500);
        speaker_control(1000,500);
        system_delay_ms(500);
        speaker_control(1000,500);
    }
    else if(mode2_2_flag==9)
    {
        speaker_control(500,1000);//警报鸣笛
        speaker_control(1000,1000);
        speaker_control(500,1000);//警报鸣笛
        speaker_control(1000,1000);
        speaker_control(500,1000);//警报鸣笛
        speaker_control(1000,1000);
        speaker_control(500,1000);//警报鸣笛
        speaker_control(1000,1000);
        speaker_control(500,1000);//警报鸣笛
        speaker_control(1000,1000);
        speaker_control(500,1000);//警报鸣笛
        speaker_control(1000,1000);
    }
}

void mode2_3(void)//运动任务
{
    uint8 mode2_3_flag = 0;
    mode2_3_flag=asr();//key1开始录音，再按一次结束
    if(mode2_3_flag==10)//通过门洞一左
    {
        flash_read_struct_toBuffer(1);
        set_control_mode(1); // 设置为自动模式
        point_num=curve_fit(buffer_point.x, buffer_point.y, buffer_point.point_num, 0.1f, path_track_points_struct.x, path_track_points_struct.y, INTEGRATE_MAX_POINTS);
        path_track_set_mode(0); // 设置为正向路径追踪模式
        path_track_load_path(&path_track_points_struct); // 加载路径点数据到路径
        path_track_start(); // 开始路径跟踪
    }
    else if(mode2_3_flag==11)//通过门洞一
    {
        flash_read_struct_toBuffer(2);
        set_control_mode(1); // 设置为自动模式
        point_num=curve_fit(buffer_point.x, buffer_point.y, buffer_point.point_num, 0.1f, path_track_points_struct.x, path_track_points_struct.y, INTEGRATE_MAX_POINTS);
        path_track_set_mode(0); // 设置为正向路径追踪模式
        path_track_load_path(&path_track_points_struct); // 加载路径点数据到路径
        path_track_start(); // 开始路径跟踪
    }
    else if(mode2_3_flag==12)//通过门洞二
    {
        flash_read_struct_toBuffer(3);
        set_control_mode(1); // 设置为自动模式
        point_num=curve_fit(buffer_point.x, buffer_point.y, buffer_point.point_num, 0.1f, path_track_points_struct.x, path_track_points_struct.y, INTEGRATE_MAX_POINTS);
        path_track_set_mode(0); // 设置为正向路径追踪模式
        path_track_load_path(&path_track_points_struct); // 加载路径点数据到路径
        path_track_start(); // 开始路径跟踪
    }
    else if(mode2_3_flag==13)//通过门洞三
    {
        flash_read_struct_toBuffer(4);
        set_control_mode(1); // 设置为自动模式
        point_num=curve_fit(buffer_point.x, buffer_point.y, buffer_point.point_num, 0.1f, path_track_points_struct.x, path_track_points_struct.y, INTEGRATE_MAX_POINTS);
        path_track_set_mode(0); // 设置为正向路径追踪模式
        path_track_load_path(&path_track_points_struct); // 加载路径点数据到路径
        path_track_start(); // 开始路径跟踪
    }
    else if(mode2_3_flag==14)//通过门洞三右
    {
        flash_read_struct_toBuffer(5);
        set_control_mode(1); // 设置为自动模式
        point_num=curve_fit(buffer_point.x, buffer_point.y, buffer_point.point_num, 0.1f, path_track_points_struct.x, path_track_points_struct.y, INTEGRATE_MAX_POINTS);
        path_track_set_mode(0); // 设置为正向路径追踪模式
        path_track_load_path(&path_track_points_struct); // 加载路径点数据到路径
        path_track_start(); // 开始路径跟踪
    }
    else if(mode2_3_flag==15)//门洞一右侧返回
    {
        flash_read_struct_toBuffer(6);
        set_control_mode(1); // 设置为自动模式
        point_num=curve_fit(buffer_point.x, buffer_point.y, buffer_point.point_num, 0.1f, path_track_points_struct.x, path_track_points_struct.y, INTEGRATE_MAX_POINTS);
        path_track_set_mode(0); // 设置为正向路径追踪模式
        path_track_load_path(&path_track_points_struct); // 加载路径点数据到路径
        path_track_start(); // 开始路径跟踪
    }
    else if(mode2_3_flag==16)//门洞一返回
    {
        flash_read_struct_toBuffer(7);
        set_control_mode(1); // 设置为自动模式
        point_num=curve_fit(buffer_point.x, buffer_point.y, buffer_point.point_num, 0.1f, path_track_points_struct.x, path_track_points_struct.y, INTEGRATE_MAX_POINTS);
        path_track_set_mode(0); // 设置为正向路径追踪模式
        path_track_load_path(&path_track_points_struct); // 加载路径点数据到路径
        path_track_start(); // 开始路径跟踪
    }
    else if(mode2_3_flag==17)//门洞二返回
    {
        flash_read_struct_toBuffer(8);
        set_control_mode(1); // 设置为自动模式
        point_num=curve_fit(buffer_point.x, buffer_point.y, buffer_point.point_num, 0.1f, path_track_points_struct.x, path_track_points_struct.y, INTEGRATE_MAX_POINTS);
        path_track_set_mode(0); // 设置为正向路径追踪模式
        path_track_load_path(&path_track_points_struct); // 加载路径点数据到路径
        path_track_start(); // 开始路径跟踪
    }
    else if(mode2_3_flag==18)//门洞三返回
    {
        flash_read_struct_toBuffer(9);
        set_control_mode(1); // 设置为自动模式
        point_num=curve_fit(buffer_point.x, buffer_point.y, buffer_point.point_num, 0.1f, path_track_points_struct.x, path_track_points_struct.y, INTEGRATE_MAX_POINTS);
        path_track_set_mode(0); // 设置为正向路径追踪模式
        path_track_load_path(&path_track_points_struct); // 加载路径点数据到路径
        path_track_start(); // 开始路径跟踪
    }
    else if(mode2_3_flag==19)//门洞三左侧返回
    {
        flash_read_struct_toBuffer(10);
        set_control_mode(1); // 设置为自动模式
        point_num=curve_fit(buffer_point.x, buffer_point.y, buffer_point.point_num, 0.1f, path_track_points_struct.x, path_track_points_struct.y, INTEGRATE_MAX_POINTS);
        path_track_set_mode(0); // 设置为正向路径追踪模式
        path_track_load_path(&path_track_points_struct); // 加载路径点数据到路径
        path_track_start(); // 开始路径跟踪
    }
}

void mode2_4(void)//运动区任务
{
    uint8 mode2_3_flag = 0;
    mode2_3_flag=asr();//key1开始录音，再按一次结束
    if(mode2_3_flag==20)//前行十米
    {
        set_target_steering_angle(0); // 设置目标转向角为0度，保持直行
        set_target_speed(2); // 设置目标速度为2 m/s
    }
    else if(mode2_3_flag==21)//后退十米
    {
        set_target_steering_angle(0); // 设置目标转向角为0度，保持直行
        set_target_speed(-2); // 设置目标速度为-2 m/s（后退）
    }
    else if(mode2_3_flag==22)//蛇形前进十米
    {
        set_control_mode(1); // 设置为自动模式
        point_num = (int)mode2_build_snake_xy_path(1.0f);
        if(point_num >= (int)MODE2_SNAKE_MIN_POINTS)
        {
            path_track_set_mode(0); // 设置为正向路径追踪模式
            path_track_load_path(&path_track_points_struct); // 加载路径点数据到路径
            set_target_speed(MODE2_SNAKE_FORWARD_SPEED_MPS);
            path_track_start(); // 开始路径跟踪
        }
        else
        {
            set_target_speed(0);
            set_target_steering_angle(0);
            path_track_stop();
        }
    }
    else if(mode2_3_flag==23)//蛇形后退十米
    {
        set_control_mode(1); // 设置为自动模式
        point_num = (int)mode2_build_snake_xy_path(-1.0f);
        if(point_num >= (int)MODE2_SNAKE_MIN_POINTS)
        {
            path_track_set_mode(1); // 设置为反向路径追踪模式
            path_track_load_path(&path_track_points_struct); // 加载路径点数据到路径
            set_target_speed(MODE2_SNAKE_REVERSE_SPEED_MPS);
            path_track_start(); // 开始路径跟踪
        }
        else
        {
            set_target_speed(0);
            set_target_steering_angle(0);
            path_track_stop();
        }
    }
    else if(mode2_3_flag==24)//逆时针转一圈
    {
        set_target_steering_angle(26); 
        set_target_speed(3);
        system_delay_ms(1603); // 根据实际测试调整转一圈所需时间
        set_target_speed(0); // 停止车辆
        set_target_steering_angle(0); // 恢复转向角为0度
    }
    else if(mode2_3_flag==25)//顺时针转一圈
    {
        set_target_steering_angle(-26); 
        set_target_speed(3);
        system_delay_ms(1603); // 根据实际测试调整转一圈所需时间
        set_target_speed(0); // 停止车辆
        set_target_steering_angle(0); // 恢复转向角为0度
    }
    else if(mode2_3_flag==26)//左转
    {
        set_target_steering_angle(26); 
        set_target_speed(3);
        system_delay_ms(400); // 根据实际测试调整转1/4圈所需时间
        set_target_speed(0); // 停止车辆
        set_target_steering_angle(0); // 恢复转向角为0度
    }
    else if(mode2_3_flag==27)//右转
    {
        set_target_steering_angle(-26); 
        set_target_speed(3);
        system_delay_ms(400); // 根据实际测试调整转1/4圈所需时间
        set_target_speed(0); // 停止车辆
        set_target_steering_angle(0); // 恢复转向角为0度
    }
}