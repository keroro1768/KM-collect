# AN5879 - Getting Started with I3C (STM32H5)

> 轉換自: [ST Wiki - Getting started with I3C](https://wiki.st.com/stm32mcu/wiki/Getting_started_with_I3C)
> 原始文件: AN5879 Introduction to I3C for STM32H5 series MCU

本文說明如何使用 STM32H5 微控制器設置 I3C controller，並使用 IKS01A3 shield board (LSM6DSO 加速度計/陀螺儀 + LPS22HH 氣壓計) 作為 I3C target。

---

## 1. I3C 簡介

MIPI I3C 是一種串行通訊介面規范，改進了 I2C 的功能、性能和功耗，同時保持向後兼容性。

I3C 匯流排控制器裝置以高達 12.5 MHz 的通信速度驅動 push-pull SCL 線。

I3C 支援多種通信格式，均共享雙線介面。兩條線 designated 為 SDA 和 SCL：

- **SDA (serial data)**: 雙向數據引腳
- **SCL (serial clock)**: 時鐘專用線

### 1.1 I3C 設備類型

- I3C main controller
- I3C secondary controller
- I3C target
- Legacy I2C target

### 1.2 功能特點

- 廣播和直接 common command code (CCC) 訊息
- 動態定址
- 私有讀寫傳輸
- Legacy I2C 訊息支援
- In-band interrupt (IBI)
- Hot-join 請求
- Controller 角色請求

### 1.3 I2C vs I3C 比較

| 特性 | MIPI I3C | I2C |
|------|----------|-----|
| Bus speed | Up to 12.5 MHz | Up to 1 MHz |
| Signal | Open-drain & push-pull | Open-drain |
| Address | Dynamic (7 bits) & static | Static (7 or 10 bits) |
| Interrupt | In-band | External I/O |
| Hot join | Yes | No |
| CCC | Yes | No |
| Clock stretching | No | Yes |
| 9th data bit | Transition bit | ACK/NACK |

---

## 2. 硬體前提

### 所需硬體

- 1x STM32 Nucleo 開發板 (例如 NUCLEO-H563ZI)
- 1x Motion MEMS and environmental sensor expansion board (X-NUCLEO-IKS01A3)

### X-NUCLEO-IKS01A3 特色

- 支援 Arduino® UNO R3 連接器
- LSM6DSO: 3-axis 加速度計 + 3-axis 陀螺儀
- LIS2MDL: 3-axis 磁力計
- LIS2DW12: 3-axis 加速度計
- HTS221: 濕度和溫度感測器
- LPS22HH: 氣壓感測器
- STTS751: 溫度感測器

---

## 3. 設置 I3C Controller

### 3.1 在 STM32CubeIDE 中建立專案

1. File > New > STM32 Project
2. 選擇 NUCLEO-H563ZI 開發板
3. 儲存專案
4. 初始化所有週邊裝置

### 3.2 配置 I3C

1. 在 Connectivity 中選擇 I3C1
2. 選擇 SCL (如 PB6) 和 SDA (如 PB7) 腳位
3. 選擇 Controller 模式
4. 設定 Frequency 為 3000 kHz (或 12.5 MHz)
5. 選擇 I3C pure bus (只有 I3C 裝置)

### 3.3 NVIC 設定

啟用 I3C1 event 和 error interrupt。

### 3.4 時鐘配置

- 系統時鐘: 250 MHz
- 選擇 HSI 作為 PLL1 Source Mux
- 選擇 PLLCLK 作為 System Clock Mux
- 設定 HCLK 為 250 MHz

---

## 4. 軟體設定

### 4.1 動態定址 (使用 ENTDAA CCC) - LSM6DSO

**保持 only JP2 jumpers**

#### 定義 Target Descriptor

在 `main.h` 中插入:
```c
typedef struct {
  char * TARGET_NAME;
  uint32_t TARGET_ID;
  uint64_t TARGET_BCR_DCR_PID;
  uint8_t STATIC_ADDR;
  uint8_t DYNAMIC_ADDR;
} TargetDesc_TypeDef;
```

建立 `target.h`:
```c
#define TARGET1_DYN_ADDR 0x32

TargetDesc_TypeDef TargetDesc1 = {
  "TARGET_ID1",
  DEVICE_ID1,
  0x0000000000000000,
  0x00,
  TARGET1_DYN_ADDR,
};
```

#### Interrupt 模式

在 `main.c` 中:

```c
/* USER CODE BEGIN Includes */
#include "target.h"
/* USER CODE END Includes */

/* USER CODE BEGIN PV */
TargetDesc_TypeDef *aTargetDesc[1] = { &TargetDesc1 };
uint8_t aPayloadBuffer[64];
/* USER CODE END PV */

/* USER CODE BEGIN 2 */
if (HAL_I3C_Ctrl_DynAddrAssign_IT(&hi3c1, I3C_RSTDAA_THEN_ENTDAA) != HAL_OK) {
  Error_Handler();
}
/* USER CODE END 2 */

/* USER CODE BEGIN 4 */
void HAL_I3C_TgtReqDynamicAddrCallback(I3C_HandleTypeDef *hi3c, uint64_t targetPayload) {
  TargetDesc1.TARGET_BCR_DCR_PID = targetPayload;
  HAL_I3C_Ctrl_SetDynAddr(hi3c, TargetDesc1.DYNAMIC_ADDR);
}

void HAL_I3C_CtrlDAACpltCallback(I3C_HandleTypeDef *hi3c) {
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
}
/* USER CODE END 4 */
```

#### Blocking 模式

```c
/* USER CODE BEGIN 2 */
uint64_t TargetPayload;
uint8_t dynamic_adr = 0x32;
HAL_StatusTypeDef status = HAL_OK;

do {
  status = HAL_I3C_Ctrl_DynAddrAssign(&hi3c1, &TargetPayload, I3C_RSTDAA_THEN_ENTDAA, 5000);
  if (status == HAL_BUSY) {
    HAL_I3C_Ctrl_SetDynAddr(&hi3c1, dynamic_adr);
  }
} while (status == HAL_BUSY);
/* USER CODE END 2 */
```

### 4.2 動態定址 (使用 SETDASA CCC) - LPS22HH

**保持 only JP4 jumpers**

在 `main.c` 中:

```c
/* USER CODE BEGIN PTD */
#define Direct_SETDASA 0x87
#define Brodacast_DISEC 0x01
#define Brodacast_RST 0x06
#define LPS22HH_DYNAMIC_ADDR 0x32
/* USER CODE END PTD */

/* USER CODE BEGIN PD */
#define STATIC_ADRESS_LPS22HH 0x5D
/* USER CODE END PD */

/* USER CODE BEGIN PV */
uint8_t aSETDASA_LPS22HH_data[1] = {(LPS22HH_DYNAMIC_ADDR << 1)};
uint8_t aDISEC_data[1] = {0x08};

I3C_XferTypeDef aContextBuffers[2];

I3C_CCCTypeDef aSET_DASA_LPS22HH[] = {
  {0x5D, Direct_SETDASA, {aSETDASA_LPS22HH_data, 1}, HAL_I3C_DIRECTION_WRITE},
};

I3C_CCCTypeDef aSET_CCC_DISEC[] = {
  {0x5D, Brodacast_DISEC, {aDISEC_data, 1}, LL_I3C_DIRECTION_WRITE},
};

I3C_CCCTypeDef aSET_CCC_RST[] = {
  {0x5D, Brodacast_RST, {NULL, 0}, LL_I3C_DIRECTION_WRITE},
};
/* USER CODE END PV */

/* USER CODE BEGIN 2 */
/* 發送 DISEC 來禁用 IBI interrupt */
aContextBuffers[I3C_IDX_FRAME_1].CtrlBuf.pBuffer = aControlBuffer;
aContextBuffers[I3C_IDX_FRAME_1].CtrlBuf.Size = 1;
aContextBuffers[I3C_IDX_FRAME_1].TxBuf.pBuffer = aTxBuffer;
aContextBuffers[I3C_IDX_FRAME_1].TxBuf.Size = 1;

if (HAL_I3C_AddDescToFrame(&hi3c1, aSET_CCC_DISEC, NULL, aContextBuffers, 1, I3C_BROADCAST_WITHOUT_DEFBYTE_RESTART) != HAL_OK) {
  Error_Handler();
}

if (HAL_I3C_Ctrl_TransmitCCC(&hi3c1, aContextBuffers, 1000) != HAL_OK) {
  Error_Handler();
}

/* 發送 RSTDAA 重置之前的動態位址 */
if (HAL_I3C_AddDescToFrame(&hi3c1, aSET_CCC_RST, NULL, aContextBuffers, 1, I3C_BROADCAST_WITHOUT_DEFBYTE_RESTART) != HAL_OK) {
  Error_Handler();
}

if (HAL_I3C_Ctrl_TransmitCCC(&hi3c1, aContextBuffers, 1000) != HAL_OK) {
  Error_Handler();
}

/* 發送 SETDASA 設定 LPS22HH 的動態位址 */
if (HAL_I3C_AddDescToFrame(&hi3c1, aSET_DASA_LPS22HH, NULL, &aContextBuffers[I3C_IDX_FRAME_1], 1, I3C_DIRECT_WITHOUT_DEFBYTE_RESTART) != HAL_OK) {
  Error_Handler();
}

if (HAL_I3C_Ctrl_TransmitCCC_IT(&hi3c1, &aContextBuffers[I3C_IDX_FRAME_1]) != HAL_OK) {
  Error_Handler();
}

/* 驗證 LPS22HH 是否準備好通訊 */
if (HAL_I3C_Ctrl_IsDeviceI3C_Ready(&hi3c1, LPS22HH_DYNAMIC_ADDR, 300, 1000) != HAL_OK) {
  Error_Handler();
}
/* USER CODE END 2 */
```

---

## 5. 編譯、燒錄與執行

1. 點擊 "Ctrl + S" 生成專案
2. 編譯並燒錄到開發板
3. 使用邏輯分析儀或示波器觀察 SDA 和 SCL 訊號

### 結果驗證 (LSM6DSO)

- Device ID register PID: 0x0208006C100B
- Bus characteristics register BCR: 0x07
- Device characteristics register DCR: 0x44

### 結果驗證 (LPS22HH)

- Broadcast DISEC CCC: 0x01 (0x08: 禁用 IBI interrupt)
- Broadcast RSTDAA: 0x06
- Direct SETDASA: 0x87 (dynamic address: 0x32)

---

## 6. 參考資源

| 文件 | 連結 |
|------|------|
| AN5879 - I3C Introduction | [PDF](https://www.st.com/resource/en/application_note/an5879-introduction-to-i3c-for-stm32h5-series-mcu-stmicroelectronics.pdf) |
| RM0492 - STM32H563 Reference Manual | [PDF](https://www.st.com/resource/en/reference_manual/rm0481-stm32h563h573-and-stm32h562-armbased-32bit-mcus-stmicroelectronics.pdf) |
| DS12140 - LSM6DSO Datasheet | [PDF](https://www.st.com/resource/en/datasheet/lsm6dso.pdf) |
| STM32CubeH5 GitHub | [GitHub](https://github.com/STMicroelectronics/STM32CubeH5) |
| Getting started with I3C (Wiki) | [ST Wiki](https://wiki.st.com/stm32mcu/wiki/Getting_started_with_I3C) |
