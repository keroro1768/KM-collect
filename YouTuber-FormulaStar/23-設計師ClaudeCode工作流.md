# 設計師的 Claude Code 工作流：18 條設計技巧

*來源：https://www.anduril.tw/schoger-claude-code-design/*
*作者：Steve Schoger（Tailwind Labs 設計師）*
*日期：2026-03-23*

---

## Steve Schoger 是誰

- Tailwind Labs 設計師，與 Tailwind CSS 創辦人 Adam Wathan 合作
- 正在開發 **UI.SH** - 給 AI coding agent 用的 skills 套件
- 技術棧：Vite + Tailwind CSS + React
- **不使用 Figma**，只用 Claude Code 從零建出專業網頁

---

## 核心工作流程

```
左邊：瀏覽器（localhost 即時預覽）
右邊：Claude Code 終端
中間：無 Figma
```

- 開 Figma 唯一理由：做向量圖形（SVG logo）
- 沒用任何 skills 或 CLAUDE.md
- 從空白畫面開始，直接用自然語言告訴 Claude 要什麼效果

---

## 18 條設計技巧完整拆解

### 1️⃣ 邊框與陰影：用 outer ring 取代 solid border

- 當元素同時有陰影和實色邊框，會產生「muddy」（混濁）效果
- **做法**：完全不用 border，改用 outer ring
- 顏色：`gray-950、opacity 10%`
- 套用：截圖容器、按鈕、navbar、feature card

### 2️⃣ Concentric radius（同心圓角）

- 圓角容器裡放另一個圓角元素時
- 內層 border-radius = 外層半徑 - padding
- 形成同心圓，視覺更和諧

### 3️⃣ Inset ring 做邊緣定義

- 淡色背景容器用 inset ring（5% opacity）
- 取代傳統 border，更微妙

### 4️⃣ Typography：Inter variable 的 display 版本

- 從 rsms.me 下載 Inter variable 版本
- 可用「中間字重」（如 550）
- 關閉 OpenType 特性 ss02（帶尾巴的小寫 L）

### 5️⃣ 大字體要收緊 tracking

- 24px 以上大字體
- 用 `tracking-tight` 或更緊
- 讓標題更有衝擊力

### 6️⃣ Eyebrow 文字用 monospace

- 小標籤公式：
  - 字型：Geist Mono
  - 全大寫
  - 加寬字距：`tracking-wider`
  - 小字體：`text-xs`
  - 灰色：`gray-600`

### 7️⃣ text-pretty vs text-balance

| 屬性 | 用途 |
|------|------|
| text-pretty | 避免段末孤字 |
| text-balance | 文字行均勻分布 |

### 8️⃣ 小字體行高可以翻倍

- 14px（text-sm）文字
- 行高可設為 28px（2倍）
- 讓副標題更易讀

### 9️⃣ 左對齊取代置中

- AI 預設置中，要改成左對齊
- 參考 Tailwind Plus 的 split headline 佈局
- 標題放左側（約 3/5），副標放右側（約 2/5）

### 🔟 Inline section heading

- 標題和副標放同一行
- 標題：深色粗體（neutral-950）
- 副標：灰色中等字重（neutral-600, medium）
- 靈感來自 Apple、Linear、Stripe、Vercel

### 1️⃣1️⃣ max-width 用 ch 單位

- 用 `max-w-[40ch]` 取代固定像素
- 好處：不管字體大小，閱讀寬度都維持舒適

### 1️⃣2️⃣ 按鈕的高度和形狀

- 高度：36-38px（用 padding 控制）
- 形狀：pill-shaped（全圓角）
- 字體：14px（text-sm）
- 拿掉預設的 icon

### 1️⃣3️⃣ 兩個按鈕高度差 2px 的解法

- 有 ring 的按鈕會比沒有的高 2px
- **Adam Wathan 的解法**：用 span 包裹有 border 的按鈕
  - 設定 `inline-flex + p-px`
  - 用 calc 計算讓兩個按鈕等高

### 1️⃣4️⃣ Well-styled container（凹陷容器）

- 極淡背景：`gray-950 at 2.5%~5% opacity`
- 移除邊框
- 截圖底部零 padding、無底部圓角
- 截圖「坐在容器底部」的效果

### 1️⃣5️⃣ 截圖是最好的視覺元素

- 用 3x 解析度擷取截圖
- 確保高 DPI 螢幕清晰

### 1️⃣6️⃣ Canvas grid（裝飾性邊框網格）

- section 間加裝飾性線條
- 水平線：viewport 全寬
- 垂直線：page container 寬度內
- 靈感來自 Stripe、Vercel、Tailwind 官網

### 1️⃣7️⃣ 背景圖片 testimonial card

- 用 AI 生成的人像照片當卡片背景
- 底部加暗色漸層
- 白色引言文字清晰可讀

### 1️⃣8️⃣ Logo cloud 處理

- 不需要標題
- 用真實 SVG logo
- 移除 opacity，直接用 gray-950

---

## Claude Code 的 prompt 策略

### 設計師的對話方式

- **不寫程式碼**，不指定 CSS 屬性
- 用**設計語言**說話

### 有效的 prompt 模式

| 模式 | 範例 |
|------|------|
| 問「這個是怎麼做的？」 | 檢查 Claude 實作方式 |
| 全站同步 | 「把這個樣式套用到下面所有區塊」 |
| 建立臨時工具 | 拖曳定位工具，調好後移除 |

### Claude 會自動學習風格一致性

- 新增的按鈕自動套用之前定義的 ring 樣式
- 不需要額外指示

---

## UI.SH

- 正在開發的 AI coding agent skills 套件
- 目標：把設計原則預先打包成 agent 可引用的設計標準
- 支援：Claude Code、Cursor、Amp

---

## 心得

> AI 生成的初版網頁和專業設計師的成品之間，差距不在程式碼，在**設計判斷**。

- AI 會用 indigo 當預設色
- 會把所有東西置中
- 會用 solid border 配 shadow 製造混濁效果

**設計師知道該改成什麼**，現在只需要說出來就好。

---

## 相關資源

| 資源 | 連結 |
|------|------|
| Steve Schoger 原始影片 | https://x.com/steveschoger/status/2033915927608390000 |
| UI.SH | https://ui.sh |
| Inter variable font | https://rsms.me/inter/ |
| Tailwind Plus | https://tailwindcss.com/plus |

---

*Tags: #ai #design #claude-code #tailwind #steve-schoger #ui-design #前端設計*
