# 任務：建立編譯環境 (T21)

> 建立日期: 2026-03-27
> 優先級: P1

---

## 📋 任務目標

建立 STM32H563ZI 開發板的命令列編譯環境，評估可用工具。

---

## 🔧 研究結果

### 1. CLI 編譯工具

| 工具 | 可用 | 說明 |
|------|------|------|
| **STM32CubeCLT** | ✅ | ST 官方 CLI 工具組 |
| **make (直接)** | ✅ | 在 CubeIDE 生成的 Debug 目錄 |
| **STM32CubeIDE headless** | ⚠️ | 可用但較慢 |

**STM32CubeCLT 包含：**
- `arm-none-abi-gcc` (編譯器)
- `arm-none-abi-g++`
- `arm-none-abi-nm`
- `STM32CubeProgrammer_CLI` (燒錄)
- Debug 工具

**下載連結：** https://www.st.com/en/development-tools/stm32cubeclt.html

### 2. ICE / Debug Probe

| 探針 | 支援 | 說明 |
|------|------|------|
| **STLINK-V3EC** | ✅ | 內建於 NUCLEO-H563ZI |
| **J-Link** | ✅ | 第三方 |
| **CMSIS-DAP** | ✅ | 開源方案 |

### 3. OpenOCD

| 狀態 | 說明 |
|------|------|
| ⚠️ 可用 | STM32H563ZI 已支援，但有穩定性問題 |
| 建議 | 使用 ST 官方的 STM32CubeProgrammer 更穩定 |

**參考配置：**
```
source [find interface/stlink-dap.cfg]
transport select "dapdirect_swd"
set CHIPNAME STM32H563ZITx
```

---

## 📝 待辦事項

- [ ] 下載並安裝 STM32CubeCLT
- [ ] 驗證編譯指令 (arm-none-abi-gcc)
- [ ] 測試燒錄 (STM32CubeProgrammer_CLI)
- [ ] 評估 OpenOCD 穩定性

---

## 💡 推薦流程

```bash
# 1. 安裝 STM32CubeCLT 後驗證
arm-none-eabi-gcc --version

# 2. 編譯專案
make -C Debug all

# 3. 燒錄
STM32CubeProgrammer_CLI -c port=SWD -w Debug/*.hex -rst
```

---

## 🔗 參考文件

- UM3088 - STM32Cube Command-line Toolset Quick Start Guide
- ST Community: Nucleo STM32H5 debugging OpenOCD
