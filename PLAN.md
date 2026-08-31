# MSPM0G3507 电赛控制类模块库实施计划（v7.2，纠偏补丁）

> v7.2 为纠偏补丁（**不改功能范围**）：① 阶段状态增加 `NOT_STARTED / IN_PROGRESS`，当前如实状态见 `docs/STATUS.md`（P0/P1A=COMPLETED、P1=IN_PROGRESS 等 80MHz 冷启动、P2/P3=NOT_STARTED；`ring_buffer`/`frame_codec` 仅为提前完成的纯软件预研资产，**非阶段完成**）；② I2C 恢复语义改为"超时即返回错误 + `recovery_pending`，控制器恢复由低优先级 service/SAFE 完成，不计入原事务 API"；③ `mspm0-ccs` skill 降为**可选自动化辅助**，强制规则以仓库内 `AGENTS.md`/`PLAN.md` 为准。真实问题通过代码审查、硬件日志与 ADR 修正。

## Context（目标 + 开发方式约束）

为 2027 电赛控制类题目，在**天猛星 MSPM0G3507** 上用 CCS 实现控制类模块库：电机+编码器+速度PI、舵机、串口帧协议、OLED+MPU6050。`mspm0-ccs` skill 为**可选自动化辅助**（已装、全局可用、描述匹配自动触发）；**所有强制规则以本仓库 `AGENTS.md` 与 `PLAN.md` 为准，不依赖协作者机器上的全局 skill**。

**开发方式**：大学生独自用 AI 工具开发，硬件逐步采购、逐步上板验证，GitHub 分支管理。因此：核心模块库与可选扩展分层、硬件参数按阶段局部门禁、验收分"基础必做/有仪器选做"、允许 `SOFTWARE_READY`（软件完成、硬件待验）。

**范围分层**：
```text
核心模块库（发布门槛）: UART0调试、UART1 transport、frame_codec、ring_buffer、
  TB6612电机、编码器测速、速度PI、舵机PWM、I2C、SSD1306 OLED、
  MPU6050原始、基础姿态(roll/pitch/gyro_z/yaw_rel)
可选扩展（不阻塞核心发布）: E1 K230会话层  E2 航向控制+云台  E3 性能/功耗/编译优化（80MHz 已于 P1 收口转入正式基线）  E4 WWDT+可靠性
核心发布点 = P10 核心集成冒烟通过
```
**本计划完全自包含，不依赖任何早期版本。**

## 一、配置事实源

| 文件 | 内容 | 权威性 |
|---|---|---|
| `control.syscfg` | 外设实例、引脚、时钟、外设底层参数 | 外设配置的**唯一机器事实源** |
| `firmware/config/hardware_config.h` | 已确认**物理硬件参数**（电机/编码器极性、编码器计数、舵机实测范围、I2C 地址、MPU6050 轴映射、供电/板级） | 已确认物理参数的**唯一编译期来源** |
| `firmware/config/project_config.h` | **软件策略参数**（UART 波特率、缓冲容量、帧超时、任务周期、STALL 阈值、IMU_STALE_MS、滤波参数、bringup 上限、I2C 超时） | 软件策略参数的**唯一编译期来源** |
| `firmware/config/config_validate.h` | 编译期合法性检查（**全部 `#if READY` 条件编译**）+ 各模块 READY 标志 | 编译期校验 |
| `firmware/app/app_config.h` | 当前 APP_ID、UART 链路类型、测试模式开关 | 当前构建选择 |
| `HARDWARE_PROFILE.md` | 物理参数来源与状态 `UNKNOWN/DATASHEET/MEASURED`、测量方法、历史 | 文档事实源（硬件） |
| `TUNING_LOG.md` | 软件控制/策略参数来源与整定历史 `DESIGN_DEFAULT/CALCULATED/MEASURED/TUNED` | 文档事实源（软件调参） |
| `PINMAP.md` | 人类可读镜像，逐引脚 `DRAFT/FROZEN` | 无权威，随 syscfg 同步 |

**config_validate.h 条件检查（示例）**：
```c
#if MOTOR_CONFIG_READY
_Static_assert(MOTOR_LEFT_DIRECTION_SIGN==1 || MOTOR_LEFT_DIRECTION_SIGN==-1, "...");
#endif
#if ENCODER_CONFIG_READY
_Static_assert(COUNTS_PER_OUTPUT_REV_MEASURED>0, "...");
#endif
#if SERVO_CONFIG_READY
_Static_assert(SERVO_SAFE_MIN_US < SERVO_CENTER_US && SERVO_CENTER_US < SERVO_SAFE_MAX_US, "...");
#endif
#if I2C_CONFIG_READY
_Static_assert(I2C_TRANSACTION_TIMEOUT_MS < CONTROL_LATE_TOLERANCE_MS, "...");
#endif
#if IMU_CONFIG_READY
_Static_assert(IMU_STALE_MS > IMU_TASK_PERIOD_MS, "...");
#endif
```
**READY 标志**：`MOTOR_CONFIG_READY / ENCODER_CONFIG_READY / SERVO_CONFIG_READY / I2C_CONFIG_READY / IMU_CONFIG_READY / UART1_LINK_CONFIG_READY`。规则：`READY=0` 时对应参数宏可不定义；只有模块源文件真正进工程才要求 `READY=1`；测试 APP 只检查自身依赖；**P10 集成 APP 要求全部核心 READY=1**；禁为编译通过填伪造配置。
冲突规则：BOARD_CONSTRAINTS↔syscfg→改 syscfg；RESOURCE_MAP↔syscfg→停止不得自动换实例；PINMAP↔syscfg→以 syscfg 为准；文档与 hardware_config.h 不一致→停止同步。**UNKNOWN 参数不得以猜测值进入任何编译配置。**
参数分工：HARDWARE_PROFILE=物理硬件/测量数据；TUNING_LOG=软件控制/策略参数（PI、STALL、滤波 τ、超时、迟到阈值）；project_config.h=当前实际编译值。STALL 阈值只有确实直接来自器件手册才标 DATASHEET。

## 二、引脚

```text
FIXED_BOARD_FUNCTION: PA5/PA6(HFXT) PA19/PA20(SWD) PA10/PA11(UART0/CH340) PB22(LED)
DO_NOT_USE: PA02, PA18, PA21, PA23
```
PB6/PB7 在 PINMAP 中须明确谁是 TX 谁是 RX。

**两阶段冻结**：
- **P1A 资源可解性预检**（`docs/preflight/pin_preflight.syscfg`，标 `NON_BUILDING_REFERENCE / DO_NOT_ADD_TO_CCS_PROJECT`）：复制当前 control.syscfg 临时加入全部未来资源做 SysConfig 冲突检查；通过后写 RESOURCE_MAP 与 PINMAP 的 DRAFT 行；**不加入 CCS 构建**。后续阶段正式加入时把对应行改 FROZEN。
- 冻结后禁自动换引脚；确需更换→决策记录 `docs/decisions/ADR-xxx-pin-change.md`。

## 三、资源、中断所有权、阶段依赖

