#include "zf_common_headfile.h"

void mode3(void)//中心对准目标色块，按key1确认目标颜色
{
    tft180_clear();
    tft180_show_string(1,1,"key1 to set color target condition");//通过图像中心数据设置色块识别阈值
    // 通过图像中心数据设置色块识别阈值
    while(!set_color_target_condi(scc8660_image[SCC8660_H / 2][SCC8660_W / 2], &target_color_condi));
    tft180_clear();
    tft180_show_string(1,1,"h min");tft180_show_uint(1,31,target_color_condi.h_min,3);
    tft180_show_string(9,1,"h max");tft180_show_uint(9,31,target_color_condi.h_max,3);
    tft180_show_string(17,1,"s min");tft180_show_uint(17,31,target_color_condi.s_min,3);
    tft180_show_string(25,1,"s max");tft180_show_uint(25,31,target_color_condi.s_max,3);
    tft180_show_string(33,2,"l min");tft180_show_uint(33,31,target_color_condi.l_min,3);
    tft180_show_string(41,2,"l max");tft180_show_uint(41,31,target_color_condi.l_max,3);

    pid_color_track_init();  // 初始化色块跟随 PID 控制器
    set_control_mode(1);     // 启用自动转向模式（转向角由 PID 驱动）

    while(1)
    {
        if(color_trace(&target_color_condi, &target_pos_out)) // x: 0-(SCC8660_W-1)  y: 0-(SCC8660_H-1)
        {
            // 检测到色块：运行 PID 调整转向与速度，使色块保持在画面中心
            pid_color_track_update((int)target_pos_out.x, (int)target_pos_out.y);
        }
        else
        {
            // 色块丢失：停车（转向角保持当前值）
            set_target_speed(0);
        }
    }
}
