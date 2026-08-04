# 例程评审：0.96 寸 IIC 单色屏（SSD1306，P8 参考）

**来源**：`examples_and_documents/立创·天猛星MSPM0G3507开发板【模块移植代码】/显示类/0.96寸IIC单色屏/`
**结构**：`BSP/OLED/oled.c + oled.h + oledfont.h + bmp.h`；`empty.c`
**接口**：`OLED_Init / OLED_Refresh / OLED_Clear / OLED_ShowPicture / OLED_ShowChinese / OLED_ShowString / OLED_ShowChar / OLED_ShowNum / OLED_ColorTurn / OLED_DisplayTurn / OLED_ScrollDisplay / LOG_D`
**地址**：0x3C（7 位，`DL_I2C_startControllerTransfer(I2C_OLED_INST, 0x3C, ...)`）

## 可取之处（借鉴）

1. **SSD1306 驱动功能完整**：初始化序列、显存缓冲、取模字库（ASCII/中文/图形）、显示图片、滚动等——协议细节（命令字、页/列寻址）可作 P8 直接参考。
2. **硬件 I2C**：用 `DL_I2C_fillControllerTXFIFO + DL_I2C_startControllerTransfer`（控制器发送）——DriverLib 用法可参考。
3. **7 位地址 0x3C**：与本项目"只探测 0x3C/0x3D、全程 7 位记法"一致。
4. **图形接口丰富**：`OLED_ShowPicture / ShowChinese / ShowNum` 等，便于电赛现场显示调试信息。

## 不足之处（弃用）

1. **整帧阻塞刷新**：`OLED_Refresh()` 一次性发送 128×64 缓冲（大块 I2C 事务）→ 本项目要求**分块刷新**（每次 service ≤16B@100k / ≤32B@400k，跨调度周期完成），防阻塞控制循环。
2. **`IIC_delay(delay_us(10))`**：写函数内嵌软件延时（疑似软件 I2C 遗留），增加阻塞 → 本项目硬件 I2C 不应有逐字节 delay。
3. **轮询 IDLE 无超时**：`while (!(DL_I2C_getControllerStatus(...) & IDLE))` 死等 → 本项目 `i2c_*_blocking` 需有限超时 + 一级恢复。
4. **无 I2C_ERR_13 处理**：未在 start 后 delay_cycles 再轮询 BUSY → 违反本项目 i2c_bus 规则（保留本地 SDK 官方例子 workaround）。
5. **全屏重绘**：demo 每帧 `OLED_Clear + 逐项 Show + Refresh`，未利用脏页增量 → 本项目维护 `current_page/column/dirty_pages`，不从页头重发。
6. **地址硬编码 0x3C**：未配置化（0x3D 需改代码）。

## 与 PLAN.md 冲突点（必不照搬）

| 本例子 | 本项目要求 |
|---|---|
| `OLED_Refresh` 整帧阻塞 | 分块刷新（≤16B/≤32B 每 service），MPU 优先总线 |
| `IIC_delay` + 无超时轮询 | 同步阻塞 I2C + 有限超时 + 一级恢复 |
| 无 I2C_ERR_13 workaround | 保留 SDK 官方例子 workaround |
| 地址硬编码 0x3C | `i2c_*_blocking` + 配置化地址 |

## 借鉴建议（应用到本项目）

- **SSD1306 指令/字库/页列寻址**：P8 直接参考其协议细节（命令表、缓冲布局），但重写为 `drivers/ssd1306`：`init / show 原语 / service（分块刷新状态机）`。
- **I2C 传输**：改为本项目 `i2c_write_read_blocking`（同步、超时、I2C_ERR_13），并实现 `current_page/current_column/dirty_pages` 增量刷新。
- **bus 优先级**：MPU6050 10ms 采样优先，OLED 之后执行；时间不足跳过本次分块。

## TI 库铁律

TI DriverLib 与生成文件**绝不允许修改**；`i2c_bus.c` 保留本地 SDK 2.10 i2c controller 例子的 `I2C_ERR_13` workaround（start 后 delay_cycles 再轮 BUSY），禁删除。