| 资源 | 用途 | 约束 |
|---|---|---|
| TIMG12 | 1ms 时间基准 | ISR 只 `g_tick_ms++`（见第六节） |
| TIMA0 | 左右电机 PWM 20kHz | 两个相互独立边沿对齐通道；默认不启用互补输出/硬件 dead-band/外部 fault；换向由 actuator_guard 软件状态机实现 |
| TIMA1 | 双舵机 50Hz | 预分频由 SysConfig 算；定时器由 servo_pwm 整体拥有（P1A 决策：TIMG6 在 LQFP-64 唯一 CCP0=PA21 属 DO_NOT_USE，改用 TIMA1 PA17/PA16，见 RESOURCE_MAP 变更记录） |
| TIMG8 | 可选单轴 HW QEI（默认禁用） | 唯一 QEI 实例，不作双轮默认 |
| TIMG7 | 备用 | 默认不启用 |
| UART0/PA10-11 | 文本调试日志 | 非阻塞 ≥512B 环形，满丢整行+debug_drop_lines++ |
| UART1/PB6-7 | 二进制协议（K230/HC-04） | 环形 RX/TX 各 512B，静态分配禁 malloc，波特率链路配置 |
| I2C0 | OLED + MPU6050 | 100k→400k，同步阻塞+有限超时 |

**中断所有权（集中式，硬件状态读取/清除责任唯一）**：只有 `bsp/interrupts.c` 定义真实 ISR。`interrupts.c` **唯一读取并确认/清除硬件事件**，把已解码事件传给模块回调；**模块回调不访问 IIDX、不再清除中断位**，只更新内部状态。若本地 DriverLib 的 IIDX 读取本身完成确认，在 MODULE_STANDARD.md 注明，避免再调清除接口。ISR 内禁打印/等待/PI/OLED。IRQ 归属：TIMG12→tick、UART0→uart_debug、UART1→uart1_transport、GPIOA/B→encoder/gpio 分发、I2C0→（核心用轮询可不定义）。

**阶段依赖表**（P1A 部分失败只阻塞相关阶段）：
```text
P2←P1+TIMG12可分配   P3←P2+UART0/1可分配   P4←P1A电机资源+电机参数
P5←P1A编码器GPIO+编码器参数   P6-SOFTWARE←P4/P5接口已冻结
P6-BOARD←P4 COMPLETED+P5 COMPLETED   P7←TIMA1资源+舵机参数
P8←I2C0资源+OLED硬件   P9←I2C总线过+MPU6050硬件
```

## 四、安全

### 分层职责
```text
drivers/motor_tb6612      PWM、方向、STBY、制动、motor_emergency_stop() —— 不查全局状态、不访问 PI
safety/actuator_guard     双电机独立通道、输出限幅、换向 deadtime、运动命令超时 —— 非阻塞状态机
safety/safety_manager     故障上报/仲裁、safety_emergency_stop、PI reset、返回建议动作
app/integration_app       系统状态机切换、模式管理（只做状态切换，不做执行器关闭细节）
```

### actuator_guard：双电机独立状态（核心结构性约束）
```c
typedef struct {
    actuator_channel_state_t state;   /* DISABLED/ENABLING/ACTIVE/REVERSING */
    motor_command_t current_command;
    motor_command_t pending_command;
    uint32_t transition_ready_ms;
} actuator_channel_t;

typedef struct {
    actuator_channel_t channel[MOTOR_COUNT];
    bool power_stage_enabled;          /* TB6612 STBY 聚合 */
} actuator_guard_t;
```
接口：
```c
actuator_status_t actuator_guard_set_command(motor_id_t motor, motor_command_t cmd, uint32_t now_ms);
void actuator_guard_service(uint32_t now_ms);
void actuator_guard_disable_all(void);
bool actuator_guard_any_active(void);
bool actuator_guard_all_zero(void);
```
行为写死：
- **单侧换向只清零该侧 PWM，另一侧可继续运行**。
- **TB6612 STBY 两路共用**：只有全局停止、DISARM 或故障时才拉低。
- 一侧命令为零、另一侧非零时系统仍保持 RUNNING。
- "zero_command → RUNNING→ARMED" 必须定义为**左右两轮目标均零**（`actuator_guard_all_zero()`），不是任意一侧为零。
- 紧急停止仍同时停两路并拉低公共 STBY。

每通道事件表（同前单通道语义，按通道独立）：
```text
DISABLED + enable                   → ENABLING（该通道 STBY 受聚合控制）
ENABLING + enable_delay_expired     → ACTIVE（该通道上报 ACTUATOR_ACTIVE）
ENABLING + stop/disarm/fault        → DISABLED
ACTIVE + same_sign_command          → 更新输出
ACTIVE + opposite_sign_command      → REVERSING，PWM 归零，保存 pending_command
ACTIVE + zero_command               → ACTIVE，PWM=0
REVERSING + new_nonzero_command     → 最新命令覆盖 pending_command
REVERSING + zero_command            → 取消 pending_command，保持 PWM=0
REVERSING + reverse_delay_expired   → pending 非零则换向进 ACTIVE；为零则 ACTIVE 零输出
任意状态 + disable/fault            → DISABLED
```
**固定"最新命令覆盖旧命令"，不得排队执行过期运动命令。**

### 系统状态机与 actuator_guard 对齐
```text
SAFE：   未授权；所有 channel 必须 DISABLED；STBY 低。
ARMED：  已授权无功率输出；所有 channel 必须 DISABLED 或 ENABLING；STBY 默认低。
RUNNING：至少一个 channel 为 ACTIVE 或 REVERSING；允许 STBY 高。
```
系统转换（事件表）：
```text
BOOT + INIT_OK                    → SAFE
BOOT + INIT_FAILED                → FAULT
SAFE + ARM + prerequisites_ok     → ARMED     ; SAFE + motion_command → reject
ARMED + valid_nonzero_command     → 提交命令+请求 enable；系统暂保持 ARMED
ARMED + any_channel_ACTIVE        → RUNNING
ARMED + DISARM                    → SAFE
RUNNING + all_wheel_commands_zero → 各通道 disable + PWM=0 + STBY=LOW → ARMED
RUNNING + DISARM                  → SAFE
RUNNING + command_timeout         → safety_emergency_stop → SAFE
任意状态 + LATCHED_FAULT          → FAULT
FAULT + CLEAR_FAULT + cause_gone  → SAFE
```
**`all_wheel_commands_zero` 定义**：左右轮有效目标均零，且无待执行换向命令；仅该条件成立才允许 RUNNING→ARMED。**进入 ARMED 时必须**：左右 PWM=0、STBY=LOW、左右 channel=DISABLED、左右 PI reset。补充：ARMED 期间新非零命令更新 pending_command；ENABLING 期间零命令取消启用回 DISABLED；**进入 SAFE/FAULT 必须验证所有 channel==DISABLED**；不允许只改系统枚举不改驱动输出；STOP 后/HEARTBEAT 恢复后不得自动回 RUNNING；FAULT 只显式清除。

### safety_manager（故障上报/仲裁/清除，接口一致）
```c
typedef enum { SAFETY_ACTION_NONE, SAFETY_ACTION_WARN,
               SAFETY_ACTION_ENTER_SAFE, SAFETY_ACTION_ENTER_FAULT } safety_action_t;

safety_action_t safety_report(stop_reason_t reason, bool condition_active);
safety_action_t safety_emergency_stop(stop_reason_t reason);
bool safety_clear_latched(stop_reason_t reason);
stop_reason_t safety_get_primary_reason(void);
uint32_t safety_get_active_fault_mask(void);
uint32_t safety_get_fault_count(stop_reason_t reason);
```
固定规则：
- `safety_report(reason, true)` 激活故障；`false` 表示故障源恢复。
- WARN 可自动清除 active 位；STOP 恢复后仍保持 SAFE 须重新 ARM；LATCHED 只能 `safety_clear_latched()` 清除。
- 应用层根据 `safety_action_t` 切换 SAFE 或 FAULT。
- **故障仲裁**：严重度 LATCHED>STOP>WARN>INFO；primary_reason=最高严重度第一个原因，低严重度不覆盖；active_fault_mask 记录全部现存故障；fault_count 累计。
- 编译期校验：`_Static_assert(STOP_REASON_COUNT <= 32, "active_fault_mask too small")`；超 32 改用 uint64_t/多位图。
- `safety_emergency_stop`：motor_emergency_stop + 左右 PI reset + actuator_guard_reset + 记录原因 + 返回建议动作。

