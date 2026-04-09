# Claude 全攻略：從 Opus 到 Claude Code 完整指南

> 建立日期: 2026-03-27
> 來源: BlockBeats/動區
> 原文: https://www.blocktempo.com/claude-opus-sonnet-haiku-code-skills-agents-complete-usage-guide/

---

## 📋 目錄

- [模型分層](#模型分層)
- [四種使用模式](#四種使用模式)
- [Cowork 桌面端 Agent](#cowork-桌面端-agent)
- [Claude Code 開發者平台](#claude-code-開發者平台)
- [Skills 與 Agents](#skills-與-agents)
- [MCP 協議](#mcp-協議)
- [Hooks 生命週期鉤子](#hooks-生命週期鉤子)

---

## 模型分層

| 模型 | 上下文 | 價格 (輸入/輸出) | 適用場景 |
|------|--------|------------------|----------|
| **Opus 4.6** | 100萬 token | $5 / $25 | 複雜大規模分析、程式碼重構、深度研究 |
| **Sonnet 4.6** | 100萬 token | $3 / $15 | 日常工作、Agent 工作流 (推薦首選) |
| **Haiku** | 100萬 token | $0.025 / $0.15 | 高併發、子代理任務 |

> 注意: 3月13日起，超過200K token不再收取溢價

---

## 四種使用模式

| 模式 | 說明 |
|------|------|
| **Chat** | 瀏覽器/行動端對話，提問/頭腦風暴 |
| **Cowork** | 桌面端 Agent，讀寫檔案、執行任務 |
| **Code** | 開發者模式，終端執行 |
| **Projects** | 持久化工作空間 |

---

## Cowork 桌面端 Agent

### 核心概念

> 不是聊天介面，而是任務委託
> 你描述「完成結果是什麼」，Claude 自動制定計劃、執行並交付

### Step 1: 搭建工作空間

```
~/Cowork/
├── context/          # 上下文件案
│   ├── about-me.md   # 你是誰
│   ├── brand-voice.md # 表達風格
│   └── working-preferences.md # 執行規範
├── projects/         # 專案資料夾
└── output/           # 輸出目錄
```

### Step 2: 上下文件案體系

**about-me.md** - 界定你的角色與工作重心
**brand-voice.md** - 固化表達風格 (語氣、常用詞、禁用詞)
**working-preferences.md** - 明確執行規範 (確認後再執行、質量標準)

### Step 3: 全域性指令

路徑: Settings > Cowork > Edit Global Instructions

### Step 4: AskUserQuestion

在提示中加入: `Start by using AskUserQuestion`

Claude 會自動生成互動式表單，詢問執行所需資訊

### Connectors (聯結器)

支援 Google Drive、Gmail、Slack、Google Calendar 等

設定路徑: Settings > Connectors

### Scheduled Tasks (定時任務)

在桌面端設定週期任務，Cowork 自動執行

### Dispatch (遠端派任)

Pro/Max 使用者可在手機端遠端控制桌面任務

---

## Claude Code 開發者平台

### 安裝

```bash
npm install -g @anthropic-ai/claude-code
cd your-project
claude
```

### CLAUDE.md 分層機制

| 層級 | 位置 | 範圍 |
|------|------|------|
| Managed Policy | IT 部署 | 全公司 |
| ~/.claude/CLAUDE.md | 全域性 | 個人偏好 |
| ./CLAUDE.md | 專案級 | 團隊共享 |
| CLAUDE.local.md | 本地覆蓋 | 個人調整 |

### Rules Directory

將規則拆分到 `.claude/rules/` 目錄

```yaml
# 僅匹配測試檔案
---
paths:
  - "**/*test*.py"
---
```

### Commands (斜槓命令)

`.claude/commands/` 目錄下的 Markdown 檔案對應 slash 命令

```bash
/project:review     # 執行 code review
/project:fix-issue 234  # 帶參數
```

---

## Skills 與 Agents

### Skills (自動觸發技能)

Skills 會自動識別場景並執行

```yaml
# SKILL.md
---
name: code-review
trigger: code review requested
effort: high
---
```

### Agents (子代理角色)

```yaml
# agents/security-auditor.md
---
name: Security Auditor
description: Perform security audit on code
tools: [Read, Grep, Glob]  # 僅讀取，無寫入
model: haiku
---
```

### Agent Teams (多代理協作)

設定: `CLAUDE_CODE_EXPERIMENTAL_AGENT_TEAMS=1`

支援最多10個成員並行執行

---

## MCP 協議

### 概念

MCP = AI 時代的 USB-C 介面

- MCP Server: 提供資料與能力
- Claude: 作為客戶端接入

### 配置位置

- 專案級: `.mcp.json`
- 個人級: `~/.claude.json`

### 常用 MCP Servers

- GitHub
- Slack
- Notion
- Jira
- Google Drive
- PostgreSQL

---

## Hooks 生命週期鉤子

在特定節點自動觸發 shell 命令

```json
// .claude/settings.json
{
  "hooks": {
    "pre-commit": ["npm run lint"],
    "post-tool-call": ["echo 'Tool executed'"]
  }
}
```

### 典型應用

- 自動執行 lint
- 阻止敏感資訊提交
- 完成後傳送 Slack 通知

---

## 安全能力

Claude Code 可進行安全審計:
- 跨檔案資料流分析
- 誤報率 < 5%
- 支援紅隊二次篩選

---

## 參考資源

- [Claude Code Cheat Sheet](https://www.blocktempo.com/claude-code-cheat-sheet-complete-guide-shortcuts-agents-mcp-skills/)
- [ Anthropic 官方](https://www.anthropic.com)
- [Claude Code GitHub](https://github.com/anthropics/claude-code)
