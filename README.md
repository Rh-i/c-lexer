# c-lexer

《编译技术》实验一：C 语言子集的词法分析器。手工实现状态转换扫描，把源程序切成记号并输出二元式，同时完成注释删除、空白删除、行号回显与错误定位。

## 交付内容

| 内容 | 位置 |
| --- | --- |
| 源代码 | `lexer/include/lexer/`、`lexer/src/` |
| 测试用例 | `lexer/tests/cases/*.c` |
| 期望输出 | `lexer/tests/expected/*.out` |
| 测试结果 | `docs/测试结果.md` |
| 使用说明 | `docs/使用说明.md` |
| 预编译产物 | `lexer/bin/lexer-linux-x86_64` |
| 实验报告 | `report/out/编译技术实验一报告-词法分析器.pdf` |

## 快速开始

### 直接运行预编译版本（Linux x86_64）

```bash
./lexer/run.sh 你的文件.c
```

`run.sh` 会依次查找 `lexer/bin/lexer-linux-x86_64`、`lexer/build/lexer`，都没有才现场构建。

### 从源码构建

```bash
cmake -S lexer -B lexer/build -DCMAKE_BUILD_TYPE=Release
cmake --build lexer/build -j
lexer/build/lexer 你的文件.c
```

静态链接便于分发：

```bash
cmake -S lexer -B lexer/build -DLEXER_STATIC=ON -DCMAKE_BUILD_TYPE=Release
```

Windows 下用同样两条命令，MSVC 会加 `--config Release`，产物在 `lexer\build\Release\lexer.exe`：

```bat
lexer\run.bat 你的文件.c
```

## 输出示例

```
=== tests/cases/t01_program.c ===
  2 | int main()
    | (6,"int") (6,"main") (7,"(") (7,")")
  3 |     int a = 12;
    | (6,"int") (1,"a") (7,"=") (2,"12") (7,";")
```

每行先回显源程序，下一行缩进给出该行的全部二元式 `(种别码,"属性")`。行尾附诊断与统计。

## 语言子集

保留字 21 个（指导书表内 `unsigned` 重复一次，已去重）：

```
auto  break  case  char  const  continue  do  double  else  enum
float if     int   long  main   return    static switch unsigned void while
```

运算符与界符：单字符 `+ - * / = < > ; ( )`，双字符 `== != <= >= && ||`。

注释支持 `{ }`、`//`、`/* */` 三种，均不嵌套。

`! & | [ ] , . # %` 等未列于课程表的字符按非法字符报错。

## 错误码

| 编码 | 含义 |
| --- | --- |
| E101 | 非法字符 |
| E102 | 字符常量未闭合 |
| E103 | 字符常量内容为空或超过一个字符 |
| E104 | 非法转义序列 |
| E105 | 字符串常量未闭合 |
| E201 | 十六进制常量缺少数字 |
| E202 | 八进制常量中出现 8 或 9 |
| E203 | 浮点常量缺少指数部分 |
| E204 | 非法数值后缀 |
| E301 | 注释未闭合 |
| E401 | 标识符长度超过 63 |

## 测试

```bash
cd lexer && ./tests/run_tests.sh
```

Windows 下用 `lexer\tests\run_tests.bat`。脚本逐用例比对 `tests/expected`，并刷新 `docs/测试结果.md`。

## 退出码

`0` 无词法错误，`1` 存在词法错误，`2` 用法错误或文件不可读。
