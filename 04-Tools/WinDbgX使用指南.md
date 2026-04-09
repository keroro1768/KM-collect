# WinDbgX (WinDbg Preview) 使用指南

## 基本資訊

- **名稱**: WinDbgX / WinDbg Preview
- **版本**: 1.2601.12001.0
- **類型**: Windows 調試工具 (Modern UI)
- **官方網站**: https://learn.microsoft.com/zh-tw/windows-hardware/drivers/debugger/

---

## 安裝路徑

### 主執行檔位置
```
C:\Users\Keror\AppData\Local\Microsoft\WindowsApps\WinDbgX.exe
```

> ⚠️ 注意：這是一個 junction/連結，真正的 Store 版本位置在：
> `C:\Users\Keror\AppData\Local\Microsoft\WindowsApps\Microsoft.WinDbg_8wekyb3d8bbwe\WinDbgX.exe`

### 啟動方式

#### 1. 直接啟動 (空白調試環境)
```powershell
# 直接開啟 WinDbg
C:\Users\Keror\AppData\Local\Microsoft\WindowsApps\WinDbgX.exe
```

#### 2. 附加到正在運行的程序 (-pn 參數)
```powershell
# 附加到指定程序名稱
C:\Users\Keror\AppData\Local\Microsoft\WindowsApps\WinDbgX.exe -pn WinAgent.exe

# 附加到指定 PID
C:\Users\Keror\AppData\Local\Microsoft\WindowsApps\WinDbgX.exe -p 14704

# 附加到遠端調試服務器
C:\Users\Keror\AppData\Local\Microsoft\WindowsApps\WinDbgX.exe -server npipe:pipe=LocalPipe_Name
```

#### 3. 開啟Dump文件
```powershell
# 開啟記憶傾印檔
C:\Users\Keror\AppData\Local\Microsoft\WindowsApps\WinDbgX.exe C:\path\to\dump.dmp
```

#### 4. 啟動並調試可執行檔
```powershell
# 啟動新程序
C:\Users\Keror\AppData\Local\Microsoft\WindowsApps\WinDbgX.exe -e "C:\path\to\app.exe"
```

---

## 命令列參數 (Command Line Options)

| 參數 | 說明 | 範例 |
|------|------|------|
| `-pn <name>` | 附加到指定名稱的正在運行的程序 | `-pn WinAgent.exe` |
| `-p <pid>` | 附加到指定 PID 的程序 | `-p 14704` |
| `-e <path>` | 啟動並調試執行檔 | `-e "C:\app.exe"` |
| `-c <command>` | 啟動後自動執行命令 | `-c "g"` (自動 continue) |
| `-server` | 作為遠端服務器 | `-server npipe:pipe=MyPipe` |
| `-y <symbols>` | 指定符號路徑 | `-y SRV*C:\Symbols*https://msdl.microsoft.com/download/symbols` |
| `-z <dump>` | 開啟 dump 檔案 | `-z C:\dump.dmp` |

---

## UI 介面介紹

### 功能區 (Ribbon)

#### Home Tab
```
├── Flow Control
│   ├── Break        - 中斷執行
│   ├── Go           - 繼續執行
│   ├── Step Into    - 單步進入
│   ├── Step Over    - 單步略過
│   └── Step Out     - 單步跳出
│
├── Reverse Flow Control (需要 Time Travel)
│   ├── Go Back
│   ├── Step Into Back
│   └── Step Over Back
│
├── End
│   ├── Restart      - 重新啟動
│   ├── Stop Debugging - 停止調試
│   └── Detach       - 脫離程序
│
└── Preferences
    ├── Settings
    ├── Source
    └── Assembly
```

#### View Tab
```
├── Windows (視窗)
│   ├── Command      - 命令視窗
│   ├── Watch        - 監看視窗
│   ├── Locals       - 區域變數
│   ├── Registers   - 暫存器
│   ├── Memory       - 記憶體
│   ├── Stack        - 堆疊
│   ├── Disassembly  - 反組譯
│   ├── Threads      - 執行緒
│   └── Modules      - 模組
│
├── Window Layout
│   └── Reset Windows
│
└── Workspace
    └── Accent color
```

---

## 常用命令

### 基本控制
| 命令 | 說明 |
|------|------|
| `g` | 繼續執行 (Go) |
| `p` | 單步執行 (Step Over) |
| `t` | 單步進入 (Step Into) |
| `pt` | 單步跳出 (Step Out) |
| `Ctrl+Break` | 中斷執行 |

### 記憶體與變數
| 命令 | 說明 |
|------|------|
| `dv` | 顯示區域變數 |
| `dt <var>` | 顯示變數類型 |
| `? <expr>` | 計算表達式 |
| `!address` | 顯示記憶體資訊 |
| `k` | 顯示堆疊追蹤 |

### 程序控制
| 命令 | 說明 |
|------|------|
| `.attach <pid>` | 附加到程序 |
| `.detach` | 脫離程序 |
| `.kill` | 終止程序 |
| `lm` | 列出模組 |
| `!runaway` | 顯示執行緒 CPU 使用時間 |

---

## 使用情境

### 1. 附加到正在運行的程序
```powershell
# 方法一：從命令列附加
WinDbgX.exe -pn WinAgent.exe

# 方法二：從 UI 選擇
# File -> Attach to process -> 選擇程序
```

### 2. 查看執行緒
- 點擊 View -> Threads
- 或在 Command 視窗輸入 `~`

### 3. 查看記憶體
- 點擊 View -> Memory
- 或輸入 `!address -summary`

### 4. Time Travel Debugging
- 支援記錄和回放程式執行
- 需要使用 `Launch executable (advanced)` 啟動

---

## 與經典 WinDbg 的差異

| 功能 | WinDbg (經典) | WinDbgX (Modern) |
|------|---------------|------------------|
| UI | 傳統選單 | Ribbon 功能區 |
| 安裝 | WDK 的一部分 | Microsoft Store |
| Time Travel | 需另外安裝 | 內建支援 |
| 遠端調試 | 命令列 | 改良的遠端支援 |

---

## 相關資源

- [Microsoft 官方文檔](https://learn.microsoft.com/zh-tw/windows-hardware/drivers/debugger/)
- [WinDbg Preview 文件](https://docs.microsoft.com/en-us/windows-hardware/drivers/debugger/)
- [Time Travel Debugging 教學](https://learn.microsoft.com/zh-tw/windows-hardware/drivers/debugger/time-travel-debugging-overview)

---

## 更新日誌

- 2026-03-25: 新增 WinDbgX 基本使用指南與命令列參數
- 2026-03-25: 記錄附加到 WinAgent (PID 14704) 的測試結果