/**
 * @file main.c
 * @brief 固件唯一入口：板级初始化、时间源自检、应用分发和单快照协作循环。
 * @details 仅依赖语义接口，无 SDK/寄存器。执行器外设不配置；自检失败不启用任务。
 *          启动日志和诊断通过 BSP 有界轮询发送，不阻塞正常调度。
 */
#include "app/app_interface.h"
#include "app/app_config.h"
#include "board/bsp/board.h"
#include "board/bsp/timebase.h"
#include "middleware/scheduler.h"
/** 启动失败进入诊断循环，无执行器输出，无任务运行。 */
int main(void)
{
    board_init();
    (void)board_print_banner(APP_SELECTED_ID);
    timebase_init();
    const app_ops_t *app = app_get_selected();
    const bool ready = timebase_selfcheck_blocking() &&
                       scheduler_init() == SCHEDULER_OK && app->init();
    bool error_reported = false;
    for (;;) {
        const uint32_t now_ms = timebase_now_ms();
        if (ready) {
            scheduler_run_once(now_ms);
            app->run_once(now_ms);
        } else if (!error_reported && board_console_idle()) {
            static const char error[] = "P2 INIT_FAILED: timebase/app; tasks disabled\r\n";
            error_reported = board_console_write(error, sizeof(error) - 1u);
        }
        board_console_service();
    }
}
