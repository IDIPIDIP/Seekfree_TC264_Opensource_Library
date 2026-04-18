#include "zf_common_headfile.h"

void coll_points(void)//过程中采点
{
    getpoint();
    //get_imu_point();
    tft180_clear();
    tft180_show_string(1,1,"point num");
    tft180_show_int(1,55,gps_point_data.point_num,2);
}

void inte_and_write_points(void)
{
    buffer_point.ID=0;//ID设置
    tft180_clear();
    tft180_show_string(1,1,"id");

    tft180_show_int(7,1,buffer_point.ID,2);
    while(1)
    {
    if(key_get_state(KEY_1) == KEY_SHORT_PRESS)//key1增加ID
    {
        key_clear_state(KEY_1);
        buffer_point.ID++;
        tft180_clear();
        tft180_show_string(1,1,"id");
        tft180_show_int(7,1,buffer_point.ID,2);
    }
    if(key_get_state(KEY_2) == KEY_SHORT_PRESS)//key2减少ID
    {
        key_clear_state(KEY_2);
        buffer_point.ID--;
        tft180_clear();
        tft180_show_string(1,1,"id");
        tft180_show_int(7,1,buffer_point.ID,2);
    }
    if(key_get_state(KEY_3) == KEY_SHORT_PRESS)//key3确认ID并写入flash
    {
        key_clear_state(KEY_3);
    //integrate(gps_point_data.gps_x, gps_point_data.gps_y,imu.x,imu.y,gps_point_data.point_num);
    buffer_point.point_num=gps_point_data.point_num;
    for(int i=0;i<gps_point_data.point_num;i++)
    {
        buffer_point.x[i]=integrate_point.x[i];
        buffer_point.y[i]=integrate_point.y[i];
    }
    flash_write_point_struct(buffer_point);
    tft180_clear();
    tft180_show_string(1,1,"write success");
    tft180_show_string(1,9,"id");
    tft180_show_int(15,9,buffer_point.ID,2);
    break;
    }
    }
}
