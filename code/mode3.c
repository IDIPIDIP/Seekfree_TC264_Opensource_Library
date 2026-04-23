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

    pid_color_track_init();  // Initialise color tracking PID controllers
    set_control_mode(1);     // Enable auto-steering mode (steering angle driven by PID)

    while(1)
    {
        if(color_trace(&target_color_condi, &target_pos_out)) // x: 0-(SCC8660_W-1)  y: 0-(SCC8660_H-1)
        {
            // Block found: run PID to steer and accelerate so the block stays at frame center
            pid_color_track_update((int)target_pos_out.x, (int)target_pos_out.y);
        }
        else
        {
            // Block lost: stop the vehicle (steering angle holds its current value)
            set_target_speed(0);
        }
    }
}