### 紧急停止（驱动层同步生效）
```c
void motor_emergency_stop(void);   /* STBY拉低→PWM清零→取消内部待执行动作→立即返回；幂等/无阻塞/无日志/不依赖调度器 */
```
**紧急停止一律 STBY 低，不用 BRAKE。** 同步故障调用 `safety_emergency_stop(reason)` 立即执行，不等待下一个调度周期。

### PWM 定时器禁止自动启动（P4/P7 门禁）
TIMA0 与 TIMA1 初始化后**不得自动启动计数器**；所有比较值初始化为 0；PWM 输出默认无效电平。电机由 `motor_enable()` 显式启动；舵机由 `servo_enable(channel)` 显式启用。Agent 只读核对生成 `ti_msp_dl_config.c`，**不手改生成文件**。

### STBY 硬件门禁（P4）
STBY 必须接 MSPM0 GPIO、**必须外部下拉（建议 10kΩ）**、禁止直接接 3.3V。HARDWARE_PROFILE 增 `tb6612_stby_gpio / tb6612_stby_external_pulldown / tb6612_stby_reset_level_measured`。用户须确认：断电测 STBY 与地有下拉；MCU 复位时 STBY 低；烧录期间电机不动；SysConfig STBY 初始输出低。

### 电机 bringup 分级 + 默认行为
首次上电最大命令 1000→确认方向/停止正常 2000→确认电流/声音/温升 3000；代码硬限 `MOTOR_BRINGUP_MAX_COMMAND=3000`，超 3000 须用户显式确认。零命令默认 COAST；BRAKE 仅显式接口；`motor_set_command()` 不得自动选 BRAKE。

### 极性配置
`MOTOR_LEFT_DIRECTION_SIGN / MOTOR_RIGHT_DIRECTION_SIGN / ENCODER_LEFT_DIRECTION_SIGN / ENCODER_RIGHT_DIRECTION_SIGN`（hardware_config.h）。应用层正 wheel_rpm 恒为前进；驱动层转真实 GPIO 方向；编码器层转统一符号；**PI 层不知电机安装方向**。P4/P5 后标 MEASURED。

### 测试程序安全（凡涉电机/舵机）
启动倒计时、自动停止时间、串口提示、无需通信自动回安全态、复位后默认禁止输出。

## 五、故障分级 + 模式依赖（完整）

| 等级 | 行为 | 典型故障 |
|---|---|---|
| INFO | 仅记录 | 通信统计变化 |
| WARN | 计数+降级，不停车 | UART0 日志丢、OLED 离线、遥测漏执行 |
| STOP | 回 SAFE 可再 ARM | 控制心跳超时、堵转、控制任务严重超期、PI 异常 |
| LATCHED | 进 FAULT 须 CLEAR_FAULT | 时钟异常、内部不变量破坏、看门狗复位后锁存 |

固定规则：UART0 满→WARN；OLED 失败→WARN 停刷新；均不停电机。
**模式依赖**：`SPEED_ONLY`＝MPU6050 失效→继续双轮速度闭环，WARN，禁用一切航向指令；`YAW_CONTROL / TURN_RELATIVE`＝MPU6050 失效→停止、进 SAFE、记录 STOP 原因。"拔 MPU6050 不停速度闭环"只对 SPEED_ONLY 成立。
**堵转判据（独立窗口，见第八节）**：阈值入 project_config.h（来源记录于 TUNING_LOG），不猜测。
控制级任务连续 2 次超期或单次 >20ms→STOP；遥测/OLED 超期只 WARN。

## 六、调度

- **TIMG12 ISR**：interrupts.c 读 IIDX 确认/清除后调 `tick_on_period_irq()`，其**只做 `g_tick_ms++`**。1ms 是**纯时间基准**，不作为必须每毫秒执行的主循环任务。
- `g_tick_ms` 为 4 字节对齐 `volatile uint32_t`；时间差一律 `(int32_t)(now-due)>=0`；所有周期/超时 < 2^31 ms；禁直接比 `now>deadline`。
- **主循环单次快照**：
```c
for (;;) { uint32_t now_ms = tick_now_ms(); scheduler_run_once(now_ms); app_run_once(now_ms); }
```
- **调度器接口（含优先级）**：
```c
typedef void (*task_callback_t)(uint32_t now_ms);
scheduler_status_t scheduler_register(task_id_t id, uint32_t period_ms,
                                      task_priority_t priority, task_callback_t cb);
scheduler_status_t scheduler_enable(task_id_t id, uint32_t now_ms);
scheduler_status_t scheduler_disable(task_id_t id);
void scheduler_run_once(uint32_t now_ms);
```
规则：固定数组容量 ≤16、禁动态分配；`period_ms==0` 或 `cb==NULL` 报错；相同 task_id 禁重复注册；未启用任务不参与 lateness 统计；P2 注册合成计数任务，不创建未来模块空回调。
- **固定优先级顺序（同优先级按 task_id 序）**：
```text
0 有界 UART1 接收与控制事件解析    1 安全状态处理    2 编码器测速+速度PI
3 MPU6050采样+姿态                  4 遥测            5 OLED刷新
```
UART1 解析置于速度 PI 前，避免 E1 的 STOP/DISARM 延迟一个控制周期；每轮处理字节数严格有上限（见第七节）。
- **正式任务表（核心运行时不注册 1ms 协作任务）**：
```text
5ms：安全监督、超时和快速输入    10ms：MPU6050采样+姿态    20ms：编码器测速+速度PI
100ms：遥测                      200ms：OLED分块刷新
```
同步故障仍立即 `safety_emergency_stop()`，不等 5ms 任务。P2 可保留 1ms 合成任务仅作调度器压力测试，但须注明：仅 P2 用、P10 不注册。
- **next_due 更新算法**（不补跑、不漂移）：
```c
lateness = now - next_due; missed = lateness / period;
missed_count += missed; next_due += (missed + 1) * period;
```
- **速度 PI 固定名义周期**：`PI_NOMINAL_DT_S=0.020f`；正常执行始终用 0.020s，不用迟到后实际大 dt 补偿；迟到超容差→本周期不运行 PI、STOP/安全降级；不补跑；`pi_init()` dt 与调度周期经静态/启动检查一致。
- **P2 验收**：注册 1/10/20ms 合成任务跑 10min，误差 ≤1 周期、missed_count==0、tick 单调、同点到期顺序正确；host 做 UINT32_MAX 附近回绕测试。
- 测试接口 + **唯一分发**：见下方 app 入口。

### 测试 APP 唯一分发（补回，防重复符号）
```c
/* app_interface.h */
typedef struct { bool (*init)(void); void (*run_once)(uint32_t now_ms); void (*deinit)(void); } app_ops_t;
const app_ops_t *app_get_selected(void);
```
测试文件用**唯一函数名**（如 `test_motor_init/test_motor_run_once/test_motor_deinit`、`test_encoder_*`…）；`app/app_dispatch.c` 根据 `APP_ID` 构建唯一函数表返回 `app_ops_t`。**禁止**：多文件定义同名 `app_init()`、依赖 CCS GUI "Exclude from Build" 切换、删除其他测试文件解决重复符号。全工程唯一 `main()`；测试文件不定义 `main()`。

## 七、通信

