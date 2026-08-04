# HARDWARE_PROFILE.md — 物理硬件参数档案

**参数来源状态**：`UNKNOWN`(未知) / `DATASHEET`(说明书) / `MEASURED`(实测)。
**规则**：`UNKNOWN` 参数不得以猜测值写入编译配置；对应模块阶段开始前补齐（见 PLAN.md 阶段进入条件）；`MEASURED` 须注明测量方法。

## 一、P0 已确认（基础）

| 参数 | 值 | 状态 | 备注 |
|---|---|---|---|
| 开发板 | LCKFB **天猛星 MSPM0G3507** | MEASURED | 持有 |
| 芯片型号/封装 | MSPM0G3507, LQFP-64(PM) | MEASURED | |
| CCS Theia | 20.5.1.00012 | MEASURED | 见 TOOLCHAIN_LOCK.md |
| MSPM0 SDK | 2.10.00.04 | MEASURED | |
| 调试器/探针 | **XDS110（主，用户持有）**；J-Link（备） | MEASURED | 官方文档确认天猛星**无板载调试器**；待插板 `detect_probe.py` 实测 |
| 板载 LED | PB22 | DATASHEET | 天猛星板文档 |

**调试与烧录方式（官方文档确认，2026-08-04）**：
- 天猛星 **无板载 XDS110 / 无板载调试器**（板载 XDS110-ET 的是 TI 官方 LP-MSPM0G3507 LaunchPad，非天猛星）。
- 板载 **CH340**（USB 转串口，TYPE-C）→ UART0 调试输出 + 串口 BSL 下载（备用，慢）。
- 常规烧录/调试用**外部 SWD 探针**：本项目主用 **XDS110**（DSLite/CCS 支持），J-Link 备用。
- **禁止 ST-LINK**（官方明确：会被锁芯片）。
- SWD 引脚：PA19=SWDIO、PA20=SWCLK；需共 GND 并提供目标供电参考（VTref）。

板上固定占用（见 PINMAP，禁挪用）：PA5/PA6=HFXT、PA19/PA20=SWD、PA10/PA11=UART0/CH340、PB22=LED。
天猛星文档禁用引脚：PA02 / PA18 / PA21 / PA23。

## 二、阶段字段（P0 全部 UNKNOWN，按阶段补齐）

### 电机 / TB6612（P4 前必须）
- `motor_rated_voltage_V` / `motor_no_load_current_A` / `motor_stall_current_A`
- `driver_continuous_current_A` / `driver_peak_current_A` / `motor_supply_current_limit_A`
- `tb6612_module_model` / `tb6612_module_cooling_condition`
- `tb6612_stby_gpio` / `tb6612_stby_external_pulldown` / `tb6612_stby_reset_level_measured`（STBY 必须外部下拉，禁直连 3.3V）

### 编码器（P5 前必须）
- `encoder_a_cycles_per_motor_rev` / `motor_revolutions_per_output_revolution`
- 实测可控最低转速 / 最高转速
- 减速箱能否安全手动反拖（决定 10 圈标定方式）

### 舵机（P7 前必须）
- `servo_probe_pulse_us`（来源 DATASHEET/USER_CONFIRMED/MEASURED）
- `servo_center_us` / `servo_safe_min_us` / `servo_safe_max_us` / `servo_max_slew_us_per_s`（脱连杆实测后标 MEASURED）

### OLED / MPU6050（P8/P9 前必须）
- SSD1306 7 位地址（0x3C/0x3D）、MPU6050 7 位地址（0x68/0x69）
- `i2c_module_supply_V` / `i2c_sda_idle_V` / `i2c_scl_idle_V` / `i2c_pullup_rail`（电平检查）
- IMU 安装轴映射 `IMU_BODY_X/Y/Z_FROM_SENSOR`（强制右手系，行列式=+1）

### UART1 链路（P3 前必须）
- 链路类型：`UART1_LOOPBACK` / `K230_DIRECT` / `HC04_BRIDGE`
- 实际波特率（HC-04 桥接时**必须读取真实波特率，不得默认假定 115200**）

## 三、测量记录（新增 MEASURED 时追加）

| 日期 | 参数 | 值 | 方法 | 对应 commit |
|---|---|---|---|---|
| — | — | — | — | — |
