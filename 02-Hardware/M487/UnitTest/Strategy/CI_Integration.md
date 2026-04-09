# CI 整合指南 (CI Integration Guide)

> 本文件說明如何將單元測試整合到 CI/CD 流程中。

---

## 📑 目錄

1. [CI 策略概述](#ci-策略概述)
2. [GitHub Actions 設定](#github-actions-設定)
3. [本地 CI 腳本](#本地-ci-腳本)
4. [自動化流程](#自動化流程)
5. [品質閘門](#品質閘門)

---

## CI 策略概述

### 測試觸發時機

```
┌─────────────────────────────────────────────────────────┐
│                                                         │
│   Commit ──→ Run Unit Tests ──→ Run Coverage ──→ Pass?  │
│                                                         │
│   PR ──→ Unit Tests + Integration Tests ──→ Code Review │
│                                                         │
│   Release ──→ Full Test Suite + Hardware Tests         │
│                                                         │
└─────────────────────────────────────────────────────────┘
```

### 測試執行時間目標

| 測試類型 | 目標時間 | 最大時間 |
|----------|----------|----------|
| 單元測試 | < 2 分鐘 | 5 分鐘 |
| 整合測試 | < 5 分鐘 | 10 分鐘 |
| 覆蓋率報告 | < 1 分鐘 | 2 分鐘 |
| **總計** | **< 8 分鐘** | **17 分鐘** |

---

## GitHub Actions 設定

### 基本工作流

```yaml
# .github/workflows/unit-test.yml
name: Unit Tests

on:
  push:
    branches: [main, develop, 'feature/*']
  pull_request:
    branches: [main]

jobs:
  # ============================================================
  # Unity/Ceedling Tests (Firmware)
  # ============================================================
  firmware-test:
    runs-on: ubuntu-latest

    steps:
      - name: Checkout code
        uses: actions/checkout@v4

      - name: Setup Ruby
        uses: ruby/setup-ruby@v1
        with:
          ruby-version: '3.2'
          bundler-cache: true

      - name: Install dependencies
        run: |
          gem install ceedling --no-doc
          gem install rspec --no-doc  # For any Ruby tests

      - name: Cache test artifacts
        uses: actions/cache@v3
        with:
          path: |
            vendor/
            ~/.ceedling/
          key: ${{ runner.os }}-ceedling-${{ hashFiles('**/project.yml') }}
          restore-keys: |
            ${{ runner.os }}-ceedling-

      - name: Run Unity/Ceedling tests
        run: |
          ceedling clobber
          ceedling test:all

      - name: Generate coverage report
        run: ceedling coverage

      - name: Upload coverage artifacts
        uses: actions/upload-artifact@v4
        with:
          name: firmware-coverage
          path: build/coverage/**/*

      - name: Publish Test Report
        uses: dorny/test-reporter@v1
        if: always()
        with:
          name: Firmware Tests
          path: build/test/**/*.xml
          reporter: junit

  # ============================================================
  # GoogleTest Tests (Host Tools)
  # ============================================================
  host-tools-test:
    runs-on: ${{ matrix.os }}
    strategy:
      matrix:
        os: [ubuntu-latest, windows-latest]
        include:
          - os: ubuntu-latest
            cmake: 'cmake -B build -S .'
            build: 'cmake --build build'
          - os: windows-latest
            cmake: 'cmake -B build -S . -G "Visual Studio 17 2022"'
            build: 'cmake --build build --config Release'

    steps:
      - name: Checkout code
        uses: actions/checkout@v4

      - name: Setup CMake
        uses: threeal/cmake-actions/setup-cmake@v1
        with:
          cmake-version: '3.28'

      - name: Configure
        run: ${{ matrix.cmake }}

      - name: Build
        run: ${{ matrix.build }}

      - name: Run tests
        run: ctest --output-on-failure --test-dir build

      - name: Upload test results
        if: always()
        uses: actions/upload-artifact@v4
        with:
          name: gtest-results-${{ matrix.os }}
          path: build/test-results/*.xml

  # ============================================================
  # Code Quality Checks
  # ============================================================
  code-quality:
    runs-on: ubuntu-latest

    steps:
      - uses: actions/checkout@v4

      - name: Run cpplint
        run: |
          pip install cpplint
          cpplint --recursive src/ include/ --filter=-build/include_what_you_use

      - name: Run clang-format
        run: |
          pip install clang-format
          find src/ include/ -name '*.c' -o -name '*.h' | xargs clang-format --dry-run --Werror

  # ============================================================
  # Security Scan
  # ============================================================
  security:
    runs-on: ubuntu-latest

    steps:
      - uses: actions/checkout@v4

      - name: Run Bandit (Python security)
        run: |
          pip install bandit
          bandit -r host_tools/ --format json --output bandit.json

      - name: Upload security report
        uses: upload-artifact@v4
        with:
          name: security-report
          path: bandit.json
```

### 進階配置

```yaml
# 帶有覆蓋率閘門的完整配置
name: Full CI Pipeline

on:
  push:
    branches: [main, develop]
  pull_request:
    branches: [main]

env:
  MIN_COVERAGE: 80
  MIN_BRANCH_COVERAGE: 70

jobs:
  test:
    runs-on: ubuntu-latest

    steps:
      # ... setup steps ...

      - name: Run all tests
        run: ceedling test:all

      - name: Generate coverage report
        run: ceedling coverage

      - name: Check coverage threshold
        run: |
          COVERAGE=$(cat build/coverage/coverage_summary.txt | grep "Lines" | awk '{print $2}' | tr -d '%')
          echo "Coverage: $COVERAGE%"
          if (( $(echo "$COVERAGE < $MIN_COVERAGE" | bc -l) )); then
            echo "Coverage $COVERAGE% is below threshold $MIN_COVERAGE%"
            exit 1
          fi

      - name: Publish coverage to Codecov
        uses: codecov/codecov-action@v3
        with:
          files: build/coverage/coverage.xml
          fail_ci_if_error: true
```

---

## 本地 CI 腳本

### 統一測試腳本

```bash
#!/bin/bash
# run_all_tests.sh - 執行所有測試

set -e  # 遇錯即停
set -o pipefail

# 顏色定義
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo "=========================================="
echo "  M487_ScsiTool Full Test Suite"
echo "=========================================="
echo ""

# 計時
START_TIME=$(date +%s)

# 1. 安裝依賴檢查
echo -e "${YELLOW}[1/6] Checking dependencies...${NC}"
command -v ruby >/dev/null 2>&1 || { echo "Ruby not found!"; exit 1; }
command -v gcc >/dev/null 2>&1 || { echo "GCC not found!"; exit 1; }
ceedling version
echo ""

# 2. 清理
echo -e "${YELLOW}[2/6] Cleaning build artifacts...${NC}"
ceedling clobber
echo ""

# 3. 單元測試
echo -e "${YELLOW}[3/6] Running unit tests...${NC}"
ceedling test:all VERBOSE=2
echo ""

# 4. 整合測試
echo -e "${YELLOW}[4/6] Running integration tests...${NC}"
ceedling test:all TEST="integration"
echo ""

# 5. 覆蓋率
echo -e "${YELLOW}[5/6] Generating coverage report...${NC}"
ceedling coverage
echo ""

# 6. 報告生成
echo -e "${YELLOW}[6/6] Generating summary...${NC}"
END_TIME=$(date +%s)
ELAPSED=$((END_TIME - START_TIME))

echo ""
echo "=========================================="
echo -e "${GREEN}✓ All tests completed successfully!${NC}"
echo "=========================================="
echo "  Time: ${ELAPSED} seconds"
echo "  Coverage: build/coverage/index.html"
echo "=========================================="
```

### Windows PowerShell 腳本

```powershell
# run_all_tests.ps1

$ErrorActionPreference = "Stop"

Write-Host "==========================================" -ForegroundColor Cyan
Write-Host "  M487_ScsiTool Test Suite" -ForegroundColor Cyan
Write-Host "==========================================" -ForegroundColor Cyan
Write-Host ""

$StartTime = Get-Date

# Check dependencies
Write-Host "[1/5] Checking dependencies..." -ForegroundColor Yellow
try {
    ceedling version
} catch {
    Write-Host "Ceedling not found! Run: gem install ceedling" -ForegroundColor Red
    exit 1
}

# Clean
Write-Host "[2/5] Cleaning..." -ForegroundColor Yellow
ceedling clobber

# Run tests
Write-Host "[3/5] Running unit tests..." -ForegroundColor Yellow
ceedling test:all VERBOSE=2

# Coverage
Write-Host "[4/5] Generating coverage..." -ForegroundColor Yellow
ceedling coverage

# Summary
$EndTime = Get-Date
$Elapsed = ($EndTime - $StartTime).TotalSeconds

Write-Host ""
Write-Host "==========================================" -ForegroundColor Green
Write-Host "  Tests completed in $($Elapsed.ToString("0.0")) seconds" -ForegroundColor Green
Write-Host "  Coverage: build\coverage\index.html" -ForegroundColor Green
Write-Host "==========================================" -ForegroundColor Green
```

---

## 自動化流程

### Pre-commit Hook

```bash
#!/bin/bash
# .git/hooks/pre-commit

echo "Running pre-commit checks..."

# 1. Run unit tests
ceedling test:all

# 2. Check code formatting
if [ -f .clang-format ]; then
    git diff --cached --name-only | grep -E '\.(c|h)$' | while read file; do
        clang-format --dry-run "$file" 2>&1 | grep -q "would be reformatted" && {
            echo "File $file needs formatting. Run: clang-format -i $file"
            exit 1
        }
    done
fi

echo "Pre-commit checks passed!"
```

### Commitizen 整合

```json
// .czrc
{
  "path": "cz-conventional-commits"
}
```

### 自動版本發布

```yaml
# .github/workflows/release.yml
name: Release

on:
  push:
    tags:
      - 'v*'

jobs:
  release:
    runs-on: ubuntu-latest

    steps:
      - uses: actions/checkout@v4

      - name: Run full test suite
        run: |
          gem install ceedling
          ./run_all_tests.sh

      - name: Build firmware
        run: |
          make clean
          make -j$(nproc)

      - name: Create Release
        uses: softprops/action-gh-release@v1
        with:
          files: |
            build/*.bin
            build/coverage/*.html
```

---

## 品質閘門

### 測試品質閘門

```yaml
# Quality Gates Configuration
quality_gates:
  unit_tests:
    enabled: true
    min_pass_rate: 100%  # All tests must pass
    max_duration: 5m

  coverage:
    enabled: true
    min_line_coverage: 80%
    min_branch_coverage: 70%
    fail_on_decrease: true

  static_analysis:
    enabled: true
    max_warnings: 0
    tools:
      - cpplint
      - clang-tidy

  complexity:
    enabled: true
    max_cyclomatic_complexity: 10
    max_function_length: 200 lines
```

### 閘門失敗處理

```bash
#!/bin/bash
# handle_gate_failure.sh

GATE_NAME=$1
REPORT_FILE=$2

echo "=========================================="
echo "  Quality Gate Failed: $GATE_NAME"
echo "=========================================="
echo ""
echo "Report: $REPORT_FILE"
echo ""
echo "Actions:"
echo "  1. Review the report"
echo "  2. Fix the issues"
echo "  3. Commit and push changes"
echo "  4. CI will re-run automatically"
echo ""
```

---

## 延伸閱讀

- [M487 測試策略](./M487_Testing_Strategy.md)
- [測試覆蓋率矩陣](./Test_Coverage_Matrix.md)
- [Unity 最佳實踐](../Unity/Best_Practices.md)

---
