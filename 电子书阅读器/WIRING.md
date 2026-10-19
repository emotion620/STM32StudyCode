# STM32 电子书阅读器 —— 接线与使用说明

把一部 txt 小说存进 **W25Q64（8MB SPI Flash）**，用 **0.96 寸 SSD1306 OLED** 一屏 5 行、每行 21 个字符地读。
工程在 `电子书阅读器\project.uvprojx`（Keil MDK，STM32F103C8，标准外设库）。

> 关于《Wonder》：这是一本在版权保护期内的书，我不能把正文写进代码或打包给你。
> 程序本身支持任意 txt —— 你自己有合法电子版的话，导出成 txt 后用 `tools\upload_book.py` 一键写进 flash 就能读。
> 仓库里附了一段我自己写的测试文本 `tools\sample.txt`，可以先把整条链路跑通。

---

## 最短路径（三步）

1. 按下面的接线表插好线，Keil 打开 `project.uvprojx`，F7 编译、F8 下载；
2. OLED 显示 `NO BOOK` 时，插上 USB-TTL，运行：
   `python upload_book.py tools\sample.txt --port COM5`
   看到 `完成！` 就说明整条链路通了；
3. 按 PB1/PB11 翻页。确认没问题后，把 `sample.txt` 换成你自己的小说 txt 再跑一次。

---

## 一、硬件清单

| 数量 | 器件 | 说明 |
|---|---|---|
| 1 | STM32F103C8 最小系统板 | 就是你现在用的那块（蓝板/江科大板都行） |
| 1 | W25Q64 模块 | 8MB SPI Flash，模块上一般标 `W25Q64` |
| 1 | 0.96 寸 OLED | SSD1306，I2C 四针（VCC/GND/SCL/SDA） |
| 2 | 轻触按键 | 接 GND 即可，用内部上拉 |
| 1 | USB-TTL 模块 | CH340 / CP2102，**电平选 3.3V** |
| 1 | ST-Link | 用来烧程序（和你平时一样） |
| 若干 | 杜邦线、面包板 | |

---

## 二、接线表（照着插就行）

### OLED（I2C，软件模拟）

| OLED | STM32 |
|---|---|
| VCC | 3.3V |
| GND | GND |
| SCL | **PB8** |
| SDA | **PB9** |

### W25Q64（SPI，软件模拟）

| W25Q64 | STM32 | 备注 |
|---|---|---|
| VCC | 3.3V | **不要接 5V** |
| GND | GND | |
| CS | **PA4** | 片选 |
| CLK | **PA5** | 时钟 |
| DO / MISO | **PA6** | 模块上可能标 DO 或 SO |
| DI / MOSI | **PA7** | 模块上可能标 DI 或 SI |
| WP | 3.3V | 写保护，**必须拉高**，悬空会写不进去 |
| HOLD | 3.3V | **必须拉高**，悬空会读不出数据 |

> 有些模块把 WP/HOLD 已经在板上拉高了，那就只接 6 根线。接之前看一眼模块丝印。

### 按键（低电平有效，内部上拉）

| 按键 | 接法 |
|---|---|
| 下一页 | 一端接 **PB1**，另一端接 **GND** |
| 上一页 | 一端接 **PB11**，另一端接 **GND** |

### 串口（下载书用，读完可以拔掉）

| USB-TTL | STM32 |
|---|---|
| TXD | **PA10**（STM32 的 RX） |
| RXD | **PA9**（STM32 的 TX） |
| GND | GND（**必须共地**） |
| 3V3 | 不接（板子用 ST-Link 或 USB 供电） |

### 供电

- ST-Link 的 3.3V 给最小系统板供电即可；
- OLED 和 W25Q64 都从板子的 3.3V 取电；
- USB-TTL 只接 TX/RX/GND，**不要**把它的 5V 接到板子上。

---

## 三、面包板怎么摆（示意）

```
        +3.3V ────────────────────────────────────────┐
          │            │            │                  │
     ┌────┴────┐  ┌────┴────┐  ┌────┴────┐             │
     │  OLED   │  │ W25Q64  │  │  按键   │             │
     │ VCC GND │  │ VCC GND │  │  1   2  │             │
     │ SCL SDA │  │ CS CLK  │  └──┬───┬──┘             │
     └──┬───┬──┘  │ DO  DI  │     │   │                │
        │   │     │ WP HOLD │     │   │                │
        │   │     └──┬─┬─┬──┘     │   │                │
        │   │        │ │ │        │   │                │
   ┌────┴───┴────────┴─┴─┴────────┴───┴────────────┐   │
   │  PB8 PB9   PA4 PA5 PA6 PA7   PB1  PB11        │   │
   │                                               │   │
   │            STM32F103C8 最小系统板              │   │
   │                                               │   │
   │  PA9  PA10          3.3V   GND                │   │
   └───┬────┬──────────────┬──────┬────────────────┘   │
       │    │              └──────┴────────────────────┘
       │    │
   ┌───┴────┴───┐
   │ TXD  RXD   │
   │  USB-TTL   │
   └────────────┘
```

面包板上的实际顺序建议：**左边 OLED，中间 W25Q64，右边两个按键**，串口线最后插。
W25Q64 的 CLK 线尽量短一点，杜邦线超过 20cm 时软件 SPI 偶尔会读错，插紧就行。

