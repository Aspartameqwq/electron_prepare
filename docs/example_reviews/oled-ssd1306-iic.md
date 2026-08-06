# 例程评审：0.96 寸 IIC 单色屏（SSD1306，P8 参考）

**source_id**：`lckfb-tmx-oled-ssd1306-iic`
**来源**：`examples_and_documents/立创·天猛星MSPM0G3507开发板【模块移植代码】/显示类/0.96寸IIC单色屏/`
**结构**：`BSP/OLED/oled.c + oled.h + oledfont.h + bmp.h`；`empty.c`
**SDK**：mspm0_sdk@2.02.00.05 / SysConfig 1.21.0（旧）
**地址**：0x3C（7 位）

## 传输方式（v7.2 纠偏：不是"一次性大块传输"，是大量微小事务）

**证据（SOURCE_FACT，来自 `oled.c`）**：
- `OLED_WR_Byte(dat, mode)`：构造 2 字节 `Send_Buff`（控制字节 + 1 数据），填 TX FIFO → 轮询 IDLE → `startControllerTransfer(0x3C, TX, 2)`；**每次调用 = 一次 2 字节 I2C 事务**；
- `OLED_Refresh()`：每页 = 3 个寻址命令 + 1 个额外数据字节 + 128 个显存字节（×8 页）→ 全屏约 **1056 次独立的两字节 I2C 发送**。

**结论（CALCULATED）**：全屏刷新是**约 1056 次微小阻塞事务**，不是一个大块事务。
**`IIC_delay()`（SOURCE_FACT）**：函数存在（`delay_us(10)`），但 `OLED_WR_Byte` **并未调用它** → 属**死代码**（且 `#include "delay.h"` 被注释），不得描述为"运行时逐字节延时"。

## 明确代码缺陷（SOURCE_FACT）

1. **`OLED_Refresh()` 中 `OLED_WR_Byte(0x40, OLED_DATA)` 多发送一个数据字节**：`OLED_WR_Byte` 已自动添加 0x40 控制字节，这行等于又发送一个像素数据 `0x40` → 每页可能多写一列。
2. **`OLED_DrawLine()` 变量错误**：`else { incy = -1; delta_y = -delta_x; }` 应对 `delta_y` 取反，当前 `-delta_x` 会破坏部分斜线绘制。
3. **事务顺序需重核**：先 `fillControllerTXFIFO` → 再轮询 IDLE → 再 `startControllerTransfer`；与 DriverLib 状态语义的先后关系需结合生成代码核对（不能仅归因为"缺超时"）。
4. **无边界检查**：`OLED_GRAM[144][8]` 与坐标未统一校验。
5. **地址硬编码 0x3C**：0x3D 需改代码。
6. **无超时轮询**：`while (!(IDLE))` 死等 → 本项目 `i2c_*_blocking` 需有限超时 + `recovery_pending`。
7. **末笔事务未显式等待完成**：`OLED_WR_Byte` 启动 I2C 事务后立即返回，刷新最后一笔是否发完未确认。

## 可取之处（借鉴）

1. **SSD1306 命令/显存布局/字库**（命令表、页列寻址、取模字库、图形接口）——P8 直接参考其协议细节。
2. 7 位地址 0x3C——与本项目"只探测 0x3C/0x3D、全程 7 位记法"一致。

## 借鉴建议（应用到本项目）

- **仅借鉴 SSD1306 命令、显存布局、字库**；**不借鉴其传输实现**。
- 重写为 `drivers/ssd1306`：`init / show 原语 / service（分块刷新状态机，维护 current_page/column/dirty_pages）`。OLED 命令与显示数据为**纯写**，用 `i2c_write_blocking`（超时 + `recovery_pending` + I2C_ERR_13 workaround）；**MPU6050 寄存器读取**才用 `i2c_write_read_blocking`。
- **许可未确认前不得直接复制字库数组**；SSD1306 命令与地址模式按**芯片数据手册**重新实现（非直接照搬例程）。
- bus 优先级：MPU6050 10ms 采样优先，OLED 之后执行；时间不足跳过本次分块。

**TI 文件规则**：`source/ti/driverlib/`＝供应商源码，**不修改**；`ti_msp_dl_config.c/h`＝SysConfig 生成产物，**不手工编辑**（改 `.syscfg` 后重新生成）；`DL_*`＝API 名称，不是文件。`i2c_bus.c` 保留本地 SDK 2.10 i2c controller 例子的 `I2C_ERR_13` workaround（start 后 delay_cycles 再轮 BUSY），禁删除。
