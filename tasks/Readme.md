# tasks/ - 任務追蹤資料夾

## 資料夾結構

```
tasks/
├── Readme.md              ← 資料夾說明如何規範使用
├── TaskList.md            ← 任務清單（總覽）
├── Task_Template.md       ← 任務模板
└── Verify_Template.md     ← 驗證模板
```

## 子任務資料夾

每個任務為一個獨立的資料夾（如 `T001/`），包含：

| 檔案 | 說明 |
|------|------|
| Task.md | 目標、進度、需求 |
| PLAN.md | 執行計畫 |
| WORKLOG.md | 立項即存在（工作日誌） |
| VERIFY.md | 完成後才新增（驗收報告） |

### 子任務範例

若任務 T027 有子任務，需建立子資料夾：

```
T027/
├── Task.md
├── PLAN.md
├── WORKLOG.md
├── VERIFY.md
├── T027a/
├── T027b/
└── T027c/
```

### 研究文件

如任務需要研究文件，可附加在任務資料夾中：

```
T033/
├── Task.md
├── ...
└── T033_GDB_RSP_Research.md
```

---

## 任務狀態

| 狀態 | 說明 |
|------|------|
| open | 待處理 |
| in-progress | 執行中 |
| review | 審查中 |
| done | 已完成 |

## 優先序

- **P0**: 緊急且重要
- **P1**: 重要
- **P2**: 普通
- **P3**: 低優先

---

建立新任務時：
1. 複製 `Task_Template.md` 為 `TXXX/Task.md`
2. 填入 ID、標題、狀態、優先序等資訊
3. 建立 `PLAN.md` 與 `WORKLOG.md`
4. 更新 `TaskList.md` 加入新任務條目