### UART 链路配置
```text
UART1_LOOPBACK(115200) | K230_DIRECT(两端共同配置) | HC04_BRIDGE(读取 HC-04 真实波特率，默认不得假定 115200)
```
参数入 project_config.h：`uart1_baud / uart1_inter_byte_timeout_ms / uart1_rx_capacity / uart1_tx_capacity`。
**ACK 超时按链路计算**：`frame_time_ms = ceil(frame_bytes×10×1000/baud)`；`ack_timeout_ms ≥ request_frame_time + peer_processing_budget + ack_frame_time + margin`。**100ms 不是全局常量**（138B 最大帧在 9600 波特下仅发送约 144ms）。

### UART1 有界接收服务（防通信洪泛饿死控制）
```c
#define UART1_RX_BYTES_PER_SERVICE   ...
#define UART1_RX_SERVICE_PERIOD_MS   1
size_t uart1_transport_service(uint32_t now_ms, size_t max_bytes);
```
规则：每次最多处理 `UART1_RX_BYTES_PER_SERVICE` 字节，到上限立即返回；RX ring 仍有数据下轮继续；核心 P3 只执行 frame parser；E1 启用后协议接收与 STOP/DISARM 事件必须先于速度 PI 任务处理（优先级 0）。

### 环形缓冲（SPSC，发布顺序写死）
- 生产/消费对：UART0 TX 主循环产/ISR 销；UART1 RX ISR 产/主循环销；UART1 TX 主循环产/ISR 销。禁 ISR 写日志。
- **容量模型**：数组 512，`head==tail` 空，保留一空槽表示满，实际可用 511；容量 2 的幂，索引 `&(capacity-1)`；`_Static_assert((RING_CAPACITY & (RING_CAPACITY-1))==0)`。
- **发布顺序**：生产者先写 buffer 最后更新 head；消费者先读 head 再读 buffer 处理完最后更新 tail。`head/tail` 用自然对齐 `volatile uint32_t`；生产者只写 head、消费者只写 tail；禁两主循环调用者并发写同一 TX ring；禁 ISR 与主循环同为同一 ring 生产者；ring 通用模块不自行开关全局中断。
- **溢出策略**：RX 满→丢新字节+`rx_overflow_count++`；TX 空间不足→`enqueue=false` 不部分写帧不阻塞；日志行放不下→整行丢弃+`debug_drop_lines++`。UART1 帧发送**全帧入队或完全不入队**。
- **RX 重同步（临界区）**：①临时关 UART1 RX 中断 ②清硬件 RX FIFO/读到空 ③重置 ring head/tail ④重置 frame parser ⑤清 `rx_stream_corrupted` ⑥`link_resync_count++` ⑦重开 RX 中断。**禁止在 ISR 仍可写 ring 时直接改两索引**；临界区只用于重同步。

### frame_codec（纯算法，时间参数化）
```c
frame_parser_init(...)
frame_parser_feed(parser, byte, now_ms)
frame_parser_poll_timeout(parser, now_ms)
frame_status_t frame_encode(const frame_t*, uint8_t *out, size_t cap, size_t *len);
```
- 错误枚举：`FRAME_COMPLETE / FRAME_INCOMPLETE / FRAME_ERR_LENGTH / FRAME_ERR_CRC / FRAME_ERR_RESERVED_FLAGS / FRAME_ERR_VERSION / FRAME_ERR_TIMEOUT / FRAME_ERR_TOTAL_TIMEOUT / FRAME_ERR_OUTPUT_CAPACITY / FRAME_ERR_INVALID_ARG`。
- **两个独立超时**：`FRAME_INTER_BYTE_TIMEOUT_MS` 与 `FRAME_TOTAL_TIMEOUT_MS`，任一满足即复位 parser（防噪声低速持续喂字节占住半帧）。
- **VERSION 错误**：VERSION≠0x01 → 不解释后续载荷、重置帧、核心只返回错误。
- `frame_encode` 顺序检查：frame/output/len 非空 → payload≤128 → 总长无溢出 → cap≥required → 全过才写。**禁序列化 packed 结构体**；多字节字段用显式 little-endian 读写。
- **SOF 重叠恢复**：解析失败/复位时若当前字节等于 SOF1，保留为新帧首字节。
- 核心阶段只返回错误，不发 ACK/NACK（会话层在 E1）。

### 帧格式 v1 + 已验证 CRC
SOF1/SOF2=`0xA5 0x5A`；VERSION=0x01；TYPE；FLAGS（bit0=ACK_REQUIRED bit1=RESPONSE bit2=ERROR bit3-7 保留须 0）；SEQ；LENGTH(2LE)；PAYLOAD 0~128 字节；CRC16(2LE)。CRC-16/CCITT-FALSE：poly 0x1021、init 0xFFFF、不反射、xorout 0x0000，**范围 = VERSION 至 PAYLOAD 的全部字节**。最大帧 138B；静态缓冲禁 malloc。
**CRC 测试向量（实测通过）**：输入 `01 10 00 01 00 00` → `0x78DA` → 线上小端 `DA 78`。

### 日志行长度上限
`DEBUG_LOG_LINE_MAX`（project_config.h）：固定大小缓冲区有界格式化，超长整行丢弃（禁半条误导日志）；不得超过 UART0 ring 可用容量；ISR 禁格式化日志；禁无界 `sprintf`。

### 基础验收（节流修正）
- **Host**：10000 帧编码→解析，不丢/越界/死循环。单测须含：SOF 重叠、g_tick_ms 回绕附近半帧超时、总帧超时、最大/空载荷、保留 FLAGS 非零、VERSION 错误、固定随机种子、输出缓冲不足。
- **板端 UART1 回环**：持续 60s 或 ≥1000 小帧；整帧入队成功才递增序号；TX 队列满等下一轮主循环**不算丢帧**；`rx_overflow_count==0`；序号连续；负载不得长期超串口容量 70~80%。
- 必须含：UART0 日志与 UART1 协议流不混。

## 八、编码器

- 术语：`encoder_a_cycles_per_motor_rev / encoder_decode_multiplier(X1=1/X2=2/X4=4) / motor_revolutions_per_output_revolution / counts_per_output_rev_nominal / counts_per_output_rev_measured`；`counts_per_output_rev_nominal = encoder_a_cycles_per_motor_rev × decode_multiplier × motor_revolutions_per_output_revolution`。**不用含义不明的 gear_ratio**。
- 默认 `encoder_gpio` 软件 X1（A 相上升沿中断+B 相判向）；默认不软件去抖（先查供电/共地/上拉，实测毛刺才加可配置最小间隔）。单调模计数 `position_mod`。
- **P5 人工标定**：输出轴标记→正反向各转 10 圈→`C_fwd/C_rev`→`C_mean=(C_fwd+C_rev)/2`；通过条件 `abs(C_fwd-C_rev) ≤ max(2×decode_multiplier, 0.5%×C_mean)`；`counts_per_output_rev_measured = C_mean/10`。减速箱无法安全反拖→低速电机驱动+轮上标记+低占空比悬空，禁强转输出轴。
- **自适应测速（有符号累计，不累绝对值）**：
```c
int32_t count_delta_accum; uint32_t window_start_ms;
/* 每20ms */ count_delta_accum += current_count - previous_count;
/* 触发 */ abs(count_delta_accum) >= 4 或 elapsed_ms >= 100
```
速度用 `count_delta_accum`（不用绝对值之和，防微动正反摆动高估）；`rpm = count_delta_accum×60000/(counts_per_output_rev_measured×Δt_ms)`；方向反转可立即结束窗口重开；更新后归零重设起点；`elapsed_ms==0` 禁除法。保存 `speed_rpm / speed_last_update_ms / speed_valid`；`now-speed_last_update>150ms → speed_valid=false`。PI 非零目标遇 speed_valid=false：输出 0→STOP→ENCODER_STALE；目标为零允许更新为 0，不得把"无脉冲"当故障。
- **堵转检测（独立固定窗口，与测速解耦）**：维护 `stall_window_start_ms / stall_count_start / stall_grace_active`；**只在 actuator_guard 该通道==ACTIVE 时检测**，ENABLING/REVERSING/DISABLED 或目标/方向明显变化时重置；电机刚启用先等 `STALL_STARTUP_GRACE_MS`；判据 `abs(target_rpm)≥STALL_TARGET_RPM ∧ abs(command)≥STALL_COMMAND ∧ abs(position_now-stall_count_start) ≤ STALL_MAX_COUNT ∧ 窗口≥STALL_TIMEOUT_MS`。不复用测速可变窗口 delta。
- **门禁**：基础必做＝理论最大边沿频率 `f_edge=C_rev×n_max/60` 已算、最高实际转速计数**无明显**漏计且多次一致、无中断风暴/主循环卡死；有仪器选做＝精确漏计率、ISR CPU 占用。
- **X1 验收**：正反向符号、计数一致性、最高转速无明显漏计、停时无虚假计数。不做完整非法跳变检测。

