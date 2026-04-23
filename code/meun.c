#include "zf_common_typedef.h"
#include "zf_common_headfile.h"

void menu(void)
{
    // tft180_clear();
    // tft180_show_string(1,1,"key1 start");
    // tft180_show_string(9,1,"key2 ");
    // tft180_show_string(25,1,"key4 point");
    while (1)
    {
    tft180_clear();
    tft180_show_string(1,1,"key1 start");
    tft180_show_string(9,1,"key2 ");
    tft180_show_string(25,1,"key4 point");
    //key1准备科目
    if(key_get_state(KEY_1) == KEY_SHORT_PRESS)
    {
        key_clear_state(KEY_1);
        tft180_clear();
        tft180_show_string(1,1,"mode1 key1");
        tft180_show_string(9,1,"mode2 key2");
        tft180_show_string(17,1,"mode3 key3");
        while(1)
        {
            tft180_clear();
            tft180_show_string(1,1,"mode1 key1");
            tft180_show_string(9,1,"mode2 key2");
            tft180_show_string(17,1,"mode3 key3");
            //key1进入mode1
            if(key_get_state(KEY_1) == KEY_SHORT_PRESS)
            {
                key_clear_state(KEY_1);
                while(mode1() != 0);//路径ID为0
                break;
            }
            //key2进入mode2
            if(key_get_state(KEY_2) == KEY_SHORT_PRESS)
            {
                key_clear_state(KEY_2);
                asr_init();//语音识别初始化
                while(1)
                {
                tft180_clear();
                tft180_show_string(1,1,"key1 mode2_1");
                tft180_show_string(9,1,"key2 mode2_2");
                tft180_show_string(17,1,"key3 mode2_3");
                tft180_show_string(25,1,"key4 mode2_4");
                tft180_show_string(33,1,"long key1 exit");
                if(key_get_state(KEY_1) == KEY_SHORT_PRESS)//key1进入mode2_1
                {
                    key_clear_state(KEY_1);
                    mode2_1();
                }
                else if(key_get_state(KEY_2) == KEY_SHORT_PRESS)//key2进入mode2_2
                {
                    key_clear_state(KEY_2);
                    mode2_2();
                }
                else if(key_get_state(KEY_3) == KEY_SHORT_PRESS)//key3进入mode2_3
                {
                    key_clear_state(KEY_3);
                    mode2_3();
                }
                else if(key_get_state(KEY_4) == KEY_SHORT_PRESS)//key4进入mode2_4
                {
                    key_clear_state(KEY_4);
                    mode2_4();
                }
                //key1长按退出mode2
                if(key_get_state(KEY_1) == KEY_LONG_PRESS)
                {
                    key_clear_state(KEY_1);
                    wifi_uart_disconnected_wifi();
                    break;
                }
                }
            }
            if(key_get_state(KEY_3) == KEY_SHORT_PRESS)//key3进入mode3
            {
                key_clear_state(KEY_3);
                mode3();
                break;
            }
        }
    }
    //key2
    if(key_get_state(KEY_2) == KEY_SHORT_PRESS)
    {
        key_clear_state(KEY_2);
    
    }

    //key3
    if(key_get_state(KEY_3) == KEY_SHORT_PRESS)
    {
        key_clear_state(KEY_3);
    
    }

    //key4点位
    if(key_get_state(KEY_4) == KEY_SHORT_PRESS)
    {
        key_clear_state(KEY_4);
        tft180_clear();
        tft180_show_string(1,1,"key1 to get point");//key1进入采点mode,按1采一个点，按2本轮采点完成进入选择ID，按3退出
        tft180_show_string(9,1,"key2 to read");//key2进入读取mode，先选择ID，按1增加ID，按2减少ID，按3确认ID并读取flash
        tft180_show_string(17,1,"key3 to exit");//key3退出
        tft180_show_string(25,1,"key4 to ");
        gps_point_data.point_num=0;
        while(1)
        {
            //KEY1采点
            if(key_get_state(KEY_1) == KEY_SHORT_PRESS)
            {
                key_clear_state(KEY_1);
                coll_points();
            }

            //KEY2读取路线
            if(key_get_state(KEY_2) == KEY_SHORT_PRESS)//进入后先输入ID，按1增加ID，按2减少ID，按3确认ID并读取flash
            {
                key_clear_state(KEY_2);
                int read_id = 0;
                tft180_clear();
                tft180_show_string(1,1,"key1 to add ID");
                tft180_show_string(9,1,"key2 to sub ID");
                tft180_show_string(17,1,"key3 to confirm ID");
                tft180_show_string(25,1,"id");
                tft180_show_int(31,1,read_id,2);
                while(1)
                {
                    if(key_get_state(KEY_1) == KEY_SHORT_PRESS)//key1增加ID
                    {
                        key_clear_state(KEY_1);
                        if(read_id < SUBJECT_THREE_END) read_id++;
                        tft180_clear();
                        tft180_show_string(1,1,"id");
                        tft180_show_int(7,1,read_id,2);
                    }
                    if(key_get_state(KEY_2) == KEY_SHORT_PRESS)//key2减少ID
                    {
                        key_clear_state(KEY_2);
                        if(read_id > SUBJECT_ONE_BEGIN) read_id--;
                        tft180_clear();
                        tft180_show_string(1,1,"id");
                        tft180_show_int(7,1,read_id,2);
                    }
                    if(key_get_state(KEY_3) == KEY_SHORT_PRESS)//key3确认ID并读取flash
                    {
                        key_clear_state(KEY_3);
                        flash_read_struct_toBuffer((flash_id_enum)read_id);
                        tft180_clear();
                        tft180_show_string(1,1,"read success");
                        tft180_show_string(9,1,"id");
                        tft180_show_int(15,9,read_id,2);
                        break;
                    }
                }
                break;
            }

            //KEY3退出
            if(key_get_state(KEY_3) == KEY_SHORT_PRESS)
            {
                key_clear_state(KEY_3);
                break;
            }
        }
    }
    }
}
