# HP Stylus UX 技術文件

> 來源：投影片拍攝 | 日期：2026/03/17

---

## 1. Common UX w/ multi vendor/proto eco-sys

### 概述
HP 提供跨供應商的統一使用者體驗（UX），確保各裝置間的一致性。

### 主要內容
- **HP owns UX**：HP 掌控整體使用者體驗設計
- **Provides common UX across all vendors**：為所有供應商提供一致的 UX
- **Vendor library implements device communication**：供應商函式庫負責裝置通訊
- **Composite pattern poses deployment challenges**：複合模式帶來部署挑戰（供應商或 HP 負責）

### 架構圖
- **橘色方塊**：HP Print Service
- **右側灰色方塊**：HP Application
- **藍色小方塊**：Plugin Pattern

---

## 2. Wake on Pen

### 功能說明
HP 應用程式透過客戶端函式庫啟用 Wake on Pen 功能。

### 主要內容
- **Default state**：TBD（待確認）
- **While enabled**：啟用時，觸控筆觸發會產生鍵盤或滑鼠事件

### API
```
GetWakeOnPen(out wake)
  └── 取得目前狀態

EnableWakeOnPen(in wake)
  └── 啟用 WoP；當數位筆處於低功耗狀態時，內部注入事件讓系統喚醒
```

### Event
```
delegate WakeOnPen(out wake)
```

---

## 3. Magnifier（放大鏡）

### 功能說明
在游標下方 150px 半徑區域放大 2 倍。

### 主要內容
- Magnifies 150px radius region under cursor by **2x**
- Inking and clicks **disabled**
- Both centre aligned except at edges
- Magnifier activated through **button click**
- Magnifier de-activated through **same click**

> **Note**：這是 Specialized action，詳見 `GetButtonAction` 和 `SetButtonAction`

---

## 4. Pen Properties - Battery Level

### 功能說明
觸控筆服務觸發事件讓主機更新電池、 docking、充電狀態等資訊。

### 主要內容
- Show toast notification when battery level is too low (**<15%**)
- Display in **%** or range (low, medium, high, etc.)

### 資料結構

#### Enum
```
ProximityType = TouchOrHover, In-BT-Range
```

#### Struct
```
DeviceProperties
  ├── ModelNo
  ├── SerialNo
  ├── FirmwareVer
  ├── BatteryLevel
  ├── Docked
  ├── Proximity
  ├── ProximityType
  └── Charging
```

### API
```
GetDeviceProperty(out DeviceProperties)
```

### Event
```
delegate DevicePropertiesChanged(out DeviceProperties)
```

---

## 📎 原始資料

- **拍攝日期**：2026/03/17
- **拍攝設備**：vivo V50 | ZEISS
- **拍攝參數**：23mm f/1.88 1/100s ISO247