## 九、速度 PI

- 符号解耦：`motor_duty_t u16 0~10000` / `motor_command_t i16 -10000~10000`；换向由 actuator_guard 统一执行；**PI 接收归一化轮速符号，不知电机安装方向**。
- **位置式 + 单位写死**：
```c
pi_init(kp, ki_per_second, dt_seconds, out_min, out_max, int_min, int_max);
integral += ki_per_second * dt_seconds * error;   /* 禁止调用方预先乘采样周期 */
```
抗饱和（输出达限且误差同向推动时不更新积分）、限幅/复位/过零/未使能清积分。**dt 用名义 0.020s**。初版 float。
- **NaN/Inf 固定策略**：`pi_init` 任一参数非有限、dt≤0、上下限倒置→`PI_ERR_INVALID_CONFIG`；`pi_update` target/measurement 非有限→输出 0、清积分、`PI_ERR_NONFINITE_INPUT`；内部状态非有限→输出 0、清状态、`PI_ERR_INTERNAL_STATE`。应用层映射 STOP。
- **float→int16 转换**：`motor_command_t control_output_to_motor_command(float output)`——查 finite→限幅 `[-10000.0f,10000.0f]`→明确四舍五入→转；禁直接 C 强转无说明截断。
- **换向 deadtime 内**：暂停 PI 并保持积分清零（选定方案）。
- **三级验收**：最低通过（门槛）＝正反向稳定闭环/无持续振荡/停止归零/积分清/换向无冲击/无持续满占空比失控/无异常复位；性能目标＝稳态误差≤10%、超调≤25%、峰峰值≤15%；增强目标＝5%/20%/10%。基准用实际安全上限。一次反向测试；CSV+电源电压记录。
- **Host 测试**：正负误差积分、上下限、条件抗饱和、reset、dt≤0 拒绝、NaN/Inf 各分支。

## 十、舵机

- `drivers/servo_pwm` 提供 `init/enable/disable/set_pulse_us/set_target_us/service`；只负责脉宽限幅、变化率限制、输出使能。HOLD/PARK 在应用层。
- **set_pulse_us vs set_target_us**：`set_pulse_us` 仅供校准/测试，立即更新下一 PWM 周期脉宽、仍受绝对安全限幅；`set_target_us` 正常运行接口，只更新目标值，由 `service()` 按 `max_slew_us_per_s` 渐进接近。
- 语义补充：未设置有效目标时 `servo_enable()` 返回错误；`disable→enable` 不得自动恢复陈旧目标（除非调用方重新确认）；`service()` 用**实际时间间隔**算最大变化量，不假定正好 20ms；校准模式与正常模式由测试 APP 显式区分。
- **共享 TIMA1**：定时器模块整体拥有，启动后保持 50Hz；每通道独立 `enabled`；禁用单通道只让该通道进无效电平，**不停共享定时器**；两通道都禁用才允许停。`service()` 每 20ms 最多更新一次脉宽。
- **禁用实现（P7 必核）**：核对 TIMA1 PWM 极性、compare=0 真实输出；优先用 DriverLib 通道输出禁用机制；若用 compare=0 禁用必须上板验证确实为低；**禁靠停整个定时器禁用单通道**。实现方法记录进 RESOURCE_MAP/验证报告。
- **初始探测脉宽**：`servo_probe_pulse_us`（project_config.h），状态 `DATASHEET/USER_CONFIRMED/MEASURED`。只有拆舵盘/断连杆后才允许用常见默认 1500μs；用户有说明书值优先用说明书值。
- 安全校准：拆舵盘/断连杆→首输 probe 脉宽若堵转/异常**立即断电**→50μs 步进扩展（每步 ≤1s 观察）→实测限位标 MEASURED→写 hardware_config.h。未校准前只允许单一人工确认脉宽，禁自动扫动。驱动初始化后不自动输出；仅测试 APP 用户确认后显式 enable。
- **P7 host 转换测试**：`pulse_us → timer_compare_count`，覆盖最小/中心/最大脉宽、溢出、舍入、不同预分频。

## 十一、I2C / OLED / MPU6050

