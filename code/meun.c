#include "menu.h"

// 固定长度数组元素个数宏。
#define MENU_ARRAY_SIZE(array) ((uint8)(sizeof(array) / sizeof((array)[0])))

// 菜单状态机中单个页面的描述结构。
typedef struct
{
    void (*on_enter)(void);                    // 进入页面时可选执行的回调。
    void (*render)(void);                      // 该页面的渲染函数。
    const menu_transition_t *transitions;      // 该页面的事件迁移表。
    uint8 transition_count;                    // 迁移表项数量。
} menu_page_desc_t;

// 运行时标志与当前页面指针。
static uint8 g_menu_inited = 0;// 菜单初始化标志。
static uint8 g_page_dirty = 1;// 页面脏标志，指示是否需要重绘
static menu_page_t g_current_page = MENU_PAGE_MAIN;

// 读取 ID 页面使用的运行时数据。
static int g_read_id = (int)SUBJECT_ONE_BEGIN;
static uint8 g_has_read_notice = 0;
static int g_last_read_id = 0;

// 状态机核心辅助函数声明。
static menu_event_t menu_poll_event(void);
static void menu_dispatch_event(menu_event_t event);
static void menu_set_page(menu_page_t next_page);

// 页面渲染函数声明。
static void menu_render_main(void);
static void menu_render_subject(void);
static void menu_render_mode2(void);
static void menu_render_point(void);
static void menu_render_point_read_id(void);

// 页面进入钩子函数声明。
static void menu_on_enter_point(void);
static void menu_on_enter_point_read_id(void);

// 迁移表中引用的动作函数声明。
static void menu_action_mode1(void);
static void menu_action_mode2_enter(void);
static void menu_action_mode2_exit(void);
static void menu_action_mode2_1(void);
static void menu_action_mode2_2(void);
static void menu_action_mode2_3(void);
static void menu_action_mode2_4(void);
static void menu_action_mode3(void);
static void menu_action_collect_point(void);
static void menu_action_read_id_inc(void);
static void menu_action_read_id_dec(void);
static void menu_action_read_id_confirm(void);

// 主页面迁移表。
static const menu_transition_t g_main_transitions[] =
{
    {MENU_EVENT_KEY1_SHORT, NULL, MENU_PAGE_SUBJECT}, // 进入科目页面。
    {MENU_EVENT_KEY4_SHORT, NULL, MENU_PAGE_POINT},   // 进入采点页面。
};

// 科目页面迁移表。
static const menu_transition_t g_subject_transitions[] =
{
    {MENU_EVENT_KEY1_SHORT, menu_action_mode1, MENU_PAGE_MAIN},        // 执行 mode1 后回主页面。
    {MENU_EVENT_KEY2_SHORT, menu_action_mode2_enter, MENU_PAGE_MODE2}, // 初始化 ASR 并进入 mode2 子菜单。
    {MENU_EVENT_KEY3_SHORT, menu_action_mode3, MENU_PAGE_MAIN},        // 执行 mode3 后回主页面。
    {MENU_EVENT_KEY4_SHORT, NULL, MENU_PAGE_MAIN},                     // 返回主页面。
};

// mode2 子菜单迁移表。
static const menu_transition_t g_mode2_transitions[] =
{
    {MENU_EVENT_KEY1_SHORT, menu_action_mode2_1, MENU_PAGE_MODE2},     // 执行 mode2_1。
    {MENU_EVENT_KEY2_SHORT, menu_action_mode2_2, MENU_PAGE_MODE2},     // 执行 mode2_2。
    {MENU_EVENT_KEY3_SHORT, menu_action_mode2_3, MENU_PAGE_MODE2},     // 执行 mode2_3。
    {MENU_EVENT_KEY4_SHORT, menu_action_mode2_4, MENU_PAGE_MODE2},     // 执行 mode2_4。
    {MENU_EVENT_KEY1_LONG, menu_action_mode2_exit, MENU_PAGE_SUBJECT}, // KEY1 长按退出 mode2 子菜单。
};

// 采点页面迁移表。
static const menu_transition_t g_point_transitions[] =
{
    {MENU_EVENT_KEY1_SHORT, menu_action_collect_point, MENU_PAGE_POINT}, // 采集一个点。
    {MENU_EVENT_KEY2_SHORT, NULL, MENU_PAGE_POINT_READ_ID},              // 进入读取 ID 页面。
    {MENU_EVENT_KEY3_SHORT, NULL, MENU_PAGE_MAIN},                       // 返回主页面。
    {MENU_EVENT_KEY4_SHORT, NULL, MENU_PAGE_MAIN},                       // 返回主页面。
};

