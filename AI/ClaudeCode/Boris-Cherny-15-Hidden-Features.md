# Claude Code 隱藏功能指南 - Boris Cherny 親推 15 項功能

> 來源：[Inside 硬科技](https://www.inside.com.tw/article/40974-claude-code-boris-cherny-hidden-features-voice-scheduling-workflow-2026)
> 日期：2026-03-31

---

## 摘要

Claude Code 創始人 Boris Cherny 公開他每日使用的 15 個隱藏功能，引發開發者社群熱烈討論（56萬+瀏覽）。

---

## 功能清單

### 1. 跨裝置連續工作
- **行動 App**: iPhone/Android 版 Claude，直接修改程式
- **--teleport**: 將雲端工作階段拉回本機
- **/remote-control**: 從手機遠端操控本機
- **Cowork Dispatch**: 安全遠端控制介面

### 2. 排程與自動化
- **/loop**: 固定間隔自動執行任務（最長一週）
  - `/loop 5m /babysit` - 每5分鐘自動 code review
  - `/loop 30m /slack-feedback` - 每30分鐘整理 Slack 回饋
- **/schedule**: 排程未來任務
- **hooks**: 在特定節點注入邏輯
  - SessionStart: 自動載入專案上下文
  - PreToolUse: 自動記錄操作日誌
  - PermissionRequest: 通知轉發至 WhatsApp
  - Stop: 自動 poke 繼續執行

### 3. 並行與擴展
- **git worktrees**: 原生支援多工作樹
- **/batch**: 大規模任務並行執行（數百/數千代理人）
  - 範例: `/batch migrate from jest to vite`

### 4. 開發工具
- **Chrome 擴充套件 (Beta)**: 直接操控瀏覽器、讀取 console
- **Claude Desktop app**: 自動啟動 web server + 內建瀏覽器測試
- **/branch**: 從同一節點分叉嘗試方向
- **/btw**: 任務執行中插入旁支問答

### 5. SDK 優化
- **--bare**: 跳過載入加速（啟動時間縮短 10 倍）
- **--add-dir**: 加入其他 repo 目錄並獲得操作權限
- **--agent**: 自訂子代理人（定義於 .claude/agents/）

### 6. 語音輸入
- **/voice**: 按住空白鍵啟用語音輸入
- iPhone 系統設定中開啟聽寫也可使用

---

## 關鍵統計

| 指標 | 數據 |
|------|------|
| Anthropic 內部程式碼產出成長 | >200% (2026年) |
| GitHub 提交量佔比 | ~4% (預測年底 20%) |
| 推文瀏覽量 | 56萬+ |

---

## 重要趨勢

Claude Code 正在從「終端機工具」轉型為：
- **多裝置自主代理人平台**
- **AI 驅動的工作作業系統**
- **從「會用」升級到「自動化」**

---

## 相關資源

- [Claude Code 官網](https://code.claude.com)
- [Auto 模式說明](https://www.inside.com.tw/article/40934-claude-code-auto-mode-permission-classifier-research-preview)
- [Claude Dispatch 攻略](https://www.inside.com.tw/article/40891-assign-tasks-to-claude-from-anywhere-in-cowork)
- [Computer Use 功能](https://www.inside.com.tw/article/40981-will-software-learning-barriers-disappear-as-ai-agents-become-standard-and-saas-interface-design-loses-importance)