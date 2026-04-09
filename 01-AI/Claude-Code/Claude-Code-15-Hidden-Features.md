---
created: 2026-04-09
tags: [Claude-Code, AI-Coding, Productivity, Tips, Automation]
aliases: [Claude-Code-15-Hidden-Features, Boris-Cherny-Tips]
---

# Claude Code 隱藏功能公開！Boris Cherny 曝 15 項技巧

> 來源：[15 Hidden & Under-Utilized Features in Claude Code — From Boris Cherny](https://github.com/shanraisshan/claude-code-best-practice/blob/main/tips/claude-boris-15-tips-30-mar-26.md)
> 作者：Boris Cherny (@bcherny)，Claude Code 創辦人
> 日期：2026年3月30日

---

## 📱 1/ Claude Code 有手機 App

Boris 經常使用 iOS App 寫程式——不需開筆電就能修改程式碼。

- 下載 Claude App（iOS/Android）
- 切換到 **Code** 標籤
- 可直接在手機上審查變更、批准 PR、寫程式

---

## 🔄 2/ 跨裝置移動 Session

使用 `/teleport` 或 `claude --teleport` 將雲端 Session 移到本地繼續。
或使用 `/remote-control` 從手機控制本地運行的 Session。

- **Teleport**：將雲端 Session 拉回本地終端機
- **Remote Control**：從任何裝置控制本地 Session
- Boris 在 `/config` 設定了「啟用所有 Session 的遠端控制」

---

## ⏰ 3/ /loop 和 /schedule — 最強大功能

排程 Claude 自動執行，可設定最長一週的間隔。

Boris 本地運行的 loops：
- `/loop 5m /babysit` — 自動處理 code review、auto-rebase、推進 PR
- `/loop 30m /slack-feedback` — 每 30 分鐘自動將 Slack 回饋轉為 PR
- `/loop /post-merge-sweeper` — 自動處理遺漏的 code review 評論
- `/loop 1h /pr-pruner` — 關閉過期或不必要的 PR

💡 實驗將工作流轉化為 skills + loops，效果強大！

---

## 🪝 4/ 使用 Hooks 執行確定性邏輯

在 agent 生命週期中運行邏輯：

- **SessionStart**：每次啟動 Claude 時動態載入 context
- **PreToolUse**：記錄模型執行的每個 bash 命令
- **PermissionRequest**：將權限提示轉到 WhatsApp 審批
- **Stop**：每次停止時提醒 Claude 繼續工作

---

## 📲 5/ Cowork Dispatch

Boris 每天使用 Dispatch 處理 Slack、郵件、管理檔案。

- Dispatch 是 Claude Desktop App 的安全遠端控制
- 可使用你的 MCPs、瀏覽器和電腦（需授權）
- 可視為從任何地方委託非編碼任務給 Claude

---

## 🌐 6/ 使用 Chrome 擴展功能做前端開發

**最重要技巧：給 Claude 一個驗證输出的方法**

- 就像要求某人建網站但不讓他用瀏覽器——結果可能不理想
- 給 Claude 瀏覽器，它會迭代直到結果完美
- Boris 每次做 web 程式碼都會用到

---

## 🖥️ 7/ 使用 Desktop App 自動啟動和測試 Web 伺服器

Desktop App 內建自動運行 web 伺服器並在內建瀏覽器測試的功能。

- 也可在 CLI 或 VSCode 中透過 Chrome 擴展實現類似功能
- 或直接使用 Desktop App 獲得整合體驗

---

## 🌿 8/ Fork 你的 Session

兩種方式：
1. 從 session 執行 `/branch`
2. 從 CLI 執行 `claude --resume <session-id> --fork-session`

`/branch` 建立分支對話——你現在在分支中。要恢復原本，使用 `claude -r <original-session-id>`

---

## 💬 9/ 使用 /btw 處理側邊問題

Boris 經常用這個在 agent 工作時快速回答問題。

範例：
```
/btw how do I spell dachshund?
> dachshund — German for "badger dog" (dachs + badger, hund + dog).
```

---

## 🌳 10/ 使用 Git Worktrees

Claude Code 深度支援 git worktrees。Boris **同時運行數十個 Claude**，就是這樣做到的。

- 使用 `claude -w` 在新 worktree 啟動新 session
- 或在 Claude Desktop App 中勾選 **worktree** 核取方塊
- 非 git VCS 使用者可使用 `WorktreeCreate` hook 新增邏輯

---

## 📦 11/ 使用 /batch 進行大規模更改

`/batch` 會訪談你，然後將工作分散到多個 **worktree agents**（數十、數百甚至數千個）完成。

- 可用於大型程式碼遷移和其他可並行的任務
- 每個 worktree agent 獨立處理自己的程式碼副本

---

## ⚡ 12/ 使用 --bare 加速 SDK 啟動高達 10 倍

預設執行 `claude -p`（或 TypeScript/Python SDK）時，Claude 會搜尋本地 CLAUDE.md、settings 和 MCPs。但對於非互動式使用，通常想透過 `--system-prompt`、`--mcp-config`、`--settings` 等明確指定。

- 這是 SDK 設計初期的疏忽
- 未來版本會將預設改為 `--bare`
- 現在可用此 flag 獲得高達 **10 倍** 的啟動加速

```bash
claude -p "summarize this codebase" \
    --output-format=stream-json \
    --verbose \
    --bare
```

---

## 📁 13/ 使用 --add-dir 讓 Claude 存取更多資料夾

跨多個 repo 工作時，Boris 通常在一個 repo 啟動 Claude，然後用 `--add-dir`（或 `/add-dir`）讓 Claude 看到其他 repo。

- 不僅讓 Claude 知道該 repo，也**給予在該 repo 工作的權限**
- 或在團隊的 `settings.json` 中加入 `"additionalDirectories"` 讓啟動時永遠載入額外資料夾

---

## 🤖 14/ 使用 --agent 給 Claude 自訂 System Prompt 和工具

Custom agents 是常被忽視的強大功能。在 `.claude/agents/` 定義新 agent，然後執行：

```bash
claude --agent=<your agent's name>
```

- Agents 可以有受限的工具、自訂描述和特定模型
- 非常適合建立唯讀 agents、專門審查 agents 或特定領域工具

---

## 🎤 15/ 使用 /voice 啟用語音輸入

有趣的事實：Boris 大部分程式碼是對著 Claude 說，而不是打字。

- 在 CLI 執行 `/voice` 後按住空格鍵說話
- 在 Desktop 按下語音按鈕
- 或在 iOS 設定中啟用聽寫

---

## 🔗 相關連結

- [Boris Cherny (@bcherny) on X — March 30, 2026](https://x.com/bcherny/status/2038454336355999749)
- [How Boris Uses Claude Code](https://howborisusesclaudecode.com/)
- [Claude Code Best Practice Repository](https://github.com/shanraisshan/claude-code-best-practice)

---

## 📎 相關筆記

- [[Claude-Code-Complete-Guide]] — Claude Code 完整指南
- [[agent-teams-management]] — Agent 團隊管理