- **核心固定同步阻塞轮询 I2C**：
```c
i2c_status_t i2c_write_blocking(..., uint32_t timeout_ms);
i2c_status_t i2c_read_blocking(..., uint32_t timeout_ms);
i2c_status_t i2c_write_read_blocking(..., uint32_t timeout_ms);
```
只允许主循环调用、ISR 禁调、总线一次一个事务、无需 I2C IRQHandler。
- **阻塞硬上限 + 恢复语义（v7.2 修正）**：`I2C_TRANSACTION_TIMEOUT_MS < 控制任务迟到阈值`；单次事务超时→**立即返回错误并置 `recovery_pending`**（**不在同一次 `_blocking()` 内**做控制器复位/重初始化/重试事务）；控制相关模式先按故障分级安全停止；控制器恢复由**后续低优先级 service 或 SAFE 状态**完成，恢复时间**不计入原事务 API**；`I2C_CONTROLLER_RECOVERY_ATTEMPTS_PER_FAILURE=1`；恢复中每个等待有独立上限；OLED 失败直接跳过当前分块不在本周期重试；MPU6050 失败返回错误由下次 10ms 任务再读。P8 记录实测最坏事务时间。
- 错误枚举：`I2C_OK / I2C_ERR_TIMEOUT / I2C_ERR_NACK / I2C_ERR_BUS_STUCK / I2C_ERR_INVALID_ARG / I2C_ERR_BUSY`。
- 只探测 7 位地址 SSD1306=0x3C/0x3D、MPU6050=0x68/0x69；全程 7 位记法。
- **I2C 电平检查（P8 进入条件）**：查明模块供电与 SDA/SCL 上拉所接电源；未接 MSPM0 时测 SDA/SCL 空闲电压须在 3.3V 逻辑范围；不确认优先 3.3V 供电；**禁仅因标注"支持 5V"假定信号 3.3V**。HARDWARE_PROFILE 增 `i2c_module_supply_V / i2c_sda_idle_V / i2c_scl_idle_V / i2c_pullup_rail`。
- `i2c_bus.c`：保留本地 SDK 2.10 i2c controller 例子的 **I2C_ERR_13 workaround**（start 后 delay_cycles 再轮 BUSY），记录所参考示例路径与 SDK 版本；每事务显式 STOP。GPIO 9 脉冲 SCL 恢复属可选增强，未验证前不启用。
- **总线调度优先级**：MPU6050 10ms 采样优先；OLED 仅在其后执行；剩余时间不足跳过本次分块，`missed_count++` 不补跑。
- **OLED 分块**：维护 `current_page / current_column / dirty_pages / refresh_in_progress`，不从页头重发；100k 每次 ≤16B、400k 每次 ≤32B（**指图像数据字节，不含 I2C 地址与 SSD1306 control byte**）；400k 不稳降回 100k 并降刷新率/运行时禁用。
- **不热拔**：设备缺失测试="断电→断开→重新上电→验证有限超时返回"，除非明确支持热插拔。
- **MPU6050**：100Hz、gyro ±500dps、accel ±4g。**校准=500 个成功读取且时间戳不同样本**，不复用旧样本凑数；WHO_AM_I 校验；**核心版人工校准流程**：UART 提示保持静止→等 `IMU_STARTUP_SETTLE_MS`→收集 500 样本→用户可明确命令取消重启；**核心版不自行猜测"用户是否移动"**（自动静止检测=后续增强）；任一 I2C 错误校准失败/重来；完成后用加速度计初始化 roll/pitch。
- **轴映射（机器可校验，强制右手系）**：hardware_config.h 用有符号枚举 `AXIS_POS_X / AXIS_NEG_X / AXIS_POS_Y / AXIS_NEG_Y / AXIS_POS_Z / AXIS_NEG_Z`；配置 `IMU_BODY_X_FROM_SENSOR / IMU_BODY_Y_FROM_SENSOR / IMU_BODY_Z_FROM_SENSOR`；**编译期/初始化检查**：三源轴不重复、覆盖 X/Y/Z、有符号排列矩阵**行列式必须为 +1**；**行列式为 -1 视为非法配置，初始化失败**，不允许"记录为左手系后继续运行"。
- **yaw 连续角**：`yaw_rel_deg` 连续不归一化累计；需要 `[-180°,180°)` 用单独 getter；核心累计不回绕。
- **yaw_rel_valid**：`calibration_complete ∧ last_read_ok ∧ (now-last_sample_ms) ≤ IMU_STALE_MS`（`IMU_STALE_MS=30`，project_config.h）。`yaw_reset()` 只重定义当前角为 0，不改变有效性。
- **姿态用实际采样间隔**：保存 `imu_last_sample_ms`，每次成功读取后 `dt=(now-imu_last_sample_ms)/1000.0f`；dt 超范围→本次不积分 yaw、valid=false、记录异常、不用过大 dt 补算。
- 互补滤波：`ATTITUDE_COMPLEMENTARY_TAU_S`（project_config.h），`α=τ/(τ+Δt)`；算法内无魔数；host 用不同 dt 验证 α。
- **P9 基础必做方向测试**：静止数据均有限；前倾 pitch 符号符合定义；左倾 roll 符号符合定义；俯视逆时针 gyro_z 为正；模拟数据超时后 `yaw_rel_valid=false`。数值精度阈值为有仪器/参考角度时的选做项。
- 坐标系入 HARDWARE_PROFILE（BODY_X 前/Y 左/Z 上，正 yaw 俯视逆时针）+ 轴映射。

## 十二、核心实施阶段（P0~P10）

**阶段结果（v7.2 增补）**：`NOT_STARTED`＝未开始；`IN_PROGRESS`＝进行中；`COMPLETED`＝软件+上板验收完成；`SOFTWARE_READY`＝源码+host+构建完成等待硬件（可进不依赖该硬件输出的软件工作，不得进依赖其真实输出的闭环阶段）；`BLOCKED`＝缺硬件/参数/引脚/工具，只阻塞直接依赖阶段；`FAILED`＝现有条件下未通过，先修复。**当前如实状态：P0=COMPLETED（探针证据+治理闭环，2026-08-04）、P1A=COMPLETED（全资源预检+DRAFT，2026-08-29）、P1=IN_PROGRESS（80MHz 正式基线已收口，系统复位×3 通过，**待用户 POR 冷启动×3 验收后 COMPLETED**）、P2/P3=NOT_STARTED（`ring_buffer`/`frame_codec` 仅为预研资产，release_gate=NOT_MET，见 `docs/STATUS.md`）。**

| 阶段 | 内容 | 关键验收（基础必做 / 有仪器选做） |
|---|---|---|
| P0 | git 骨架+分支规范；AGENTS.md/CLAUDE.md；env 分离；TOOLCHAIN_LOCK（脱敏）+Host 规范；仅需板/芯片/CCS/SDK/调试器信息 | 分支规则就绪；探测探针存档；host 工具链确认 |
| P1 | CCS 最小工程；单 main.c；**80MHz 正式基线**（HFXT 40MHz+SYSPLL）；PB22 LED+UART0 启动日志；安全启动序 | 冷启动×3、电机舵机无输出、warning 0、烧录退出码 0 |
| P1A | 预检配置分离（pin_preflight.syscfg 非构建）；全资源共存检查→RESOURCE_MAP/PINMAP DRAFT 行 | 实例可共存、无 error、warning 0 或书面豁免；部分失败只阻塞依赖阶段 |
| P2 | 1ms 时间基准（集中式 ISR 只 tick）+调度器（优先级/注册接口/next_due/快照；1ms 合成任务仅测试） | 合成任务 1/10/20ms 跑 10min 误差 ≤1 周期、missed 0、tick 单调、同点到期顺序正确；回绕 host 测试 |
| P3 | 核心通信：ring_buffer+uart1_transport（有界 service）+frame_codec+host 单测 | Host 10000 帧无丢/越界/死循环；板端回环 60s 或 1000 帧 rx_overflow=0 序号连续；UART0/1 不混流；有界 service 不饿死控制 |
| P4 | TB6612 开环；STBY 硬件门禁；PWM 禁自动启动核对；bringup 分级；极性 | 上电/复位不转、duty=0 无输出、motor_emergency_stop 同步生效（STBY 先低）、无换向反冲；悬空+限流首测 |
| P5 | 双编码器 X1+人工标定+有符号累计测速+堵转独立窗口 | 正反向符号对、10 圈标定差 ≤max(2×multiplier,0.5%×C_mean)、最高转速无明显漏计、停时无虚假计数；精确漏计率选做 |
| P6 | 速度 PI（软件先行 P6-SOFTWARE：PI/适配层/host 测试，接口冻结即可；P6-BOARD 在 P4+P5 COMPLETED 后闭环） | 最低通过为门槛；一次反向测试；CSV+电源电压；纯软件阶段标 SOFTWARE_READY |
| P7 | 双舵机（共享定时器单通道禁用+实现核对；set_pulse/set_target；probe 校准；host 转换测试） | 周期 20ms、单通道禁用不影响另一通道、无上电跳变、越界限幅；脉宽误差 ≤5μs 选做 |
| P8 | I2C 电平检查→同步阻塞 I2C(100k→400k)+OLED 分块；阻塞硬上限；不热拔 | 地址实测一致、断电断设备重上电有限超时返回、OLED 10min 无锁死；**超期检查**：P6 已完成则真实 20ms 任务（电机禁用）missed=0、CONTROL_LATE 连续 0；未完则 20ms 高优先级合成任务；记录 I2C 单次最坏阻塞时间 |
| P9 | MPU6050 校准（500 有效样本）+基础姿态（实际 dt、连续 yaw、**右手系轴映射**、τ 配置） | 方向/符号/stale 基础必做；roll/pitch ≤3°、gyro_z 均值 ≤0.5°/s 选做；yaw_rel 漂移记录不称绝对 |
| P10 | **核心集成冒烟**：集成 APP 同编全部核心模块；电机/舵机默认禁用；UART0 周期状态；**UART1 按当前链路类型验收**；OLED 低频；MPU 100Hz；**使用最终发布配置（如 Release）烧录并跑全部 10min**；检查 ISR 唯一性；记 Flash/RAM+构建配置/编译器/优化等级/.out SHA-256 | 验收按任务等级分层（见下）；集成启动执行器禁用；发布配置下通过 |

