#ifndef CODE_MENU_H_
#define CODE_MENU_H_

#include "zf_common_typedef.h"
#include "zf_common_headfile.h"

// 菜单状态机消费的按键事件。
typedef enum
{
	MENU_EVENT_NONE = 0,     // 无事件。
	MENU_EVENT_KEY1_SHORT,   // KEY_1 短按。
	MENU_EVENT_KEY2_SHORT,   // KEY_2 短按。
	MENU_EVENT_KEY3_SHORT,   // KEY_3 短按。
	MENU_EVENT_KEY4_SHORT,   // KEY_4 短按。
	MENU_EVENT_KEY1_LONG,    // KEY_1 长按。
} menu_event_t;

// 菜单状态机管理的页面枚举。
typedef enum
{
	MENU_PAGE_MAIN = 0,      // 主页面。
	MENU_PAGE_SUBJECT,       // 科目/模式选择页面。
	MENU_PAGE_MODE2,         // mode2 子菜单页面。
	MENU_PAGE_POINT,         // 采点页面。
	MENU_PAGE_POINT_READ_ID, // 路径读取 ID 页面。
	MENU_PAGE_COUNT,         // 页面数量。
} menu_page_t;

// 迁移命中后执行的动作回调。
typedef void (*menu_action_fn_t)(void);

// 页面转移表中的单条规则。
typedef struct
{
	menu_event_t event;        // 触发该迁移的事件。
	menu_action_fn_t action;   // 页面切换前可选执行的动作。
	menu_page_t next_page;     // 迁移后的目标页面。
} menu_transition_t;

// 初始化菜单运行时状态。
void menu_init(void);
// 执行一次菜单处理：采集事件、分发迁移、按需渲染。
void menu_process(void);
// 标记当前页面为脏，在下一次处理时强制重绘。
void menu_force_refresh(void);
// 兼容入口：阻塞循环调用 menu_process。
void menu(void);


#endif /* CODE_MENU_H_ */