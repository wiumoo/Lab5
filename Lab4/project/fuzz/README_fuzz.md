# 实验5：模糊测试（afl++）运行说明

> 目标：对本项目补充一次可复现的模糊测试流程。
> - 如果能触发崩溃：收集 testcase + 复现崩溃并截图。
> - 如果无法触发崩溃：至少运行 5 小时并截图证明。

本目录提供一个最小 fuzz harness：`fuzz_notes_parser.cpp`。
同时提供一个可选的“崩溃演示开关”，用于在实验报告中快速得到可复现的 crash（不影响正常构建）：
- 编译时加 `-DENABLE_CRASH_DEMO` 才会启用。

## 1. 环境与依赖

推荐在 Linux/WSL(Ubuntu) 下运行（afl++ 更成熟）。

安装：

```bash
sudo apt update
sudo apt install -y afl++ clang lld
```

可选（便于更快发现问题）：

```bash
sudo apt install -y llvm
```

## 2. 编译 fuzz 目标

进入本目录：

```bash
cd Lab4/project/fuzz
```

使用 afl++ 的编译器 wrapper 编译（建议加 sanitizer）：

```bash
AFL_USE_ASAN=1 AFL_USE_UBSAN=1 afl-clang-fast++ -O0 -g -fno-omit-frame-pointer \
  -DENABLE_CRASH_DEMO fuzz_notes_parser.cpp -o fuzz_notes_parser
```

验证运行：

```bash
echo "AAAA" > seed
./fuzz_notes_parser seed
```

## 3. 准备初始语料并运行 afl-fuzz

准备 input 目录：

```bash
mkdir -p in out
printf "LEDG" > in/seed1
printf "BUDGET:100" > in/seed2
```

开始 fuzz：

```bash
# 使用 notes.dict（包含 "CRASH" 等 token）提升覆盖关键分支的概率
AFL_NO_UI=1 afl-fuzz -i in -o out -x notes.dict -m none -- ./fuzz_notes_parser @@
```

## 4. 收集崩溃与复现（如果出现）

如果检测到崩溃，afl++ 会把触发崩溃的样本放到类似目录：

- `out/crashes/`

复现方式：

```bash
./fuzz_notes_parser out/crashes/<crash_file>
```

请截图：
- afl-fuzz 运行界面（显示发现 crash）
- `crashes/` 目录里 testcase 文件
- 复现崩溃的终端输出

## 5. 如果没有崩溃：5 小时证明

如果跑不出崩溃：
- 保持 afl-fuzz 持续运行 >= 5 小时
- 截图保存 afl-fuzz 状态页（包含运行时长、execs、paths 等信息）

## 6. 报告里怎么写（建议结构）

- 工具选型：afl++ + clang/asan/ubsan
- 安装过程截图
- 使用方法：编译命令、运行命令截图
- 结果：
  - 有 crash：记录 testcase + 复现步骤
  - 无 crash：提供 5 小时运行证明