// 读取 ID 页面迁移表。
static const menu_transition_t g_point_read_id_transitions[] =
{
    {MENU_EVENT_KEY1_SHORT, menu_action_read_id_inc, MENU_PAGE_POINT_READ_ID},     // 增加 ID。
    {MENU_EVENT_KEY2_SHORT, menu_action_read_id_dec, MENU_PAGE_POINT_READ_ID},     // 减少 ID。
    {MENU_EVENT_KEY3_SHORT, menu_action_read_id_confirm, MENU_PAGE_MAIN},           // 确认并读取路径。
    {MENU_EVENT_KEY4_SHORT, NULL, MENU_PAGE_POINT},                                 // 返回采点页面。
};

// 所有页面的静态注册表。
static const menu_page_desc_t g_pages[MENU_PAGE_COUNT] =
{
    [MENU_PAGE_MAIN] =
    {
        NULL,
        menu_render_main,
        g_main_transitions,
        MENU_ARRAY_SIZE(g_main_transitions),
    },
    [MENU_PAGE_SUBJECT] =
    {
        NULL,
        menu_render_subject,
        g_subject_transitions,
        MENU_ARRAY_SIZE(g_subject_transitions),
    },
    [MENU_PAGE_MODE2] =
    {
        NULL,
        menu_render_mode2,
        g_mode2_transitions,
        MENU_ARRAY_SIZE(g_mode2_transitions),
    },
    [MENU_PAGE_POINT] =
    {
        menu_on_enter_point,
        menu_render_point,
        g_point_transitions,
        MENU_ARRAY_SIZE(g_point_transitions),
    },
    [MENU_PAGE_POINT_READ_ID] =
    {
        menu_on_enter_point_read_id,
        menu_render_point_read_id,
        g_point_read_id_transitions,
        MENU_ARRAY_SIZE(g_point_read_id_transitions),
    },
};

// 渲染主页面。
static void menu_render_main(void)
{
    tft180_clear();
    tft180_show_string(1, 1, "key1 start");
    tft180_show_string(1, 9, "key4 point");
    tft180_show_string(1, 17, "main menu");
    if(g_has_read_notice)
    {
        tft180_show_string(1, 25, "read id");
        tft180_show_int(43, 25, g_last_read_id, 2);
    }
}

// 渲染科目选择页面。
static void menu_render_subject(void)
{
    tft180_clear();
    tft180_show_string(1, 1, "mode1 key1");
    tft180_show_string(9, 1, "mode2 key2");
    tft180_show_string(17, 1, "mode3 key3");
    tft180_show_string(25, 1, "back  key4");
}

// 渲染 mode2 子菜单页面。
static void menu_render_mode2(void)
{
    tft180_clear();
    tft180_show_string(1, 1, "key1 mode2_1");
    tft180_show_string(9, 1, "key2 mode2_2");
    tft180_show_string(17, 1, "key3 mode2_3");
    tft180_show_string(25, 1, "key4 mode2_4");
    tft180_show_string(33, 1, "long key1 exit");
}

// 渲染采点页面。
static void menu_render_point(void)
{
    tft180_clear();
    tft180_show_string(1, 1, "key1 get point");
    tft180_show_string(9, 1, "key2 read path");
    tft180_show_string(17, 1, "key3/4 to exit");
    tft180_show_string(25, 1, "point num");
    tft180_show_int(25, 49, gps_point_data.point_num, 3);
}

// 渲染路径读取 ID 页面。
static void menu_render_point_read_id(void)
{
    tft180_clear();
    tft180_show_string(1, 1, "key1 add id");
    tft180_show_string(9, 1, "key2 sub id");
    tft180_show_string(17, 1, "key3 confirm");
    tft180_show_string(25, 1, "key4 back");
    tft180_show_string(33, 1, "id");
    tft180_show_int(33, 17, g_read_id, 2);
}

// 进入采点页面时，重置本次会话采点计数。
static void menu_on_enter_point(void)
{
    gps_point_data.point_num = 0;
}

// 进入读取 ID 页面时，将当前 ID 重置为最小有效值。
static void menu_on_enter_point_read_id(void)
{
    g_read_id = (int)SUBJECT_ONE_BEGIN;
}

// 执行 mode1 动作。
static void menu_action_mode1(void)
{
    while(mode1() != 0)
    {
    }
}

// 进入 mode2 子菜单前初始化资源。
static void menu_action_mode2_enter(void)
{
    asr_init();
}

// 离开 mode2 子菜单时释放资源。
static void menu_action_mode2_exit(void)
{
    wifi_spi_socket_disconnect();
}

// 执行 mode2_1，返回后强制重绘当前页面。
static void menu_action_mode2_1(void)
{
    mode2_1();
    g_page_dirty = 1;
}