---

## 四、烧程序

1. 用 Keil 打开 `电子书阅读器\project.uvprojx`；
2. `Options for Target → Debug` 选你的 ST-Link，勾上 `Reset and Run`；
3. F7 编译（已经验证过：0 error 0 warning），F8 下载；
4. 下载完 OLED 应显示 `NO BOOK IN FLASH / SEND VIA UART`。

> 工程里的源文件编码是 UTF-8。如果 Keil 里中文注释是乱码：
> `Edit → Configuration → Editor → Encoding` 选 `UTF-8`。

---

## 五、把书写进 W25Q64

### 1) 准备 txt

- 从你的电子版导出 txt（UTF-8 或 GBK 都可以，脚本两种都能读）；
- 不用手动排版，脚本会自动按 21 字符换行、5 行分页，把智能引号、破折号、重音字母转成 ASCII；
- 一段超过 21 字符的单词会被硬切开，书名超过 24 字符会被截断。

### 2) 关掉串口助手，运行脚本

先确认串口号（设备管理器 → 端口），然后：

```bat
cd /d E:\STM32_GitCode\电子书阅读器\tools
python upload_book.py Wonder.txt --port COM5 --title Wonder
```

脚本会：

1. 打印总页数和占用空间；
2. 发 `BEGIN`，板子擦除对应扇区（大约 5~15 秒，OLED 显示 `ERASING FLASH 37%`）；
3. 逐页发送，OLED 显示 `RECEIVING 1234/3810`；
4. 发 `END` 写书头，再发 `CRC` 做整本校验；
5. 显示 `完成！` 后，板子自动回到第一页。

写一本 40 万字符的小说大约需要 **1~3 分钟**（115200 波特率）。

### 3) 只想先看看会分多少页

```bat
python upload_book.py Wonder.txt --dry-run
python upload_book.py Wonder.txt --show 0
```

`--show 0` 会把第 0 页按屏幕的样子打印出来。`--bin x.bin` 可以只生成镜像文件不连板子。

---

## 六、怎么读

| 操作 | 效果 |
|---|---|
| 按一下 PB1 | 下一页 |
| 按一下 PB11 | 上一页 |
| 按住不放 | 0.5 秒后每 0.1 秒翻一页，快速跳页 |
| 断电再上电 | 自动回到上次看到的那一页（每 2 秒静置后自动记录进度） |

屏幕最下面那条白色进度条是整本书的阅读位置。

---

## 七、串口命令（可选，调试用）

串口助手 115200 8N1、发 `\n` 结尾：

| 发送 | 返回 | 作用 |
|---|---|---|
| `INFO` | `I 3810 21 5 Wonder` | 查询页数/版面/书名 |
| `PAGE` | `P 123 3810` | 当前页码 |
| `GOTO 1200` | `OK` | 直接跳到第 1200 页 |
| `NEXT` / `PREV` | `OK` | 翻页 |

---

## 八、出问题先看这里

| 现象 | 先查什么 |
|---|---|
| OLED 全黑 | PB8/PB9 是否接反（SCL/SDA）；模块有没有 4.7k 上拉（多数自带）；3.3V 有没有电 |
| OLED 花屏/闪 | 杜邦线太长；换短一点的线，或在 `OLED.c` 里把 `OLED_I2C_DELAY()` 的 20 调大 |
| `NO BOOK` 一直不变 | 说明没写进去：先确认 W25Q64 的 WP、HOLD 是否接了 3.3V |
| 写书时卡在 `ERASING` | 串口 TX/RX 是否交叉接（TXD→PA10，RXD→PA9）；GND 是否共地 |
| 校验 `CRC` 失败 | 一般是 SPI 线太长或接触不良，重插后重跑一次脚本 |
| 字太小看不清 | 这是 6x11 字体、5 行 21 列的密度换来的；想更大可以改成 8x16 字体 4 行，见下 |
| 翻页很慢 | 正常，整屏 1024 字节走软件 I2C 大约 60ms；换成硬件 I2C 可再快几倍 |

### 想改版面

- 每屏字符数：`User\Book.h` 里的 `BOOK_COLS` / `BOOK_LINES`，同时要改 `tools\upload_book.py` 顶部的 `COLS` / `LINES`（两边必须一致，否则书头校验不过）；
- 字体大小：`Hardware\OLED.c` 里的 `OLED_ShowChar` 用的是 `OLED_Font6x11`，换成 8x16 字库需要同时改 `OLED_CHAR_W/H` 和 `DrawPage` 的行距。

---

## 九、文件结构

```
电子书阅读器\
├─ project.uvprojx            Keil 工程
├─ Hardware\                  OLED / W25Q64 / 软件 SPI / 按键 / 串口
├─ System\Delay.c             SysTick 延时
├─ User\main.c                阅读器主逻辑 + 串口协议
├─ User\Book.c                flash 分区、书头、进度、CRC
├─ tools\upload_book.py       PC 端分页 + 串口写入
├─ tools\gen_font.py          6x11 字库生成脚本（改字体时用）
└─ tools\sample.txt           测试用短文本
```

flash 布局：

```
0x000000  书头（magic "WBK1"、版面参数、页数、书名）
0x001000  正文，每页固定 128 字节（105 个字符 + 补零）
0x7FF000  阅读进度（断电续读）
```
