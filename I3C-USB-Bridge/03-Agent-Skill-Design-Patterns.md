# Google 5 大 Agent Skill 設計模式

*來源：數位時代 / Google Cloud Tech*
*日期：2026-03-22*

---

## 概述

Agent Skill 是一個自成一體的功能單位，透過 SKILL.md 檔案決定：
- 如何觸發
- 該讀取哪些指令
- 該參考哪些範本

---

## 1. Tool Wrapper（工具包裝）

把特定服務或框架的使用規則封裝成獨立技能。

### 適用情境
- 第三方 API（Stripe、Slack、Gmail）
- 框架慣例（FastAPI、React）

### 運作方式
- 明確寫出「什麼時候啟動這個 Skill」
- 從 references/ 載入內部規範
- 將操作說明從 system prompt 拆分出來

### 範例結構
```yaml
---
name: api-expert
description: FastAPI development best practices...
metadata:
  pattern: tool-wrapper
  domain: fastapi
---
```

---

## 2. Generator（生成器）

用模板固定輸出結構，確保格式一致。

### 適用情境
- 技術報告生成
- API 文件撰寫
- SQL 語句生成
- 專案樣板、程式碼骨架

### 運作方式
1. 載入風格指南（references/style-guide.md）
2. 載入輸出模板（assets/output-template.md）
3. 詢問缺少的關鍵變數
4. 照模板填完每個欄位

---

## 3. Reviewer（審查者）

把「要檢查什麼」和「怎麼檢查」分開。

### 適用情境
- 程式碼審查
- 內容審查
- 事實與引文核實
- SEO 結構檢查

### 運作方式
- 檢查標準放在 references/checklist.md
- SKILL.md 負責定義審查流程
- 依嚴重程度分級（Critical / Major / Minor）
- 給出具體修正建議

---

## 4. Inversion（反轉收集）

由 agent 主導對話，先收集所有必要條件。

### 適用情境
- 專案規劃
- 系統設計
- 訂單/表單流程

### 運作方式
- 設定「閘門規則」：資訊不全禁止開始
- 分階段提問（需求探索 → 技術限制 → 非功能性需求）
- 一次只問一題，等回答後才進入下一題

### 常見組合
`Inversion → Generator`：先收集資訊，再用模板生成成果

---

## 5. Pipeline（流水線）

處理「不能跳步驟」的複雜任務。

### 適用情境
- CI/CD 部署
- 多階段构建流程
- 複雜的自動化任務

### 運作方式
- 把工作拆成一系列明確步驟
- 每步設計「硬門檻」（hard gate）
- 上一階段沒完成，不能進入下一階段

---

## SKILL.md 基本結構

```yaml
---
name: <技能名稱>
description: <說明何時使用>
metadata:
  pattern: <tool-wrapper|generator|reviewer|inversion|pipeline>
  domain: <領域>
---

# 詳細指令
```

### 可選資料夾
- `references/` - 規範、風格指南、檢查清單
- `assets/` - 輸出模板
- `scripts/` - 輔助腳本

---

## 現有 Skills 對應

| 模式 | 現有 Skills |
|------|-----------|
| Tool Wrapper | github, gmail, notion-api, slack |
| Generator | academic-deep-research, web-scraper |
| Reviewer | self-improvement |
| Inversion | find-skills |
| Pipeline | windows-form-layout |

---

*Tags: #agent #skill #design-pattern #google #prompt-engineering*