// 执行 mode2_2，返回后强制重绘当前页面。
static void menu_action_mode2_2(void)
{
    mode2_2();
    g_page_dirty = 1;
}

// 执行 mode2_3，返回后强制重绘当前页面。
static void menu_action_mode2_3(void)
{
    mode2_3();
    g_page_dirty = 1;
}

// 执行 mode2_4，返回后强制重绘当前页面。
static void menu_action_mode2_4(void)
{
    mode2_4();
    g_page_dirty = 1;
}

// 执行 mode3 动作。
static void menu_action_mode3(void)
{
    mode3();
}

// 采集一个点，并刷新页面点数显示。
static void menu_action_collect_point(void)
{
    coll_points();
    g_page_dirty = 1;
}

// 在合法 Flash ID 范围内增加读取 ID。
static void menu_action_read_id_inc(void)
{
    if(g_read_id < (int)SUBJECT_THREE_END)
    {
        g_read_id++;
        g_page_dirty = 1;
    }
}

// 在合法 Flash ID 范围内减少读取 ID。
static void menu_action_read_id_dec(void)
{
    if(g_read_id > (int)SUBJECT_ONE_BEGIN)
    {
        g_read_id--;
        g_page_dirty = 1;
    }
}

// 确认读取 ID，并从 Flash 读取路径数据。
static void menu_action_read_id_confirm(void)
{
    flash_read_struct_toBuffer((flash_id_enum)g_read_id);
    g_last_read_id = g_read_id;
    g_has_read_notice = 1;
}

// 读取按键状态并转换为统一菜单事件。
static menu_event_t menu_poll_event(void)
{
    if(key_get_state(KEY_1) == KEY_LONG_PRESS)
    {
        key_clear_state(KEY_1);
        return MENU_EVENT_KEY1_LONG;
    }
    if(key_get_state(KEY_1) == KEY_SHORT_PRESS)
    {
        key_clear_state(KEY_1);
        return MENU_EVENT_KEY1_SHORT;
    }
    if(key_get_state(KEY_2) == KEY_SHORT_PRESS)
    {
        key_clear_state(KEY_2);
        return MENU_EVENT_KEY2_SHORT;
    }
    if(key_get_state(KEY_3) == KEY_SHORT_PRESS)
    {
        key_clear_state(KEY_3);
        return MENU_EVENT_KEY3_SHORT;
    }
    if(key_get_state(KEY_4) == KEY_SHORT_PRESS)
    {
        key_clear_state(KEY_4);
        return MENU_EVENT_KEY4_SHORT;
    }
    return MENU_EVENT_NONE;
}

// 切换到目标页面，仅在页面发生变化时执行进入钩子。
static void menu_set_page(menu_page_t next_page)
{
    if(next_page >= MENU_PAGE_COUNT)
    {
        return;
    }

    if(next_page != g_current_page)
    {
        g_current_page = next_page;
        g_page_dirty = 1;
        if(g_pages[g_current_page].on_enter != NULL)
        {
            g_pages[g_current_page].on_enter();
        }
    }
}

// 在当前页面迁移表中扫描并分发一次事件。
static void menu_dispatch_event(menu_event_t event)
{
    uint8 i = 0;
    const menu_page_desc_t *desc = &g_pages[g_current_page];

    for(i = 0; i < desc->transition_count; i++)
    {
        const menu_transition_t *transition = &desc->transitions[i];
        if(transition->event != event)
        {
            continue;
        }

        if(transition->action != NULL)
        {
            transition->action();
        }

        menu_set_page(transition->next_page);
        return;
    }
}

// 初始化菜单运行时数据，并清理历史按键状态。
void menu_init(void)
{
    g_current_page = MENU_PAGE_MAIN;
    g_page_dirty = 1;
    g_menu_inited = 1;
    g_read_id = (int)SUBJECT_ONE_BEGIN;
    g_has_read_notice = 0;
    g_last_read_id = 0;
    key_clear_all_state();
}

// 将当前页面标记为脏，下一次处理时重绘。
void menu_force_refresh(void)
{
    g_page_dirty = 1;
}

// 一次非阻塞菜单处理：先按需渲染，再处理一个事件，再按需重绘。
void menu_process(void)
{
    menu_event_t event;

    if(!g_menu_inited)
    {
        menu_init();
    }

    if(g_page_dirty)
    {
        g_pages[g_current_page].render();
        g_page_dirty = 0;
    }

    event = menu_poll_event();
    if(event != MENU_EVENT_NONE)
    {
        menu_dispatch_event(event);
    }

    if(g_page_dirty)
    {
        g_pages[g_current_page].render();
        g_page_dirty = 0;
    }
}

// 兼容接口：阻塞循环持续调用 menu_process。
void menu(void)
{
    menu_init();
    while(1)
    {
        menu_process();
    }
}