**P10 验收分层（不能统一要求所有任务 missed==0）**：
```text
安全与控制任务(5/20ms)：missed_count==0、连续 CONTROL_LATE==0
MPU6050(10ms)：无连续数据过期，允许记录偶发读错误但不得长期失效
UART1：rx_overflow_count==0；TX 队列背压次数可记录，不直接失败
遥测/OLED(100/200ms)：允许 missed_count>0，但不得阻塞控制、不得致程序失效
```
**P10 UART1 按链路验收**：`UART1_LOOPBACK`＝板内 TX/RX 短接本地序号回环；`K230_DIRECT`＝K230 运行 echo 验证双向帧；`HC04_BRIDGE`＝两端 HC-04 已确认波特率、另一端 echo、记录实际波特率与丢帧统计。P10 只执行当前构建所选链路对应测试。若外部对端不可用：UART1 保留 P3 BOARD_TESTED 结果，P10 只验证初始化不影响其他模块，集成报告明确记录"外部链路未在本次 P10 重测"，不得描述为本次集成回环通过。

**发布门槛**：每个模块达到 VERIFICATION_MATRIX（见十四）最低状态 + P10 当前 tested_code_commit 为 INTEGRATED + 无未解释构建警告 + Flash/RAM 已记录 + 集成启动执行器禁用，才打 `v1.0-module-library`。部分模块仅 SOFTWARE_READY 时可打 `v0.9-software-ready`。
**资源使用率提示（非失败项）**：P10 记录后，使用率 <80% 正常、80%~90% 记 WARN、>90% 先分析内存再开始大型扩展。

## 十三、可选扩展（不阻塞核心发布）

- **E1 K230 会话层（protocol_session）**：SEQ/ACK/NACK/超时重发/去重/分发。
  - 命令分类：事务命令（ARM/DISARM/**STOP**/CLEAR_FAULT/参数配置）→可 ACK+重发；流式命令（SET_WHEEL_RPM/SET_YAW_RATE）→**不重发，最新覆盖旧命令**；遥测（STATUS/FAULT_REPORT）→不 ACK 不重发。
  - **STOP 语义**：STOP=幂等事务命令（要求 ACK、允许重发、重复无额外副作用、收到立即安全停止）；`SET_WHEEL_RPM(0,0)`=流式命令不需重发；二者都能停车但语义不同。
  - **ACK/NACK FLAGS**：ACK＝RESPONSE=1, ERROR=0, ACK_REQUIRED=0；NACK＝RESPONSE=1, ERROR=1, ACK_REQUIRED=0；**ACK/NACK 自身绝不要求 ACK**。
  - **HEARTBEAT 只证链路；每条运动命令 TTL 独立；HEARTBEAT 不得延长旧运动 TTL；运动 TTL 到期即使 HEARTBEAT 正常也必须停。**
  - 去重键=`TYPE+SEQ+PAYLOAD_CRC`；**去重缓存带过期** `DEDUPE_ENTRY_TTL_MS > 单次事务最大重发总时长`（防 SEQ 循环后把合法新命令误判旧命令）；同键→不重复执行重发原 ACK；同 TYPE/SEQ 不同 CRC→NACK/SEQ_CONFLICT。
  - 重发：初次不计，最多 3 次，仅 ACK_REQUIRED 入队，同时刻仅一个待 ACK 命令。ACK 超时按第七节公式。
  - **ACK/NACK（完整）**：`TYPE_ACK=0x01`、`TYPE_NACK=0x02`；响应 SEQ=原命令 SEQ；`PAYLOAD[0]=原TYPE`、`PAYLOAD[1]=status_code`；LENGTH=2。
  - **状态码（完整）**：`0x00 OK / 0x01 BAD_LENGTH / 0x02 BAD_PAYLOAD / 0x03 UNSUPPORTED_TYPE / 0x04 INVALID_STATE / 0x05 BUSY / 0x06 SEQ_CONFLICT / 0x07 SAFETY_REJECTED`。
  - K230 断电 >500ms 无有效心跳→停机。
- **E2 航向控制与云台随动**：输入=左右轮速差+gyro_z+短期 yaw_rel（先角速度阻尼后相对航向）；模式依赖见第五节。
- **E3 性能/功耗/编译优化**：80MHz 时钟基线已于 P1 收口（2026-08-29）转入正式基线（HFXT 40MHz+SYSPLL，`control.syscfg`），**不再是可选优化**。E3 现指：编译优化等级评估、功耗剖析、控制环采样率提升等后期优化；如启用 FCC 校验等深度时钟验证也在此阶段。
- **E4 WWDT 与长期可靠性**：故障注入（符合第五节分级）+ WWDT 最后启用 + 30min 稳定性 + Flash/RAM 记录 + 版本标签。

## 十四、验证与边界

- **两级验收**：上板步骤标注"基础必做（板/串口/万用表/执行器动作/人工观察）/ 有仪器选做（示波器/逻辑分析仪/限流电源/脉冲源）"；无仪器时增强项不作为失败原因。
- **Host 工具链（固定）**：优先 clang、C11、`-std=c11 -Wall -Wextra -Werror`。`test_host.ps1` 编译**项目真实 C 源码**并执行，任一失败返回非零退出码；**不得用 Python 重写同一算法替代 C 测试**；随机测试固定种子并输出；可选 ASan/UBSan；编译命令与结果写日志。算法模块不得 include MSPM0 SDK 头。
- **错误返回原则**：驱动/中间件用模块专属枚举；`bool` 只用于无需区分原因的成功/失败；驱动返回原始错误、应用层映射 WARN/STOP/LATCHED；驱动不记录全局故障状态；不允许共享含义模糊的通用 ERROR。
- **VERIFICATION_MATRIX.md（各模块发布前最低独立状态）**：
```text
纯算法模块(ring/frame_codec/PI/attitude数学) → HOST_TESTED（被 P10 集成固件包含）
UART/I2C/定时器驱动                          → BOARD_TESTED（P10 再共存验证）
电机/编码器/舵机驱动                          → BOARD_TESTED（P10 启动时默认禁用）
集成应用                                     → INTEGRATED（P10 通过）
```
发布门槛＝各模块达到上表最低状态 + P10 当前 tested_code_commit 为 INTEGRATED。**不统一要求所有模块 BOARD_TESTED。**
- **阶段结果 vs 模块验证状态（分开）**：`phase_result`（NOT_STARTED/IN_PROGRESS/COMPLETED/SOFTWARE_READY/BLOCKED/FAILED）只描述阶段执行结果；`module_verification`（SOURCE_ONLY/HOST_TESTED/BOARD_TESTED/INTEGRATED）只描述模块验证等级；STATUS.md 用两列或两张表，不得混填同字段。
- **STATUS.md**：模块/状态/`tested_code_commit`/`evidence_record_commit`/硬件版本/测试日期/证据/已知限制。测试结果只对记录 commit 有效；新 commit 不删旧结果（历史证据）；Agent 报告"本次改动使哪些旧结果失效"。
- **Agent 与用户验证边界**：Agent 只能声称"静态检查过/host 测试过/CCS 构建过/烧录退出码 0"。**仅用户提供证据**（串口日志/照片/视频摘要/口述/仪器结果）才可记 BOARD_TESTED。Agent 禁止声称"上板验证通过、电机方向正确、舵机无抖动、OLED 正常、姿态达标"除非用户返回结果。本机无原生 C 编译器时禁标 HOST_TESTED。
- **最小 CI 属 P0 门禁（已建 `.github/workflows/p0-gate.yml`）**：manifest 校验 / 文档一致性 / Markdown 本地链接 / host 测试；不在 CI 构建 CCS/SysConfig/烧录/硬件测试。**P3 仅扩展**纯算法测试范围与 UART transport 测试，**不再负责首次建立 CI**。

