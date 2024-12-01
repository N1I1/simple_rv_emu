## 这是什么

一个非常非常非常简单的 riscv 模拟器

## 为什么要有

1. 试用 copilot (copilot 比我强多了)
2. 测试 cpu

## 怎么用

将 hex 文件放在 assets 下

`make run` 生成 reg.log 在 build 目录下

### log 输出格式
格式
```plaintext
PC: 0x...   Instr: 0x...
x0: 0x...  ...
x4: 0x...
...
x28: 0x...

PC:.....
```

### 退出指令

退出指令设置为 `0xffffffff`，也就是说，需要在指令最后加上 `ffffffff` 退出。

### 最大执行指令数

另外设置了最大执行指令数量，默认为 100。

### 支持设置

`./build/emulator hex_file start_pc num_instrs log_file log_enabled`