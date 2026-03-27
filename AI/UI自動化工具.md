# UI 自動化工具

## 工具清單

### 1. FlaUI
- **官網**: https://github.com/FlaUI/FlaUI
- **描述**: .NET  library，幫助自動化 UI 測試 Windows 應用程式
- **支援類型**: Win32, WinForms, WPF, Store Apps (UWP)
- **用途**:
  - 讀取 UI 控件（按鈕、輸入框、清單等）
  - 自動點擊按鈕
  - 自動化測試

### 2. FlaUI 2
- **描述**: FlaUI 的更新版本
- **狀態**: 建議使用最新版本

### 3. Accessibility Insights
- **官網**: https://accessibilityinsights.io/
- **描述**: 微軟官方的 UI 檢測工具
- **用途**:
  - 檢測 UI 元素
  - 驗證Accessibility
  - 自動化輔助功能測試

### 4. UI Spy
- **描述**: 舊版但仍可使用的 UI 檢測工具
- **用途**: 檢視 UI 元素階層

---

## 相關文章

- [A Beginner's Guide to Using FlaUI](https://www.thegreenreport.blog/articles/a-beginners-guide-to-using-flaui-for-windows-desktop-app-automation/)
- [FlaUI: UI Testing Framework for Windows Apps](https://www.einfochips.com/blog/getting-started-with-flaui-open-source-ui-automation-for-windows-apps/)
- [Automate Desktop Application with FlaUI](https://blog.nashtechglobal.com/desktop-automation-test-with-flaui/)

---

## WinDbg UI 結構範例

```
WinDbg 1.2601.12001.0
├── Ribbon
│   ├── FileOptions (Menu)
│   ├── Home (TabItem) ◀ 目前選取
│   │   └── Flow Control (Group)
│   │       ├── Break (Button)
│   │       ├── Go (SplitButton)
│   │       ├── Step Out (Button)
│   │       ├── Step Into (Button)
│   │       └── Step Over (Button)
│   ├── View (TabItem) ◀ 旁邊的 Tab
│   ├── Breakpoints (TabItem)
│   ├── Time Travel (TabItem)
│   ├── Model (TabItem)
│   ├── Scripting (TabItem)
│   ├── Source (TabItem)
│   ├── Memory (TabItem)
│   └── Extensions (TabItem)
└── ActionCenter (Pane)
```

---

## 更新日誌

- 2026-03-25: 新增 FlaUI, Accessibility Insights, UI Spy 工具資訊