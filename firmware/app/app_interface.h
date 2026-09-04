/**
 * @file app_interface.h
 * @brief 编译期应用分发表；依赖标准类型，main 通过唯一 ops 表启动和运行。
 */
#ifndef APP_INTERFACE_H_
#define APP_INTERFACE_H_
#include <stdbool.h>
#include <stdint.h>
typedef struct {
    bool (*init)(void);               /**< 初始化一次，失败返回 false。 */
    void (*run_once)(uint32_t now_ms); /**< 单快照、非阻塞主循环服务。 */
    void (*deinit)(void);             /**< 停用自身任务，可重复调用。 */
} app_ops_t;
/** 返回编译期选择的非空静态表；非法 ID 在 app_config.h 报错。 */
const app_ops_t *app_get_selected(void);
#endif