## 十五、工程、Git、三方许可与恢复

- **目录按阶段创建**，不生成空壳 .c/.h。P1 建 app/board/基础配置；P3 建 UART/ring/frame；P4 建 motor；P9 建 attitude。
- **模块标准 MODULE_STANDARD.md**：公共头不暴露 SysConfig 宏；算法模块不 include MSPM0 SDK 头；默认禁 malloc/free；阻塞接口带 `_blocking` 后缀+有限超时；无后缀不得等待；驱动不直接调 fault_manager/printf（返回错误由应用层映射）；单位入接口名或类型；ISR 可调用接口标 `isr_safe`；init 注明是否可重复调用；**注明 IIDX 读取是否自带中断确认**。
- **AGENTS.md**（CLAUDE.md 只写"执行前先读 AGENTS.md"）git 规则：开始前 `git status`+`git branch --show-current`；main 上禁直接改先建分支；未授权不 push/merge；禁 force-push main；禁提交本机绝对路径；禁提交 Debug/Release、`*.out/*.obj/*.map`；一个 commit 一个问题；提交前 `git diff --stat`+摘要。**脏工作树保护**：存在非本次 Agent 创建的未提交改动→立即停止写操作并报告，禁自动 reset/checkout/stash/clean/删未跟踪文件（仅用户明确要求或确认属当前任务时例外）。
- **提交策略**：默认 Agent 改完停在未提交状态并给建议 commit message；仅用户明确要求才 commit；push/PR 始终需明确授权。
- **分支工作流**：`main` + `feat/pNN-*`，阶段内多个小 commit，阶段结束 PR 合并 main、用户 GitHub 自审。
- **本机路径分离**：提交 `scripts/env.example.ps1`；`env.local.ps1`（CCS_HOME/MSPM0_SDK_ROOT/DSLITE_PATH/SERIAL_PORT）入 .gitignore；仓库只记版本不记机器路径。
- **TOOLCHAIN_LOCK（脱敏）**：仓库内只留工具名/版本/文件哈希/脱敏后命令/检查日期；**原始命令输出放 `logs/tmp/toolchain/` 并 .gitignore**；更新前移除 `C:\Users\<用户名>`、盘符绝对路径、COM 口、机器名。
- **ERRATA_CHECKLIST（按使用筛选，无"批准人"）**：状态 `NOT_RELEVANT/HANDLED_BY_SDK/APPLICATION_WORKAROUND_REQUIRED/VERIFIED`；只查当前阶段用到的功能；SDK 已处理不重复实现；优先对照本地 SDK 2.10 例子。编译+链接 warning 0、SysConfig error 0、warning 0 或书面豁免（禁"已知可忽略"）。v1 不进 STOP/STANDBY/SHUTDOWN 低功耗模式。
- **THIRD_PARTY_NOTICES.md + 许可证保留**：外部实现（SSD1306/MPU6050/CRC）记录来源仓库或官方示例、版本/commit、许可证、修改内容；**原代码要求保留的版权头不得删除**；许可证要求附带全文时放 `licenses/`；从 TI SDK 改写的记录具体示例路径与 SDK 版本；**禁把轻微改名描述成"完全自行实现"规避记录**。优先序：本地 TI SDK 2.10 官方示例→数据手册→有许可证三方→自行实现。
- **BLOCKED/FAILED 恢复记录**：`docs/verification/blocked/Pxx-<topic>.md`，字段＝`base_commit`、`working_tree_dirty`、`diff_summary`、`tested_code_commit`（**没有则写 N/A，禁止为填字段自动提交**）、失败步骤、执行命令、退出码、错误摘要、已修改文件、是否回滚、缺少的硬件或参数、用户需执行动作、恢复时第一条命令。

## 十六、单次任务范围

一次任务只允许：一个实施阶段 / 一个独立模块 / 一个明确 bug / 一组紧密相关测试 / 一项文档同步。开始前输出：分支、阶段、目标、不处理内容、拟改文件、所需硬件参数、可自动测试、需用户上板测试。完成后输出：实际改文件、执行命令、构建结果、host 结果、未验证内容、用户下一步上板操作、建议 commit。

## 十七、模块解耦架构（P2 起强制，2026-08-29 增补）

**目标**：模块高解耦、可按需移植（换 MCU 时主要替换 BSP 层）；**不过度抽象**——禁止动态多态、malloc、复杂函数指针注册体系、空壳接口、未使用的抽象层；优先简单、静态、明确的 C 接口。

**依赖方向（只能向下，禁止反向）**：
```text
app → control / estimation / middleware / device drivers → BSP/platform → MSPM0 DriverLib + SysConfig
```

**平台相关代码隔离**：只有 BSP/platform 层允许 include MSPM0 SDK/DriverLib 头、`ti_msp_dl_config.h`、使用 SysConfig 生成宏、知道 TIMG12/TIMA0/UART0/UART1/I2C0/PAxx 等具体实例、操作寄存器/中断号/GPIO。上层模块禁止出现这些内容。

**公共接口语义化**：上层只见 `timebase_now_ms() / motor_set_duty() / encoder_get_count() / uart_write() / i2c_transfer_blocking()` 等语义接口；不见 `TIMG12 / GPIOA / DEBUG_UART_INST / DL_Timer_* / SysConfig pin 宏`。所有接口明确单位（`_ms/_us/_hz/_rpm/_permille`）。

**scheduler 纯软件可移植（P2）**：不 include MSPM0 SDK、不访问 TIMG12、不自定义硬件 ISR、不依赖 `CPUCLK_FREQ`；只接收逻辑时间 `uint32_t now_ms`。TIMG12 只是 MSPM0 BSP 提供 now_ms 的一种实现。

**TIMG12 中断边界**：只有集中式 `bsp/interrupts.c` 定义真实 ISR（读/确认硬件事件→调用 timebase 极小 ISR-side hook→只维护 tick 状态）；ISR 不跑 scheduler task、不打印、不阻塞、不执行 PI/OLED/协议解析。scheduler 在主循环执行。

**算法模块保持 host 可编译**：scheduler、ring_buffer、frame_codec、PI、姿态估计、状态机、actuator_guard 纯策略部分——一律不得 include MSPM0 SDK。

**时钟基线约束**：80MHz 是 `control.syscfg` 的正式配置；`CPUCLK_FREQ`/PLL/SYSOSC 具体值只允许出现在 BSP/platform、SysConfig 生成接口或启动诊断。**业务模块不得假定或硬编码 80000000**，只用毫秒/Hz/秒等语义化时间接口。

**配置分层不变**：`control.syscfg`（MCU 外设/引脚/时钟）/ `hardware_config.h`（物理参数）/ `project_config.h`（软件策略）/ `app_config.h`（应用选择）；不把 MSPM0 外设实例塞进通用算法配置。

## 需要你配合

- 插上开发板；P1 首次 CCS GUI 建工程需你点一次向导。
- 硬件参数按阶段给：P4 前电机/TB6612（含 STBY 下拉确认）、P5 前编码器（含 10 圈标定）、P6 前可控转速范围、P7 前舵机（含 probe 脉宽来源）、P8/P9 前 OLED/MPU6050 供电与地址、**UART1 实际链路类型（HC-04 波特率）**。
- 上板验证按"十四"提供证据才可标 BOARD_TESTED。
